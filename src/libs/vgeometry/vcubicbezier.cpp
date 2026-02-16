/************************************************************************
 **
 **  @file   vcubicbezier.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 3, 2016
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

#include "vcubicbezier.h"

#include <QLineF>

#include "../ifc/ifcdef.h"
#include "vcubicbezier_p.h"
#include "vspline.h"
#include "vsplinepath.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
VCubicBezier::VCubicBezier()
  : VAbstractCubicBezier(GOType::CubicBezier),
    d(new VCubicBezierData)
{
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL_2(VCubicBezier, VAbstractCubicBezier)

//---------------------------------------------------------------------------------------------------------------------
VCubicBezier::VCubicBezier(const VPointF &p1, const VPointF &p2, const VPointF &p3, const VPointF &p4, quint32 idObject,
                           Draw mode)
  : VAbstractCubicBezier(GOType::CubicBezier, idObject, mode),
    d(new VCubicBezierData(p1, p2, p3, p4))
{
    VCubicBezier::CreateName();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezier::operator=(const VCubicBezier &curve) -> VCubicBezier &
{
    if (&curve == this)
    {
        return *this;
    }
    VAbstractCubicBezier::operator=(curve);
    d = curve.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VCubicBezier::VCubicBezier(VCubicBezier &&curve) noexcept
  : VAbstractCubicBezier(std::move(curve)),
    d(std::move(curve.d)) // NOLINT(bugprone-use-after-move)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezier::operator=(VCubicBezier &&curve) noexcept -> VCubicBezier &
{
    VAbstractCubicBezier::operator=(curve);
    std::swap(d, curve.d);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezier::Rotate(const QPointF &originPoint, qreal degrees, const QString &name) const -> VCubicBezier
{
    const VPointF p1 = GetP1().Rotate(originPoint, degrees);
    const VPointF p2 = GetP2().Rotate(originPoint, degrees);
    const VPointF p3 = GetP3().Rotate(originPoint, degrees);
    const VPointF p4 = GetP4().Rotate(originPoint, degrees);
    VCubicBezier curve(p1, p2, p3, p4);
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
auto VCubicBezier::Flip(const QLineF &axis, const QString &name) const -> VCubicBezier
{
    const VPointF p1 = GetP1().Flip(axis);
    const VPointF p2 = GetP2().Flip(axis);
    const VPointF p3 = GetP3().Flip(axis);
    const VPointF p4 = GetP4().Flip(axis);
    VCubicBezier curve(p1, p2, p3, p4);
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
auto VCubicBezier::Move(qreal length, qreal angle, const QString &name) const -> VCubicBezier
{
    const VPointF p1 = GetP1().Move(length, angle);
    const VPointF p2 = GetP2().Move(length, angle);
    const VPointF p3 = GetP3().Move(length, angle);
    const VPointF p4 = GetP4().Move(length, angle);
    VCubicBezier curve(p1, p2, p3, p4);
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
auto VCubicBezier::Offset(qreal distance, const QString &name) const -> VSplinePath
{
    VSpline spl(GetP1(), GetP2().toQPointF(), GetP3().toQPointF(), GetP4());
    spl.SetApproximationScale(GetApproximationScale());

    QVector<VSpline> const subSplines = spl.OffsetPath(distance);

    VSplinePath splPath(subSplines);
    if (!name.isEmpty())
    {
        splPath.SetNameSuffix(name);
        splPath.SetMainNameForHistory(name);
    }

    splPath.SetColor(GetColor());
    splPath.SetPenStyle(GetPenStyle());
    splPath.SetApproximationScale(GetApproximationScale());
    splPath.SetDerivative(true);
    return splPath;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezier::Outline(const QVector<qreal> &distances, const QString &name) const -> VSplinePath
{
    VSpline spl(GetP1(), GetP2().toQPointF(), GetP3().toQPointF(), GetP4());
    spl.SetApproximationScale(GetApproximationScale());

    QVector<VSpline> const subSplines = spl.OutlinePath(distances);

    VSplinePath splPath(subSplines);
    if (!name.isEmpty())
    {
        splPath.SetNameSuffix(name);
        splPath.SetMainNameForHistory(name);
    }

    splPath.SetColor(GetColor());
    splPath.SetPenStyle(GetPenStyle());
    splPath.SetApproximationScale(GetApproximationScale());
    splPath.SetDerivative(true);
    return splPath;
}

//---------------------------------------------------------------------------------------------------------------------
VCubicBezier::~VCubicBezier() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezier::GetP1() const -> VPointF
{
    return d->p1;
}

//---------------------------------------------------------------------------------------------------------------------
void VCubicBezier::SetP1(const VPointF &p)
{
    d->p1 = p;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezier::GetP2() const -> VPointF
{
    return d->p2;
}

//---------------------------------------------------------------------------------------------------------------------
void VCubicBezier::SetP2(const VPointF &p)
{
    d->p2 = p;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezier::GetP3() const -> VPointF
{
    return d->p3;
}

//---------------------------------------------------------------------------------------------------------------------
void VCubicBezier::SetP3(const VPointF &p)
{
    d->p3 = p;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezier::GetP4() const -> VPointF
{
    return d->p4;
}

//---------------------------------------------------------------------------------------------------------------------
void VCubicBezier::SetP4(const VPointF &p)
{
    d->p4 = p;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezier::GetStartAngle() const -> qreal
{
    return QLineF(static_cast<QPointF>(GetP1()), static_cast<QPointF>(GetP2())).angle();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezier::GetEndAngle() const -> qreal
{
    return QLineF(static_cast<QPointF>(GetP4()), static_cast<QPointF>(GetP3())).angle();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetLength return length of cubic bezier curve.
 * @return length.
 */
auto VCubicBezier::GetLength() const -> qreal
{
    return LengthBezier(static_cast<QPointF>(GetP1()), static_cast<QPointF>(GetP2()), static_cast<QPointF>(GetP3()),
                        static_cast<QPointF>(GetP4()), GetApproximationScale());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetPoints return list with cubic bezier curve points.
 * @return list of points.
 */
auto VCubicBezier::GetPoints() const -> QVector<QPointF>
{
    return GetCubicBezierPoints(static_cast<QPointF>(GetP1()), static_cast<QPointF>(GetP2()),
                                static_cast<QPointF>(GetP3()), static_cast<QPointF>(GetP4()), GetApproximationScale());
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezier::GetC1Length() const -> qreal
{
    return QLineF(static_cast<QPointF>(GetP1()), static_cast<QPointF>(GetP2())).length();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezier::GetC2Length() const -> qreal
{
    return QLineF(static_cast<QPointF>(GetP4()), static_cast<QPointF>(GetP3())).length();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezier::GetControlPoint1() const -> QPointF
{
    return static_cast<QPointF>(GetP2());
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezier::GetControlPoint2() const -> QPointF
{
    return static_cast<QPointF>(GetP3());
}

//---------------------------------------------------------------------------------------------------------------------
auto VCubicBezier::GetRealLength() const -> qreal
{
    return LengthBezier(static_cast<QPointF>(GetP1()), static_cast<QPointF>(GetP2()), static_cast<QPointF>(GetP3()),
                        static_cast<QPointF>(GetP4()), maxCurveApproximationScale);
}
