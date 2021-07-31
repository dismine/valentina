/************************************************************************
 **
 **  @file   vpgraphicspiece.cpp
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

#include "vpgraphicspiece.h"

#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsSceneContextMenuEvent>
#include <QMenu>
#include <QtMath>
#include <QGraphicsScene>
#include <QApplication>

#include "vppiece.h"
#include "vplayout.h"
#include "vpsheet.h"

#include "vlayoutpiecepath.h"
#include "vplacelabelitem.h"

#include <QLoggingCategory>
Q_LOGGING_CATEGORY(pGraphicsPiece, "p.graphicsPiece")

//---------------------------------------------------------------------------------------------------------------------
VPGraphicsPiece::VPGraphicsPiece(VPPiece *piece, QGraphicsItem *parent) :
    QGraphicsObject(parent),
    m_piece(piece)
{
    QPixmap cursor_pixmap = QIcon("://puzzleicon/svg/cursor_rotate.svg").pixmap(QSize(32,32));
    m_rotateCursor= QCursor(cursor_pixmap, 16, 16);

    Init();
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::Init()
{
    // set some infos
    setFlags(ItemIsSelectable | ItemIsMovable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    setCursor(QCursor(Qt::OpenHandCursor));

    // initialises the seam line
    QVector<QPointF> seamLinePoints = m_piece->GetMappedContourPoints();
    if(!seamLinePoints.isEmpty())
    {
        m_seamLine.moveTo(seamLinePoints.first());
        for (int i = 1; i < seamLinePoints.size(); i++)
        {
            m_seamLine.lineTo(seamLinePoints.at(i));
        }
    }

    // initiliases the cutting line
    QVector<QPointF> cuttingLinepoints = m_piece->GetMappedSeamAllowancePoints();
    if(!cuttingLinepoints.isEmpty())
    {
        m_cuttingLine.moveTo(cuttingLinepoints.first());
        for (int i = 1; i < cuttingLinepoints.size(); i++)
        {
            m_cuttingLine.lineTo(cuttingLinepoints.at(i));
        }
    }

    // initialises the grainline
    if(m_piece->IsGrainlineEnabled())
    {
        QVector<QPointF> grainLinepoints = m_piece->GetMappedGrainline();
        if(!grainLinepoints.isEmpty())
        {
            m_grainline.moveTo(grainLinepoints.first());
            for (int i = 1; i < grainLinepoints.size(); i++)
            {
                m_grainline.lineTo(grainLinepoints.at(i));
            }
        }
    }

    // initialises the internal paths
    QVector<VLayoutPiecePath> internalPaths = m_piece->GetInternalPaths();
    for (const auto& piecePath : internalPaths)
    {
        QPainterPath path = m_piece->GetMatrix().map(piecePath.GetPainterPath());
        m_internalPaths.append(path);
        m_internalPathsPenStyle.append(piecePath.PenStyle());
    }

    // initialises the passmarks
    QVector<VLayoutPassmark> passmarks = m_piece->GetMappedPassmarks();
    for(auto &passmark : passmarks)
    {
        for (auto &line : passmark.lines)
        {
            m_passmarks.moveTo(line.p1());
            m_passmarks.lineTo(line.p2());
        }
    }

    // initialises the place labels (buttons etc)
    QVector<VLayoutPlaceLabel> placeLabels = m_piece->GetPlaceLabels();
    for(auto &placeLabel : placeLabels)
    {
        QPainterPath path = VPlaceLabelItem::LabelShapePath(placeLabel.shape);
        m_placeLabels.append(path);
    }

    // TODO : initialises the text labels


    // Init position
    on_PiecePositionChanged();
    on_PieceRotationChanged();
    on_PieceSelectionChanged();

    // Initialises the connectors
    connect(m_piece, &VPPiece::SelectionChanged, this, &VPGraphicsPiece::on_PieceSelectionChanged);
    connect(m_piece, &VPPiece::PositionChanged, this, &VPGraphicsPiece::on_PiecePositionChanged);
    connect(m_piece, &VPPiece::RotationChanged, this, &VPGraphicsPiece::on_PieceRotationChanged);
    connect(m_piece, &VPPiece::PropertiesChanged, this, &VPGraphicsPiece::on_PiecePropertiesChanged);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPiece::GetPiece() -> VPPiece*
{
    return m_piece;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPiece::boundingRect() const -> QRectF
{
    if(!m_cuttingLine.isEmpty())
    {
        return m_cuttingLine.boundingRect();
    }

    return m_seamLine.boundingRect();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPiece::shape() const -> QPainterPath
{
    if(!m_cuttingLine.isEmpty())
    {
        return m_cuttingLine;
    }

    return m_seamLine;
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    QPen pen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    QBrush noBrush(Qt::NoBrush);
    QBrush selectionBrush(QColor(255,160,160,60));
    QBrush blackBrush(Qt::black);

    painter->setPen(pen);

    // selection
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
    if(!m_seamLine.isEmpty() && m_piece->GetShowSeamLine())
    {
        painter->drawPath(m_seamLine);
    }

    painter->setBrush(noBrush);

    // paint the grainline
    if(!m_grainline.isEmpty())
    {
        // here to fill the grainlines arrow. Not wanted for mvp
        // later maybe if it's configurable
//        painter->setBrush(blackBrush);

        painter->drawPath(m_grainline);
    }

    // paint the internal paths
    painter->setBrush(noBrush);
    if(!m_internalPaths.isEmpty())
    {
        Qt::PenStyle penStyleTmp = pen.style();

        for (int i = 0; i < m_internalPaths.size(); i++)
        {
            painter->setPen(m_internalPathsPenStyle.at(i));
            painter->drawPath(m_internalPaths.at(i));
        }
        painter->setPen(penStyleTmp);
    }

    // paint the passmarks
    if(!m_passmarks.isEmpty())
    {
        painter->drawPath(m_passmarks);
    }

    // paint the place labels (buttons etc)
    if(!m_placeLabels.isEmpty())
    {
        for(auto &placeLabel : m_placeLabels)
        {
            painter->drawPath(placeLabel);
        }
    }


    // TODO Detail & Piece Label

//    QPointF position = m_piece->GetPatternTextPosition();
//    QStringList texts = m_piece->GetPatternText();

//    painter->drawText();



    // when using m_piece->GetItem(), the results were quite bad

}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    bool selectionState = isSelected();
    //perform the default behaviour
    QGraphicsItem::mousePressEvent(event);

    // change the cursor when clicking the left button
    if((event->button() == Qt::LeftButton))
    {
        if((event->modifiers() & Qt::AltModifier) != 0U)
        {
            setCursor(m_rotateCursor);
        }
        else
        {
            setCursor(Qt::ClosedHandCursor);
        }
    }


    // change the selected state when clicking left button
    if (event->button() == Qt::LeftButton)
    {
        setSelected(true);

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
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    if((event->buttons() == Qt::LeftButton) && (event->modifiers() & Qt::AltModifier))
    {
        //FIXME: it flickers between the arrow cursor and the rotate cursor
        setCursor(m_rotateCursor);

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
void VPGraphicsPiece::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
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
void VPGraphicsPiece::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if(event->modifiers() & Qt::AltModifier)
    {
        //FIXME: it flickers between the arrow cursor and the rotate cursor
        setCursor(m_rotateCursor);
    }
    else
    {
        setCursor(Qt::OpenHandCursor);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{

    // TODO/FIXME   context menu needs to be refactored

//    QMenu menu;

//    // move to piece list actions  -- TODO : To be tested properly when we have several piece lists
//    QList<VPPieceList*> pieceLists =  QList<VPPieceList*>();
//    for(auto sheet : m_piece->GetPieceList()->GetLayout()->GetSheets())
//    {
//        pieceLists.append(sheet->GetPieceList());
//    }

//    pieceLists.removeAll(m_piece->GetPieceList());

//    if(pieceLists.count() > 0)
//    {
//        QMenu *moveMenu = menu.addMenu(tr("Move to"));

//        // TODO order in alphabetical order

//        for (auto pieceList : pieceLists)
//        {
//            QAction* moveToPieceList = moveMenu->addAction(pieceList->GetName());
//            QVariant data = QVariant::fromValue(pieceList);
//            moveToPieceList->setData(data);

//            connect(moveToPieceList, &QAction::triggered, this, &VPGraphicsPiece::on_ActionPieceMovedToPieceList);
//        }
//    }

//    // remove from layout action
//    QAction *removeAction = menu.addAction(tr("Remove from Sheet"));
//    QVariant data = QVariant::fromValue(m_piece->GetPieceList()->GetLayout()->GetUnplacedPieceList());
//    removeAction->setData(data);
//    connect(removeAction, &QAction::triggered, this, &VPGraphicsPiece::on_ActionPieceMovedToPieceList);

//    menu.exec(event->screenPos());
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::on_PieceSelectionChanged()
{
    setSelected(m_piece->GetIsSelected());
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::on_PiecePositionChanged()
{
    setPos(m_piece->GetPosition());
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::on_PieceRotationChanged()
{
    setTransformOriginPoint(boundingRect().center());
    setRotation(-m_piece->GetRotation());
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::on_PiecePropertiesChanged()
{
    if(scene() != nullptr)
    {
        scene()->update();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPiece::itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant
{
    if (scene() != nullptr)
    {

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

