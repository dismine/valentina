/************************************************************************
 **
 **  @file   vpointf.cpp
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

#include "vpointf.h"
#include "vpointf_p.h"
#include <QJsonObject>
#include <QLineF>
#include <QPointF>
#include <QString>
#include <QTransform>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VPointF creat empty point
 */
VPointF::VPointF()
  : VGObject(GOType::Point, 0, Draw::Calculation),
    d(new VPointFData)
{
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL_2(VPointF, VGObject)

//---------------------------------------------------------------------------------------------------------------------
VPointF::VPointF(const QPointF &point)
  : VGObject(VPointF()),
    d(new VPointFData(point))
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VPointF create new point
 * @param x x coordinate
 * @param y y coordinate
 * @param name point label
 * @param mx offset name respect to x
 * @param my offset name respect to y
 */
VPointF::VPointF(qreal x, qreal y, const QString &name, qreal mx, qreal my, quint32 idObject, const Draw &mode)
  : VGObject(GOType::Point, idObject, mode),
    d(new VPointFData(x, y, mx, my))
{
    setName(name);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VPointF create new point
 * @param point point
 * @param name point label
 * @param mx offset name respect to x
 * @param my offset name respect to y
 */
VPointF::VPointF(const QPointF &point, const QString &name, qreal mx, qreal my, quint32 idObject, const Draw &mode)
  : VGObject(GOType::Point, idObject, mode),
    d(new VPointFData(point, mx, my))
{
    setName(name);
}

//---------------------------------------------------------------------------------------------------------------------
VPointF::~VPointF() = default;

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief operator = assignment operator
 * @param point point
 * @return point
 */
auto VPointF::operator=(const VPointF &point) -> VPointF &
{
    if (&point == this)
    {
        return *this;
    }
    VGObject::operator=(point);
    d = point.d;
    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VPointF::VPointF(VPointF &&point) noexcept
  : VGObject(std::move(point)),
    d(std::move(point.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VPointF::operator=(VPointF &&point) noexcept -> VPointF &
{
    VGObject::operator=(point);
    std::swap(d, point.d);
    return *this;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
VPointF::operator QPointF() const
{
    return toQPointF();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPointF::Rotate(const QPointF &originPoint, qreal degrees, const QString &prefix) const -> VPointF
{
    const QPointF p = RotatePF(originPoint, toQPointF(), degrees);
    VPointF rotated(p, name() + prefix, mx(), my());
    rotated.SetShowLabel(IsShowLabel());
    return rotated;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPointF::Flip(const QLineF &axis, const QString &prefix) const -> VPointF
{
    const QPointF p = FlipPF(axis, toQPointF());
    VPointF flipped(p, name() + prefix, mx(), my());
    flipped.SetShowLabel(IsShowLabel());
    return flipped;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPointF::Move(qreal length, qreal angle, const QString &prefix) const -> VPointF
{
    const QPointF p = MovePF(toQPointF(), length, angle);
    VPointF moved(p, name() + prefix, mx(), my());
    moved.SetShowLabel(IsShowLabel());

    return moved;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief mx return offset name respect to x
 * @return offset
 */
auto VPointF::mx() const -> qreal
{
    return d->_mx;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief my return offset name respect to y
 * @return offset
 */
auto VPointF::my() const -> qreal
{
    return d->_my;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setMx set offset name respect to x
 * @param mx offset
 */
void VPointF::setMx(qreal mx)
{
    d->_mx = mx;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setMy set offset name respect to y
 * @param my offset
 */
void VPointF::setMy(qreal my)
{
    d->_my = my;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPointF::toQPointF() const -> QPointF
{
    return QPointF(d->_x, d->_y);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief x return x coordinate
 * @return value
 */
auto VPointF::x() const -> qreal
{
    return d->_x;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setX set x coordinate
 * @param value x coordinate
 */
void VPointF::setX(const qreal &value)
{
    d->_x = value;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief y return y coordinate
 * @return value
 */
auto VPointF::y() const -> qreal
{
    return d->_y;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setY set y coordinate
 * @param value y coordinate
 */
void VPointF::setY(const qreal &value)
{
    d->_y = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPointF::IsShowLabel() const -> bool
{
    return d->m_showLabel;
}

//---------------------------------------------------------------------------------------------------------------------
void VPointF::SetShowLabel(bool hide)
{
    d->m_showLabel = hide;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPointF::ToJson() const -> QJsonObject
{
    QJsonObject object = VGObject::ToJson();
    object["x"_L1] = x();
    object["y"_L1] = y();
    object["name"_L1] = name();
    object["mx"_L1] = mx();
    object["my"_L1] = my();
    return object;
}

//---------------------------------------------------------------------------------------------------------------------
void VPointF::SetAlias(const QString &alias)
{
    Q_UNUSED(alias);
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
void VPointF::SetAliasSuffix(const QString &aliasSuffix)
{
    Q_UNUSED(aliasSuffix);
    // do nothing
}

//---------------------------------------------------------------------------------------------------------------------
auto VPointF::RotatePF(const QPointF &originPoint, const QPointF &point, qreal degrees) -> QPointF
{
    QLineF axis(originPoint, point);
    axis.setAngle(axis.angle() + degrees);
    return axis.p2();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPointF::FlipPF(const QLineF &axis, const QPointF &point) -> QPointF
{
    const QTransform matrix = FlippingMatrix(axis);
    return matrix.map(point);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPointF::MovePF(const QPointF &originPoint, qreal length, qreal angle) -> QPointF
{
    QLineF line(originPoint.x(), originPoint.y(), originPoint.x() + length, originPoint.y());
    line.setAngle(angle);
    return line.p2();
}
