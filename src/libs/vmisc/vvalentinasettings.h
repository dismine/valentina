/************************************************************************
 **
 **  @file   vvalentinasettings.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 11, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#ifndef VVALENTINASETTINGS_H
#define VVALENTINASETTINGS_H

#include <QMetaObject>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QtGlobal>

#include "vcommonsettings.h"

enum class VToolPointerMode
{
    ToolIcon = 0,
    Arrow = 1
};

class VValentinaSettings : public VCommonSettings
{
    Q_OBJECT // NOLINT

public:
    VValentinaSettings(Format format, Scope scope, const QString &organization, const QString &application = QString(),
                       QObject *parent = nullptr);
    VValentinaSettings(const QString &fileName, Format format, QObject *parent = nullptr);

    auto GetLabelLanguage() const -> QString;
    void SetLabelLanguage(const QString &value);

    auto GetPathLayout() const -> QString;
    void SetPathLayout(const QString &value);

    auto GetPathLabelTemplate() const -> QString;
    void SetPathLabelTemplate(const QString &value);

    // Layout settings
    auto GetLayoutPaperHeight() const -> qreal;
    void SetLayoutPaperHeight(qreal value);

    auto GetLayoutPaperWidth() const -> qreal;
    void SetLayoutPaperWidth(qreal value);

    auto GetLayoutWidth() const -> qreal;
    static auto GetDefLayoutWidth() -> qreal;
    void SetLayoutWidth(qreal value);

    auto GetNestingTime() const -> int;
    static auto GetDefNestingTime() -> int { return 1; }
    void SetNestingTime(int value);

    auto GetEfficiencyCoefficient() const -> qreal;
    static auto GetDefEfficiencyCoefficient() -> qreal { return 0.0; }
    void SetEfficiencyCoefficient(qreal value);

    auto GetFields(const QMarginsF &def = QMarginsF()) const -> QMarginsF;
    void SetFields(const QMarginsF &value);

    auto GetLayoutGroup() const -> Cases;
    static auto GetDefLayoutGroup() -> Cases;
    void SetLayoutGroup(const Cases &value);

    auto GetLayoutFollowGrainline() const -> bool;
    static auto GetDefLayoutFollowGrainline() -> bool;
    void SetLayoutFollowGrainline(bool value);

    auto GetLayoutManualPriority() const -> bool;
    static auto GetDefLayoutManualPriority() -> bool;
    void SetLayoutManualPriority(bool value);

    auto GetLayoutNestQuantity() const -> bool;
    static auto GetDefLayoutNestQuantity() -> bool;
    void SetLayoutNestQuantity(bool value);

    auto GetLayoutAutoCropLength() const -> bool;
    static auto GetDefLayoutAutoCropLength() -> bool;
    void SetLayoutAutoCropLength(bool value);

    auto GetLayoutAutoCropWidth() const -> bool;
    static auto GetDefLayoutAutoCropWidth() -> bool;
    void SetLayoutAutoCropWidth(bool value);

    auto GetLayoutSaveLength() const -> bool;
    static auto GetDefLayoutSaveLength() -> bool;
    void SetLayoutSaveLength(bool value);

    auto GetLayoutPreferOneSheetSolution() const -> bool;
    static auto GetDefLayoutPreferOneSheetSolution() -> bool;
    void SetLayoutPreferOneSheetSolution(bool value);

    auto GetLayoutUnitePages() const -> bool;
    static auto GetDefLayoutUnitePages() -> bool;
    void SetLayoutUnitePages(bool value);

    auto GetIgnoreAllFields() const -> bool;
    static auto GetDefIgnoreAllFields() -> bool;
    void SetIgnoreAllFields(bool value);

    auto GetStripOptimization() const -> bool;
    static auto GetDefStripOptimization() -> bool;
    void SetStripOptimization(bool value);

    auto GetMultiplier() const -> quint8;
    static auto GetDefMultiplier() -> quint8;
    void SetMultiplier(quint8 value);

    auto GetTextAsPaths() const -> bool;
    static auto GetDefTextAsPaths() -> bool;
    void SetTextAsPaths(bool value);

    auto GetKnownMaterials() const -> QStringList;
    void SetKnownMaterials(const QStringList &list);

    auto IsRememberPatternMaterials() const -> bool;
    void SetRememberPatternMaterials(bool value);

    // settings for the tiled PDFs
    auto GetTiledPDFPaperHeight(const Unit &unit) const -> qreal;
    void SetTiledPDFPaperHeight(qreal value, const Unit &unit);

    auto GetTiledPDFPaperWidth(const Unit &unit) const -> qreal;
    void SetTiledPDFPaperWidth(qreal value, const Unit &unit);

    auto IsDockWidgetGroupsActive() const -> bool;
    static auto GetDefDockWidgetGroupsActive() -> bool;
    void SetDockWidgetGroupsActive(bool value);

    auto IsDockWidgetToolOptionsActive() const -> bool;
    static auto GetDefDockWidgetToolOptionsActive() -> bool;
    void SetDockWidgetToolOptionsActive(bool value);

    auto IsDockWidgetPatternMessagesActive() const -> bool;
    static auto GetDefDockWidgetPatternMessagesActive() -> bool;
    void SetDockWidgetPatternMessagesActive(bool value);

    auto IsDockWidgetBackgroundImagesActive() const -> bool;
    static auto GetDefDockWidgetBackgroundImagesActive() -> bool;
    void SetDockWidgetBackgroundImagesActive(bool value);

    auto GetPatternMessageFontSize(int fontSizeDef) const -> int;
    static auto GetDefMinPatternMessageFontSize() -> int;
    static auto GetDefMaxPatternMessageFontSize() -> int;
    void SetPatternMessageFontSize(int size);

    auto GetAutoRefreshPatternMessage() const -> bool;
    void SetAutoRefreshPatternMessage(bool value);

    auto GetLayoutExportFormat() const -> qint8;
    void SetLayoutExportFormat(qint8 format);

    auto GetDetailExportFormat() const -> qint8;
    void SetDetailExportFormat(qint8 format);

    auto GetHistorySearchHistory() const -> QStringList;
    void SetHistorySearchHistory(const QStringList &history);

    auto GetIncrementsSearchHistory() const -> QStringList;
    void SetIncrementsSearchHistory(const QStringList &history);

    auto GetPreviewCalculationsSearchHistory() const -> QStringList;
    void SetPreviewCalculationsSearchHistory(const QStringList &history);

    auto GetFinalMeasurementsSearchHistory() const -> QStringList;
    void SetFinalMeasurementsSearchHistory(const QStringList &history);

    auto GetHistorySearchOptionUseUnicodeProperties() const -> bool;
    void SetHistorySearchOptionUseUnicodeProperties(bool value);

    auto GetHistorySearchOptionWholeWord() const -> bool;
    void SetHistorySearchOptionWholeWord(bool value);

    auto GetHistorySearchOptionRegexp() const -> bool;
    void SetHistorySearchOptionRegexp(bool value);

    auto GetHistorySearchOptionMatchCase() const -> bool;
    void SetHistorySearchOptionMatchCase(bool value);

    auto GetIncrementsSearchOptionUseUnicodeProperties() const -> bool;
    void SetIncrementsSearchOptionUseUnicodeProperties(bool value);

    auto GetIncrementsSearchOptionWholeWord() const -> bool;
    void SetIncrementsSearchOptionWholeWord(bool value);

    auto GetIncrementsSearchOptionRegexp() const -> bool;
    void SetIncrementsSearchOptionRegexp(bool value);

    auto GetIncrementsSearchOptionMatchCase() const -> bool;
    void SetIncrementsSearchOptionMatchCase(bool value);

    auto GetPreviewCalculationsSearchOptionUseUnicodeProperties() const -> bool;
    void SetPreviewCalculationsSearchOptionUseUnicodeProperties(bool value);

    auto GetPreviewCalculationsSearchOptionWholeWord() const -> bool;
    void SetPreviewCalculationsSearchOptionWholeWord(bool value);

    auto GetPreviewCalculationsSearchOptionRegexp() const -> bool;
    void SetPreviewCalculationsSearchOptionRegexp(bool value);

    auto GetPreviewCalculationsSearchOptionMatchCase() const -> bool;
    void SetPreviewCalculationsSearchOptionMatchCase(bool value);

    auto GetFinalMeasurementsSearchOptionUseUnicodeProperties() const -> bool;
    void SetFinalMeasurementsSearchOptionUseUnicodeProperties(bool value);

    auto GetFinalMeasurementsSearchOptionWholeWord() const -> bool;
    void SetFinalMeasurementsSearchOptionWholeWord(bool value);

    auto GetFinalMeasurementsSearchOptionRegexp() const -> bool;
    void SetFinalMeasurementsSearchOptionRegexp(bool value);

    auto GetFinalMeasurementsSearchOptionMatchCase() const -> bool;
    void SetFinalMeasurementsSearchOptionMatchCase(bool value);

    auto GetBackgroundImageDefOpacity() const -> int;
    void SetBackgroundImageDefOpacity(int value);

    auto GetShowGrainline() const -> bool;
    void SetShowGrainline(bool value);

    auto GetPointerMode() const -> VToolPointerMode;
    void SetPointerMode(VToolPointerMode mode);

    auto IsUseToolGroups() const -> bool;
    void SetUseToolGroups(bool value);

private:
    Q_DISABLE_COPY_MOVE(VValentinaSettings) // NOLINT

    template <typename T>
    auto GetCachedValue(T &cache, const QString &setting, T defValue, T valueMin, T valueMax) const -> T;
};

#endif // VVALENTINASETTINGS_H
