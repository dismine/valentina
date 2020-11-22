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
    // --- clears the content of the carrousel
    Clear();

    // --- add the content saved in the layout to the carrousel.
    // Do not rely on m_layout because we do not control it.
    m_pieceLists = QList<VPPieceList*>();
    m_pieceLists.append(m_layout->GetUnplacedPieceList());
    m_pieceLists.append(m_layout->GetFocusedSheet()->GetPieceList());

    ui->comboBoxPieceList->blockSignals(true);

    ui->comboBoxPieceList->addItem(m_layout->GetUnplacedPieceList()->GetName());
    ui->comboBoxPieceList->addItem(tr("Pieces of ") + m_layout->GetFocusedSheet()->GetName());

    ui->comboBoxPieceList->blockSignals(false);

    on_ActivePieceListChanged(0);

    RefreshOrientation();
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrousel::RefreshFocusedSheetName()
{
    // FIXME : This implementation will need a refactoring when we have multiple sheets, now it's not very nice!!

    ui->comboBoxPieceList->setItemText(1, tr("Pieces of ") + m_layout->GetFocusedSheet()->GetName());
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

    if (index >= 0 && index < m_pieceLists.size())
    {
        VPPieceList *pieceList = m_pieceLists.at(index);

        ui->listWidget->SetCurrentPieceList(pieceList);
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
    m_layout->ClearSelection();
}


//---------------------------------------------------------------------------------------------------------------------
void VPCarrousel::ClearSelectionExceptForCurrentPieceList()
{
    if (m_layout != nullptr)
    {
        m_layout->ClearSelectionExceptForGivenPieceList(ui->listWidget->GetCurrentPieceList());
    }
}
