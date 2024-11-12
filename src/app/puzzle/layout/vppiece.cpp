/************************************************************************
 **
 **  @file   vppiece.cpp
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
#include "vppiece.h"

#include "../vgeometry/vlayoutplacelabel.h"
#include "../vlayout/vlayoutpiecepath.h"
#include "../vlayout/vtextmanager.h"
#include "../vwidgets/vpiecegrainline.h"
#include "vplayout.h"
#include "vpsheet.h"

#include <QIcon>
#include <QLine>
#include <QLoggingCategory>
#include <QPainter>
#include <QPainterPath>
#include <QtConcurrent>
#include <QtMath>
#include <functional>
#include <limits>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(pPiece, "p.piece") // NOLINT

QT_WARNING_POP

namespace
{
constexpr qreal minStickyDistance = MmToPixel(3.);
constexpr qreal maxStickyDistance = MmToPixel(15.);
constexpr qreal stickyShift = MmToPixel(5.);

//---------------------------------------------------------------------------------------------------------------------
auto CutEdge(const QLineF &edge) -> QVector<QPointF>
{
    QVector<QPointF> points;
    if (qFuzzyIsNull(stickyShift))
    {
        points.append(edge.p1());
        points.append(edge.p2());
    }
    else
    {
        const int n = qFloor(edge.length() / stickyShift);

        if (n <= 0)
        {
            points.append(edge.p1());
            points.append(edge.p2());
        }
        else
        {
            points.reserve(n);
            const qreal nShift = edge.length() / n;
            for (int i = 1; i <= n + 1; ++i)
            {
                QLineF l1 = edge;
                l1.setLength(nShift * (i - 1));
                points.append(l1.p2());
            }
        }
    }
    return points;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VPPiece::VPPiece(const VLayoutPiece &layoutPiece)
  : VLayoutPiece(layoutPiece)
{
    ClearTransformations();
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::Update(const VPPiecePtr &piece)
{
    if (piece.isNull())
    {
        return;
    }

    SetName(piece->GetName());
    SetContourPoints(piece->GetContourPoints(), piece->IsHideMainPath());
    SetSeamAllowancePoints(piece->GetSeamAllowancePoints(), piece->IsSeamAllowance(), piece->IsSeamAllowanceBuiltIn());
    SetInternalPaths(piece->GetInternalPaths());
    SetPassmarks(piece->GetPassmarks());
    SetPlaceLabels(piece->GetPlaceLabels());
    SetVPGrainline(piece->GetGrainline());
    SetPieceLabelRect(piece->GetPieceLabelRect());
    SetPieceLabelData(piece->GetPieceLabelData());
    SetPatternLabelRect(piece->GetPatternLabelRect());
    SetPatternLabelData(piece->GetPatternLabelData());
    SetSeamMirrorLine(piece->GetSeamMirrorLine());
    SetSeamAllowanceMirrorLine(piece->GetSeamAllowanceMirrorLine());
    SetFoldLineLabel(piece->GetFoldLineLabel());
    SetFoldLineLabelAlignment(piece->GetFoldLineLabelAlignment());
    SetFoldLineType(piece->GetFoldLineType());
    SetFoldLineSvgFontSize(piece->GetFoldLineSvgFontSize());
    SetFoldLineLabelFontItalic(piece->IsFoldLineLabelFontItalic());
    SetFoldLineLabelFontBold(piece->IsFoldLineLabelFontBold());
    SetFoldLineOutlineFont(piece->GetFoldLineOutlineFont());
    SetFoldLineSVGFontFamily(piece->GetFoldLineSVGFontFamily());
    SetFoldLineHeight(piece->GetFoldLineHeight());
    SetFoldLineWidth(piece->GetFoldLineWidth());
    SetForceFlipping(piece->IsForceFlipping());
    SetForbidFlipping(piece->IsForbidFlipping());
    SetShowMirrorLine(piece->IsShowMirrorLine());

    if ((IsForceFlipping() && !IsVerticallyFlipped()) || (IsForbidFlipping() && IsVerticallyFlipped()))
    {
        FlipVertically();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPPiece::GetUniqueID() const -> QString
{
    QString id = VLayoutPiece::GetUniqueID();

    if (m_copyNumber > 1)
    {
        id = id + '_'_L1 + QString::number(m_copyNumber);
    }

    return id;
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::ClearTransformations()
{
    // Reset the piece position to the default state
    SetMatrix(QTransform());

    // restore original size
    QTransform m;
    m.scale(GetXScale(), GetYScale());
    QTransform matrix = GetMatrix();
    matrix *= m;
    SetMatrix(matrix);

    // translate the piece so that the top left corner of the bouding rect of the piece is at the position
    // (0,0) in the sheet coordinate system
    const QPointF offset = MappedDetailBoundingRect().topLeft();
    Translate(-offset.x(), -offset.y());

    SetVerticallyFlipped(false);
    SetHorizontallyFlipped(false);

    if (IsForceFlipping())
    {
        FlipVertically();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetPosition(QPointF point)
{
    QTransform matrix = GetMatrix();
    const QPointF offset = MappedDetailBoundingRect().topLeft();
    matrix.translate(point.x() - offset.x(), point.y() - offset.y());
    SetMatrix(matrix);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPPiece::GetPosition() const -> QPointF
{
    QTransform const matrix = GetMatrix();
    return {matrix.dx(), matrix.dy()};
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::RotateToGrainline(const VPTransformationOrigon &origin)
{
    VPSheetPtr const sheet = Sheet();
    if (not IsGrainlineEnabled() || sheet.isNull())
    {
        return;
    }

    const QLineF grainline = GetMappedGrainlineMainLine();
    if (grainline.isNull())
    {
        return;
    }

    QLineF fabricGrainline(grainline.p1().x(), grainline.p1().y(), grainline.p1().x() - 100, grainline.p1().y());
    if (sheet->GrainlineOrientation() == GrainlineType::Vertical)
    {
        fabricGrainline.setAngle(fabricGrainline.angle() - 90);
    }

    QVector<qreal> angles;
    angles.reserve(8);

    const VPieceGrainline pieceGrainline = GetGrainline();

    if (pieceGrainline.IsArrowUpEnabled())
    {
        qreal const angle = grainline.angleTo(fabricGrainline);
        angles.append(angle);
        angles.append(-(360. - angle));
    }

    if (pieceGrainline.IsArrowDownEnabled())
    {
        QLineF arrow = grainline;
        arrow.setAngle(arrow.angle() + 180);

        QLineF fabricArrow = fabricGrainline;
        fabricArrow.setAngle(fabricArrow.angle() + 180);

        qreal const angle = arrow.angleTo(fabricArrow);
        angles.append(angle);
        angles.append(-(360. - angle));
    }

    if (pieceGrainline.IsArrowLeftEnabled())
    {
        QLineF arrow = grainline;
        arrow.setAngle(arrow.angle() + 90);

        QLineF fabricArrow = fabricGrainline;
        fabricArrow.setAngle(fabricArrow.angle() + 90);

        qreal const angle = arrow.angleTo(fabricArrow);
        angles.append(angle);
        angles.append(-(360. - angle));
    }

    if (pieceGrainline.IsArrowRightEnabled())
    {
        QLineF arrow = grainline;
        arrow.setAngle(arrow.angle() - 90);

        QLineF fabricArrow = fabricGrainline;
        fabricArrow.setAngle(fabricArrow.angle() - 90);

        qreal const angle = arrow.angleTo(fabricArrow);
        angles.append(angle);
        angles.append(-(360. - angle));
    }

    qreal degrees = 0;
    if (not angles.isEmpty())
    {
        qreal minAbsAngle = qAbs(angles.constFirst());
        degrees = angles.constFirst();

        for (int i = 1; i < angles.size(); ++i)
        {
            qreal const absAngle = qAbs(angles.at(i));
            if (absAngle < minAbsAngle)
            {
                minAbsAngle = absAngle;
                degrees = angles.at(i);
            }
        }
    }

    Rotate(origin.custom ? MappedDetailBoundingRect().center() : origin.origin, degrees);
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetVPGrainline(const VPieceGrainline &grainline)
{
    VLayoutPiece::SetGrainline(grainline);
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::FlipVertically()
{
    QTransform pieceMatrix = GetMatrix();
    QPointF const center = pieceMatrix.map(DetailBoundingRect().center());

    QTransform m;
    m.translate(center.x(), 0);
    m.scale(-1, 1);
    m.translate(-center.x(), 0);

    pieceMatrix *= m;
    SetMatrix(pieceMatrix);
    SetVerticallyFlipped(!IsVerticallyFlipped());
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::FlipHorizontally()
{
    QTransform pieceMatrix = GetMatrix();
    QPointF const center = pieceMatrix.map(DetailBoundingRect().center());

    QTransform m;
    m.translate(0, center.y());
    m.scale(1, -1);
    m.translate(0, -center.y());

    pieceMatrix *= m;
    SetMatrix(pieceMatrix);
    SetHorizontallyFlipped(!IsHorizontallyFlipped());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPPiece::HasInvalidPieceGapPosition() const -> bool
{
    return m_invalidPieceGapPosition;
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetHasInvalidPieceGapPosition(bool status)
{
    m_invalidPieceGapPosition = status;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPPiece::StickyPosition(qreal &dx, qreal &dy) const -> bool
{
    if (VPLayoutPtr const layout = Layout(); layout.isNull() || not layout->LayoutSettings().IsStickyEdges())
    {
        return false;
    }

    if (VPSheetPtr const sheet = Sheet(); sheet.isNull())
    {
        return false;
    }

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    VStickyDistance match;

    QT_WARNING_POP

    if (!StickySheet(match))
    {
        return false;
    }

    if (!StickyPieces(match))
    {
        return false;
    }

    if (match.m_closestDistance.isNull())
    {
        return false;
    }

    qreal extraZone = qBound(minStickyDistance, match.m_pieceGap * 50 / 100, maxStickyDistance);
    if (qFuzzyIsNull(match.m_pieceGap))
    {
        extraZone = maxStickyDistance;
    }
    const qreal length = match.m_closestDistance.length();

    if (length > match.m_pieceGap && length <= match.m_pieceGap + extraZone)
    {
        match.m_closestDistance.setLength(length - match.m_pieceGap);
        QPointF const diff = match.m_closestDistance.p2() - match.m_closestDistance.p1();
        dx = diff.x();
        dy = diff.y();
        return true;
    }

    if (length < match.m_pieceGap && length >= match.m_pieceGap - extraZone)
    {
        match.m_closestDistance.setAngle(match.m_closestDistance.angle() + 180);
        match.m_closestDistance.setLength(match.m_pieceGap - length);
        QPointF const diff = match.m_closestDistance.p2() - match.m_closestDistance.p1();
        dx = diff.x();
        dy = diff.y();
        return true;
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPPiece::PathsSuperposition(const QVector<QPointF> &path1, const QVector<QPointF> &path2) -> bool
{
    const QRectF path1Rect = VLayoutPiece::BoundingRect(path1);
    const QPainterPath path1Path = VGObject::PainterPath(path1);

    const QRectF path2Rect = VLayoutPiece::BoundingRect(path2);
    const QPainterPath path2Path = VGObject::PainterPath(path2);

    return (path1Rect.intersects(path2Rect) || path2Rect.contains(path1Rect) || path1Rect.contains(path2Rect)) &&
           (path1Path.contains(path2Path) || path2Path.contains(path1Path) || path1Path.intersects(path2Path));
}

//---------------------------------------------------------------------------------------------------------------------
auto VPPiece::PrepareStickyPath(const QVector<QPointF> &path) -> QVector<QPointF>
{
    if (path.size() < 2)
    {
        return path;
    }

    QVector<QPointF> stickyPath;

    for (int i = 0; i < path.size(); ++i)
    {
        stickyPath += CutEdge(QLineF(path.at(i), path.at(i < path.size() - 1 ? i + 1 : 0)));
    }

    return stickyPath;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPPiece::ClosestDistance(const QVector<QPointF> &path1, const QVector<QPointF> &path2) -> QLineF
{
    const int maxThreads = QThread::idealThreadCount();

    QVector<QVector<QPointF>> path1Chunks;
    path1Chunks.reserve(maxThreads);
    const vsizetype chunkSize = (path1.size() + maxThreads - 1) / maxThreads; // Round up
    for (vsizetype i = 0; i < path1.size(); i += chunkSize)
    {
        path1Chunks.append(path1.mid(i, chunkSize));
    }

    std::function<void(QLineF &, const QLineF &)> const ReduceFunc = [](QLineF &result, const QLineF &next)
    {
        qreal const dist1 = result.length();
        qreal const dist2 = next.length();
        if (result.isNull() || dist2 < dist1)
        {
            result = next;
        }
    };

    std::function<QLineF(const QVector<QPointF> &)> const CalculateClosestDistanceForChunk =
        [path2](const QVector<QPointF> &chunk)
    {
        qreal minLocalDistance = std::numeric_limits<qreal>::max();
        QLineF localClosestDistance;

        for (const auto &c : chunk)
        {
            for (const auto &p2 : path2)
            {
                QLineF const d(c, p2);
                qreal const length = d.length();
                if (length < minLocalDistance)
                {
                    minLocalDistance = length;
                    localClosestDistance = d;
                }
            }
        }

        return localClosestDistance;
    };

    return QtConcurrent::blockingMappedReduced<QLineF>(path1Chunks, CalculateClosestDistanceForChunk, ReduceFunc);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPPiece::IsValid(QString &error) const -> bool
{
    if (not IsHideMainPath() && GetContourPoints().isEmpty())
    {
        error = tr("Seam line is empty");
        return false;
    }

    if (IsSeamAllowance() && IsSeamAllowanceBuiltIn() && GetContourPoints().isEmpty())
    {
        error = tr("Seam line is empty");
        return false;
    }

    if (IsSeamAllowance() && not IsSeamAllowanceBuiltIn() && GetSeamAllowancePoints().isEmpty())
    {
        error = tr("Seam allowance line is empty");
        return false;
    }

    if (IsGrainlineEnabled() && not GetGrainline().IsShapeValid())
    {
        error = tr("Grainline is empty");
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPPiece::CopyNumber() const -> quint16
{
    return m_copyNumber;
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetCopyNumber(quint16 newCopyNumber)
{
    m_copyNumber = qMax(static_cast<quint16>(1), newCopyNumber);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPPiece::StickySheet(VStickyDistance &match) const -> bool
{
    VPLayoutPtr const layout = Layout();
    if (layout.isNull())
    {
        return false;
    }

    const qreal pieceGap = layout->LayoutSettings().GetPiecesGap();
    if (pieceGap <= 0)
    {
        return false;
    }

    VPSheetPtr const sheet = Sheet();
    if (sheet.isNull())
    {
        return false;
    }

    QRectF sheetRect = sheet->GetMarginsRect();
    QVector<QPointF> path;
    CastTo(GetMappedExternalContourPoints(), path);
    QRectF const boundingRect = VLayoutPiece::BoundingRect(path);
    if (!sheetRect.contains(boundingRect))
    {
        return true;
    }

    const qreal stickyDistance = maxStickyDistance;
    if (auto const stickyZone =
            QRectF(boundingRect.topLeft().x() - stickyDistance, boundingRect.topLeft().y() - stickyDistance,
                   boundingRect.width() + stickyDistance * 2, boundingRect.height() + stickyDistance * 2);
        !stickyZone.intersects(sheetRect))
    {
        return true;
    }

    QVector<QPointF> const stickyPath = PrepareStickyPath(path);

    if (!layout->LayoutSettings().IsCutOnFold())
    {
        sheetRect.adjust(accuracyPointOnLine, accuracyPointOnLine, -accuracyPointOnLine, -accuracyPointOnLine);
        QVector<QPointF> const sheetPath{sheetRect.topLeft(), sheetRect.topRight(), sheetRect.bottomRight(),
                                         sheetRect.bottomLeft(), sheetRect.topLeft()};
        QVector<QPointF> const sheetStickyPath = PrepareStickyPath(sheetPath);
        if (QLineF const distance = ClosestDistance(stickyPath, sheetStickyPath);
            match.m_closestDistance.isNull() || distance.length() < match.m_closestDistance.length())
        {
            match.m_closestDistance = distance;
        }
        return true;
    }

    if (GetSeamAllowanceMirrorLine().isNull() || (!GetSeamAllowanceMirrorLine().isNull() && IsShowFullPiece()))
    { // regular piece
        QVector<QPointF> sheetPath;
        if (sheetRect.width() >= sheetRect.height())
        {
            sheetRect.adjust(accuracyPointOnLine, accuracyPointOnLine, -accuracyPointOnLine, -accuracyPointOnLine);
            QPointF const shift(0, pieceGap / 2. - accuracyPointOnLine);
            sheetPath = {sheetRect.topLeft() + shift, sheetRect.topRight() + shift, sheetRect.bottomRight(),
                         sheetRect.bottomLeft(), sheetRect.topLeft() + shift};
        }
        else
        {
            sheetRect.adjust(accuracyPointOnLine, accuracyPointOnLine, -accuracyPointOnLine, -accuracyPointOnLine);
            QPointF const shift(pieceGap / 2. - accuracyPointOnLine, 0);
            sheetPath = {sheetRect.topLeft(), sheetRect.topRight() - shift, sheetRect.bottomRight() - shift,
                         sheetRect.bottomLeft(), sheetRect.topLeft()};
        }

        QVector<QPointF> const sheetStickyPath = PrepareStickyPath(sheetPath);
        if (QLineF const distance = ClosestDistance(stickyPath, sheetStickyPath);
            match.m_closestDistance.isNull() || distance.length() < match.m_closestDistance.length())
        {
            match.m_closestDistance = distance;
        }
        return true;
    }

    // mirrored piece
    QVector<QPointF> sheetPath;
    if (sheetRect.width() >= sheetRect.height())
    {
        QPointF const shift(0, accuracyPointOnLine);
        sheetPath = {sheetRect.topLeft() + shift, sheetRect.topRight() + shift};
    }
    else
    {
        QPointF const shift(accuracyPointOnLine, 0);
        sheetPath = {sheetRect.topRight() - shift, sheetRect.bottomRight() - shift};
    }

    QVector<QPointF> const sheetStickyPath = PrepareStickyPath(sheetPath);
    if (QLineF const distance = ClosestDistance(stickyPath, sheetStickyPath);
        match.m_closestDistance.isNull() || distance.length() < match.m_closestDistance.length())
    {
        match.m_closestDistance = distance;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPPiece::StickyPieces(VStickyDistance &match) const -> bool
{
    VPLayoutPtr const layout = Layout();
    if (layout.isNull())
    {
        return false;
    }

    const qreal pieceGap = layout->LayoutSettings().GetPiecesGap();
    if (pieceGap <= 0)
    {
        return false;
    }

    VPSheetPtr const sheet = Sheet();
    if (sheet.isNull())
    {
        return false;
    }

    QList<VPPiecePtr> const allPieces = sheet->GetPieces();
    if (allPieces.count() < 2)
    {
        return true;
    }

    QVector<QPointF> path;
    CastTo(GetMappedExternalContourPoints(), path);
    QRectF const boundingRect = VLayoutPiece::BoundingRect(path);
    const qreal stickyDistance = pieceGap + minStickyDistance;
    auto const stickyZone =
        QRectF(boundingRect.topLeft().x() - stickyDistance, boundingRect.topLeft().y() - stickyDistance,
               boundingRect.width() + stickyDistance * 2, boundingRect.height() + stickyDistance * 2);

    QVector<QPointF> const stickyPath = PrepareStickyPath(path);

    for (const auto &piece : allPieces)
    {
        if (piece.isNull() || piece->GetUniqueID() == GetUniqueID())
        {
            continue;
        }

        QVector<QPointF> piecePath;
        CastTo(piece->GetMappedExternalContourPoints(), piecePath);
        QRectF const pieceBoundingRect = VLayoutPiece::BoundingRect(piecePath);

        if ((stickyZone.intersects(pieceBoundingRect) || pieceBoundingRect.contains(stickyZone) ||
             stickyZone.contains(pieceBoundingRect)) &&
            not VPPiece::PathsSuperposition(path, piecePath))
        {

            QVector<QPointF> const pieceStickyPath = PrepareStickyPath(piecePath);
            if (QLineF const distance = ClosestDistance(stickyPath, pieceStickyPath);
                match.m_closestDistance.isNull() || distance.length() < match.m_closestDistance.length())
            {
                match.m_closestDistance = distance;
                match.m_pieceGap = pieceGap;
            }
        }
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::CleanPosition(const VPPiecePtr &piece)
{
    QVector<QPointF> points;
    CastTo(piece->GetExternalContourPoints(), points);
    if (points.isEmpty())
    {
        return;
    }

    const QPointF offset = BoundingRect(points).topLeft();
    if (qFuzzyIsNull(offset.x()) && qFuzzyIsNull(offset.y()))
    {
        return;
    }

    QTransform matrix;
    matrix.translate(-offset.x(), -offset.y());

    piece->SetContourPoints(MapVector(piece->GetContourPoints(), matrix), piece->IsHideMainPath());
    piece->SetSeamAllowancePoints(MapVector(piece->GetSeamAllowancePoints(), matrix), piece->IsSeamAllowance(),
                                  piece->IsSeamAllowanceBuiltIn());

    {
        QVector<VLayoutPiecePath> internalPaths = piece->GetInternalPaths();
        for (auto &path : internalPaths)
        {
            path.SetPoints(MapVector(path.Points(), matrix));
        }
        piece->SetInternalPaths(internalPaths);
    }

    {
        QVector<VLayoutPassmark> passmarks = piece->GetPassmarks();
        for (auto &passmark : passmarks)
        {
            passmark.lines = MapVector(passmark.lines, matrix);
            passmark.baseLine = matrix.map(passmark.baseLine);
        }
        piece->SetPassmarks(passmarks);
    }

    {
        QVector<VLayoutPlaceLabel> placeLabels = piece->GetPlaceLabels();
        for (auto &label : placeLabels)
        {
            label.SetRotationMatrix(label.RotationMatrix() * matrix);
        }
        piece->SetPlaceLabels(placeLabels);
    }

    {
        VPieceGrainline grainline = piece->GetGrainline();
        grainline.SetMainLine(matrix.map(grainline.GetMainLine()));
        piece->SetVPGrainline(grainline);
    }

    piece->SetPieceLabelRect(MapVector(piece->GetPieceLabelRect(), matrix));
    piece->SetPatternLabelRect(MapVector(piece->GetPatternLabelRect(), matrix));
    piece->SetSeamMirrorLine(matrix.map(piece->GetSeamMirrorLine()));
    piece->SetSeamAllowanceMirrorLine(matrix.map(piece->GetSeamAllowanceMirrorLine()));
}
