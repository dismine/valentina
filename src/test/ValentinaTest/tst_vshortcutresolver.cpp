/************************************************************************
 **
 **  @file   tst_vshortcutresolver.cpp
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

#include "tst_vshortcutresolver.h"

#include "../../app/valentina/core/vvalentinashortcutmanager.h"
#include "../vmisc/vabstractshortcutmanager.h"
#include "../vmisc/vshortcutresolver.h"

#include <QFile>
#include <QHash>
#include <QSettings>
#include <QTemporaryDir>
#include <QTextStream>
#include <QVariant>
#include <QtTest>

namespace
{
using VSShortcut = VAbstractShortcutManager::VSShortcut;

//---------------------------------------------------------------------------------------------------------------------
auto FindShortcut(const QList<VSShortcut> &shortcuts, VShortcutAction type) -> const VSShortcut *
{
    for (const VSShortcut &shortcut : shortcuts)
    {
        if (shortcut.type == type)
        {
            return &shortcut;
        }
    }

    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto PreferencesFromSavedStock() -> QList<VShortcutPreference>
{
    QList<VShortcutPreference> preferences;
    for (const VSShortcut &shortcut : VValentinaShortcutManager::DefaultShortcuts())
    {
        VShortcutPreference preference;
        preference.defaults = shortcut.defaultShortcuts;
        preference.previousDefaults = shortcut.previousDefaults;
        preference.isStored = true;
        // Opening Preferences writes every action, including ones that had no binding (@Invalid()).
        preference.stored = shortcut.previousDefaults.value_or(shortcut.defaultShortcuts);
        preferences.append(preference);
    }
    return preferences;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
TST_VShortcutResolver::TST_VShortcutResolver(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VShortcutResolver::DefaultsDoNotCollide()
{
    QList<QStringList> bindings;
    for (const VSShortcut &shortcut : VValentinaShortcutManager::DefaultShortcuts())
    {
        bindings.append(shortcut.defaultShortcuts);
    }

    const QStringList conflicts = ConflictingShortcutSequences(bindings);
    QVERIFY2(conflicts.isEmpty(), qUtf8Printable(conflicts.join(QStringLiteral(", "))));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VShortcutResolver::ToolDefaultsMatchDraftingSet()
{
    const QHash<VShortcutAction, QString> expected{
        {VShortcutAction::ToolLineIntersect, "I"},
        {VShortcutAction::ToolAlongLine, "Shift+D"},
        {VShortcutAction::ToolArc, "A"},
        {VShortcutAction::ToolArcIntersectAxis, "Alt+I"},
        {VShortcutAction::ToolArcWithLength, "Shift+A"},
        {VShortcutAction::ToolBisector, "B"},
        {VShortcutAction::ToolCurveIntersectAxis, "Ctrl+I"},
        {VShortcutAction::ToolCutArc, "Alt+S"},
        {VShortcutAction::ToolCutSpline, "S"},
        {VShortcutAction::ToolCutSplinePath, "Shift+S"},
        {VShortcutAction::ToolEndLine, "D"},
        {VShortcutAction::ToolFlippingByLine, "F"},
        {VShortcutAction::ToolFlippingByAxis, "Shift+F"},
        {VShortcutAction::ToolGroup, "V"},
        {VShortcutAction::ToolHeight, "Shift+P"},
        {VShortcutAction::ToolInsertNode, "N"},
        {VShortcutAction::ToolLine, "L"},
        {VShortcutAction::ToolLineIntersectAxis, "Shift+I"},
        {VShortcutAction::ToolMidpoint, "Ctrl+Shift+D"},
        {VShortcutAction::ToolMove, "M"},
        {VShortcutAction::ToolNormal, "P"},
        {VShortcutAction::ToolPiece, "W"},
        {VShortcutAction::ToolPiecePath, "Shift+W"},
        {VShortcutAction::ToolPin, "Ctrl+P"},
        {VShortcutAction::ToolPlaceLabel, "Ctrl+W"},
        {VShortcutAction::ToolPointFromArcAndTangent, "T"},
        {VShortcutAction::ToolPointFromCircleAndTangent, "Shift+T"},
        {VShortcutAction::ToolPointOfIntersection, "X"},
        {VShortcutAction::ToolPointOfIntersectionArcs, "Shift+X"},
        {VShortcutAction::ToolPointOfIntersectionCurves, "Ctrl+X"},
        {VShortcutAction::ToolRotation, "R"},
        {VShortcutAction::ToolSpline, "C"},
        {VShortcutAction::ToolSplinePath, "Shift+C"},
        {VShortcutAction::ToolArcEnd, "K"},
        {VShortcutAction::ToolArcStart, "Shift+K"},
    };

    const QList<VSShortcut> defaults = VValentinaShortcutManager::DefaultShortcuts();
    for (auto it = expected.cbegin(); it != expected.cend(); ++it)
    {
        const VSShortcut *shortcut = FindShortcut(defaults, it.key());
        QVERIFY2(shortcut != nullptr, qUtf8Printable(VAbstractShortcutManager::ShortcutActionToString(it.key())));
        QCOMPARE(shortcut->defaultShortcuts, QStringList{it.value()});
        QVERIFY2(shortcut->previousDefaults.has_value(),
                 "A new tool binding has to remember that it used to be unset.");
        QVERIFY(shortcut->previousDefaults->isEmpty());
    }

    const QList<VShortcutAction> stillUnbound{
        VShortcutAction::ToolCubicBezier,
        VShortcutAction::ToolCubicBezierPath,
        VShortcutAction::ToolDuplicateDetail,
        VShortcutAction::ToolEllipticalArc,
        VShortcutAction::ToolPointOfContact,
        VShortcutAction::ToolPointOfIntersectionCircles,
        VShortcutAction::ToolShoulderPoint,
        VShortcutAction::ToolTriangle,
        VShortcutAction::ToolTrueDarts,
        VShortcutAction::ToolUnionDetails,
        VShortcutAction::ToolEllipticalArcWithLength,
        VShortcutAction::ToolParallelCurve,
        VShortcutAction::ToolGraduatedCurve,
    };

    for (const VShortcutAction type : stillUnbound)
    {
        const VSShortcut *shortcut = FindShortcut(defaults, type);
        QVERIFY2(shortcut != nullptr, qUtf8Printable(VAbstractShortcutManager::ShortcutActionToString(type)));
        QVERIFY2(shortcut->defaultShortcuts.isEmpty(),
                 qUtf8Printable(VAbstractShortcutManager::ShortcutActionToString(type)));
        QVERIFY2(!shortcut->previousDefaults.has_value(), "An unchanged empty binding must not be upgraded later.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VShortcutResolver::DisplacedCommandsKeepABinding()
{
    const QList<VSShortcut> defaults = VValentinaShortcutManager::DefaultShortcuts();

    const VSShortcut *lastTool = FindShortcut(defaults, VShortcutAction::LastTool);
    QVERIFY(lastTool != nullptr);
    QCOMPARE(lastTool->defaultShortcuts, QStringList{QStringLiteral("Ctrl+Shift+L")});
    QCOMPARE(lastTool->previousDefaults, std::optional<QStringList>{QStringList{QStringLiteral("L")}});

    const VSShortcut *drawMode = FindShortcut(defaults, VShortcutAction::DrawMode);
    QVERIFY(drawMode != nullptr);
    QCOMPARE(drawMode->defaultShortcuts, QStringList{QStringLiteral("Ctrl+Shift+W")});
    QCOMPARE(drawMode->previousDefaults, std::optional<QStringList>{QStringList{QStringLiteral("Ctrl+W")}});

    const VSShortcut *measurements = FindShortcut(defaults, VShortcutAction::FinalMeasurements);
    QVERIFY(measurements != nullptr);
    QCOMPARE(measurements->defaultShortcuts, QStringList{QStringLiteral("Ctrl+Shift+I")});
    QCOMPARE(measurements->previousDefaults, std::optional<QStringList>{QStringList{QStringLiteral("Ctrl+I")}});
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VShortcutResolver::UnchangedActionsAreNotMigrated()
{
    const QList<VSShortcut> defaults = VValentinaShortcutManager::DefaultShortcuts();
    for (const VShortcutAction type :
         {VShortcutAction::ZoomIn, VShortcutAction::DetailsMode, VShortcutAction::LayoutMode, VShortcutAction::Save,
          VShortcutAction::CaseSensitiveMatch, VShortcutAction::AutoArrangeLabels})
    {
        const VSShortcut *shortcut = FindShortcut(defaults, type);
        QVERIFY(shortcut != nullptr);
        QVERIFY2(!shortcut->previousDefaults.has_value(),
                 qUtf8Printable(VAbstractShortcutManager::ShortcutActionToString(type)));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VShortcutResolver::MissingKeysUseDefaults()
{
    const QList<VSShortcut> defaults = VValentinaShortcutManager::DefaultShortcuts();
    QList<VShortcutPreference> preferences;
    for (const VSShortcut &shortcut : defaults)
    {
        VShortcutPreference preference;
        preference.defaults = shortcut.defaultShortcuts;
        preference.previousDefaults = shortcut.previousDefaults;
        preference.isStored = false;
        preferences.append(preference);
    }

    const QList<VShortcutResolution> resolved = ResolveShortcutPreferences(preferences);
    QCOMPARE(resolved.size(), defaults.size());
    for (int i = 0; i < resolved.size(); ++i)
    {
        QCOMPARE(resolved.at(i).shortcuts, defaults.at(i).defaultShortcuts);
        QVERIFY(!resolved.at(i).persist);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VShortcutResolver::SavedOldDefaultIsUpgraded()
{
    const QList<VSShortcut> defaults = VValentinaShortcutManager::DefaultShortcuts();
    const QList<VShortcutResolution> resolved = ResolveShortcutPreferences(PreferencesFromSavedStock());
    QCOMPARE(resolved.size(), defaults.size());

    QList<QStringList> bindings;
    for (int i = 0; i < resolved.size(); ++i)
    {
        QCOMPARE(resolved.at(i).shortcuts, defaults.at(i).defaultShortcuts);
        const bool changed = defaults.at(i).previousDefaults.has_value()
                             && NormalizeShortcutSequences(*defaults.at(i).previousDefaults)
                                    != NormalizeShortcutSequences(defaults.at(i).defaultShortcuts);
        QCOMPARE(resolved.at(i).persist, changed);
        bindings.append(resolved.at(i).shortcuts);
    }

    QVERIFY(ConflictingShortcutSequences(bindings).isEmpty());
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VShortcutResolver::SavedCustomShortcutIsKept()
{
    VShortcutPreference endLine;
    endLine.defaults = QStringList{QStringLiteral("D")};
    endLine.previousDefaults = QStringList{};
    endLine.isStored = true;
    endLine.stored = QStringList{QStringLiteral("Ctrl+1")};

    VShortcutPreference line;
    line.defaults = QStringList{QStringLiteral("L")};
    line.previousDefaults = QStringList{};
    line.isStored = true;
    line.stored = QStringList{QStringLiteral("L")};

    const QList<VShortcutResolution> resolved = ResolveShortcutPreferences({endLine, line});
    QCOMPARE(resolved.at(0).shortcuts, QStringList{QStringLiteral("Ctrl+1")});
    QVERIFY(!resolved.at(0).persist);
    QCOMPARE(resolved.at(1).shortcuts, QStringList{QStringLiteral("L")});
    QVERIFY(!resolved.at(1).persist);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VShortcutResolver::CustomKeyBlocksTheNewDefault()
{
    // Move was explicitly bound to L (any capitalization). Line's new default is L, so Line stays unset.
    VShortcutPreference moveAction;
    moveAction.defaults = QStringList{QStringLiteral("M")};
    moveAction.previousDefaults = QStringList{};
    moveAction.isStored = true;
    moveAction.stored = QStringList{QStringLiteral("l")};

    VShortcutPreference line;
    line.defaults = QStringList{QStringLiteral("L")};
    line.previousDefaults = QStringList{};
    line.isStored = true;
    line.stored = QStringList{};

    // Group was explicitly bound to the chord Last Tool would otherwise move to. Last Tool's old key is L,
    // which Move already owns, so Last Tool is left unset and that saved L is overwritten.
    VShortcutPreference group;
    group.defaults = QStringList{QStringLiteral("V")};
    group.previousDefaults = QStringList{};
    group.isStored = true;
    group.stored = QStringList{QStringLiteral("Ctrl+Shift+L")};

    VShortcutPreference lastTool;
    lastTool.defaults = QStringList{QStringLiteral("Ctrl+Shift+L")};
    lastTool.previousDefaults = QStringList{QStringLiteral("L")};
    lastTool.isStored = true;
    lastTool.stored = QStringList{QStringLiteral("L")};

    const QList<VShortcutResolution> resolved = ResolveShortcutPreferences({moveAction, line, group, lastTool});
    QCOMPARE(resolved.at(0).shortcuts, QStringList{QStringLiteral("l")});
    QVERIFY(!resolved.at(0).persist);

    QVERIFY(resolved.at(1).shortcuts.isEmpty());
    QVERIFY(!resolved.at(1).persist);

    QCOMPARE(resolved.at(2).shortcuts, QStringList{QStringLiteral("Ctrl+Shift+L")});
    QVERIFY(!resolved.at(2).persist);

    QVERIFY(resolved.at(3).shortcuts.isEmpty());
    QVERIFY2(resolved.at(3).persist,
             "The saved L has to be cleared, or the next launch binds Last Tool and Move both to L.");
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VShortcutResolver::ClearedShortcutStaysClearedAfterMigration()
{
    // schemeversion is already written, so previousDefaults is not supplied. Empty means the user cleared it.
    VShortcutPreference endLine;
    endLine.defaults = QStringList{QStringLiteral("D")};
    endLine.isStored = true;
    endLine.stored = QStringList{};

    const QList<VShortcutResolution> resolved = ResolveShortcutPreferences({endLine});
    QVERIFY(resolved.constFirst().shortcuts.isEmpty());
    QVERIFY(!resolved.constFirst().persist);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VShortcutResolver::InvalidIniValueIsAnEmptyShortcut()
{
    QTemporaryDir dir;
    QVERIFY2(dir.isValid(), qUtf8Printable(dir.errorString()));

    const QString path = dir.filePath(QStringLiteral("common.ini"));
    {
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
        QTextStream stream(&file);
        stream << "[shortcuts]\n";
        stream << "toolendline=@Invalid()\n";
        stream << "lasttool=L\n";
        stream << "drawmode=Ctrl+W\n";
        stream << "toolline=P\n";
    }

    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("shortcuts"));
    QVERIFY(settings.contains(QStringLiteral("toolendline")));
    QVERIFY(settings.value(QStringLiteral("toolendline")).toStringList().isEmpty());

    VShortcutPreference endLine;
    endLine.defaults = QStringList{QStringLiteral("D")};
    endLine.previousDefaults = QStringList{};
    endLine.isStored = true;
    endLine.stored = settings.value(QStringLiteral("toolendline")).toStringList();

    VShortcutPreference lastTool;
    lastTool.defaults = QStringList{QStringLiteral("Ctrl+Shift+L")};
    lastTool.previousDefaults = QStringList{QStringLiteral("L")};
    lastTool.isStored = true;
    lastTool.stored = settings.value(QStringLiteral("lasttool")).toStringList();

    VShortcutPreference drawMode;
    drawMode.defaults = QStringList{QStringLiteral("Ctrl+Shift+W")};
    drawMode.previousDefaults = QStringList{QStringLiteral("Ctrl+W")};
    drawMode.isStored = true;
    drawMode.stored = settings.value(QStringLiteral("drawmode")).toStringList();

    VShortcutPreference line;
    line.defaults = QStringList{QStringLiteral("L")};
    line.previousDefaults = QStringList{};
    line.isStored = true;
    line.stored = settings.value(QStringLiteral("toolline")).toStringList();

    const QList<VShortcutResolution> resolved = ResolveShortcutPreferences({endLine, lastTool, drawMode, line});
    QCOMPARE(resolved.at(0).shortcuts, QStringList{QStringLiteral("D")});
    QVERIFY(resolved.at(0).persist);
    QCOMPARE(resolved.at(1).shortcuts, QStringList{QStringLiteral("Ctrl+Shift+L")});
    QVERIFY(resolved.at(1).persist);
    QCOMPARE(resolved.at(2).shortcuts, QStringList{QStringLiteral("Ctrl+Shift+W")});
    QVERIFY(resolved.at(2).persist);
    QCOMPARE(resolved.at(3).shortcuts, QStringList{QStringLiteral("P")});
    QVERIFY(!resolved.at(3).persist);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VShortcutResolver::BlankIniValueIsNotAShortcut()
{
    // `key=` reads back as one empty string. That is not a binding, and it must not restore a cleared shortcut.
    VShortcutPreference clearedSave;
    clearedSave.defaults = QStringList{QStringLiteral("Ctrl+S")};
    clearedSave.isStored = true;
    clearedSave.stored = QStringList{QString()};

    const QList<VShortcutResolution> kept = ResolveShortcutPreferences({clearedSave});
    QVERIFY(NormalizeShortcutSequences(kept.constFirst().shortcuts).isEmpty());
    QVERIFY(!kept.constFirst().persist);

    // The same blank value on a tool whose old default was "unset" is the shipped default, so it upgrades.
    VShortcutPreference unsetTool;
    unsetTool.defaults = QStringList{QStringLiteral("D")};
    unsetTool.previousDefaults = QStringList{};
    unsetTool.isStored = true;
    unsetTool.stored = QStringList{QString()};

    const QList<VShortcutResolution> upgraded = ResolveShortcutPreferences({unsetTool});
    QCOMPARE(upgraded.constFirst().shortcuts, QStringList{QStringLiteral("D")});
    QVERIFY(upgraded.constFirst().persist);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VShortcutResolver::ExplicitEmptyListIsTheInvalidToken()
{
    QTemporaryDir dir;
    QVERIFY2(dir.isValid(), qUtf8Printable(dir.errorString()));

    const QString path = dir.filePath(QStringLiteral("common.ini"));
    {
        QSettings settings(path, QSettings::IniFormat);
        settings.beginGroup(QStringLiteral("shortcuts"));
        settings.setValue(QStringLiteral("save"), QStringList{});
        settings.setValue(QStringLiteral("blank"), QString());
        settings.setValue(QStringLiteral("quoted"), QStringLiteral(""));
        settings.setValue(QStringLiteral("schemeversion"), 2);
        settings.sync();
    }

    QFile file(path);
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString text = QString::fromUtf8(file.readAll());
    QVERIFY2(text.contains(QStringLiteral("save=@Invalid()")), qUtf8Printable(text));

    QSettings settings(path, QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("shortcuts"));

    const QVariant savedEmpty = settings.value(QStringLiteral("save"));
    QVERIFY(savedEmpty.isValid());
    QVERIFY(savedEmpty.toStringList().isEmpty());
    QVERIFY(IsEmptyShortcutPlaceholder(savedEmpty));

    const QVariant blank = settings.value(QStringLiteral("blank"));
    QCOMPARE(blank.toStringList(), QStringList{QString()});
    QVERIFY(IsEmptyShortcutPlaceholder(blank));
    QVERIFY(IsEmptyShortcutPlaceholder(settings.value(QStringLiteral("quoted"))));
    QVERIFY(!IsEmptyShortcutPlaceholder(settings.value(QStringLiteral("schemeversion"))));
    QVERIFY(!IsEmptyShortcutPlaceholder(QVariant(QStringLiteral("Ctrl+S"))));

    QCOMPARE(ShortcutSaveAction({}, {}), VShortcutSave::RemoveKey);
    QCOMPARE(ShortcutSaveAction(QStringList{QString()}, {}), VShortcutSave::RemoveKey);
    QCOMPARE(ShortcutSaveAction({}, QStringList{QStringLiteral("Ctrl+S")}), VShortcutSave::WriteExplicitEmpty);
    QCOMPARE(ShortcutSaveAction(QStringList{QStringLiteral("D")}, QStringList{QStringLiteral("D")}),
             VShortcutSave::WriteSequences);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VShortcutResolver::EmptyPlaceholderKeysSkipKnownActionsAndScheme()
{
    QHash<QString, QVariant> values;
    values.insert(QStringLiteral("schemeversion"), 1);
    values.insert(QStringLiteral("toolcubicbezier"), QVariant());
    values.insert(QStringLiteral("save"), QStringList{});
    values.insert(QStringLiteral("mystery"), QVariant());
    values.insert(QStringLiteral("kept"), QStringLiteral("Ctrl+1"));

    const QStringList keys{QStringLiteral("schemeversion"), QStringLiteral("toolcubicbezier"), QStringLiteral("save"),
                           QStringLiteral("mystery"), QStringLiteral("kept")};
    const QStringList known{QStringLiteral("toolcubicbezier"), QStringLiteral("save")};
    const QStringList drop = EmptyShortcutPlaceholderKeys(keys, values, known);
    QCOMPARE(drop, QStringList{QStringLiteral("mystery")});
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VShortcutResolver::SchemeKeepsBindingUpgradeSeparateFromCleanup()
{
    QCOMPARE(VValentinaShortcutManager::bindingUpgradeScheme, 1);
    QCOMPARE(VValentinaShortcutManager::shortcutScheme, 2);
    QVERIFY(VValentinaShortcutManager::bindingUpgradeScheme < VValentinaShortcutManager::shortcutScheme);
}
