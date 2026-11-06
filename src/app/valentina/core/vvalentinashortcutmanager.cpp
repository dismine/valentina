/************************************************************************
 **
 **  @file   vvalentinashortcutmanager.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 10, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#include "vvalentinashortcutmanager.h"

#include <QKeySequence>
#include <QList>
#include <QStringList>

#include "vabstractapplication.h"
#include "vcommonsettings.h"
#include "vshortcutresolver.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
VValentinaShortcutManager::VValentinaShortcutManager(QObject *parent)
  : VAbstractShortcutManager{parent}
{
    Q_STATIC_ASSERT_X(static_cast<int>(VShortcutAction::LAST_ONE_DO_NOT_USE) == 90, "Check if all actions were used.");

    const QList<VSShortcut> defaults = DefaultShortcuts();
    QList<QStringList> bindings;
    bindings.reserve(defaults.size());
    for (const VSShortcut &shortcut : defaults)
    {
        AddShortcut(shortcut);
        bindings.append(shortcut.defaultShortcuts);
    }

    // Two actions on one key both fire. Fail here instead of shipping that.
    const QStringList conflicts = ConflictingShortcutSequences(bindings);
    Q_ASSERT_X(conflicts.isEmpty(), Q_FUNC_INFO, qPrintable(conflicts.join(", "_L1)));
    Q_UNUSED(conflicts)
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaShortcutManager::UpdateShortcuts()
{
    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    const int storedScheme = settings->GetShortcutScheme();
    // Binding upgrades stay on scheme 1. Placeholder cleanup is scheme 2 and must not run those upgrades
    // again, or a tool the user cleared after the upgrade (stored as @Invalid()) would come back.
    const bool migrateBindings = storedScheme < bindingUpgradeScheme;
    const bool clearPlaceholders = storedScheme < shortcutScheme;

    QList<VShortcutPreference> preferences;
    preferences.reserve(m_shortcutsList.size());
    QStringList knownNames;
    knownNames.reserve(m_shortcutsList.size());
    for (const VSShortcut &shortcut : m_shortcutsList)
    {
        const QString name = ShortcutActionToString(shortcut.type);
        knownNames.append(name);

        VShortcutPreference preference;
        preference.defaults = shortcut.defaultShortcuts;
        if (migrateBindings)
        {
            preference.previousDefaults = shortcut.previousDefaults;
        }

        preference.isStored = settings->HasActionShortcut(name);
        if (preference.isStored)
        {
            preference.stored = settings->GetActionShortcuts(name, {});
        }

        preferences.append(preference);
    }

    const QList<VShortcutResolution> resolved = ResolveShortcutPreferences(preferences);
    for (int i = 0; i < m_shortcutsList.size(); ++i)
    {
        const QString name = knownNames.at(i);
        QStringList shortcuts = resolved.at(i).shortcuts;
        // A blank ini value is one empty string. That is not a shortcut.
        if (NormalizeShortcutSequences(shortcuts).isEmpty())
        {
            shortcuts.clear();
        }

        m_shortcutsList[i].shortcuts = shortcuts;

        const QStringList defaults = m_shortcutsList.at(i).defaultShortcuts;
        if (resolved.at(i).persist)
        {
            settings->SetActionShortcuts(name, shortcuts, defaults);
            continue;
        }

        if (!clearPlaceholders || !preferences.at(i).isStored || !shortcuts.isEmpty())
        {
            continue;
        }

        if (NormalizeShortcutSequences(defaults).isEmpty())
        {
            // Still unbound. @Invalid() here only repeats the built-in default, so drop the key.
            settings->RemoveActionShortcut(name);
            continue;
        }

        // Cleared on purpose. A blank `key=` has to become an empty list, which is stored as @Invalid().
        if (!preferences.at(i).stored.isEmpty())
        {
            settings->SetActionShortcuts(name, {}, defaults);
        }
    }

    if (clearPlaceholders)
    {
        settings->RemoveEmptyShortcutPlaceholders(knownNames);
        // Missing keys keep following the built-in binding. A shortcut cleared after this stays cleared.
        settings->SetShortcutScheme(shortcutScheme);
    }

    emit ShortcutsUpdated();
}
