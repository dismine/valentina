/************************************************************************
 **
 **  @file   vgeometrydef.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   1 7, 2019
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2019 Valentina project
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

#include "vgeometrydef.h"

#include "../vmisc/exception/vexception.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonObject>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

const quint32 VLayoutPassmark::streamHeader = 0x943E2759; // CRC-32Q string "VLayoutPassmark"
const quint16 VLayoutPassmark::classVersion = 4;

namespace
{
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief PerpDotProduct Calculates the area of the parallelogram of the three points.
 * This is actually the same as the area of the triangle defined by the three points, multiplied by 2.
 * @return 2 * triangleArea(a,b,c)
 */
auto PerpDotProduct(const QPointF &p1, const QPointF &p2, const QPointF &t) -> double
{
    return (p1.x() - t.x()) * (p2.y() - t.y()) - (p1.y() - t.y()) * (p2.x() - t.x());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetEpsilon solve the floating-point accuraccy problem.
 *
 * There is the floating-point accuraccy problem, so instead of checking against zero, some epsilon value has to be
 * used. Because the size of the pdp value depends on the length of the vectors, no static value can be used. One
 * approach is to compare the pdp/area value to the fraction of another area which also depends on the length of the
 * line e1=(p1, p2), e.g. the minimal area calucalted with PerpDotProduc() if point still not on the line. This distance
 * is controled by variable accuracyPointOnLine
 */
auto GetEpsilon(const QPointF &t, QPointF p1, QPointF p2, qreal accuracy) -> double
{
    QLineF edge1(p1, p2);
    if (QLineF const edge2(p1, t); edge2.length() > edge1.length())
    {
        edge1.setLength(edge2.length());
        p1 = edge1.p1();
        p2 = edge1.p2();
    }

    QLineF line(p1, p2);
    line.setAngle(line.angle() + 90);
    line.setLength(accuracy); // less than accuracy means the same point

    return qAbs(PerpDotProduct(p1, p2, line.p2()));
}
} // namespace

// Friend functions
//---------------------------------------------------------------------------------------------------------------------
auto operator<<(QDataStream &dataStream, const VLayoutPassmark &data) -> QDataStream &
{
    dataStream << VLayoutPassmark::streamHeader << VLayoutPassmark::classVersion;

    dataStream << data.lines << data.type << data.baseLine << data.isBuiltIn;

    // Added in classVersion = 2
    dataStream << data.isClockwiseOpening;

    // Added in classVersion = 3
    dataStream << data.label;

    // Added in classVersion = 4
    dataStream << data.notMirrored;

    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
auto operator>>(QDataStream &dataStream, VLayoutPassmark &data) -> QDataStream &
{
    quint32 actualStreamHeader = 0;
    dataStream >> actualStreamHeader;

    if (actualStreamHeader != VLayoutPassmark::streamHeader)
    {
        QString const message =
            QCoreApplication::tr("VLayoutPassmark prefix mismatch error: actualStreamHeader = 0x%1 and "
                                 "streamHeader = 0x%2")
                .arg(actualStreamHeader, 8, 0x10, '0'_L1)
                .arg(VLayoutPassmark::streamHeader, 8, 0x10, '0'_L1);
        throw VException(message);
    }

    quint16 actualClassVersion = 0;
    dataStream >> actualClassVersion;

    if (actualClassVersion > VLayoutPassmark::classVersion)
    {
        QString const message = QCoreApplication::tr("VLayoutPassmark compatibility error: actualClassVersion = %1 and "
                                                     "classVersion = %2")
                                    .arg(actualClassVersion)
                                    .arg(VLayoutPassmark::classVersion);
        throw VException(message);
    }

    dataStream >> data.lines >> data.type >> data.baseLine >> data.isBuiltIn;

    if (actualClassVersion >= 2)
    {
        dataStream >> data.isClockwiseOpening;
    }

    if (actualClassVersion >= 3)
    {
        dataStream >> data.label;
    }

    if (actualClassVersion >= 4)
    {
        dataStream >> data.notMirrored;
    }

    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
auto SingleParallelPoint(const QPointF &p1, const QPointF &p2, qreal angle, qreal width) -> QPointF
{
    if (qFuzzyIsNull(width))
    {
        return p1;
    }

    QLineF pLine(p1, p2);
    pLine.setAngle(pLine.angle() + angle);
    pLine.setLength(width);
    return pLine.p2();
}

//---------------------------------------------------------------------------------------------------------------------
auto SimpleParallelLine(const QPointF &p1, const QPointF &p2, qreal width) -> QLineF
{
    return {SingleParallelPoint(p1, p2, 90, width), SingleParallelPoint(p2, p1, -90, width)};
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPassmark::toJson() const -> QJsonObject
{
    QJsonObject object;
    object["type"_L1] = "VLayoutPassmark";

    // Serialize lines if not empty
    if (!lines.isEmpty())
    {
        QJsonArray linesArray;
        for (const auto &line : lines)
        {
            QJsonObject lineObject;
            lineObject["x1"_L1] = line.x1();
            lineObject["y1"_L1] = line.y1();
            lineObject["x2"_L1] = line.x2();
            lineObject["y2"_L1] = line.y2();
            linesArray.append(lineObject);
        }
        object["lines"_L1] = linesArray;
    }

    // Serialize type if not the default value
    if (type != PassmarkLineType::OneLine)
    {
        object["passmarkType"_L1] = static_cast<int>(type);
    }

    // Serialize baseLine if it's not default
    if (baseLine != QLineF{})
    {
        object["baseLine"_L1] = QJsonObject{
            {"x1"_L1, baseLine.x1()},
            {"y1"_L1, baseLine.y1()},
            {"x2"_L1, baseLine.x2()},
            {"y2"_L1, baseLine.y2()},
        };
    }

    // Serialize isBuiltIn if not false
    if (isBuiltIn)
    {
        object["isBuiltIn"_L1] = isBuiltIn;
    }

    // Serialize isClockwiseOpening if not false
    if (isClockwiseOpening)
    {
        object["isClockwiseOpening"_L1] = isClockwiseOpening;
    }

    // Serialize label if not empty
    if (!label.isEmpty())
    {
        object["label"_L1] = label;
    }

    object["notMirrored"_L1] = notMirrored;

    return object;
}

//---------------------------------------------------------------------------------------------------------------------
auto IsLineSegmentOnLineSegment(const QLineF &seg1, const QLineF &seg2, qreal accuracy) -> bool
{
    const bool onLine = IsPointOnLineviaPDP(seg1.p1(), seg2.p1(), seg2.p2(), accuracy)
                        && IsPointOnLineviaPDP(seg1.p2(), seg2.p1(), seg2.p2(), accuracy);
    if (onLine)
    {
        return IsPointOnLineSegment(seg1.p1(), seg2.p1(), seg2.p2(), accuracy)
               || IsPointOnLineSegment(seg1.p2(), seg2.p1(), seg2.p2(), accuracy)
               || IsPointOnLineSegment(seg2.p1(), seg1.p1(), seg1.p2(), accuracy)
               || IsPointOnLineSegment(seg2.p2(), seg1.p1(), seg1.p2(), accuracy);
    }

    return onLine;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief IsPointOnLineviaPDP use the perp dot product (PDP) way.
 *
 *  The pdp is zero only if the t lies on the line e1 = vector from p1 to p2.
 * @return true if point is on line
 */
auto IsPointOnLineviaPDP(const QPointF &t, const QPointF &p1, const QPointF &p2, qreal accuracy) -> bool
{
    if (p1 == p2)
    {
        return VFuzzyComparePoints(p1, t, accuracy);
    }

    const double p = qAbs(PerpDotProduct(p1, p2, t));
    const double e = GetEpsilon(t, p1, p2, accuracy);

    // We can't use common "<=" here because of the floating-point accuraccy problem
    return p < e || VFuzzyComparePossibleNulls(p, e);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief IsPointOnLineSegment Check if the point is on the line segment.
 */
auto IsPointOnLineSegment(const QPointF &t, const QPointF &p1, const QPointF &p2, qreal accuracy) -> bool
{
    // Because of accuracy issues, this operation is slightly different from ordinary checking point on segment.
    // Here we deal with more like cigar shape.

    // Front and rear easy to check
    if (VFuzzyComparePoints(p1, t, accuracy) || VFuzzyComparePoints(p2, t, accuracy))
    {
        return true;
    }

    // Check if we have a segment. On previous step we already confirmed that we don't have intersection
    if (VFuzzyComparePoints(p1, p2, accuracy))
    {
        return false;
    }

    // Calculate the main rectangle shape. QLineF is not 100% accurate in calculating positions for points, but this
    // should be good enough for us.

    // Compute the perpendicular vector scaled by `accuracy`
    QLineF edge(p1, p2);
    edge.setAngle(edge.angle() + 90);
    edge.setLength(accuracy);
    QPointF const offset = edge.p2() - p1; // Store the perpendicular offset vector

    // Define the expanded bounding rectangle
    const QPointF sP1 = p1 + offset;
    const QPointF sP2 = p2 + offset;
    const QPointF sP3 = p2 - offset;
    const QPointF sP4 = p1 - offset;

    // Early exit if `t` is outside the bounding box (expanded by `accuracy`)
    const qreal minX = std::min({sP1.x(), sP2.x(), sP3.x(), sP4.x()});
    const qreal maxX = std::max({sP1.x(), sP2.x(), sP3.x(), sP4.x()});
    const qreal minY = std::min({sP1.y(), sP2.y(), sP3.y(), sP4.y()});
    const qreal maxY = std::max({sP1.y(), sP2.y(), sP3.y(), sP4.y()});

    if (t.x() < minX || t.x() > maxX || t.y() < minY || t.y() > maxY)
    {
        return false;
    }

    // Use cross-product to determine if `t` is inside the expanded rectangle
    auto CrossProduct = [](QPointF a, QPointF b, QPointF c)
    { return (b.x() - a.x()) * (c.y() - a.y()) - (b.y() - a.y()) * (c.x() - a.x()); };

    return !(CrossProduct(sP1, sP2, t) < 0 || CrossProduct(sP2, sP3, t) < 0 || CrossProduct(sP3, sP4, t) < 0
             || CrossProduct(sP4, sP1, t) < 0);
}
