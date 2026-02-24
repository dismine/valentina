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
#include "../vformat/vsinglelineoutlinechar.h"
#include "../vgeometry/vlayoutplacelabel.h"
#include "../vlayout/vfoldline.h"
#include "../vlayout/vgraphicsfillitem.h"
#include "../vlayout/vlayoutpiecepath.h"
#include "../vlayout/vtextmanager.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/svgfont/vsvgfont.h"
#include "../vmisc/svgfont/vsvgfontdatabase.h"
#include "../vmisc/svgfont/vsvgfontengine.h"
#include "../vmisc/theme/vscenestylesheet.h"
#include "../vpapplication.h"
#include "../vpatterndb/vpiecepath.h"
#include "undocommands/vpundomovepieceonsheet.h"
#include "undocommands/vpundopiecemove.h"
#include "vpiecegrainline.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
#include "../vmisc/backport/qpainterstateguard.h"
#else
#include <QPainterStateGuard>
#endif

#include <QLoggingCategory>

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(pGraphicsPiece, "p.graphicsPiece") // NOLINT

QT_WARNING_POP

namespace
{
//---------------------------------------------------------------------------------------------------------------------
inline auto LineFont(const TextLine &tl, const QFont &base) -> QFont
{
    QFont fnt = base;
    fnt.setPointSize(qMax(base.pointSize() + tl.iFontSize, 1));
    if (!VAbstractApplication::VApp()->Settings()->GetSingleStrokeOutlineFont())
    {
        fnt.setBold(tl.bold);
    }
    fnt.setItalic(tl.italic);
    return fnt;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto LineFont(const TextLine &tl, const VSvgFont &base) -> VSvgFont
{
    VSvgFont fnt = base;
    fnt.SetPointSize(base.PointSize() + tl.iFontSize);
    fnt.SetBold(tl.bold);
    fnt.SetItalic(tl.italic);
    return fnt;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto LineAlign(const TextLine &tl, const QString &text, const QFontMetrics &fm, qreal width) -> qreal
{
    const int lineWidth = fm.horizontalAdvance(text);

    qreal dX = 0;
    if (tl.eAlign == 0 || (tl.eAlign & Qt::AlignLeft) > 0)
    {
        dX = 0;
    }
    else if ((tl.eAlign & Qt::AlignHCenter) > 0)
    {
        dX = (width - lineWidth) / 2;
    }
    else if ((tl.eAlign & Qt::AlignRight) > 0)
    {
        dX = width - lineWidth;
    }

    return dX;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto LineAlign(const TextLine &tl, const QString &text, const VSvgFontEngine &engine, qreal width,
                      qreal penWidth) -> qreal
{
    const int lineWidth = qRound(engine.TextWidth(text, penWidth));

    qreal dX = 0;
    if (tl.eAlign == 0 || (tl.eAlign & Qt::AlignLeft) > 0)
    {
        dX = 0;
    }
    else if ((tl.eAlign & Qt::AlignHCenter) > 0)
    {
        dX = (width - lineWidth) / 2;
    }
    else if ((tl.eAlign & Qt::AlignRight) > 0)
    {
        dX = width - lineWidth;
    }

    return dX;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto SelectionBrush() -> QBrush
{
    return {VSceneStylesheet::ManualLayoutStyle().PieceSelectionBrushColor()};
}

//---------------------------------------------------------------------------------------------------------------------
auto ShouldSkipPainting(const VPPiecePtr &piece) -> bool
{
    return (piece->GetFoldLineType() == FoldLineType::None || (piece->IsShowFullPiece() && !piece->IsShowMirrorLine()));
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
auto VPGraphicsPiece::GetPiece() const -> VPPiecePtr
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
    shape.addPath(m_foldLineMarkPath);
    shape.addPath(m_foldLineLabelPath);
    shape.addPath(m_mirrorLinePath);

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
    QPen const pen(PieceColor(), settings->GetLayoutLineWidth(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
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

        if (VPPiecePtr const piece = m_piece.toStrongRef(); not piece.isNull())
        {
            if (VPLayoutPtr const layout = piece->Layout();
                not layout.isNull() && (layout->LayoutSettings().IsStickyEdges() && m_hasStickyPosition))
            {
                auto *command = new VPUndoPieceMove(piece, m_stickyTranslateX, m_stickyTranslateY, m_allowChangeMerge);
                layout->UndoStack()->push(command);

                SetStickyPoints(QVector<QPointF>());
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
    VPPiecePtr const piece = m_piece.toStrongRef();
    if (piece.isNull())
    {
        return;
    }

    VPLayoutPtr const layout = piece->Layout();
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

        for (const auto &sheet : std::as_const(sheets))
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
void VPGraphicsPiece::SetPrintMode(bool newPrintMode)
{
    m_printMode = newPrintMode;
    InitGrainlineItem();
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::InitLabels()
{
    qDeleteAll(m_labelPathItems);
    qDeleteAll(m_labelTextItems);

    m_labelPathItems.clear();
    m_labelTextItems.clear();

    VPPiecePtr const piece = m_piece.toStrongRef();
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
void VPGraphicsPiece::InitPieceLabelSVGFont(const QVector<QPointF> &labelShape, const VTextManager &tm)
{
    VPPiecePtr const piece = m_piece.toStrongRef();
    if (piece.isNull())
    {
        return;
    }

    if (labelShape.count() <= 2)
    {
        return;
    }

    VSvgFontDatabase *db = VAbstractApplication::VApp()->SVGFontDatabase();
    VSvgFontEngine engine =
        db->FontEngine(tm.GetSVGFontFamily(), SVGFontStyle::Normal, SVGFontWeight::Normal, tm.GetSVGFontPointSize());

    VSvgFont const svgFont = engine.Font();
    if (!svgFont.IsValid())
    {
        auto const errorMsg = QStringLiteral("Invalid SVG font '%1'. Fallback to outline font.").arg(svgFont.Name());
        qDebug() << errorMsg;
        InitPieceLabelOutlineFont(labelShape, tm);
        return;
    }

    qreal const penWidth = VPApplication::VApp()->PuzzleSettings()->GetLayoutLineWidth();

    const qreal dW = QLineF(labelShape.at(0), labelShape.at(1)).length();
    const qreal dH = QLineF(labelShape.at(1), labelShape.at(2)).length();
    const qreal angle = -QLineF(labelShape.at(0), labelShape.at(1)).angle();
    const QColor color = PieceColor();

    qreal dY = penWidth;

    const QVector<TextLine> labelLines = tm.GetLabelSourceLines(qFloor(dW), svgFont, penWidth);

    for (const auto &tl : labelLines)
    {
        const VSvgFont fnt = LineFont(tl, svgFont);
        engine = db->FontEngine(fnt);

        if (dY + engine.FontHeight() + penWidth > dH)
        {
            break;
        }

        const QString qsText = tl.qsText;
        const qreal dX = LineAlign(tl, qsText, engine, dW, penWidth);
        // set up the rotation around top-left corner matrix
        const QTransform lineMatrix = piece->LineMatrix(labelShape.at(0), angle, QPointF(dX, dY), dW);

        auto *item = new QGraphicsPathItem(this);
        item->setPath(engine.DrawPath(QPointF(), qsText));

        QPen itemPen = item->pen();
        itemPen.setColor(color);
        itemPen.setCapStyle(Qt::RoundCap);
        itemPen.setJoinStyle(Qt::RoundJoin);
        itemPen.setWidthF(penWidth);
        item->setPen(itemPen);

        item->setBrush(QBrush(Qt::NoBrush));
        item->setTransform(lineMatrix);
        m_labelPathItems.append(item);

        dY += engine.FontHeight() - penWidth * 2 + tm.GetSpacing();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::InitPieceLabelOutlineFont(const QVector<QPointF> &labelShape, const VTextManager &tm)
{
    VPPiecePtr const piece = m_piece.toStrongRef();
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
    qreal const penWidth = VPApplication::VApp()->PuzzleSettings()->GetLayoutLineWidth();

    qreal dY = 0;

    const VCommonSettings *settings = VAbstractApplication::VApp()->Settings();

    bool textAsPaths = m_textAsPaths;
    if (settings->GetSingleStrokeOutlineFont())
    {
        textAsPaths = true;
        dY += penWidth;
    }

    const QVector<TextLine> labelLines = tm.GetLabelSourceLines(qFloor(dW), tm.GetFont());

    for (const auto &tl : labelLines)
    {
        const QFont fnt = LineFont(tl, tm.GetFont());
        QFontMetrics const fm(fnt);

        if (dY + fm.height() > dH)
        {
            break;
        }

        const qreal dX = LineAlign(tl, tl.qsText, fm, dW);
        // set up the rotation around top-left corner matrix
        const QTransform lineMatrix = piece->LineMatrix(labelShape.at(0), angle, QPointF(dX, dY), dW);

        if (textAsPaths)
        {
            QPainterPath path;

            if (settings->GetSingleStrokeOutlineFont())
            {
                VSingleLineOutlineChar const corrector(fnt);
                if (!corrector.IsPopulated())
                {
                    corrector.LoadCorrections(settings->GetPathFontCorrections());
                }

                int w = 0;
                for (auto c : std::as_const(tl.qsText))
                {
                    path.addPath(corrector.DrawChar(w, static_cast<qreal>(fm.ascent()), c));
                    w += fm.horizontalAdvance(c);
                }
            }
            else
            {
                path.addText(0, static_cast<qreal>(fm.ascent()), fnt, tl.qsText);
            }

            auto *item = new QGraphicsPathItem(this);
            item->setPath(path);

            QPen itemPen = item->pen();
            itemPen.setColor(color);
            itemPen.setCapStyle(Qt::RoundCap);
            itemPen.setJoinStyle(Qt::RoundJoin);
            itemPen.setWidthF(penWidth);
            item->setPen(itemPen);
            item->setBrush(settings->GetSingleStrokeOutlineFont() ? QBrush(Qt::NoBrush) : QBrush(color));
            item->setTransform(lineMatrix);
            m_labelPathItems.append(item);

            dY += fm.height() + penWidth + MmToPixel(1.5) + tm.GetSpacing();
        }
        else
        {
            auto *item = new QGraphicsSimpleTextItem(this);
            item->setFont(fnt);
            item->setText(tl.qsText);
            item->setBrush(QBrush(color));
            item->setTransform(lineMatrix);
            m_labelTextItems.append(item);

            dY += (fm.height() + MmToPixel(1.5) + tm.GetSpacing());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::InitPieceLabel(const QVector<QPointF> &labelShape, const VTextManager &tm)
{
    if (const VCommonSettings *settings = VAbstractApplication::VApp()->Settings(); settings->GetSingleLineFonts())
    {
        InitPieceLabelSVGFont(labelShape, tm);
    }
    else
    {
        InitPieceLabelOutlineFont(labelShape, tm);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::InitGrainlineItem()
{
    if (m_grainlineItem != nullptr)
    {
        m_grainlineItem->hide();
    }

    VPPiecePtr const piece = m_piece.toStrongRef();
    if (piece.isNull())
    {
        return;
    }

    if (piece->IsGrainlineEnabled() && piece->IsGrainlineVisible())
    {
        if (m_grainlineItem == nullptr)
        {
            m_grainlineItem = new VGraphicsFillItem(this);
        }
        m_grainlineItem->setPath(VLayoutPiece::GrainlinePath(piece->GetMappedGrainlineShape()));

        const VPSettings *settings = VPApplication::VApp()->PuzzleSettings();

        QPen const pen(PieceColor(), settings->GetLayoutLineWidth(), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        m_grainlineItem->SetCustomPen(true);
        m_grainlineItem->setPen(pen);

        const bool penPrinting = m_printMode
                                 && (settings->GetSingleLineFonts() || settings->GetSingleStrokeOutlineFont());
        m_grainlineItem->SetNoBrush(penPrinting);

        m_grainlineItem->show();
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
    m_foldLineMarkPath = QPainterPath();
    m_foldLineLabelPath = QPainterPath();
    m_mirrorLinePath = QPainterPath();

    VPPiecePtr const piece = m_piece.toStrongRef();
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

    PaintMirrorLine(painter, piece);

    PaintFoldLine(painter, piece);

    PaintStickyPath(painter);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::PaintSeamLine(QPainter *painter, const VPPiecePtr &piece)
{
    if (piece->IsHideMainPath() && piece->IsSeamAllowance() && not piece->IsSeamAllowanceBuiltIn())
    {
        return;
    }

    VPLayoutPtr const layout = piece->Layout();
    if (layout.isNull())
    {
        return;
    }

    QVector<VLayoutPoint> const seamLinePoints = piece->GetMappedFullContourPoints(
        layout->LayoutSettings().IsBoundaryTogetherWithNotches());
    if (seamLinePoints.isEmpty())
    {
        return;
    }

    QVector<QPointF> convertedPoints;
    CastTo(seamLinePoints, convertedPoints);

    m_seamLine.addPolygon(QPolygonF(convertedPoints));
    m_seamLine.closeSubpath();

    if (painter != nullptr)
    {
        QPainterStateGuard const guard(painter);
        painter->setBrush(piece->IsSelected() ? SelectionBrush() : NoBrush());
        painter->drawPath(m_seamLine);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::PaintCuttingLine(QPainter *painter, const VPPiecePtr &piece)
{
    if (!piece->IsSeamAllowance() || piece->IsSeamAllowanceBuiltIn())
    {
        return;
    }

    VPLayoutPtr const layout = piece->Layout();
    if (layout.isNull())
    {
        return;
    }

    QVector<VLayoutPoint> const cuttingLinepoints = piece->GetMappedFullSeamAllowancePoints(
        layout->LayoutSettings().IsBoundaryTogetherWithNotches());
    if (cuttingLinepoints.isEmpty())
    {
        return;
    }

    QVector<QPointF> convertedPoints;
    CastTo(cuttingLinepoints, convertedPoints);

    m_cuttingLine.addPolygon(QPolygonF(convertedPoints));
    m_cuttingLine.closeSubpath();

    if (painter != nullptr)
    {
        QPainterStateGuard const guard(painter);
        painter->setBrush(piece->IsSelected() ? SelectionBrush() : NoBrush());
        painter->drawPath(m_cuttingLine);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::PaintInternalPaths(QPainter *painter, const VPPiecePtr &piece)
{
    QVector<VLayoutPiecePath> const internalPaths = piece->GetInternalPaths();
    for (const auto &piecePath : internalPaths)
    {
        QPainterPath path = piece->GetMatrix().map(piecePath.GetPainterPath());

        if (!piecePath.IsNotMirrored() && piece->IsShowFullPiece() && !piece->GetSeamMirrorLine().isNull())
        {
            QVector<VLayoutPoint> points = piecePath.Points();
            const QTransform matrix = VGObject::FlippingMatrix(piece->GetSeamMirrorLine());
            std::transform(points.begin(), points.end(), points.begin(),
                           [&matrix](const VLayoutPoint &point) { return VAbstractPiece::MapPoint(point, matrix); });
            QVector<QPointF> casted;
            CastTo(points, casted);
            path.addPath(piece->GetMatrix().map(VPiecePath::MakePainterPath(casted)));
        }

        if (painter != nullptr)
        {
            QPainterStateGuard const guard(painter);
            QPen pen = painter->pen();
            pen.setStyle(piecePath.PenStyle());
            painter->setPen(pen);
            painter->drawPath(path);
        }
        m_internalPaths.addPath(path);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::PaintPassmarks(QPainter *painter, const VPPiecePtr &piece)
{
    VPLayoutPtr const layout = piece->Layout();
    if (layout.isNull())
    {
        return;
    }

    if (layout->LayoutSettings().IsBoundaryTogetherWithNotches())
    {
        return;
    }

    QVector<VLayoutPassmark> const passmarks = piece->GetMappedPassmarks();
    for (const auto &passmark : passmarks)
    {
        if (piece->IsHideMainPath() && passmark.isBuiltIn)
        {
            continue;
        }

        QPainterPath passmarkPath;
        for (const auto &line : passmark.lines)
        {
            passmarkPath.moveTo(line.p1());
            passmarkPath.lineTo(line.p2());
        }

        m_passmarks.addPath(passmarkPath);

        if (QLineF const seamAllowanceMirrorLine = piece->GetMappedSeamAllowanceMirrorLine();
            !seamAllowanceMirrorLine.isNull() && piece->IsShowFullPiece()
            && !IsPointOnLineviaPDP(passmark.baseLine.p1(), seamAllowanceMirrorLine.p1(), seamAllowanceMirrorLine.p2())
            && !passmark.notMirrored)
        {
            QPainterPath mirroredPassmaksPath;
            for (const auto &line : passmark.lines)
            {
                mirroredPassmaksPath.moveTo(line.p1());
                mirroredPassmaksPath.lineTo(line.p2());
            }
            const QTransform matrix = VGObject::FlippingMatrix(seamAllowanceMirrorLine);
            m_passmarks.addPath(matrix.map(mirroredPassmaksPath));
        }
    }

    if (painter != nullptr)
    {
        QPainterStateGuard const guard(painter);
        painter->setBrush(NoBrush());
        painter->drawPath(m_passmarks);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::PaintPlaceLabels(QPainter *painter, const VPPiecePtr &piece)
{
    QVector<VLayoutPlaceLabel> const placeLabels = piece->GetPlaceLabels();
    for (const auto &placeLabel : placeLabels)
    {
        QPainterPath path =
            VAbstractPiece::LabelShapePath(piece->MapPlaceLabelShape(VAbstractPiece::PlaceLabelShape(placeLabel)));

        if (!placeLabel.IsNotMirrored() && piece->IsShowFullPiece() && !piece->GetSeamMirrorLine().isNull())
        {
            PlaceLabelImg shape = VAbstractPiece::PlaceLabelShape(placeLabel);
            const QTransform matrix = VGObject::FlippingMatrix(piece->GetSeamMirrorLine());
            for (auto &points : shape)
            {
                std::transform(points.begin(), points.end(), points.begin(),
                               [&matrix](const VLayoutPoint &point)
                               { return VAbstractPiece::MapPoint(point, matrix); });
            }

            path.addPath(VAbstractPiece::LabelShapePath(piece->MapPlaceLabelShape(shape)));
        }

        if (painter != nullptr)
        {
            QPainterStateGuard const guard(painter);
            painter->setBrush(NoBrush());
            painter->drawPath(path);
        }

        m_placeLabels.addPath(path);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::PaintStickyPath(QPainter *painter)
{
    if (not m_stickyPoints.isEmpty())
    {
        m_stickyPath.moveTo(m_stickyPoints.constFirst());
        for (int i = 1; i < m_stickyPoints.size(); i++)
        {
            m_stickyPath.lineTo(m_stickyPoints.at(i));
        }

        if (painter != nullptr)
        {
            QPainterStateGuard const guard(painter);
            painter->setBrush(QBrush(VSceneStylesheet::ManualLayoutStyle().PieceOkColor(), Qt::BDiagPattern));

            QPen pen = painter->pen();
            pen.setStyle(Qt::DashLine);
            pen.setColor(VSceneStylesheet::ManualLayoutStyle().PieceOkColor());
            painter->setPen(pen);

            painter->drawPath(m_stickyPath);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::PaintMirrorLine(QPainter *painter, const VPPiecePtr &piece)
{
    VPLayoutPtr const layout = piece->Layout();
    if (layout.isNull() || !piece->IsShowFullPiece() || !piece->IsShowMirrorLine())
    {
        return;
    }

    QLineF const mirrorLine = piece->GetMappedCorrectedMirrorLine(
        layout->LayoutSettings().IsBoundaryTogetherWithNotches());
    if (mirrorLine.isNull())
    {
        return;
    }

    QPainterPath mirrorPath;
    mirrorPath.moveTo(mirrorLine.p1());
    mirrorPath.lineTo(mirrorLine.p2());
    m_mirrorLinePath.addPath(mirrorPath);

    if (painter != nullptr)
    {
        QPainterStateGuard const guard(painter);
        QPen pen = painter->pen();
        pen.setStyle(Qt::DashDotLine);
        painter->setPen(pen);
        painter->drawPath(m_mirrorLinePath);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::HideFoldLineLabel()
{
    if (m_foldLineLabelText != nullptr)
    {
        m_foldLineLabelText->setVisible(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::PrepareFoldLineLabel(const VFoldLine &fLine, const VCommonSettings *settings)
{
    if (!m_textAsPaths && !settings->GetSingleStrokeOutlineFont() && !settings->GetSingleLineFonts())
    {
        if (m_foldLineLabelText == nullptr)
        {
            m_foldLineLabelText = new QGraphicsSimpleTextItem(this);
        }
        fLine.UpdateFoldLineLabel(m_foldLineLabelText);
        m_foldLineLabelText->setBrush(QBrush(PieceColor()));
    }
    else if (m_foldLineLabelText != nullptr)
    {
        m_foldLineLabelText->setVisible(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::AddFoldLinePaths(const VPPiecePtr &piece, const QVector<QPainterPath> &shape, bool singleLineFont)
{
    if (piece->GetFoldLineType() == FoldLineType::ThreeDots || piece->GetFoldLineType() == FoldLineType::ThreeX ||
        piece->GetFoldLineType() == FoldLineType::TwoArrows)
    {
        m_foldLineMarkPath.addPath(shape.constFirst());
    }
    else if (piece->GetFoldLineType() == FoldLineType::Text)
    {
        if (singleLineFont || m_textAsPaths)
        {
            m_foldLineLabelPath.addPath(shape.constFirst());
        }
    }
    else
    {
        m_foldLineMarkPath.addPath(shape.constFirst());

        if (shape.size() > 1 && (singleLineFont || m_textAsPaths))
        {
            m_foldLineLabelPath.addPath(shape.constLast());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::DrawFoldLineMark(QPainter *painter) const
{
    QPainterStateGuard const guard(painter);
    painter->setBrush(Qt::SolidPattern);
    painter->drawPath(m_foldLineMarkPath);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::DrawFoldLineLabel(QPainter *painter, const VPPiecePtr &piece, bool singleLineFont) const
{
    const qreal penWidth = VPApplication::VApp()->PuzzleSettings()->GetLayoutLineWidth();

    QPainterStateGuard const guard(painter);
    QPen pen = painter->pen();
    pen.setWidthF(penWidth * qMin(piece->GetXScale(), piece->GetYScale()));
    pen.setColor(PieceColor());
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(singleLineFont ? Qt::NoBrush : Qt::SolidPattern);
    painter->drawPath(m_foldLineLabelPath);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::PaintFoldLine(QPainter *painter, const VPPiecePtr &piece)
{
    if (ShouldSkipPainting(piece))
    {
        HideFoldLineLabel();

        return;
    }

    VFoldLine const fLine = piece->FoldLine();
    QVector<QPainterPath> const shape = fLine.FoldLinePath();

    if (shape.isEmpty())
    {
        return;
    }

    const VCommonSettings *settings = VAbstractApplication::VApp()->Settings();

    PrepareFoldLineLabel(fLine, settings);

    const bool singleLineFont = settings->GetSingleStrokeOutlineFont() || settings->GetSingleLineFonts();
    AddFoldLinePaths(piece, shape, singleLineFont);

    if (painter != nullptr)
    {
        DrawFoldLineMark(painter);
        DrawFoldLineLabel(painter, piece, singleLineFont);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::GroupMove(const QPointF &pos)
{
    VPPiecePtr const piece = m_piece.toStrongRef();
    if (piece.isNull())
    {
        return;
    }

    VPLayoutPtr const layout = piece->Layout();
    if (layout.isNull())
    {
        return;
    }

    auto PreparePieces = [layout]()
    {
        QList<VPPiecePtr> pieces;

        if (VPSheetPtr const sheet = layout->GetFocusedSheet(); not sheet.isNull())
        {
            return sheet->GetSelectedPieces();
        }

        return pieces;
    };

    QList<VPPiecePtr> const pieces = PreparePieces();
    QPointF const newPos = pos - m_moveStartPoint;

    if (qFuzzyIsNull(newPos.x()) && qFuzzyIsNull(newPos.y()))
    {
        return;
    }

    if (pieces.size() == 1)
    {
        const VPPiecePtr &p = pieces.constFirst();
        auto *command = new VPUndoPieceMove(piece, newPos.x(), newPos.y(), m_allowChangeMerge);
        layout->UndoStack()->push(command);

        if (layout->LayoutSettings().IsStickyEdges())
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
    VPPiecePtr const piece = m_piece.toStrongRef();
    if (piece.isNull())
    {
        return VSceneStylesheet::ManualLayoutStyle().PieceOkColor();
    }

    VPLayoutPtr const layout = piece->Layout();
    if (layout.isNull())
    {
        return VSceneStylesheet::ManualLayoutStyle().PieceOkColor();
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

    bool pieceGape = false;
    if (layout->LayoutSettings().GetWarningPieceGapePosition())
    {
        pieceGape = piece->HasInvalidPieceGapPosition();
    }

    if (outOfBound || superposition || pieceGape)
    {
        return VSceneStylesheet::ManualLayoutStyle().PieceErrorColor();
    }

    return VSceneStylesheet::ManualLayoutStyle().PieceOkColor();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPiece::NoBrush() const -> QBrush
{
    return m_hoverMode ? QBrush(VSceneStylesheet::ManualLayoutStyle().PieceHoverColor()) : QBrush(Qt::NoBrush);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPiece::on_RefreshPiece(const VPPiecePtr &piece)
{
    VPPiecePtr const p = m_piece.toStrongRef();
    if (p.isNull() || piece.isNull())
    {
        return;
    }

    if (p->GetUniqueID() == piece->GetUniqueID())
    {
        setZValue(piece->ZValue());

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
    VPPiecePtr const p = m_piece.toStrongRef();
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
    if (scene() != nullptr && change == ItemSelectedHasChanged)
    {
        VPPiecePtr const piece = m_piece.toStrongRef();
        if (not piece.isNull())
        {
            piece->SetSelected(value.toBool());

            VPLayoutPtr const layout = piece->Layout();
            if (not layout.isNull())
            {
                emit layout->PieceSelectionChanged(piece);
            }
        }
    }

    return QGraphicsObject::itemChange(change, value);
}
