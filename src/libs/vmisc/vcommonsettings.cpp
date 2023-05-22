/************************************************************************
 **
 **  @file   vcommonsettings.cpp
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

#include "vcommonsettings.h"

#include <QApplication>
#include <QColor>
#include <QDate>
#include <QDir>
#include <QFont>
#include <QGlobalStatic>
#include <QLocale>
#include <QMarginsF>
#include <QMessageLogger>
#include <QString>
#include <QVariant>
#include <QtDebug>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include "vtextcodec.h"
#else
#include <QTextCodec>
#endif

#include "compatibility.h"
#include "def.h"
#include "defglobal.h"
#include "literals.h"

const int VCommonSettings::defaultScrollingDuration = 300;
const int VCommonSettings::scrollingDurationMin = 100;
const int VCommonSettings::scrollingDurationMax = 1000;

const int VCommonSettings::defaultScrollingUpdateInterval = 30;
const int VCommonSettings::scrollingUpdateIntervalMin = 10;
const int VCommonSettings::scrollingUpdateIntervalMax = 100;

const qreal VCommonSettings::defaultSensorMouseScale = 2.0;
const qreal VCommonSettings::sensorMouseScaleMin = 1.0;
const qreal VCommonSettings::sensorMouseScaleMax = 10.0;

const qreal VCommonSettings::defaultWheelMouseScale = 45.0;
const qreal VCommonSettings::wheelMouseScaleMin = 1.0;
const qreal VCommonSettings::wheelMouseScaleMax = 100.0;

const qreal VCommonSettings::defaultScrollingAcceleration = 1.3;
const qreal VCommonSettings::scrollingAccelerationMin = 1.0;
const qreal VCommonSettings::scrollingAccelerationMax = 10.0;

const QString VCommonSettings::defaultPieceLabelLocale = QStringLiteral("default");

#ifndef QPRINTENGINE_H
Q_DECLARE_METATYPE(QMarginsF) // NOLINT
#endif

namespace
{
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPathsIndividualMeasurements,
                          (QLatin1String("paths/individual_measurements"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPathsMultisizeMeasurements,
                          (QLatin1String("paths/standard_measurements")))                                   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPathsPattern, (QLatin1String("paths/pattern")))             // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPathsTemplates, (QLatin1String("paths/templates")))         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPathsLabelTemplate, (QLatin1String("paths/labels")))        // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPathsManualLayouts, (QLatin1String("paths/manualLayouts"))) // NOLINT

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingConfigurationOsSeparator,
                          (QLatin1String("configuration/osSeparator"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingConfigurationAutosaveState,
                          (QLatin1String("configuration/autosave/state"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingConfigurationAutosaveTime,
                          (QLatin1String("configuration/autosave/time")))                                     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingConfigurationLocale, (QLatin1String("configuration/locale"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingConfigurationPieceLabelLocale,                                // NOLINT
                          (QLatin1String("configuration/pieceLabelLocale")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPMSystemCode, (QLatin1String("configuration/pmscode")))   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingConfigurationUnit, (QLatin1String("configuration/unit"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingConfigurationConfirmItemDeletion,
                          (QLatin1String("configuration/confirm_item_deletion"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingConfigurationConfirmFormatRewriting,
                          (QLatin1String("configuration/confirm_format_rewriting"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingConfigurationAskContinueIfLayoutStale,
                          (QLatin1String("configuration/askContinueIfLayoutStale"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingConfigurationToolBarStyle,
                          (QLatin1String("configuration/tool_bar_style"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingConfigurationFreeCurveMode,
                          (QLatin1String("configuration/freeCurveMode"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingConfigurationDoubleClickZoomFitBestCurrentPP,
                          (QLatin1String("configuration/doubleClickZoomFitBestCurrentPP"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingConfigurationInteractiveTools,
                          (QLatin1String("configuration/interactiveTools"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingConfigurationDontUseNativeDialog,
                          (QLatin1String("configuration/dontUseNativeDialog"))) // NOLINT

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternUndo, (QLatin1String("pattern/undo"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternForbidFlipping,
                          (QLatin1String("pattern/forbidFlipping"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternForceFlipping,
                          (QLatin1String("pattern/forceFlipping"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternSewLineOnDrawing,
                          (QLatin1String("pattern/sewLineOnDrawing")))                                        // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternHideMainPath, (QLatin1String("pattern/hideMainPath"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingDoublePassmark, (QLatin1String("pattern/doublePassmark")))    // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternDefaultSeamAllowance,
                          (QLatin1String("pattern/defaultSeamAllowance")))                              // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternLabelFont, (QLatin1String("pattern/labelFont"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPieceLabelFontPointSize,                                // NOLINT
                          (QLatin1String("pattern/pieceLabelFontPointSize")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternLineWidth, (QLatin1String("pattern/lineWidth"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternCurveApproximationScale,
                          (QLatin1String("pattern/curveApproximationScale"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternShowCurveDetails,
                          (QLatin1String("pattern/showCurveDetails"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternPieceShowMainPath,
                          (QLatin1String("pattern/pieceShowMainPath"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternLabelFontSize,
                          (QLatin1String("pattern/labelFontSize")))                                       // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternHideLabels, (QLatin1String("pattern/hideLabels"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternShowAccuracyRadius,
                          (QLatin1String("pattern/showAccuracyRadius"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternUseOpenGLRender,
                          (QLatin1String("pattern/useOpenGLRender"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPatternGraphicalOutput,
                          (QLatin1String("pattern/graphicalOutput"))) // NOLINT

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingGeneralRecentFileList, (QLatin1String("recentFileList")))   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingGeneralRestoreFileList, (QLatin1String("restoreFileList"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingGeneralGeometry, (QLatin1String("geometry")))               // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingGeneralToolbarsState, (QLatin1String("toolbarsState")))     // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingConfigurationDarkMode,
                          (QLatin1String("configuration/dark_mode")))                                          // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingPreferenceDialogSize, (QLatin1String("preferenceDialogSize"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingToolSeamAllowanceDialogSize,
                          (QLatin1String("toolSeamAllowanceDialogSize"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingIncrementsDialogSize,
                          (QLatin1String("toolIncrementsDialogSize"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingFormulaWizardDialogSize,
                          (QLatin1String("formulaWizardDialogSize"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingFinalMeasurementsDialogSize,
                          (QLatin1String("finalMeasurementsDialogSize"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLayoutSettingsDialogSize,
                          (QLatin1String("layoutSettingsDialogSize")))                                         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingDialogSplinePathSize, (QLatin1String("splinePathDialogSize"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingAutomaticallyCheckUpdates,
                          (QLatin1String("automaticallyCheckUpdates"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLatestSkippedVersion,
                          (QLatin1String("lastestSkippedVersion")))                                    // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingDateOfLastRemind, (QLatin1String("dateOfLastRemind"))) // NOLINT

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingCSVWithHeader, (QLatin1String("csv/withHeader")))   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingCSVCodec, (QLatin1String("csv/withCodec")))         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingCSVSeparator, (QLatin1String("csv/withSeparator"))) // NOLINT

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLabelDateFormat, (QLatin1String("label/dateFormat"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLabelUserDateFormats,
                          (QLatin1String("label/userDateFormats")))                                   // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLabelTimeFormat, (QLatin1String("label/timeFormat"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingLabelUserTimeFormats,
                          (QLatin1String("label/userTimeFormats"))) // NOLINT

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingScrollingDuration, (QLatin1String("scrolling/duration"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingScrollingUpdateInterval,
                          (QLatin1String("scrolling/updateInterval"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingScrollingSensorMouseScale,
                          (QLatin1String("scrolling/sensorMouseScale"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingScrollingWheelMouseScale,
                          (QLatin1String("scrolling/wheelMouseScale"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingScrollingAcceleration,
                          (QLatin1String("scrolling/acceleration"))) // NOLINT

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingTiledPDFMargins, (QLatin1String("tiledPDF/margins")))         // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingTiledPDFOrientation, (QLatin1String("tiledPDF/orientation"))) // NOLINT

Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingWatermarkEditorSize, (QLatin1String("watermarkEditorSize"))) // NOLINT
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingWatermarkCustomColors,
                          (QLatin1String("watermarkCustomColors"))) // NOLINT

// Reading settings file is very expensive, cache curve approximation to speed up getting value
qreal curveApproximationCached = -1;        // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
Q_GLOBAL_STATIC(QString, localeCached)      // NOLINT
qreal lineWidthCached = 0;                  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
int labelFontSizeCached = 0;                // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
int pieceShowMainPath = -1;                 // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
int scrollingDurationCached = -1;           // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
int scrollingUpdateIntervalCached = -1;     // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
qreal scrollingSensorMouseScaleCached = -1; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
qreal scrollingWheelMouseScaleCached = -1;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
qreal scrollingAccelerationCached = -1;     // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

//---------------------------------------------------------------------------------------------------------------------
auto ClearFormats(const QStringList &predefinedFormats, QStringList formats) -> QStringList
{
    for (const auto &f : predefinedFormats)
    {
        formats.removeAll(f);
    }
    return formats;
}
} // namespace

Q_GLOBAL_STATIC_WITH_ARGS(const QString, commonIniFilename, (QLatin1String("common"))) // NOLINT

#if !defined(Q_OS_WIN)
const QString VCommonSettings::unixStandardSharePath = QStringLiteral(PKGDATADIR); // NOLINT(cert-err58-cpp)
#endif

namespace
{
//---------------------------------------------------------------------------------------------------------------------
void SymlinkCopyDirRecursive(const QString &fromDir, const QString &toDir, bool replaceOnConflit)
{
    QDir dir;
    dir.setPath(fromDir);

    const QStringList list = dir.entryList(QDir::Files);
    for (const QString &copyFile : list)
    {
        const QString from = fromDir + QDir::separator() + copyFile;
        QString to = toDir + QDir::separator() + copyFile;

#ifdef Q_OS_WIN
        {
            // To fix issue #702 check each not symlink if it is actually broken symlink.
            // Also trying to mimic Unix symlink. If a file eaxists do not create a symlink and remove it if exists.
            QFile fileTo(to);
            if (fileTo.exists())
            {
                if (not fileTo.rename(to + QLatin1String(".lnk")))
                {
                    QFile::remove(to + QLatin1String(".lnk"));
                    fileTo.rename(to + QLatin1String(".lnk"));
                }

                QFileInfo info(to + QLatin1String(".lnk"));
                if (info.symLinkTarget().isEmpty())
                {
                    fileTo.copy(to);
                    fileTo.remove();
                    continue; // The file already exists, skip creating shortcut
                }
            }
        }

        to = to + QLatin1String(".lnk");
#endif

        QFileInfo fileTo(to);
        if (not fileTo.isSymLink() && fileTo.exists())
        {
            if (replaceOnConflit)
            {
                QFile::remove(to);
            }
            else
            {
                continue;
            }
        }
        else if (fileTo.isSymLink())
        {
            if (not fileTo.exists() || replaceOnConflit)
            { // automatically fix broken symlink
                QFile::remove(to);
            }
            else
            {
                continue;
            }
        }

        QFile::link(from, to);
    }

    const QStringList dirList = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &copyDir : dirList)
    {
        const QString from = fromDir + QDir::separator() + copyDir;
        const QString to = toDir + QDir::separator() + copyDir;

        if (not dir.mkpath(to))
        {
            return;
        }

        SymlinkCopyDirRecursive(from, to, replaceOnConflit);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto PrepareStandardFiles(const QString &currentPath, const QString &standardPath, const QString &defPath) -> QString
{
    QDir standardPathDir(standardPath);
    QDir currentPathDir(currentPath);
    if ((currentPath == defPath || not currentPathDir.exists()) && standardPathDir.exists())
    {
        const QDir localdata(defPath);
        if (localdata.mkpath(QChar('.')))
        {
            SymlinkCopyDirRecursive(standardPath, defPath, false);
        }
        return defPath;
    }
    return currentPath;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VCommonSettings::VCommonSettings(Format format, Scope scope, const QString &organization, const QString &application,
                                 QObject *parent)
  : QSettings(format, scope, organization, application, parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
VCommonSettings::VCommonSettings(const QString &fileName, QSettings::Format format, QObject *parent)
  : QSettings(fileName, format, parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::SharePath(const QString &shareItem) -> QString
{
#ifdef Q_OS_WIN
    return QCoreApplication::applicationDirPath() + shareItem;
#elif defined(Q_OS_MAC)
    QDir dirBundle(QCoreApplication::applicationDirPath() + QStringLiteral("/../Resources") + shareItem);
    if (dirBundle.exists())
    {
        return dirBundle.absolutePath();
    }
    else
    {
        QDir appDir = QDir(qApp->applicationDirPath());
        appDir.cdUp();
        appDir.cdUp();
        appDir.cdUp();
        QDir dir(appDir.absolutePath() + shareItem);
        if (dir.exists())
        {
            return dir.absolutePath();
        }
        else
        {
            return VCommonSettings::unixStandardSharePath + shareItem;
        }
    }
#else // Unix
#ifdef QT_DEBUG
    return QCoreApplication::applicationDirPath() + shareItem;
#else
    QDir dir(QCoreApplication::applicationDirPath() + shareItem);
    if (dir.exists())
    {
        return dir.absolutePath();
    }
    else
    {
        return VCommonSettings::unixStandardSharePath + shareItem;
    }
#endif
#endif
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::MultisizeTablesPath() -> QString
{
    return SharePath(QStringLiteral("/tables/multisize"));
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::StandardTemplatesPath() -> QString
{
    return SharePath(QStringLiteral("/tables/templates"));
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::LabelTemplatesPath() -> QString
{
    return SharePath(QStringLiteral("/labels"));
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::PrepareStandardTemplates(const QString &currentPath) -> QString
{
    return PrepareStandardFiles(currentPath, StandardTemplatesPath(), GetDefPathTemplate());
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::PrepareMultisizeTables(const QString &currentPath) -> QString
{
    return PrepareStandardFiles(currentPath, MultisizeTablesPath(), GetDefPathMultisizeMeasurements());
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::PrepareLabelTemplates(const QString &currentPath) -> QString
{
    return PrepareStandardFiles(currentPath, LabelTemplatesPath(), GetDefPathLabelTemplate());
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::CastToLayoutExportFormat(qint8 f) -> qint8
{
    qint8 format = f;
    if (f == 15 || (f > 16 && f < 23))
    {
        format = 16;
    }
    else if (f == 24 || (f > 26 && f < 33))
    {
        format = 26;
    }

    return format;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetDefPathIndividualMeasurements() -> QString
{
    return QDir::homePath() + QStringLiteral("/valentina/") + tr("measurements") + QStringLiteral("/") +
           tr("individual");
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetPathIndividualMeasurements() const -> QString
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    return settings.value(*settingPathsIndividualMeasurements, GetDefPathIndividualMeasurements()).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetPathIndividualMeasurements(const QString &value)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    settings.setValue(*settingPathsIndividualMeasurements, value);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetDefPathMultisizeMeasurements() -> QString
{
    return QDir::homePath() + QStringLiteral("/valentina/") + tr("measurements") + QChar('/') + tr("multisize");
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetPathMultisizeMeasurements() const -> QString
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    return settings.value(*settingPathsMultisizeMeasurements, GetDefPathMultisizeMeasurements()).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetPathMultisizeMeasurements(const QString &value)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    settings.setValue(*settingPathsMultisizeMeasurements, value);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetDefPathPattern() -> QString
{
    return QDir::homePath() + QStringLiteral("/valentina/") + tr("patterns");
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetPathPattern() const -> QString
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    return settings.value(*settingPathsPattern, GetDefPathPattern()).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetPathPattern(const QString &value)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    settings.setValue(*settingPathsPattern, value);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetDefPathTemplate() -> QString
{
    return QDir::homePath() + QStringLiteral("/valentina/") + tr("templates");
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetPathTemplate() const -> QString
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    return settings.value(*settingPathsTemplates, GetDefPathTemplate()).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetPathTemplate(const QString &value)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    settings.setValue(*settingPathsTemplates, value);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetDefPathLabelTemplate() -> QString
{
    return QDir::homePath() + QStringLiteral("/valentina/") + tr("label templates");
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetPathLabelTemplate() const -> QString
{
    return value(*settingPathsLabelTemplate, GetDefPathLabelTemplate()).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetPathLabelTemplate(const QString &value)
{
    setValue(*settingPathsLabelTemplate, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetDefPathManualLayouts() -> QString
{
    return QDir::homePath() + QStringLiteral("/valentina/") + tr("manual layouts");
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetPathManualLayouts() const -> QString
{
    return value(*settingPathsManualLayouts, GetDefPathManualLayouts()).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetPathManualLayouts(const QString &value)
{
    setValue(*settingPathsManualLayouts, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetOsSeparator() const -> bool
{
    return value(*settingConfigurationOsSeparator, 1).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetOsSeparator(const bool &value)
{
    setValue(*settingConfigurationOsSeparator, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetAutosaveState() const -> bool
{
    return value(*settingConfigurationAutosaveState, 1).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetAutosaveState(const bool &value)
{
    setValue(*settingConfigurationAutosaveState, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetAutosaveTime() const -> int
{
    bool ok = false;
    int val = value(*settingConfigurationAutosaveTime, 1).toInt(&ok);
    if (not ok)
    {
        qDebug() << "Could not convert value" << value(*settingConfigurationAutosaveTime, 1)
                 << "to int. Return default value for autosave time" << 1 << "minutes.";
        val = 1;
    }
    return val;
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetAutosaveTime(const int &value)
{
    setValue(*settingConfigurationAutosaveTime, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::IsLocaleSelected() const -> bool
{
    const QString fakeLocale = QStringLiteral("Fake");
    QString locale = value(*settingConfigurationLocale, fakeLocale).toString();

    return locale != fakeLocale;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetLocale() const -> QString
{
    if (localeCached->isEmpty())
    {
        *localeCached = value(*settingConfigurationLocale, QLocale().name()).toString();
    }
    return *localeCached;
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetLocale(const QString &value)
{
    setValue(*settingConfigurationLocale, value);
    *localeCached = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetPieceLabelLocale() const -> QString
{
    return value(*settingConfigurationPieceLabelLocale, VCommonSettings::defaultPieceLabelLocale).toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetPieceLabelLocale(const QString &value)
{
    setValue(*settingConfigurationPieceLabelLocale, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetPMSystemCode() const -> QString
{
    return value(*settingPMSystemCode, "p998").toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetPMSystemCode(const QString &value)
{
    setValue(*settingPMSystemCode, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetUnit() const -> QString
{
    return value(*settingConfigurationUnit, QLocale().measurementSystem() == QLocale::MetricSystem ? unitCM : unitINCH)
        .toString();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetUnit(const QString &value)
{
    setValue(*settingConfigurationUnit, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetConfirmItemDelete() const -> bool
{
    return value(*settingConfigurationConfirmItemDeletion, 1).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetConfirmItemDelete(const bool &value)
{
    setValue(*settingConfigurationConfirmItemDeletion, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetConfirmFormatRewriting() const -> bool
{
    return value(*settingConfigurationConfirmFormatRewriting, 1).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetConfirmFormatRewriting(const bool &value)
{
    setValue(*settingConfigurationConfirmFormatRewriting, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetAskContinueIfLayoutStale() const -> bool
{
    return value(*settingConfigurationAskContinueIfLayoutStale, 1).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetAskContinueIfLayoutStale(const bool &value)
{
    setValue(*settingConfigurationAskContinueIfLayoutStale, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetToolBarStyle() const -> bool
{
    return value(*settingConfigurationToolBarStyle, 1).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetToolBarStyle(const bool &value)
{
    setValue(*settingConfigurationToolBarStyle, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetDarkMode() const -> bool
{
    return value(*settingConfigurationDarkMode, 0).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetDarkMode(const bool &value)
{
    setValue(*settingConfigurationDarkMode, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::IsFreeCurveMode() const -> bool
{
    return value(*settingConfigurationFreeCurveMode, 1).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetFreeCurveMode(bool value)
{
    setValue(*settingConfigurationFreeCurveMode, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::IsDoubleClickZoomFitBestCurrentPP() const -> bool
{
    return value(*settingConfigurationDoubleClickZoomFitBestCurrentPP, 1).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetDoubleClickZoomFitBestCurrentPP(bool value)
{
    setValue(*settingConfigurationDoubleClickZoomFitBestCurrentPP, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::IsInteractiveTools() const -> bool
{
    return value(*settingConfigurationInteractiveTools, true).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetInteractiveTools(bool value)
{
    setValue(*settingConfigurationInteractiveTools, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetUndoCount() const -> int
{
    bool ok = false;
    int val = value(*settingPatternUndo, 0).toInt(&ok);
    if (not ok)
    {
        qDebug() << "Could not convert value" << value(*settingPatternUndo, 0)
                 << "to int. Return default value for undo counts 0 (no limit).";
        val = 0;
    }
    return val;
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetUndoCount(const int &value)
{
    setValue(*settingPatternUndo, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetRecentFileList() const -> QStringList
{
    const QStringList files = value(*settingGeneralRecentFileList).toStringList();
    QStringList cleared;

    for (const auto &f : files)
    {
        if (QFileInfo::exists(f))
        {
            cleared.append(f);
        }
    }

    return cleared;
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetRecentFileList(const QStringList &value)
{
    setValue(*settingGeneralRecentFileList, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetRestoreFileList() const -> QStringList
{
    return value(*settingGeneralRestoreFileList).toStringList();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetRestoreFileList(const QStringList &value)
{
    setValue(*settingGeneralRestoreFileList, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetGeometry() const -> QByteArray
{
    return value(*settingGeneralGeometry).toByteArray();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetGeometry(const QByteArray &value)
{
    setValue(*settingGeneralGeometry, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetToolbarsState() const -> QByteArray
{
    return value(*settingGeneralToolbarsState).toByteArray();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetToolbarsState(const QByteArray &value)
{
    setValue(*settingGeneralToolbarsState, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetPreferenceDialogSize() const -> QSize
{
    return value(*settingPreferenceDialogSize, QSize(0, 0)).toSize();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetPreferenceDialogSize(const QSize &sz)
{
    setValue(*settingPreferenceDialogSize, sz);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetToolSeamAllowanceDialogSize() const -> QSize
{
    return value(*settingToolSeamAllowanceDialogSize, QSize(0, 0)).toSize();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetToolSeamAllowanceDialogSize(const QSize &sz)
{
    setValue(*settingToolSeamAllowanceDialogSize, sz);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetFormulaWizardDialogSize() const -> QSize
{
    return value(*settingFormulaWizardDialogSize, QSize(0, 0)).toSize();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetFormulaWizardDialogSize(const QSize &sz)
{
    setValue(*settingFormulaWizardDialogSize, sz);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetIncrementsDialogSize() const -> QSize
{
    return value(*settingIncrementsDialogSize, QSize(0, 0)).toSize();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetIncrementsDialogSize(const QSize &sz)
{
    setValue(*settingIncrementsDialogSize, sz);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetFinalMeasurementsDialogSize() const -> QSize
{
    return value(*settingFinalMeasurementsDialogSize, QSize(0, 0)).toSize();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetFinalMeasurementsDialogSize(const QSize &sz)
{
    setValue(*settingFinalMeasurementsDialogSize, sz);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetLayoutSettingsDialogSize() const -> QSize
{
    return value(*settingLayoutSettingsDialogSize, QSize(0, 0)).toSize();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetLayoutSettingsDialogSize(const QSize &sz)
{
    setValue(*settingLayoutSettingsDialogSize, sz);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetDialogSplinePathSize() const -> QSize
{
    return value(*settingDialogSplinePathSize, QSize(0, 0)).toSize();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetDialogSplinePathSize(const QSize &sz)
{
    setValue(*settingDialogSplinePathSize, sz);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::IsAutomaticallyCheckUpdates() const -> bool
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    return settings.value(*settingAutomaticallyCheckUpdates, true).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetAutomaticallyCheckUpdates(bool value)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    settings.setValue(*settingAutomaticallyCheckUpdates, value);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetLatestSkippedVersion() const -> unsigned
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    return settings.value(*settingLatestSkippedVersion, 0x0).toUInt();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetLatestSkippedVersion(unsigned value)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    settings.setValue(*settingLatestSkippedVersion, value);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetDateOfLastRemind() const -> QDate
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    return settings.value(*settingDateOfLastRemind, QDate(1900, 1, 1)).toDate();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetDateOfLastRemind(const QDate &date)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    settings.setValue(*settingDateOfLastRemind, date);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetForbidWorkpieceFlipping() const -> bool
{
    return value(*settingPatternForbidFlipping, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetForbidWorkpieceFlipping(bool value)
{
    setValue(*settingPatternForbidFlipping, value);

    if (value)
    {
        SetForceWorkpieceFlipping(not value);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetForceWorkpieceFlipping() const -> bool
{
    return value(*settingPatternForceFlipping, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetForceWorkpieceFlipping(bool value)
{
    setValue(*settingPatternForceFlipping, value);

    if (value)
    {
        SetForbidWorkpieceFlipping(not value);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetSewLineOnDrawing() const -> bool
{
    return value(*settingPatternSewLineOnDrawing, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetSewLineOnDrawing(bool value)
{
    setValue(*settingPatternSewLineOnDrawing, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::IsHideMainPath() const -> bool
{
    return value(*settingPatternHideMainPath, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetHideMainPath(bool value)
{
    setValue(*settingPatternHideMainPath, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::IsDoublePassmark() const -> bool
{
    return value(*settingDoublePassmark, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetDoublePassmark(bool value)
{
    setValue(*settingDoublePassmark, value);
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetCSVWithHeader(bool withHeader)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    settings.setValue(*settingCSVWithHeader, withHeader);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetCSVWithHeader() const -> bool
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    return settings.value(*settingCSVWithHeader, GetDefCSVWithHeader()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetDefCSVWithHeader() -> bool
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetCSVCodec(int mib)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    settings.setValue(*settingCSVCodec, mib);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetCSVCodec() const -> int
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    return settings.value(*settingCSVCodec, GetDefCSVCodec()).toInt();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetDefCSVCodec() -> int
{
    return VTextCodec::codecForLocale()->mibEnum();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetCSVSeparator(const QChar &separator)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    switch (separator.toLatin1())
    {
        case '\t':
            settings.setValue(*settingCSVSeparator, 0);
            break;
        case ';':
            settings.setValue(*settingCSVSeparator, 1);
            break;
        case ' ':
            settings.setValue(*settingCSVSeparator, 2);
            break;
        default:
            settings.setValue(*settingCSVSeparator, 3);
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetCSVSeparator() const -> QChar
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    const quint8 separator = static_cast<quint8>(settings.value(*settingCSVSeparator, 3).toUInt());
    switch (separator)
    {
        case 0:
            return QChar('\t');
        case 1:
            return QChar(';');
        case 2:
            return QChar(' ');
        default:
            return QChar(',');
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetDefCSVSeparator() -> QChar
{
    return QChar(',');
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetDefaultSeamAllowance(double value)
{
    setValue(*settingPatternDefaultSeamAllowance, UnitConvertor(value, StrToUnits(GetUnit()), Unit::Cm));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief returns the default seam allowance. The corresponding unit is the default unit.
 * @return the default seam allowance
 */
auto VCommonSettings::GetDefaultSeamAllowance() -> double
{
    double defaultValue;

    const Unit globalUnit = StrToUnits(GetUnit());

    switch (globalUnit)
    {
        case Unit::Mm:
            defaultValue = 10;
            break;
        case Unit::Inch:
            defaultValue = 0.25;
            break;
        case Unit::Cm:
        default:
            defaultValue = 1;
            break;
    }

    bool ok = false;
    double val = value(*settingPatternDefaultSeamAllowance, -1).toDouble(&ok);
    if (not ok)
    {
        qDebug() << "Could not convert value" << value(*settingPatternDefaultSeamAllowance, 0)
                 << "to real. Return default value for default seam allowance is " << defaultValue << QChar('.');
        val = defaultValue;
    }

    if (val < 0)
    {
        val = defaultValue;
    }
    else
    {
        val = UnitConvertor(val, Unit::Cm, globalUnit);
    }

    return val;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetLabelFont() const -> QFont
{
    return qvariant_cast<QFont>(value(*settingPatternLabelFont, QApplication::font()));
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetLabelFont(const QFont &f)
{
    setValue(*settingPatternLabelFont, f);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetPieceLabelFontPointSize() const -> int
{
    bool ok = false;
    int val = value(*settingPieceLabelFontPointSize, VCommonSettings::MinPieceLabelFontPointSize()).toInt(&ok);
    if (not ok)
    {
        qDebug() << "Could not convert value"
                 << value(*settingPieceLabelFontPointSize, VCommonSettings::MinPieceLabelFontPointSize())
                 << "to int. Return default value for label font size.";
        val = 12;
    }
    else
    {
        if (val < VCommonSettings::MinPieceLabelFontPointSize())
        {
            return VCommonSettings::MinPieceLabelFontPointSize();
        }
    }
    return val;
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetPieceLabelFontPointSize(int size)
{
    setValue(*settingPieceLabelFontPointSize, size);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::MinPieceLabelFontPointSize() -> int
{
    return 5;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetPatternLabelFontSize() const -> int
{
    if (labelFontSizeCached <= 0)
    {
        bool ok = false;
        labelFontSizeCached = value(*settingPatternLabelFontSize, GetDefPatternLabelFontSize()).toInt(&ok);
        if (not ok)
        {
            labelFontSizeCached = GetDefPatternLabelFontSize();
        }
        labelFontSizeCached = qBound(minLabelFontSize, labelFontSizeCached, maxLabelFontSize);
    }
    return labelFontSizeCached;
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetPatternLabelFontSize(int size)
{
    size = qBound(minLabelFontSize, size, maxLabelFontSize);
    setValue(*settingPatternLabelFontSize, size);
    labelFontSizeCached = size;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetDefPatternLabelFontSize() -> int
{
    return 32;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetHideLabels() const -> bool
{
    return value(*settingPatternHideLabels, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetHideLabels(bool value)
{
    setValue(*settingPatternHideLabels, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetShowAccuracyRadius() const -> bool
{
    return value(*settingPatternShowAccuracyRadius, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetShowAccuracyRadius(bool value)
{
    setValue(*settingPatternShowAccuracyRadius, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetLabelDateFormat() const -> QString
{
    const QString format =
        value(*settingLabelDateFormat, ConstFirst<QString>(VCommonSettings::PredefinedDateFormats())).toString();
    const QStringList allFormats = VCommonSettings::PredefinedDateFormats() + GetUserDefinedDateFormats();

    return allFormats.contains(format) ? format : ConstFirst<QString>(VCommonSettings::PredefinedDateFormats());
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetLabelDateFormat(const QString &format)
{
    setValue(*settingLabelDateFormat, format);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::PredefinedDateFormats() -> QStringList
{
    return QStringList{"MM-dd-yyyy",
                       "d/M/yy",
                       "ddddMMMM dd, yyyy",
                       "dd/MM/yy",
                       "dd/MM/yyyy",
                       "MMM d, yy",
                       "MMM d, yyyy",
                       "d. MMM. yyyy",
                       "MMMM d, yyyy",
                       "d. MMMM yyyy",
                       "ddd, MMM d, yy",
                       "ddd dd/MMM yy",
                       "ddd, MMMM d, yyyy",
                       "ddddMMMM d, yyyy",
                       "MM-dd",
                       "yy-MM-dd",
                       "yyyy-MM-dd",
                       "MM/yy",
                       "MMM dd",
                       "MMMM"};
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetUserDefinedDateFormats() const -> QStringList
{
    return value(*settingLabelUserDateFormats, QStringList()).toStringList();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetUserDefinedDateFormats(const QStringList &formats)
{
    setValue(*settingLabelUserDateFormats, ClearFormats(VCommonSettings::PredefinedDateFormats(), formats));
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetLabelTimeFormat() const -> QString
{
    const QString format =
        value(*settingLabelTimeFormat, ConstFirst<QString>(VCommonSettings::PredefinedTimeFormats())).toString();
    const QStringList allFormats = VCommonSettings::PredefinedTimeFormats() + GetUserDefinedTimeFormats();

    return allFormats.contains(format) ? format : ConstFirst<QString>(VCommonSettings::PredefinedTimeFormats());
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetLabelTimeFormat(const QString &format)
{
    setValue(*settingLabelTimeFormat, format);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::PredefinedTimeFormats() -> QStringList
{
    return QStringList{"hh:mm:ss", "hh:mm:ss AP", "hh:mm", "hh:mm AP"};
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetUserDefinedTimeFormats() const -> QStringList
{
    return value(*settingLabelUserTimeFormats, QStringList()).toStringList();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetUserDefinedTimeFormats(const QStringList &formats)
{
    setValue(*settingLabelUserTimeFormats, ClearFormats(VCommonSettings::PredefinedTimeFormats(), formats));
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetCurveApproximationScale() const -> qreal
{
    if (curveApproximationCached < 0)
    {
        bool ok = false;
        const qreal scale = value(*settingPatternCurveApproximationScale, defCurveApproximationScale).toDouble(&ok);
        if (ok && scale >= minCurveApproximationScale && scale <= maxCurveApproximationScale)
        {
            curveApproximationCached = scale;
        }
        else
        {
            curveApproximationCached = defCurveApproximationScale;
        }
    }

    return curveApproximationCached;
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetCurveApproximationScale(qreal value)
{
    if (value >= minCurveApproximationScale && value <= maxCurveApproximationScale)
    {
        setValue(*settingPatternCurveApproximationScale, value);
        curveApproximationCached = value;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::IsShowCurveDetails() const -> bool
{
    return value(*settingPatternShowCurveDetails, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetShowCurveDetails(bool value)
{
    setValue(*settingPatternShowCurveDetails, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::IsPieceShowMainPath() const -> bool
{
    if (pieceShowMainPath < 0)
    {
        pieceShowMainPath = value(*settingPatternPieceShowMainPath, 0).toInt();
    }
    return pieceShowMainPath;
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetPieceShowMainPath(bool value)
{
    pieceShowMainPath = value;
    setValue(*settingPatternPieceShowMainPath, pieceShowMainPath);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::IsDontUseNativeDialog() const -> bool
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    return settings.value(*settingConfigurationDontUseNativeDialog, false).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetDontUseNativeDialog(bool value)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    settings.setValue(*settingConfigurationDontUseNativeDialog, value);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetLineWidth() const -> qreal
{
    if (lineWidthCached <= 0)
    {
        lineWidthCached = qBound(VCommonSettings::MinimalLineWidth(), value(*settingPatternLineWidth, 1.2).toDouble(),
                                 VCommonSettings::MaximalLineWidth());
    }

    return lineWidthCached;
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetLineWidth(qreal width)
{
    lineWidthCached = qBound(VCommonSettings::MinimalLineWidth(), width, VCommonSettings::MaximalLineWidth());
    setValue(*settingPatternLineWidth, lineWidthCached);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::WidthMainLine() const -> qreal
{
    return GetLineWidth() / 25.4 * PrintDPI;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::WidthHairLine() const -> qreal
{
    return WidthMainLine() / 3.0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetScrollingDuration() const -> int
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    return GetCachedValue(settings, scrollingDurationCached, *settingScrollingDuration, defaultScrollingDuration,
                          scrollingDurationMin, scrollingDurationMax);
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetScrollingDuration(int duration)
{
    scrollingDurationCached = qBound(scrollingDurationMin, duration, scrollingDurationMax);
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    settings.setValue(*settingScrollingDuration, scrollingDurationCached);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetScrollingUpdateInterval() const -> int
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    return GetCachedValue(settings, scrollingUpdateIntervalCached, *settingScrollingUpdateInterval,
                          defaultScrollingUpdateInterval, scrollingUpdateIntervalMin, scrollingUpdateIntervalMax);
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetScrollingUpdateInterval(int updateInterval)
{
    scrollingUpdateIntervalCached = qBound(scrollingUpdateIntervalMin, updateInterval, scrollingUpdateIntervalMax);
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    settings.setValue(*settingScrollingUpdateInterval, scrollingUpdateIntervalCached);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetSensorMouseScale() const -> qreal
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    return GetCachedValue(settings, scrollingSensorMouseScaleCached, *settingScrollingSensorMouseScale,
                          defaultSensorMouseScale, sensorMouseScaleMin, sensorMouseScaleMax);
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetSensorMouseScale(qreal scale)
{
    scrollingSensorMouseScaleCached = qBound(sensorMouseScaleMin, scale, sensorMouseScaleMax);
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    settings.setValue(*settingScrollingSensorMouseScale, scrollingSensorMouseScaleCached);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetWheelMouseScale() const -> qreal
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    return GetCachedValue(settings, scrollingWheelMouseScaleCached, *settingScrollingWheelMouseScale,
                          defaultWheelMouseScale, wheelMouseScaleMin, wheelMouseScaleMax);
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetWheelMouseScale(qreal scale)
{
    scrollingWheelMouseScaleCached = qBound(wheelMouseScaleMin, scale, wheelMouseScaleMax);
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    settings.setValue(*settingScrollingWheelMouseScale, scrollingWheelMouseScaleCached);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetScrollingAcceleration() const -> qreal
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    return GetCachedValue(settings, scrollingAccelerationCached, *settingScrollingAcceleration,
                          defaultScrollingAcceleration, scrollingAccelerationMin, scrollingAccelerationMax);
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetScrollingAcceleration(qreal acceleration)
{
    scrollingAccelerationCached = qBound(scrollingAccelerationMin, acceleration, scrollingAccelerationMax);
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    settings.setValue(*settingScrollingAcceleration, scrollingAccelerationCached);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::IsOpenGLRender() const -> bool
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    return settings.value(*settingPatternUseOpenGLRender, 0).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetOpenGLRender(bool value)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    settings.setValue(*settingPatternUseOpenGLRender, value);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetGraphicalOutput() const -> bool
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    return settings.value(*settingPatternGraphicalOutput, 1).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetGraphicalOutput(const bool &value)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    settings.setValue(*settingPatternGraphicalOutput, value);
    settings.sync();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetTiledPDFMargins returns the tiled pdf margins in the given unit. When the setting is
 * called for the first time, the 4 default margins are 10mm.
 * @param unit the unit in which are the value. Necessary because we save the values
 * internaly as mm so there is conversion beeing made.
 * @return tiled pdf margins
 */
auto VCommonSettings::GetTiledPDFMargins(const Unit &unit) const -> QMarginsF
{
    // default value is 10mm. We save the margins in mm in the setting.
    return UnitConvertor(ValueOrDef<QMarginsF>(*this, *settingTiledPDFMargins, QMarginsF(10, 10, 10, 10)), Unit::Mm,
                         unit);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SetTiledPDFMargins sets the setting tiled pdf margins to the given value.
 * @param value the margins to save
 * @param unit the unit in which are the value. Necessary because we save the values
 * internaly as mm so there is conversion beeing made.
 */
void VCommonSettings::SetTiledPDFMargins(const QMarginsF &value, const Unit &unit)
{
    setValue(*settingTiledPDFMargins, QVariant::fromValue(UnitConvertor(value, unit, Unit::Mm)));
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetTiledPDFOrientation() const -> PageOrientation
{
    bool defaultValue = static_cast<bool>(PageOrientation::Portrait);
    bool result = value(*settingTiledPDFOrientation, defaultValue).toBool();
    return static_cast<PageOrientation>(result);
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetTiledPDFOrientation(PageOrientation value)
{
    setValue(*settingTiledPDFOrientation, static_cast<bool>(value));
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetWatermarkEditorSize() const -> QSize
{
    return value(*settingWatermarkEditorSize, QSize(0, 0)).toSize();
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetWatermarkEditorSize(const QSize &sz)
{
    setValue(*settingWatermarkEditorSize, sz);
}

//---------------------------------------------------------------------------------------------------------------------
auto VCommonSettings::GetWatermarkCustomColors() const -> QVector<QColor>
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);
    QStringList colors = settings.value(*settingPatternGraphicalOutput, 1).toStringList();

    QVector<QColor> customColors;
    customColors.reserve(colors.size());

    for (const auto &color : colors)
    {
        QColor c(color);
        if (c.isValid())
        {
            customColors.append(c);
        }
    }

    if (customColors.count() > 7)
    {
        customColors.remove(0, customColors.count() - 7);
    }

    return customColors;
}

//---------------------------------------------------------------------------------------------------------------------
void VCommonSettings::SetWatermarkCustomColors(QVector<QColor> colors)
{
    QSettings settings(this->format(), this->scope(), this->organizationName(), *commonIniFilename);

    if (colors.count() > 7)
    {
        colors.remove(0, colors.count() - 7);
    }

    QStringList customColors;
    customColors.reserve(colors.size());

    for (const auto &color : colors)
    {
        customColors.append(color.name());
    }

    settings.setValue(*settingWatermarkCustomColors, customColors);
    settings.sync();
}
