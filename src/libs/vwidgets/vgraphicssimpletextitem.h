/************************************************************************
 **
 **  @file   vgraphicssimpletextitem.h
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

#ifndef VGRAPHICSSIMPLETEXTITEM_H
#define VGRAPHICSSIMPLETEXTITEM_H

#include <QGraphicsItem>
#include <QGraphicsSimpleTextItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVariant>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "../vmisc/theme/themeDef.h"

/**
 * @brief The VGraphicsSimpleTextItem class pointer label.
 */
class VGraphicsSimpleTextItem : public QObject, public QGraphicsSimpleTextItem
{
    Q_OBJECT // NOLINT

public:
    explicit VGraphicsSimpleTextItem(VColorRole textColor, VColorRole textHoverColor, QGraphicsItem *parent = nullptr);
    explicit VGraphicsSimpleTextItem(const QString &text, VColorRole textColor, VColorRole textHoverColor,
                                     QGraphicsItem *parent = nullptr);
    ~VGraphicsSimpleTextItem() override = default;

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Vis::GraphicsSimpleTextItem)
    };

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    void SetEnabledState(bool enabled);
    void LabelSelectionType(const SelectionType &type);

    void SetShowParentTooltip(bool show);

    void SetRealPos(const QPointF &pos);

    void SetDestination(const QPointF &destination);

    void RefreshColor();

    auto GetTextColor() const -> VColorRole;
    void SetTextColor(VColorRole newTextColor);

    auto GetTextHoverColor() const -> VColorRole;
    void SetTextHoverColor(VColorRole newTextHoverColor);

signals:
    /**
     * @brief NameChangePosition emit when label change position.
     * @param pos new posotion.
     */
    void NameChangePosition(const QPointF &pos);
    /**
     * @brief ShowContextMenu emit when need show tool context menu.
     * @param event context menu event.
     */
    void ShowContextMenu(QGraphicsSceneContextMenuEvent *event);
    void DeleteTool();
    void PointChoosed();
    void PointSelected(bool selected);

protected:
    auto itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private slots:
    void UpdateFontSize(int size);

private:
    Q_DISABLE_COPY_MOVE(VGraphicsSimpleTextItem) // NOLINT
    SelectionType selectionType{SelectionType::ByMouseRelease};
    qreal m_oldScale{1};
    bool m_showParentTooltip{true};
    QPointF m_realPos{};
    QPointF m_destination{};
    bool m_hoverFlag{false};
    VColorRole m_textColor;
    VColorRole m_textHoverColor;

    void Init();

    void CorrectLabelPosition();

    void UpdateGeometry();
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VGraphicsSimpleTextItem::GetTextHoverColor() const -> VColorRole
{
    return m_textHoverColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VGraphicsSimpleTextItem::SetTextHoverColor(VColorRole newTextHoverColor)
{
    m_textHoverColor = newTextHoverColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VGraphicsSimpleTextItem::GetTextColor() const -> VColorRole
{
    return m_textColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VGraphicsSimpleTextItem::SetTextColor(VColorRole newTextColor)
{
    m_textColor = newTextColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VGraphicsSimpleTextItem::SetDestination(const QPointF &destination)
{
    m_destination = destination;
}

#endif // VGRAPHICSSIMPLETEXTITEM_H
