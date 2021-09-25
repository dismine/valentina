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
#include "../ifc/ifcdef.h"
#include "../vmisc/diagnostic.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VGObjectData final : public QSharedData
{
public:
    VGObjectData()
    {}

    VGObjectData(const GOType &type, const quint32 &idObject, const Draw &mode)
        :type(type),
         idObject(idObject),
         mode(mode)
    {}

    VGObjectData(const VGObjectData &obj)
        :QSharedData(obj),
        _id(obj._id),
        type(obj.type),
        idObject(obj.idObject),
        _name(obj._name),
        mode(obj.mode),
        m_alias(obj.m_alias),
        m_aliasSuffix(obj.m_aliasSuffix)
    {}

    virtual ~VGObjectData();

    /** @brief _id id in container. Ned for arcs, spline and spline paths. */
    quint32 _id{NULL_ID};

    /** @brief type type of graphical object */
    GOType  type{GOType::Unknown};

    /** @brief idObject id of parent object. */
    quint32 idObject{NULL_ID};

    /** @brief _name object name */
    QString _name{};

    /** @brief mode object created in calculation or drawing mode */
    Draw    mode{Draw::Calculation};

    QString m_alias{};
    QString m_aliasSuffix{};

private:
    Q_DISABLE_ASSIGN(VGObjectData)
};

VGObjectData::~VGObjectData()
{}

QT_WARNING_POP

#endif // VGOBJECT_P_H
