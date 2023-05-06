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

#include <QVector>
#include <QPolygonF>
#include <QTransform>

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
enum class SplinePointPosition : qint8 { FirstPoint, LastPoint };

// Keep synchronized with XSD schema
enum class PlaceLabelType :  quint8
{
    Segment= 0,
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

    friend auto operator<<(QDataStream& dataStream, const VLayoutPassmark& data) -> QDataStream&;
    friend auto operator>>(QDataStream& dataStream, VLayoutPassmark& data) -> QDataStream&;
private:
    static const quint32 streamHeader;
    static const quint16 classVersion;
};
Q_DECLARE_METATYPE(VLayoutPassmark) // NOLINT

constexpr qreal accuracyPointOnLine = MmToPixel(0.1555);

Q_REQUIRED_RESULT static inline auto VFuzzyComparePoints(const QPointF &p1, const QPointF &p2,
                                                         qreal accuracy = accuracyPointOnLine) -> bool;
static inline auto VFuzzyComparePoints(const QPointF &p1, const QPointF &p2, qreal accuracy) -> bool
{
    return QLineF(p1, p2).length() <= accuracy;
}

Q_REQUIRED_RESULT static inline auto VFuzzyOnAxis(qreal v1, qreal v2, qreal accuracy = accuracyPointOnLine) -> bool;
static inline auto VFuzzyOnAxis(qreal v1, qreal v2, qreal accuracy) -> bool
{
    return qAbs(v1 - v2) <= accuracy;
}

#endif // VGEOMETRYDEF_H
