/************************************************************************
 **
 **  @file   vcontour.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 1, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#ifndef VCONTOUR_H
#define VCONTOUR_H

#include <QSharedDataPointer>
#include <QSizeF>
#include <QTypeInfo>
#include <QVector>
#include <QtGlobal>

#include "vlayoutdef.h"
#include "../vmisc/defglobal.h"

class VContourData;
class QPointF;
class QLineF;
class QRectF;
class QPainterPath;
class VLayoutPiece;

class VContour
{
public:
    VContour();
    VContour(int height, int width, qreal layoutWidth);
    VContour(const VContour &contour);

    ~VContour();

    auto operator=(const VContour &contour) -> VContour &;
#ifdef Q_COMPILER_RVALUE_REFS
    VContour(VContour &&contour) Q_DECL_NOTHROW;
    auto operator=(VContour &&contour) Q_DECL_NOTHROW->VContour &;
#endif

    void CeateEmptySheetContour();

    void             SetContour(const QVector<QPointF> &contour);
    auto GetContour() const -> QVector<QPointF>;

    auto GetShift() const -> qreal;
    void  SetShift(qreal shift);

    auto GetHeight() const -> int;
    void SetHeight(int height);

    auto GetWidth() const -> int;
    void SetWidth(int width);

    auto IsPortrait() const -> bool;

    auto GetSize() const -> QSizeF;

    auto UniteWithContour(const VLayoutPiece &detail, int globalI, int detJ, BestFrom type) const -> QVector<QPointF>;

    auto EmptySheetEdge() const -> QLineF;
    auto GlobalEdgesCount() const -> vsizetype;
    auto GlobalEdge(int i) const -> QLineF;
    auto CutEdge(const QLineF &edge) const -> QVector<QPointF>;
    auto CutEmptySheetEdge() const -> QVector<QPointF>;

    auto at(int i) const -> const QPointF &;

private:
    QSharedDataPointer<VContourData> d;

    void AppendWhole(QVector<QPointF> &contour, const VLayoutPiece &detail, int detJ) const;
    void InsertDetail(QVector<QPointF> &contour, const VLayoutPiece &detail, int detJ) const;

    void ResetAttributes();

    auto EmptySheetEdgesCount() const -> int;
};

Q_DECLARE_TYPEINFO(VContour, Q_MOVABLE_TYPE); // NOLINT

#endif // VCONTOUR_H
