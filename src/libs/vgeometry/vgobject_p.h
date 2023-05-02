/************************************************************************
 **
 **  @file   vgobject_p.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 8, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#ifndef VGOBJECT_P_H
#define VGOBJECT_P_H

#include <QSharedData>
#include "vgeometrydef.h"
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)

#include "../vmisc/typedef.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VGObjectData final : public QSharedData
{
public:
    VGObjectData() = default;
    VGObjectData(const GOType &type, const quint32 &idObject, const Draw &mode)
        :type(type),
         idObject(idObject),
         mode(mode)
    {}

    VGObjectData(const VGObjectData &obj) = default;
    virtual ~VGObjectData() = default;

    /** @brief _id id in container. Ned for arcs, spline and spline paths. */
    quint32 _id{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief type type of graphical object */
    GOType  type{GOType::Unknown}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief idObject id of parent object. */
    quint32 idObject{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief _name object name */
    QString _name{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief mode object created in calculation or drawing mode */
    Draw    mode{Draw::Calculation}; // NOLINT(misc-non-private-member-variables-in-classes)

    QString m_alias{}; // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_aliasSuffix{}; // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VGObjectData) // NOLINT
};

QT_WARNING_POP

#endif // VGOBJECT_P_H
