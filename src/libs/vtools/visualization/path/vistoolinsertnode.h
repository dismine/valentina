/************************************************************************
 **
 **  @file   vistoolinsertnode.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 4, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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
#ifndef VISTOOLINSERTNODE_H
#define VISTOOLINSERTNODE_H

#include <QObject>

#include "../vpatterndb/vpiecenode.h"
#include "vispath.h"

class VisToolInsertNode : public VisPath
{
    Q_OBJECT // NOLINT

public:
    explicit VisToolInsertNode(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolInsertNode() override = default;

    void VisualMode(quint32 id) override;
    void RefreshGeometry() override;

    void SetNodes(const QVector<quint32> &nodes);

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Vis::ToolInsertNode)
    };

private:
    Q_DISABLE_COPY_MOVE(VisToolInsertNode) // NOLINT

    QVector<VScaledEllipse *> m_points{};
    QVector<VCurvePathItem *> m_curves{};

    QVector<quint32> m_nodes{};

    void HideAllItems();

    auto GetPoint(quint32 i, VColorRole role) -> VScaledEllipse *;
    auto GetCurve(quint32 i, VColorRole role) -> VCurvePathItem *;

    auto CreateNodeCurve(quint32 id, int &i) -> QGraphicsPathItem *;
    auto CreateNodePoint(quint32 id, int &i) -> VScaledEllipse *;
};

#endif // VISTOOLINSERTNODE_H
