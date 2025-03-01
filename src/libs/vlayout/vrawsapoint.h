/************************************************************************
 **
 **  @file   vrawsapoint.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   31 10, 2019
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2019 Valentina project
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
#ifndef VRAWSAPOINT_H
#define VRAWSAPOINT_H

#include <QtGlobal>

#include "vlayoutpoint.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
// cppcheck-suppress unknownMacro
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")
QT_WARNING_DISABLE_CLANG("-Wnon-virtual-dtor")

class VRawSAPoint final : public VLayoutPoint
{
public:
    constexpr VRawSAPoint() noexcept = default;
    constexpr VRawSAPoint(qreal xpos, qreal ypos) noexcept;
    constexpr explicit VRawSAPoint(QPointF p) noexcept;
    constexpr explicit VRawSAPoint(const VLayoutPoint &p) noexcept;
    constexpr VRawSAPoint(const VRawSAPoint &other) noexcept;
    constexpr VRawSAPoint(QPointF p, bool curvePoint, bool turnPoint) noexcept;
    constexpr VRawSAPoint(QPointF p, bool curvePoint, bool turnPoint, bool loopPoint) noexcept;
    constexpr VRawSAPoint &operator=(const VRawSAPoint &other) noexcept = default;
    ~VRawSAPoint() override = default;

    constexpr auto LoopPoint() const noexcept -> bool;
    constexpr void SetLoopPoint(bool loopPoint) noexcept;

    constexpr auto Primary() const noexcept -> bool;
    constexpr void SetPrimary(bool primary) noexcept;

    auto toJson() const -> QJsonObject override;

private:
    bool m_loopPoint{false};
    bool m_primary{false};
};

Q_DECLARE_METATYPE(VRawSAPoint)                  // NOLINT
Q_DECLARE_TYPEINFO(VRawSAPoint, Q_MOVABLE_TYPE); // NOLINT

//---------------------------------------------------------------------------------------------------------------------
constexpr VRawSAPoint::VRawSAPoint(qreal xpos, qreal ypos) noexcept
  : VLayoutPoint(xpos, ypos)
{
}

//---------------------------------------------------------------------------------------------------------------------
constexpr VRawSAPoint::VRawSAPoint(QPointF p) noexcept
  : VLayoutPoint(p)
{
}

//---------------------------------------------------------------------------------------------------------------------
constexpr VRawSAPoint::VRawSAPoint(const VLayoutPoint &p) noexcept
  : VLayoutPoint(p)
{
}

//---------------------------------------------------------------------------------------------------------------------
constexpr VRawSAPoint::VRawSAPoint(const VRawSAPoint &other) noexcept
  : VLayoutPoint(static_cast<const VLayoutPoint &>(other)),
    m_loopPoint(other.m_loopPoint),
    m_primary(other.m_primary)
{
}

//---------------------------------------------------------------------------------------------------------------------
constexpr VRawSAPoint::VRawSAPoint(QPointF p, bool curvePoint, bool turnPoint) noexcept
  : VLayoutPoint(p)
{
    SetCurvePoint(curvePoint);
    SetTurnPoint(turnPoint);
}

//---------------------------------------------------------------------------------------------------------------------
constexpr VRawSAPoint::VRawSAPoint(QPointF p, bool curvePoint, bool turnPoint, bool loopPoint) noexcept
  : VLayoutPoint(p),
    m_loopPoint(loopPoint)
{
    SetCurvePoint(curvePoint);
    SetTurnPoint(turnPoint);
}

//---------------------------------------------------------------------------------------------------------------------
constexpr auto VRawSAPoint::LoopPoint() const noexcept -> bool
{
    return m_loopPoint;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr void VRawSAPoint::SetLoopPoint(bool loopPoint) noexcept
{
    m_loopPoint = loopPoint;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr auto VRawSAPoint::Primary() const noexcept -> bool
{
    return m_primary;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr void VRawSAPoint::SetPrimary(bool primary) noexcept
{
    m_primary = primary;
}

QT_WARNING_POP

#endif // VRAWSAPOINT_H
