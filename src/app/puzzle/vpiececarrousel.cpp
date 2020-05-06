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
#include <QVBoxLayout>
#include <QMessageBox>
#include <QScrollBar>

#include "../vmisc/backport/qoverload.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pCarrousel, "p.carrousel")

//---------------------------------------------------------------------------------------------------------------------
VPieceCarrousel::VPieceCarrousel(VPuzzleLayout *layout, QWidget *parent) :
    QWidget(parent),
    m_layout(layout),
    m_comboBoxLayer(new QComboBox(this)),
    m_scrollArea(new QScrollArea(this)),
    m_layersContainer(new QWidget(this)),
    m_carrouselLayers(QList<VPieceCarrouselLayer *>())
{
    Init();
}

//---------------------------------------------------------------------------------------------------------------------
VPieceCarrousel::~VPieceCarrousel()
{
    delete m_comboBoxLayer;
    delete m_layersContainer;
}


//---------------------------------------------------------------------------------------------------------------------
void VPieceCarrousel::Init()
{
    // ------ first we initialize the structure of the carrousel

    // init the combo box
    connect(m_comboBoxLayer, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
              &VPieceCarrousel::on_ActiveLayerChanged);

    // init the layers container and corresponding scroll area
    QWidget *layersContainerWrapper = new QWidget();

    QVBoxLayout *layersContainerWrapperLayout = new QVBoxLayout();
    layersContainerWrapperLayout->setMargin(0);
    layersContainerWrapper->setLayout(layersContainerWrapperLayout);

    QVBoxLayout *layersContainerLayout = new QVBoxLayout();
    layersContainerLayout->setMargin(0);
    m_layersContainer->setLayout(layersContainerLayout);
    m_layersContainer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);


    layersContainerWrapperLayout->addWidget(m_layersContainer);
    layersContainerWrapperLayout->addSpacerItem(spacer);

    m_scrollArea->setWidgetResizable( true );
    m_scrollArea->setWidget(layersContainerWrapper);

    // init the layout of the piece carrousel
    QVBoxLayout *mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    mainLayout->addWidget(m_comboBoxLayer);
    mainLayout->addWidget(m_scrollArea);

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
    QList<VPuzzleLayer*> layers = m_layout->GetLayers();
    layers.prepend(m_layout->GetUnplacedPiecesLayer());

    for (auto layer : layers)
    {
        // add layer name to combo
        m_comboBoxLayer->addItem(layer->GetName());

        // add new carrousel layer
        VPieceCarrouselLayer *carrouselLayer = new VPieceCarrouselLayer(layer, this);
        m_carrouselLayers.append(carrouselLayer);
        m_layersContainer->layout()->addWidget(carrouselLayer);
    }

    on_ActiveLayerChanged(0);

    RefreshOrientation();
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceCarrousel::Clear()
{
    // remove the combobox entries
    int layerCount = m_comboBoxLayer->count();
    for(int i=0;i<layerCount;i++)
    {
        m_comboBoxLayer->removeItem(0);
    }

    // remove the carrousel layers from the qlayout
    while(!m_layersContainer->layout()->isEmpty())
    {
        QLayoutItem* item = m_layersContainer->layout()->takeAt(0);
        if(item != nullptr)
        {
            delete item;
        }
    }

    // Removes and deletes the carrousel layers from the list
    while (!m_carrouselLayers.isEmpty())
    {
        VPieceCarrouselLayer *carrouselLayer = m_carrouselLayers.takeLast();
        if(carrouselLayer != nullptr)
        {
            delete carrouselLayer;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceCarrousel::on_ActiveLayerChanged(int index)
{
    qCDebug(pCarrousel, "index changed %i", index);

    int j=0;
    for (VPieceCarrouselLayer *carrouselLayer: m_carrouselLayers) {
        carrouselLayer->setVisible(j == index);
        j++;
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
    QBoxLayout::Direction direction = (m_orientation == Qt::Horizontal)?
                QBoxLayout::LeftToRight
                :
                QBoxLayout::TopToBottom;

    // Update the various qlayouts
    QBoxLayout* mainScrollAreaLayout = qobject_cast<QBoxLayout*>(m_layersContainer->layout());
    mainScrollAreaLayout->setDirection(direction);

    QBoxLayout* layerContainerWrapper = qobject_cast<QBoxLayout*>(m_scrollArea->widget()->layout());
    layerContainerWrapper->setDirection(direction);

    for (VPieceCarrouselLayer *widget: m_carrouselLayers) {
        QBoxLayout* layerLayout = qobject_cast<QBoxLayout*>(widget->layout());
        layerLayout->setDirection(direction);
    }

    // then update the scrollarea min height / width and scrollbar behaviour
    if(m_orientation == Qt::Horizontal)
    {
        m_comboBoxLayer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

        // scroll bar policy of scroll area
        m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

        // FIXME: find a nicer way than putting directly the 120 height of the piece
        m_scrollArea->setMinimumHeight(128 + m_scrollArea->horizontalScrollBar()->sizeHint().height()+2);
        m_scrollArea->setMinimumWidth(0);
    }
    else // Qt::Vertical
    {
        m_comboBoxLayer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        // scroll bar policy of scroll area
        m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

        m_scrollArea->setMinimumHeight(0);
        m_scrollArea->setMinimumWidth(124 + m_scrollArea->verticalScrollBar()->sizeHint().width()+2);
        // FIXME: find a nicer way than putting directly the 120 width of the piece
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceCarrousel::ClearSelection()
{
    m_layout->ClearSelection();
}
