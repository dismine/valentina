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

#include <QtGlobal>
#include <QMetaType>

#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/def.h"
#include "../vgeometry/vgeometrydef.h"
#include "vlayoutpoint.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")
QT_WARNING_DISABLE_CLANG("-Wnon-virtual-dtor")
QT_WARNING_DISABLE_CLANG("-Wdelete-non-virtual-dtor")

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
    Q_DECL_CONSTEXPR auto GetPasskmarkLength() const -> qreal;

    Q_DECL_RELAXED_CONSTEXPR auto GetSABefore(qreal width) const -> qreal;
    Q_DECL_RELAXED_CONSTEXPR auto GetSAAfter(qreal width) const -> qreal;

    Q_DECL_RELAXED_CONSTEXPR void SetSABefore(qreal value);
    Q_DECL_RELAXED_CONSTEXPR void SetSAAfter(qreal value);

    Q_DECL_RELAXED_CONSTEXPR void SetAngleType(PieceNodeAngle value);
    Q_DECL_RELAXED_CONSTEXPR void SetManualPasskmarkLength(bool value);
    Q_DECL_RELAXED_CONSTEXPR void SetPasskmarkLength(qreal value);

    Q_DECL_RELAXED_CONSTEXPR auto MaxLocalSA(qreal width) const -> qreal;
    Q_DECL_RELAXED_CONSTEXPR auto PassmarkLength(qreal width) const -> qreal;

    auto toJson() const -> QJsonObject override;

    static constexpr qreal passmarkFactor{0.5};
    static constexpr qreal maxPassmarkLength{MmToPixel(10.)};
    static constexpr qreal minSAWidth{accuracyPointOnLine + accuracyPointOnLine*0.5};

private:
    qreal          m_before{-1};
    qreal          m_after{-1};
    PieceNodeAngle m_angle{PieceNodeAngle::ByLength};
    bool           m_manualPassmarkLength{false};
    qreal          m_passmarkLength{0};
};

Q_DECLARE_METATYPE(VSAPoint)  // NOLINT
Q_DECLARE_TYPEINFO(VSAPoint, Q_MOVABLE_TYPE); // NOLINT

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline VSAPoint::VSAPoint(qreal xpos, qreal ypos)
    : VLayoutPoint(xpos, ypos)
{}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline VSAPoint::VSAPoint(QPointF p)
    : VLayoutPoint(p)
{}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline VSAPoint::VSAPoint(const VLayoutPoint &p)
    : VLayoutPoint(p)
{}

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
Q_DECL_RELAXED_CONSTEXPR inline void VSAPoint::SetManualPasskmarkLength(bool value)
{
    m_manualPassmarkLength = value;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline auto VSAPoint::GetPasskmarkLength() const -> qreal
{
    return m_passmarkLength;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_RELAXED_CONSTEXPR inline void VSAPoint::SetPasskmarkLength(qreal value)
{
    m_passmarkLength = value;
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

QT_WARNING_POP

#endif // VSAPOINT_H
