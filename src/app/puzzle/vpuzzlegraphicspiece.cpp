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
    m_seamLine(QPainterPath()),
    m_grainline(QPainterPath())
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

    // initialises the grainline
    QVector<QPointF> grainLinepoints = m_piece->GetGrainline();
    m_grainline.moveTo(grainLinepoints.first());
    for (int i = 1; i < grainLinepoints.size(); ++i)
        m_grainline.lineTo(grainLinepoints.at(i));


    // TODO : initialises the other elements labels, passmarks etc.

    // Initialises the connectors
    connect(m_piece, &VPuzzlePiece::SelectionChanged, this, &VPuzzleGraphicsPiece::on_PieceSelectionChanged);
    connect(m_piece, &VPuzzlePiece::PositionChanged, this, &VPuzzleGraphicsPiece::on_PiecePositionChanged);
}

//---------------------------------------------------------------------------------------------------------------------
VPuzzlePiece* VPuzzleGraphicsPiece::GetPiece()
{
    return m_piece;
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
    QBrush noBrush(Qt::NoBrush);
    QBrush selectionBrush(QColor(255,160,160,60));

    painter->setPen(pen);

    if(isSelected())
    {
       painter->setBrush(selectionBrush);
    }
    else
    {
        painter->setBrush(noBrush);
    }

    // paint the cutting line
    if(!m_cuttingLine.isEmpty())
    {
        painter->drawPath(m_cuttingLine);
        painter->setBrush(noBrush);
    }

    // paint the seam line
    if(!m_seamLine.isEmpty())
    {
        painter->drawPath(m_seamLine);
    }

    painter->setBrush(noBrush);

    // paint the grainline
    if(!m_grainline.isEmpty())
    {
        painter->drawPath(m_grainline);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleGraphicsPiece::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    bool selectionState = isSelected();
    //perform the default behaviour
    QGraphicsItem::mousePressEvent(event);

    // change the cursor when clicking left button
    if (event->button() == Qt::LeftButton)
    {
        setSelected(true);
        setCursor(Qt::ClosedHandCursor);

        if (event->modifiers() & Qt::ControlModifier)
        {
            setSelected(!selectionState);
        }
        else
        {
            setSelected(true);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleGraphicsPiece::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    bool selectionState = isSelected();

    //perform the default behaviour
    QGraphicsItem::mouseReleaseEvent(event);

    qCDebug(pGraphicsPiece, "piiiiieeece --- mouse release");

    // change the cursor when clicking left button

    if (event->button() == Qt::LeftButton)
    {
        setCursor(Qt::OpenHandCursor);

        qCDebug(pGraphicsPiece, "piiiiieeece --- left button");

        setSelected(selectionState);
    }
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

