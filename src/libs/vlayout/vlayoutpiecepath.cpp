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

#include "vlayoutpiecepath.h"
#include "vlayoutpiecepath_p.h"

#include <QPainterPath>

// Friend functions
//---------------------------------------------------------------------------------------------------------------------
auto operator<<(QDataStream &dataStream, const VLayoutPiecePath &path) -> QDataStream &
{
    dataStream << *path.d;
    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
auto operator>>(QDataStream &dataStream, VLayoutPiecePath &path) -> QDataStream &
{
    dataStream >> *path.d;
    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
VLayoutPiecePath::VLayoutPiecePath()
  : d(new VLayoutPiecePathData)
{
}

//---------------------------------------------------------------------------------------------------------------------
VLayoutPiecePath::VLayoutPiecePath(const QVector<VLayoutPoint> &points)
  : d(new VLayoutPiecePathData(points))
{
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL(VLayoutPiecePath)

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiecePath::operator=(const VLayoutPiecePath &path) -> VLayoutPiecePath &
{
    if (&path == this)
    {
        return *this;
    }
    d = path.d;
    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VLayoutPiecePath::VLayoutPiecePath(VLayoutPiecePath &&path) noexcept
  : d(std::move(path.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiecePath::operator=(VLayoutPiecePath &&path) noexcept -> VLayoutPiecePath &
{
    std::swap(d, path.d);
    return *this;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
VLayoutPiecePath::~VLayoutPiecePath() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiecePath::GetPainterPath() const -> QPainterPath
{
    QPainterPath path;
    if (not d->m_points.isEmpty())
    {
        QVector<QPointF> points;
        CastTo(d->m_points, points);
        path.addPolygon(QPolygonF(points));
        path.setFillRule(Qt::WindingFill);
    }
    return path;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiecePath::Points() const -> QVector<VLayoutPoint>
{
    return d->m_points;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiecePath::SetPoints(const QVector<VLayoutPoint> &points)
{
    d->m_points = points;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiecePath::PenStyle() const -> Qt::PenStyle
{
    return d->m_penStyle;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiecePath::SetPenStyle(const Qt::PenStyle &penStyle)
{
    d->m_penStyle = penStyle;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiecePath::IsCutPath() const -> bool
{
    return d->m_cut;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiecePath::SetCutPath(bool cut)
{
    d->m_cut = cut;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPiecePath::IsNotMirrored() const -> bool
{
    return d->m_notMirrored;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPiecePath::SetNotMirrored(bool value)
{
    d->m_notMirrored = value;
}
