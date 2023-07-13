/************************************************************************
 **
 **  @file   vnodedetail_p.h
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

#ifndef VNODEDETAIL_P_H
#define VNODEDETAIL_P_H

#include <QSharedData>
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)

#include "../vmisc/def.h"
#include "../vmisc/typedef.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VNodeDetailData : public QSharedData
{
public:
    VNodeDetailData() = default;
    VNodeDetailData(quint32 id, Tool typeTool, NodeDetail typeNode, qreal mx, qreal my, bool reverse);
    VNodeDetailData(const VNodeDetailData &node) = default;
    ~VNodeDetailData() = default;

    /**
     * @brief id object id.
     */
    quint32 id{NULL_ID}; // NOLINT(misc-non-private-member-variables-in-classes)
    /**
     * @brief typeTool type of tool
     */
    Tool typeTool{Tool::NodePoint}; // NOLINT(misc-non-private-member-variables-in-classes)
    /**
     * @brief typeNode node type.
     */
    NodeDetail typeNode{NodeDetail::Contour}; // NOLINT(misc-non-private-member-variables-in-classes)
    /**
     * @brief mx bias x axis.
     */
    qreal mx{0}; // NOLINT(misc-non-private-member-variables-in-classes)
    /**
     * @brief my bias y axis.
     */
    qreal my{0}; // NOLINT(misc-non-private-member-variables-in-classes)
    /**
     * @brief reverse true if need reverse points list for node.
     */
    bool reverse{false}; // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VNodeDetailData) // NOLINT
};

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
inline VNodeDetailData::VNodeDetailData(quint32 id, Tool typeTool, NodeDetail typeNode, qreal mx, qreal my,
                                        bool reverse)
  : id(id),
    typeTool(typeTool),
    typeNode(typeNode),
    mx(mx),
    my(my),
    reverse(reverse)
{
}

#endif // VNODEDETAIL_P_H
