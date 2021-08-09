/************************************************************************
 **
 **  @file   vpcarrousel.cpp
 **  @author Ronan Le Tiec
 **  @date   13 4, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
 **  <https://gitlab.com/smart-pattern/valentina> All Rights Reserved.
 **
 **  Valentina is free software: you can redistribute it and/or modify
 **  it under the terms of the GNU General Public License as published by
 **  the Free Software Foundation, either version 3 of the License, or
 **  (at your option) any later version.
 **
 **  Valentina is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 **  GNU General Public License for more details.
 **
 **  You should have received a copy of the GNU General Public License
 **  along with Valentina.  If not, see <http://www.gnu.org/licenses/>.
 **
 *************************************************************************/
#include "vpcarrousel.h"
#include "ui_vpcarrousel.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QScrollBar>
#include <QFontMetrics>

#include "../vmisc/backport/qoverload.h"
#include "../layout/vpsheet.h"

#include <QLoggingCategory>
#include <QMenu>
#include <QPainter>

Q_LOGGING_CATEGORY(pCarrousel, "p.carrousel")

//---------------------------------------------------------------------------------------------------------------------
VPCarrousel::VPCarrousel(VPLayout *layout, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VPCarrousel),
    m_layout(layout)
{
    SCASSERT(m_layout != nullptr)
    ui->setupUi(this);
    ui->listWidget->SetCarrousel(this);

    // init the combo box
    connect(ui->comboBoxPieceList, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &VPCarrousel::on_ActivePieceListChanged);

    // ------ then we fill the carrousel with the layout content
    Refresh();
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrousel::Refresh()
{
    const QUuid sheetUuid = ui->comboBoxPieceList->currentData().toUuid();

    // --- clears the content of the carrousel
    ui->comboBoxPieceList->blockSignals(true);
    Clear();
    ui->comboBoxPieceList->blockSignals(false);

    // --- add the content saved in the layout to the carrousel.
    // Do not rely on m_layout because we do not control it.
    m_pieceLists = QList<VPCarrouselSheet>();

    if (m_layout != nullptr)
    {
        {
            VPCarrouselSheet carrouselSheet;
            carrouselSheet.unplaced = true;
            carrouselSheet.active = false;
            carrouselSheet.name = tr("Unplaced pieces");
            carrouselSheet.pieces = m_layout->GetUnplacedPieces();

            m_pieceLists.append(carrouselSheet);
        }

        QList<VPSheet *> sheets = m_layout->GetSheets();
        for (auto *sheet : sheets)
        {
            if (sheet->IsVisible())
            {
                VPCarrouselSheet carrouselSheet;
                carrouselSheet.unplaced = false;
                carrouselSheet.active = (sheet == m_layout->GetFocusedSheet());
                carrouselSheet.name = sheet->GetName();
                carrouselSheet.pieces = sheet->GetPieces();
                carrouselSheet.sheetUuid = sheet->Uuid();

                m_pieceLists.append(carrouselSheet);
            }
        }

        ui->comboBoxPieceList->blockSignals(true);

        for (const auto& sheet: m_pieceLists)
        {
            ui->comboBoxPieceList->addItem(GetSheetName(sheet), sheet.sheetUuid);
        }

        ui->comboBoxPieceList->blockSignals(false);
    }

    ui->comboBoxPieceList->blockSignals(true);
    ui->comboBoxPieceList->setCurrentIndex(-1);
    ui->comboBoxPieceList->blockSignals(false);

    int index = ui->comboBoxPieceList->findData(sheetUuid);
    ui->comboBoxPieceList->setCurrentIndex(index != -1 ? index : 0);

    RefreshOrientation();
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrousel::RefreshSheetNames()
{
    for (int i=0; i < m_pieceLists.size(); ++i)
    {
        if (not m_pieceLists.at(i).unplaced)
        {
            VPSheet *sheet = m_layout->GetSheet(m_pieceLists.at(i).sheetUuid);
            if (sheet != nullptr)
            {
                m_pieceLists[i].name = sheet->GetName();
                m_pieceLists[i].active = (sheet == m_layout->GetFocusedSheet());
            }
        }
        else
        {
            m_pieceLists[i].name = tr("Unplaced pieces");
        }

        ui->comboBoxPieceList->setItemText(i, GetSheetName(m_pieceLists.at(i)));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrousel::Clear()
{
    // remove the combobox entries
    ui->comboBoxPieceList->clear();

    ui->listWidget->clear();
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrousel::on_ActivePieceListChanged(int index)
{
    qCDebug(pCarrousel, "index changed %i", index);

    if (not m_pieceLists.isEmpty() && index >= 0 && index < m_pieceLists.size())
    {
        ui->listWidget->SetCurrentPieceList(m_pieceLists.at(index).pieces);

        if (index > 0)
        {
            QUuid sheetUuid = ui->comboBoxPieceList->currentData().toUuid();
            VPSheet *sheet = m_layout->GetSheet(sheetUuid);

            if (sheet != nullptr)
            {
                m_layout->SetFocusedSheet(sheet);
                emit on_ActiveSheetChanged();
            }
        }
    }
    else
    {
        ui->listWidget->SetCurrentPieceList(QList<VPPiece *>());
        m_layout->SetFocusedSheet(nullptr);
        emit on_ActiveSheetChanged();
    }

    RefreshSheetNames();
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrousel::SetOrientation(Qt::Orientation orientation)
{
    m_orientation = orientation;
    RefreshOrientation();
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrousel::RefreshOrientation()
{
    // then update the scrollarea min height / width and scrollbar behaviour
    if(m_orientation == Qt::Horizontal)
    {
        ui->comboBoxPieceList->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        // scroll bar policy of scroll area
        ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

        ui->listWidget->setFlow(QListView::TopToBottom);
    }
    else // Qt::Vertical
    {
        ui->comboBoxPieceList->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        // scroll bar policy of scroll area
        ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        ui->listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

        ui->listWidget->setFlow(QListView::LeftToRight);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrousel::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        // retranslate designer form (single inheritance approach)
        ui->retranslateUi(this);

        RefreshSheetNames();
        on_ActivePieceListChanged(ui->comboBoxPieceList->currentIndex());
    }

    // remember to call base class implementation
    QWidget::changeEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPCarrousel::GetSheetName(const VPCarrouselSheet &sheet) -> QString
{
    if (sheet.unplaced)
    {
        return sheet.name;
    }

    if (sheet.active)
    {
        return QStringLiteral("--> %1 %2 <--").arg(tr("Pieces of"), sheet.name);
    }

    return tr("Pieces of ") + sheet.name;
}
