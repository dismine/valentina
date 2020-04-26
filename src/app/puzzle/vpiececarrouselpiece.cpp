/************************************************************************
 **
 **  @file   vpiececarrouselpiece.cpp
 **  @author Ronan Le Tiec
 **  @date   25 4, 2020
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

#include "vpiececarrouselpiece.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QGraphicsScene>
#include <QPainter>

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pCarrouselPiece, "p.carrouselPiece")

//---------------------------------------------------------------------------------------------------------------------
VPieceCarrouselPiece::VPieceCarrouselPiece(VPuzzlePiece *piece, QWidget *parent) : QWidget(parent), m_piece(piece)
{
    Init();
}


//---------------------------------------------------------------------------------------------------------------------
VPieceCarrouselPiece::~VPieceCarrouselPiece()
{
    delete m_graphicsView;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceCarrouselPiece::Init()
{
    //m_label->setStyleSheet("background-color:cornflowerblue");

    // Define the structure
    setFixedSize(120,120);
    QVBoxLayout *pieceLayout = new QVBoxLayout();
    pieceLayout->setMargin(0);
    setLayout(pieceLayout);

    // define the preview of the piece
    m_graphicsView = new QGraphicsView(this);
    QGraphicsScene *graphicsScene = new QGraphicsScene(this);
    m_graphicsView->setScene(graphicsScene);
    m_graphicsView->setFixedSize(120,100);

    // define the label
    m_label = new QLabel();
    m_label->sizePolicy();
    m_label->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    m_label->setFixedSize(120,20);

    pieceLayout->addWidget(m_graphicsView);
    pieceLayout->addWidget(m_label);

    // then refresh the data
    Refresh();
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceCarrouselPiece::Refresh()
{
    // update the graphic view / the scene

    // TODO / FIXME : not perfect and maybe not the right way, still need to work on this
    QVector<QPointF> points = m_piece->GetCuttingLine();

    QPen pen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    pen.setCosmetic(true);
    QBrush noBrush(Qt::NoBrush);

    QPainterPath path;
    path.moveTo(points.first());
    for (int i = 1; i < points.size(); ++i)
        path.lineTo(points.at(i));
    m_graphicsView->scene()->addPath(path, pen, noBrush);

    m_graphicsView->fitInView(m_graphicsView->scene()->sceneRect(), Qt::KeepAspectRatio);

    // update the label of the piece
    QFontMetrics metrix(m_label->font());
    int width = m_label->width() - 8;
    QString clippedText = metrix.elidedText(m_piece->GetName(), Qt::ElideRight, width);
    m_label->setText(clippedText);

    m_label->setToolTip(m_piece->GetName());

}
