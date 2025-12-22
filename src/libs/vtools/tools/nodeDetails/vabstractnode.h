/************************************************************************
 **
 **  @file   vabstractnode.h
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

#ifndef VABSTRACTNODE_H
#define VABSTRACTNODE_H

#include <QColor>
#include <QDomElement>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vabstracttool.h"

enum class ParentType : bool
{
    Scene,
    Item
};

struct VAbstractNodeInitData : VAbstractToolInitData
{
    VAbstractNodeInitData() = default;

    quint32 idObject{NULL_ID};
    QString drawName{};
    quint32 idTool{NULL_ID};
    qreal mx{0};
    qreal my{0};
    bool showLabel{true};
};

/**
 * @brief The VAbstractNode class parent class for all detail node.
 */
class VAbstractNode : public VAbstractTool
{
    Q_OBJECT // NOLINT

public:
    VAbstractNode(VAbstractPattern *doc, VContainer *data, const quint32 &id, const quint32 &idNode,
                  const QString &drawName = QString(), const quint32 &idTool = 0, QObject *parent = nullptr);
    virtual ~VAbstractNode() override = default;

    static const QString AttrIdTool;
    void ShowVisualization(bool show) override;
    void incrementReferens() override;
    void decrementReferens() override;

    auto GetParentType() const -> ParentType;
    void SetParentType(const ParentType &value);

    auto GetIdTool() const -> quint32;

    void GroupVisibility(quint32 object, bool visible) override;

    auto IsExluded() const -> bool;
    void SetExluded(bool exluded);

protected:
    ParentType parentType;

    /** @brief idNodenode id. */
    quint32 idNode;

    /** @brief idTool id tool. */
    quint32 idTool;

    QString m_drawName;

    bool m_exluded;

    void AddToModeling(const QDomElement &domElement);
    void ToolCreation(const Source &typeCreation) override;
    void SetVisualization() override {}

    virtual void ShowNode() = 0;
    virtual void HideNode() = 0;

private:
    Q_DISABLE_COPY_MOVE(VAbstractNode) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstractNode::GetIdTool() const -> quint32
{
    return idTool;
}

#endif // VABSTRACTNODE_H
