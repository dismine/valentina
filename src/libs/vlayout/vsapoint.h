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
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    Q_DECL_CONSTEXPR VSAPoint() = default;

    QT_WARNING_POP

    Q_DECL_CONSTEXPR VSAPoint(qreal xpos, qreal ypos);
    Q_DECL_CONSTEXPR explicit VSAPoint(QPointF p);
    Q_DECL_CONSTEXPR explicit VSAPoint(const VLayoutPoint &p);

    Q_DECL_CONSTEXPR auto GetSABefore() const -> qreal;
    Q_DECL_CONSTEXPR auto GetSAAfter() const -> qreal;

    Q_DECL_CONSTEXPR auto GetAngleType() const -> PieceNodeAngle;

    Q_DECL_CONSTEXPR auto IsManualPasskmarkLength() const -> bool;
    Q_DECL_CONSTEXPR auto IsManualPasskmarkWidth() const -> bool;
    Q_DECL_CONSTEXPR auto IsManualPasskmarkAngle() const -> bool;

    Q_DECL_CONSTEXPR auto GetPasskmarkLength() const -> qreal;
    Q_DECL_CONSTEXPR auto GetPasskmarkWidth() const -> qreal;
    Q_DECL_CONSTEXPR auto GetPasskmarkAngle() const -> qreal;

    Q_DECL_RELAXED_CONSTEXPR auto GetSABefore(qreal width) const -> qreal;
    Q_DECL_RELAXED_CONSTEXPR auto GetSAAfter(qreal width) const -> qreal;

    Q_DECL_RELAXED_CONSTEXPR void SetSABefore(qreal value);
    Q_DECL_RELAXED_CONSTEXPR void SetSAAfter(qreal value);

    Q_DECL_RELAXED_CONSTEXPR void SetAngleType(PieceNodeAngle value);

    Q_DECL_RELAXED_CONSTEXPR void SetManualPasskmarkLength(bool value);
    Q_DECL_RELAXED_CONSTEXPR void SetManualPasskmarkWidth(bool value);
    Q_DECL_RELAXED_CONSTEXPR void SetManualPasskmarkAngle(bool value);

    Q_DECL_RELAXED_CONSTEXPR void SetPasskmarkLength(qreal value);
    Q_DECL_RELAXED_CONSTEXPR void SetPasskmarkWidth(qreal value);
    Q_DECL_RELAXED_CONSTEXPR void SetPasskmarkAngle(qreal value);

    Q_DECL_RELAXED_CONSTEXPR auto MaxLocalSA(qreal width) const -> qreal;
    Q_DECL_RELAXED_CONSTEXPR auto PassmarkLength(qreal width) const -> qreal;

    Q_DECL_CONSTEXPR auto IsPassmarkClockwiseOpening() const -> bool;
    Q_DECL_RELAXED_CONSTEXPR void SetPassmarkClockwiseOpening(bool clockwise);

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
Q_DECL_CONSTEXPR inline VSAPoint::VSAPoint(qreal xpos, qreal ypos)
  : VLayoutPoint(xpos, ypos)
{
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline VSAPoint::VSAPoint(QPointF p)
  : VLayoutPoint(p)
{
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline VSAPoint::VSAPoint(const VLayoutPoint &p)
  : VLayoutPoint(p)
{
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline auto VSAPoint::GetSABefore() const -> qreal
{
    return m_before;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline auto VSAPoint::GetSABefore(qreal width) const -> qreal
{
    if (m_before < 0)
    {
        return width;
    }
    return qMax(m_before, minSAWidth);
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline void VSAPoint::SetSABefore(qreal value)
{
    value < 0 ? m_before = -1 : m_before = value;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline auto VSAPoint::GetSAAfter() const -> qreal
{
    return m_after;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline auto VSAPoint::GetSAAfter(qreal width) const -> qreal
{
    if (m_after < 0)
    {
        return width;
    }
    return qMax(m_after, minSAWidth);
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline void VSAPoint::SetSAAfter(qreal value)
{
    value < 0 ? m_after = -1 : m_after = value;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline auto VSAPoint::GetAngleType() const -> PieceNodeAngle
{
    return m_angle;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline void VSAPoint::SetAngleType(PieceNodeAngle value)
{
    m_angle = value;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline auto VSAPoint::IsManualPasskmarkLength() const -> bool
{
    return m_manualPassmarkLength;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline auto VSAPoint::IsManualPasskmarkWidth() const -> bool
{
    return m_manualPassmarkWidth;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline auto VSAPoint::IsManualPasskmarkAngle() const -> bool
{
    return m_manualPassmarkAngle;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline void VSAPoint::SetManualPasskmarkLength(bool value)
{
    m_manualPassmarkLength = value;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline void VSAPoint::SetManualPasskmarkWidth(bool value)
{
    m_manualPassmarkWidth = value;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline void VSAPoint::SetManualPasskmarkAngle(bool value)
{
    m_manualPassmarkAngle = value;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline auto VSAPoint::GetPasskmarkLength() const -> qreal
{
    return m_passmarkLength;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline auto VSAPoint::GetPasskmarkWidth() const -> qreal
{
    return m_passmarkWidth;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline auto VSAPoint::GetPasskmarkAngle() const -> qreal
{
    return m_passmarkAngle;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline void VSAPoint::SetPasskmarkLength(qreal value)
{
    m_passmarkLength = value;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline void VSAPoint::SetPasskmarkWidth(qreal value)
{
    m_passmarkWidth = value;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline void VSAPoint::SetPasskmarkAngle(qreal value)
{
    m_passmarkAngle = value;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline auto VSAPoint::MaxLocalSA(qreal width) const -> qreal
{
    return qMax(GetSAAfter(width), GetSABefore(width));
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline auto VSAPoint::PassmarkLength(qreal width) const -> qreal
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
Q_DECL_CONSTEXPR inline auto VSAPoint::IsPassmarkClockwiseOpening() const -> bool
{
    return m_passmarkClockwiseOpening;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline void VSAPoint::SetPassmarkClockwiseOpening(bool clockwise)
{
    m_passmarkClockwiseOpening = clockwise;
}

QT_WARNING_POP

#endif // VSAPOINT_H
