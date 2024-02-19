/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 2, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#ifndef VLAYOUTPIECEPATH_H
#define VLAYOUTPIECEPATH_H

#include "vlayoutpoint.h"
#include <QMetaType>
#include <QPointF>
#include <QSharedDataPointer>

class VLayoutPiecePathData;
class QPainterPath;

class VLayoutPiecePath
{
public:
    VLayoutPiecePath();
    explicit VLayoutPiecePath(const QVector<VLayoutPoint> &points);
    VLayoutPiecePath(const VLayoutPiecePath &path);

    virtual ~VLayoutPiecePath();

    auto operator=(const VLayoutPiecePath &path) -> VLayoutPiecePath &;

    VLayoutPiecePath(VLayoutPiecePath &&path) noexcept;
    auto operator=(VLayoutPiecePath &&path) noexcept -> VLayoutPiecePath &;

    auto GetPainterPath() const -> QPainterPath;

    auto Points() const -> QVector<VLayoutPoint>;
    void SetPoints(const QVector<VLayoutPoint> &points);

    auto PenStyle() const -> Qt::PenStyle;
    void SetPenStyle(const Qt::PenStyle &penStyle);

    auto IsCutPath() const -> bool;
    void SetCutPath(bool cut);

    auto IsNotMirrored() const -> bool;
    void SetNotMirrored(bool value);

    friend auto operator<<(QDataStream &dataStream, const VLayoutPiecePath &path) -> QDataStream &;
    friend auto operator>>(QDataStream &dataStream, VLayoutPiecePath &path) -> QDataStream &;

private:
    QSharedDataPointer<VLayoutPiecePathData> d;
};

Q_DECLARE_METATYPE(VLayoutPiecePath)                  // NOLINT
Q_DECLARE_TYPEINFO(VLayoutPiecePath, Q_MOVABLE_TYPE); // NOLINT

#endif // VLAYOUTPIECEPATH_H
