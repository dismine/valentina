/************************************************************************
 **
 **  @file   vvalentinashortcutmanager.h
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
#ifndef VVALENTINASHORTCUTMANAGER_H
#define VVALENTINASHORTCUTMANAGER_H

#include "../vmisc/vabstractshortcutmanager.h"

#include <QList>

class VValentinaShortcutManager : public VAbstractShortcutManager
{
    Q_OBJECT // NOLINT

public:
    // common.ini [shortcuts] schemeversion. When a built-in binding changes, bump both numbers and put the
    // binding it replaces in previousDefaults. A saved value equal to that old binding is upgraded once.
    // Bump only shortcutScheme for a one-time ini cleanup that does not change bindings. Raising
    // bindingUpgradeScheme again would restore a shortcut the user cleared after the earlier upgrade.
    static constexpr int bindingUpgradeScheme = 1;
    static constexpr int shortcutScheme = 2;

    explicit VValentinaShortcutManager(QObject *parent = nullptr);
    ~VValentinaShortcutManager() override = default;

    void UpdateShortcuts() override;

    static auto DefaultShortcuts() -> QList<VSShortcut>;

private:
    Q_DISABLE_COPY_MOVE(VValentinaShortcutManager) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VValentinaShortcutManager::DefaultShortcuts() -> QList<VSShortcut>
{
    const auto chord = [](Qt::Key key, Qt::KeyboardModifiers modifiers = Qt::NoModifier) -> QStringList
    { return {QKeySequence(modifiers | key).toString()}; };

    // previousDefaults is the binding shipped before bindingUpgradeScheme. An empty list means the action had no binding.
    const auto upgraded = [](VShortcutAction type, const QStringList &defaults,
                              const QStringList &previous) -> VSShortcut { return {type, defaults, {}, previous}; };

    const QStringList unbound{};

    return {
        {VShortcutAction::ZoomIn, KeyBindingsToStringList(QKeySequence::ZoomIn), {}},
        {VShortcutAction::ZoomOut, KeyBindingsToStringList(QKeySequence::ZoomOut), {}},
        {VShortcutAction::ZoomOriginal,
         {QKeySequence(Qt::ControlModifier | Qt::Key_0).toString(),
          QKeySequence(Qt::ControlModifier | Qt::KeypadModifier | Qt::Key_0).toString()},
         {}},
        {VShortcutAction::ZoomFitBest, {QKeySequence(Qt::ControlModifier | Qt::Key_Equal).toString()}, {}},
        {VShortcutAction::ZoomFitBestCurrent, {QKeySequence(Qt::ControlModifier | Qt::Key_M).toString()}, {}},
        {VShortcutAction::IncreaseLabelFont, {QKeySequence(Qt::ShiftModifier | Qt::Key_Plus).toString()}, {}},
        {VShortcutAction::DecreaseLabelFont, {QKeySequence(Qt::ShiftModifier | Qt::Key_Minus).toString()}, {}},
        {VShortcutAction::OriginalLabelFont, {QKeySequence(Qt::ShiftModifier | Qt::Key_0).toString()}, {}},
        {VShortcutAction::HideLabels, {QKeySequence(Qt::AltModifier | Qt::Key_L).toString()}, {}},
        {VShortcutAction::Undo, KeyBindingsToStringList(QKeySequence::Undo), {}},
        {VShortcutAction::Redo, KeyBindingsToStringList(QKeySequence::Redo), {}},
        {VShortcutAction::New, KeyBindingsToStringList(QKeySequence::New), {}},
        {VShortcutAction::Open, KeyBindingsToStringList(QKeySequence::Open), {}},
        {VShortcutAction::Save, KeyBindingsToStringList(QKeySequence::Save), {}},
        {VShortcutAction::SaveAs, KeyBindingsToStringList(QKeySequence::SaveAs), {}},
        // Ctrl+W is Place label now. Draw mode stays one modifier away.
        upgraded(VShortcutAction::DrawMode, chord(Qt::Key_W, Qt::ControlModifier | Qt::ShiftModifier),
                 chord(Qt::Key_W, Qt::ControlModifier)),
        {VShortcutAction::DetailsMode, {QKeySequence(Qt::ControlModifier | Qt::Key_E).toString()}, {}},
        {VShortcutAction::LayoutMode, {QKeySequence(Qt::ControlModifier | Qt::Key_L).toString()}, {}},
        {VShortcutAction::NewPatternPiece,
         {QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_N).toString()},
         {}},
        {VShortcutAction::NextPatternPiece, {QKeySequence(Qt::ControlModifier | Qt::Key_PageDown).toString()}, {}},
        {VShortcutAction::PreviusPatternPiece, {QKeySequence(Qt::ControlModifier | Qt::Key_PageUp).toString()}, {}},
        {VShortcutAction::InteractiveTools, {QKeySequence(Qt::ControlModifier | Qt::Key_D).toString()}, {}},
        {VShortcutAction::TableOfVariables, {QKeySequence(Qt::ControlModifier | Qt::Key_T).toString()}, {}},
        {VShortcutAction::Quit, KeyBindingsToStringList(QKeySequence::Quit), {}},
        {VShortcutAction::CurveDetails, {QKeySequence(Qt::Key_F2).toString()}, {}},
        {VShortcutAction::ShowSegmentLabels, {}, {}},
        // Ctrl+I is Point of intersection curve and axis now.
        upgraded(VShortcutAction::FinalMeasurements, chord(Qt::Key_I, Qt::ControlModifier | Qt::ShiftModifier),
                 chord(Qt::Key_I, Qt::ControlModifier)),
        {VShortcutAction::ReloadPieceLabels, {QKeySequence(Qt::Key_F9).toString()}, {}},
        {VShortcutAction::CaseSensitiveMatch, {QKeySequence(Qt::AltModifier | Qt::Key_C).toString()}, {}},
        {VShortcutAction::WholeWordMatch, {QKeySequence(Qt::AltModifier | Qt::Key_W).toString()}, {}},
        {VShortcutAction::RegexMatch, {QKeySequence(Qt::AltModifier | Qt::Key_X).toString()}, {}},
        {VShortcutAction::SearchHistory, {QKeySequence(Qt::AltModifier | Qt::Key_Down).toString()}, {}},
        {VShortcutAction::RegexMatchUnicodeProperties, {QKeySequence(Qt::AltModifier | Qt::Key_U).toString()}, {}},
        {VShortcutAction::FindNext, {QKeySequence(Qt::Key_F3).toString()}, {}},
        {VShortcutAction::FindPrevious, {QKeySequence(Qt::ShiftModifier | Qt::Key_F3).toString()}, {}},
        // L is Line between points now.
        upgraded(VShortcutAction::LastTool, chord(Qt::Key_L, Qt::ControlModifier | Qt::ShiftModifier),
                 chord(Qt::Key_L)),
        upgraded(VShortcutAction::ToolEndLine, chord(Qt::Key_D), unbound),
        upgraded(VShortcutAction::ToolLine, chord(Qt::Key_L), unbound),
        upgraded(VShortcutAction::ToolAlongLine, chord(Qt::Key_D, Qt::ShiftModifier), unbound),
        {VShortcutAction::ToolShoulderPoint, {}, {}},
        upgraded(VShortcutAction::ToolNormal, chord(Qt::Key_P), unbound),
        upgraded(VShortcutAction::ToolBisector, chord(Qt::Key_B), unbound),
        upgraded(VShortcutAction::ToolLineIntersect, chord(Qt::Key_I), unbound),
        upgraded(VShortcutAction::ToolSpline, chord(Qt::Key_C), unbound),
        {VShortcutAction::ToolCubicBezier, {}, {}},
        upgraded(VShortcutAction::ToolArc, chord(Qt::Key_A), unbound),
        upgraded(VShortcutAction::ToolSplinePath, chord(Qt::Key_C, Qt::ShiftModifier), unbound),
        {VShortcutAction::ToolCubicBezierPath, {}, {}},
        {VShortcutAction::ToolPointOfContact, {}, {}},
        upgraded(VShortcutAction::ToolPiece, chord(Qt::Key_W), unbound),
        upgraded(VShortcutAction::ToolPiecePath, chord(Qt::Key_W, Qt::ShiftModifier), unbound),
        upgraded(VShortcutAction::ToolHeight, chord(Qt::Key_P, Qt::ShiftModifier), unbound),
        {VShortcutAction::ToolTriangle, {}, {}},
        upgraded(VShortcutAction::ToolPointOfIntersection, chord(Qt::Key_X), unbound),
        upgraded(VShortcutAction::ToolCutSpline, chord(Qt::Key_S), unbound),
        upgraded(VShortcutAction::ToolCutSplinePath, chord(Qt::Key_S, Qt::ShiftModifier), unbound),
        {VShortcutAction::ToolUnionDetails, {}, {}},
        {VShortcutAction::ToolDuplicateDetail, {}, {}},
        upgraded(VShortcutAction::ToolCutArc, chord(Qt::Key_S, Qt::AltModifier), unbound),
        upgraded(VShortcutAction::ToolLineIntersectAxis, chord(Qt::Key_I, Qt::ShiftModifier), unbound),
        upgraded(VShortcutAction::ToolCurveIntersectAxis, chord(Qt::Key_I, Qt::ControlModifier), unbound),
        upgraded(VShortcutAction::ToolArcIntersectAxis, chord(Qt::Key_I, Qt::AltModifier), unbound),
        {VShortcutAction::LayoutSettings, {}, {}},
        upgraded(VShortcutAction::ToolPointOfIntersectionArcs, chord(Qt::Key_X, Qt::ShiftModifier), unbound),
        {VShortcutAction::ToolPointOfIntersectionCircles, {}, {}},
        upgraded(VShortcutAction::ToolPointOfIntersectionCurves, chord(Qt::Key_X, Qt::ControlModifier), unbound),
        upgraded(VShortcutAction::ToolPointFromCircleAndTangent, chord(Qt::Key_T, Qt::ShiftModifier), unbound),
        upgraded(VShortcutAction::ToolPointFromArcAndTangent, chord(Qt::Key_T), unbound),
        upgraded(VShortcutAction::ToolArcWithLength, chord(Qt::Key_A, Qt::ShiftModifier), unbound),
        {VShortcutAction::ToolTrueDarts, {}, {}},
        upgraded(VShortcutAction::ToolGroup, chord(Qt::Key_V), unbound),
        upgraded(VShortcutAction::ToolRotation, chord(Qt::Key_R), unbound),
        upgraded(VShortcutAction::ToolFlippingByLine, chord(Qt::Key_F), unbound),
        upgraded(VShortcutAction::ToolFlippingByAxis, chord(Qt::Key_F, Qt::ShiftModifier), unbound),
        upgraded(VShortcutAction::ToolMove, chord(Qt::Key_M), unbound),
        upgraded(VShortcutAction::ToolMidpoint, chord(Qt::Key_D, Qt::ControlModifier | Qt::ShiftModifier), unbound),
        {VShortcutAction::ExportDrawAs, {}, {}},
        {VShortcutAction::ExportLayoutAs, {}, {}},
        {VShortcutAction::ExportDetailsAs, {}, {}},
        {VShortcutAction::ToolEllipticalArc, {}, {}},
        upgraded(VShortcutAction::ToolPin, chord(Qt::Key_P, Qt::ControlModifier), unbound),
        upgraded(VShortcutAction::ToolInsertNode, chord(Qt::Key_N), unbound),
        upgraded(VShortcutAction::ToolPlaceLabel, chord(Qt::Key_W, Qt::ControlModifier), unbound),
        upgraded(VShortcutAction::ToolArcStart, chord(Qt::Key_K, Qt::ShiftModifier), unbound),
        upgraded(VShortcutAction::ToolArcEnd, chord(Qt::Key_K), unbound),
        {VShortcutAction::ToolEllipticalArcWithLength, {}, {}},
        {VShortcutAction::ToolParallelCurve, {}, {}},
        {VShortcutAction::ToolGraduatedCurve, {}, {}},
        {VShortcutAction::AutoArrangeLabels,
         {QKeySequence(Qt::ControlModifier | Qt::ShiftModifier | Qt::Key_A).toString()},
         {}},
    };
}

#endif // VVALENTINASHORTCUTMANAGER_H
