/************************************************************************
 **
 **  @file   vnestingframe.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 October, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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

#include "vnestingframe.h"
#include "../vwidgets/vpiecegrainline.h"
#include "vabstractpiece.h"
#include "vlayoutpiece.h"

#include <QJsonDocument>
#include <QLineF>
#include <QLocale>
#include <QPolygonF>
#include <QtMath>

#include <cmath>
#include <limits>
#include <utility>

// Qt's QTransform(m11, m12, m21, m22, dx, dy) maps (x, y) to (m11 x + m21 y + dx, m12 x + m22 y + dy) and
// a * b applies a first, then b.

namespace
{
constexpr qreal kAlmost = 1e-8;

auto RoundMm(qreal value) -> double
{
    return std::round(value * 1000.0) / 1000.0;
}

auto CleanRing(QVector<QPointF> poly) -> QVector<QPointF>
{
    QVector<QPointF> out;
    out.reserve(poly.size());
    for (const QPointF &point : poly)
    {
        if (out.isEmpty() || QLineF(out.constLast(), point).length() > kAlmost)
        {
            out.append(point);
        }
    }
    if (out.size() >= 2 && QLineF(out.constFirst(), out.constLast()).length() <= kAlmost)
    {
        out.removeLast();
    }
    return out;
}

auto SignedArea(const QVector<QPointF> &poly) -> qreal
{
    qreal sum = 0;
    const int n = static_cast<int>(poly.size());
    for (int i = 0; i < n; ++i)
    {
        const QPointF a = poly.at(i);
        const QPointF b = poly.at((i + 1) % n);
        sum += a.x() * b.y() - b.x() * a.y();
    }
    return sum / 2;
}

auto UnitVector(const QPointF &v) -> QPointF
{
    const qreal len = std::hypot(v.x(), v.y());
    if (len < kAlmost)
    {
        return {};
    }
    return {v.x() / len, v.y() / len};
}

// Right-hand normal of a unit direction. For a CCW ring it points outward.
auto RightNormal(const QPointF &direction) -> QPointF
{
    return {direction.y(), -direction.x()};
}

auto JsonRing(const QVector<QPointF> &poly) -> QJsonArray
{
    QJsonArray ring;
    for (const QPointF &point : poly)
    {
        ring.append(QJsonArray{RoundMm(point.x()), RoundMm(point.y())});
    }
    return ring;
}

auto ProfileOrigin(const QVector<QPointF> &contourPx, const QTransform &base) -> QPointF
{
    const QPolygonF strip = (base * NestingLocalToStrip()).map(QPolygonF(contourPx));
    return strip.boundingRect().topLeft(); // minimum x and minimum y
}

auto ProfilePoints(const QVector<QPointF> &contourPx, const QTransform &base) -> QVector<QPointF>
{
    const QPointF origin = ProfileOrigin(contourPx, base);
    const QTransform toProfile = base * NestingLocalToStrip() * QTransform::fromTranslate(-origin.x(), -origin.y());
    QVector<QPointF> points;
    points.reserve(contourPx.size());
    for (const QPointF &point : contourPx)
    {
        points.append(toProfile.map(point));
    }
    return CleanRing(points);
}

auto FlipPoint(const QPointF &point, const QString &flip) -> QPointF
{
    if (flip == QLatin1String("x"))
    {
        return {-point.x(), point.y()};
    }
    if (flip == QLatin1String("y"))
    {
        return {point.x(), -point.y()};
    }
    if (flip == QLatin1String("both"))
    {
        return {-point.x(), -point.y()};
    }
    return point;
}

auto RotatePoint(const QPointF &point, qreal degrees) -> QPointF
{
    const qreal th = qDegreesToRadians(degrees);
    const qreal c = qCos(th);
    const qreal s = qSin(th);
    return {point.x() * c - point.y() * s, point.x() * s + point.y() * c};
}

// The rigid map that takes profile vertices onto the same vertices in marker millimetres.
auto RecoverLock(const QVector<QPointF> &profile, const QVector<QPointF> &marker) -> VNestingPlacement
{
    VNestingPlacement best;
    best.flip.clear();
    qreal bestError = std::numeric_limits<qreal>::max();
    if (profile.size() < 2 || profile.size() != marker.size())
    {
        return best;
    }
    int far = 1;
    qreal farLen = 0;
    for (int i = 1; i < profile.size(); ++i)
    {
        const qreal len = QLineF(profile.at(0), profile.at(i)).length();
        if (len > farLen)
        {
            farLen = len;
            far = i;
        }
    }
    if (farLen < kAlmost)
    {
        return best;
    }
    // "y" before "x": the host's mirror is a reflection across the grain, and an x-mirror plus a turn can
    // describe the same pose. The first exact match is the one the host would have written.
    const QStringList flips{QStringLiteral("none"), QStringLiteral("y"), QStringLiteral("x"), QStringLiteral("both")};
    for (const QString &flip : flips)
    {
        const QPointF a = FlipPoint(profile.at(far), flip) - FlipPoint(profile.at(0), flip);
        const QPointF b = marker.at(far) - marker.at(0);
        if (std::hypot(a.x(), a.y()) < kAlmost || std::hypot(b.x(), b.y()) < kAlmost)
        {
            continue;
        }
        qreal degrees = qRadiansToDegrees(std::atan2(b.y(), b.x()) - std::atan2(a.y(), a.x()));
        while (degrees < 0)
        {
            degrees += 360;
        }
        while (degrees >= 360)
        {
            degrees -= 360;
        }
        const QPointF turned = RotatePoint(FlipPoint(profile.at(0), flip), degrees);
        const QPointF translation = marker.at(0) - turned;
        qreal error = 0;
        for (int i = 0; i < profile.size(); ++i)
        {
            const QPointF landed = RotatePoint(FlipPoint(profile.at(i), flip), degrees) + translation;
            error = qMax(error, QLineF(landed, marker.at(i)).length());
        }
        // A millimetre covers rounding. A larger miss means the rings are not the same vertices.
        if (error < bestError && error <= 1.0)
        {
            bestError = error;
            best.flip = flip;
            best.rotationDeg = degrees;
            best.xMm = translation.x();
            best.yMm = translation.y();
        }
    }
    return best;
}

auto PlacementObject(const VNestingPlacement &placement) -> QJsonObject
{
    return {{QStringLiteral("flip"), placement.flip},
            {QStringLiteral("rotationDeg"), RoundMm(placement.rotationDeg)},
            {QStringLiteral("translation"),
             QJsonObject{{QStringLiteral("xMm"), RoundMm(placement.xMm)},
                         {QStringLiteral("yMm"), RoundMm(placement.yMm)}}}};
}

auto ReadPlacement(const QJsonObject &object) -> VNestingPlacement
{
    const QJsonObject translation = object.value(QStringLiteral("translation")).toObject();
    VNestingPlacement placement;
    placement.instanceId = object.value(QStringLiteral("instanceId")).toString();
    placement.flip = object.value(QStringLiteral("flip")).toString(QStringLiteral("none"));
    placement.rotationDeg = object.value(QStringLiteral("rotationDeg")).toDouble();
    placement.xMm = translation.value(QStringLiteral("xMm")).toDouble();
    placement.yMm = translation.value(QStringLiteral("yMm")).toDouble();
    return placement;
}

auto ReadBinPlacements(const QJsonObject &bin) -> QHash<QString, VNestingPlacement>
{
    QHash<QString, VNestingPlacement> map;
    const QJsonArray placements = bin.value(QStringLiteral("placements")).toArray();
    for (const QJsonValue value : placements)
    {
        const VNestingPlacement placement = ReadPlacement(value.toObject());
        if (not placement.instanceId.isEmpty())
        {
            map.insert(placement.instanceId, placement);
        }
    }
    return map;
}

auto PolygonArea(const QJsonArray &ring) -> qreal
{
    QVector<QPointF> points;
    points.reserve(ring.size());
    for (const QJsonValue value : ring)
    {
        const QJsonArray pair = value.toArray();
        if (pair.size() >= 2)
        {
            points.append(QPointF(pair.at(0).toDouble(), pair.at(1).toDouble()));
        }
    }
    return qAbs(SignedArea(points));
}

} // namespace

//---------------------------------------------------------------------------------------------------------------------
auto NestingMapRotationAndFlip(NestingRotations rotations, bool forbidFlipping, bool forceFlipping)
    -> NestingRotationFlip
{
    // See the declaration. The loss lives in this one function and nowhere else in the job.
    NestingRotationFlip mapped;
    switch (rotations)
    {
        case NestingRotations::ZeroOnly:
            mapped.rotation = QStringLiteral("oneWay");
            break;
        case NestingRotations::Any:
            mapped.rotation = QStringLiteral("any");
            break;
        case NestingRotations::ZeroAnd180:
        default:
            mapped.rotation = QStringLiteral("twoWay");
            break;
    }
    mapped.flipAllowance = (forbidFlipping || forceFlipping) ? QStringLiteral("none") : QStringLiteral("y");
    return mapped;
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingLocalToStrip() -> QTransform
{
    // (x, y) px -> (y, -x) / k mm: local +y (grainline) becomes profile +x. Pattern +X becomes profile −Y.
    // A rotation, determinant +1. This is the contract's base transfer; the engine must not repeat it.
    const qreal s = 1 / NestingPxPerMm;
    return {0, -s, s, 0, 0, 0};
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingFlipMatrix(const QString &flip) -> QTransform
{
    // x mirrors across profile +Y. y mirrors across profile +X. both is the two mirrors, in that order.
    if (flip == QLatin1String("x"))
    {
        return QTransform::fromScale(-1, 1);
    }
    if (flip == QLatin1String("y"))
    {
        return QTransform::fromScale(1, -1);
    }
    if (flip == QLatin1String("both"))
    {
        return QTransform::fromScale(-1, -1);
    }
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingPlacementMatrix(const VNestingPlacement &p) -> QTransform
{
    const qreal th = qDegreesToRadians(p.rotationDeg);
    const qreal c = qCos(th);
    const qreal s = qSin(th);
    const QTransform rotate(c, s, -s, c, 0, 0); // counterclockwise: x' = x cos − y sin, y' = x sin + y cos
    return NestingFlipMatrix(p.flip) * rotate * QTransform::fromTranslate(p.xMm, p.yMm);
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingMarkerToSheet(const QRectF &marginsRectPx, bool grainlineVertical) -> QTransform
{
    const qreal k = NestingPxPerMm;
    if (grainlineVertical)
    {
        // Portrait. Marker +X (pattern +Y, the length) runs up. Marker +Y (pattern −X, the width) runs left.
        // sheet = (right − k y, bottom − k x). Qt Y points down, so this matrix has a negative determinant.
        return {0, -k, -k, 0, marginsRectPx.right(), marginsRectPx.bottom()};
    }
    // Landscape, the default view. Marker +X runs right. Marker +Y runs up.
    // sheet = (left + k x, bottom − k y)
    return {k, 0, 0, -k, marginsRectPx.left(), marginsRectPx.bottom()};
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingSheetToMarker(const QRectF &marginsRectPx, bool grainlineVertical) -> QTransform
{
    return NestingMarkerToSheet(marginsRectPx, grainlineVertical).inverted();
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingPieceMatrix(const QVector<QPointF> &contourPx,
                        const QTransform &base,
                        const VNestingPlacement &p,
                        const QRectF &marginsRectPx,
                        bool grainlineVertical) -> QTransform
{
    const QPointF origin = ProfileOrigin(contourPx, base);
    return base * NestingLocalToStrip() * QTransform::fromTranslate(-origin.x(), -origin.y()) * NestingPlacementMatrix(p)
           * NestingMarkerToSheet(marginsRectPx, grainlineVertical);
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingFlipped(bool baseFlipped, bool mirrored) -> bool
{
    return baseFlipped != mirrored;
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingFlipMirrors(const QString &flip) -> bool
{
    // x and y reverse handedness. both is two mirrors, so the handedness is unchanged.
    return flip == QLatin1String("x") || flip == QLatin1String("y");
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingAroundBuffer(const QVector<QPointF> &cutMm, qreal distanceMm) -> QVector<QPointF>
{
    const QVector<QPointF> ring = CleanRing(cutMm);
    if (distanceMm <= 0 || ring.size() < 3)
    {
        return ring;
    }
    const bool ccw = SignedArea(ring) > 0;
    const int n = static_cast<int>(ring.size());
    QVector<QPointF> normals(n);
    QVector<QPointF> directions(n);
    for (int i = 0; i < n; ++i)
    {
        const QPointF direction = UnitVector(ring.at((i + 1) % n) - ring.at(i));
        directions[i] = direction;
        const QPointF right = RightNormal(direction);
        normals[i] = ccw ? right : QPointF(-right.x(), -right.y());
    }

    QVector<QPointF> out;
    out.reserve(n * 4);
    for (int i = 0; i < n; ++i)
    {
        const int prev = (i + n - 1) % n;
        const QPointF nIn = normals.at(prev);
        const QPointF nOut = normals.at(i);
        const QPointF dIn = directions.at(prev);
        const QPointF dOut = directions.at(i);
        const qreal cross = dIn.x() * dOut.y() - dIn.y() * dOut.x();
        const bool convex = ccw ? cross > kAlmost : cross < -kAlmost;
        const bool concave = ccw ? cross < -kAlmost : cross > kAlmost;
        if (convex)
        {
            const qreal a0 = std::atan2(nIn.y(), nIn.x());
            qreal sweep = std::atan2(nOut.y(), nOut.x()) - a0;
            while (sweep <= 0)
            {
                sweep += 2 * M_PI;
            }
            if (sweep > 2 * M_PI - 1e-4)
            {
                sweep = 0;
            }
            const int steps = qMax(1, qCeil(qRadiansToDegrees(sweep) / 15.0));
            for (int step = 0; step <= steps; ++step)
            {
                const qreal a = a0 + sweep * (static_cast<qreal>(step) / steps);
                out.append(ring.at(i) + QPointF(qCos(a), qSin(a)) * distanceMm);
            }
        }
        else if (concave)
        {
            const QPointF p = ring.at(i) + nIn * distanceMm;
            const QPointF q = ring.at(i) + nOut * distanceMm;
            const qreal denom = dIn.x() * dOut.y() - dIn.y() * dOut.x();
            if (qAbs(denom) < kAlmost)
            {
                out.append(ring.at(i) + nOut * distanceMm);
            }
            else
            {
                const qreal t = ((q.x() - p.x()) * dOut.y() - (q.y() - p.y()) * dOut.x()) / denom;
                out.append(p + dIn * t);
            }
        }
        else
        {
            out.append(ring.at(i) + nOut * distanceMm);
        }
    }
    return CleanRing(out);
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingRequest(const QVector<VNestingPieceInput> &pieces,
                    qreal usableWidthMm,
                    qreal gapMm,
                    const VNestingOptions &options,
                    const QRectF &marginsRectPx,
                    bool grainlineVertical,
                    QVector<int> &skipped,
                    QStringList *instanceOf) -> QJsonObject
{
    if (instanceOf != nullptr)
    {
        instanceOf->fill(QString(), static_cast<int>(pieces.size()));
    }
    QJsonArray pieceRecords;
    QJsonArray instances;
    const qreal bufferMm = qMax(0.0, gapMm) / 2; // the gap is the distance between pieces; each side carries half
    for (int index = 0; index < pieces.size(); ++index)
    {
        const VNestingPieceInput &piece = pieces.at(index);
        const QRectF box = QPolygonF(piece.contourPx).boundingRect();
        if (piece.contourPx.size() < 3 || qFuzzyIsNull(box.height()) || qFuzzyIsNull(box.width()))
        {
            skipped.append(index);
            continue;
        }
        const QVector<QPointF> cut = ProfilePoints(piece.contourPx, piece.base);
        if (cut.size() < 3)
        {
            skipped.append(index);
            continue;
        }
        const NestingRotationFlip allowance
            = NestingMapRotationAndFlip(options.rotations, piece.forbidFlipping, piece.forceFlipping);
        const QString pieceId = piece.id.isEmpty() ? QStringLiteral("p%1").arg(index)
                                                    : QStringLiteral("%1#%2").arg(piece.id).arg(index);
        QJsonObject record{{QStringLiteral("id"), pieceId},
                           {QStringLiteral("collision"), JsonRing(NestingAroundBuffer(cut, bufferMm))},
                           {QStringLiteral("cut"), JsonRing(cut)},
                           {QStringLiteral("holes"), QJsonArray()},
                           {QStringLiteral("rotation"), allowance.rotation},
                           {QStringLiteral("tiltCw"), 0},
                           {QStringLiteral("tiltCcw"), 0},
                           {QStringLiteral("flipAllowance"), allowance.flipAllowance},
                           {QStringLiteral("fold"), QStringLiteral("none")},
                           {QStringLiteral("match"), QJsonArray()}};
        pieceRecords.append(record);

        const int copies = qMax(1, piece.quantity);
        QString firstInstance;
        for (int copy = 0; copy < copies; ++copy)
        {
            // pieceId + sizeName + side + index. The size is the one geometry already evaluated. side is authored.
            const QString instanceId = QStringLiteral("%1//authored/%2").arg(pieceId).arg(copy);
            if (copy == 0)
            {
                firstInstance = instanceId;
            }
            QJsonObject instance{{QStringLiteral("id"), instanceId},
                                 {QStringLiteral("pieceId"), pieceId},
                                 {QStringLiteral("sizeName"), QString()},
                                 {QStringLiteral("side"), QStringLiteral("authored")},
                                 {QStringLiteral("index"), copy}};
            if (copy == 0 && not piece.placedSheetPx.isEmpty())
            {
                QVector<QPointF> marker;
                marker.reserve(piece.placedSheetPx.size());
                const QTransform toMarker = NestingSheetToMarker(marginsRectPx, grainlineVertical);
                for (const QPointF &point : piece.placedSheetPx)
                {
                    marker.append(toMarker.map(point));
                }
                // The placed ring and the profile ring are the same vertices. A lock is that pose.
                const QVector<QPointF> placed = CleanRing(marker);
                const QVector<QPointF> profile = cut;
                if (placed.size() == profile.size())
                {
                    VNestingPlacement lock = RecoverLock(profile, placed);
                    if (not lock.flip.isEmpty())
                    {
                        lock.instanceId = instanceId;
                        instance.insert(QStringLiteral("lock"), PlacementObject(lock));
                    }
                    else
                    {
                        instance.insert(QStringLiteral("lock"), QJsonValue::Null);
                    }
                }
                else
                {
                    instance.insert(QStringLiteral("lock"), QJsonValue::Null);
                }
            }
            else
            {
                instance.insert(QStringLiteral("lock"), QJsonValue::Null);
            }
            instances.append(instance);
        }
        if (instanceOf != nullptr)
        {
            (*instanceOf)[index] = firstInstance;
        }
    }

    const QString lengthMode = options.fixedLength ? QStringLiteral("fixedFail") : QStringLiteral("open");
    QJsonObject bin{{QStringLiteral("usableWidthMm"), usableWidthMm},
                    {QStringLiteral("lengthMode"), lengthMode},
                    {QStringLiteral("layoutMode"), QStringLiteral("single")},
                    {QStringLiteral("obstacles"), QJsonArray()}};
    if (options.fixedLength)
    {
        bin.insert(QStringLiteral("usableLengthMm"), options.usableLengthMm);
    }
    // Subset of a version 1 job: no priority (today's 0 means last; the contract's 0 means first), no match,
    // no flaws, no fold lines, no nestable holes. Groups are not a constraint. The gap is in the polygons.
    return {{QStringLiteral("version"), 1},
            {QStringLiteral("geometryToleranceMm"), NestingGeometryToleranceMm},
            {QStringLiteral("timeBudgetS"), options.timeBudgetS},
            {QStringLiteral("honorPlacement"), false},
            {QStringLiteral("matchMode"), QStringLiteral("off")},
            {QStringLiteral("bin"), bin},
            {QStringLiteral("repeatDatum"), QJsonObject{{QStringLiteral("xMm"), 0}, {QStringLiteral("yMm"), 0}}},
            {QStringLiteral("pieces"), pieceRecords},
            {QStringLiteral("instances"), instances}};
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingRequestArea(const QJsonObject &job) -> qreal
{
    QHash<QString, qreal> areaOf;
    const QJsonArray pieces = job.value(QStringLiteral("pieces")).toArray();
    for (const QJsonValue value : pieces)
    {
        const QJsonObject piece = value.toObject();
        areaOf.insert(piece.value(QStringLiteral("id")).toString(), PolygonArea(piece.value(QStringLiteral("cut")).toArray()));
    }
    qreal total = 0;
    const QJsonArray instances = job.value(QStringLiteral("instances")).toArray();
    for (const QJsonValue value : instances)
    {
        total += areaOf.value(value.toObject().value(QStringLiteral("pieceId")).toString());
    }
    return total;
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingContour(const VLayoutPiece &piece, bool togetherWithNotches) -> QVector<QPointF>
{
    QVector<QPointF> contour;
    CastTo(piece.IsSeamAllowance() && not piece.IsSeamAllowanceBuiltIn()
               ? piece.GetFullSeamAllowancePoints(togetherWithNotches)
               : piece.GetFullContourPoints(togetherWithNotches),
           contour);
    return contour;
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingGrainlineAlignment(const VLayoutPiece &piece, const QTransform &base) -> QTransform
{
    if (not piece.IsGrainlineEnabled())
    {
        return {};
    }
    const QLineF grainline = base.map(piece.GetGrainlineMainLine());
    if (grainline.isNull())
    {
        return {};
    }
    // Qt +Y (down the screen) is the axis NestingLocalToStrip writes as profile +X, the length. The on-screen
    // grainline, toward the end of the roll, is a different aim: VPPiece::RotateToGrainline. A one-way arrow
    // fixes the direction. Two opposite arrows take the smaller turn.
    const QLineF fabric(grainline.p1(), grainline.p1() + QPointF(0, 100));
    const VPieceGrainline arrows = piece.GetGrainline();
    QVector<qreal> angles;
    auto Consider = [&angles, &grainline, &fabric](qreal offset)
    {
        QLineF arrow = grainline;
        arrow.setAngle(arrow.angle() + offset);
        const qreal angle = arrow.angleTo(fabric);
        angles.append(angle);
        angles.append(-(360. - angle));
    };
    if (arrows.IsArrowUpEnabled())
    {
        Consider(0);
    }
    if (arrows.IsArrowDownEnabled())
    {
        Consider(180);
    }
    if (arrows.IsArrowLeftEnabled())
    {
        Consider(90);
    }
    if (arrows.IsArrowRightEnabled())
    {
        Consider(-90);
    }
    qreal degrees = 0;
    if (not angles.isEmpty())
    {
        degrees = angles.constFirst();
        for (const qreal a : std::as_const(angles))
        {
            if (qAbs(a) < qAbs(degrees))
            {
                degrees = a;
            }
        }
    }
    QTransform m;
    m.rotate(-degrees); // same sign convention as VLayoutPiece::Rotate
    return m;
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingLayoutBase(const VLayoutPiece &piece) -> QTransform
{
    const QTransform base = piece.IsForceFlipping() ? QTransform::fromScale(-1, 1) : QTransform();
    return base * NestingGrainlineAlignment(piece, base);
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingReadResult(const QJsonObject &result) -> VNestingRead
{
    VNestingRead read;
    read.status = result.value(QStringLiteral("status")).toString();
    const QJsonArray bins = result.value(QStringLiteral("bins")).toArray();
    for (const QJsonValue value : bins)
    {
        const QJsonObject bin = value.toObject();
        if (qFuzzyIsNull(read.lengthUsedMm))
        {
            read.lengthUsedMm = bin.value(QStringLiteral("lengthUsedMm")).toDouble();
        }
        const QHash<QString, VNestingPlacement> own = ReadBinPlacements(bin);
        for (auto it = own.cbegin(); it != own.cend(); ++it)
        {
            read.placements.insert(it.key(), it.value());
        }
    }
    const QJsonArray unplaced = result.value(QStringLiteral("unplaced")).toArray();
    for (const QJsonValue value : unplaced)
    {
        const QString id = value.toString();
        if (not id.isEmpty())
        {
            read.unplaced.append(id);
        }
    }
    read.hasReport = result.contains(QStringLiteral("report"));
    if (read.hasReport && result.value(QStringLiteral("report")).isObject())
    {
        read.report = result.value(QStringLiteral("report")).toObject();
    }
    read.complete = read.status == QLatin1String("complete") && read.unplaced.isEmpty();
    return read;
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingPlanComplete(const QJsonObject &result, const QStringList &instanceIds) -> bool
{
    const VNestingRead read = NestingReadResult(result);
    if (not read.complete)
    {
        return false;
    }
    for (const QString &id : instanceIds)
    {
        if (id.isEmpty() || not read.placements.contains(id))
        {
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingPlacements(const QJsonObject &result) -> QHash<QString, VNestingPlacement>
{
    return NestingReadResult(result).placements;
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingSummaryText(const QJsonObject &result, const QJsonObject &job) -> QString
{
    const QLocale locale;
    const VNestingRead read = NestingReadResult(result);
    bool finished = read.status == QLatin1String("complete") && read.unplaced.isEmpty();
    if (not job.isEmpty())
    {
        for (const QJsonValue value : job.value(QStringLiteral("instances")).toArray())
        {
            if (not read.placements.contains(value.toObject().value(QStringLiteral("id")).toString()))
            {
                finished = false;
            }
        }
    }
    QStringList lines;
    if (not finished)
    {
        lines << QObject::tr("The lay is not finished.");
    }
    else if (read.status == QLatin1String("complete"))
    {
        lines << QObject::tr("Every piece is placed.");
    }
    if (read.lengthUsedMm > 0)
    {
        const qreal width = job.value(QStringLiteral("bin")).toObject().value(QStringLiteral("usableWidthMm")).toDouble();
        const qreal area = NestingRequestArea(job);
        if (width > 0 && area > 0)
        {
            // Efficiency uses placed cut area. Unplaced instances are still in the job area, so subtract them.
            qreal unplacedArea = 0;
            QHash<QString, qreal> areaOf;
            for (const QJsonValue value : job.value(QStringLiteral("pieces")).toArray())
            {
                const QJsonObject piece = value.toObject();
                areaOf.insert(piece.value(QStringLiteral("id")).toString(),
                              PolygonArea(piece.value(QStringLiteral("cut")).toArray()));
            }
            for (const QJsonValue value : job.value(QStringLiteral("instances")).toArray())
            {
                const QJsonObject instance = value.toObject();
                const QString id = instance.value(QStringLiteral("id")).toString();
                if (not read.placements.contains(id))
                {
                    unplacedArea += areaOf.value(instance.value(QStringLiteral("pieceId")).toString());
                }
            }
            const qreal placed = qMax(0.0, area - unplacedArea);
            const qreal efficiency = placed / (width * read.lengthUsedMm) * 100;
            lines << QObject::tr("Used length %1 m, efficiency %2 %.")
                         .arg(locale.toString(read.lengthUsedMm / 1000, 'f', 2), locale.toString(efficiency, 'f', 1));
        }
        else
        {
            lines << QObject::tr("Used length %1 m.").arg(locale.toString(read.lengthUsedMm / 1000, 'f', 2));
        }
    }
    QStringList missing = read.unplaced;
    if (not job.isEmpty())
    {
        for (const QJsonValue value : job.value(QStringLiteral("instances")).toArray())
        {
            const QString id = value.toObject().value(QStringLiteral("id")).toString();
            if (not id.isEmpty() && not read.placements.contains(id) && not missing.contains(id))
            {
                missing.append(id);
            }
        }
    }
    if (not missing.isEmpty())
    {
        lines << QObject::tr("Unplaced: %1").arg(missing.join(QStringLiteral(", ")));
    }
    if (read.hasReport && not read.report.isEmpty())
    {
        const QString reason = read.report.value(QStringLiteral("stopReason")).toString();
        if (not reason.isEmpty())
        {
            lines << QObject::tr("Engine report: %1").arg(reason);
        }
        for (auto it = read.report.constBegin(); it != read.report.constEnd(); ++it)
        {
            if (it.key() == QLatin1String("stopReason"))
            {
                continue;
            }
            const QString text = it.value().isString() ? it.value().toString()
                                 : it.value().isDouble() ? locale.toString(it.value().toDouble())
                                                         : QString();
            if (not text.isEmpty())
            {
                lines << QStringLiteral("%1: %2").arg(it.key(), text);
            }
        }
    }
    if (lines.isEmpty())
    {
        return read.status.isEmpty() ? QObject::tr("The engine returned no lay.") : read.status;
    }
    return lines.join('\n');
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingResultPaper(const QVector<VLayoutPiece> &sent,
                        const QStringList &instanceIds,
                        const QHash<QString, VNestingPlacement> &placements,
                        qreal fabricWidthPx,
                        qreal markerLengthPx,
                        qreal layoutWidthPx,
                        bool togetherWithNotches) -> VLayoutPaper
{
    const QRectF sheet(0, 0, fabricWidthPx, markerLengthPx);
    QVector<VLayoutPiece> details;
    details.reserve(placements.size());
    const auto n = qMin(sent.size(), instanceIds.size());
    for (vsizetype i = 0; i < n; ++i)
    {
        const auto found = placements.constFind(instanceIds.at(i));
        if (found == placements.cend())
        {
            continue;
        }
        VLayoutPiece copy = sent.at(i);
        copy.SetLayoutWidth(layoutWidthPx);
        copy.SetLayoutAllowancePoints(togetherWithNotches); // from the unmapped contour, so before SetMatrix
        const QVector<QPointF> contour = NestingContour(copy, togetherWithNotches);
        copy.SetMatrix(NestingPieceMatrix(contour, NestingLayoutBase(copy), found.value(), sheet, true));
        copy.SetVerticallyFlipped(NestingFlipped(copy.IsForceFlipping(), NestingFlipMirrors(found.value().flip)));
        details.append(copy);
    }
    VLayoutPaper paper(qCeil(markerLengthPx), qCeil(fabricWidthPx), layoutWidthPx);
    paper.SetDetails(details);
    return paper;
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingResultPapers(const QVector<VLayoutPiece> &sent,
                         const QStringList &instanceIds,
                         const QJsonObject &result,
                         qreal fabricWidthPx,
                         qreal layoutWidthPx,
                         bool togetherWithNotches) -> QVector<VLayoutPaper>
{
    const QJsonArray bins = result.value(QStringLiteral("bins")).toArray();
    QVector<VLayoutPaper> papers;
    papers.reserve(bins.size());
    for (const QJsonValue value : bins)
    {
        const QJsonObject bin = value.toObject();
        const qreal lengthPx = bin.value(QStringLiteral("lengthUsedMm")).toDouble() * NestingPxPerMm;
        papers.append(NestingResultPaper(sent, instanceIds, ReadBinPlacements(bin), fabricWidthPx, lengthPx,
                                         layoutWidthPx, togetherWithNotches));
    }
    return papers;
}
