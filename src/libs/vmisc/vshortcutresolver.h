/************************************************************************
 **
 **  @file   vshortcutresolver.h
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
#ifndef VSHORTCUTRESOLVER_H
#define VSHORTCUTRESOLVER_H

#include <QHash>
#include <QKeySequence>
#include <QList>
#include <QMetaType>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QVariant>

#include <optional>

// One action's built-in binding plus whatever common.ini currently has for it.
// previousDefaults is set only while migrating an action whose built-in binding changed. A saved
// value equal to that old binding is still the shipped default, not a choice the user made.
struct VShortcutPreference
{
    QStringList defaults{};
    std::optional<QStringList> previousDefaults{};
    bool isStored{false};
    QStringList stored{};
};

struct VShortcutResolution
{
    QStringList shortcuts{};
    bool persist{false};
};

// What to write for one shortcut key. QSettings stores an empty QStringList as @Invalid().
// A blank `key=` is a different value (one empty string) and is not how an empty shortcut is saved.
enum class VShortcutSave
{
    WriteSequences,
    WriteExplicitEmpty,
    RemoveKey
};

//---------------------------------------------------------------------------------------------------------------------
// Portable form, empty sequences dropped, order-independent. "ctrl+w" and "Ctrl+W" compare equal, and
// @Invalid() (how QSettings writes an empty shortcut list) compares equal to no shortcut.
inline auto NormalizeShortcutSequences(const QStringList &shortcuts) -> QStringList
{
    QStringList normalized;
    normalized.reserve(shortcuts.size());
    for (const QString &shortcut : shortcuts)
    {
        const QString portable = QKeySequence::fromString(shortcut).toString(QKeySequence::PortableText);
        if (!portable.isEmpty())
        {
            normalized.append(portable);
        }
    }

    normalized.removeDuplicates();
    normalized.sort();
    return normalized;
}

//---------------------------------------------------------------------------------------------------------------------
// @Invalid() reads back as an invalid variant or as an empty list. A blank `key=` reads back as one
// empty string. Both are "no shortcut". schemeversion is an int and is not a placeholder.
inline auto IsEmptyShortcutPlaceholder(const QVariant &value) -> bool
{
    if (!value.isValid())
    {
        return true;
    }

    const int type = value.userType();
    if (type != QMetaType::QString && type != QMetaType::QStringList)
    {
        return false;
    }

    return NormalizeShortcutSequences(value.toStringList()).isEmpty();
}

//---------------------------------------------------------------------------------------------------------------------
// Omit the key when the saved binding and the built-in default are both empty, so a missing key keeps
// meaning "use the default". Keep an explicit empty list when the user cleared a real default.
inline auto ShortcutSaveAction(const QStringList &shortcuts, const QStringList &defaultShortcuts) -> VShortcutSave
{
    if (!NormalizeShortcutSequences(shortcuts).isEmpty())
    {
        return VShortcutSave::WriteSequences;
    }

    if (NormalizeShortcutSequences(defaultShortcuts).isEmpty())
    {
        return VShortcutSave::RemoveKey;
    }

    return VShortcutSave::WriteExplicitEmpty;
}

//---------------------------------------------------------------------------------------------------------------------
// Unknown [shortcuts] keys with no sequences. Known actions are decided from their built-in default,
// and schemeversion is never a shortcut.
inline auto EmptyShortcutPlaceholderKeys(const QStringList &keys, const QHash<QString, QVariant> &values,
                                         const QStringList &knownNames) -> QStringList
{
    QStringList drop;
    for (const QString &key : keys)
    {
        if (key == QStringLiteral("schemeversion") || knownNames.contains(key))
        {
            continue;
        }

        const auto it = values.constFind(key);
        if (it != values.cend() && IsEmptyShortcutPlaceholder(it.value()))
        {
            drop.append(key);
        }
    }

    return drop;
}

//---------------------------------------------------------------------------------------------------------------------
// Sequences that more than one action would claim. Empty bindings are not sequences.
inline auto ConflictingShortcutSequences(const QList<QStringList> &bindings) -> QStringList
{
    QHash<QString, int> counts;
    for (const QStringList &binding : bindings)
    {
        for (const QString &sequence : NormalizeShortcutSequences(binding))
        {
            counts[sequence] += 1;
        }
    }

    QStringList conflicts;
    for (auto it = counts.cbegin(); it != counts.cend(); ++it)
    {
        if (it.value() > 1)
        {
            conflicts.append(it.key());
        }
    }

    conflicts.sort();
    return conflicts;
}

//---------------------------------------------------------------------------------------------------------------------
// Saved shortcuts that differ from the previous built-in binding stay as the user left them. A saved
// copy of the old built-in binding is replaced by the new one. If that new binding uses a key the user
// gave to some other action, the new binding is skipped so the user's key keeps doing what they chose.
inline auto ResolveShortcutPreferences(const QList<VShortcutPreference> &preferences) -> QList<VShortcutResolution>
{
    const int count = static_cast<int>(preferences.size());
    QList<VShortcutResolution> resolved(count);
    QList<bool> isCustom(count, false);
    QList<bool> blocked(count, false);

    for (int i = 0; i < count; ++i)
    {
        const VShortcutPreference &preference = preferences.at(i);
        const QStringList stored = NormalizeShortcutSequences(preference.stored);
        const QStringList defaults = NormalizeShortcutSequences(preference.defaults);
        const bool matchesDefault = stored == defaults;
        const bool matchesPrevious = preference.previousDefaults.has_value()
                                      && stored == NormalizeShortcutSequences(*preference.previousDefaults);

        if (!preference.isStored)
        {
            resolved[i].shortcuts = preference.defaults;
        }
        else if (matchesPrevious && !matchesDefault)
        {
            resolved[i].shortcuts = preference.defaults;
            resolved[i].persist = true;
        }
        else
        {
            resolved[i].shortcuts = preference.stored;
            isCustom[i] = !matchesDefault;
        }
    }

    QSet<QString> taken;
    for (int i = 0; i < count; ++i)
    {
        if (!isCustom.at(i))
        {
            continue;
        }

        for (const QString &sequence : NormalizeShortcutSequences(resolved.at(i).shortcuts))
        {
            taken.insert(sequence);
        }
    }

    for (int i = 0; i < count; ++i)
    {
        if (isCustom.at(i))
        {
            continue;
        }

        bool overlaps = false;
        const QStringList sequences = NormalizeShortcutSequences(resolved.at(i).shortcuts);
        for (const QString &sequence : sequences)
        {
            if (taken.contains(sequence))
            {
                overlaps = true;
                break;
            }
        }

        if (overlaps)
        {
            blocked[i] = true;
            continue;
        }

        for (const QString &sequence : sequences)
        {
            taken.insert(sequence);
        }
    }

    for (int i = 0; i < count; ++i)
    {
        if (!blocked.at(i))
        {
            continue;
        }

        // The new default collides with a shortcut the user set. Keep the old saved binding when it is
        // still free; otherwise leave the action unbound rather than firing two actions from one key.
        const QStringList fallback = preferences.at(i).isStored ? preferences.at(i).stored : QStringList{};
        bool fallbackOverlaps = false;
        for (const QString &sequence : NormalizeShortcutSequences(fallback))
        {
            if (taken.contains(sequence))
            {
                fallbackOverlaps = true;
                break;
            }
        }

        if (fallbackOverlaps)
        {
            resolved[i].shortcuts.clear();
        }
        else
        {
            resolved[i].shortcuts = fallback;
            for (const QString &sequence : NormalizeShortcutSequences(fallback))
            {
                taken.insert(sequence);
            }
        }

        resolved[i].persist = preferences.at(i).isStored
                              && NormalizeShortcutSequences(resolved.at(i).shortcuts)
                                     != NormalizeShortcutSequences(preferences.at(i).stored);
    }

    return resolved;
}

#endif // VSHORTCUTRESOLVER_H
