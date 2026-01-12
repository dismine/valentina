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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include "../vmisc/compatibility.h"
#endif

//---------------------------------------------------------------------------------------------------------------------
VValentinaShortcutManager::VValentinaShortcutManager(QObject *parent)
  : VAbstractShortcutManager{parent}
{
    Q_STATIC_ASSERT_X(static_cast<int>(VShortcutAction::LAST_ONE_DO_NOT_USE) == 87, "Check if all actions were used.");

    QT_WARNING_PUSH
#if !defined(Q_OS_MACOS) && defined(Q_CC_CLANG)
    QT_WARNING_DISABLE_CLANG("-Wenum-enum-conversion")
#endif

    AddShortcut({VShortcutAction::ZoomIn, KeyBindingsToStringList(QKeySequence::ZoomIn), {}});
    AddShortcut({VShortcutAction::ZoomOut, KeyBindingsToStringList(QKeySequence::ZoomOut), {}});
    AddShortcut({VShortcutAction::ZoomOriginal,
                 {QKeySequence(Qt::ControlModifier QKEY_SEQUENCE_OP Qt::Key_0).toString(),
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                  QKeySequence(Qt::ControlModifier | Qt::KeypadModifier | Qt::Key_0).toString()
#else
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
    AddShortcut({VShortcutAction::Quit, KeyBindingsToStringList(QKeySequence::Quit), {}});
    AddShortcut({VShortcutAction::CurveDetails, {QKeySequence(Qt::Key_F2).toString()}, {}});
    AddShortcut({VShortcutAction::FinalMeasurements,
                 {QKeySequence(Qt::ControlModifier QKEY_SEQUENCE_OP Qt::Key_I).toString()},
                 {}});
    AddShortcut({VShortcutAction::ReloadPieceLabels, {QKeySequence(Qt::Key_F9).toString()}, {}});

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
    AddShortcut({VShortcutAction::LastTool, {QKeySequence(Qt::Key_L).toString()}, {}});
    AddShortcut({VShortcutAction::ToolEndLine, {}, {}});
    AddShortcut({VShortcutAction::ToolLine, {}, {}});
    AddShortcut({VShortcutAction::ToolAlongLine, {}, {}});
    AddShortcut({VShortcutAction::ToolShoulderPoint, {}, {}});
    AddShortcut({VShortcutAction::ToolNormal, {}, {}});
    AddShortcut({VShortcutAction::ToolBisector, {}, {}});
    AddShortcut({VShortcutAction::ToolLineIntersect, {}, {}});
    AddShortcut({VShortcutAction::ToolSpline, {}, {}});
    AddShortcut({VShortcutAction::ToolCubicBezier, {}, {}});
    AddShortcut({VShortcutAction::ToolArc, {}, {}});
    AddShortcut({VShortcutAction::ToolSplinePath, {}, {}});
    AddShortcut({VShortcutAction::ToolCubicBezierPath, {}, {}});
    AddShortcut({VShortcutAction::ToolPointOfContact, {}, {}});
    AddShortcut({VShortcutAction::ToolPiece, {}, {}});
    AddShortcut({VShortcutAction::ToolPiecePath, {}, {}});
    AddShortcut({VShortcutAction::ToolHeight, {}, {}});
    AddShortcut({VShortcutAction::ToolTriangle, {}, {}});
    AddShortcut({VShortcutAction::ToolPointOfIntersection, {}, {}});
    AddShortcut({VShortcutAction::ToolCutSpline, {}, {}});
    AddShortcut({VShortcutAction::ToolCutSplinePath, {}, {}});
    AddShortcut({VShortcutAction::ToolUnionDetails, {}, {}});
    AddShortcut({VShortcutAction::ToolDuplicateDetail, {}, {}});
    AddShortcut({VShortcutAction::ToolCutArc, {}, {}});
    AddShortcut({VShortcutAction::ToolLineIntersectAxis, {}, {}});
    AddShortcut({VShortcutAction::ToolCurveIntersectAxis, {}, {}});
    AddShortcut({VShortcutAction::ToolArcIntersectAxis, {}, {}});
    AddShortcut({VShortcutAction::LayoutSettings, {}, {}});
    AddShortcut({VShortcutAction::ToolPointOfIntersectionArcs, {}, {}});
    AddShortcut({VShortcutAction::ToolPointOfIntersectionCircles, {}, {}});
    AddShortcut({VShortcutAction::ToolPointOfIntersectionCurves, {}, {}});
    AddShortcut({VShortcutAction::ToolPointFromCircleAndTangent, {}, {}});
    AddShortcut({VShortcutAction::ToolPointFromArcAndTangent, {}, {}});
    AddShortcut({VShortcutAction::ToolArcWithLength, {}, {}});
    AddShortcut({VShortcutAction::ToolTrueDarts, {}, {}});
    AddShortcut({VShortcutAction::ToolGroup, {}, {}});
    AddShortcut({VShortcutAction::ToolRotation, {}, {}});
    AddShortcut({VShortcutAction::ToolFlippingByLine, {}, {}});
    AddShortcut({VShortcutAction::ToolFlippingByAxis, {}, {}});
    AddShortcut({VShortcutAction::ToolMove, {}, {}});
    AddShortcut({VShortcutAction::ToolMidpoint, {}, {}});
    AddShortcut({VShortcutAction::ExportDrawAs, {}, {}});
    AddShortcut({VShortcutAction::ExportLayoutAs, {}, {}});
    AddShortcut({VShortcutAction::ExportDetailsAs, {}, {}});
    AddShortcut({VShortcutAction::ToolEllipticalArc, {}, {}});
    AddShortcut({VShortcutAction::ToolPin, {}, {}});
    AddShortcut({VShortcutAction::ToolInsertNode, {}, {}});
    AddShortcut({VShortcutAction::ToolPlaceLabel, {}, {}});
    AddShortcut({VShortcutAction::ToolArcStart, {}, {}});
    AddShortcut({VShortcutAction::ToolArcEnd, {}, {}});
    AddShortcut({VShortcutAction::ToolEllipticalArcWithLength, {}, {}});
    AddShortcut({VShortcutAction::ToolParallelCurve, {}, {}});
    AddShortcut({VShortcutAction::ToolGraduatedCurve, {}, {}});

    QT_WARNING_POP
}
