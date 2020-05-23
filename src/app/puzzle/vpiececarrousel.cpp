/************************************************************************
 **
 **  @file   vpiececarrousel.cpp
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
#include "vpiececarrousel.h"
#include "ui_vpiececarrousel.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QScrollBar>

#include "../vmisc/backport/qoverload.h"
#include "vpuzzlelayer.h"

#include <QLoggingCategory>
#include <QMenu>
#include <QPainter>

Q_LOGGING_CATEGORY(pCarrousel, "p.carrousel")

//---------------------------------------------------------------------------------------------------------------------
VPieceCarrousel::VPieceCarrousel(VPuzzleLayout *layout, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VPieceCarrousel),
    m_layout(layout)
{
    ui->setupUi(this);

    // init the combo box
    connect(ui->comboBoxLayer, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &VPieceCarrousel::on_ActiveLayerChanged);

    ui->listWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    // ------ then we fill the carrousel with the layout content
    Refresh();
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceCarrousel::Refresh()
{
    // NOTE: alternative to clearing the carrousel and adding things again, we could make comparision

    // --- clears the content of the carrousel
    Clear();

    // --- add the content saved in the layout to the carrousel.
    // Do not rely on m_layout because we do not control it.
    m_layers = m_layout->GetLayers();
    m_layers.prepend(m_layout->GetUnplacedPiecesLayer());

    for (auto layer : m_layers)
    {
        // add layer name to combo
        ui->comboBoxLayer->blockSignals(true);
        ui->comboBoxLayer->addItem(layer->GetName());
        ui->comboBoxLayer->blockSignals(false);
    }

    on_ActiveLayerChanged(0);

    RefreshOrientation();
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceCarrousel::Clear()
{
    // remove the combobox entries
    ui->comboBoxLayer->clear();

    ui->listWidget->clear();
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceCarrousel::on_ActiveLayerChanged(int index)
{
    qCDebug(pCarrousel, "index changed %i", index);

    ui->listWidget->clear();

    if (index >= 0 && index < m_layers.size())
    {
        VPuzzleLayer *layer = m_layers.at(index);

        if (layer)
        {
            QList<VPuzzlePiece*> pieces = layer->GetPieces();

            for (auto piece : pieces)
            {
                new QListWidgetItem(piece->PieceIcon(QSize(120, 120)) , piece->GetName(), ui->listWidget);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceCarrousel::SetOrientation(Qt::Orientation orientation)
{
    m_orientation = orientation;
    RefreshOrientation();
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceCarrousel::RefreshOrientation()
{
    // then update the scrollarea min height / width and scrollbar behaviour
    if(m_orientation == Qt::Horizontal)
    {
        ui->comboBoxLayer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        // scroll bar policy of scroll area
        ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        ui->listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

        ui->listWidget->setFlow(QListView::TopToBottom);
    }
    else // Qt::Vertical
    {
        ui->comboBoxLayer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        // scroll bar policy of scroll area
        ui->listWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        ui->listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

        ui->listWidget->setFlow(QListView::LeftToRight);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceCarrousel::ClearSelection()
{
    m_layout->ClearSelection();
}
