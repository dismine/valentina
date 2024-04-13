/************************************************************************
 **
 **  @file   vcrashpaths.cpp
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
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <utility>

#include "../projectversion.h"
#include "vcrashpaths.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
auto LogDirPath(const QString &appName) -> QString
{
    const QString logs = QStringLiteral("Logs");

    QString logDirPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    if (logDirPath.isEmpty())
    {
#if defined(Q_OS_WINDOWS)
        return QStringList{QCoreApplication::applicationDirPath(), logs, appName}.join(QDir::separator());
#else
        return QStringList{QDir::homePath(), VER_COMPANYNAME_STR, logs, appName}.join(QDir::separator());
#endif
    }
#if defined(Q_OS_WINDOWS)
    QString path = QStringList{logDirPath, logs}.join(QDir::separator());
#else
    QString path = QStringList{logDirPath, VER_COMPANYNAME_STR, logs, appName}.join(QDir::separator());
#endif
    return path;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VCrashPaths::VCrashPaths(QString exeDir)
  : m_exeDir(std::move(exeDir))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VCrashPaths::GetAttachmentPath(const QString &appName) -> QString
{
    return QStringLiteral("%1/%2-pid%3.log")
        .arg(LogDirPath(appName), appName.toLower())
        .arg(QCoreApplication::applicationPid());
}

//---------------------------------------------------------------------------------------------------------------------
auto VCrashPaths::GetHandlerPath() const -> QString
{
#if defined(Q_OS_WINDOWS)
    const QString handler = QStringLiteral("crashpad_handler.exe");
#elif defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    const QString handler = QStringLiteral("crashpad_handler");
#else
#error GetHandlerPath not implemented on this platform
#endif
    return m_exeDir + QDir::separator() + handler;
}

//---------------------------------------------------------------------------------------------------------------------
auto VCrashPaths::GetReportsPath() -> QString
{
#if defined(Q_OS_WINDOWS)
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator() +
           QStringList{"User Data", "Crashpad", "Reports"}.join(QDir::separator());
#else
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator() +
           QStringList{VER_COMPANYNAME_STR, "User Data", "Crashpad", "Reports"}.join(QDir::separator());
#endif
}

//---------------------------------------------------------------------------------------------------------------------
auto VCrashPaths::GetMetricsPath() -> QString
{
#if defined(Q_OS_WINDOWS)
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator() +
           QStringList{"User Data", "Crashpad", "Metrics"}.join(QDir::separator());
#else
    return QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator() +
           QStringList{VER_COMPANYNAME_STR, "User Data", "Crashpad", "Metrics"}.join(QDir::separator());
#endif
}

//---------------------------------------------------------------------------------------------------------------------

auto VCrashPaths::GetPlatformString(const QString &string) -> base::FilePath::StringType
{
#if defined(Q_OS_UNIX)
    return string.toStdString();
#elif defined(Q_OS_WINDOWS)
    return string.toStdWString();
#else
#error GetPlatformString not implemented on this platform
#endif
}
