/************************************************************************
 **
 **  @file   vpuzzlegraphicspiece.cpp
 **  @author Ronan Le Tiec
 **  @date   4 5, 2020
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

#include "vpuzzlegraphicspiece.h"

#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>

#include "vpuzzlepiece.h"

#include <QLoggingCategory>
Q_LOGGING_CATEGORY(pGraphicsPiece, "p.graphicsPiece")

//---------------------------------------------------------------------------------------------------------------------
VPuzzleGraphicsPiece::VPuzzleGraphicsPiece(VPuzzlePiece *piece, QGraphicsItem *parent) :
    QGraphicsObject(parent),
    m_piece(piece),
    m_cuttingLine(QPainterPath()),
    m_seamLine(QPainterPath())
{
    Init();
}

//---------------------------------------------------------------------------------------------------------------------
VPuzzleGraphicsPiece::~VPuzzleGraphicsPiece()
{

}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleGraphicsPiece::Init()
{
    // set some infos
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    setCursor(QCursor(Qt::OpenHandCursor));

    //setAcceptHoverEvents(true); // maybe we can do some stuff with this


    // initialises the seam line
    QVector<QPointF> seamLinePoints = m_piece->GetSeamLine();
    m_seamLine.moveTo(seamLinePoints.first());
    for (int i = 1; i < seamLinePoints.size(); ++i)
        m_seamLine.lineTo(seamLinePoints.at(i));

    // initiliases the cutting line
    QVector<QPointF> cuttingLinepoints = m_piece->GetCuttingLine();
    m_cuttingLine.moveTo(cuttingLinepoints.first());
    for (int i = 1; i < cuttingLinepoints.size(); ++i)
        m_cuttingLine.lineTo(cuttingLinepoints.at(i));


    // TODO : initialises the other elements like grain line, labels, passmarks etc.

    // Initialises the connectors
    connect(m_piece, &VPuzzlePiece::SelectionChanged, this, &VPuzzleGraphicsPiece::on_PieceSelectionChanged);
    connect(m_piece, &VPuzzlePiece::PositionChanged, this, &VPuzzleGraphicsPiece::on_PiecePositionChanged);
}


//---------------------------------------------------------------------------------------------------------------------
QRectF VPuzzleGraphicsPiece::boundingRect() const
{
    if(!m_cuttingLine.isEmpty())
    {
        return m_cuttingLine.boundingRect();
    }

    return m_seamLine.boundingRect();
}

//---------------------------------------------------------------------------------------------------------------------
QPainterPath VPuzzleGraphicsPiece::shape() const
{
    if(!m_cuttingLine.isEmpty())
    {
        return m_cuttingLine;
    }

    return m_seamLine;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleGraphicsPiece::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    QPen pen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    if(isSelected())
    {
        pen.setColor(Qt::red);
    }

    QBrush noBrush(Qt::NoBrush);

    painter->setPen(pen);
    painter->setBrush(noBrush);

    if(!m_cuttingLine.isEmpty())
    {
        painter->drawPath(m_cuttingLine);
    }

    if(!m_seamLine.isEmpty())
    {
        painter->drawPath(m_seamLine);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleGraphicsPiece::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //perform the default behaviour
    QGraphicsItem::mousePressEvent(event);

    // change the cursor when clicking left button
    if (!(event->buttons() & Qt::LeftButton))
    {
        return;
    }

    setSelected(true);

    setCursor(Qt::ClosedHandCursor);
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleGraphicsPiece::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    //perform the default behaviour
    QGraphicsItem::mouseReleaseEvent(event);

    // change the cursor when clicking left button
    if (!(event->buttons() & Qt::LeftButton))
    {
        return;
    }

    setCursor(Qt::OpenHandCursor);
}


//---------------------------------------------------------------------------------------------------------------------
void VPuzzleGraphicsPiece::on_PieceSelectionChanged()
{
    setSelected(m_piece->GetIsSelected());
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleGraphicsPiece::on_PiecePositionChanged()
{
    setPos(m_piece->GetPosition());
}

//---------------------------------------------------------------------------------------------------------------------
QVariant VPuzzleGraphicsPiece::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (scene()) {
        if(change == ItemPositionHasChanged)
        {
            blockSignals(true);
            m_piece->SetPosition(pos());
            blockSignals(false);
        }

        if(change == ItemSelectedHasChanged)
        {
            if(m_piece->GetIsSelected() != isSelected())
            {
                m_piece->SetIsSelected(isSelected());
            }
        }
    }

    return QGraphicsObject::itemChange(change, value);
}

