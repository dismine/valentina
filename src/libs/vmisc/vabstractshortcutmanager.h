/************************************************************************
 **
 **  @file   vabstractshortcutmanager.h
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
#ifndef VABSTRACTSHORTCUTMANAGER_H
#define VABSTRACTSHORTCUTMANAGER_H

#include <QKeySequence>
#include <QObject>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtCore/QHashFunctions>
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "defglobal.h"
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define QKEY_SEQUENCE_OP |
#else
#define QKEY_SEQUENCE_OP +
#endif

enum class VShortcutAction
{
    ZoomIn,
    ZoomOut,
    ZoomOriginal,
    ZoomFitBest,
    ZoomFitBestCurrent,
    IncreaseLabelFont,
    DecreaseLabelFont,
    OriginalLabelFont,
    HideLabels,
    Undo,
    Redo,
    New,
    Open,
    Save,
    SaveAs,
    DrawMode,
    DetailsMode,
    LayoutMode,
    NewPatternPiece,
    NextPatternPiece,
    PreviusPatternPiece,
    InteractiveTools,
    TableOfVariables,
    PatternHistory,
    Quit,
    LastTool,
    CurveDetails,
    FinalMeasurements,
    CaseSensitiveMatch,
    WholeWordMatch,
    RegexMatch,
    SearchHistory,
    RegexMatchUnicodeProperties,
    FindNext,
    FindPrevious,
    LAST_ONE_DO_NOT_USE
};

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
Q_DECL_CONST_FUNCTION inline auto qHash(VShortcutAction key, uint seed = 0) noexcept -> uint
{
    auto underlyingValue = static_cast<typename std::underlying_type<VShortcutAction>::type>(key);
    return ::qHash(underlyingValue, seed);
}
#endif

class QAction;
class QAbstractButton;

class VAbstractShortcutManager : public QObject
{
    Q_OBJECT // NOLINT

public:
    struct VSShortcut
    {
        VShortcutAction type{};
        QStringList defaultShortcuts{};
        QStringList shortcuts{};
    };

    explicit VAbstractShortcutManager(QObject *parent = nullptr);
    ~VAbstractShortcutManager() override = default;

    void UpdateShortcuts();
    void UpdateActionShortcuts(const QMultiHash<VShortcutAction, QAction *> &actions);
    void UpdateButtonShortcut(const QMultiHash<VShortcutAction, QAbstractButton *> &buttons);

    auto GetShortcutsList() const -> QList<VSShortcut>;

    static auto ShortcutActionToString(VShortcutAction type) -> QString;
    static auto ReadableName(VShortcutAction type) -> QString;
    static auto StringListToReadableString(const QStringList &stringList) -> QString;
    static auto StringListToKeySequenceList(const QStringList &stringList) -> QList<QKeySequence>;

signals:
    void ShortcutsUpdated();

protected:
    void AddShortcut(const VSShortcut &shortcut);

    static auto CustomKeyBindings(QKeySequence::StandardKey sequence) -> QList<QKeySequence>;
    static auto KeyBindingsToStringList(QKeySequence::StandardKey sequence) -> QStringList;

private:
    Q_DISABLE_COPY_MOVE(VAbstractShortcutManager) // NOLINT

    QList<VSShortcut> m_shortcutsList{};
};

#endif // VABSTRACTSHORTCUTMANAGER_H
