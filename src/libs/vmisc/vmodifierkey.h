/************************************************************************
 **
 **  @file   vmodifierkey.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 2, 2019
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2019 Valentina project
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
#ifndef VMODIFIERKEY_H
#define VMODIFIERKEY_H

#include <QString>

/**
 * A class that helps to deal efficiently with platform and localization issues
 * of modifier keys.
 *
 * It is based on QKeySequence::toString(QKeySequence::NativeText) which provides
 * localization and deals with swapping Ctrl and Cmd on Mac OS X. In contrast
 * to QKeySequence, VModifierKey has an implicit operator for casting to QString,
 * and it removes the trailing '+' from pseudo key sequences which consist of
 * modifier keys only. Static methods provide efficient translations of the
 * pure modifier keys.
 *
 * For true QKeySequences, call QKeySequence::toString(QKeySequence::NativeText)
 * directly.
 *
 * On Windows and Linux, the keys will be displayed as word ("Shift" etc.).
 * On OS X, the keys will be displayed as graphical symbols ("⇧", i.e.
 * Unicode character U+21E7, etc.).
 *
 * Synopsis:
 *
 *     QString text = tr("%1+Click to add a point.").arg(VModifierKey::control());
 *     QString more = tr("%1+Click to select a point.").arg(VModifierKey(Qt::ALT + Qt::ShiftModifier));
 *
 *     // BUT:
 *     QString help = help_action.shortcut().toString(QKeySequence::NativeText);
 */

class VModifierKey
{
public:
    /** Constructs a new VModifierKey for the given combination of KeyboardModifiers. */
    explicit VModifierKey(Qt::KeyboardModifiers keys);

    /** Constructs a new VModifierKey for the given key. */
    explicit VModifierKey(Qt::Key key);

    /** Returns a string representation for user interface purposes.
     *
     * This operator is intented to be used for implicit type casts. */
    operator QString() const;

    /** Returns a shared Alt modifier key. */
    static auto Alt() -> const VModifierKey &;

    /** Returns a shared Control modifier key. */
    static auto Control() -> const VModifierKey &;

    /** Returns a shared Control+Shift modifier key. */
    static auto ControlShift() -> const VModifierKey &;

    /** Returns a shared Meta modifier key. */
    static auto Meta() -> const VModifierKey &;

    /** Returns a shared Shift modifier key. */
    static auto Shift() -> const VModifierKey &;

    /** Returns a shared Space key. */
    static auto Space() -> const VModifierKey &;

    /** Returns a shared Return key. */
    static auto ReturnKey() -> const VModifierKey &;

    /** Returns a shared Enter key. */
    static auto EnterKey() -> const VModifierKey &;

    /** Returns a shared Backspace key. */
    static auto Backspace() -> const VModifierKey &;

    /** Returns a shared Escape modifier key. */
    static auto Escape() -> const VModifierKey &;

protected:
    /** Constructs a new VModifierKey for the given key. */
    explicit VModifierKey(int key);

private:
    /** The native text (localized, adapted to the system). */
    QString m_nativeText;
};

//---------------------------------------------------------------------------------------------------------------------
inline VModifierKey::operator QString() const
{
    return m_nativeText;
}

#endif // VMODIFIERKEY_H
