/************************************************************************
 **
 **  @file   vnodedetail.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#ifndef VNODEDETAIL_H
#define VNODEDETAIL_H

#include <QMetaType>
#include <QSharedDataPointer>
#include <QTypeInfo>
#include <QtGlobal>

#include "../vmisc/def.h"

class VNodeDetailData;
class VPieceNode;
class VContainer;

/**
 * @brief The VNodeDetail class keep information about detail node.
 */
class VNodeDetail
{
public:
    /**
     * @brief VNodeDetail default constructor.
     */
    VNodeDetail();
    /**
     * @brief VNodeDetail constructor.
     * @param id object id
     * @param typeTool type tool
     * @param typeNode type node
     * @param mx object bias x axis
     * @param my object bias y axis
     */
    VNodeDetail(quint32 id, Tool typeTool, NodeDetail typeNode, qreal mx = 0, qreal my = 0, bool reverse = false);
    /**
     * @brief VNodeDetail copy constructor
     * @param node node
     */
    VNodeDetail(const VNodeDetail &node);

    ~VNodeDetail();

    /**
     * @brief operator = assignment operator
     * @param node node
     * @return node
     */
    auto operator=(const VNodeDetail &node) -> VNodeDetail &;

    VNodeDetail(VNodeDetail &&node) noexcept;
    auto operator=(VNodeDetail &&node) noexcept -> VNodeDetail &;

    /**
     * @brief getId return object id.
     * @return id.
     */
    auto getId() const -> quint32;
    /**
     * @brief setId set object id.
     * @param value object id.
     */
    void setId(const quint32 &value);
    /**
     * @brief getTypeTool return tool type.
     * @return tool type.
     */
    auto getTypeTool() const -> Tool;
    /**
     * @brief setTypeTool set tool type.
     * @param value tool type.
     */
    void setTypeTool(const Tool &value);
    /**
     * @brief getTypeNode return node type.
     * @return node type.
     */
    auto getTypeNode() const -> NodeDetail;
    /**
     * @brief setTypeNode set node type.
     * @param value node type.
     */
    void setTypeNode(const NodeDetail &value);
    /**
     * @brief getMx return object bias x axis.
     * @return bias x axis.
     */
    auto getMx() const -> qreal;
    /**
     * @brief setMx set object bias x axis.
     * @param value bias x axis.
     */
    void setMx(const qreal &value);
    /**
     * @brief getMy return object bias y axis.
     * @return bias y axis.
     */
    auto getMy() const -> qreal;
    /**
     * @brief setMy set object bias y axis.
     * @param value bias y axis.
     */
    void setMy(const qreal &value);

    auto getReverse() const -> bool;
    void setReverse(bool reverse);

    static auto Convert(const VContainer *data, const QVector<VNodeDetail> &nodes, qreal width, bool closed)
        -> QVector<VPieceNode>;

private:
    QSharedDataPointer<VNodeDetailData> d;
};

Q_DECLARE_METATYPE(VNodeDetail)                  // NOLINT
Q_DECLARE_TYPEINFO(VNodeDetail, Q_MOVABLE_TYPE); // NOLINT

#endif // VNODEDETAIL_H
