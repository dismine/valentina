/************************************************************************
 **
 **  @file   vsplinepoint.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#include "vsplinepoint.h"

#include <QJsonObject>
#include <QLineF>

#include "../qmuparser/qmutokenparser.h"
#include "vsplinepoint_p.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VFSplinePoint default constructor.
 */
VFSplinePoint::VFSplinePoint()
  : d(new VFSplinePointData)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VFSplinePoint constructor.
 * @param pSpline spline point.
 * @param kAsm1 coefficient of length first control line.
 * @param angle1 first angle control line.
 * @param kAsm2 coefficient of length second control line.
 * @param angle2 second angle control line.
 */
VFSplinePoint::VFSplinePoint(const VPointF &pSpline, qreal kAsm1, qreal angle1, qreal kAsm2, qreal angle2)
  : d(new VFSplinePointData(pSpline, kAsm1, angle1, kAsm2, angle2))
{
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL(VFSplinePoint)

//---------------------------------------------------------------------------------------------------------------------
auto VFSplinePoint::operator=(const VFSplinePoint &point) -> VFSplinePoint &
{
    if (&point == this)
    {
        return *this;
    }
    d = point.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VFSplinePoint::VFSplinePoint(VFSplinePoint &&point) noexcept
  : d(std::move(point.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VFSplinePoint::operator=(VFSplinePoint &&point) noexcept -> VFSplinePoint &
{
    std::swap(d, point.d);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VFSplinePoint::~VFSplinePoint() = default;

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetAngle1 set first angle of spline.
 * @param value angle.
 */
void VFSplinePoint::SetAngle1(const qreal &value)
{
    QLineF line(0, 0, 100, 0);
    line.setAngle(value);
    d->angle1 = line.angle();

    line.setAngle(d->angle1 + 180);
    d->angle2 = line.angle();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetAngle2 set second angle of spline.
 * @param value angle.
 */
void VFSplinePoint::SetAngle2(const qreal &value)
{
    QLineF line(0, 0, 100, 0);
    line.setAngle(value);
    d->angle2 = line.angle();

    line.setAngle(d->angle2 + 180);
    d->angle1 = line.angle();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief P return point.
 * @return point.
 */
auto VFSplinePoint::P() const -> VPointF
{
    return d->pSpline;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetP set point.
 * @param value point.
 */
void VFSplinePoint::SetP(const VPointF &value)
{
    d->pSpline = value;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Angle1 return first angle of spline.
 * @return angle.
 */
auto VFSplinePoint::Angle1() const -> qreal
{
    return d->angle1;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Angle2 return second angle of spline.
 * @return angle.
 */
auto VFSplinePoint::Angle2() const -> qreal
{
    return d->angle2;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief KAsm1 return coefficient of length first control line.
 * @return coefficient.
 */
auto VFSplinePoint::KAsm1() const -> qreal
{
    return d->kAsm1;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetKAsm1 set coefficient of length first control line.
 * @param value coefficient.
 */
void VFSplinePoint::SetKAsm1(const qreal &value)
{
    d->kAsm1 = value;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief KAsm2 return coefficient of length second control line.
 * @return coefficient.
 */
auto VFSplinePoint::KAsm2() const -> qreal
{
    return d->kAsm2;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetKAsm2 set coefficient of length second control line.
 * @param value coefficient.
 */
void VFSplinePoint::SetKAsm2(const qreal &value)
{
    d->kAsm2 = value;
}

//------------------------------------------VSplinePoint---------------------------------------------------------------
VSplinePoint::VSplinePoint()
  : d(new VSplinePointData)
{
}

//---------------------------------------------------------------------------------------------------------------------
VSplinePoint::VSplinePoint(const VPointF &pSpline,
                           qreal angle1,
                           const QString &angle1F,
                           qreal angle2,
                           const QString &angle2F,
                           qreal length1,
                           const QString &length1F,
                           qreal length2,
                           const QString &length2F,
                           bool strict)
  : d(new VSplinePointData(pSpline, angle1, angle1F, angle2, angle2F, length1, length1F, length2, length2F, strict))
{
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL(VSplinePoint)

//---------------------------------------------------------------------------------------------------------------------
auto VSplinePoint::operator=(const VSplinePoint &point) -> VSplinePoint &
{
    if (&point == this)
    {
        return *this;
    }
    d = point.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VSplinePoint::VSplinePoint(VSplinePoint &&point) noexcept
  : d(std::move(point.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VSplinePoint::operator=(VSplinePoint &&point) noexcept -> VSplinePoint &
{
    std::swap(d, point.d);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VSplinePoint::~VSplinePoint() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VSplinePoint::P() const -> VPointF
{
    return d->pSpline;
}

//---------------------------------------------------------------------------------------------------------------------
void VSplinePoint::SetP(const VPointF &value)
{
    d->pSpline = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSplinePoint::Angle1() const -> qreal
{
    return d->angle1;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSplinePoint::Angle1Formula() const -> QString
{
    return d->angle1F;
}

//---------------------------------------------------------------------------------------------------------------------
void VSplinePoint::SetAngle1(const qreal &value, const QString &angle1F)
{
    d->angle1F = angle1F;

    QLineF line(0, 0, 100, 0);
    line.setAngle(value);
    d->angle1 = line.angle();

    if (d->strict)
    {
        line.setAngle(d->angle1 + 180);
        d->angle2 = line.angle();
        d->angle2F = QString::number(d->angle2);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VSplinePoint::Angle2() const -> qreal
{
    return d->angle2;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSplinePoint::Angle2Formula() const -> QString
{
    return d->angle2F;
}

//---------------------------------------------------------------------------------------------------------------------
void VSplinePoint::SetAngle2(const qreal &value, const QString &angle2F)
{
    d->angle2F = angle2F;

    QLineF line(0, 0, 100, 0);
    line.setAngle(value);
    d->angle2 = line.angle();

    if (d->strict)
    {
        line.setAngle(d->angle2 + 180);
        d->angle1 = line.angle();
        d->angle1F = QString::number(d->angle1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VSplinePoint::Length1() const -> qreal
{
    return d->length1;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSplinePoint::Length1Formula() const -> QString
{
    return d->length1F;
}

//---------------------------------------------------------------------------------------------------------------------
void VSplinePoint::SetLength1(const qreal &value, const QString &length1F)
{
    d->length1 = value;
    d->length1F = length1F;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSplinePoint::Length2() const -> qreal
{
    return d->length2;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSplinePoint::Length2Formula() const -> QString
{
    return d->length2F;
}

//---------------------------------------------------------------------------------------------------------------------
void VSplinePoint::SetLength2(const qreal &value, const QString &length2F)
{
    d->length2 = value;
    d->length2F = length2F;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSplinePoint::IsMovable() const -> bool
{
    return qmu::QmuTokenParser::IsSingle(d->angle1F) && qmu::QmuTokenParser::IsSingle(d->angle2F) &&
           qmu::QmuTokenParser::IsSingle(d->length1F) && qmu::QmuTokenParser::IsSingle(d->length2F);
}

//---------------------------------------------------------------------------------------------------------------------
auto VSplinePoint::IsStrict() const -> bool
{
    return d->strict;
}

//---------------------------------------------------------------------------------------------------------------------
void VSplinePoint::SetStrict(bool value)
{
    d->strict = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSplinePoint::ToJson() const -> QJsonObject
{
    QJsonObject object{
        {"point", d->pSpline.ToJson()},
        {"angle1", d->angle1},
        {"angle1Formula", d->angle1F},
        {"angle2", d->angle2},
        {"angle2Formula", d->angle2F},
        {"length1", d->length1},
        {"length1Formula", d->length1F},
        {"length2", d->length2},
        {"length2Formula", d->length2F},
        {"strict", d->strict},
    };
    return object;
}
