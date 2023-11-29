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

#include <QtMath>

#include "../vgeometry/vlayoutplacelabel.h"
#include "../vlayout/vlayoutpiecepath.h"
#include "../vlayout/vtextmanager.h"
#include "qline.h"
#include "vpiecegrainline.h"
#include "vplayout.h"
#include "vpsheet.h"

#include <QIcon>
#include <QLoggingCategory>
#include <QPainter>
#include <QPainterPath>

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
constexpr qreal maxStickyDistance = MmToPixel(10.);
constexpr qreal stickyShift = MmToPixel(1.);

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

//---------------------------------------------------------------------------------------------------------------------
auto PrepareStickyPath(const QVector<QPointF> &path) -> QVector<QPointF>
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
auto ClosestDistance(const QVector<QPointF> &path1, const QVector<QPointF> &path2) -> QLineF
{
    qreal distance = INT_MAX;
    QLineF closestDistance;

    for (auto p1 : path1)
    {
        for (auto p2 : path2)
        {
            QLineF d(p1, p2);
            if (d.length() <= distance)
            {
                distance = d.length();
                closestDistance = d;
            }
        }
    }

    return closestDistance;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VPPiece::VPPiece(const VLayoutPiece &layoutPiece)
  : VLayoutPiece(layoutPiece)
{
    ClearTransformations();

    if (IsForceFlipping())
    {
        FlipVertically();
    }
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
    SetGrainline(piece->GetGrainline());
    SetPieceLabelRect(piece->GetPieceLabelRect());
    SetPieceLabelData(piece->GetPieceLabelData());
    SetPatternLabelRect(piece->GetPatternLabelRect());
    SetPatternLabelData(piece->GetPatternLabelData());
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
auto VPPiece::GetPosition() -> QPointF
{
    QTransform matrix = GetMatrix();
    return {matrix.dx(), matrix.dy()};
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::RotateToGrainline(const VPTransformationOrigon &origin)
{
    VPSheetPtr sheet = Sheet();
    if (not IsGrainlineEnabled() || sheet.isNull())
    {
        return;
    }

    const QLineF grainline = GetMappedGrainlineMainLine();
    if (grainline.isNull())
    {
        return;
    }

    QLineF fabricGrainline(grainline.p1().x(), grainline.p1().y(), grainline.p1().x() + 100, grainline.p1().y());
    if (sheet->GrainlineOrientation() == GrainlineType::Vertical)
    {
        fabricGrainline.setAngle(fabricGrainline.angle() - 90);
    }

    QVector<qreal> angles;
    angles.reserve(4);

    const VPieceGrainline pieceGrainline = GetGrainline();

    if (pieceGrainline.IsArrowUpEnabled())
    {
        angles.append(grainline.angleTo(fabricGrainline));
    }

    if (pieceGrainline.IsArrowDownEnabled())
    {
        QLineF arrow = grainline;
        arrow.setAngle(arrow.angle() + 180);
        angles.append(arrow.angleTo(fabricGrainline));
    }

    if (pieceGrainline.IsArrowLeftEnabled())
    {
        QLineF arrow = grainline;
        arrow.setAngle(arrow.angle() + 90);
        angles.append(arrow.angleTo(fabricGrainline));
    }

    if (pieceGrainline.IsArrowRightEnabled())
    {
        QLineF arrow = grainline;
        arrow.setAngle(arrow.angle() - 90);
        angles.append(arrow.angleTo(fabricGrainline));
    }

    qreal degrees = 0;
    if (not angles.isEmpty())
    {
        degrees = *std::min_element(angles.constBegin(), angles.constEnd());
    }

    Rotate(origin.custom ? MappedDetailBoundingRect().center() : origin.origin, degrees);
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::SetGrainline(const VPieceGrainline &grainline)
{
    VLayoutPiece::SetGrainline(grainline);
}

//---------------------------------------------------------------------------------------------------------------------
void VPPiece::FlipVertically()
{
    QTransform pieceMatrix = GetMatrix();
    QPointF center = pieceMatrix.map(DetailBoundingRect().center());

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
    QPointF center = pieceMatrix.map(DetailBoundingRect().center());

    QTransform m;
    m.translate(0, center.y());
    m.scale(1, -1);
    m.translate(0, -center.y());

    pieceMatrix *= m;
    SetMatrix(pieceMatrix);
    SetHorizontallyFlipped(!IsHorizontallyFlipped());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPPiece::StickyPosition(qreal &dx, qreal &dy) const -> bool
{
    VPLayoutPtr layout = Layout();
    if (layout.isNull() || not layout->LayoutSettings().GetStickyEdges())
    {
        return false;
    }

    const qreal pieceGap = layout->LayoutSettings().GetPiecesGap();
    if (pieceGap <= 0)
    {
        return false;
    }

    VPSheetPtr sheet = Sheet();
    if (sheet.isNull())
    {
        return false;
    }

    QList<VPPiecePtr> allPieces = sheet->GetPieces();

    if (allPieces.count() < 2)
    {
        return false;
    }

    QVector<QPointF> path;
    CastTo(GetMappedExternalContourPoints(), path);
    QRectF boundingRect = VLayoutPiece::BoundingRect(path);
    const qreal stickyDistance = pieceGap + minStickyDistance;
    QRectF stickyZone = QRectF(boundingRect.topLeft().x() - stickyDistance, boundingRect.topLeft().y() - stickyDistance,
                               boundingRect.width() + stickyDistance * 2, boundingRect.height() + stickyDistance * 2);

    QVector<QPointF> stickyPath = PrepareStickyPath(path);
    QLineF closestDistance;

    for (const auto &piece : allPieces)
    {
        if (piece.isNull() || piece->GetUniqueID() == GetUniqueID())
        {
            continue;
        }

        QVector<QPointF> piecePath;
        CastTo(piece->GetMappedExternalContourPoints(), piecePath);
        QRectF pieceBoundingRect = VLayoutPiece::BoundingRect(piecePath);

        if (stickyZone.intersects(pieceBoundingRect) || pieceBoundingRect.contains(stickyZone) ||
            stickyZone.contains(pieceBoundingRect))
        {
            if (not VPPiece::PathsSuperposition(path, piecePath))
            {
                QVector<QPointF> pieceStickyPath = PrepareStickyPath(piecePath);
                closestDistance = ClosestDistance(stickyPath, pieceStickyPath);
            }
        }
    }

    if (closestDistance.isNull())
    {
        return false;
    }

    const qreal extraZone = qBound(minStickyDistance, pieceGap * 50 / 100, maxStickyDistance);
    const qreal length = closestDistance.length();

    if (length > pieceGap && length <= pieceGap + extraZone)
    {
        closestDistance.setLength(length - pieceGap);
        QPointF diff = closestDistance.p2() - closestDistance.p1();
        dx = diff.x();
        dy = diff.y();
        return true;
    }

    if (length < pieceGap && length >= pieceGap - extraZone)
    {
        closestDistance.setAngle(closestDistance.angle() + 180);
        closestDistance.setLength(pieceGap - length);
        QPointF diff = closestDistance.p2() - closestDistance.p1();
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

    if (path1Rect.intersects(path2Rect) || path2Rect.contains(path1Rect) || path1Rect.contains(path2Rect))
    {
        if (path1Path.contains(path2Path) || path2Path.contains(path1Path) || path1Path.intersects(path2Path))
        {
            return true;
        }
    }

    return false;
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
        piece->SetGrainline(grainline);
    }

    piece->SetPieceLabelRect(MapVector(piece->GetPieceLabelRect(), matrix));
    piece->SetPatternLabelRect(MapVector(piece->GetPatternLabelRect(), matrix));
}
