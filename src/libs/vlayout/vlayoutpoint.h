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
    constexpr VLayoutPoint() noexcept = default;
    constexpr VLayoutPoint(qreal xpos, qreal ypos) noexcept;
    constexpr explicit VLayoutPoint(QPointF p) noexcept;
    constexpr VLayoutPoint(const VLayoutPoint &other) noexcept = default;
    constexpr VLayoutPoint &operator=(const VLayoutPoint &other) noexcept;
    virtual ~VLayoutPoint() = default;

    constexpr auto TurnPoint() const noexcept -> bool;
    constexpr auto CurvePoint() const noexcept -> bool;

    constexpr void SetTurnPoint(bool newTurnPoint) noexcept;
    constexpr void SetCurvePoint(bool newCurvePoint) noexcept;

    virtual auto toJson() const -> QJsonObject;
    auto ToQPointF() const -> QPointF;

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
template <class Derived, class Base, std::enable_if_t<std::is_base_of_v<Base, Derived>> * = nullptr>
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
template <class Base, class Derived, std::enable_if_t<std::is_base_of_v<Base, Derived>> * = nullptr>
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
constexpr VLayoutPoint::VLayoutPoint(qreal xpos, qreal ypos) noexcept
  : QPointF(xpos, ypos)
{
}

//---------------------------------------------------------------------------------------------------------------------
constexpr VLayoutPoint::VLayoutPoint(QPointF p) noexcept
  : QPointF(p)
{
}

//---------------------------------------------------------------------------------------------------------------------
constexpr VLayoutPoint &VLayoutPoint::operator=(const VLayoutPoint &other) noexcept
{
    if (this != &other)
    {
        QPointF::operator=(other);
        m_turnPoint = other.m_turnPoint;
        m_curvePoint = other.m_curvePoint;
    }
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr auto VLayoutPoint::TurnPoint() const noexcept -> bool
{
    return m_turnPoint;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr void VLayoutPoint::SetTurnPoint(bool newTurnPoint) noexcept
{
    m_turnPoint = newTurnPoint;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr auto VLayoutPoint::CurvePoint() const noexcept -> bool
{
    return m_curvePoint;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr void VLayoutPoint::SetCurvePoint(bool newCurvePoint) noexcept
{
    m_curvePoint = newCurvePoint;
}

QT_WARNING_POP

#endif // VLAYOUTPOINT_H
