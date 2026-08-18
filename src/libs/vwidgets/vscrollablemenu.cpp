/************************************************************************
 **
 **  @file   vscrollablemenu.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 8, 2026
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
#include "vscrollablemenu.h"

#include <QAbstractScrollArea>
#include <QAction>
#include <QCursor>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QListWidget>
#include <QListWidgetItem>
#include <QScreen>
#include <QWidgetAction>

namespace
{
// ponytail: a fixed ratio of the screen the popup appears on is a good-enough cap;
// exact available space depends on where the button ends up positioned, which QMenu
// itself only finalizes after this point.
constexpr qreal maxHeightScreenRatio = 0.7;
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VScrollableMenu::VScrollableMenu(QWidget *parent)
  : QMenu(parent),
    m_listWidget(new QListWidget(this))
{
    m_listWidget->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);
    m_listWidget->setFrameShape(QFrame::NoFrame);
    m_listWidget->installEventFilter(this);

    connect(m_listWidget, &QListWidget::itemClicked, this, &VScrollableMenu::ItemActivated);

    auto *widgetAction = new QWidgetAction(this);
    widgetAction->setDefaultWidget(m_listWidget);
    QMenu::addAction(widgetAction);

    connect(this,
            &QMenu::aboutToShow,
            this,
            [this]()
            {
                if (const QScreen *screen = QGuiApplication::screenAt(QCursor::pos()); screen != nullptr)
                {
                    m_listWidget->setMaximumHeight(qRound(screen->availableGeometry().height() * maxHeightScreenRatio));
                }
                m_listWidget->setFocus();
            });
}

//---------------------------------------------------------------------------------------------------------------------
auto VScrollableMenu::addAction(const QString &text) -> QAction *
{
    auto *action = new QAction(text, this);

    auto *item = new QListWidgetItem();
    m_listWidget->addItem(item);
    m_itemForAction.insert(action, item);
    m_actionForItem.insert(item, action);

    connect(action, &QAction::changed, this, [this, action]() { UpdateItem(action); });
    UpdateItem(action);

    return action;
}

//---------------------------------------------------------------------------------------------------------------------
void VScrollableMenu::clear()
{
    m_listWidget->clear(); // deletes the QListWidgetItem objects
    qDeleteAll(m_itemForAction.keys());
    m_itemForAction.clear();
    m_actionForItem.clear();
}

//---------------------------------------------------------------------------------------------------------------------
auto VScrollableMenu::eventFilter(QObject *watched, QEvent *event) -> bool
{
    if (watched == m_listWidget && event->type() == QEvent::KeyPress)
    {
        if (const auto *keyEvent = static_cast<QKeyEvent *>(event);
            keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            ItemActivated(m_listWidget->currentItem());
            return true;
        }
    }

    return QMenu::eventFilter(watched, event);
}

//---------------------------------------------------------------------------------------------------------------------
void VScrollableMenu::UpdateItem(QAction *action)
{
    QListWidgetItem *item = m_itemForAction.value(action);
    if (item == nullptr)
    {
        return;
    }

    item->setText(action->text());
    item->setToolTip(action->toolTip());
    item->setHidden(!action->isVisible());

    Qt::ItemFlags flags = Qt::ItemIsSelectable;
    if (action->isEnabled())
    {
        flags |= Qt::ItemIsEnabled;
    }
    item->setFlags(flags);
}

//---------------------------------------------------------------------------------------------------------------------
void VScrollableMenu::ItemActivated(QListWidgetItem *item)
{
    if (item == nullptr || !(item->flags() & Qt::ItemIsEnabled))
    {
        return;
    }

    if (QAction *action = m_actionForItem.value(item); action != nullptr)
    {
        action->trigger();
        close();
    }
}
