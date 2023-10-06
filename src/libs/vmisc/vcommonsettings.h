/************************************************************************
 **
 **  @file   vcommonsettings.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 7, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#ifndef VCOMMONSETTINGS_H
#define VCOMMONSETTINGS_H

#include <QByteArray>
#include <QMetaObject>
#include <QObject>
#include <QSettings>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QtGlobal>

#include "../vlayout/vlayoutdef.h"
#include "../vmisc/def.h"

class QMarginsF;

enum class VThemeMode
{
    System = 0,
    Dark = 1,
    Light = 2
};

class VCommonSettings : public QSettings
{
    Q_OBJECT // NOLINT

public:
    VCommonSettings(Format format, Scope scope, const QString &organization, const QString &application = QString(),
                    QObject *parent = nullptr);
    VCommonSettings(const QString &fileName, Format format, QObject *parent = nullptr);
    ~VCommonSettings() override = default;

    static auto CastToLayoutExportFormat(qint8 f) -> qint8;

    auto GetPathIndividualMeasurements() const -> QString;
    void SetPathIndividualMeasurements(const QString &value);

    auto GetPathMultisizeMeasurements() const -> QString;
    void SetPathMultisizeMeasurements(const QString &value);

    auto GetPathPattern() const -> QString;
    void SetPathPattern(const QString &value);

    auto GetPathManualLayouts() const -> QString;
    void SetPathManualLayouts(const QString &value);

    static auto GetDefPathSVGFonts() -> QString;
    auto GetPathSVGFonts() const -> QString;
    void SetPathSVGFonts(const QString &value);

    static auto GetDefPathFontCorrections() -> QString;
    auto GetPathFontCorrections() const -> QString;
    void SetPathFontCorrections(const QString &value);

    auto GetOsSeparator() const -> bool;
    void SetOsSeparator(const bool &value);

    auto GetAutosaveState() const -> bool;
    void SetAutosaveState(const bool &value);

    auto GetAutosaveTime() const -> int;
    void SetAutosaveTime(const int &value);

    auto IsLocaleSelected() const -> bool;
    auto GetLocale() const -> QString;
    void SetLocale(const QString &value);

    static const QString defaultPieceLabelLocale;
    auto GetPieceLabelLocale() const -> QString;
    void SetPieceLabelLocale(const QString &value);

    auto GetPMSystemCode() const -> QString;
    void SetPMSystemCode(const QString &value);

    auto GetUnit() const -> QString;
    void SetUnit(const QString &value);

    auto GetConfirmItemDelete() const -> bool;
    void SetConfirmItemDelete(const bool &value);

    auto GetConfirmFormatRewriting() const -> bool;
    void SetConfirmFormatRewriting(const bool &value);

    auto GetAskContinueIfLayoutStale() const -> bool;
    void SetAskContinueIfLayoutStale(const bool &value);

    auto GetToolBarStyle() const -> bool;
    void SetToolBarStyle(const bool &value);

    auto GetToolboxIconSizeSmall() const -> bool;
    void SetToolboxIconSizeSmall(bool value);

    auto GetThemeMode() const -> VThemeMode;
    void SetThemeMode(VThemeMode mode);

    auto IsFreeCurveMode() const -> bool;
    void SetFreeCurveMode(bool value);

    auto IsDoubleClickZoomFitBestCurrentPP() const -> bool;
    void SetDoubleClickZoomFitBestCurrentPP(bool value);

    auto IsInteractiveTools() const -> bool;
    void SetInteractiveTools(bool value);

    auto GetUndoCount() const -> int;
    void SetUndoCount(const int &value);

    auto GetRecentFileList() const -> QStringList;
    void SetRecentFileList(const QStringList &value);

    auto GetRestoreFileList() const -> QStringList;
    void SetRestoreFileList(const QStringList &value);

    auto GetGeometry() const -> QByteArray;
    void SetGeometry(const QByteArray &value);

    auto GetToolbarsState() const -> QByteArray;
    void SetToolbarsState(const QByteArray &value);

    auto GetPreferenceDialogSize() const -> QSize;
    void SetPreferenceDialogSize(const QSize &sz);

    auto GetToolSeamAllowanceDialogSize() const -> QSize;
    void SetToolSeamAllowanceDialogSize(const QSize &sz);

    auto GetFormulaWizardDialogSize() const -> QSize;
    void SetFormulaWizardDialogSize(const QSize &sz);

    auto GetIncrementsDialogSize() const -> QSize;
    void SetIncrementsDialogSize(const QSize &sz);

    auto GetFinalMeasurementsDialogSize() const -> QSize;
    void SetFinalMeasurementsDialogSize(const QSize &sz);

    auto GetLayoutSettingsDialogSize() const -> QSize;
    void SetLayoutSettingsDialogSize(const QSize &sz);

    auto GetDialogSplinePathSize() const -> QSize;
    void SetDialogSplinePathSize(const QSize &sz);

    auto IsAutomaticallyCheckUpdates() const -> bool;
    void SetAutomaticallyCheckUpdates(bool value);

    auto GetLatestSkippedVersion() const -> unsigned;
    void SetLatestSkippedVersion(unsigned value);

    auto GetDateOfLastRemind() const -> QDate;
    void SetDateOfLastRemind(const QDate &date);

    auto GetForbidWorkpieceFlipping() const -> bool;
    void SetForbidWorkpieceFlipping(bool value);

    auto GetForceWorkpieceFlipping() const -> bool;
    void SetForceWorkpieceFlipping(bool value);

    auto GetSewLineOnDrawing() const -> bool;
    void SetSewLineOnDrawing(bool value);

    auto IsHideMainPath() const -> bool;
    void SetHideMainPath(bool value);

    auto IsDoublePassmark() const -> bool;
    void SetDoublePassmark(bool value);

    void SetCSVWithHeader(bool withHeader);
    auto GetCSVWithHeader() const -> bool;
    static auto GetDefCSVWithHeader() -> bool;

    void SetCSVCodec(int mib);
    auto GetCSVCodec() const -> int;
    static auto GetDefCSVCodec() -> int;

    void SetCSVSeparator(const QChar &separator);
    auto GetCSVSeparator() const -> QChar;
    static auto GetDefCSVSeparator() -> QChar;

    void SetDefaultSeamAllowance(double value);
    auto GetDefaultSeamAllowance() -> double;

    auto GetLabelFont() const -> QFont;
    void SetLabelFont(const QFont &f);

    auto GetLabelSVGFont() const -> QString;
    void SetLabelSVGFont(const QString &family);

    auto GetSingleStrokeOutlineFont() const -> bool;
    void SetSingleStrokeOutlineFont(bool value);

    auto GetSingleLineFonts() const -> bool;
    void SetSingleLineFonts(bool value);

    auto GetPieceLabelFontPointSize() const -> int;
    void SetPieceLabelFontPointSize(int size);
    static auto MinPieceLabelFontPointSize() -> int;

    auto GetPatternLabelFontSize() const -> int;
    void SetPatternLabelFontSize(int size);
    static auto GetDefPatternLabelFontSize() -> int;

    auto GetHideLabels() const -> bool;
    void SetHideLabels(bool value);

    auto GetShowAccuracyRadius() const -> bool;
    void SetShowAccuracyRadius(bool value);

#if !defined(Q_OS_WIN)
    static const QString unixStandardSharePath;
#endif

    auto GetLabelDateFormat() const -> QString;
    void SetLabelDateFormat(const QString &format);

    static auto PredefinedDateFormats() -> QStringList;
    auto GetUserDefinedDateFormats() const -> QStringList;
    void SetUserDefinedDateFormats(const QStringList &formats);

    auto GetLabelTimeFormat() const -> QString;
    void SetLabelTimeFormat(const QString &format);

    static auto PredefinedTimeFormats() -> QStringList;
    auto GetUserDefinedTimeFormats() const -> QStringList;
    void SetUserDefinedTimeFormats(const QStringList &formats);

    auto GetCurveApproximationScale() const -> qreal;
    void SetCurveApproximationScale(qreal value);

    auto IsShowCurveDetails() const -> bool;
    void SetShowCurveDetails(bool value);

    auto IsPieceShowMainPath() const -> bool;
    void SetPieceShowMainPath(bool value);

    auto IsDontUseNativeDialog() const -> bool;
    void SetDontUseNativeDialog(bool value);

    static auto DefaultLineWidth() -> qreal;
    static auto MinimalLineWidth() -> qreal;
    static auto MaximalLineWidth() -> qreal;
    auto GetLineWidth() const -> qreal;
    void SetLineWidth(qreal width);
    auto WidthMainLine() const -> qreal;
    auto WidthHairLine() const -> qreal;

    // settings for the tiled PDFs
    auto GetTiledPDFMargins(const Unit &unit) const -> QMarginsF;
    void SetTiledPDFMargins(const QMarginsF &value, const Unit &unit);

    auto GetTiledPDFOrientation() const -> PageOrientation;
    void SetTiledPDFOrientation(PageOrientation value);

    static const int defaultScrollingDuration;
    static const int scrollingDurationMin;
    static const int scrollingDurationMax;
    auto GetScrollingDuration() const -> int;
    void SetScrollingDuration(int duration);

    static const int defaultScrollingUpdateInterval;
    static const int scrollingUpdateIntervalMin;
    static const int scrollingUpdateIntervalMax;
    auto GetScrollingUpdateInterval() const -> int;
    void SetScrollingUpdateInterval(int updateInterval);

    static const qreal defaultSensorMouseScale;
    static const qreal sensorMouseScaleMin;
    static const qreal sensorMouseScaleMax;
    auto GetSensorMouseScale() const -> qreal;
    void SetSensorMouseScale(qreal scale);

    static const qreal defaultWheelMouseScale;
    static const qreal wheelMouseScaleMin;
    static const qreal wheelMouseScaleMax;
    auto GetWheelMouseScale() const -> qreal;
    void SetWheelMouseScale(qreal scale);

    static const qreal defaultScrollingAcceleration;
    static const qreal scrollingAccelerationMin;
    static const qreal scrollingAccelerationMax;
    auto GetScrollingAcceleration() const -> qreal;
    void SetScrollingAcceleration(qreal acceleration);

    auto IsOpenGLRender() const -> bool;
    void SetOpenGLRender(bool value);

    auto GetGraphicalOutput() const -> bool;
    void SetGraphicalOutput(const bool &value);

    auto GetWatermarkEditorSize() const -> QSize;
    void SetWatermarkEditorSize(const QSize &sz);

    auto GetWatermarkCustomColors() const -> QVector<QColor>;
    void SetWatermarkCustomColors(QVector<QColor> colors);

    auto IsAskCollectStatistic() const -> bool;
    void SetAskCollectStatistic(bool value);

    auto IsCollectStatistic() const -> bool;
    void SetCollectStatistic(bool value);

    auto GetClientID() const -> QString;
    void SetClientID(const QString &clientID);

    auto IsTranslateFormula() const -> bool;
    void SetTranslateFormula(bool value);

signals:
    void SVGFontsPathChanged(const QString &oldPath, const QString &newPath);

protected:
    template <typename T>
    static auto GetCachedValue(const QSettings &settings, T &cache, const QString &setting, T defValue, T valueMin,
                               T valueMax) -> T;

    template <class T>
    static auto ValueOrDef(const QSettings &settings, const QString &setting, const T &defValue) -> T;

private:
    Q_DISABLE_COPY_MOVE(VCommonSettings) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VCommonSettings::DefaultLineWidth() -> qreal
{
    return 1.2; // mm
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VCommonSettings::MinimalLineWidth() -> qreal
{
    return 0.5; // mm
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VCommonSettings::MaximalLineWidth() -> qreal
{
    return 5.0; // mm
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
auto VCommonSettings::GetCachedValue(const QSettings &settings, T &cache, const QString &setting, T defValue,
                                     T valueMin, T valueMax) -> T
{
    if (cache < 0)
    {
        cache = qBound(valueMin, ValueOrDef(settings, setting, defValue), valueMax);
    }

    return cache;
}

//---------------------------------------------------------------------------------------------------------------------
template <class T>
inline auto VCommonSettings::ValueOrDef(const QSettings &settings, const QString &setting, const T &defValue) -> T
{
    const QVariant val = settings.value(setting, QVariant::fromValue(defValue));
    return val.canConvert<T>() ? val.value<T>() : defValue;
}

//---------------------------------------------------------------------------------------------------------------------
template <>
inline auto VCommonSettings::ValueOrDef<Cases>(const QSettings &settings, const QString &setting, const Cases &defValue)
    -> Cases
{
    const QVariant val = settings.value(setting, QVariant::fromValue(static_cast<int>(defValue)));
    const int g = val.canConvert<int>() ? val.toInt() : static_cast<int>(defValue);
    if (g < static_cast<int>(Cases::CaseThreeGroup) || g >= static_cast<int>(Cases::UnknownCase))
    {
        return defValue;
    }
    return static_cast<Cases>(g);
}

#endif // VCOMMONSETTINGS_H
