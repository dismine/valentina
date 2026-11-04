/************************************************************************
 **
 **  @file   vmaingraphicsscene.cpp
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

#include "vmaingraphicsscene.h"

#include <QBrush>
#include <QEvent>
#include <QGraphicsItem>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsView>
#include <QLineF>
#include <QPen>

#include "global.h"

namespace
{
// Marks scene items that make up the origin (axis) decoration, so SetOriginsVisible() can find them by
// querying the live scene instead of caching raw pointers that dangle once the items are deleted.
constexpr int originItemData = 0;
} // namespace

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VMainGraphicsScene default constructor.
 */
VMainGraphicsScene::VMainGraphicsScene(QObject *parent)
  : QGraphicsScene(parent),
    horScrollBar(0),
    verScrollBar(0),
    _transform(QTransform()),
    scenePos(QPointF())
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VMainGraphicsScene constructor.
 * @param sceneRect scene rect.
 * @param parent parent object.
 */
VMainGraphicsScene::VMainGraphicsScene(const QRectF &sceneRect, QObject *parent)
  : QGraphicsScene(sceneRect, parent),
    horScrollBar(0),
    verScrollBar(0),
    _transform(QTransform()),
    scenePos()
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief mouseMoveEvent handle mouse move events.
 * @param event mouse move event.
 */
void VMainGraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    scenePos = event->scenePos();
    emit mouseMove(event->scenePos());
    QGraphicsScene::mouseMoveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief mousePressEvent mouse press events.
 * @param event mouse press event
 */
void VMainGraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
    {
        emit MouseLeftPressed();

        QTransform deviceTransform;
        auto *view = qobject_cast<QGraphicsView *>(event->widget());
        if (view != nullptr)
        {
            deviceTransform = view->transform();
        }
        emit ItemByMousePress(itemAt(event->scenePos(), deviceTransform));
    }

    QGraphicsScene::mousePressEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
    {
        emit MouseLeftReleased();

        QTransform deviceTransform;
        auto *view = qobject_cast<QGraphicsView *>(event->widget());
        if (view != nullptr)
        {
            deviceTransform = view->transform();
        }
        emit ItemByMouseRelease(itemAt(event->scenePos(), deviceTransform));
    }
    QGraphicsScene::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::SetAcceptDrop(bool newAcceptDrop)
{
    m_acceptDrop = newAcceptDrop;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMainGraphicsScene::AcceptDrop() const -> bool
{
    return m_acceptDrop;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMainGraphicsScene::IsNonInteractive() const -> bool
{
    return m_nonInteractive;
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::SetNonInteractive(bool nonInteractive)
{
    m_nonInteractive = nonInteractive;
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::InitOrigins()
{
    // Draw mode, as drawn today: +X to the right, +Y down the screen.
    PlaceOrigins(QPointF(1, 0), QPointF(0, 1), -0.5);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::PlaceOrigins(const QPointF &xDirection, const QPointF &yDirection, qreal zValue)
{
    const QList<QGraphicsItem *> sceneItems = items();
    for (auto *item : sceneItems)
    {
        if (item != nullptr && item->parentItem() == nullptr && item->data(originItemData).toBool())
        {
            removeItem(item);
            delete item;
        }
    }

    auto AddAxis = [this, zValue](const QPointF &direction, const QString &text)
    {
        QPen const originsPen(Qt::green, MmToPixel(1.2 / 3.0), Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        QBrush const axisTextBrush(Qt::green);
        const QPointF tip = direction * 25.0;
        const QPointF tail = direction * -5.0;
        const QLineF shaft(tip, tail);

        auto *line = new QGraphicsLineItem(shaft);
        line->setPen(originsPen);
        line->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        line->setZValue(zValue);
        line->setData(originItemData, true);
        addItem(line);

        auto AddBarb = [&](qreal delta)
        {
            QLineF barb = shaft;
            barb.setAngle(shaft.angle() + delta);
            barb.setLength(12.0);
            auto *barbLine = new QGraphicsLineItem(barb);
            barbLine->setPen(originsPen);
            barbLine->setFlag(QGraphicsItem::ItemIgnoresTransformations);
            barbLine->setZValue(zValue);
            barbLine->setData(originItemData, true);
            addItem(barbLine);
        };
        AddBarb(-35.0);
        AddBarb(35.0);

        auto *label = new QGraphicsSimpleTextItem(text, line);
        label->setBrush(axisTextBrush);
        label->setFlag(QGraphicsItem::ItemIgnoresTransformations);
        label->setZValue(zValue);
        label->setData(originItemData, true);
        const QRectF bounds = label->boundingRect();
        if (qAbs(direction.x()) >= qAbs(direction.y()))
        {
            label->setPos(direction.x() * 30.0, -bounds.height() / 2.0);
        }
        else
        {
            label->setPos(-bounds.width() / 2.0, direction.y() * 30.0);
        }
    };

    AddAxis(xDirection, QStringLiteral("X"));
    AddAxis(yDirection, QStringLiteral("Y"));
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::SetOriginsVisible(bool visible) const
{
    // Derive the origin items from the live scene instead of a cache, so items deleted by clear() can never
    // be dereferenced here.
    const QList<QGraphicsItem *> sceneItems = items();
    for (auto *item : sceneItems)
    {
        if (item != nullptr && item->data(originItemData).toBool())
        {
            item->setVisible(visible);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMainGraphicsScene::getScenePos() const -> QPointF
{
    return scenePos;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMainGraphicsScene::VisibleItemsBoundingRect() const -> QRectF
{
    QRectF rect;
    const QList<QGraphicsItem *> qItems = items();
    for (auto *item : qItems)
    {
        if (not item->isVisible())
        {
            continue;
        }
        rect = rect.united(item->sceneBoundingRect());
    }
    return rect;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief transform return view transformation.
 * @return view transformation.
 */
auto VMainGraphicsScene::transform() const -> QTransform
{
    return _transform;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setTransform set view transformation.
 * @param transform view transformation.
 */
void VMainGraphicsScene::setTransform(const QTransform &transform)
{
    _transform = transform;
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::EnableTools()
{
    emit EnableItem();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ChoosedItem emit ChoosedObject signal.
 * @param id object id.
 * @param type object scene type.
 */
void VMainGraphicsScene::ChoosedItem(quint32 id, const SceneObject &type)
{
    emit ChoosedObject(id, type);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::SelectedItem(bool selected, quint32 object, quint32 tool)
{
    emit SelectedObject(selected, object, tool);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::EnableItemMove(bool move)
{
    emit EnableToolMove(move);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::EnableDetailsMode(bool mode)
{
    emit CurveDetailsMode(mode);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ItemsSelection(const SelectionType &type)
{
    emit ItemSelection(type);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::HighlightItem(quint32 id)
{
    emit HighlightDetail(id);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::UpdatePiecePassmarks()
{
    emit UpdatePassmarks();
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleLabelSelection(bool enabled)
{
    emit EnableLabelItemSelection(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::TogglePointSelection(bool enabled)
{
    emit EnablePointItemSelection(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleLineSelection(bool enabled)
{
    emit EnableLineItemSelection(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleArcSelection(bool enabled)
{
    emit EnableArcItemSelection(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleElArcSelection(bool enabled)
{
    emit EnableElArcItemSelection(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleSplineSelection(bool enabled)
{
    emit EnableSplineItemSelection(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleSplinePathSelection(bool enabled)
{
    emit EnableSplinePathItemSelection(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleNodeLabelSelection(bool enabled)
{
    emit EnableNodeLabelItemSelection(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleNodePointSelection(bool enabled)
{
    emit EnableNodePointItemSelection(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleDetailSelection(bool enabled)
{
    emit EnableDetailItemSelection(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleLabelHover(bool enabled)
{
    emit EnableLabelItemHover(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::TogglePointHover(bool enabled)
{
    emit EnablePointItemHover(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleLineHover(bool enabled)
{
    emit EnableLineItemHover(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleArcHover(bool enabled)
{
    emit EnableArcItemHover(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleElArcHover(bool enabled)
{
    emit EnableElArcItemHover(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleSplineHover(bool enabled)
{
    emit EnableSplineItemHover(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleSplinePathHover(bool enabled)
{
    emit EnableSplinePathItemHover(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleNodeLabelHover(bool enabled)
{
    emit EnableNodeLabelItemHover(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleNodePointHover(bool enabled)
{
    emit EnableNodePointItemHover(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleDetailHover(bool enabled)
{
    emit EnableDetailItemHover(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleArcSegmentVisibility(bool visible)
{
    emit ShowArcSegmentLabel(visible);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleElArcSegmentVisibility(bool visible)
{
    emit ShowElArcSegmentLabel(visible);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleSplineSegmentVisibility(bool visible)
{
    emit ShowSplineSegmentLabel(visible);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsScene::ToggleSplinePathSegmentVisibility(bool visible)
{
    emit ShowSplinePathSegmentLabel(visible);
}
