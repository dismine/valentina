/************************************************************************
 **
 **  @file   vcontour.cpp
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

#include "vcontour.h"

#include <QLineF>
#include <QPainterPath>
#include <QPoint>
#include <QPointF>
#include <QPolygonF>
#include <QRectF>
#include <QtMath>

#include "../vgeometry/vgeometrydef.h"
#include "../vgeometry/vgobject.h"
#include "vcontour_p.h"
#include "vlayoutpiece.h"

namespace
{
//---------------------------------------------------------------------------------------------------------------------
void AppendToContour(QVector<QPointF> &contour, QPointF point)
{
    if (not contour.isEmpty())
    {
        if (not VFuzzyComparePoints(contour.constLast(), point))
        {
            contour.append(point);
        }
    }
    else
    {
        contour.append(point);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto OptimizeCombining(const QVector<QPointF> &contour, const QPointF &withdrawEnd) -> QVector<QPointF>
{
    if (contour.size() < 2)
    {
        return contour;
    }

    QPointF withdrawFirst = contour.constLast();
    bool optimize = false;
    int count = 0;
    vsizetype cutIndex = -1;

    for (auto i = contour.size() - 2; i >= 0; --i)
    {
        if (not VGObject::IsPointOnLineSegment(contour.at(i), withdrawFirst, withdrawEnd, accuracyPointOnLine * 2))
        {
            optimize = true;
            cutIndex = i + 1;
            break;
        }

        ++count;
    }

    if (optimize && count > 0)
    {
        return contour.mid(0, cutIndex + 1);
    }

    return contour;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VContour::VContour()
  : d(new VContourData())
{
}

//---------------------------------------------------------------------------------------------------------------------
VContour::VContour(int height, int width, qreal layoutWidth)
  : d(new VContourData(height, width, layoutWidth))
{
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL(VContour)

//---------------------------------------------------------------------------------------------------------------------
auto VContour::operator=(const VContour &contour) -> VContour &
{
    if (&contour == this)
    {
        return *this;
    }
    d = contour.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VContour::VContour(VContour &&contour) noexcept
  : d(std::move(contour.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VContour::operator=(VContour &&contour) noexcept -> VContour &
{
    std::swap(d, contour.d);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VContour::~VContour() = default;

//---------------------------------------------------------------------------------------------------------------------
void VContour::CeateEmptySheetContour()
{
    if (d->globalContour.isEmpty())
    {
        d->globalContour = CutEmptySheetEdge();
        d->globalContour.append(d->globalContour.constFirst()); // Close path

        ResetAttributes();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VContour::SetContour(const QVector<QPointF> &contour)
{
    d->globalContour = contour;

    ResetAttributes();
}

//---------------------------------------------------------------------------------------------------------------------
auto VContour::GetContour() const -> QVector<QPointF>
{
    return d->globalContour;
}

//---------------------------------------------------------------------------------------------------------------------
auto VContour::GetShift() const -> qreal
{
    return d->shift;
}

//---------------------------------------------------------------------------------------------------------------------
void VContour::SetShift(qreal shift)
{
    d->shift = shift;

    ResetAttributes();
}

//---------------------------------------------------------------------------------------------------------------------
auto VContour::GetHeight() const -> int
{
    return d->paperHeight;
}

//---------------------------------------------------------------------------------------------------------------------
void VContour::SetHeight(int height)
{
    d->paperHeight = height;
}

//---------------------------------------------------------------------------------------------------------------------
auto VContour::GetWidth() const -> int
{
    return d->paperWidth;
}

//---------------------------------------------------------------------------------------------------------------------
void VContour::SetWidth(int width)
{
    d->paperWidth = width;
}

//---------------------------------------------------------------------------------------------------------------------
auto VContour::GetSize() const -> QSizeF
{
    return {static_cast<qreal>(d->paperWidth), static_cast<qreal>(d->paperHeight)};
}

//---------------------------------------------------------------------------------------------------------------------
auto VContour::UniteWithContour(const VLayoutPiece &detail, int globalI, int detJ, BestFrom type) const
    -> QVector<QPointF>
{
    QVector<QPointF> newContour;
    if (d->globalContour.isEmpty()) //-V807
    {
        AppendWhole(newContour, detail, 0);
    }
    else
    {
        if (globalI <= 0 || globalI > GlobalEdgesCount())
        {
            return {};
        }

        if (detJ <= 0 || detJ > detail.LayoutEdgesCount())
        {
            return {};
        }

        int i = 0;
        while (i < d->globalContour.count())
        {
            AppendToContour(newContour, d->globalContour.at(i));

            if (type == BestFrom::Rotation)
            {
                if (i == globalI)
                {
                    AppendWhole(newContour, detail, detJ);
                }
            }
            else
            {
                if (i == globalI - 1)
                {
                    InsertDetail(newContour, detail, detJ);
                }
            }

            ++i;
        }
    }
    return newContour;
}

//---------------------------------------------------------------------------------------------------------------------
auto VContour::GlobalEdgesCount() const -> vsizetype
{
    return d->m_emptySheetEdgesCount;
}

//---------------------------------------------------------------------------------------------------------------------
auto VContour::GlobalEdge(int i) const -> QLineF
{
    QLineF edge;
    if (d->globalContour.isEmpty()) //-V807
    {
        // Because sheet is blank we have one global edge for all cases.
        if (i < 1 || i > GlobalEdgesCount())
        { // Doesn't exist such edge
            return EmptySheetEdge();
        }

        const QLineF emptyEdge = EmptySheetEdge();
        const qreal nShift = emptyEdge.length() / static_cast<int>(GlobalEdgesCount());
        edge = IsPortrait() ? QLineF(nShift * (i - 1) + emptyEdge.x1(), emptyEdge.y1(), nShift * i + emptyEdge.x1(),
                                     emptyEdge.y2())
                            : QLineF(emptyEdge.x1(), nShift * (i - 1) + emptyEdge.y1(), emptyEdge.x2(),
                                     nShift * i + emptyEdge.y1());
    }
    else
    {
        if (i < 1 || i > GlobalEdgesCount())
        { // Doesn't exist such edge
            return {};
        }

        if (i < GlobalEdgesCount())
        {
            edge = QLineF(d->globalContour.at(i - 1), d->globalContour.at(i));
        }
        else
        { // Closed contour
            edge = QLineF(d->globalContour.at(GlobalEdgesCount() - 1), d->globalContour.at(0));
        }
    }
    return edge;
}

//---------------------------------------------------------------------------------------------------------------------
auto VContour::CutEdge(const QLineF &edge) const -> QVector<QPointF>
{
    QVector<QPointF> points;
    if (qFuzzyIsNull(d->shift))
    {
        points.append(edge.p1());
        points.append(edge.p2());
    }
    else
    {
        const int n = qFloor(edge.length() / d->shift);

        if (n <= 0)
        {
            points.append(edge.p1());
            points.append(edge.p2());
        }
        else
        {
            points.reserve(n);
            const qreal nShift = edge.length() / n;
            for (int i = 1; i <= n + 1; ++i)
            {
                QLineF l1 = edge;
                l1.setLength(nShift * (i - 1));
                points.append(l1.p2());
            }
        }
    }
    return points;
}

//---------------------------------------------------------------------------------------------------------------------
auto VContour::CutEmptySheetEdge() const -> QVector<QPointF>
{
    QVector<QPointF> points;
    points.reserve(GlobalEdgesCount() + 1);

    const qreal nShift = EmptySheetEdge().length() / static_cast<int>(GlobalEdgesCount());
    for (int i = 1; i <= GlobalEdgesCount() + 1; ++i)
    {
        QLineF l1 = EmptySheetEdge();
        l1.setLength(nShift * (i - 1));
        points.append(l1.p2());
    }
    return points;
}

//---------------------------------------------------------------------------------------------------------------------
auto VContour::at(int i) const -> const QPointF &
{
    return d->globalContour.at(i);
}

//---------------------------------------------------------------------------------------------------------------------
void VContour::AppendWhole(QVector<QPointF> &contour, const VLayoutPiece &detail, int detJ) const
{
    int processedEdges = 0;
    const auto nD = detail.LayoutEdgesCount();
    int j = detJ;

    contour = OptimizeCombining(contour, detail.LayoutEdge(j).p2());

    do
    {
        if (j >= nD)
        {
            j = 0;
        }

        for (auto &point : CutEdge(detail.LayoutEdge(j + 1)))
        {
            AppendToContour(contour, point);
        }
        ++processedEdges;
        ++j;
    } while (processedEdges < nD);
}

//---------------------------------------------------------------------------------------------------------------------
void VContour::InsertDetail(QVector<QPointF> &contour, const VLayoutPiece &detail, int detJ) const
{
    int processedEdges = 0;
    const auto nD = detail.LayoutEdgesCount();
    int j = detJ;

    contour = OptimizeCombining(contour, detail.LayoutEdge(j).p2());

    do
    {
        if (j >= nD)
        {
            j = 0;
        }

        if (j != detJ - 1)
        {
            for (auto &point : CutEdge(detail.LayoutEdge(j + 1)))
            {
                AppendToContour(contour, point);
            }
        }
        ++processedEdges;
        ++j;
    } while (processedEdges < nD);
}

//---------------------------------------------------------------------------------------------------------------------
void VContour::ResetAttributes()
{
    if (not d->globalContour.isEmpty())
    {
        d->m_emptySheetEdgesCount = d->globalContour.count(); // Edges count
    }
    else
    {
        d->m_emptySheetEdgesCount = EmptySheetEdgesCount(); // Edges count
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VContour::EmptySheetEdgesCount() const -> int
{
    qreal defaultShift = ToPixel(0.5, Unit::Cm);
    const qreal emptyEdgeLength = EmptySheetEdge().length();
    if (emptyEdgeLength < defaultShift)
    {
        defaultShift = emptyEdgeLength / 2.0;
    }

    const qreal shift = (qFuzzyIsNull(d->shift) || d->shift > defaultShift) ? defaultShift : d->shift;
    return qFloor(emptyEdgeLength / shift);
}

//---------------------------------------------------------------------------------------------------------------------
auto VContour::IsPortrait() const -> bool
{
    return d->paperHeight >= d->paperWidth;
}

//---------------------------------------------------------------------------------------------------------------------
auto VContour::EmptySheetEdge() const -> QLineF
{
    const int offset = qRound(accuracyPointOnLine * 4.);
    const int layoutOffset = qCeil(d->layoutWidth - accuracyPointOnLine * 2.);
    return IsPortrait() ? QLineF(offset, -layoutOffset, d->paperWidth - offset, -layoutOffset)
                        : QLineF(-layoutOffset, offset, -layoutOffset, d->paperHeight - offset);
}
