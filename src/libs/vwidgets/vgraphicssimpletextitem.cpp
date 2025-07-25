/************************************************************************
 **
 **  @file   vgraphicssimpletextitem.cpp
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

#include "vgraphicssimpletextitem.h"

#include <QEvent>
#include <QFlags>
#include <QFont>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QKeyEvent>
#include <QList>
#include <QMessageLogger>
#include <QPalette>
#include <QPoint>
#include <QPolygonF>
#include <QRectF>

#include "../vmisc/literals.h"
#include "../vmisc/vabstractvalapplication.h"
#include "global.h"
#include "theme/vscenestylesheet.h"
#include "vmaingraphicsview.h"
#include "vscenepoint.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VGraphicsSimpleTextItem default constructor.
 * @param parent parent object.
 */
VGraphicsSimpleTextItem::VGraphicsSimpleTextItem(VColorRole textColor, VColorRole textHoverColor, QGraphicsItem *parent)
  : QGraphicsSimpleTextItem(parent),
    m_textColor(textColor),
    m_textHoverColor(textHoverColor)
{
    Init();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VGraphicsSimpleTextItem constructor.
 * @param text text.
 * @param parent parent object.
 */
VGraphicsSimpleTextItem::VGraphicsSimpleTextItem(const QString &text, VColorRole textColor, VColorRole textHoverColor,
                                                 QGraphicsItem *parent)
  : QGraphicsSimpleTextItem(text, parent),
    m_textColor(textColor),
    m_textHoverColor(textHoverColor)
{
    Init();
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsSimpleTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    auto UpdateLine = [this]()
    {
        if (auto *parent = dynamic_cast<VScenePoint *>(parentItem()))
        {
            parent->RefreshLine();
        }
    };

    RefreshColor();

    QFont font = this->font();
    if (font.pointSize() != VAbstractApplication::VApp()->Settings()->GetPatternLabelFontSize())
    {
        font.setPointSize(qMax(VAbstractApplication::VApp()->Settings()->GetPatternLabelFontSize(),
                               static_cast<int>(minVisibleFontSize)));
        setFont(font);
    }

    QGraphicsScene *scene = this->scene();
    const qreal scale = SceneScale(scene);
    if (scale > 1 && not VFuzzyComparePossibleNulls(m_oldScale, scale))
    {
        setScale(1 / scale);
        CorrectLabelPosition();
        UpdateLine();
        m_oldScale = scale;
    }
    else if (scale <= 1 && not VFuzzyComparePossibleNulls(m_oldScale, 1.0))
    {
        setScale(1);
        CorrectLabelPosition();
        UpdateLine();
        m_oldScale = 1;
    }

    if (QGraphicsView *view = scene->views().at(0))
    {
        VMainGraphicsView::NewSceneRect(scene, view, this);
    }

    if (font.pointSize() * scale < minVisibleFontSize)
    {
        return; // smaller font size will cause problems on Windows
    }

    PaintWithFixItemHighlightSelected<QGraphicsSimpleTextItem>(this, painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsSimpleTextItem::SetEnabledState(bool enabled)
{
    QGraphicsSimpleTextItem::setEnabled(enabled);
    if (scene() != nullptr)
    {
        setBrush(scene()->palette().brush(enabled ? QPalette::Active : QPalette::Disabled, QPalette::Text));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsSimpleTextItem::LabelSelectionType(const SelectionType &type)
{
    selectionType = type;
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsSimpleTextItem::SetShowParentTooltip(bool show)
{
    m_showParentTooltip = show;
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsSimpleTextItem::SetRealPos(const QPointF &pos)
{
    m_realPos = pos;

    CorrectLabelPosition();
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsSimpleTextItem::CorrectLabelPosition()
{
    const qreal scale = SceneScale(scene());
    QPointF newPos = m_realPos;

    if (scale > 1)
    {
        QLineF line(m_destination, m_realPos);
        line.setLength(line.length() / scale);
        newPos = line.p2();
    }

    blockSignals(true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    setPos(newPos);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief itemChange handle item change.
 * @param change change.
 * @param value value.
 * @return value.
 */
auto VGraphicsSimpleTextItem::itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant
{
    if (change == ItemPositionChange && scene())
    {
        // Each time we move something we call recalculation scene rect. In some cases this can cause moving
        // objects positions. And this cause infinite redrawing. That's why we wait the finish of saving the last move.
        static bool changeFinished = true;
        if (changeFinished)
        {
            changeFinished = false;
            if (scene())
            {
                const QList<QGraphicsView *> viewList = scene()->views();
                if (not viewList.isEmpty())
                {
                    if (auto *view = qobject_cast<VMainGraphicsView *>(viewList.at(0)))
                    {
                        view->EnsureItemVisibleWithDelay(this, VMainGraphicsView::scrollDelay);
                    }
                }
            }

            m_realPos = value.toPointF();
            const qreal scale = SceneScale(scene());
            if (scale > 1)
            {
                QLineF line(QPointF(), m_realPos);
                line.setLength(line.length() * scale);
                m_realPos = line.p2();
            }
            emit NameChangePosition(m_realPos + this->parentItem()->pos());

            changeFinished = true;
        }
    }
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        setFlag(QGraphicsItem::ItemIsFocusable, value.toBool());
        emit PointSelected(value.toBool());
    }
    return QGraphicsSimpleTextItem::itemChange(change, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief hoverEnterEvent handle hover enter events.
 * @param event hover enter event.
 */
void VGraphicsSimpleTextItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
    }
    else
    {
        setCursor(VAbstractValApplication::VApp()->getSceneView()->viewport()->cursor());
    }

    m_hoverFlag = true;
    setBrush(Qt::green);

    QGraphicsItem *parent = parentItem();
    if (parent && m_showParentTooltip)
    {
        setToolTip(parent->toolTip());
    }

    QGraphicsSimpleTextItem::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief hoverLeaveEvent handle hover leave events.
 * @param event hover leave event.
 */
void VGraphicsSimpleTextItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_hoverFlag = false;
    QGraphicsSimpleTextItem::hoverLeaveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief contextMenuEvent handle context menu events.
 * @param event context menu event.
 */
void VGraphicsSimpleTextItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    emit ShowContextMenu(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsSimpleTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Special for not selectable item first need to call standard mousePressEvent then accept event
    QGraphicsSimpleTextItem::mousePressEvent(event);

    // Somehow clicking on notselectable object do not clean previous selections.
    if (not(flags() & ItemIsSelectable) && scene())
    {
        scene()->clearSelection();
    }

    if (flags() & QGraphicsItem::ItemIsMovable && event->button() == Qt::LeftButton &&
        event->type() != QEvent::GraphicsSceneMouseDoubleClick)
    {
        SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
        event->accept();
    }
    if (selectionType == SelectionType::ByMouseRelease)
    {
        event->accept(); // This help for not selectable items still receive mouseReleaseEvent events
    }
    else
    {
        if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
        {
            emit PointChoosed();
            event->accept();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsSimpleTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (flags() & QGraphicsItem::ItemIsMovable && event->button() == Qt::LeftButton &&
        event->type() != QEvent::GraphicsSceneMouseDoubleClick)
    {
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
    }

    if (selectionType == SelectionType::ByMouseRelease && IsSelectedByReleaseEvent(this, event))
    {
        emit PointChoosed();
    }

    QGraphicsSimpleTextItem::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsSimpleTextItem::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Delete:
            emit DeleteTool();
            return; // Leave this method immediately after call!!!
        default:
            break;
    }
    QGraphicsSimpleTextItem::keyReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsSimpleTextItem::RefreshColor()
{
    setBrush(VSceneStylesheet::Color(m_hoverFlag ? m_textHoverColor : m_textColor));
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsSimpleTextItem::Init()
{
    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    this->setFlag(QGraphicsItem::ItemIsSelectable, true);
    this->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    this->setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus
    this->setAcceptHoverEvents(true);
    QFont font = this->font();
    font.setPointSize(qMax(VAbstractApplication::VApp()->Settings()->GetPatternLabelFontSize(), 1));
    setFont(font);
    m_oldScale = minVisibleFontSize / VAbstractApplication::VApp()->Settings()->GetPatternLabelFontSize();
    setScale(m_oldScale);
}
