/************************************************************************
 **
 **  @file   vcrashpaths.h
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
#ifndef VCRASHPATHS_H
#define VCRASHPATHS_H

#include <QString>

#include <base/files/file_path.h>

class VCrashPaths
{
public:
    explicit VCrashPaths(QString exeDir);

    auto GetHandlerPath() const -> QString;

    static auto GetAttachmentPath(const QString &appName) -> QString;
    static auto GetReportsPath() -> QString;
    static auto GetMetricsPath() -> QString;

#if defined(Q_OS_UNIX) || defined(Q_OS_WINDOWS)
    static auto GetPlatformString(const QString &string) -> base::FilePath::StringType;
#elif
#error GetPlatformString not implemented on this platform
#endif

private:
    QString m_exeDir;
};

#endif // VCRASHPATHS_H
