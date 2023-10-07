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
#include <QFontMetrics>
#include <QMessageBox>
#include <QScrollBar>
#include <QVBoxLayout>

#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../vmisc/backport/qoverload.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
#include "../layout/vplayout.h"
#include "../layout/vpsheet.h"

#include <QLoggingCategory>
#include <QMenu>
#include <QPainter>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(pCarrousel, "p.carrousel") // NOLINT

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
VPCarrousel::VPCarrousel(const VPLayoutPtr &layout, QWidget *parent)
  : QWidget(parent),
    ui(new Ui::VPCarrousel),
    m_layout(layout)
{
    SCASSERT(not layout.isNull())
    ui->setupUi(this);
    ui->listWidget->SetCarrousel(this);

    // init the combo box
    connect(ui->comboBoxPieceList, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &VPCarrousel::on_ActivePieceListChanged);

    connect(layout.data(), &VPLayout::ActiveSheetChanged, this, &VPCarrousel::on_ActiveSheetChanged);
    connect(layout.data(), &VPLayout::SheetListChanged, this, &VPCarrousel::Refresh);

    // ------ then we fill the carrousel with the layout content
    Refresh();
}

//---------------------------------------------------------------------------------------------------------------------
VPCarrousel::~VPCarrousel()
{
    delete ui;
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

    VPLayoutPtr layout = m_layout.toStrongRef();
    if (not layout.isNull())
    {
        {
            VPCarrouselSheet carrouselSheet;
            carrouselSheet.unplaced = true;
            carrouselSheet.active = false;
            carrouselSheet.name = tr("Unplaced pieces");
            carrouselSheet.pieces = layout->GetUnplacedPieces();

            m_pieceLists.append(carrouselSheet);
        }

        QList<VPSheetPtr> sheets = layout->GetSheets();
        for (const auto &sheet : sheets)
        {
            if (not sheet.isNull())
            {
                VPCarrouselSheet carrouselSheet;
                carrouselSheet.unplaced = false;
                carrouselSheet.active = (sheet == layout->GetFocusedSheet());
                carrouselSheet.name = sheet->GetName();
                carrouselSheet.pieces = sheet->GetPieces();
                carrouselSheet.sheetUuid = sheet->Uuid();

                m_pieceLists.append(carrouselSheet);
            }
        }

        ui->comboBoxPieceList->blockSignals(true);

        for (const auto &sheet : qAsConst(m_pieceLists))
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
void VPCarrousel::on_ActiveSheetChanged(const VPSheetPtr &sheet)
{
    if (not sheet.isNull())
    {
        int index = ui->comboBoxPieceList->findData(sheet->Uuid());
        if (index != -1)
        {
            ui->comboBoxPieceList->setCurrentIndex(index);
        }
    }
    else
    {
        ui->comboBoxPieceList->setCurrentIndex(0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrousel::RefreshSheetNames()
{
    VPLayoutPtr layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return;
    }

    for (int i = 0; i < m_pieceLists.size(); ++i)
    {
        if (not m_pieceLists.at(i).unplaced)
        {
            VPSheetPtr sheet = layout->GetSheet(m_pieceLists.at(i).sheetUuid);
            if (not sheet.isNull())
            {
                m_pieceLists[i].name = sheet->GetName();
                m_pieceLists[i].active = (sheet == layout->GetFocusedSheet());
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

    VPLayoutPtr layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return;
    }

    if (not m_pieceLists.isEmpty() && index >= 0 && index < m_pieceLists.size())
    {
        ui->listWidget->SetCurrentPieceList(m_pieceLists.at(index).pieces);

        if (index > 0)
        {
            QUuid sheetUuid = ui->comboBoxPieceList->currentData().toUuid();
            VPSheetPtr sheet = layout->GetSheet(sheetUuid);

            if (not sheet.isNull())
            {
                VPSheetPtr activeSheet = layout->GetFocusedSheet();
                if (not activeSheet.isNull())
                {
                    activeSheet->ClearSelection();
                }

                m_ignoreActiveSheetChange = true;
                layout->SetFocusedSheet(sheet);
                m_ignoreActiveSheetChange = false;
            }
        }
    }
    else
    {
        ui->listWidget->SetCurrentPieceList(QList<VPPiecePtr>());
        m_ignoreActiveSheetChange = true;
        layout->SetFocusedSheet(VPSheetPtr());
        m_ignoreActiveSheetChange = false;
    }

    RefreshSheetNames();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPCarrousel::Layout() const -> VPLayoutWeakPtr
{
    return m_layout;
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
    if (m_orientation == Qt::Horizontal)
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

    return tr("Pieces of") + ' '_L1 + sheet.name;
}
