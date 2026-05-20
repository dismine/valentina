/************************************************************************
 **
 **  @file   labelarrangetypes.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 5, 2026
 **
 **  @brief  Shared data types for the automatic label arrangement feature.
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

#ifndef LABELARRANGETYPES_H
#define LABELARRANGETYPES_H

#include <QPointF>
#include <QSizeF>
#include <QtGlobal>

// Keys stored via QGraphicsItem::setData() by tool constructors in vtools.
// Read by collectors in vwidgets without depending on vtools types.
enum LabelDataRole : quint16
{
    LabelToolIdRole   = Qt::UserRole + 200, ///< quint32 – owning tool / element id
    LabelKindRole     = Qt::UserRole + 201, ///< int cast of LabelMoveRequest::Kind
    LabelPointIdRole  = Qt::UserRole + 202, ///< quint32 – sub-point id (Double / Operation)
    LabelExtraIdxRole = Qt::UserRole + 203  ///< int – MoveDoublePoint or SegmentLabel enum value
};

/// Input record for the geometry-only placement engine.
struct LabelArrangeData
{
    QPointF anchor{};          ///< Anchor point in scene coordinates (pixels)
    QSizeF  labelSize{};       ///< Label bounding-box size in scene coordinates (pixels)
    QPointF currentOffset{};   ///< Current (mx, my) offset from anchor in scene pixels
    qreal   anchorRadius{4.0}; ///< Radius of the anchor-point dot in scene pixels (from ScaledRadius)
};

/// Enough information to create any of the four label-move undo commands.
struct LabelMoveRequest
{
    enum class Kind : quint8
    {
        Point,     ///< MoveLabel          – single construction point
        Double,    ///< MoveDoubleLabel    – arc/spline intersection (two result pts)
        Operation, ///< OperationMoveLabel – move/rotate/flip result points
        Segment    ///< MoveSegmentLabel   – curve segment label
    };

    Kind    kind{Kind::Point};
    quint32 toolId{0};   ///< ID of the owning tool / operation element
    quint32 pointId{0};  ///< Sub-point id (Double and Operation only)
    int     extraIdx{0}; ///< MoveDoublePoint (0/1) or SegmentLabel (0-3) cast to int
    QPointF newOffset{}; ///< New (mx, my) in scene pixels – filled in after arrangement
};

#endif // LABELARRANGETYPES_H
