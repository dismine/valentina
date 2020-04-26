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

#include "../vmisc/backport/qoverload.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pCarrousel, "p.carrousel")

//---------------------------------------------------------------------------------------------------------------------
VPieceCarrousel::VPieceCarrousel(VPuzzleLayout *layout, QWidget *parent) :
    QWidget(parent),
    m_layout(layout),
    m_comboBoxLayer(new QComboBox(this)),
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
    QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);

    layersContainerWrapperLayout->addWidget(m_layersContainer);
    layersContainerWrapperLayout->addSpacerItem(spacer);

    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable( true );
    scrollArea->setWidget(layersContainerWrapper);

    // init the layout of the piece carrousel
    QVBoxLayout *mainLayout = new QVBoxLayout();
    setLayout(mainLayout);
    setMinimumSize(140,140);

    mainLayout->addWidget(m_comboBoxLayer);
    mainLayout->addWidget(scrollArea);

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

        qCDebug(pCarrousel, "layer name : %s", layer->GetName().toStdString().c_str());

        // add new carrousel layer
        VPieceCarrouselLayer *carrouselLayer = new VPieceCarrouselLayer(layer, this);
        m_carrouselLayers.append(carrouselLayer);
        m_layersContainer->layout()->addWidget(carrouselLayer);
    }

    m_comboBoxLayer->setCurrentIndex(0);
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

    // Removes and deletes the carrousel layer from the list
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
    QBoxLayout::Direction direction = QBoxLayout::LeftToRight;

    if(orientation == Qt::Horizontal)
    {
        m_comboBoxLayer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }
    else // Qt::Vertical
    {
        direction = QBoxLayout::TopToBottom;
        m_comboBoxLayer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    }

    // TODO: it's not updated anymore:

    QBoxLayout* mainScrollAreaLayout = qobject_cast<QBoxLayout*>(m_layersContainer->layout());
    mainScrollAreaLayout->setDirection(direction);

    for (VPieceCarrouselLayer *widget: m_carrouselLayers) {
        QBoxLayout* layerLayout = qobject_cast<QBoxLayout*>(widget->layout());
        layerLayout->setDirection(direction);
    }
}


