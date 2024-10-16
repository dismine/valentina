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

#ifndef VABSTRACTFLOATITEMDATA_P_H
#define VABSTRACTFLOATITEMDATA_P_H

#include <QPointF>
#include <QSharedData>

#include "../vmisc/defglobal.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VAbstractFloatItemDataPrivate : public QSharedData
{
public:
    VAbstractFloatItemDataPrivate() = default;
    VAbstractFloatItemDataPrivate(const VAbstractFloatItemDataPrivate &data) = default;
    ~VAbstractFloatItemDataPrivate() = default;

    /** @brief m_ptPos position of label's top left corner */
    QPointF m_ptPos{}; // NOLINT(misc-non-private-member-variables-in-classes)
    /** @brief m_bEnabled activity flag */
    bool m_bEnabled{false}; // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VAbstractFloatItemDataPrivate) // NOLINT
};

QT_WARNING_POP

#endif // VABSTRACTFLOATITEMDATA_P_H
