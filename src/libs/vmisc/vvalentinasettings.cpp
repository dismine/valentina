/************************************************************************
 **
 **  @file   vsettings.cpp
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

#include "vvalentinasettings.h"

#include <QDir>
#include <QGlobalStatic>
#include <QLocale>
#include <QMargins>
#include <QMetaType>
#include <QPrinter>
#include <QPrinterInfo>
#include <QSharedPointer>
#include <QVariant>
#include <QtDebug>

#include "compatibility.h"
#include "def.h"
#include "qglobal.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)

#ifndef QPRINTENGINE_H
Q_DECLARE_METATYPE(QMarginsF) // NOLINT
#endif

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingConfigurationLabelLanguage,
                          (QLatin1String("configuration/label_language")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingAutoRefreshPatternMessage,
                          (QLatin1String("configuration/autoRefreshPatternMessage")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingToolPanelScaling, (QLatin1String("configuration/toolPanelScaling")))

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPathsLayout, (QLatin1String("paths/layout"))) // NOLINT

// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternKnownMaterials, (QLatin1String("pattern/knownMaterials")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternRememberMaterials, (QLatin1String("pattern/rememberMaterials")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternBackgroundImageDefOpacity,
                          (QLatin1String("pattern/backgroundImageDefOpacity")))

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutWidth, (QLatin1String("layout/width")))             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutSorting, (QLatin1String("layout/sorting")))         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutPaperHeight, (QLatin1String("layout/paperHeight"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutPaperWidth, (QLatin1String("layout/paperWidth")))   // NOLINT
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutFollowGrainline, (QLatin1String("layout/followGrainline")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutManualPriority, (QLatin1String("layout/manualPriority")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutNestQuantity, (QLatin1String("layout/nestQuantity"))) // NOLINT
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutAutoCropLength, (QLatin1String("layout/autoCropLength")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutAutoCropWidth, (QLatin1String("layout/autoCropWidth"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutSaveLength, (QLatin1String("layout/saveLength")))       // NOLINT
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutPreferOneSheetSolution,
                          (QLatin1String("layout/preferOneSheetSolution")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutUnitePages, (QLatin1String("layout/unitePages")))       // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutShowGrainline, (QLatin1String("layout/showGrainline"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingFields, (QLatin1String("layout/fields")))                     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingIgnoreFields, (QLatin1String("layout/ignoreFields")))         // NOLINT
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingStripOptimization, (QLatin1String("layout/stripOptimization")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingMultiplier, (QLatin1String("layout/multiplier")))   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingTextAsPaths, (QLatin1String("layout/textAsPaths"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingNestingTime, (QLatin1String("layout/time")))        // NOLINT
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingEfficiencyCoefficient, (QLatin1String("layout/efficiencyCoefficient")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutExportFormat, (QLatin1String("layout/exportFormat"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingDetailExportFormat, (QLatin1String("detail/exportFormat"))) // NOLINT

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingTiledPDFPaperHeight, (QLatin1String("tiledPDF/paperHeight"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingTiledPDFPaperWidth, (QLatin1String("tiledPDF/paperWidth")))   // NOLINT

// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingdockWidgetGroupsActive, (QLatin1String("dockWidget/groupsActive")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingDockWidgetToolOptionsActive,
                          (QLatin1String("dockWidget/toolOptionsActive")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingDockWidgetPatternMessagesActive,
                          (QLatin1String("dockWidget/patternMessagesActive")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingDockWidgetBackgroundImagesActive,
                          (QLatin1String("dockWidget/backgroundImagesActive")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternMessagesFontSize, (QLatin1String("font/patternMessagesSize")))

// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchHistoryHistory, (QLatin1String("searchHistory/history")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchHistoryIncrements, (QLatin1String("searchHistory/increments")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchHistoryPreviewCalculations,
                          (QLatin1String("searchHistory/previewCalculations")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchHistoryFinalMeasurements,
                          (QLatin1String("searchHistory/finalMeasurements")))

// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsHistoryUseUnicodeProperties,
                          (QLatin1String("searchOptions/historyUseUnicodeProperties")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsHistoryWholeWord,
                          (QLatin1String("searchOptions/historyWholeWord")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsHistoryRegexp,
                          (QLatin1String("searchOptions/historyRegexp")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsHistoryMatchCase,
                          (QLatin1String("searchOptions/historyMatchCase")))

// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsIncrementsUseUnicodeProperties,
                          (QLatin1String("searchOptions/incrementsUseUnicodeProperties")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsIncrementsWholeWord,
                          (QLatin1String("searchOptions/incrementsWholeWord")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsIncrementsRegexp,
                          (QLatin1String("searchOptions/incrementsRegexp")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsIncrementsMatchCase,
                          (QLatin1String("searchOptions/incrementsMatchCase")))

// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsPreviewCalculationsUseUnicodeProperties,
                          (QLatin1String("searchOptions/previewCalculationsUseUnicodeProperties")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsPreviewCalculationsWholeWord,
                          (QLatin1String("searchOptions/previewCalculationsWholeWord")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsPreviewCalculationsRegexp,
                          (QLatin1String("searchOptions/previewCalculationsRegexp")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsPreviewCalculationsMatchCase,
                          (QLatin1String("searchOptions/previewCalculationsMatchCase")))

// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsFinalMeasurementsUseUnicodeProperties,
                          (QLatin1String("searchOptions/finalMeasurementsUseUnicodeProperties")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsFinalMeasurementsWholeWord,
                          (QLatin1String("searchOptions/finalMeasurementsWholeWord")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsFinalMeasurementsRegexp,
                          (QLatin1String("searchOptions/finalMeasurementsRegexp")))
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsFinalMeasurementsMatchCase,
                          (QLatin1String("searchOptions/finalMeasurementsMatchCase")))
QT_WARNING_POP
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VValentinaSettings::VValentinaSettings(Format format, Scope scope, const QString &organization,
                                       const QString &application, QObject *parent)
  : VCommonSettings(format, scope, organization, application, parent)
{
    REGISTER_META_TYPE_STREAM_OPERATORS(QMarginsF);
}

//---------------------------------------------------------------------------------------------------------------------
VValentinaSettings::VValentinaSettings(const QString &fileName, QSettings::Format format, QObject *parent)
  : VCommonSettings(fileName, format, parent)
{
    REGISTER_META_TYPE_STREAM_OPERATORS(QMarginsF);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetLabelLanguage() const -> QString
{
    return value(*settingConfigurationLabelLanguage, QLocale().bcp47Name()).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLabelLanguage(const QString &value)
{
    setValue(*settingConfigurationLabelLanguage, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefPathLayout() -> QString
{
    return QDir::homePath() + QStringLiteral("/valentina/") + tr("layouts");
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetPathLayout() const -> QString
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), this->applicationName());
    return settings.value(*settingPathsLayout, GetDefPathLayout()).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetPathLayout(const QString &value)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), this->applicationName());
    settings.setValue(*settingPathsLayout, value);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetLayoutPaperHeight() const -> qreal
{
    return ValueOrDef<qreal>(*this, *settingLayoutPaperHeight, UnitConvertor(1189 /*A0*/, Unit::Mm, Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutPaperHeight(qreal value)
{
    setValue(*settingLayoutPaperHeight, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetLayoutPaperWidth() const -> qreal
{
    return ValueOrDef<qreal>(*this, *settingLayoutPaperWidth, UnitConvertor(841 /*A0*/, Unit::Mm, Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutPaperWidth(qreal value)
{
    setValue(*settingLayoutPaperWidth, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetLayoutWidth() const -> qreal
{
    return ValueOrDef<qreal>(*this, *settingLayoutWidth, GetDefLayoutWidth());
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefLayoutWidth() -> qreal
{
    return UnitConvertor(2.5, Unit::Mm, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutWidth(qreal value)
{
    setValue(*settingLayoutWidth, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetNestingTime() const -> int
{
    return ValueOrDef<int>(*this, *settingNestingTime, GetDefNestingTime());
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetNestingTime(int value)
{
    setValue(*settingNestingTime, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetEfficiencyCoefficient() const -> qreal
{
    return ValueOrDef<qreal>(*this, *settingEfficiencyCoefficient, GetDefEfficiencyCoefficient());
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetEfficiencyCoefficient(qreal value)
{
    setValue(*settingEfficiencyCoefficient, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetFields(const QMarginsF &def) const -> QMarginsF
{
    return ValueOrDef<QMarginsF>(*this, *settingFields, def);
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetFields(const QMarginsF &value)
{
    setValue(*settingFields, QVariant::fromValue(value));
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetLayoutGroup() const -> Cases
{
    return ValueOrDef<Cases>(*this, *settingLayoutSorting, GetDefLayoutGroup());
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefLayoutGroup() -> Cases
{
    return Cases::CaseDesc;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutGroup(const Cases &value)
{
    setValue(*settingLayoutSorting, static_cast<int>(value));
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetLayoutFollowGrainline() const -> bool
{
    return value(*settingLayoutFollowGrainline, GetDefLayoutFollowGrainline()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefLayoutFollowGrainline() -> bool
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutFollowGrainline(bool value)
{
    setValue(*settingLayoutFollowGrainline, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetLayoutManualPriority() const -> bool
{
    return value(*settingLayoutManualPriority, GetDefLayoutManualPriority()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefLayoutManualPriority() -> bool
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutManualPriority(bool value)
{
    setValue(*settingLayoutManualPriority, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetLayoutNestQuantity() const -> bool
{
    return value(*settingLayoutNestQuantity, GetDefLayoutNestQuantity()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefLayoutNestQuantity() -> bool
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutNestQuantity(bool value)
{
    setValue(*settingLayoutNestQuantity, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetLayoutAutoCropLength() const -> bool
{
    return value(*settingLayoutAutoCropLength, GetDefLayoutAutoCropLength()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefLayoutAutoCropLength() -> bool
{
    return false;
}
//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutAutoCropLength(bool value)
{
    setValue(*settingLayoutAutoCropLength, value);
}
//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetLayoutAutoCropWidth() const -> bool
{
    return value(*settingLayoutAutoCropWidth, GetDefLayoutAutoCropWidth()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefLayoutAutoCropWidth() -> bool
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutAutoCropWidth(bool value)
{
    setValue(*settingLayoutAutoCropWidth, value);
}
//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetLayoutSaveLength() const -> bool
{
    return value(*settingLayoutSaveLength, GetDefLayoutSaveLength()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefLayoutSaveLength() -> bool
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutSaveLength(bool value)
{
    setValue(*settingLayoutSaveLength, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetLayoutPreferOneSheetSolution() const -> bool
{
    return value(*settingLayoutPreferOneSheetSolution, GetDefLayoutPreferOneSheetSolution()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefLayoutPreferOneSheetSolution() -> bool
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutPreferOneSheetSolution(bool value)
{
    setValue(*settingLayoutPreferOneSheetSolution, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetLayoutUnitePages() const -> bool
{
    return value(*settingLayoutUnitePages, GetDefLayoutUnitePages()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefLayoutUnitePages() -> bool
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutUnitePages(bool value)
{
    setValue(*settingLayoutUnitePages, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetIgnoreAllFields() const -> bool
{
    return value(*settingIgnoreFields, GetDefIgnoreAllFields()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefIgnoreAllFields() -> bool
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetIgnoreAllFields(bool value)
{
    setValue(*settingIgnoreFields, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetStripOptimization() const -> bool
{
    return value(*settingStripOptimization, GetDefStripOptimization()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefStripOptimization() -> bool
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetStripOptimization(bool value)
{
    setValue(*settingStripOptimization, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetMultiplier() const -> quint8
{
    return static_cast<quint8>(value(*settingMultiplier, GetDefMultiplier()).toUInt());
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefMultiplier() -> quint8
{
    return 1;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetMultiplier(quint8 value)
{
    setValue(*settingMultiplier, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetTextAsPaths() const -> bool
{
    return value(*settingTextAsPaths, GetDefTextAsPaths()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefTextAsPaths() -> bool
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetTextAsPaths(bool value)
{
    setValue(*settingTextAsPaths, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetKnownMaterials() const -> QStringList
{
    return value(*settingPatternKnownMaterials, QStringList()).toStringList();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetKnownMaterials(const QStringList &list)
{
    setValue(*settingPatternKnownMaterials, list);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::IsRememberPatternMaterials() const -> bool
{
    return value(*settingPatternRememberMaterials, true).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetRememberPatternMaterials(bool value)
{
    setValue(*settingPatternRememberMaterials, value);
}

// settings for the tiled PDFs
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetTiledPDFPaperHeight returns the paper height of tiled pdf in the desired unit.
 * @param unit the unit in which are the value. Necessary because we save the values
 * internaly as mm so there is conversion beeing made.
 * @return tiled pdf paper height
 */
auto VValentinaSettings::GetTiledPDFPaperHeight(const Unit &unit) const -> qreal
{
    return UnitConvertor(ValueOrDef<qreal>(*this, *settingTiledPDFPaperHeight, 297 /*A4*/), Unit::Mm, unit);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetTiledPDFPaperHeight sets the tiled pdf paper height
 * @param value in mm
 * @param unit unit of the given value
 */
void VValentinaSettings::SetTiledPDFPaperHeight(qreal value, const Unit &unit)
{
    setValue(*settingTiledPDFPaperHeight, UnitConvertor(value, unit, Unit::Mm));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetTiledPDFPaperWidth returns the paper height of tiled pdf in the desired unit.
 * @param unit the unit in which are the value. Necessary because we save the values
 * internaly as mm so there is conversion beeing made.
 * @return tiled pdf paper width
 */
auto VValentinaSettings::GetTiledPDFPaperWidth(const Unit &unit) const -> qreal
{
    return UnitConvertor(ValueOrDef<qreal>(*this, *settingTiledPDFPaperWidth, 210 /*A4*/), Unit::Mm, unit);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetTiledPDFPaperWidth sets the tiled pdf paper width
 * @param unit unit of the given value
 * @param value in mm
 */
void VValentinaSettings::SetTiledPDFPaperWidth(qreal value, const Unit &unit)
{
    setValue(*settingTiledPDFPaperWidth, UnitConvertor(value, unit, Unit::Mm));
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::IsDockWidgetGroupsActive() const -> bool
{
    return value(*settingdockWidgetGroupsActive, GetDefDockWidgetGroupsActive()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefDockWidgetGroupsActive() -> bool
{
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetDockWidgetGroupsActive(bool value)
{
    setValue(*settingdockWidgetGroupsActive, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::IsDockWidgetToolOptionsActive() const -> bool
{
    return value(*settingDockWidgetToolOptionsActive, GetDefDockWidgetToolOptionsActive()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefDockWidgetToolOptionsActive() -> bool
{
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetDockWidgetToolOptionsActive(bool value)
{
    setValue(*settingDockWidgetToolOptionsActive, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::IsDockWidgetPatternMessagesActive() const -> bool
{
    return value(*settingDockWidgetPatternMessagesActive, GetDefDockWidgetPatternMessagesActive()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefDockWidgetPatternMessagesActive() -> bool
{
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetDockWidgetPatternMessagesActive(bool value)
{
    setValue(*settingDockWidgetPatternMessagesActive, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::IsDockWidgetBackgroundImagesActive() const -> bool
{
    return value(*settingDockWidgetBackgroundImagesActive, GetDefDockWidgetBackgroundImagesActive()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefDockWidgetBackgroundImagesActive() -> bool
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetDockWidgetBackgroundImagesActive(bool value)
{
    setValue(*settingDockWidgetBackgroundImagesActive, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetPatternMessageFontSize(int fontSizeDef) const -> int
{
    fontSizeDef = qBound(GetDefMinPatternMessageFontSize(), fontSizeDef, GetDefMaxPatternMessageFontSize());
    const int fontSize = value(*settingPatternMessagesFontSize, fontSizeDef).toInt();
    return qBound(GetDefMinPatternMessageFontSize(), fontSize, GetDefMaxPatternMessageFontSize());
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefMinPatternMessageFontSize() -> int
{
    return 5;
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDefMaxPatternMessageFontSize() -> int
{
    return 40;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetPatternMessageFontSize(int size)
{
    setValue(*settingPatternMessagesFontSize,
             qBound(GetDefMinPatternMessageFontSize(), size, GetDefMaxPatternMessageFontSize()));
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetAutoRefreshPatternMessage() const -> bool
{
    return value(*settingAutoRefreshPatternMessage, true).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetAutoRefreshPatternMessage(bool value)
{
    setValue(*settingAutoRefreshPatternMessage, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetToolPanelScaling() const -> bool
{
    return value(*settingToolPanelScaling, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetToolPanelScaling(const bool &value)
{
    setValue(*settingToolPanelScaling, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetLayoutExportFormat() const -> qint8
{
    return CastToLayoutExportFormat(qvariant_cast<qint8>(value(*settingLayoutExportFormat, 0)));
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutExportFormat(qint8 format)
{
    setValue(*settingLayoutExportFormat, format);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetDetailExportFormat() const -> qint8
{
    return CastToLayoutExportFormat(qvariant_cast<qint8>(value(*settingDetailExportFormat, 0)));
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetDetailExportFormat(qint8 format)
{
    setValue(*settingDetailExportFormat, format);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetHistorySearchHistory() const -> QStringList
{
    return value(*settingSearchHistoryHistory).toStringList();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetHistorySearchHistory(const QStringList &history)
{
    setValue(*settingSearchHistoryHistory, history);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetIncrementsSearchHistory() const -> QStringList
{
    return value(*settingSearchHistoryIncrements).toStringList();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetIncrementsSearchHistory(const QStringList &history)
{
    setValue(*settingSearchHistoryIncrements, history);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetPreviewCalculationsSearchHistory() const -> QStringList
{
    return value(*settingSearchHistoryPreviewCalculations).toStringList();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetPreviewCalculationsSearchHistory(const QStringList &history)
{
    setValue(*settingSearchHistoryPreviewCalculations, history);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetFinalMeasurementsSearchHistory() const -> QStringList
{
    return value(*settingSearchHistoryFinalMeasurements).toStringList();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetFinalMeasurementsSearchHistory(const QStringList &history)
{
    setValue(*settingSearchHistoryFinalMeasurements, history);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetHistorySearchOptionUseUnicodeProperties() const -> bool
{
    return value(*settingSearchOptionsHistoryUseUnicodeProperties, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetHistorySearchOptionUseUnicodeProperties(bool value)
{
    setValue(*settingSearchOptionsHistoryUseUnicodeProperties, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetHistorySearchOptionWholeWord() const -> bool
{
    return value(*settingSearchOptionsHistoryWholeWord, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetHistorySearchOptionWholeWord(bool value)
{
    setValue(*settingSearchOptionsHistoryWholeWord, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetHistorySearchOptionRegexp() const -> bool
{
    return value(*settingSearchOptionsHistoryRegexp, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetHistorySearchOptionRegexp(bool value)
{
    setValue(*settingSearchOptionsHistoryRegexp, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetHistorySearchOptionMatchCase() const -> bool
{
    return value(*settingSearchOptionsHistoryMatchCase, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetHistorySearchOptionMatchCase(bool value)
{
    setValue(*settingSearchOptionsHistoryMatchCase, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetIncrementsSearchOptionUseUnicodeProperties() const -> bool
{
    return value(*settingSearchOptionsIncrementsUseUnicodeProperties, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetIncrementsSearchOptionUseUnicodeProperties(bool value)
{
    setValue(*settingSearchOptionsIncrementsUseUnicodeProperties, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetIncrementsSearchOptionWholeWord() const -> bool
{
    return value(*settingSearchOptionsIncrementsWholeWord, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetIncrementsSearchOptionWholeWord(bool value)
{
    setValue(*settingSearchOptionsIncrementsWholeWord, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetIncrementsSearchOptionRegexp() const -> bool
{
    return value(*settingSearchOptionsIncrementsRegexp, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetIncrementsSearchOptionRegexp(bool value)
{
    setValue(*settingSearchOptionsIncrementsRegexp, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetIncrementsSearchOptionMatchCase() const -> bool
{
    return value(*settingSearchOptionsIncrementsMatchCase, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetIncrementsSearchOptionMatchCase(bool value)
{
    setValue(*settingSearchOptionsIncrementsMatchCase, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetPreviewCalculationsSearchOptionUseUnicodeProperties() const -> bool
{
    return value(*settingSearchOptionsPreviewCalculationsUseUnicodeProperties, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetPreviewCalculationsSearchOptionUseUnicodeProperties(bool value)
{
    setValue(*settingSearchOptionsPreviewCalculationsUseUnicodeProperties, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetPreviewCalculationsSearchOptionWholeWord() const -> bool
{
    return value(*settingSearchOptionsPreviewCalculationsWholeWord, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetPreviewCalculationsSearchOptionWholeWord(bool value)
{
    setValue(*settingSearchOptionsPreviewCalculationsWholeWord, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetPreviewCalculationsSearchOptionRegexp() const -> bool
{
    return value(*settingSearchOptionsPreviewCalculationsRegexp, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetPreviewCalculationsSearchOptionRegexp(bool value)
{
    setValue(*settingSearchOptionsPreviewCalculationsRegexp, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetPreviewCalculationsSearchOptionMatchCase() const -> bool
{
    return value(*settingSearchOptionsPreviewCalculationsMatchCase, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetPreviewCalculationsSearchOptionMatchCase(bool value)
{
    setValue(*settingSearchOptionsPreviewCalculationsMatchCase, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetFinalMeasurementsSearchOptionUseUnicodeProperties() const -> bool
{
    return value(*settingSearchOptionsFinalMeasurementsUseUnicodeProperties, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetFinalMeasurementsSearchOptionUseUnicodeProperties(bool value)
{
    setValue(*settingSearchOptionsFinalMeasurementsUseUnicodeProperties, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetFinalMeasurementsSearchOptionWholeWord() const -> bool
{
    return value(*settingSearchOptionsFinalMeasurementsWholeWord, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetFinalMeasurementsSearchOptionWholeWord(bool value)
{
    setValue(*settingSearchOptionsFinalMeasurementsWholeWord, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetFinalMeasurementsSearchOptionRegexp() const -> bool
{
    return value(*settingSearchOptionsFinalMeasurementsRegexp, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetFinalMeasurementsSearchOptionRegexp(bool value)
{
    setValue(*settingSearchOptionsFinalMeasurementsRegexp, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetFinalMeasurementsSearchOptionMatchCase() const -> bool
{
    return value(*settingSearchOptionsFinalMeasurementsMatchCase, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetFinalMeasurementsSearchOptionMatchCase(bool value)
{
    setValue(*settingSearchOptionsFinalMeasurementsMatchCase, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetBackgroundImageDefOpacity() const -> int
{
    return value(*settingPatternBackgroundImageDefOpacity, 100).toInt();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetBackgroundImageDefOpacity(int value)
{
    setValue(*settingPatternBackgroundImageDefOpacity, qBound(0, value, 100));
}

//---------------------------------------------------------------------------------------------------------------------
auto VValentinaSettings::GetShowGrainline() const -> bool
{
    return value(*settingLayoutShowGrainline, true).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetShowGrainline(bool value)
{
    setValue(*settingLayoutShowGrainline, value);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
auto VValentinaSettings::GetCachedValue(T &cache, const QString &setting, T defValue, T valueMin, T valueMax) const -> T
{
    if (cache < 0)
    {
        cache = qBound(valueMin, ValueOrDef(setting, defValue), valueMax);
    }

    return cache;
}
