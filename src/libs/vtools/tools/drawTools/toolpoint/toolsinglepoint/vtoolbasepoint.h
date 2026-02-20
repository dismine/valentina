/************************************************************************
 **
 **  @file   vtoolsinglepoint.h
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

#ifndef VTOOLBASEPOINT_H
#define VTOOLBASEPOINT_H

#include <QDomElement>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "vtoolsinglepoint.h"

template <class T> class QSharedPointer;

struct VToolBasePointInitData : VToolSinglePointInitData
{
    qreal x{labelMX};
    qreal y{labelMY};
};

/**
 * @brief The VToolBasePoint class tool for creation pattern base point. Only base point can move. All object
 * pattern peace depend on base point.
 */
class VToolBasePoint : public VToolSinglePoint
{
    Q_OBJECT // NOLINT

public:
    ~VToolBasePoint() override = default;
    void SetDialog() override;
    static auto Create(VToolBasePointInitData initData) -> VToolBasePoint *;
    static const QString ToolType;
    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::BasePoint)
    };
    void ShowVisualization(bool show) override;

    auto IsRemovable() const -> RemoveStatus override;

    auto GetBasePointPos() const -> QPointF;
    void SetBasePointPos(const QPointF &pos);
public slots:
    void FullUpdateFromFile() override;
    void EnableToolMove(bool move) override;

protected:
    void AddToFile() override;
    auto itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant override;
    void DeleteToolWithConfirm(bool ask = true) override;
    void SaveDialog(QDomElement &domElement) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj) override;
    void SetVisualization() override {}
    auto MakeToolTip() const -> QString override;
    void ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement) override;

private slots:
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id = NULL_ID) override;

private:
    Q_DISABLE_COPY_MOVE(VToolBasePoint) // NOLINT

    int m_indexPatternBlock{-1};

    explicit VToolBasePoint(const VToolBasePointInitData &initData, QGraphicsItem *parent = nullptr);

    auto GatherToolChanges() const -> ToolChanges;
};

#endif // VTOOLBASEPOINT_H
