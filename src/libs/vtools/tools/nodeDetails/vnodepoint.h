/************************************************************************
 **
 **  @file   vnodepoint.h
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

#ifndef VNODEPOINT_H
#define VNODEPOINT_H

#include <QGraphicsEllipseItem>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "../vwidgets/vscenepoint.h"
#include "vabstractnode.h"

/**
 * @brief The VNodePoint class point detail node.
 */
class VNodePoint : public VAbstractNode, public VScenePoint
{
    Q_OBJECT // NOLINT

public:
    ~VNodePoint() override = default;

    static void Create(const VAbstractNodeInitData &initData);

    static const QString ToolType;
    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Tool::NodePoint)
    };
    auto getTagName() const -> QString override;

    void ChangeLabelPosition(quint32 id, const QPointF &pos) override;
    void SetLabelVisible(quint32 id, bool visible) override;
signals:
    void ShowOptions();
    void ToggleInLayout(bool checked);
    void ToggleForbidFlipping(bool checked);
    void ToggleForceFlipping(bool checked);
    void Delete();
    void ToggleExcludeState(quint32 id);
    void ToggleTurnPointState(quint32 id);
    void ToggleSeamAllowanceAngleType(quint32 id, PieceNodeAngle type);
    void TogglePassmark(quint32 id, bool toggle);
    void TogglePassmarkAngleType(quint32 id, PassmarkAngleType type);
    void TogglePassmarkLineType(quint32 id, PassmarkLineType type);
    void ResetPieceLabelTemplate();
public slots:
    void FullUpdateFromFile() override;
    void NameChangePosition(const QPointF &pos);
    void PointChoosed();
    void EnableToolMove(bool move);
    void AllowHover(bool enabled) override;
    void AllowSelecting(bool enabled) override;
    void AllowLabelHover(bool enabled);
    void AllowLabelSelecting(bool enabled);

protected:
    void AddToFile() override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void ShowNode() override;
    void HideNode() override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VNodePoint) // NOLINT

    explicit VNodePoint(const VAbstractNodeInitData &initData, QObject *qoParent = nullptr,
                        QGraphicsItem *parent = nullptr);

    auto InitContextMenu(QMenu *menu, vidtype pieceId, quint32 referens) -> QHash<int, QAction *>;
    void InitPassmarkMenu(QMenu *menu, vidtype pieceId, QHash<int, QAction *> &contextMenu);
    void InitAngleTypeMenu(QMenu *menu, vidtype pieceId, QHash<int, QAction *> &contextMenu);
    void InitPassmarkAngleTypeMenu(QMenu *menu, vidtype pieceId, QHash<int, QAction *> &contextMenu);
};

#endif // VNODEPOINT_H
