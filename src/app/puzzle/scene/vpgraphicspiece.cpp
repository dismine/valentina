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

#include <QApplication>
#include <QBrush>
#include <QGraphicsScene>
#include <QGraphicsSceneContextMenuEvent>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainter>
#include <QPen>
#include <QStyleOptionGraphicsItem>
#include <QtMath>

#include "../layout/vplayout.h"
#include "../layout/vppiece.h"
#include "../layout/vpsheet.h"
#include "../vlayout/vgraphicsfillitem.h"
#include "../vlayout/vtextmanager.h"

#include "../vpapplication.h"

#include "../vlayout/vlayoutpiecepath.h"
#include "compatibility.h"

#include "../vgeometry/vlayoutplacelabel.h"

#include "undocommands/vpundomovepieceonsheet.h"
#include "undocommands/vpundopiecemove.h"
#include "vpiecegrainline.h"

#include <QLoggingCategory>

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(pGraphicsPiece, "p.graphicsPiece") // NOLINT

QT_WARNING_POP

namespace
{
Q_GLOBAL_STATIC_WITH_ARGS(QColor, mainColor, (Qt::black)) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(QColor, errorColor, (Qt::red))  // NOLINT

//---------------------------------------------------------------------------------------------------------------------
inline auto LineMatrix(const VPPiecePtr &piece, const QPointF &topLeft, qreal angle, const QPointF &linePos,
                       int maxLineWidth) -> QTransform
{
    if (piece.isNull())
    {
        return {};
    }

    QTransform labelMatrix;
    labelMatrix.translate(topLeft.x(), topLeft.y());

    if (piece->IsMirror())
    {
        labelMatrix.scale(-1, 1);
        labelMatrix.rotate(-angle);
        labelMatrix.translate(-maxLineWidth, 0);
    }
    else
    {
        labelMatrix.rotate(angle);
    }

    labelMatrix.translate(linePos.x(), linePos.y()); // Each string has own position
    labelMatrix *= piece->GetMatrix();

    return labelMatrix;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto LineFont(const TextLine &tl, const QFont &base) -> QFont
{
    QFont fnt = base;
    fnt.setPixelSize(base.pixelSize() + tl.m_iFontSize);
    fnt.setBold(tl.m_bold);
    fnt.setItalic(tl.m_italic);
    return fnt;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto LineText(const TextLine &tl, const QFontMetrics &fm, qreal width) -> QString
{
    QString qsText = tl.m_qsText;
    if (TextWidth(fm, qsText) > width)
    {
        qsText = fm.elidedText(qsText, Qt::ElideMiddle, static_cast<int>(width));
    }

    return qsText;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto LineAlign(const TextLine &tl, const QString &text, const QFontMetrics &fm, qreal width) -> qreal
{
    const int lineWidth = TextWidth(fm, text);

    qreal dX = 0;
    if (tl.m_eAlign == 0 || (tl.m_eAlign & Qt::AlignLeft) > 0)
    {
        dX = 0;
    }
    else if ((tl.m_eAlign & Qt::AlignHCenter) > 0)
    {
        dX = (width - lineWidth) / 2;
    }
    else if ((tl.m_eAlign & Qt::AlignRight) > 0)
    {
        dX = width - lineWidth;
    }

    return dX;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto SelectionBrush() -> QBrush
{
    return {QColor(255, 160, 160, 60)};
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VPGraphicsPiece::VPGraphicsPiece(const VPPiecePtr &piece, QGraphicsItem *parent)
  : QGraphicsObject(parent),
    m_piece(piece)
{
    // set some infos
    setFlags(ItemIsSelectable | ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    setCursor(Qt::OpenHandCursor);

    if (not piece.isNull())
    {
        setZValue(piece->ZValue());
    }

    PaintPiece();
    InitLabels();
    InitGrainlineItem();
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
    shape.addPath(m_internalPaths);
    shape.addPath(m_passmarks);
    shape.addPath(m_placeLabels);
    shape.addPath(m_stickyPath);

    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();
    const qreal halfPenWidth = settings->GetLayoutLineWidth() / 2.;

    return shape.boundingRect().adjusted(-halfPenWidth, -halfPenWidth, halfPenWidth, halfPenWidth);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPiece::shape() const -> QPainterPath
{
    if (!m_cuttingLine.isEmpty())
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

    VPSettings *settings = VPApplication::VApp()->PuzzleSettings();
    QPen pen(PieceColor(), settings->GetLayoutLineWidth(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);

    PaintPiece(painter);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // perform the default behaviour
    QGraphicsObject::mousePressEvent(event);

    // change the cursor when clicking the left button
    if (event->button() == Qt::LeftButton)
    {
        setCursor(Qt::ClosedHandCursor);

        m_moveStartPoint = event->pos();
        emit HideTransformationHandles(true);
        m_hasStickyPosition = false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsObject::mouseMoveEvent(event);

    GroupMove(event->pos());

    m_moveStartPoint = event->pos();
    m_allowChangeMerge = true;
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    // perform the default behaviour
    QGraphicsItem::mouseReleaseEvent(event);

    // change the cursor when clicking left button
    if (event->button() == Qt::LeftButton)
    {
        setCursor(Qt::OpenHandCursor);
        emit HideTransformationHandles(false);

        VPPiecePtr piece = m_piece.toStrongRef();
        if (not piece.isNull())
        {
            VPLayoutPtr layout = piece->Layout();
            if (not layout.isNull())
            {
                if (layout->LayoutSettings().GetStickyEdges() && m_hasStickyPosition)
                {
                    auto *command =
                        new VPUndoPieceMove(piece, m_stickyTranslateX, m_stickyTranslateY, m_allowChangeMerge);
                    layout->UndoStack()->push(command);

                    SetStickyPoints(QVector<QPointF>());
                }
            }
        }

        m_allowChangeMerge = false;
        m_hasStickyPosition = false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_hoverMode = true;
    QGraphicsObject::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_hoverMode = false;
    QGraphicsObject::hoverLeaveEvent(event);
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

    QVector<QAction *> moveToActions;

    if (not sheets.isEmpty())
    {
        QMenu *moveMenu = menu.addMenu(tr("Move to"));

        for (const auto &sheet : sheets)
        {
            if (not sheet.isNull())
            {
                QAction *moveToSheet = moveMenu->addAction(sheet->GetName());
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
void VPGraphicsPiece::SetTextAsPaths(bool newTextAsPaths)
{
    m_textAsPaths = newTextAsPaths;
    InitLabels();
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::InitLabels()
{
    qDeleteAll(m_labelPathItems);
    qDeleteAll(m_labelTextItems);

    m_labelPathItems.clear();
    m_labelTextItems.clear();

    VPPiecePtr piece = m_piece.toStrongRef();
    if (piece.isNull())
    {
        return;
    }

    InitPieceLabel(piece->GetPieceLabelRect(), piece->GetPieceLabelData());
    InitPieceLabel(piece->GetPatternLabelRect(), piece->GetPatternLabelData());
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::SetStickyPoints(const QVector<QPointF> &newStickyPoint)
{
    m_stickyPoints = newStickyPoint;

    prepareGeometryChange();
    PaintPiece(); // refresh shapes
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::InitPieceLabel(const QVector<QPointF> &labelShape, const VTextManager &tm)
{
    VPPiecePtr piece = m_piece.toStrongRef();
    if (piece.isNull())
    {
        return;
    }

    if (labelShape.count() <= 2)
    {
        return;
    }

    const qreal dW = QLineF(labelShape.at(0), labelShape.at(1)).length();
    const qreal dH = QLineF(labelShape.at(1), labelShape.at(2)).length();
    const qreal angle = -QLineF(labelShape.at(0), labelShape.at(1)).angle();
    const QColor color = PieceColor();
    const int maxLineWidth = tm.MaxLineWidth(static_cast<int>(dW));

    qreal dY = 0;

    for (int i = 0; i < tm.GetSourceLinesCount(); ++i)
    {
        const TextLine& tl = tm.GetSourceLine(i);
        const QFont fnt = LineFont(tl, tm.GetFont());
        const QFontMetrics fm(fnt);

        if (m_textAsPaths)
        {
            dY += fm.height();
        }

        if (dY > dH)
        {
            break;
        }

        const QString qsText = LineText(tl, fm, dW);
        const qreal dX = LineAlign(tl, qsText, fm, dW);
        // set up the rotation around top-left corner matrix
        const QTransform lineMatrix = LineMatrix(piece, labelShape.at(0), angle, QPointF(dX, dY), maxLineWidth);

        if (m_textAsPaths)
        {
            QPainterPath path;
            path.addText(0, -static_cast<qreal>(fm.ascent()) / 6., fnt, qsText);

            auto *item = new QGraphicsPathItem(this);
            item->setPath(path);
            item->setBrush(QBrush(color));
            item->setTransform(lineMatrix);
            m_labelPathItems.append(item);

            dY += tm.GetSpacing();
        }
        else
        {
            auto *item = new QGraphicsSimpleTextItem(this);
            item->setFont(fnt);
            item->setText(qsText);
            item->setBrush(QBrush(color));
            item->setTransform(lineMatrix);
            m_labelTextItems.append(item);

            dY += (fm.height() + tm.GetSpacing());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::InitGrainlineItem()
{
    delete m_grainlineItem;

    VPPiecePtr piece = m_piece.toStrongRef();
    if (piece.isNull())
    {
        return;
    }

    if (piece->IsGrainlineEnabled())
    {
        m_grainlineItem = new VGraphicsFillItem(this);
        m_grainlineItem->setPath(VLayoutPiece::GrainlinePath(piece->GetMappedGrainlineShape()));

        VPSettings *settings = VPApplication::VApp()->PuzzleSettings();
        QPen pen(PieceColor(), settings->GetLayoutLineWidth(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        m_grainlineItem->SetCustomPen(true);
        m_grainlineItem->setPen(pen);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::PaintPiece(QPainter *painter)
{
    m_seamLine = QPainterPath();
    m_cuttingLine = QPainterPath();
    m_internalPaths = QPainterPath();
    m_passmarks = QPainterPath();
    m_placeLabels = QPainterPath();
    m_stickyPath = QPainterPath();

    VPPiecePtr piece = m_piece.toStrongRef();
    if (piece.isNull())
    {
        return;
    }

    // initialises the seam line
    PaintSeamLine(painter, piece);

    // initiliases the cutting line
    PaintCuttingLine(painter, piece);

    // initialises the internal paths
    PaintInternalPaths(painter, piece);

    // initialises the passmarks
    PaintPassmarks(painter, piece);

    // initialises the place labels (buttons etc)
    PaintPlaceLabels(painter, piece);

    PaintStickyPath(painter);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::PaintSeamLine(QPainter *painter, const VPPiecePtr &piece)
{
    if (not piece->IsHideMainPath() || not piece->IsSeamAllowance())
    {
        QVector<VLayoutPoint> seamLinePoints = piece->GetMappedContourPoints();
        if (!seamLinePoints.isEmpty())
        {
            m_seamLine.moveTo(ConstFirst(seamLinePoints));
            for (int i = 1; i < seamLinePoints.size(); i++)
            {
                m_seamLine.lineTo(seamLinePoints.at(i));
            }

            if (painter != nullptr)
            {
                painter->save();
                painter->setBrush(piece->IsSelected() ? SelectionBrush() : NoBrush());
                painter->drawPath(m_seamLine);
                painter->restore();
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::PaintCuttingLine(QPainter *painter, const VPPiecePtr &piece)
{
    if (piece->IsSeamAllowance() && not piece->IsSeamAllowanceBuiltIn())
    {
        QVector<VLayoutPoint> cuttingLinepoints = piece->GetMappedSeamAllowancePoints();
        if (!cuttingLinepoints.isEmpty())
        {
            m_cuttingLine.moveTo(ConstFirst(cuttingLinepoints));
            for (int i = 1; i < cuttingLinepoints.size(); i++)
            {
                m_cuttingLine.lineTo(cuttingLinepoints.at(i));
            }

            if (painter != nullptr)
            {
                painter->save();
                painter->setBrush(piece->IsSelected() ? SelectionBrush() : NoBrush());
                painter->drawPath(m_cuttingLine);
                painter->restore();
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::PaintInternalPaths(QPainter *painter, const VPPiecePtr &piece)
{
    QVector<VLayoutPiecePath> internalPaths = piece->GetInternalPaths();
    for (const auto &piecePath : internalPaths)
    {
        QPainterPath path = piece->GetMatrix().map(piecePath.GetPainterPath());

        if (painter != nullptr)
        {
            painter->save();
            QPen pen = painter->pen();
            pen.setStyle(piecePath.PenStyle());
            painter->setPen(pen);
            painter->drawPath(path);
            painter->restore();
        }
        m_internalPaths.addPath(path);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::PaintPassmarks(QPainter *painter, const VPPiecePtr &piece)
{
    QVector<VLayoutPassmark> passmarks = piece->GetMappedPassmarks();
    for (auto &passmark : passmarks)
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
            painter->setBrush(NoBrush());
            painter->drawPath(passmarkPath);
            painter->restore();
        }

        m_passmarks.addPath(passmarkPath);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::PaintPlaceLabels(QPainter *painter, const VPPiecePtr &piece)
{
    QVector<VLayoutPlaceLabel> placeLabels = piece->GetPlaceLabels();
    for (auto &placeLabel : placeLabels)
    {
        QPainterPath path =
            VAbstractPiece::LabelShapePath(piece->MapPlaceLabelShape(VAbstractPiece::PlaceLabelShape(placeLabel)));

        if (painter != nullptr)
        {
            painter->save();
            painter->setBrush(NoBrush());
            painter->drawPath(path);
            painter->restore();
        }

        m_placeLabels.addPath(path);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::PaintStickyPath(QPainter *painter)
{
    if (not m_stickyPoints.isEmpty())
    {
        m_stickyPath.moveTo(ConstFirst(m_stickyPoints));
        for (int i = 1; i < m_stickyPoints.size(); i++)
        {
            m_stickyPath.lineTo(m_stickyPoints.at(i));
        }

        if (painter != nullptr)
        {
            painter->save();
            painter->setBrush(QBrush(Qt::BDiagPattern));

            QPen pen = painter->pen();
            pen.setStyle(Qt::DashLine);
            pen.setColor(*mainColor);
            painter->setPen(pen);

            painter->drawPath(m_stickyPath);
            painter->restore();
        }
    }
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
        const VPPiecePtr &p = ConstFirst(pieces);
        auto *command = new VPUndoPieceMove(piece, newPos.x(), newPos.y(), m_allowChangeMerge);
        layout->UndoStack()->push(command);

        if (layout->LayoutSettings().GetStickyEdges())
        {
            QVector<QPointF> path;
            if (not p.isNull() && p->StickyPosition(m_stickyTranslateX, m_stickyTranslateY))
            {
                CastTo(p->GetMappedExternalContourPoints(), path);
                QTransform m;
                m.translate(m_stickyTranslateX, m_stickyTranslateY);
                path = m.map(path);
                m_hasStickyPosition = true;
            }
            else
            {
                m_hasStickyPosition = false;
            }

            SetStickyPoints(path);
        }
    }
    else if (pieces.size() > 1)
    {
        auto *command = new VPUndoPiecesMove(pieces, newPos.x(), newPos.y(), m_allowChangeMerge);
        layout->UndoStack()->push(command);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPiece::PieceColor() const -> QColor
{
    VPPiecePtr piece = m_piece.toStrongRef();
    if (piece.isNull())
    {
        return *mainColor;
    }

    VPLayoutPtr layout = piece->Layout();
    if (layout.isNull())
    {
        return *mainColor;
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
        return *errorColor;
    }

    return *mainColor;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPiece::NoBrush() const -> QBrush
{
    return m_hoverMode ? QBrush(QColor(199, 244, 249, 60)) : QBrush(Qt::NoBrush);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::on_RefreshPiece(const VPPiecePtr &piece)
{
    VPPiecePtr p = m_piece.toStrongRef();
    if (p.isNull())
    {
        return;
    }

    if (p->GetUniqueID() == piece->GetUniqueID())
    {
        if (not piece.isNull())
        {
            setZValue(piece->ZValue());
        }

        prepareGeometryChange();
        PaintPiece(); // refresh shapes
        InitLabels();
        InitGrainlineItem();
        emit PieceTransformationChanged();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::PieceZValueChanged(const VPPiecePtr &piece)
{
    VPPiecePtr p = m_piece.toStrongRef();
    if (p.isNull() || piece.isNull())
    {
        return;
    }

    if (p->GetUniqueID() == piece->GetUniqueID())
    {
        setZValue(piece->ZValue());
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPiece::itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant
{
    if (scene() != nullptr)
    {
        if (change == ItemSelectedHasChanged)
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
