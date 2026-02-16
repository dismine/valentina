/************************************************************************
 **
 **  @file   vcubicbezierpath.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 3, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#include "vcubicbezierpath.h"

#include <QLineF>
#include <QPoint>
#include <QtMath>

#include "../vmisc/exception/vexception.h"
#include "vabstractcurve.h"
#include "vcubicbezierpath_p.h"
#include "vspline.h"
#include "vsplinepath.h"
#include "vsplinepoint.h"

//---------------------------------------------------------------------------------------------------------------------
VCubicBezierPath::VCubicBezierPath(quint32 idObject, Draw mode)
  : VAbstractCubicBezierPath(GOType::CubicBezierPath, idObject, mode),
    d(new VCubicBezierPathData())
{
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL_2(VCubicBezierPath, VAbstractCubicBezierPath)

//---------------------------------------------------------------------------------------------------------------------
VCubicBezierPath::VCubicBezierPath(const QVector<VPointF> &points, quint32 idObject, Draw mode)
  : VAbstractCubicBezierPath(GOType::CubicBezierPath, idObject, mode),
    d(new VCubicBezierPathData())
{
    if (points.isEmpty())
    {
        return;
    }

    d->path = points;
    VCubicBezierPath::CreateName();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::operator=(const VCubicBezierPath &curve) -> VCubicBezierPath &
{
    if (&curve == this)
    {
        return *this;
    }
    VAbstractCurve::operator=(curve);
    d = curve.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VCubicBezierPath::VCubicBezierPath(VCubicBezierPath &&curve) noexcept
  : VAbstractCubicBezierPath(std::move(curve)),
    d(std::move(curve.d)) // NOLINT(bugprone-use-after-move)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::operator=(VCubicBezierPath &&curve) noexcept -> VCubicBezierPath &
{
    VAbstractCubicBezierPath::operator=(curve);
    std::swap(d, curve.d);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::Rotate(const QPointF &originPoint, qreal degrees, const QString &name) const -> VCubicBezierPath
{
    const QVector<VPointF> points = GetCubicPath();
    VCubicBezierPath curve;
    for (const auto &point : points)
    {
        curve.append(point.Rotate(originPoint, degrees));
    }
    if (!name.isEmpty())
    {
        curve.SetNameSuffix(name);
    }
    curve.SetColor(GetColor());
    curve.SetPenStyle(GetPenStyle());
    curve.SetApproximationScale(GetApproximationScale());
    curve.SetDerivative(true);
    return curve;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::Flip(const QLineF &axis, const QString &name) const -> VCubicBezierPath
{
    const QVector<VPointF> points = GetCubicPath();
    VCubicBezierPath curve;
    for (const auto &point : points)
    {
        curve.append(point.Flip(axis));
    }
    if (!name.isEmpty())
    {
        curve.SetNameSuffix(name);
    }
    curve.SetColor(GetColor());
    curve.SetPenStyle(GetPenStyle());
    curve.SetApproximationScale(GetApproximationScale());
    curve.SetDerivative(true);
    return curve;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::Move(qreal length, qreal angle, const QString &name) const -> VCubicBezierPath
{
    const QVector<VPointF> points = GetCubicPath();
    VCubicBezierPath curve;
    for (const auto &point : points)
    {
        curve.append(point.Move(length, angle));
    }
    if (!name.isEmpty())
    {
        curve.SetNameSuffix(name);
    }
    curve.SetColor(GetColor());
    curve.SetPenStyle(GetPenStyle());
    curve.SetApproximationScale(GetApproximationScale());
    curve.SetDerivative(true);
    return curve;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::Offset(qreal distance, const QString &name) const -> VSplinePath
{
    VSplinePath splPath(GetSplinePath());
    splPath.SetApproximationScale(GetApproximationScale());

    splPath = splPath.Offset(distance, name);
    splPath.SetColor(GetColor());
    splPath.SetPenStyle(GetPenStyle());
    splPath.SetDerivative(true);
    return splPath;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::Outline(const QVector<qreal> &distances, const QString &name) const -> VSplinePath
{
    VSplinePath splPath(GetSplinePath());
    splPath.SetApproximationScale(GetApproximationScale());

    splPath = splPath.Outline(distances, name);
    splPath.SetColor(GetColor());
    splPath.SetPenStyle(GetPenStyle());
    splPath.SetDerivative(true);
    return splPath;
}

//---------------------------------------------------------------------------------------------------------------------
VCubicBezierPath::~VCubicBezierPath() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::operator[](vsizetype indx) -> VPointF &
{
    return d->path[indx];
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::at(vsizetype indx) const -> const VPointF &
{
    return d->path[indx];
}

//---------------------------------------------------------------------------------------------------------------------
void VCubicBezierPath::append(const VPointF &point)
{
    d->path.append(point);
    CreateName();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::CountSubSpl() const -> vsizetype
{
    return CountSubSpl(d->path.size());
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::CountPoints() const -> vsizetype
{
    return d->path.size();
}

//---------------------------------------------------------------------------------------------------------------------
void VCubicBezierPath::Clear()
{
    d->path.clear();
    SetDuplicate(0);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::GetSpline(vsizetype index) const -> VSpline
{
    if (CountPoints() < 4)
    {
        throw VException(tr("Not enough points to create the spline."));
    }

    if (index < 1 || index > CountSubSpl())
    {
        throw VException(tr("This spline does not exist."));
    }

    const vsizetype base = SubSplOffset(index);

    // Correction the first control point of each next spline curve except for the first.
    auto p2 = static_cast<QPointF>(d->path.at(base + 1));
    if (base + 1 > 1)
    {
        const auto b = static_cast<QPointF>(d->path.at(base));
        QLineF const foot1(b, static_cast<QPointF>(d->path.at(base - 1)));
        QLineF foot2(b, p2);

        foot2.setAngle(foot1.angle() + 180);
        p2 = foot2.p2();
    }

    VSpline spl(d->path.at(base), p2, static_cast<QPointF>(d->path.at(base + 2)), d->path.at(base + 3));
    spl.SetApproximationScale(GetApproximationScale());
    return spl;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::GetStartAngle() const -> qreal
{
    if (CountSubSpl() > 0)
    {
        return GetSpline(1).GetStartAngle();
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::GetEndAngle() const -> qreal
{
    if (const vsizetype count = CountSubSpl(); count > 0)
    {
        return GetSpline(count).GetEndAngle();
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::GetC1Length() const -> qreal
{
    if (CountSubSpl() > 0)
    {
        return GetSpline(1).GetC1Length();
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::GetC2Length() const -> qreal
{
    if (const vsizetype count = CountSubSpl(); count > 0)
    {
        return GetSpline(count).GetC2Length();
    }
    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::GetSplinePath() const -> QVector<VSplinePoint>
{
    const vsizetype size = CountSubSpl();
    QVector<VSplinePoint> splPoints(size + 1);

    for (qint32 i = 1; i <= size; ++i)
    {
        const VSpline spl = GetSpline(i);

        {
            VSplinePoint p = splPoints.at(i - 1);
            p.SetP(spl.GetP1());
            p.SetAngle2(spl.GetStartAngle(), spl.GetStartAngleFormula());
            p.SetLength2(spl.GetC1Length(), spl.GetC1LengthFormula());
            splPoints[i - 1] = p;
        }

        {
            VSplinePoint p = splPoints.at(i);
            p.SetP(spl.GetP4());
            p.SetAngle1(spl.GetEndAngle(), spl.GetEndAngleFormula());
            p.SetLength1(spl.GetC2Length(), spl.GetC2LengthFormula());
            splPoints[i] = p;
        }
    }
    return splPoints;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::GetCubicPath() const -> QVector<VPointF>
{
    return d->path;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::CountSubSpl(vsizetype size) -> vsizetype
{
    if (size <= 0)
    {
        return 0;
    }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return qFloor(qAbs((size - 4) / 3.0L + 1));
#else
    return qFloor(qAbs((size - 4) / 3.0 + 1));
#endif
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::SubSplOffset(vsizetype subSplIndex) -> vsizetype
{
    if (subSplIndex <= 0)
    {
        return -1;
    }

    return (subSplIndex - 1) * 3;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::SubSplPointsCount(vsizetype countSubSpl) -> vsizetype
{
    if (countSubSpl <= 0)
    {
        return 0;
    }

    return ((countSubSpl - 1) * 3 + 4);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::FirstPoint() const -> VPointF
{
    if (not d->path.isEmpty())
    {
        return d->path.constFirst();
    }
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezierPath::LastPoint() const -> VPointF
{
    if (const vsizetype count = CountSubSpl(); count >= 1)
    {
        return d->path.at(SubSplOffset(count) + 3); // Take last point of the last real spline
    }

    return {};
}
