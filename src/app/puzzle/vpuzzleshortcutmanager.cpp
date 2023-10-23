/************************************************************************
 **
 **  @file   vpuzzleshortcutmanager.cpp
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
#include "vpuzzleshortcutmanager.h"

VPuzzleShortcutManager::VPuzzleShortcutManager(QObject *parent)
  : VAbstractShortcutManager{parent}
{
    QT_WARNING_PUSH
#if !defined(Q_OS_MACOS) && defined(Q_CC_CLANG)
    QT_WARNING_DISABLE_CLANG("-Wenum-enum-conversion")
#endif

    AddShortcut({VShortcutAction::New, KeyBindingsToStringList(QKeySequence::New), {}});
    AddShortcut({VShortcutAction::Open, KeyBindingsToStringList(QKeySequence::Open), {}});
    AddShortcut({VShortcutAction::Save, KeyBindingsToStringList(QKeySequence::Save), {}});
    AddShortcut({VShortcutAction::SaveAs, KeyBindingsToStringList(QKeySequence::SaveAs), {}});
    AddShortcut({VShortcutAction::Undo, KeyBindingsToStringList(QKeySequence::Undo), {}});
    AddShortcut({VShortcutAction::Redo, KeyBindingsToStringList(QKeySequence::Redo), {}});
    AddShortcut({VShortcutAction::ZoomIn, KeyBindingsToStringList(QKeySequence::ZoomIn), {}});
    AddShortcut({VShortcutAction::ZoomOut, KeyBindingsToStringList(QKeySequence::ZoomOut), {}});
    AddShortcut({VShortcutAction::ZoomOriginal,
                 {QKeySequence(Qt::ControlModifier QKEY_SEQUENCE_OP Qt::Key_0).toString(),
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                  QKeySequence(Qt::ControlModifier + Qt::Key_0 + Qt::KeypadModifier).toString()
#endif
                 },
                 {}});
    AddShortcut({VShortcutAction::ZoomFitBest,
                 {QKeySequence(Qt::ControlModifier QKEY_SEQUENCE_OP Qt::Key_Equal).toString()},
                 {}});
    AddShortcut({VShortcutAction::Quit, KeyBindingsToStringList(QKeySequence::Quit), {}});

    QT_WARNING_POP
}
