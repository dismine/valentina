/************************************************************************
 **
 **  @file   vabstractshortcutmanager.cpp
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
#include "vabstractshortcutmanager.h"

#include <QAbstractButton>
#include <QAction>
#include <QMultiHash>

#include "vabstractapplication.h"
#include "vcommonsettings.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
VAbstractShortcutManager::VAbstractShortcutManager(QObject *parent)
  : QObject{parent}
{
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractShortcutManager::UpdateShortcuts()
{
    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    // Set all shortcuts to the user-set shortcut or the default
    for (auto &shortcut : m_shortcutsList)
    {
        shortcut.shortcuts =
            settings->GetActionShortcuts(ShortcutActionToString(shortcut.type), shortcut.defaultShortcuts);
    }

    emit ShortcutsUpdated();
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractShortcutManager::UpdateActionShortcuts(const QMultiHash<VShortcutAction, QAction *> &actions)
{
    for (const auto &shortcut : m_shortcutsList)
    {
        const auto actionList = actions.values(shortcut.type);

        for (const auto &action : actionList)
        {
            if (action)
            {
                action->setShortcuts(StringListToKeySequenceList(shortcut.shortcuts));
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractShortcutManager::UpdateButtonShortcut(const QMultiHash<VShortcutAction, QAbstractButton *> &buttons)
{
    for (const auto &shortcut : m_shortcutsList)
    {
        const auto buttonList = buttons.values(shortcut.type);

        for (const auto &button : buttonList)
        {
            if (button)
            {
                QList<QKeySequence> const keySequence = StringListToKeySequenceList(shortcut.shortcuts);
                if (!keySequence.isEmpty())
                {
                    button->setShortcut(keySequence.constFirst());
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractShortcutManager::GetShortcutsList() const -> QList<VAbstractShortcutManager::VSShortcut>
{
    return m_shortcutsList;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractShortcutManager::ShortcutActionToString(VShortcutAction type) -> QString
{
    Q_STATIC_ASSERT_X(static_cast<int>(VShortcutAction::LAST_ONE_DO_NOT_USE) == 87, "Convert all actions.");

    switch (type)
    {
        case VShortcutAction::ZoomIn:
            return "zoomin"_L1;
        case VShortcutAction::ZoomOut:
            return "zoomout"_L1;
        case VShortcutAction::ZoomOriginal:
            return "zoomoriginal"_L1;
        case VShortcutAction::ZoomFitBest:
            return "zoomfitbest"_L1;
        case VShortcutAction::ZoomFitBestCurrent:
            return "zoomfitbestcurrent"_L1;
        case VShortcutAction::IncreaseLabelFont:
            return "increaselabelfont"_L1;
        case VShortcutAction::DecreaseLabelFont:
            return "decreaselabelfont"_L1;
        case VShortcutAction::OriginalLabelFont:
            return "originallabelfont"_L1;
        case VShortcutAction::HideLabels:
            return "hidelabels"_L1;
        case VShortcutAction::Undo:
            return "undo"_L1;
        case VShortcutAction::Redo:
            return "redo"_L1;
        case VShortcutAction::New:
            return "new"_L1;
        case VShortcutAction::Open:
            return "open"_L1;
        case VShortcutAction::Save:
            return "save"_L1;
        case VShortcutAction::SaveAs:
            return "saveas"_L1;
        case VShortcutAction::DrawMode:
            return "drawmode"_L1;
        case VShortcutAction::DetailsMode:
            return "detailsmode"_L1;
        case VShortcutAction::LayoutMode:
            return "layoutmode"_L1;
        case VShortcutAction::NewPatternPiece:
            return "newpatternpiece"_L1;
        case VShortcutAction::NextPatternPiece:
            return "nextpatternpiece"_L1;
        case VShortcutAction::PreviusPatternPiece:
            return "previuspatternpiece"_L1;
        case VShortcutAction::InteractiveTools:
            return "interactivetools"_L1;
        case VShortcutAction::TableOfVariables:
            return "tableofvariables"_L1;
        case VShortcutAction::Quit:
            return "quit"_L1;
        case VShortcutAction::LastTool:
            return "lasttool"_L1;
        case VShortcutAction::CurveDetails:
            return "curvedetails"_L1;
        case VShortcutAction::FinalMeasurements:
            return "finalmeasurements"_L1;
        case VShortcutAction::ReloadPieceLabels:
            return "reloadpiecelabels"_L1;
        case VShortcutAction::CaseSensitiveMatch:
            return "casesensitivematch"_L1;
        case VShortcutAction::WholeWordMatch:
            return "wholewordmatch"_L1;
        case VShortcutAction::RegexMatch:
            return "regexmatch"_L1;
        case VShortcutAction::SearchHistory:
            return "searchhistory"_L1;
        case VShortcutAction::RegexMatchUnicodeProperties:
            return "regexmatchunicodeproperties"_L1;
        case VShortcutAction::FindNext:
            return "findnext"_L1;
        case VShortcutAction::FindPrevious:
            return "findprevious"_L1;
        case VShortcutAction::ToolEndLine:
            return "toolendline"_L1;
        case VShortcutAction::ToolLine:
            return "toolline"_L1;
        case VShortcutAction::ToolAlongLine:
            return "toolalongline"_L1;
        case VShortcutAction::ToolShoulderPoint:
            return "toolshoulderpoint"_L1;
        case VShortcutAction::ToolNormal:
            return "toolnormal"_L1;
        case VShortcutAction::ToolBisector:
            return "toolbisector"_L1;
        case VShortcutAction::ToolLineIntersect:
            return "toolLineintersect"_L1;
        case VShortcutAction::ToolSpline:
            return "toolspline"_L1;
        case VShortcutAction::ToolCubicBezier:
            return "toolcubicbezier"_L1;
        case VShortcutAction::ToolArc:
            return "toolarc"_L1;
        case VShortcutAction::ToolSplinePath:
            return "toolsplinepath"_L1;
        case VShortcutAction::ToolCubicBezierPath:
            return "toolcubicbezierpath"_L1;
        case VShortcutAction::ToolPointOfContact:
            return "toolpointofcontact"_L1;
        case VShortcutAction::ToolPiece:
            return "toolpiece"_L1;
        case VShortcutAction::ToolPiecePath:
            return "toolpiecepath"_L1;
        case VShortcutAction::ToolHeight:
            return "toolheight"_L1;
        case VShortcutAction::ToolTriangle:
            return "tooltriangle"_L1;
        case VShortcutAction::ToolPointOfIntersection:
            return "toolpointofintersection"_L1;
        case VShortcutAction::ToolCutSpline:
            return "toolcutspline"_L1;
        case VShortcutAction::ToolCutSplinePath:
            return "toolcutsplinepath"_L1;
        case VShortcutAction::ToolUnionDetails:
            return "tooluniondetails"_L1;
        case VShortcutAction::ToolDuplicateDetail:
            return "toolduplicatedetail"_L1;
        case VShortcutAction::ToolCutArc:
            return "toolcutarc"_L1;
        case VShortcutAction::ToolLineIntersectAxis:
            return "toollineintersectaxis"_L1;
        case VShortcutAction::ToolCurveIntersectAxis:
            return "toolcurveintersectaxis"_L1;
        case VShortcutAction::ToolArcIntersectAxis:
            return "toolarcintersectaxis"_L1;
        case VShortcutAction::LayoutSettings:
            return "layoutsettings"_L1;
        case VShortcutAction::ToolPointOfIntersectionArcs:
            return "toolpointofintersectionarcs"_L1;
        case VShortcutAction::ToolPointOfIntersectionCircles:
            return "toolpointofintersectioncircles"_L1;
        case VShortcutAction::ToolPointOfIntersectionCurves:
            return "toolpointofintersectioncurves"_L1;
        case VShortcutAction::ToolPointFromCircleAndTangent:
            return "toolpointfromcircleandtangent"_L1;
        case VShortcutAction::ToolPointFromArcAndTangent:
            return "toolpointfromarcandtangent"_L1;
        case VShortcutAction::ToolArcWithLength:
            return "toolarcwithlength"_L1;
        case VShortcutAction::ToolTrueDarts:
            return "tooltruedarts"_L1;
        case VShortcutAction::ToolGroup:
            return "toolgroup"_L1;
        case VShortcutAction::ToolRotation:
            return "toolrotation"_L1;
        case VShortcutAction::ToolFlippingByLine:
            return "toolflippingbyline"_L1;
        case VShortcutAction::ToolFlippingByAxis:
            return "toolflippingfyaxis"_L1;
        case VShortcutAction::ToolMove:
            return "toolmove"_L1;
        case VShortcutAction::ToolMidpoint:
            return "toolmidpoint"_L1;
        case VShortcutAction::ExportDrawAs:
            return "exportdrawas"_L1;
        case VShortcutAction::ExportLayoutAs:
            return "exportlayoutas"_L1;
        case VShortcutAction::ExportDetailsAs:
            return "exportdetailsas"_L1;
        case VShortcutAction::ToolEllipticalArc:
            return "toolellipticalarc"_L1;
        case VShortcutAction::ToolPin:
            return "toolpin"_L1;
        case VShortcutAction::ToolInsertNode:
            return "toolinsertnode"_L1;
        case VShortcutAction::ToolPlaceLabel:
            return "toolplacelabel"_L1;
        case VShortcutAction::ToolArcStart:
            return "toolarcstart"_L1;
        case VShortcutAction::ToolArcEnd:
            return "toolarcend"_L1;
        case VShortcutAction::ToolEllipticalArcWithLength:
            return "toolellipticalarcwithlength"_L1;
        case VShortcutAction::ToolParallelCurve:
            return "toolparallelcurve"_L1;
        case VShortcutAction::ToolGraduatedCurve:
            return "toolgraduatedcurve"_L1;
        case VShortcutAction::LAST_ONE_DO_NOT_USE:
            Q_UNREACHABLE();
            break;
        default:
            break;
    };
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractShortcutManager::ReadableName(VShortcutAction type) -> QString
{
    Q_STATIC_ASSERT_X(static_cast<int>(VShortcutAction::LAST_ONE_DO_NOT_USE) == 87, "Translate all actions.");

    switch (type)
    {
        case VShortcutAction::ZoomIn:
            return tr("Zoom in", "shortcut for action");
        case VShortcutAction::ZoomOut:
            return tr("Zoom out", "shortcut for action");
        case VShortcutAction::ZoomOriginal:
            return tr("Zoom original", "shortcut for action");
        case VShortcutAction::ZoomFitBest:
            return tr("Zoom fit best", "shortcut for action");
        case VShortcutAction::ZoomFitBestCurrent:
            return tr("Zoom fit best current", "shortcut for action");
        case VShortcutAction::IncreaseLabelFont:
            return tr("Increase point label font size", "shortcut for action");
        case VShortcutAction::DecreaseLabelFont:
            return tr("Decrease point label font size", "shortcut for action");
        case VShortcutAction::OriginalLabelFont:
            return tr("Original point label font size", "shortcut for action");
        case VShortcutAction::HideLabels:
            return tr("Hide point labels", "shortcut for action");
        case VShortcutAction::Undo:
            return tr("Undo", "shortcut for action");
        case VShortcutAction::Redo:
            return tr("Redo", "shortcut for action");
        case VShortcutAction::New:
            return tr("New", "shortcut for action");
        case VShortcutAction::Open:
            return tr("Open", "shortcut for action");
        case VShortcutAction::Save:
            return tr("Save", "shortcut for action");
        case VShortcutAction::SaveAs:
            return tr("Save as", "shortcut for action");
        case VShortcutAction::DrawMode:
            return tr("Draw mode", "shortcut for action");
        case VShortcutAction::DetailsMode:
            return tr("Details mode", "shortcut for action");
        case VShortcutAction::LayoutMode:
            return tr("Layout mode", "shortcut for action");
        case VShortcutAction::NewPatternPiece:
            return tr("New pattern piece", "shortcut for action");
        case VShortcutAction::NextPatternPiece:
            return tr("Next pattern piece", "shortcut for action");
        case VShortcutAction::PreviusPatternPiece:
            return tr("Previous pattern piece", "shortcut for action");
        case VShortcutAction::InteractiveTools:
            return tr("Interactive tools mode", "shortcut for action");
        case VShortcutAction::TableOfVariables:
            return tr("Open Table of variables dialog", "shortcut for action");
        case VShortcutAction::Quit:
            return tr("Quit app", "shortcut for action");
        case VShortcutAction::LastTool:
            return tr("Activate last used tool", "shortcut for action");
        case VShortcutAction::CurveDetails:
            return tr("Show curve details", "shortcut for action");
        case VShortcutAction::FinalMeasurements:
            return tr("Open final measurements dialog", "shortcut for action");
        case VShortcutAction::ReloadPieceLabels:
            return tr("Reload piece labels", "shortcut for action");
        case VShortcutAction::CaseSensitiveMatch:
            return tr("Case sensitive match", "shortcut for action");
        case VShortcutAction::WholeWordMatch:
            return tr("Whole word match", "shortcut for action");
        case VShortcutAction::RegexMatch:
            return tr("Regex match", "shortcut for action");
        case VShortcutAction::SearchHistory:
            return tr("Search history", "shortcut for action");
        case VShortcutAction::RegexMatchUnicodeProperties:
            return tr("Regex match by unicode properties", "shortcut for action");
        case VShortcutAction::FindNext:
            return tr("Find next match", "shortcut for action");
        case VShortcutAction::FindPrevious:
            return tr("Find previus match", "shortcut for action");
        case VShortcutAction::ToolEndLine:
            return tr("Tool 'Point at distance and angle'", "shortcut for action");
        case VShortcutAction::ToolLine:
            return tr("Tool 'Line between points'", "shortcut for action");
        case VShortcutAction::ToolAlongLine:
            return tr("Tool 'Point at distance along line'", "shortcut for action");
        case VShortcutAction::ToolShoulderPoint:
            return tr("Tool 'Special point on shoulder'", "shortcut for action");
        case VShortcutAction::ToolNormal:
            return tr("Tool 'Point along perpendicular'", "shortcut for action");
        case VShortcutAction::ToolBisector:
            return tr("Tool 'Point along bisector'", "shortcut for action");
        case VShortcutAction::ToolLineIntersect:
            return tr("Tool 'Point at line intersection'", "shortcut for action");
        case VShortcutAction::ToolSpline:
            return tr("Tool 'Simple curve'", "shortcut for action");
        case VShortcutAction::ToolCubicBezier:
            return tr("Tool 'Curve which uses point as control handle'", "shortcut for action");
        case VShortcutAction::ToolArc:
            return tr("Tool 'Arc'", "shortcut for action");
        case VShortcutAction::ToolSplinePath:
            return tr("Tool 'Curved path'", "shortcut for action");
        case VShortcutAction::ToolCubicBezierPath:
            return tr("Tool 'Curved path which uses point as control handle'", "shortcut for action");
        case VShortcutAction::ToolPointOfContact:
            return tr("Tool 'Point of intersection circle and segment'", "shortcut for action");
        case VShortcutAction::ToolPiece:
            return tr("Tool 'Workpiece tool'", "shortcut for action");
        case VShortcutAction::ToolPiecePath:
            return tr("Tool 'Internal path tool'", "shortcut for action");
        case VShortcutAction::ToolHeight:
            return tr("Tool 'Perpendicular point along line'", "shortcut for action");
        case VShortcutAction::ToolTriangle:
            return tr("Tool 'Triangle'", "shortcut for action");
        case VShortcutAction::ToolPointOfIntersection:
            return tr("Tool 'Point from X and Y of two other points'", "shortcut for action");
        case VShortcutAction::ToolCutSpline:
            return tr("Tool 'Segmenting a simple curve'", "shortcut for action");
        case VShortcutAction::ToolCutSplinePath:
            return tr("Tool 'Segment a curved path'", "shortcut for action");
        case VShortcutAction::ToolUnionDetails:
            return tr("Tool 'Piece union'", "shortcut for action");
        case VShortcutAction::ToolDuplicateDetail:
            return tr("Tool 'Duplicate piece'", "shortcut for action");
        case VShortcutAction::ToolCutArc:
            return tr("Tool 'Segment an arc'", "shortcut for action");
        case VShortcutAction::ToolLineIntersectAxis:
            return tr("Tool 'Point of intersection line and axis'", "shortcut for action");
        case VShortcutAction::ToolCurveIntersectAxis:
            return tr("Tool 'Point of intersection curve and axis'", "shortcut for action");
        case VShortcutAction::ToolArcIntersectAxis:
            return tr("Tool 'Point of intersection arc and axis'", "shortcut for action");
        case VShortcutAction::LayoutSettings:
            return tr("Layout settings", "shortcut for action");
        case VShortcutAction::ToolPointOfIntersectionArcs:
            return tr("Tool 'Point of intersection arcs'", "shortcut for action");
        case VShortcutAction::ToolPointOfIntersectionCircles:
            return tr("Tool 'Point of intersection circles'", "shortcut for action");
        case VShortcutAction::ToolPointOfIntersectionCurves:
            return tr("Tool 'Point intersection curves'", "shortcut for action");
        case VShortcutAction::ToolPointFromCircleAndTangent:
            return tr("Tool 'Point from circle and tangent'", "shortcut for action");
        case VShortcutAction::ToolPointFromArcAndTangent:
            return tr("Tool 'Point from arc and tangent'", "shortcut for action");
        case VShortcutAction::ToolArcWithLength:
            return tr("Tool 'Arc with given length'", "shortcut for action");
        case VShortcutAction::ToolTrueDarts:
            return tr("Tool 'True darts'", "shortcut for action");
        case VShortcutAction::ToolGroup:
            return tr("Tool 'Visibility group'", "shortcut for action");
        case VShortcutAction::ToolRotation:
            return tr("Tool 'Rotate objects'", "shortcut for action");
        case VShortcutAction::ToolFlippingByLine:
            return tr("Tool 'Flipping objects by line'", "shortcut for action");
        case VShortcutAction::ToolFlippingByAxis:
            return tr("Tool 'Flipping objects by axis'", "shortcut for action");
        case VShortcutAction::ToolMove:
            return tr("Tool 'Move objects'", "shortcut for action");
        case VShortcutAction::ToolMidpoint:
            return tr("Tool 'Midpoint between two points'", "shortcut for action");
        case VShortcutAction::ExportDrawAs:
            return tr("Export draw as", "shortcut for action");
        case VShortcutAction::ExportLayoutAs:
            return tr("Export layout as", "shortcut for action");
        case VShortcutAction::ExportDetailsAs:
            return tr("Export details as", "shortcut for action");
        case VShortcutAction::ToolEllipticalArc:
            return tr("Tool 'Elliptical Arc'", "shortcut for action");
        case VShortcutAction::ToolPin:
            return tr("Tool 'Pin'", "shortcut for action");
        case VShortcutAction::ToolInsertNode:
            return tr("Tool 'Insert node'", "shortcut for action");
        case VShortcutAction::ToolPlaceLabel:
            return tr("Tool 'Place label'", "shortcut for action");
        case VShortcutAction::ToolArcStart:
            return tr("Tool 'Arc start'", "shortcut for action");
        case VShortcutAction::ToolArcEnd:
            return tr("Tool 'Arc end'", "shortcut for action");
        case VShortcutAction::ToolEllipticalArcWithLength:
            return tr("Tool 'Elliptical arc with given length'", "shortcut for action");
        case VShortcutAction::ToolParallelCurve:
            return tr("Tool 'Parallel curve'", "shortcut for action");
        case VShortcutAction::ToolGraduatedCurve:
            return tr("Tool 'Graduated curve'", "shortcut for action");
        case VShortcutAction::LAST_ONE_DO_NOT_USE:
            Q_UNREACHABLE(); //-V501
            break;
        default:
            break;
    };
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractShortcutManager::StringListToReadableString(const QStringList &stringList) -> QString
{
    QString s = QKeySequence::fromString(stringList.join(", "_L1)).toString(QKeySequence::NativeText);
    return s;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractShortcutManager::AddShortcut(const VSShortcut &shortcut)
{
    m_shortcutsList.append(shortcut);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractShortcutManager::CustomKeyBindings(QKeySequence::StandardKey sequence) -> QList<QKeySequence>
{
    QT_WARNING_PUSH
#if !defined(Q_OS_MACOS) && defined(Q_CC_CLANG)
    QT_WARNING_DISABLE_CLANG("-Wenum-enum-conversion")
#endif

/*
 * Because keypad "-" and "+" not the same keys like in main keypad, shortcut Ctrl+"-" or "+" from keypad will not
 * working with standard shortcut (QKeySequence::ZoomIn or QKeySequence::ZoomOut). For examle "+" is Qt::Key_Plus +
 * Qt::KeypadModifier for keypad. Also for me don't work Qt:CTRL and work Qt::ControlModifier.
 */
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    switch (sequence)
    {
        case QKeySequence::ZoomIn:
            return {QKeySequence(Qt::ControlModifier + Qt::Key_Plus + Qt::KeypadModifier)};
        case QKeySequence::ZoomOut:
            return {QKeySequence(Qt::ControlModifier + Qt::Key_Minus + Qt::KeypadModifier)};
        default:
            break;
    }
#else
    Q_UNUSED(sequence)
#endif

    return {};

    QT_WARNING_POP
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractShortcutManager::KeyBindingsToStringList(QKeySequence::StandardKey sequence) -> QStringList
{
    const auto seqList = QKeySequence::keyBindings(sequence) + CustomKeyBindings(sequence);
    QStringList strings;
    strings.reserve(seqList.size());
    for (const auto &seq : seqList)
    {
        strings << seq.toString();
    }
    return strings;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractShortcutManager::StringListToKeySequenceList(const QStringList &stringList) -> QList<QKeySequence>
{
    QList<QKeySequence> keySequences;
    keySequences.reserve(stringList.size());
    for (const auto &string : stringList)
    {
        keySequences << QKeySequence::fromString(string);
    }
    return keySequences;
}
