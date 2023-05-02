/************************************************************************
 **
 **  @file   vpiecegrainline.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 4, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#ifndef VPIECEGRAINLINE_H
#define VPIECEGRAINLINE_H

#include <QSharedDataPointer>
#include <QMetaType>
#include "../vpatterndb/floatItemData/floatitemdef.h"

class QPointF;
class VPieceGrainlinePrivate;
class QLineF;
class QPolygonF;
class QRectF;

using GrainlineShape = QVector<QVector<QPointF>>;

class VPieceGrainline
{
public:
    VPieceGrainline();
    VPieceGrainline(const QLineF &mainLine, GrainlineArrowDirection arrowType);
    VPieceGrainline(const QPointF &p1, qreal length, qreal angle, GrainlineArrowDirection arrowType);
    VPieceGrainline(const VPieceGrainline &other);

    ~VPieceGrainline();

    auto operator=(const VPieceGrainline &grainline) -> VPieceGrainline &;
#ifdef Q_COMPILER_RVALUE_REFS
    VPieceGrainline(VPieceGrainline &&grainline) Q_DECL_NOTHROW;
    auto operator=(VPieceGrainline &&grainline) Q_DECL_NOTHROW -> VPieceGrainline &;
#endif

    auto GetMainLine() const -> QLineF;
    void SetMainLine(const QLineF &mainLine);

    auto GetArrowType() const -> GrainlineArrowDirection;
    void SetArrowType(GrainlineArrowDirection arrowType);

    auto IsEnabled() const -> bool;
    void SetEnabled(bool enabled);

    auto SecondaryLine() const -> QLineF;

    auto IsFourWays() const -> bool;

    auto IsArrowUpEnabled() const -> bool;
    auto IsArrowDownEnabled() const -> bool;
    auto IsArrowLeftEnabled() const -> bool;
    auto IsArrowRightEnabled() const -> bool;

    auto ArrowUp() const -> QPolygonF;
    auto ArrowDown() const -> QPolygonF;
    auto ArrowLeft() const -> QPolygonF;
    auto ArrowRight() const -> QPolygonF;

    auto Shape() const -> GrainlineShape;

    auto IsContained(const QRectF &boundingRect, qreal &dX, qreal &dY) const -> bool;
    auto IsPositionValid(const QVector<QPointF> &contourPoints) const -> bool;

    auto IsShapeValid() const -> bool;

    friend auto operator<< (QDataStream& dataStream, const VPieceGrainline& grainline) -> QDataStream&;
    friend auto operator>> (QDataStream& dataStream, VPieceGrainline& grainline) -> QDataStream&;
private:
    QSharedDataPointer<VPieceGrainlinePrivate> d;
};

Q_DECLARE_METATYPE(VPieceGrainline)
Q_DECLARE_TYPEINFO(VPieceGrainline, Q_MOVABLE_TYPE); // NOLINT

#endif // VPIECEGRAINLINE_H
