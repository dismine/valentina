/************************************************************************
 **
 **  @file   vscrollablemenu.h
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
#ifndef VSCROLLABLEMENU_H
#define VSCROLLABLEMENU_H

#include <QHash>
#include <QMenu>

class QListWidget;
class QListWidgetItem;

// A QMenu that always lays out its items in a single, natively scrollable column,
// regardless of platform/style. QMenu's own answer to this ("QMenu { menu-scrollable: 1; }")
// is an undocumented, private style hint that different Qt styles honor inconsistently
// (some ignore it outright), so a long flat action list can still fall back to multiple
// columns. This class sidesteps the style-dependent layout entirely by hosting the items
// in a QListWidget embedded via a single QWidgetAction.
class VScrollableMenu final : public QMenu
{
    Q_OBJECT // NOLINT

public:
    explicit VScrollableMenu(QWidget *parent = nullptr);
    ~VScrollableMenu() override = default;

    auto AddAction(const QString &text) -> QAction *;
    void Clear();

private:
    Q_DISABLE_COPY_MOVE(VScrollableMenu) // NOLINT

    QListWidget *m_listWidget;
    QHash<QAction *, QListWidgetItem *> m_itemForAction{};
    QHash<QListWidgetItem *, QAction *> m_actionForItem{};

    auto eventFilter(QObject *watched, QEvent *event) -> bool override;
    void UpdateItem(QAction *action) const;
    void ItemActivated(QListWidgetItem *item);
};

#endif // VSCROLLABLEMENU_H
