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
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QtMath>
#include <QGraphicsScene>

#include "vpuzzlepiece.h"
#include "vpuzzlelayer.h"
#include "vpuzzlelayout.h"

#include <QLoggingCategory>
Q_LOGGING_CATEGORY(pGraphicsPiece, "p.graphicsPiece")

//---------------------------------------------------------------------------------------------------------------------
VPuzzleGraphicsPiece::VPuzzleGraphicsPiece(VPuzzlePiece *piece, QGraphicsItem *parent) :
    QGraphicsObject(parent),
    m_piece(piece),
    m_cuttingLine(QPainterPath()),
    m_seamLine(QPainterPath()),
    m_grainline(QPainterPath()),
    m_rotationStartPoint(QPointF())
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
    setAcceptHoverEvents(true);
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
    connect(m_piece, &VPuzzlePiece::RotationChanged, this, &VPuzzleGraphicsPiece::on_PieceRotationChanged);
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

    if((event->button() == Qt::LeftButton) && (event->modifiers() & Qt::AltModifier))
    {
        m_rotationStartPoint = event->scenePos();

        QPixmap cursor_pixmap = QPixmap(":/cursor_rotate");
        cursor_pixmap = cursor_pixmap.scaledToWidth(32);
        QCursor cursor_rotate = QCursor(cursor_pixmap, 16, 16);

        setCursor(cursor_rotate);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleGraphicsPiece::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    if((event->buttons() == Qt::LeftButton) && (event->modifiers() & Qt::AltModifier))
    {

        QPointF rotationNewPoint = event->scenePos();
        QPointF rotationCenter = sceneBoundingRect().center();

        // get the angle from the center to the initial click point
          qreal init_x = m_rotationStartPoint.x() - rotationCenter.x();
          qreal init_y = m_rotationStartPoint.y() - rotationCenter.y();
          qreal initial_angle = qAtan2(init_y, init_x);

          qreal x = rotationNewPoint.x() - rotationCenter.x();
          qreal y = rotationNewPoint.y() - rotationCenter.y();
          qreal mv_angle = qAtan2(y,x);

          qreal angle = (initial_angle-mv_angle)*180/M_PI;

          setTransformOriginPoint(boundingRect().center());
          setRotation(-(angle+m_piece->GetRotation()));
          event->accept();
    }
    else
    {
       QGraphicsItem::mouseMoveEvent(event);
    }
}


//---------------------------------------------------------------------------------------------------------------------
void VPuzzleGraphicsPiece::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    bool selectionState = isSelected();

    //perform the default behaviour
    QGraphicsItem::mouseReleaseEvent(event);

    // change the cursor when clicking left button
    if (event->button() == Qt::LeftButton)
    {
        setCursor(Qt::OpenHandCursor);

        setSelected(selectionState);

        if(m_piece->GetPosition() != pos())
        {
            m_piece->SetPosition(pos());
        }
    }

    if((event->button() == Qt::LeftButton) && (event->modifiers() & Qt::AltModifier))
    {
        m_piece->SetRotation(-rotation());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleGraphicsPiece::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{

    if(event->modifiers() & Qt::AltModifier)
    {
        // TODO FIXME: find a more efficient way

        QPixmap cursor_pixmap = QPixmap(":/cursor_rotate");
        cursor_pixmap = cursor_pixmap.scaledToWidth(32);
        QCursor cursor_rotate = QCursor(cursor_pixmap, 16, 16);

        setCursor(cursor_rotate);
    }
    else
    {
        setCursor(QCursor(Qt::OpenHandCursor));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleGraphicsPiece::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu contextMenu;

    // move to layer actions  -- TODO : To be tested properly when we have several layers
    QList<VPuzzleLayer*> layers = m_piece->GetLayer()->GetLayout()->GetLayers();
    layers.removeAll(m_piece->GetLayer());

    if(layers.count() > 0)
    {
        QMenu *moveMenu = contextMenu.addMenu(tr("Move to"));

        // TODO order in alphabetical order

        for (auto layer : layers)
        {
            QAction* moveToLayer = moveMenu->addAction(layer->GetName());
            QVariant data = QVariant::fromValue(layer);
            moveToLayer->setData(data);

            connect(moveToLayer, &QAction::triggered, this, &VPuzzleGraphicsPiece::on_ActionPieceMovedToLayer);
        }
    }

    // remove from layout action
    QAction *removeAction = contextMenu.addAction(tr("Remove from Layout"));
    QVariant data = QVariant::fromValue(m_piece->GetLayer()->GetLayout()->GetUnplacedPiecesLayer());
    removeAction->setData(data);
    connect(removeAction, &QAction::triggered, this, &VPuzzleGraphicsPiece::on_ActionPieceMovedToLayer);

    contextMenu.exec(event->screenPos());
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleGraphicsPiece::on_ActionPieceMovedToLayer()
{
    QAction *act = qobject_cast<QAction *>(sender());
    QVariant v = act->data();
    VPuzzleLayer *layer = v.value<VPuzzleLayer *>();
    if(layer != nullptr)
    {
        layer->GetLayout()->MovePieceToLayer(m_piece, layer);
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
void VPuzzleGraphicsPiece::on_PieceRotationChanged()
{
    setTransformOriginPoint(boundingRect().center());
    setRotation(-m_piece->GetRotation());
}

//---------------------------------------------------------------------------------------------------------------------
QVariant VPuzzleGraphicsPiece::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (scene()) {

        // we do this in the mouseRelease button to avoid updated this property all the time.
//        if(change == ItemPositionHasChanged)
//        {
//            blockSignals(true);
//            m_piece->SetPosition(pos());
//            blockSignals(false);
//        }

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

