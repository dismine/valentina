/************************************************************************
 **
 **  @file   vcubicbezier_p.h
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

#ifndef VCUBICBEZIER_P_H
#define VCUBICBEZIER_P_H

#include <QLineF>
#include <QSharedData>

#include "vpointf.h"
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VCubicBezierData final : public QSharedData
{
public:
    VCubicBezierData() = default;
    VCubicBezierData(const VCubicBezierData &curve) = default;
    VCubicBezierData(const VPointF &p1, const VPointF &p2, const VPointF &p3, const VPointF &p4);
    ~VCubicBezierData() = default;

    /** @brief p1 first spline point. */
    VPointF p1{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief p2 fourth spline point. */
    VPointF p2{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief p3 fourth spline point. */
    VPointF p3{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief p4 fourth spline point. */
    VPointF p4{}; // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VCubicBezierData) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
inline VCubicBezierData::VCubicBezierData(const VPointF &p1, const VPointF &p2, const VPointF &p3, const VPointF &p4)
  : p1(p1),
    p2(p2),
    p3(p3),
    p4(p4)
{
}

QT_WARNING_POP

#endif // VCUBICBEZIER_P_H
