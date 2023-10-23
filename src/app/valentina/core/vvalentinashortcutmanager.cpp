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
#include "qnamespace.h"
#include "vabstractshortcutmanager.h"

#include <QKeySequence>

//---------------------------------------------------------------------------------------------------------------------
VValentinaShortcutManager::VValentinaShortcutManager(QObject *parent)
  : VAbstractShortcutManager{parent}
{
    QT_WARNING_PUSH
#if !defined(Q_OS_MACOS) && defined(Q_CC_CLANG)
    QT_WARNING_DISABLE_CLANG("-Wenum-enum-conversion")
#endif

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
    AddShortcut({VShortcutAction::ZoomFitBestCurrent,
                 {QKeySequence(Qt::ControlModifier QKEY_SEQUENCE_OP Qt::Key_M).toString()},
                 {}});
    AddShortcut({VShortcutAction::IncreaseLabelFont,
                 {QKeySequence(Qt::ShiftModifier QKEY_SEQUENCE_OP Qt::Key_Plus).toString()},
                 {}});
    AddShortcut({VShortcutAction::DecreaseLabelFont,
                 {QKeySequence(Qt::ShiftModifier QKEY_SEQUENCE_OP Qt::Key_Minus).toString()},
                 {}});
    AddShortcut({VShortcutAction::OriginalLabelFont,
                 {QKeySequence(Qt::ShiftModifier QKEY_SEQUENCE_OP Qt::Key_0).toString()},
                 {}});
    AddShortcut(
        {VShortcutAction::HideLabels, {QKeySequence(Qt::AltModifier QKEY_SEQUENCE_OP Qt::Key_L).toString()}, {}});
    AddShortcut({VShortcutAction::Undo, KeyBindingsToStringList(QKeySequence::Undo), {}});
    AddShortcut({VShortcutAction::Redo, KeyBindingsToStringList(QKeySequence::Redo), {}});
    AddShortcut({VShortcutAction::New, KeyBindingsToStringList(QKeySequence::New), {}});
    AddShortcut({VShortcutAction::Open, KeyBindingsToStringList(QKeySequence::Open), {}});
    AddShortcut({VShortcutAction::Save, KeyBindingsToStringList(QKeySequence::Save), {}});
    AddShortcut({VShortcutAction::SaveAs, KeyBindingsToStringList(QKeySequence::SaveAs), {}});
    AddShortcut(
        {VShortcutAction::DrawMode, {QKeySequence(Qt::ControlModifier QKEY_SEQUENCE_OP Qt::Key_W).toString()}, {}});
    AddShortcut(
        {VShortcutAction::DetailsMode, {QKeySequence(Qt::ControlModifier QKEY_SEQUENCE_OP Qt::Key_E).toString()}, {}});
    AddShortcut(
        {VShortcutAction::LayoutMode, {QKeySequence(Qt::ControlModifier QKEY_SEQUENCE_OP Qt::Key_L).toString()}, {}});
    AddShortcut(
        {VShortcutAction::NewPatternPiece,
         {QKeySequence(Qt::ControlModifier QKEY_SEQUENCE_OP Qt::ShiftModifier QKEY_SEQUENCE_OP Qt::Key_N).toString()},
         {}});
    AddShortcut({VShortcutAction::NextPatternPiece,
                 {QKeySequence(Qt::ControlModifier QKEY_SEQUENCE_OP Qt::Key_PageDown).toString()},
                 {}});
    AddShortcut({VShortcutAction::PreviusPatternPiece,
                 {QKeySequence(Qt::ControlModifier QKEY_SEQUENCE_OP Qt::Key_PageUp).toString()},
                 {}});
    AddShortcut({VShortcutAction::InteractiveTools,
                 {QKeySequence(Qt::ControlModifier QKEY_SEQUENCE_OP Qt::Key_D).toString()},
                 {}});
    AddShortcut({VShortcutAction::TableOfVariables,
                 {QKeySequence(Qt::ControlModifier QKEY_SEQUENCE_OP Qt::Key_T).toString()},
                 {}});
    AddShortcut({VShortcutAction::PatternHistory,
                 {QKeySequence(Qt::ControlModifier QKEY_SEQUENCE_OP Qt::Key_H).toString()},
                 {}});
    AddShortcut({VShortcutAction::Quit, KeyBindingsToStringList(QKeySequence::Quit), {}});
    AddShortcut({VShortcutAction::LastTool, {QKeySequence(Qt::Key_L).toString()}, {}});
    AddShortcut({VShortcutAction::CurveDetails, {QKeySequence(Qt::Key_F2).toString()}, {}});
    AddShortcut({VShortcutAction::FinalMeasurements,
                 {QKeySequence(Qt::ControlModifier QKEY_SEQUENCE_OP Qt::Key_I).toString()},
                 {}});

    AddShortcut({VShortcutAction::CaseSensitiveMatch,
                 {QKeySequence(Qt::AltModifier QKEY_SEQUENCE_OP Qt::Key_C).toString()},
                 {}});
    AddShortcut(
        {VShortcutAction::WholeWordMatch, {QKeySequence(Qt::AltModifier QKEY_SEQUENCE_OP Qt::Key_W).toString()}, {}});
    AddShortcut(
        {VShortcutAction::RegexMatch, {QKeySequence(Qt::AltModifier QKEY_SEQUENCE_OP Qt::Key_X).toString()}, {}});
    AddShortcut(
        {VShortcutAction::SearchHistory, {QKeySequence(Qt::AltModifier QKEY_SEQUENCE_OP Qt::Key_Down).toString()}, {}});
    AddShortcut({VShortcutAction::RegexMatchUnicodeProperties,
                 {QKeySequence(Qt::AltModifier QKEY_SEQUENCE_OP Qt::Key_U).toString()},
                 {}});
    AddShortcut({VShortcutAction::FindNext, {QKeySequence(Qt::Key_F3).toString()}, {}});
    AddShortcut(
        {VShortcutAction::FindPrevious, {QKeySequence(Qt::ShiftModifier QKEY_SEQUENCE_OP Qt::Key_F3).toString()}, {}});

    QT_WARNING_POP
}
