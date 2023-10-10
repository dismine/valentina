/************************************************************************
 **
 **  @file   vabstractcurve.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 6, 2014
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

#include "vabstractcurve.h"

#include <QLine>
#include <QLineF>
#include <QMessageLogger>
#include <QPainterPath>
#include <QPoint>
#include <QtDebug>

#include "../ifc/exception/vexceptionobjecterror.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/vabstractvalapplication.h"
#include "vabstractcurve_p.h"

// See https://stackoverflow.com/a/46719572/3045403
#if __cplusplus < 201703L
constexpr qreal VAbstractCurve::minLength; // NOLINT(readability-redundant-declaration)
#endif

//---------------------------------------------------------------------------------------------------------------------
VAbstractCurve::VAbstractCurve(const GOType &type, const quint32 &idObject, const Draw &mode)
  : VGObject(type, idObject, mode),
    d(new VAbstractCurveData())
{
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL_2(VAbstractCurve, VGObject)

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::operator=(const VAbstractCurve &curve) -> VAbstractCurve &
{
    if (&curve == this)
    {
        return *this;
    }
    VGObject::operator=(curve);
    d = curve.d;
    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VAbstractCurve::VAbstractCurve(VAbstractCurve &&curve) noexcept
  : VGObject(std::move(curve)),
    d(std::move(curve.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::operator=(VAbstractCurve &&curve) noexcept -> VAbstractCurve &
{
    VGObject::operator=(curve);
    std::swap(d, curve.d);
    return *this;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
VAbstractCurve::~VAbstractCurve() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::GetSegmentPoints(const QVector<QPointF> &points, const QPointF &begin, const QPointF &end,
                                      bool reverse, QString &error) -> QVector<QPointF>
{
    QVector<QPointF> segment = points;
    if (reverse)
    {
        segment = Reverse(segment);
    }

    QPointF start = begin;
    QPointF finish = end;

    if (begin == end)
    {
        start = segment.constFirst();
        finish = segment.constLast();
    }

    bool ok = false;
    segment = FromBegin(segment, start, &ok);

    if (not ok)
    {
        error = QObject::tr("Could not find the segment start.");
        return segment;
    }

    ok = false;
    segment = ToEnd(segment, finish, &ok);

    if (not ok)
    {
        error = QObject::tr("Could not find the segment end.");
        return segment;
    }

    if (segment.length() < 2)
    {
        error = QObject::tr("Segment is too short.");
    }

    return segment;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::GetSegmentPoints(const QPointF &begin, const QPointF &end, bool reverse,
                                      const QString &piece) const -> QVector<QPointF>
{
    QString error;
    QVector<QPointF> segment = GetSegmentPoints(GetPoints(), begin, end, reverse, error);

    if (not error.isEmpty())
    {
        QString errorMsg;
        if (piece.isEmpty())
        {
            errorMsg = QObject::tr("Error calculating segment for curve '%1'. %2").arg(name(), error);
        }
        else
        {
            errorMsg = QObject::tr("Error in path '%1'. Calculating segment for curve '%2' has failed. %3")
                           .arg(piece, name(), error);
        }
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VExceptionObjectError(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
    }

    return segment;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::FromBegin(const QVector<QPointF> &points, const QPointF &begin, bool *ok) -> QVector<QPointF>
{
    auto SetResult = [&ok](bool res)
    {
        if (ok != nullptr)
        {
            *ok = res;
        }
    };

    if (points.count() < 2)
    {
        SetResult(false);
        return points;
    }

    if (points.constFirst().toPoint() == begin.toPoint())
    {
        SetResult(true);
        return points;
    }

    QVector<QPointF> segment;
    bool theBegin = false;
    for (qint32 i = 0; i < points.count() - 1; ++i)
    {
        if (not theBegin)
        {
            if (IsPointOnLineSegment(begin, points.at(i), points.at(i + 1)))
            {
                theBegin = true;

                if (not VFuzzyComparePoints(begin, points.at(i + 1)))
                {
                    segment.append(begin);
                }

                if (i == points.count() - 2)
                {
                    segment.append(points.at(i + 1));
                }
            }
        }
        else
        {
            segment.append(points.at(i));
            if (i == points.count() - 2)
            {
                segment.append(points.at(i + 1));
            }
        }
    }

    if (segment.isEmpty())
    {
        SetResult(false);
        return points;
    }

    SetResult(true);
    return segment;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::ToEnd(const QVector<QPointF> &points, const QPointF &end, bool *ok) -> QVector<QPointF>
{
    QVector<QPointF> reversed = Reverse(points);
    reversed = FromBegin(reversed, end, ok);
    return Reverse(reversed);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::ClosestPoint(QPointF scenePoint) const -> QPointF
{
    const QVector<QPointF> points = GetPoints();
    if (points.count() < 2)
    {
        return {};
    }

    if (VFuzzyComparePoints(points.constFirst(), scenePoint))
    {
        return points.constFirst();
    }

    QPointF candidatePoint;
    qreal bestDistance = INT_MAX;
    bool found = false;

    for (qint32 i = 0; i < points.count() - 1; ++i)
    {
        const QPointF cPoint = VGObject::ClosestPoint(QLineF(points.at(i), points.at(i + 1)), scenePoint);

        if (IsPointOnLineSegment(cPoint, points.at(i), points.at(i + 1)))
        {
            const qreal length = QLineF(scenePoint, cPoint).length();
            if (length < bestDistance)
            {
                candidatePoint = cPoint;
                bestDistance = length;
                found = true;
            }
        }
    }

    if (found)
    {
        return candidatePoint;
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::GetPath() const -> QPainterPath
{
    QPainterPath path;

    const QVector<QPointF> points = GetPoints();
    if (points.count() >= 2)
    {
        path.addPolygon(QPolygonF(points));
    }
    else
    {
        qDebug() << "points.count() < 2" << Q_FUNC_INFO;
    }
    return path;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::GetLengthByPoint(const QPointF &point) const -> qreal
{
    const QVector<QPointF> points = GetPoints();
    if (points.size() < 2)
    {
        return -1;
    }

    if (points.constFirst().toPoint() == point.toPoint())
    {
        return 0;
    }

    bool ok = false;
    const QVector<QPointF> segment = ToEnd(points, point, &ok);
    if (not ok)
    {
        return -1;
    }
    return PathLength(segment);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief IntersectLine return list of points for real intersection with line
 * @param line line that intersect with curve
 * @return list of intersection points
 */
auto VAbstractCurve::IntersectLine(const QLineF &line) const -> QVector<QPointF>
{
    return CurveIntersectLine(this->GetPoints(), line);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::IsIntersectLine(const QLineF &line) const -> bool
{
    const QVector<QPointF> points = IntersectLine(line);
    return not points.isEmpty();
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::IsPointOnCurve(const QVector<QPointF> &points, const QPointF &p) -> bool
{
    if (points.isEmpty())
    {
        return false;
    }

    if (points.size() < 2)
    {
        return points.at(0) == p;
    }

    for (qint32 i = 0; i < points.count() - 1; ++i)
    {
        if (IsPointOnLineSegment(p, points.at(i), points.at(i + 1)))
        {
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::IsPointOnCurve(const QPointF &p) const -> bool
{
    return IsPointOnCurve(GetPoints(), p);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::SubdividePath(const QVector<QPointF> &points, QPointF p, QVector<QPointF> &sub1,
                                   QVector<QPointF> &sub2) -> bool
{
    if (points.size() < 2)
    {
        return false;
    }

    bool found = false;
    sub1.clear();
    sub2.clear();

    for (qint32 i = 0; i < points.count() - 1; ++i)
    {
        if (not found)
        {
            if (IsPointOnLineSegment(p, points.at(i), points.at(i + 1)))
            {
                if (not VFuzzyComparePoints(points.at(i), p))
                {
                    sub1.append(points.at(i));
                    sub1.append(p);
                }
                else
                {
                    if (not sub1.isEmpty())
                    {
                        sub1.append(p);
                    }
                }

                if (not VFuzzyComparePoints(points.at(i + 1), p))
                {
                    sub2.append(p);

                    if (i + 1 == points.count() - 1)
                    {
                        sub2.append(points.at(i + 1));
                    }
                }

                found = true;
            }
            else
            {
                sub1.append(points.at(i));
            }
        }
        else
        {
            sub2.append(points.at(i));

            if (i + 1 == points.count() - 1)
            {
                sub2.append(points.at(i + 1));
            }
        }
    }

    if (not found)
    {
        sub1.clear();
    }

    return found;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::GetDuplicate() const -> quint32
{
    return d->duplicate;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractCurve::SetDuplicate(quint32 number)
{
    d->duplicate = number;
    CreateName();
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::GetColor() const -> QString
{
    return d->color;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractCurve::SetColor(const QString &color)
{
    d->color = color;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::GetPenStyle() const -> QString
{
    return d->penStyle;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractCurve::SetPenStyle(const QString &penStyle)
{
    d->penStyle = penStyle;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::GetApproximationScale() const -> qreal
{
    return d->approximationScale;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractCurve::SetApproximationScale(qreal value)
{
    d->approximationScale = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::CurveIntersectLine(const QVector<QPointF> &points, const QLineF &line) -> QVector<QPointF>
{
    QVector<QPointF> intersections;
    intersections.reserve(points.count() - 1);
    for (auto i = 0; i < points.count() - 1; ++i)
    {
        QPointF crosPoint;
        auto type = Intersects(line, QLineF(points.at(i), points.at(i + 1)), &crosPoint);

        // QLineF::intersects not always accurate on edge cases
        if (type == QLineF::BoundedIntersection ||
            (VGObject::IsPointOnLineSegment(crosPoint, points.at(i), points.at(i + 1)) &&
             VGObject::IsPointOnLineSegment(crosPoint, line.p1(), line.p2())))
        {
            intersections.append(crosPoint);
        }
    }
    return intersections;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::CurveIntersectAxis(const QPointF &point, qreal angle, const QVector<QPointF> &curvePoints,
                                        QPointF *intersectionPoint) -> bool
{
    SCASSERT(intersectionPoint != nullptr)

    // Normalize an angle
    {
        QLineF line(QPointF(10, 10), QPointF(100, 10));
        line.setAngle(angle);
        angle = line.angle();
    }

    QRectF rec = QRectF(0, 0, INT_MAX, INT_MAX);
    rec.translate(-INT_MAX / 2.0, -INT_MAX / 2.0);

    // Instead of using axis compare two rays. See issue #963.
    QLineF axis = QLineF(point, VGObject::BuildRay(point, angle, rec));
    QVector<QPointF> points = VAbstractCurve::CurveIntersectLine(curvePoints, axis);

    axis = QLineF(point, VGObject::BuildRay(point, angle + 180, rec));
    points += VAbstractCurve::CurveIntersectLine(curvePoints, axis);

    if (not points.isEmpty())
    {
        if (points.size() == 1)
        {
            *intersectionPoint = points.at(0);
            return true;
        }

        QMap<qreal, int> forward;
        QMap<qreal, int> backward;

        for (qint32 i = 0; i < points.size(); ++i)
        {
            if (VFuzzyComparePoints(points.at(i), point))
            { // Always seek unique intersection
                continue;
            }

            const QLineF length(point, points.at(i));
            if (qAbs(length.angle() - angle) < 0.1)
            {
                forward.insert(length.length(), i);
            }
            else
            {
                backward.insert(length.length(), i);
            }
        }

        // Closest point is not always want we need. First return point in forward direction if exists.
        if (not forward.isEmpty())
        {
            *intersectionPoint = points.at(forward.first());
            return true;
        }

        if (not backward.isEmpty())
        {
            *intersectionPoint = points.at(backward.first());
            return true;
        }

        if (VAbstractCurve::IsPointOnCurve(curvePoints, point))
        {
            *intersectionPoint = point;
            return true;
        }
    }
    else
    {
        if (VAbstractCurve::IsPointOnCurve(curvePoints, point))
        {
            *intersectionPoint = point;
            return true;
        }
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::DirectionArrows() const -> QVector<DirectionArrow>
{
    QVector<DirectionArrow> arrows;

    const QVector<QPointF> points = GetPoints();
    if (points.count() >= 2)
    {
        /*Need find coordinate midle of curve.
          Universal way is take all points and find sum.*/
        const qreal seek_length = qAbs(GetLength()) / 2.0;
        qreal found_length = 0;
        QLineF arrow;
        for (qint32 i = 1; i <= points.size() - 1; ++i)
        {
            arrow = QLineF(points.at(i - 1), points.at(i));
            found_length += arrow.length(); // Length that we aready find

            if (seek_length <= found_length) // if have found more that need stop.
            {
                // subtract length in last line and you will find position of the middle point.
                arrow.setLength(arrow.length() - (found_length - seek_length));
                break;
            }
        }

        // Reverse line because we want start arrow from this point
        arrow = QLineF(arrow.p2(), arrow.p1());
        const qreal angle = arrow.angle(); // we each time change line angle, better save original angle value
        arrow.setLength(VAbstractCurve::LengthCurveDirectionArrow());

        DirectionArrow dArrow;

        arrow.setAngle(angle - 35);
        dArrow.first = arrow;

        arrow.setAngle(angle + 35);
        dArrow.second = arrow;

        arrows.append(dArrow);
    }
    return arrows;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::ShowDirection(const QVector<DirectionArrow> &arrows, qreal width) -> QPainterPath
{
    QPainterPath path;

    for (auto arrow : arrows)
    {
        if (not arrow.first.isNull() && not arrow.second.isNull())
        {
            QPainterPath arrowPath;

            QLineF line = arrow.first;
            line.setLength(width);
            arrowPath.moveTo(line.p1());
            arrowPath.lineTo(line.p2());

            line = arrow.second;
            line.setLength(width);
            arrowPath.moveTo(line.p1());
            arrowPath.lineTo(line.p2());

            path.addPath(arrowPath);
        }
    }
    return path;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::LengthCurveDirectionArrow() -> qreal
{
    return VAbstractApplication::VApp()->Settings()->GetLineWidth() * 8.0;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractCurve::SetAliasSuffix(const QString &aliasSuffix)
{
    VGObject::SetAliasSuffix(aliasSuffix);
    CreateAlias();
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCurve::PathLength(const QVector<QPointF> &path) -> qreal
{
    if (path.size() < 2)
    {
        return 0;
    }

    QPainterPath splinePath;
    splinePath.moveTo(path.at(0));
    for (qint32 i = 1; i < path.count(); ++i)
    {
        splinePath.lineTo(path.at(i));
    }
    return splinePath.length();
}
