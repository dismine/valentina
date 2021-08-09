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

#include "../layout/vppiece.h"
#include "../layout/vplayout.h"
#include "../layout/vpsheet.h"

#include "vlayoutpiecepath.h"
#include "vplacelabelitem.h"

#include <QLoggingCategory>
Q_LOGGING_CATEGORY(pGraphicsPiece, "p.graphicsPiece")

namespace
{
constexpr qreal penWidth = 1;
}

//---------------------------------------------------------------------------------------------------------------------
VPGraphicsPiece::VPGraphicsPiece(VPPiece *piece, QGraphicsItem *parent) :
    QGraphicsObject(parent),
    m_piece(piece)
{
    QPixmap cursor_pixmap = QIcon("://puzzleicon/svg/cursor_rotate.svg").pixmap(QSize(32,32));
    m_rotateCursor= QCursor(cursor_pixmap, 16, 16);

    // set some infos
    setFlags(ItemIsSelectable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    setCursor(Qt::OpenHandCursor);

    PaintPiece();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPiece::GetPiece() -> VPPiece*
{
    return m_piece;
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::TranslatePiece(qreal dx, qreal dy)
{
    TranslatePiece(QPointF(dx, dy));
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::TranslatePiece(const QPointF &p)
{
    prepareGeometryChange();
    m_piece->Translate(p);
    PaintPiece(); // refresh shapes
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPiece::boundingRect() const -> QRectF
{
    constexpr qreal halfPenWidth = penWidth/2.;
    if(!m_cuttingLine.isEmpty())
    {
        return m_cuttingLine.boundingRect().adjusted(-halfPenWidth, -halfPenWidth, halfPenWidth, halfPenWidth);
    }

    return m_seamLine.boundingRect().adjusted(-halfPenWidth, -halfPenWidth, halfPenWidth, halfPenWidth);
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

    QPen pen(Qt::black, penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);

    PaintPiece(painter);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //perform the default behaviour
    QGraphicsObject::mousePressEvent(event);

    // change the cursor when clicking the left button
    if((event->button() == Qt::LeftButton))
    {
        setCursor(Qt::ClosedHandCursor);

        m_moveStartPoint = event->pos();
        emit HideTransformationHandles(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsObject::mouseMoveEvent(event);

    GroupMove(event->pos());

    m_moveStartPoint = event->pos();
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    //perform the default behaviour
    QGraphicsItem::mouseReleaseEvent(event);

    // change the cursor when clicking left button
    if (event->button() == Qt::LeftButton)
    {
        setCursor(Qt::OpenHandCursor);
        GroupMove(event->pos());
        emit HideTransformationHandles(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    QMenu menu;

    QList<VPSheet *> sheets = m_piece->Layout()->GetSheets();
    sheets.removeAll(m_piece->Sheet());

    QVector<QAction*> moveToActions;

    if (not sheets.isEmpty())
    {
        QMenu *moveMenu = menu.addMenu(tr("Move to"));

        for (auto *sheet : sheets)
        {
            QAction* moveToSheet = moveMenu->addAction(sheet->GetName());
            moveToSheet->setData(QVariant::fromValue(sheet));
            moveToActions.append(moveToSheet);
        }
    }

    // remove from layout action
    QAction *removeAction = menu.addAction(tr("Remove from Sheet"));

    QAction *selectedAction = menu.exec(event->screenPos());

    if (moveToActions.contains(selectedAction))
    {
        m_piece->SetSheet(qvariant_cast<VPSheet *>(selectedAction->data()));
        emit m_piece->Layout()->PieceSheetChanged(m_piece);
    }
    else if (selectedAction == removeAction)
    {
        m_piece->SetSheet(nullptr);
        emit m_piece->Layout()->PieceSheetChanged(m_piece);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::PaintPiece(QPainter *painter)
{
    QBrush noBrush(Qt::NoBrush);
    QBrush selectionBrush(QColor(255,160,160,60));

    QRectF rect = m_piece->MappedDetailBoundingRect();
    QPointF p = rect.topLeft();

    // initialises the seam line
    QVector<QPointF> seamLinePoints = m_piece->GetMappedContourPoints();
    if(!seamLinePoints.isEmpty())
    {
        m_seamLine = QPainterPath();
        m_seamLine.moveTo(seamLinePoints.first());
        for (int i = 1; i < seamLinePoints.size(); i++)
        {
            m_seamLine.lineTo(seamLinePoints.at(i));
        }

        if (painter != nullptr)
        {
            painter->save();
            painter->setBrush(isSelected() ? selectionBrush : noBrush);
            painter->drawPath(m_seamLine);
            painter->restore();
        }
    }

    // initiliases the cutting line
    QVector<QPointF> cuttingLinepoints = m_piece->GetMappedSeamAllowancePoints();
    if(!cuttingLinepoints.isEmpty())
    {
        m_cuttingLine = QPainterPath();
        m_cuttingLine.moveTo(cuttingLinepoints.first());
        for (int i = 1; i < cuttingLinepoints.size(); i++)
        {
            m_cuttingLine.lineTo(cuttingLinepoints.at(i));
        }

        if (painter != nullptr)
        {
            painter->save();
            painter->setBrush(isSelected() ? selectionBrush : noBrush);
            painter->drawPath(m_cuttingLine);
            painter->restore();
        }
    }

    // initialises the grainline
    if(m_piece->IsGrainlineEnabled())
    {
        QVector<QPointF> grainLinepoints = m_piece->GetMappedGrainline();
        if(!grainLinepoints.isEmpty())
        {
            QPainterPath grainline;
            grainline.moveTo(grainLinepoints.first());
            for (int i = 1; i < grainLinepoints.size(); i++)
            {
                grainline.lineTo(grainLinepoints.at(i));
            }

            if (painter != nullptr)
            {
                painter->save();
                // here to fill the grainlines arrow. Not wanted for mvp
                // later maybe if it's configurable
//                painter->setBrush(blackBrush);

                painter->setBrush(noBrush);
                painter->drawPath(grainline);
                painter->restore();
            }
        }
    }

    // initialises the internal paths
    QVector<VLayoutPiecePath> internalPaths = m_piece->GetInternalPaths();
    for (const auto& piecePath : internalPaths)
    {
        QPainterPath path = m_piece->GetMatrix().map(piecePath.GetPainterPath());

        if (painter != nullptr)
        {
            painter->save();
            painter->setPen(piecePath.PenStyle());
            painter->drawPath(path);
            painter->restore();
        }
    }

    // initialises the passmarks
    QVector<VLayoutPassmark> passmarks = m_piece->GetMappedPassmarks();
    for(auto &passmark : passmarks)
    {
        QPainterPath passmarkPath;
        for (auto &line : passmark.lines)
        {
            passmarkPath.moveTo(line.p1());
            passmarkPath.lineTo(line.p2());
        }

        if (painter != nullptr)
        {
            painter->save();
            painter->setBrush(noBrush);
            painter->drawPath(passmarkPath);
            painter->restore();
        }
    }

    // initialises the place labels (buttons etc)
    QVector<VLayoutPlaceLabel> placeLabels = m_piece->GetPlaceLabels();
    for(auto &placeLabel : placeLabels)
    {
        QPainterPath path = VPlaceLabelItem::LabelShapePath(placeLabel.shape);

        if (painter != nullptr)
        {
            painter->save();
            painter->setBrush(noBrush);
            painter->drawPath(path);
            painter->restore();
        }
    }

    // TODO : initialises the text labels

//    QPointF position = m_piece->GetPatternTextPosition();
//    QStringList texts = m_piece->GetPatternText();

    //    painter->drawText();
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::GroupMove(const QPointF &pos)
{
    if (scene() != nullptr)
    {
        QList<QGraphicsItem *> list = scene()->selectedItems();
        for (auto *item : list)
        {
            if (item->type() == UserType + static_cast<int>(PGraphicsItem::Piece))
            {
                auto *pieceItem = dynamic_cast<VPGraphicsPiece*>(item);
                pieceItem->TranslatePiece(pos-m_moveStartPoint);
            }
        }
        emit PiecePositionChanged();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::on_Rotate(const QPointF &center, qreal angle)
{
    if (isSelected())
    {
        prepareGeometryChange();
        m_piece->Rotate(center, angle);
        PaintPiece(); // Update shapes
        update();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPiece::itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant
{
    if (scene() != nullptr)
    {
        if(change == ItemSelectedHasChanged)
        {
            emit PieceSelectionChanged();
            m_piece->SetSelected(value.toBool());
        }
    }

    return QGraphicsObject::itemChange(change, value);
}

