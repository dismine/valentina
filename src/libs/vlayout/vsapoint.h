/************************************************************************
 **
 **  @file   vsapoint.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   1 9, 2019
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
#ifndef VSAPOINT_H
#define VSAPOINT_H

#include <QMetaType>
#include <QtGlobal>

#include "../vgeometry/vgeometrydef.h"
#include "../vmisc/def.h"
#include "vlayoutpoint.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")
QT_WARNING_DISABLE_CLANG("-Wnon-virtual-dtor")

/**
 * @brief The VSAPoint class seam allowance point
 */
class VSAPoint final : public VLayoutPoint
{
public:
    constexpr VSAPoint() noexcept;

    constexpr VSAPoint(qreal xpos, qreal ypos);
    constexpr explicit VSAPoint(QPointF p);
    constexpr explicit VSAPoint(const VLayoutPoint &p);

    constexpr auto GetSABefore() const -> qreal;
    constexpr auto GetSAAfter() const -> qreal;

    constexpr auto GetAngleType() const -> PieceNodeAngle;

    constexpr auto IsManualPasskmarkLength() const -> bool;
    constexpr auto IsManualPasskmarkWidth() const -> bool;
    constexpr auto IsManualPasskmarkAngle() const -> bool;

    constexpr auto GetPasskmarkLength() const -> qreal;
    constexpr auto GetPasskmarkWidth() const -> qreal;
    constexpr auto GetPasskmarkAngle() const -> qreal;

    constexpr auto GetSABefore(qreal width) const -> qreal;
    constexpr auto GetSAAfter(qreal width) const -> qreal;

    constexpr void SetSABefore(qreal value);
    constexpr void SetSAAfter(qreal value);

    constexpr void SetAngleType(PieceNodeAngle value);

    constexpr void SetManualPasskmarkLength(bool value);
    constexpr void SetManualPasskmarkWidth(bool value);
    constexpr void SetManualPasskmarkAngle(bool value);

    constexpr void SetPasskmarkLength(qreal value);
    constexpr void SetPasskmarkWidth(qreal value);
    constexpr void SetPasskmarkAngle(qreal value);

    constexpr auto MaxLocalSA(qreal width) const -> qreal;
    constexpr auto PassmarkLength(qreal width) const -> qreal;

    constexpr auto IsPassmarkClockwiseOpening() const -> bool;
    constexpr void SetPassmarkClockwiseOpening(bool clockwise);

    auto toJson() const -> QJsonObject;

    static constexpr qreal passmarkFactor{0.5};
    static constexpr qreal maxPassmarkLength{MmToPixel(10.)};
    static constexpr qreal minSAWidth{accuracyPointOnLine + accuracyPointOnLine * 0.5};

private:
    qreal m_before{-1};
    qreal m_after{-1};
    PieceNodeAngle m_angle{PieceNodeAngle::ByLength};
    bool m_manualPassmarkLength{false};
    bool m_manualPassmarkWidth{false};
    bool m_manualPassmarkAngle{false};
    qreal m_passmarkLength{0};
    qreal m_passmarkWidth{0};
    qreal m_passmarkAngle{0};
    bool m_passmarkClockwiseOpening{false};
};

Q_DECLARE_METATYPE(VSAPoint)                  // NOLINT
Q_DECLARE_TYPEINFO(VSAPoint, Q_MOVABLE_TYPE); // NOLINT

//---------------------------------------------------------------------------------------------------------------------
constexpr inline VSAPoint::VSAPoint() noexcept // NOLINT(hicpp-use-equals-default)
{
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline VSAPoint::VSAPoint(qreal xpos, qreal ypos)
  : VLayoutPoint(xpos, ypos)
{
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline VSAPoint::VSAPoint(QPointF p)
  : VLayoutPoint(p)
{
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline VSAPoint::VSAPoint(const VLayoutPoint &p)
  : VLayoutPoint(p)
{
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline auto VSAPoint::GetSABefore() const -> qreal
{
    return m_before;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline auto VSAPoint::GetSABefore(qreal width) const -> qreal
{
    if (m_before < 0)
    {
        return width;
    }
    return qMax(m_before, minSAWidth);
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline void VSAPoint::SetSABefore(qreal value)
{
    value < 0 ? m_before = -1 : m_before = value;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline auto VSAPoint::GetSAAfter() const -> qreal
{
    return m_after;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline auto VSAPoint::GetSAAfter(qreal width) const -> qreal
{
    if (m_after < 0)
    {
        return width;
    }
    return qMax(m_after, minSAWidth);
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline void VSAPoint::SetSAAfter(qreal value)
{
    value < 0 ? m_after = -1 : m_after = value;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline auto VSAPoint::GetAngleType() const -> PieceNodeAngle
{
    return m_angle;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline void VSAPoint::SetAngleType(PieceNodeAngle value)
{
    m_angle = value;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline auto VSAPoint::IsManualPasskmarkLength() const -> bool
{
    return m_manualPassmarkLength;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline auto VSAPoint::IsManualPasskmarkWidth() const -> bool
{
    return m_manualPassmarkWidth;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline auto VSAPoint::IsManualPasskmarkAngle() const -> bool
{
    return m_manualPassmarkAngle;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline void VSAPoint::SetManualPasskmarkLength(bool value)
{
    m_manualPassmarkLength = value;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline void VSAPoint::SetManualPasskmarkWidth(bool value)
{
    m_manualPassmarkWidth = value;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline void VSAPoint::SetManualPasskmarkAngle(bool value)
{
    m_manualPassmarkAngle = value;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline auto VSAPoint::GetPasskmarkLength() const -> qreal
{
    return m_passmarkLength;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline auto VSAPoint::GetPasskmarkWidth() const -> qreal
{
    return m_passmarkWidth;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline auto VSAPoint::GetPasskmarkAngle() const -> qreal
{
    return m_passmarkAngle;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline void VSAPoint::SetPasskmarkLength(qreal value)
{
    m_passmarkLength = value;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline void VSAPoint::SetPasskmarkWidth(qreal value)
{
    m_passmarkWidth = value;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline void VSAPoint::SetPasskmarkAngle(qreal value)
{
    m_passmarkAngle = value;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline auto VSAPoint::MaxLocalSA(qreal width) const -> qreal
{
    return qMax(GetSAAfter(width), GetSABefore(width));
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline auto VSAPoint::PassmarkLength(qreal width) const -> qreal
{
    if (not m_manualPassmarkLength)
    {
        qreal passmarkLength = MaxLocalSA(width) * passmarkFactor;
        passmarkLength = qMin(passmarkLength, maxPassmarkLength);
        return passmarkLength;
    }

    return m_passmarkLength;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline auto VSAPoint::IsPassmarkClockwiseOpening() const -> bool
{
    return m_passmarkClockwiseOpening;
}

//---------------------------------------------------------------------------------------------------------------------
constexpr inline void VSAPoint::SetPassmarkClockwiseOpening(bool clockwise)
{
    m_passmarkClockwiseOpening = clockwise;
}

QT_WARNING_POP

#endif // VSAPOINT_H
