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
#include "vppiecelist.h"
#include "vpsheet.h"

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
    const int index = ui->comboBoxPieceList->currentIndex();

    // --- clears the content of the carrousel
    Clear();

    // --- add the content saved in the layout to the carrousel.
    // Do not rely on m_layout because we do not control it.
    m_pieceLists = QList<VPCarrouselSheet>();

    if (m_layout != nullptr)
    {
        {
            VPCarrouselSheet carrouselSheet;
            carrouselSheet.unplaced = true;
            carrouselSheet.name = tr("Unplaced pieces");
            carrouselSheet.pieces = m_layout->GetUnplacedPieceList();

            m_pieceLists.append(carrouselSheet);
        }

        QList<VPSheet *> sheets = m_layout->GetSheets();
        for (auto *sheet : sheets)
        {
            VPCarrouselSheet carrouselSheet;
            carrouselSheet.unplaced = false;
            carrouselSheet.name = sheet->GetName();
            carrouselSheet.pieces = sheet->GetPieceList();

            m_pieceLists.append(carrouselSheet);
        }

        ui->comboBoxPieceList->blockSignals(true);

        for (const auto& sheet: m_pieceLists)
        {
            ui->comboBoxPieceList->addItem(GetSheetName(sheet));
        }

        ui->comboBoxPieceList->blockSignals(false);
    }

    ui->comboBoxPieceList->setCurrentIndex(-1);
    ui->comboBoxPieceList->setCurrentIndex(index != -1 ? index : 0);

    RefreshOrientation();
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrousel::RefreshSheetNames()
{
    // Here we assume that order and number of sheets are the same in layout and here
    QList<VPSheet *> sheets = m_layout->GetSheets();
    if (m_pieceLists.size() != sheets.size()+1)
    {
        return;
    }

    for (int i=0; i < sheets.size(); ++i)
    {
        m_pieceLists[i+1].name = sheets.at(i)->GetName();
        ui->comboBoxPieceList->setItemText(i+1, GetSheetName(m_pieceLists.at(i+1)));
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
    }
    else
    {
        ui->listWidget->SetCurrentPieceList(nullptr);
    }
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
void VPCarrousel::ClearSelection()
{
    if (m_layout != nullptr)
    {
        m_layout->ClearSelection();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrousel::ClearSelectionExceptForCurrentPieceList()
{
    if (m_layout != nullptr)
    {
        m_layout->ClearSelectionExceptForGivenPieceList(ui->listWidget->GetCurrentPieceList());
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
    return sheet.unplaced ? sheet.name : tr("Pieces of ") + sheet.name;
}
