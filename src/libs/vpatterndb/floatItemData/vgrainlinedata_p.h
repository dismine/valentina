/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 2, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#ifndef VGRAINLINEDATA_P_H
#define VGRAINLINEDATA_P_H

#include <QPointF>
#include <QSharedData>

#include "../vmisc/defglobal.h"
#include "../vmisc/typedef.h"
#include "floatitemdef.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VGrainlineDataPrivate : public QSharedData
{
public:
    VGrainlineDataPrivate() = default;
    VGrainlineDataPrivate(const VGrainlineDataPrivate &data) = default;
    ~VGrainlineDataPrivate() = default;

    /** @brief m_dLength formula to calculate the length of grainline */
    QString m_qsLength{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief m_dRotation formula to calculate the rotation of grainline in [degrees] */
    QString m_dRotation{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief m_eArrowType type of arrow on the grainline */
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    GrainlineArrowDirection m_eArrowType{GrainlineArrowDirection::twoWaysUpDown};

    /** @brief m_centerPin center pin id */
    quint32 m_centerPin{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief m_topPin top pin id */
    quint32 m_topPin{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief m_bottomPin bottom pin id */
    quint32 m_bottomPin{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief m_visible visibility flag */
    bool m_visible{true}; // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VGrainlineDataPrivate) // NOLINT
};

QT_WARNING_POP

#endif // VGRAINLINEDATA_P_H
