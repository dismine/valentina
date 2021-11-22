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
#include <QLocale>
#include <QMetaType>
#include <QPrinter>
#include <QSharedPointer>
#include <QStaticStringData>
#include <QStringData>
#include <QStringDataPtr>
#include <QVariant>
#include <QPrinterInfo>
#include <QtDebug>
#include <QGlobalStatic>

#include "../vmisc/def.h"
#include "../vmisc/vmath.h"
#include "../vlayout/vbank.h"

Q_DECLARE_METATYPE(QMarginsF)

namespace
{
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingConfigurationLabelLanguage,
                          (QLatin1String("configuration/label_language")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingAutoRefreshPatternMessage,
                          (QLatin1String("configuration/autoRefreshPatternMessage")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingToolPanelScaling,
                          (QLatin1String("configuration/toolPanelScaling")))

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPathsLayout, (QLatin1String("paths/layout")))

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternKnownMaterials, (QLatin1String("pattern/knownMaterials")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternRememberMaterials, (QLatin1String("pattern/rememberMaterials")))

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutWidth, (QLatin1String("layout/width")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutSorting, (QLatin1String("layout/sorting")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutPaperHeight, (QLatin1String("layout/paperHeight")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutPaperWidth, (QLatin1String("layout/paperWidth")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutFollowGrainline, (QLatin1String("layout/followGrainline")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutManualPriority, (QLatin1String("layout/manualPriority")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutNestQuantity, (QLatin1String("layout/nestQuantity")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutAutoCropLength, (QLatin1String("layout/autoCropLength")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutAutoCropWidth, (QLatin1String("layout/autoCropWidth")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutSaveLength, (QLatin1String("layout/saveLength")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutPreferOneSheetSolution,
                          (QLatin1String("layout/preferOneSheetSolution")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutUnitePages, (QLatin1String("layout/unitePages")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingFields, (QLatin1String("layout/fields")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingIgnoreFields, (QLatin1String("layout/ignoreFields")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingStripOptimization, (QLatin1String("layout/stripOptimization")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingMultiplier, (QLatin1String("layout/multiplier")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingTextAsPaths, (QLatin1String("layout/textAsPaths")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingNestingTime, (QLatin1String("layout/time")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingEfficiencyCoefficient, (QLatin1String("layout/efficiencyCoefficient")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutExportFormat, (QLatin1String("layout/exportFormat")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingDetailExportFormat, (QLatin1String("detail/exportFormat")))

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingTiledPDFPaperHeight, (QLatin1String("tiledPDF/paperHeight")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingTiledPDFPaperWidth, (QLatin1String("tiledPDF/paperWidth")))

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingdockWidgetGroupsActive, (QLatin1String("dockWidget/groupsActive")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingDockWidgetToolOptionsActive,
                          (QLatin1String("dockWidget/toolOptionsActive")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingDockWidgetPatternMessagesActive,
                          (QLatin1String("dockWidget/patternMessagesActive")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternMessagesFontSize, (QLatin1String("font/patternMessagesSize")))

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchHistoryHistory, (QLatin1String("searchHistory/history")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchHistoryIncrements, (QLatin1String("searchHistory/increments")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchHistoryPreviewCalculations,
                          (QLatin1String("searchHistory/previewCalculations")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchHistoryFinalMeasurements,
                          (QLatin1String("searchHistory/finalMeasurements")))

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsHistoryUseUnicodeProperties,
                          (QLatin1String("searchOptions/historyUseUnicodeProperties")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsHistoryWholeWord,
                          (QLatin1String("searchOptions/historyWholeWord")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsHistoryRegexp,
                          (QLatin1String("searchOptions/historyRegexp")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsHistoryMatchCase,
                          (QLatin1String("searchOptions/historyMatchCase")))

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsIncrementsUseUnicodeProperties,
                          (QLatin1String("searchOptions/incrementsUseUnicodeProperties")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsIncrementsWholeWord,
                          (QLatin1String("searchOptions/incrementsWholeWord")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsIncrementsRegexp,
                          (QLatin1String("searchOptions/incrementsRegexp")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsIncrementsMatchCase,
                          (QLatin1String("searchOptions/incrementsMatchCase")))

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsPreviewCalculationsUseUnicodeProperties,
                          (QLatin1String("searchOptions/previewCalculationsUseUnicodeProperties")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsPreviewCalculationsWholeWord,
                          (QLatin1String("searchOptions/previewCalculationsWholeWord")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsPreviewCalculationsRegexp,
                          (QLatin1String("searchOptions/previewCalculationsRegexp")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsPreviewCalculationsMatchCase,
                          (QLatin1String("searchOptions/previewCalculationsMatchCase")))

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsFinalMeasurementsUseUnicodeProperties,
                          (QLatin1String("searchOptions/finalMeasurementsUseUnicodeProperties")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsFinalMeasurementsWholeWord,
                          (QLatin1String("searchOptions/finalMeasurementsWholeWord")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsFinalMeasurementsRegexp,
                          (QLatin1String("searchOptions/finalMeasurementsRegexp")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingSearchOptionsFinalMeasurementsMatchCase,
                          (QLatin1String("searchOptions/finalMeasurementsMatchCase")))
}  // namespace

//---------------------------------------------------------------------------------------------------------------------
VValentinaSettings::VValentinaSettings(Format format, Scope scope, const QString &organization, const QString &application,
                     QObject *parent)
    :VCommonSettings(format, scope, organization, application, parent)
{
    qRegisterMetaTypeStreamOperators<QMarginsF>("QMarginsF");
}

//---------------------------------------------------------------------------------------------------------------------
VValentinaSettings::VValentinaSettings(const QString &fileName, QSettings::Format format, QObject *parent)
    :VCommonSettings(fileName, format, parent)
{
    qRegisterMetaTypeStreamOperators<QMarginsF>("QMarginsF");
}

//---------------------------------------------------------------------------------------------------------------------
QString VValentinaSettings::GetLabelLanguage() const
{
    return value(*settingConfigurationLabelLanguage, QLocale().bcp47Name()).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLabelLanguage(const QString &value)
{
    setValue(*settingConfigurationLabelLanguage, value);
}

//---------------------------------------------------------------------------------------------------------------------
QString VValentinaSettings::GetDefPathLayout()
{
    return QDir::homePath() + QStringLiteral("/valentina/") + tr("layouts");
}

//---------------------------------------------------------------------------------------------------------------------
QString VValentinaSettings::GetPathLayout() const
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
qreal VValentinaSettings::GetLayoutPaperHeight() const
{
    return ValueOrDef<qreal>(*this, *settingLayoutPaperHeight, UnitConvertor(1189/*A0*/, Unit::Mm, Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutPaperHeight(qreal value)
{
    setValue(*settingLayoutPaperHeight, value);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VValentinaSettings::GetLayoutPaperWidth() const
{
    return ValueOrDef<qreal>(*this, *settingLayoutPaperWidth, UnitConvertor(841/*A0*/, Unit::Mm, Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutPaperWidth(qreal value)
{
    setValue(*settingLayoutPaperWidth, value);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VValentinaSettings::GetLayoutWidth() const
{
    return ValueOrDef<qreal>(*this, *settingLayoutWidth, GetDefLayoutWidth());
}

//---------------------------------------------------------------------------------------------------------------------
qreal VValentinaSettings::GetDefLayoutWidth()
{
    return UnitConvertor(2.5, Unit::Mm, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutWidth(qreal value)
{
    setValue(*settingLayoutWidth, value);
}

//---------------------------------------------------------------------------------------------------------------------
int VValentinaSettings::GetNestingTime() const
{
    return ValueOrDef<int>(*this, *settingNestingTime, GetDefNestingTime());
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetNestingTime(int value)
{
    setValue(*settingNestingTime, value);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VValentinaSettings::GetEfficiencyCoefficient() const
{
    return ValueOrDef<qreal>(*this, *settingEfficiencyCoefficient, GetDefEfficiencyCoefficient());
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetEfficiencyCoefficient(qreal value)
{
    setValue(*settingEfficiencyCoefficient, value);
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF VValentinaSettings::GetFields(const QMarginsF &def) const
{
    return ValueOrDef<QMarginsF>(*this, *settingFields, def);
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetFields(const QMarginsF &value)
{
    setValue(*settingFields, QVariant::fromValue(value));
}

//---------------------------------------------------------------------------------------------------------------------
Cases VValentinaSettings::GetLayoutGroup() const
{
    return ValueOrDef<Cases>(*this, *settingLayoutSorting, GetDefLayoutGroup());
}

//---------------------------------------------------------------------------------------------------------------------
Cases VValentinaSettings::GetDefLayoutGroup()
{
    return Cases::CaseDesc;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutGroup(const Cases &value)
{
    setValue(*settingLayoutSorting, static_cast<int>(value));
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetLayoutFollowGrainline() const
{
    return value(*settingLayoutFollowGrainline, GetDefLayoutFollowGrainline()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetDefLayoutFollowGrainline()
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutFollowGrainline(bool value)
{
    setValue(*settingLayoutFollowGrainline, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetLayoutManualPriority() const
{
    return value(*settingLayoutManualPriority, GetDefLayoutManualPriority()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetDefLayoutManualPriority()
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutManualPriority(bool value)
{
    setValue(*settingLayoutManualPriority, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetLayoutNestQuantity() const
{
    return value(*settingLayoutNestQuantity, GetDefLayoutNestQuantity()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetDefLayoutNestQuantity()
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutNestQuantity(bool value)
{
    setValue(*settingLayoutNestQuantity, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetLayoutAutoCropLength() const
{
    return value(*settingLayoutAutoCropLength, GetDefLayoutAutoCropLength()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetDefLayoutAutoCropLength()
{
    return false;
}
//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutAutoCropLength(bool value)
{
    setValue(*settingLayoutAutoCropLength, value);
}
//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetLayoutAutoCropWidth() const
{
    return value(*settingLayoutAutoCropWidth, GetDefLayoutAutoCropWidth()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetDefLayoutAutoCropWidth()
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutAutoCropWidth(bool value)
{
    setValue(*settingLayoutAutoCropWidth, value);
}
//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetLayoutSaveLength() const
{
    return value(*settingLayoutSaveLength, GetDefLayoutSaveLength()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetDefLayoutSaveLength()
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutSaveLength(bool value)
{
    setValue(*settingLayoutSaveLength, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetLayoutPreferOneSheetSolution() const
{
    return value(*settingLayoutPreferOneSheetSolution, GetDefLayoutPreferOneSheetSolution()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetDefLayoutPreferOneSheetSolution()
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutPreferOneSheetSolution(bool value)
{
    setValue(*settingLayoutPreferOneSheetSolution, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetLayoutUnitePages() const
{
    return value(*settingLayoutUnitePages, GetDefLayoutUnitePages()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetDefLayoutUnitePages()
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutUnitePages(bool value)
{
    setValue(*settingLayoutUnitePages, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetIgnoreAllFields() const
{
    return value(*settingIgnoreFields, GetDefIgnoreAllFields()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetDefIgnoreAllFields()
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetIgnoreAllFields(bool value)
{
    setValue(*settingIgnoreFields, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetStripOptimization() const
{
    return value(*settingStripOptimization, GetDefStripOptimization()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetDefStripOptimization()
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetStripOptimization(bool value)
{
    setValue(*settingStripOptimization, value);
}

//---------------------------------------------------------------------------------------------------------------------
quint8 VValentinaSettings::GetMultiplier() const
{
    return static_cast<quint8>(value(*settingMultiplier, GetDefMultiplier()).toUInt());
}

//---------------------------------------------------------------------------------------------------------------------
quint8 VValentinaSettings::GetDefMultiplier()
{
    return 1;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetMultiplier(quint8 value)
{
    setValue(*settingMultiplier, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetTextAsPaths() const
{
    return value(*settingTextAsPaths, GetDefTextAsPaths()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetDefTextAsPaths()
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetTextAsPaths(bool value)
{
    setValue(*settingTextAsPaths, value);
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VValentinaSettings::GetKnownMaterials() const
{
    return value(*settingPatternKnownMaterials, QStringList()).toStringList();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetKnownMaterials(const QStringList &list)
{
    setValue(*settingPatternKnownMaterials, list);
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::IsRememberPatternMaterials() const
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
qreal VValentinaSettings::GetTiledPDFPaperHeight(const Unit &unit) const
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
qreal VValentinaSettings::GetTiledPDFPaperWidth(const Unit &unit) const
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
    setValue(*settingTiledPDFPaperWidth, UnitConvertor(value,unit, Unit::Mm));
}


//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::IsDockWidgetGroupsActive() const
{
    return value(*settingdockWidgetGroupsActive, GetDefDockWidgetGroupsActive()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetDefDockWidgetGroupsActive()
{
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetDockWidgetGroupsActive(bool value)
{
    setValue(*settingdockWidgetGroupsActive, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::IsDockWidgetToolOptionsActive() const
{
    return value(*settingDockWidgetToolOptionsActive, GetDefDockWidgetToolOptionsActive()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetDefDockWidgetToolOptionsActive()
{
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetDockWidgetToolOptionsActive(bool value)
{
    setValue(*settingDockWidgetToolOptionsActive, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::IsDockWidgetPatternMessagesActive() const
{
    return value(*settingDockWidgetPatternMessagesActive, GetDefDockWidgetPatternMessagesActive()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetDefDockWidgetPatternMessagesActive()
{
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetDockWidgetPatternMessagesActive(bool value)
{
    setValue(*settingDockWidgetPatternMessagesActive, value);
}

//---------------------------------------------------------------------------------------------------------------------
int VValentinaSettings::GetPatternMessageFontSize(int fontSizeDef) const
{
    fontSizeDef = qBound(GetDefMinPatternMessageFontSize(), fontSizeDef, GetDefMaxPatternMessageFontSize());
    const int fontSize = value(*settingPatternMessagesFontSize, fontSizeDef).toInt();
    return qBound(GetDefMinPatternMessageFontSize(), fontSize, GetDefMaxPatternMessageFontSize());
}

//---------------------------------------------------------------------------------------------------------------------
int VValentinaSettings::GetDefMinPatternMessageFontSize()
{
    return 5;
}

//---------------------------------------------------------------------------------------------------------------------
int VValentinaSettings::GetDefMaxPatternMessageFontSize()
{
    return 40;
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetPatternMessageFontSize(int size)
{
    setValue(*settingPatternMessagesFontSize, qBound(GetDefMinPatternMessageFontSize(), size,
                                                     GetDefMaxPatternMessageFontSize()));
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetAutoRefreshPatternMessage() const
{
    return value(*settingAutoRefreshPatternMessage, true).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetAutoRefreshPatternMessage(bool value)
{
    setValue(*settingAutoRefreshPatternMessage, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VValentinaSettings::GetToolPanelScaling() const
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
    return qvariant_cast<qint8>(value(*settingLayoutExportFormat, 0));
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetLayoutExportFormat(qint8 format)
{
    setValue(*settingLayoutExportFormat, format);
}

//---------------------------------------------------------------------------------------------------------------------
qint8 VValentinaSettings::GetDetailExportFormat() const
{
    return qvariant_cast<qint8>(value(*settingDetailExportFormat, 0));
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
QStringList VValentinaSettings::GetIncrementsSearchHistory() const
{
    return value(*settingSearchHistoryIncrements).toStringList();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetIncrementsSearchHistory(const QStringList &history)
{
    setValue(*settingSearchHistoryIncrements, history);
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VValentinaSettings::GetPreviewCalculationsSearchHistory() const
{
    return value(*settingSearchHistoryPreviewCalculations).toStringList();
}

//---------------------------------------------------------------------------------------------------------------------
void VValentinaSettings::SetPreviewCalculationsSearchHistory(const QStringList &history)
{
    setValue(*settingSearchHistoryPreviewCalculations, history);
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VValentinaSettings::GetFinalMeasurementsSearchHistory() const
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
template<typename T>
T VValentinaSettings::GetCachedValue(T &cache, const QString &setting, T defValue, T valueMin, T valueMax) const
{
    if (cache < 0)
    {
        cache = qBound(valueMin, ValueOrDef(setting, defValue), valueMax);
    }

    return cache;
}
