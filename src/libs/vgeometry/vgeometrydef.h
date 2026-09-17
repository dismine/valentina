/************************************************************************
 **
 **  @file   vgeometrydef.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 5, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#ifndef VGEOMETRYDEF_H
#define VGEOMETRYDEF_H

#include <QPolygonF>
#include <QTransform>
#include <QVector>
#include <algorithm>
#include <iterator>

#include "../vmisc/def.h"

enum class GOType : qint8
{
    Point,
    Arc,
    EllipticalArc,
    Spline,
    SplinePath,
    CubicBezier,
    CubicBezierPath,
    PlaceLabel,
    Unknown
};

Q_DECLARE_METATYPE(GOType) // NOLINT

enum class SplinePointPosition : qint8
{
    FirstPoint,
    LastPoint
};

// Keep synchronized with XSD schema
enum class PlaceLabelType : quint8
{
    Segment = 0,
    Rectangle = 1,
    Cross = 2,
    Tshaped = 3,
    Doubletree = 4,
    Corner = 5,
    Triangle = 6,
    Hshaped = 7,
    Button = 8,
    Circle = 9
};

struct VLayoutPassmark
{
    QVector<QLineF> lines{};
    PassmarkLineType type{PassmarkLineType::OneLine};
    QLineF baseLine{};
    bool isBuiltIn{false};
    bool isClockwiseOpening{false};
    QString label{};
    bool notMirrored{false};

    friend auto operator<<(QDataStream &dataStream, const VLayoutPassmark &data) -> QDataStream &;
    friend auto operator>>(QDataStream &dataStream, VLayoutPassmark &data) -> QDataStream &;

    auto toJson() const -> QJsonObject;

private:
    static const quint32 streamHeader;
    static const quint16 classVersion;
};
Q_DECLARE_METATYPE(VLayoutPassmark) // NOLINT

constexpr qreal accuracyPointOnLine = MmToPixel(0.1555);

Q_REQUIRED_RESULT inline auto VFuzzyComparePoints(const QPointF &p1, const QPointF &p2,
                                                  qreal accuracy = accuracyPointOnLine) -> bool;
inline auto VFuzzyComparePoints(const QPointF &p1, const QPointF &p2, qreal accuracy) -> bool
{
    return QLineF(p1, p2).length() <= accuracy;
}

Q_REQUIRED_RESULT inline auto VFuzzyOnAxis(qreal v1, qreal v2, qreal accuracy = accuracyPointOnLine) -> bool;
inline auto VFuzzyOnAxis(qreal v1, qreal v2, qreal accuracy) -> bool
{
    return qAbs(v1 - v2) <= accuracy;
}

template <class T> inline void Swap(T &line)
{
    line = T(line.p2(), line.p1());
}

auto SingleParallelPoint(const QPointF &p1, const QPointF &p2, qreal angle, qreal width) -> QPointF;
auto SimpleParallelLine(const QPointF &p1, const QPointF &p2, qreal width) -> QLineF;

auto IsPointOnLineviaPDP(const QPointF &t, const QPointF &p1, const QPointF &p2, qreal accuracy = accuracyPointOnLine)
    -> bool;
auto IsLineSegmentOnLineSegment(const QLineF &seg1, const QLineF &seg2, qreal accuracy = accuracyPointOnLine) -> bool;
auto IsPointOnLineSegment(const QPointF &t, const QPointF &p1, const QPointF &p2, qreal accuracy = accuracyPointOnLine)
    -> bool;

//---------------------------------------------------------------------------------------------------------------------
template<class T>
inline auto IntersectionPoint(QPointF crosPoint, const T &l1p1, const T &l1p2, const T &l2p1, const T &l2p2) -> T
{
    T point(crosPoint);

    if ((l1p1.CurvePoint() && l1p2.CurvePoint()) || (l2p1.CurvePoint() && l2p2.CurvePoint())
        || (l1p1.CurvePoint() && l2p2.CurvePoint()))
    {
        point.SetCurvePoint(true);
    }

    if ((l1p1.TurnPoint() && l1p2.TurnPoint()) || (l2p1.TurnPoint() && l2p2.TurnPoint())
        || (l1p1.TurnPoint() && l2p2.TurnPoint()))
    {
        point.SetTurnPoint(true);
    }

    return point;
}

//---------------------------------------------------------------------------------------------------------------------
template<>
inline auto IntersectionPoint<QPointF>(QPointF crosPoint,
                                       const QPointF & /*unused*/,
                                       const QPointF & /*unused*/,
                                       const QPointF & /*unused*/,
                                       const QPointF & /*unused*/) -> QPointF
{
    return crosPoint;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CheckLoops seek and delete loops in equidistant.
 * @param points vector of points of equidistant.
 * @return vector of points of equidistant.
 */
template<class T>
inline auto CheckLoops(QVector<T> points) -> QVector<T>
{
    //    DumpVector(points, QStringLiteral("input.json.XXXXXX")); // Uncomment for dumping test data

    /*If we got less than 4 points no need seek loops.*/
    if (points.size() < 4)
    {
        return points;
    }

    bool loopFound = false;
    const int maxLoops = 10000; // limit number of loops to be removed

    for (qint32 i = 0; i < maxLoops; ++i)
    {
        points = CheckLoop(points, loopFound);
        if (not loopFound)
        {
            break;
        }
    }

    //    DumpVector(ekvPoints, QStringLiteral("output.json.XXXXXX")); // Uncomment for dumping test data
    return points;
}

//---------------------------------------------------------------------------------------------------------------------
template<class T>
inline auto CheckLoop(const QVector<T> &points, bool &loopFound) -> QVector<T>
{
    loopFound = false;

    const bool pathClosed = VFuzzyComparePoints(points.constFirst(), points.constLast());
    const auto size = points.size();

    // Shoelace prefix sums, so that the area of any loop costs O(1) to evaluate.
    QVector<qreal> chain(size, 0);
    for (auto k = decltype(size){1}; k < size; ++k)
    {
        const QPointF &prev = points.at(k - 1);
        const QPointF &cur = points.at(k);
        chain[k] = chain.at(k - 1) + (prev.x() * cur.y() - cur.x() * prev.y());
    }

    auto Wedge = [](const QPointF &u, const QPointF &v) { return u.x() * v.y() - v.x() * u.y(); };

    // Cutting off the first loop met in scan order makes the result depend on which node happens to be first in
    // the path: started at another index the very same outline hits a crossing that spans almost all of it, and
    // everything in between is thrown away. A parasitic loop is a small one, so weigh every crossing and cut off
    // the smallest loop only. The rest, if any, is handled by the next pass.
    auto bestI = decltype(size){-1};
    auto bestJ = decltype(size){-1};
    qreal bestArea = 0;
    T bestCross;
    bool bestParallel = false;

    for (auto i = decltype(size){0}; i < size - 3; ++i)
    {
        const QLineF line1(points.at(i), points.at(i + 1));
        const int limit = pathClosed && i == 0 ? 2 : 1;

        for (auto j = i + 2; j < size - limit; ++j)
        {
            const QLineF line2(points.at(j), points.at(j + 1));

            QPointF crosPoint;
            bool parallel = false;

            const QLineF::IntersectType intersect = line1.intersects(line2, &crosPoint);

            // Collinear segments have a zero denominator only in exact arithmetic. Rounding leaves it barely
            // non-zero instead, and QLineF then calls them a bounded intersection and hands back a point that
            // lies on neither segment. Taking that point for a crossing cuts away everything between the two
            // segments, so make sure it really is on both of them.
            const bool collinear = intersect == QLineF::BoundedIntersection
                                   && (not IsPointOnLineSegment(crosPoint, line1.p1(), line1.p2())
                                       || not IsPointOnLineSegment(crosPoint, line2.p1(), line2.p2()));

            if (intersect == QLineF::NoIntersection || collinear)
            { // According to the documentation QLineF::NoIntersection indicates that the lines do not intersect;
                // i.e. they are parallel. But parallel also mean they can be on the same line.
                // Method IsLineSegmentOnLineSegment will check it.
                if (not IsLineSegmentOnLineSegment(line1, line2))
                {
                    continue;
                }
                parallel = true;
                crosPoint = points.at(j + 1);
            }
            else if (intersect != QLineF::BoundedIntersection)
            {
                continue;
            }

            // The loop is: the crossing, points i+1...j, back to the crossing.
            const qreal loopArea = qAbs(Wedge(crosPoint, points.at(i + 1)) + (chain.at(j) - chain.at(i + 1))
                                        + Wedge(points.at(j), crosPoint));

            if (bestI == -1 || loopArea < bestArea)
            {
                bestI = i;
                bestJ = j;
                bestArea = loopArea;
                bestCross = IntersectionPoint(crosPoint, points.at(i), points.at(i + 1), points.at(j), points.at(j + 1));
                bestParallel = parallel;
            }
        }
    }

    if (bestI == -1)
    {
        return points;
    }

    loopFound = true;

    QVector<T> ekvPoints;
    ekvPoints.reserve(size);
    std::copy(points.begin(), points.begin() + bestI + 1, std::back_inserter(ekvPoints));
    if (not bestParallel)
    { // For a parallel hit the crossing is points.at(bestJ + 1) itself and gets copied below anyway.
        ekvPoints.append(bestCross);
    }
    std::copy(points.begin() + bestJ + 1, points.end(), std::back_inserter(ekvPoints));
    return ekvPoints;
}

#endif // VGEOMETRYDEF_H
