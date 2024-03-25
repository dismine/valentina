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

//---------------------------------------------------------------------------------------------------------------------
VCrashPaths::VCrashPaths(QString exeDir)
  : m_exeDir(std::move(exeDir))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VCrashPaths::GetAttachmentPath(const QString &appName) -> QString
{
    const QString logDirPath =
        QStandardPaths::locate(QStandardPaths::ConfigLocation, QString(), QStandardPaths::LocateDirectory) +
        QStringLiteral(VER_COMPANYNAME_STR);
    return QStringLiteral("%1/%2-pid%3.log").arg(logDirPath, appName.toLower()).arg(QCoreApplication::applicationPid());
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
    return QStandardPaths::locate(QStandardPaths::AppConfigLocation, QString(), QStandardPaths::LocateDirectory) +
           QStringList{VER_COMPANYNAME_STR, "User Data", "Crashpad", "Reports"}.join(QDir::separator());
}

//---------------------------------------------------------------------------------------------------------------------
auto VCrashPaths::GetMetricsPath() -> QString
{
    return QStandardPaths::locate(QStandardPaths::AppConfigLocation, QString(), QStandardPaths::LocateDirectory) +
           QStringList{VER_COMPANYNAME_STR, "User Data", "Crashpad", "Metrics"}.join(QDir::separator());
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
