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

#include "undocommands/vpundopiecemove.h"
#include "undocommands/vpundomovepieceonsheet.h"

#include <QLoggingCategory>
Q_LOGGING_CATEGORY(pGraphicsPiece, "p.graphicsPiece")

namespace
{
constexpr qreal penWidth = 1;

QColor mainColor = Qt::black;
QColor errorColor = Qt::red;
}

//---------------------------------------------------------------------------------------------------------------------
VPGraphicsPiece::VPGraphicsPiece(const VPPiecePtr &piece, QGraphicsItem *parent) :
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
auto VPGraphicsPiece::GetPiece() -> VPPiecePtr
{
    return m_piece.toStrongRef();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPiece::boundingRect() const -> QRectF
{
    QPainterPath shape;
    shape.addPath(m_seamLine);
    shape.addPath(m_cuttingLine);
    shape.addPath(m_grainline);
    shape.addPath(m_internalPaths);
    shape.addPath(m_passmarks);
    shape.addPath(m_placeLabels);

    constexpr qreal halfPenWidth = penWidth/2.;

    return shape.boundingRect().adjusted(-halfPenWidth, -halfPenWidth, halfPenWidth, halfPenWidth);
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

    QPen pen(PieceColor(), penWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
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
    allowChangeMerge = true;
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
        emit HideTransformationHandles(false);
        allowChangeMerge = false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    VPPiecePtr piece = m_piece.toStrongRef();
    if (piece.isNull())
    {
        return;
    }

    VPLayoutPtr layout = piece->Layout();
    if (layout.isNull())
    {
        return;
    }

    QMenu menu;
    QList<VPSheetPtr> sheets = layout->GetSheets();
    sheets.removeAll(piece->Sheet());

    QVector<QAction*> moveToActions;

    if (not sheets.isEmpty())
    {
        QMenu *moveMenu = menu.addMenu(tr("Move to"));

        for (const auto &sheet : sheets)
        {
            if (not sheet.isNull())
            {
                QAction* moveToSheet = moveMenu->addAction(sheet->GetName());
                moveToSheet->setData(QVariant::fromValue(sheet));
                moveToActions.append(moveToSheet);
            }
        }
    }

    // remove from layout action
    QAction *removeAction = menu.addAction(tr("Remove from Sheet"));

    QAction *selectedAction = menu.exec(event->screenPos());

    if (moveToActions.contains(selectedAction))
    {
        auto *command = new VPUndoMovePieceOnSheet(qvariant_cast<VPSheetPtr>(selectedAction->data()), piece);
        layout->UndoStack()->push(command);
    }
    else if (selectedAction == removeAction)
    {
        auto *command = new VPUndoMovePieceOnSheet(VPSheetPtr(), piece);
        layout->UndoStack()->push(command);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::PaintPiece(QPainter *painter)
{
    QBrush noBrush(Qt::NoBrush);
    QBrush selectionBrush(QColor(255,160,160,60));

    m_seamLine = QPainterPath();
    m_cuttingLine = QPainterPath();
    m_grainline = QPainterPath();
    m_internalPaths = QPainterPath();
    m_passmarks = QPainterPath();
    m_placeLabels = QPainterPath();

    VPPiecePtr piece = m_piece.toStrongRef();
    if (piece.isNull())
    {
        return;
    }

    // initialises the seam line
    QVector<QPointF> seamLinePoints = piece->GetMappedContourPoints();
    if(!seamLinePoints.isEmpty())
    {
        m_seamLine.moveTo(seamLinePoints.first());
        for (int i = 1; i < seamLinePoints.size(); i++)
        {
            m_seamLine.lineTo(seamLinePoints.at(i));
        }

        if (painter != nullptr)
        {
            painter->save();
            painter->setBrush(piece->IsSelected() ? selectionBrush : noBrush);
            painter->drawPath(m_seamLine);
            painter->restore();
        }
    }

    // initiliases the cutting line
    QVector<QPointF> cuttingLinepoints = piece->GetMappedSeamAllowancePoints();
    if(!cuttingLinepoints.isEmpty())
    {
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
    if(piece->IsGrainlineEnabled())
    {
        QVector<QPointF> grainLinepoints = piece->GetMappedGrainline();
        if(!grainLinepoints.isEmpty())
        {
            m_grainline.moveTo(grainLinepoints.first());
            for (int i = 1; i < grainLinepoints.size(); i++)
            {
                m_grainline.lineTo(grainLinepoints.at(i));
            }

            if (painter != nullptr)
            {
                painter->save();
                // here to fill the grainlines arrow. Not wanted for mvp
                // later maybe if it's configurable
//                painter->setBrush(blackBrush);

                painter->setBrush(noBrush);
                painter->drawPath(m_grainline);
                painter->restore();
            }
        }
    }

    // initialises the internal paths
    QVector<VLayoutPiecePath> internalPaths = piece->GetInternalPaths();
    for (const auto& piecePath : internalPaths)
    {
        QPainterPath path = piece->GetMatrix().map(piecePath.GetPainterPath());

        if (painter != nullptr)
        {
            painter->save();
            painter->setPen(piecePath.PenStyle());
            painter->drawPath(path);
            painter->restore();
        }
        m_internalPaths.addPath(path);
    }

    // initialises the passmarks
    QVector<VLayoutPassmark> passmarks = piece->GetMappedPassmarks();
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

        m_passmarks.addPath(passmarkPath);
    }

    // initialises the place labels (buttons etc)
    QVector<VLayoutPlaceLabel> placeLabels = piece->GetMappedPlaceLabels();
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

        m_placeLabels.addPath(path);
    }

    // TODO : initialises the text labels

//    QPointF position = m_piece->GetPatternTextPosition();
//    QStringList texts = m_piece->GetPatternText();

    //    painter->drawText();
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::GroupMove(const QPointF &pos)
{
    VPPiecePtr piece = m_piece.toStrongRef();
    if (piece.isNull())
    {
        return;
    }

    VPLayoutPtr layout = piece->Layout();
    if (layout.isNull())
    {
        return;
    }

    auto PreparePieces = [layout]()
    {
        QList<VPPiecePtr> pieces;

        VPSheetPtr sheet = layout->GetFocusedSheet();
        if (not sheet.isNull())
        {
            return sheet->GetSelectedPieces();
        }

        return pieces;
    };

    QList<VPPiecePtr> pieces = PreparePieces();
    QPointF newPos = pos - m_moveStartPoint;

    if (qFuzzyIsNull(newPos.x()) && qFuzzyIsNull(newPos.y()))
    {
        return;
    }

    if (pieces.size() == 1)
    {
        auto *command = new VPUndoPieceMove(pieces.first(), newPos.x(), newPos.y(), allowChangeMerge);
        layout->UndoStack()->push(command);
    }
    else if (pieces.size() > 1)
    {
        auto *command = new VPUndoPiecesMove(pieces, newPos.x(), newPos.y(), allowChangeMerge);
        layout->UndoStack()->push(command);
    }
}

//---------------------------------------------------------------------------------------------------------------------
QColor VPGraphicsPiece::PieceColor() const
{
    VPPiecePtr piece = m_piece.toStrongRef();
    if (piece.isNull())
    {
        return mainColor;
    }

    VPLayoutPtr layout = piece->Layout();
    if (layout.isNull())
    {
        return mainColor;
    }

    bool outOfBound = false;
    if (layout->LayoutSettings().GetWarningPiecesOutOfBound())
    {
        outOfBound = piece->OutOfBound();
    }

    bool superposition = false;
    if (layout->LayoutSettings().GetWarningSuperpositionOfPieces())
    {
        superposition = piece->HasSuperpositionWithPieces();
    }

    if (outOfBound || superposition)
    {
        return errorColor;
    }

    return mainColor;
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::on_RefreshPiece(const VPPiecePtr &piece)
{
    if (m_piece == piece)
    {
        prepareGeometryChange();
        PaintPiece(); // refresh shapes
        emit PieceTransformationChanged();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPiece::itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant
{
    if (scene() != nullptr)
    {
        if(change == ItemSelectedHasChanged)
        {
            VPPiecePtr piece = m_piece.toStrongRef();
            if (not piece.isNull())
            {
                piece->SetSelected(value.toBool());

                VPLayoutPtr layout = piece->Layout();
                if (not layout.isNull())
                {
                    emit layout->PieceSelectionChanged(piece);
                }
            }
        }
    }

    return QGraphicsObject::itemChange(change, value);
}

