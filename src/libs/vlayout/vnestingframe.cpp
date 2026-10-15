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

#include <QHash>
#include <QJsonDocument>
#include <QPolygonF>
#include <QtMath>

// Qt's QTransform(m11, m12, m21, m22, dx, dy) maps (x, y) to (m11 x + m21 y + dx, m12 x + m22 y + dy) and
// a * b applies a first, then b.

//---------------------------------------------------------------------------------------------------------------------
auto NestingLocalToStrip() -> QTransform
{
    // (x, y) px -> (y, -x) / k mm: local +y (grainline) becomes strip +x; a rotation, determinant +1
    const qreal s = 1 / NestingPxPerMm;
    return {0, -s, s, 0, 0, 0};
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingPlacementMatrix(const VNestingPlacement &p) -> QTransform
{
    const qreal th = qDegreesToRadians(p.rotationDeg);
    const qreal c = qCos(th);
    const qreal s = qSin(th);
    const QTransform mirror = p.mirrored ? QTransform(1, 0, 0, -1, 0, 0) : QTransform();
    const QTransform rotate(c, s, -s, c, 0, 0); // x' = x cos - y sin, y' = x sin + y cos
    return mirror * rotate * QTransform::fromTranslate(p.x, p.y);
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingStripToSheet(const QRectF &marginsRectPx, bool grainlineVertical) -> QTransform
{
    const qreal k = NestingPxPerMm;
    if (grainlineVertical)
    {
        // strip x runs down the sheet, strip y runs from the right margin leftwards: the inverse rotation of
        // NestingLocalToStrip scaled by k, so (local -> strip -> sheet) is a pure translation
        return {0, k, -k, 0, marginsRectPx.right(), marginsRectPx.top()};
    }
    return {k, 0, 0, k, marginsRectPx.left(), marginsRectPx.top()};
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingPieceMatrix(const QTransform &base,
                        const VNestingPlacement &p,
                        const QRectF &marginsRectPx,
                        bool grainlineVertical) -> QTransform
{
    return base * NestingLocalToStrip() * NestingPlacementMatrix(p)
           * NestingStripToSheet(marginsRectPx, grainlineVertical);
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingFlipped(bool baseFlipped, bool mirrored) -> bool
{
    return baseFlipped != mirrored;
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingFixedPolygon(const QVector<QPointF> &sheetPx, const QRectF &marginsRectPx, bool grainlineVertical)
    -> QVector<QPointF>
{
    return NestingStripToSheet(marginsRectPx, grainlineVertical).inverted().map(QPolygonF(sheetPx));
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingPolygon(const QVector<QPointF> &localPx, const QTransform &base) -> QJsonArray
{
    const QTransform m = base * NestingLocalToStrip();
    QJsonArray poly;
    for (const auto &pt : localPx)
    {
        const QPointF s = m.map(pt);
        poly.append(QJsonArray{std::round(s.x() * 1000) / 1000, std::round(s.y() * 1000) / 1000}); // body size
    }
    return poly;
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingRotationsString(NestingRotations rotations) -> QString
{
    switch (rotations)
    {
        case NestingRotations::ZeroOnly:
            return QStringLiteral("0");
        case NestingRotations::Any:
            return QStringLiteral("continuous");
        case NestingRotations::ZeroAnd180:
        default:
            return QStringLiteral("0,180");
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingSpeedString(NestingSpeed speed) -> QString
{
    switch (speed)
    {
        case NestingSpeed::Fast:
            return QStringLiteral("fast");
        case NestingSpeed::Ultra:
            return QStringLiteral("ultra");
        case NestingSpeed::Normal:
        default:
            return QStringLiteral("normal");
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingRequestArea(const QJsonObject &request) -> qreal
{
    qreal total = 0;
    const QJsonArray pieces = request.value(QLatin1String("pieces")).toArray();
    for (const auto item : pieces) // NOLINT(performance-for-range-copy)
    {
        const QJsonObject piece = item.toObject();
        QPolygonF polygon;
        const QJsonArray points = piece.value(QLatin1String("polygon")).toArray();
        for (const auto pt : points) // NOLINT(performance-for-range-copy)
        {
            polygon << QPointF(pt.toArray().at(0).toDouble(), pt.toArray().at(1).toDouble());
        }
        // SumTrapezoids is the shoelace sum, twice the signed area
        total += qAbs(VAbstractPiece::SumTrapezoids(polygon)) / 2 * piece.value(QLatin1String("quantity")).toInt(1);
    }
    return total;
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingRequest(const QVector<VNestingPieceInput> &pieces,
                    qreal fabricWidthMm,
                    qreal gapMm,
                    const VNestingOptions &options,
                    QVector<int> &skipped,
                    QVector<int> *itemOf) -> QJsonObject
{
    const NestingRotations rotations = options.rotations;
    const QString rotationsText = NestingRotationsString(rotations);
    QJsonArray items;
    QHash<QString, int> merged; // identity of a free piece -> its item
    if (itemOf != nullptr)
    {
        itemOf->fill(-1, static_cast<int>(pieces.size()));
    }
    auto record = [itemOf](vsizetype index, int item)
    {
        if (itemOf != nullptr)
        {
            (*itemOf)[static_cast<int>(index)] = item;
        }
    };
    for (vsizetype index = 0; index < pieces.size(); ++index)
    {
        const VNestingPieceInput &piece = pieces.at(index);
        if (not piece.fixedMm.isEmpty())
        {
            // spec 21.4: already placed, the service nests the free pieces around it
            QJsonArray poly;
            for (const auto &pt : piece.fixedMm)
            {
                poly.append(QJsonArray{pt.x(), pt.y()});
            }
            QJsonObject fixed{{"id", static_cast<int>(items.size())}, {"name", piece.name}, {"polygon", poly},
                              {"quantity", 1},  {"mirror", false},     {"fixed", true}};
            if (not piece.group.isEmpty())
            {
                fixed.insert(QStringLiteral("group"), piece.group);
            }
            record(index, static_cast<int>(items.size()));
            items.append(fixed);
            continue;
        }
        const QRectF box = QPolygonF(piece.contourPx).boundingRect();
        if (piece.contourPx.size() < 3 || qFuzzyIsNull(box.height()) || qFuzzyIsNull(box.width()))
        {
            skipped.append(static_cast<int>(index));
            continue;
        }
        // base already carries Puzzle's forced flip (ClearTransformations() flips force-flipped pieces), so a
        // force-flipped piece is sent already mirrored and the service must not mirror it again
        const QJsonArray polygon = NestingPolygon(piece.contourPx, piece.base);
        const QString identity = piece.name + '\n' + piece.group + '\n' + QString::number(piece.forbidFlipping) +
                                 QString::number(piece.forceFlipping) + '\n' +
                                 QString::fromUtf8(QJsonDocument(polygon).toJson(QJsonDocument::Compact));
        if (const auto twin = merged.constFind(identity); twin != merged.constEnd())
        {
            QJsonObject item = items.at(twin.value()).toObject();
            item.insert(QStringLiteral("quantity"), item.value(QLatin1String("quantity")).toInt() + piece.quantity);
            items.replace(twin.value(), item);
            record(index, twin.value());
            continue;
        }
        merged.insert(identity, static_cast<int>(items.size()));
        record(index, static_cast<int>(items.size()));
        QJsonObject item{{"id", static_cast<int>(items.size())},
                         {"name", piece.name},
                         {"polygon", polygon},
                         {"quantity", piece.quantity},
                         {"mirror", not piece.forbidFlipping && not piece.forceFlipping}};
        if (rotations == NestingRotations::Any)
        {
            item.insert(QStringLiteral("rotations"), rotationsText);
        }
        else
        {
            QJsonArray angles;
            const QStringList parts = rotationsText.split(',');
            for (const auto &a : parts)
            {
                angles.append(a.trimmed().toDouble());
            }
            item.insert(QStringLiteral("rotations"), angles);
        }
        if (not piece.group.isEmpty())
        {
            item.insert(QStringLiteral("group"), piece.group);
        }
        items.append(item);
    }
    QJsonObject request{{"name", "puzzle"},
                        {"fabric_width_mm", fabricWidthMm},
                        {"gap_mm", gapMm},
                        {"time_s", 0}, // spec 20: no wall cap, the budget below or the user stops the job
                        {"speed", NestingSpeedString(options.speed)},
                        {"until_converged", options.untilConverged},
                        {"preview", options.preview},
                        {"pieces", items}};
    if (options.budgetTokens > 0)
    {
        request.insert(QStringLiteral("budget_tokens"), options.budgetTokens);
    }
    if (options.groupByGroup)
    {
        request.insert(QStringLiteral("group_by_group"), true);
        if (options.noBackfill)
        {
            request.insert(QStringLiteral("no_backfill"), true);
        }
    }
    if (options.targetLengthMm > 0)
    {
        request.insert(QStringLiteral("target_length_mm"), options.targetLengthMm);
    }
    if (not options.rollsMm.isEmpty())
    {
        QJsonArray rolls;
        for (const qreal length : options.rollsMm)
        {
            rolls.append(length);
        }
        request.insert(QStringLiteral("rolls_mm"), rolls);
        request.insert(QStringLiteral("roll_junction_mm"), options.rollJunctionMm);
    }
    return request;
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
    // the fabric grainline points "up" on screen (QLineF angle 90°), as VPPiece::RotateToGrainline uses for a
    // vertical sheet; a one-way arrow fixes the direction, two opposite arrows take the smaller turn
    const QLineF fabric(grainline.p1(), grainline.p1() + QPointF(0, -100));
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
auto NestingPlanComplete(const QJsonObject &result, const QVector<int> &quantities) -> bool
{
    const QJsonArray placements = result.value(QStringLiteral("placements")).toArray();
    QVector<int> seen(quantities.size(), 0);
    for (const QJsonValue p : placements) // NOLINT(performance-for-range-copy)
    {
        const int id = p.toObject().value(QStringLiteral("piece_id")).toInt(-1);
        if (id < 0 || id >= quantities.size())
        {
            return false;
        }
        ++seen[id];
    }
    return seen == quantities;
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingPlacements(const QJsonObject &result) -> QMultiMap<int, VNestingPlacement>
{
    QMultiMap<int, VNestingPlacement> map;
    const QJsonArray placements = result.value(QStringLiteral("placements")).toArray();
    for (const QJsonValue p : placements) // NOLINT(performance-for-range-copy)
    {
        const QJsonObject o = p.toObject();
        map.insert(o.value(QStringLiteral("piece_id")).toInt(),
                   {o.value(QStringLiteral("x")).toDouble(),
                    o.value(QStringLiteral("y")).toDouble(),
                    o.value(QStringLiteral("rotation_deg")).toDouble(),
                    o.value(QStringLiteral("mirrored")).toBool(),
                    o.value(QStringLiteral("roll")).toInt(0)});
    }
    return map;
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingResultPaper(const QVector<VLayoutPiece> &sent,
                        const QMultiMap<int, VNestingPlacement> &placements,
                        qreal fabricWidthPx,
                        qreal markerLengthPx,
                        qreal layoutWidthPx,
                        bool togetherWithNotches) -> VLayoutPaper
{
    const QRectF sheet(0, 0, fabricWidthPx, markerLengthPx);
    QVector<VLayoutPiece> details;
    details.reserve(placements.size());
    for (auto it = placements.cbegin(); it != placements.cend(); ++it)
    {
        if (it.key() < 0 || it.key() >= sent.size())
        {
            continue;
        }
        VLayoutPiece copy = sent.at(it.key());
        copy.SetLayoutWidth(layoutWidthPx);
        copy.SetLayoutAllowancePoints(togetherWithNotches); // from the unmapped contour, so before SetMatrix
        copy.SetMatrix(NestingPieceMatrix(NestingLayoutBase(copy), it.value(), sheet, true));
        copy.SetVerticallyFlipped(NestingFlipped(copy.IsForceFlipping(), it.value().mirrored));
        details.append(copy);
    }
    VLayoutPaper paper(qCeil(markerLengthPx), qCeil(fabricWidthPx), layoutWidthPx);
    paper.SetDetails(details);
    return paper;
}

//---------------------------------------------------------------------------------------------------------------------
auto NestingResultPapers(const QVector<VLayoutPiece> &sent,
                         const QMultiMap<int, VNestingPlacement> &placements,
                         const QJsonObject &result,
                         qreal fabricWidthPx,
                         qreal layoutWidthPx,
                         bool togetherWithNotches) -> QVector<VLayoutPaper>
{
    const QJsonArray rolls = result.value(QStringLiteral("rolls")).toArray();
    if (rolls.isEmpty())
    {
        const qreal markerLengthPx = result.value(QStringLiteral("marker_length_mm")).toDouble() * NestingPxPerMm;
        return {NestingResultPaper(sent, placements, fabricWidthPx, markerLengthPx, layoutWidthPx, togetherWithNotches)};
    }
    QVector<VLayoutPaper> papers;
    papers.reserve(rolls.size());
    for (const auto r : rolls) // NOLINT(performance-for-range-copy)
    {
        const QJsonObject roll = r.toObject();
        const int index = roll.value(QStringLiteral("index")).toInt();
        const qreal startMm = roll.value(QStringLiteral("start_mm")).toDouble();
        QMultiMap<int, VNestingPlacement> own;
        for (auto it = placements.cbegin(); it != placements.cend(); ++it)
        {
            if (it.value().roll == index)
            {
                VNestingPlacement p = it.value();
                p.x -= startMm; // each roll is its own paper, starting at the roll's physical start
                own.insert(it.key(), p);
            }
        }
        papers.append(NestingResultPaper(sent, own, fabricWidthPx,
                                         roll.value(QStringLiteral("length_mm")).toDouble() * NestingPxPerMm,
                                         layoutWidthPx, togetherWithNotches));
    }
    return papers;
}
