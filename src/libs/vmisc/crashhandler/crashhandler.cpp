/************************************************************************
 **
 **  @file   crashhandler.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   4 3, 2024
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2024 Valentina project
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
#include "crashhandler.h"

#if defined(Q_OS_WIN)
#define NOMINMAX
#include <windows.h>
#endif

#if defined(Q_OS_MAC)
#include <mach-o/dyld.h>
#endif

#if defined(Q_OS_LINUX)
#include <QDir>
#include <QMap>
#include <unistd.h>
#define MIN(x, y) (((x) < (y)) ? (x) : (y)) // NOLINT
#endif

#include <string>
#include <vector>

#include <client/crash_report_database.h>
#include <client/crashpad_client.h>
#include <client/settings.h>

#include "vcrashpaths.h"

#include "../projectversion.h"
#include "../vcommonsettings.h"

#if defined(APPIMAGE) && defined(Q_OS_LINUX)
#include "../appimage.h"
#endif

#include <vcsRepoState.h>

using namespace crashpad;

namespace
{
//---------------------------------------------------------------------------------------------------------------------
Q_REQUIRED_RESULT auto AppSettings(const QString &appName) -> VCommonSettings *
{
    return new VCommonSettings(QSettings::IniFormat, QSettings::UserScope, QStringLiteral(VER_COMPANYNAME_STR),
                               appName);
}

//---------------------------------------------------------------------------------------------------------------------
auto AppCrashVersion() -> QString
{
    auto const version = QStringLiteral("%1_%2_%3").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(DEBUG_VERSION);
    auto const qtVersion = QStringLiteral("Qt_%1_%2").arg(QT_VERSION_MAJOR).arg(QT_VERSION_MINOR);
    QString multibundle; // NOLINT(misc-const-correctness)

#if defined(Q_OS_MACOS)
    auto const platform = QStringLiteral("macos");
#if defined(MULTI_BUNDLE)
    multibundle = QStringLiteral("-multibundle");
#endif
#elif defined(Q_OS_WIN)
    auto const platform = QStringLiteral("windows");
#elif defined(Q_OS_LINUX)
    auto const platform = QStringLiteral("linux");
#else
    auto const platform = QStringLiteral("unknown");
#endif

    auto revision = QStringLiteral(VCS_REPO_STATE_REVISION);
    if (!revision.startsWith('g'))
    { // assume always git
        revision.prepend('g');
    }

    return QStringLiteral("%1-%2-%3-%4%5").arg(version, revision, qtVersion, platform, multibundle);
}

//---------------------------------------------------------------------------------------------------------------------
auto GetExecutableDir() -> QString
{
#if defined(Q_OS_MAC)
    unsigned int bufferSize = 512;
    std::vector<char> buffer(bufferSize + 1);

    if (_NSGetExecutablePath(&buffer[0], &bufferSize))
    {
        buffer.resize(bufferSize);
        _NSGetExecutablePath(&buffer[0], &bufferSize);
    }

    char *lastForwardSlash = strrchr(&buffer[0], '/');
    if (lastForwardSlash == nullptr)
    {
        return nullptr;
    }
    *lastForwardSlash = 0;

    return &buffer[0];
#elif defined(Q_OS_WINDOWS)
    HMODULE hModule = GetModuleHandleW(NULL);
    WCHAR path[MAX_PATH];
    DWORD retVal = GetModuleFileNameW(hModule, path, MAX_PATH);
    if (retVal == 0)
    {
        return nullptr;
    }

    wchar_t *lastBackslash = wcsrchr(path, '\\');
    if (lastBackslash == nullptr)
    {
        return nullptr;
    }
    *lastBackslash = 0;

    return QString::fromWCharArray(path);
#elif defined(Q_OS_LINUX)
    char pBuf[FILENAME_MAX];
    int len = sizeof(pBuf);
    int bytes = MIN(static_cast<int>(readlink("/proc/self/exe", pBuf, static_cast<size_t>(len))), len - 1);
    if (bytes >= 0)
    {
        pBuf[bytes] = '\0';
    }

    char *lastForwardSlash = strrchr(&pBuf[0], '/');
    if (lastForwardSlash == nullptr)
    {
        return nullptr;
    }
    *lastForwardSlash = '\0';

    return QString::fromStdString(pBuf);
#else
#error GetExecutableDir not implemented on this platform
#endif
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
auto InitializeCrashpad(const QString &appName) -> bool
{
    QScopedPointer<VCommonSettings> const appSettings(AppSettings(appName.toLower()));

    if (!appSettings->IsSendCrashReport())
    {
        return true;
    }

// Get directory where the exe lives so we can pass a full path to handler, reportsDir and metricsDir
#if defined(APPIMAGE) && defined(Q_OS_LINUX)
    QString const exeDir = AppImageRoot(GetExecutableDir(), QString(BINDIR)) + QString(BINDIR);
#else
    QString const exeDir = GetExecutableDir();
#endif

    // Helper class for cross-platform file systems
    VCrashPaths const crashpadPaths(exeDir);

    auto MakeDir = [](const QString &path)
    {
        if (QDir const directory(path); not directory.exists())
        {
            directory.mkpath(QChar('.'));
        }
    };

    // Directory where reports will be saved. Important! Must be writable or crashpad_handler will crash.
    QString const reportsPath = VCrashPaths::GetReportsPath();
    MakeDir(reportsPath);
    base::FilePath const reportsDir(VCrashPaths::GetPlatformString(reportsPath));

    // Initialize crashpad database
    std::unique_ptr<CrashReportDatabase> database = CrashReportDatabase::Initialize(reportsDir);
    if (database == nullptr)
    {
        return false;
    }

    // Enable automated crash uploads
    Settings *settings = database->GetSettings();
    if (settings == nullptr)
    {
        return false;
    }
    settings->SetUploadsEnabled(true);

    // Attachments to be uploaded alongside the crash - default bundle size limit is 20MB
    std::vector<base::FilePath> attachments;
    base::FilePath const attachment(VCrashPaths::GetPlatformString(VCrashPaths::GetAttachmentPath(appName)));
    attachments.push_back(attachment);

    // Ensure that crashpad_handler is shipped with your application
    base::FilePath const handler(VCrashPaths::GetPlatformString(crashpadPaths.GetHandlerPath()));

    // Directory where metrics will be saved. Important! Must be writable or crashpad_handler will crash.
    QString const metricsPath = VCrashPaths::GetMetricsPath();
    MakeDir(metricsPath);
    base::FilePath const metricsDir(VCrashPaths::GetPlatformString(metricsPath));

    auto const dbName = QStringLiteral("valentina");
    // Configure url with your BugSplat database
    auto const url = QStringLiteral("https://%1.bugsplat.com/post/bp/crash/crashpad.php").arg(dbName);

    // Metadata that will be posted to BugSplat
    QMap<std::string, std::string> annotations;
    annotations["format"] = "minidump";                       // Required: Crashpad setting to save crash as a
    annotations["database"] = dbName.toStdString();           // Required: BugSplat database
    annotations["product"] = appName.toLower().toStdString(); // Required: BugSplat appName
    annotations["version"] = AppCrashVersion().toStdString(); // Required: BugSplat appVersion

    QString clientID = appSettings->GetClientID();
    if (clientID.isEmpty())
    {
        clientID = QUuid::createUuid().toString();
        appSettings->SetClientID(clientID);
    }
    annotations["key"] = clientID.toStdString(); // Optional: BugSplat key field

    QString const userEmail = appSettings->GetCrashEmail();
    if (!userEmail.isEmpty())
    {
        annotations["user"] = userEmail.toStdString(); // Optional: BugSplat user email
    }

    // Disable crashpad rate limiting so that all crashes have dmp files
    std::vector<std::string> arguments;
    arguments.emplace_back("--no-rate-limit");

    // Start crash handler
    CrashpadClient client;
    return client.StartHandler(handler, reportsDir, metricsDir, url.toStdString(), "", annotations.toStdMap(),
                               arguments, true, true, attachments);
}
