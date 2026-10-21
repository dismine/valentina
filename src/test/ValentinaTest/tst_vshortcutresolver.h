/************************************************************************
 **
 **  @file   tst_vshortcutresolver.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 10, 2026
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

#ifndef TST_VSHORTCUTRESOLVER_H
#define TST_VSHORTCUTRESOLVER_H

#include <QObject>

class TST_VShortcutResolver : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit TST_VShortcutResolver(QObject *parent = nullptr);

private slots:
    void DefaultsDoNotCollide();
    void ToolDefaultsMatchDraftingSet();
    void DisplacedCommandsKeepABinding();
    void UnchangedActionsAreNotMigrated();
    void MissingKeysUseDefaults();
    void SavedOldDefaultIsUpgraded();
    void SavedCustomShortcutIsKept();
    void CustomKeyBlocksTheNewDefault();
    void ClearedShortcutStaysClearedAfterMigration();
    void InvalidIniValueIsAnEmptyShortcut();
    void BlankIniValueIsNotAShortcut();
    void ExplicitEmptyListIsTheInvalidToken();
    void EmptyPlaceholderKeysSkipKnownActionsAndScheme();
    void SchemeKeepsBindingUpgradeSeparateFromCleanup();
};

#endif // TST_VSHORTCUTRESOLVER_H
