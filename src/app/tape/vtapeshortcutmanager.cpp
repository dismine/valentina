/************************************************************************
 **
 **  @file   vtapeshortcutmanager.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 10, 2023
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
#include "vtapeshortcutmanager.h"

VTapeShortcutManager::VTapeShortcutManager(QObject *parent)
  : VAbstractShortcutManager{parent}
{
    QT_WARNING_PUSH
#if !defined(Q_OS_MACOS) && defined(Q_CC_CLANG)
    QT_WARNING_DISABLE_CLANG("-Wenum-enum-conversion")
#endif

    AddShortcut({VShortcutAction::New, KeyBindingsToStringList(QKeySequence::New), {}});
    AddShortcut({VShortcutAction::Save, KeyBindingsToStringList(QKeySequence::Save), {}});
    AddShortcut({VShortcutAction::SaveAs, KeyBindingsToStringList(QKeySequence::SaveAs), {}});
    AddShortcut({VShortcutAction::Quit, KeyBindingsToStringList(QKeySequence::Quit), {}});
    AddShortcut({VShortcutAction::CaseSensitiveMatch,
                 {QKeySequence(Qt::AltModifier | Qt::Key_C).toString()},
                 {}});
    AddShortcut(
        {VShortcutAction::WholeWordMatch, {QKeySequence(Qt::AltModifier | Qt::Key_W).toString()}, {}});
    AddShortcut(
        {VShortcutAction::RegexMatch, {QKeySequence(Qt::AltModifier | Qt::Key_X).toString()}, {}});
    AddShortcut(
        {VShortcutAction::SearchHistory, {QKeySequence(Qt::AltModifier | Qt::Key_Down).toString()}, {}});
    AddShortcut({VShortcutAction::RegexMatchUnicodeProperties,
                 {QKeySequence(Qt::AltModifier | Qt::Key_U).toString()},
                 {}});
    AddShortcut({VShortcutAction::FindNext, {QKeySequence(Qt::Key_F3).toString()}, {}});
    AddShortcut(
        {VShortcutAction::FindPrevious, {QKeySequence(Qt::ShiftModifier | Qt::Key_F3).toString()}, {}});

    QT_WARNING_POP
}
