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
#include <QLabel>
#include <QMessageBox>

//---------------------------------------------------------------------------------------------------------------------
VPieceCarrousel::VPieceCarrousel(QWidget *parent) :
    QWidget(parent),
    comboBoxLayer(new QComboBox),
    mainScrollArea(new QScrollArea(this)),
    layers(QList<QWidget *>())
{

    QVBoxLayout *mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    setMinimumSize(140,140);

    mainLayout->addWidget(comboBoxLayer);
    comboBoxLayer->addItem(tr("Unplaced pieces"));
    comboBoxLayer->addItem(tr("Layout"));
    comboBoxLayer->setCurrentIndex(0);
    connect(comboBoxLayer, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
              &VPieceCarrousel::ActiveLayerChanged);

    QWidget *widget = new QWidget();
    QVBoxLayout *mainScrollAreaLayout = new QVBoxLayout();
    mainScrollAreaLayout->setMargin(0);
    widget->setLayout(mainScrollAreaLayout);
    mainScrollArea->setWidget(widget);

    mainLayout->addWidget(mainScrollArea);
//    mainScrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    mainScrollArea->setWidgetResizable( true );


    // this code is for test purpuses, it needs to be updated when we have proper data!

    QWidget *unplacedPieces = new QWidget();
    QVBoxLayout *unplacedPiecesLayout = new QVBoxLayout();
    unplacedPiecesLayout->setMargin(0);
    unplacedPieces->setLayout(unplacedPiecesLayout);
    for(int i=0; i<=10; ++i)
    {
        QLabel *myLabel = new QLabel();
        myLabel->setText(QString ("Element A.%1").arg(i));
        myLabel->setFixedSize(120,120);
        myLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        if(i%2 ==0)
        {
            myLabel->setStyleSheet("background-color:white");
        }
        else {
            myLabel->setStyleSheet("background-color:red");
        }
        unplacedPiecesLayout->addWidget(myLabel);
    }
    mainScrollAreaLayout->addWidget(unplacedPieces);
    layers.append(unplacedPieces);

    QWidget *layoutPieces = new QWidget();
    QVBoxLayout *layoutPiecesLayout = new QVBoxLayout();
    layoutPiecesLayout->setMargin(0);
    layoutPieces->setLayout(layoutPiecesLayout);
    for(int i=0; i<=5; ++i)
    {
        QLabel *myLabel = new QLabel();
        myLabel->setText(QString ("Element B.%1").arg(i));
        myLabel->setFixedSize(120,120);
        myLabel->sizePolicy();
        myLabel->setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
        myLabel->setStyleSheet("background-color:cornflowerblue");
        layoutPiecesLayout->addWidget(myLabel);
    }
    mainScrollAreaLayout->addWidget(layoutPieces);
    layers.append(layoutPieces);

    QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Expanding);
    mainScrollAreaLayout->addSpacerItem(spacer);

    // -------------------- init the layers combobox ---------------------
    ActiveLayerChanged(0);
}

//---------------------------------------------------------------------------------------------------------------------
VPieceCarrousel::~VPieceCarrousel()
{
    delete comboBoxLayer;
    delete mainScrollArea;
}


//---------------------------------------------------------------------------------------------------------------------
void VPieceCarrousel::ActiveLayerChanged(int index)
{
    int j=0;
    for (QWidget *widget: layers) {
        widget->setVisible(j == index);
        j++;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceCarrousel::setOrientation(Qt::Orientation orientation)
{
    QBoxLayout::Direction direction = QBoxLayout::LeftToRight;

    if(orientation == Qt::Horizontal)
    {
        comboBoxLayer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    }
    else // Qt::Vertical
    {
        direction = QBoxLayout::TopToBottom;
        comboBoxLayer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    }

    QBoxLayout* mainScrollAreaLayout = qobject_cast<QBoxLayout*>(mainScrollArea->widget()->layout());
    mainScrollAreaLayout->setDirection(direction);

    for (QWidget *widget: layers) {
        QBoxLayout* layerLayout = qobject_cast<QBoxLayout*>(widget->layout());
        layerLayout->setDirection(direction);
    }
}

