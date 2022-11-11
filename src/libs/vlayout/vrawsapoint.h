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

#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "vlayoutpoint.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
// cppcheck-suppress unknownMacro
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")
QT_WARNING_DISABLE_CLANG("-Wnon-virtual-dtor")

class VRawSAPoint final : public VLayoutPoint
{
public:
    Q_DECL_CONSTEXPR VRawSAPoint() = default;
    Q_DECL_CONSTEXPR VRawSAPoint(qreal xpos, qreal ypos);
    Q_DECL_CONSTEXPR explicit VRawSAPoint(QPointF p);
    Q_DECL_CONSTEXPR explicit VRawSAPoint(const VLayoutPoint &p);
    Q_DECL_RELAXED_CONSTEXPR VRawSAPoint(QPointF p, bool curvePoint, bool turnPoint);
    Q_DECL_RELAXED_CONSTEXPR VRawSAPoint(QPointF p, bool curvePoint, bool turnPoint, bool loopPoint);

    Q_DECL_CONSTEXPR auto LoopPoint() const -> bool;
    Q_DECL_RELAXED_CONSTEXPR void SetLoopPoint(bool loopPoint);

    Q_DECL_CONSTEXPR auto Primary() const -> bool;
    Q_DECL_RELAXED_CONSTEXPR void SetPrimary(bool primary);

    auto toJson() const -> QJsonObject;

private:
    bool m_loopPoint{false};
    bool m_primary{false};
};

Q_DECLARE_METATYPE(VRawSAPoint) // NOLINT
Q_DECLARE_TYPEINFO(VRawSAPoint, Q_MOVABLE_TYPE); // NOLINT

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline VRawSAPoint::VRawSAPoint(qreal xpos, qreal ypos)
    : VLayoutPoint(xpos, ypos)
{}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline VRawSAPoint::VRawSAPoint(QPointF p)
    : VLayoutPoint(p)
{}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline VRawSAPoint::VRawSAPoint(const VLayoutPoint &p)
    : VLayoutPoint(p)
{}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline VRawSAPoint::VRawSAPoint(QPointF p, bool curvePoint, bool turnPoint)
    : VLayoutPoint(p)
{
    SetCurvePoint(curvePoint);
    SetTurnPoint(turnPoint);
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline VRawSAPoint::VRawSAPoint(QPointF p, bool curvePoint, bool turnPoint, bool loopPoint)
    : VLayoutPoint(p),
      m_loopPoint(loopPoint)
{
    SetCurvePoint(curvePoint);
    SetTurnPoint(turnPoint);
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline auto VRawSAPoint::LoopPoint() const -> bool
{
    return m_loopPoint;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline void VRawSAPoint::SetLoopPoint(bool loopPoint)
{
    m_loopPoint = loopPoint;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline auto VRawSAPoint::Primary() const -> bool
{
    return m_primary;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline void VRawSAPoint::SetPrimary(bool primary)
{
    m_primary = primary;
}

QT_WARNING_POP

#endif // VRAWSAPOINT_H
