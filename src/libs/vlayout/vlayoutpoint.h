/************************************************************************
 **
 **  @file   vlayoutpoint.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 10, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Valentina project
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
#ifndef VLAYOUTPOINT_H
#define VLAYOUTPOINT_H

#include <QMetaType>
#include <QPointF>
#include <QVector>
#include <QtGlobal>

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")
QT_WARNING_DISABLE_CLANG("-Wnon-virtual-dtor")

class VLayoutPoint : public QPointF
{
public:
    Q_DECL_CONSTEXPR VLayoutPoint() = default;
    Q_DECL_CONSTEXPR VLayoutPoint(qreal xpos, qreal ypos);
    Q_DECL_CONSTEXPR explicit VLayoutPoint(QPointF p);

    Q_DECL_CONSTEXPR auto TurnPoint() const -> bool;
    Q_DECL_CONSTEXPR auto CurvePoint() const -> bool;

    Q_DECL_RELAXED_CONSTEXPR void SetTurnPoint(bool newTurnPoint);
    Q_DECL_RELAXED_CONSTEXPR void SetCurvePoint(bool newCurvePoint);

    auto toJson() const -> QJsonObject;

private:
    bool m_turnPoint{false};
    bool m_curvePoint{false};
};

Q_DECLARE_METATYPE(VLayoutPoint)                  // NOLINT
Q_DECLARE_TYPEINFO(VLayoutPoint, Q_MOVABLE_TYPE); // NOLINT

//---------------------------------------------------------------------------------------------------------------------
template <class T> inline auto CastTo(const QVector<T> &points, QVector<T> &casted) -> void
{
    Q_UNUSED(points)
    Q_UNUSED(casted)
}

//---------------------------------------------------------------------------------------------------------------------
// upcast
template <class Derived, class Base, typename std::enable_if<std::is_base_of<Base, Derived>::value>::type * = nullptr>
inline auto CastTo(const QVector<Base> &points, QVector<Derived> &casted) -> void
{
    casted.clear();
    casted.reserve(points.size());
    for (const auto &p : points)
    {
        casted.append(Derived(p));
    }
}

//---------------------------------------------------------------------------------------------------------------------
// downcast
template <class Base, class Derived, typename std::enable_if<std::is_base_of<Base, Derived>::value>::type * = nullptr>
inline auto CastTo(const QVector<Derived> &points, QVector<Base> &casted) -> void
{
    casted.clear();
    casted.reserve(points.size());
    for (const auto &p : points)
    {
        casted.append(p);
    }
}

/*****************************************************************************
  VLayoutPoint stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
auto operator<<(QDataStream &, const VLayoutPoint &) -> QDataStream &;
auto operator>>(QDataStream &, VLayoutPoint &) -> QDataStream &;
#endif

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline VLayoutPoint::VLayoutPoint(qreal xpos, qreal ypos)
  : QPointF(xpos, ypos)
{
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline VLayoutPoint::VLayoutPoint(QPointF p)
  : QPointF(p)
{
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline auto VLayoutPoint::TurnPoint() const -> bool
{
    return m_turnPoint;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline void VLayoutPoint::SetTurnPoint(bool newTurnPoint)
{
    m_turnPoint = newTurnPoint;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline auto VLayoutPoint::CurvePoint() const -> bool
{
    return m_curvePoint;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline void VLayoutPoint::SetCurvePoint(bool newCurvePoint)
{
    m_curvePoint = newCurvePoint;
}

QT_WARNING_POP

#endif // VLAYOUTPOINT_H
