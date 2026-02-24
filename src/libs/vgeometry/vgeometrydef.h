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

    QVector<T> ekvPoints;
    ekvPoints.reserve(points.size());

    for (qint32 i = 0; i < points.size(); ++i)
    {
        /*Last three points no need to check.*/
        /*Triangle can not contain a loop*/
        if (loopFound || i > points.size() - 4)
        {
            ekvPoints.append(points.at(i));
            continue;
        }

        enum LoopIntersectType
        {
            NoIntersection,
            BoundedIntersection,
            ParallelIntersection
        };

        QPointF crosPoint;
        LoopIntersectType status = NoIntersection;
        const QLineF line1(points.at(i), points.at(i + 1));

        const int limit = pathClosed && i == 0 ? 2 : 1;
        qint32 j;
        for (j = i + 2; j < points.size() - limit; ++j)
        {
            QLineF line2(points.at(j), points.at(j + 1));

            const QLineF::IntersectType intersect = line1.intersects(line2, &crosPoint);
            if (intersect == QLineF::NoIntersection)
            { // According to the documentation QLineF::NoIntersection indicates that the lines do not intersect;
                // i.e. they are parallel. But parallel also mean they can be on the same line.
                // Method IsLineSegmentOnLineSegment will check it.
                if (IsLineSegmentOnLineSegment(line1, line2))
                { // Now we really sure that segments are on the same line and have real intersections.
                    status = ParallelIntersection;
                    break;
                }
            }
            else if (intersect == QLineF::BoundedIntersection)
            {
                status = BoundedIntersection;
                break;
            }
        }

        switch (status)
        {
            case ParallelIntersection:
                /*We have found a loop.*/
                ekvPoints.append(points.at(i));
                ekvPoints.append(points.at(j + 1));
                i = j + 1; // Skip a loop
                loopFound = true;
                break;
            case BoundedIntersection:
                ekvPoints.append(points.at(i));
                ekvPoints.append(
                    IntersectionPoint(crosPoint, points.at(i), points.at(i + 1), points.at(j), points.at(j + 1)));
                i = j;
                loopFound = true;
                break;
            case NoIntersection:
                /*We have not found loop.*/
                ekvPoints.append(points.at(i));
                break;
            default:
                break;
        }
    }
    return ekvPoints;
}

#endif // VGEOMETRYDEF_H
