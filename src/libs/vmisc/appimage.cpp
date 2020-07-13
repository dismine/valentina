/************************************************************************
 **
 **  @file   appimage.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 11, 2019
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2019 Valentina project
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
#include "appimage.h"

#include <stdlib.h>
#include <unicode/putil.h>
#include <QString>
#include <QCoreApplication>
#include <cstring>
#include <QVector>
extern "C" {
#include "binreloc.h"
}
#include "../vmisc/def.h"
#include "compatibility.h"

//---------------------------------------------------------------------------------------------------------------------
/* When deploying with AppImage based on OpenSuse, the ICU library has a hardcoded path to the icudt*.dat file.
 * This prevents the library from using shared in memory data. There are few ways to resolve this issue. According
 * to documentation we can either use ICU_DATA environment variable or the function u_setDataDirectory().
 */
char* IcuDataPath(const char* correction)
{
    char * data_path = nullptr;
    BrInitError error;
    if (br_init (&error))
    {
        char *path = br_find_exe_dir(nullptr);
        if (path)
        {
            data_path = static_cast<char *> (malloc(strlen(path) + strlen(correction) + 1));
            if(data_path)
            {
                strcpy(data_path, path);
                strcat(data_path, correction);
                u_setDataDirectory(data_path);
            }
            free(path);
        }
    }

    return data_path;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AppImageRoot returns path to AppImage root folder. Because after mount path to /usr folder looks like
 * /tmp/.mount_valentXwmbwE/usr, the root part must be extracted to fix all paths when application is in AppImage
 * format.
 *
 * Function we return empty if expected path is not compatible with current application directory path.
 * @return path to AppImage root folder
 */
QString AppImageRoot()
{
    return AppImageRoot(QCoreApplication::applicationDirPath(), QString(BINDIR));
}

//---------------------------------------------------------------------------------------------------------------------
QString AppImageRoot(const QString &applicationDir, const QString &defaultAppDir)
{
    QStringList appSub = SplitFilePaths(applicationDir);
    QStringList defaultSub = SplitFilePaths(defaultAppDir);

    if (appSub.isEmpty() || defaultSub.isEmpty() || appSub.size() <= defaultSub.size())
    {
        return QString();
    }

    appSub = Reverse(appSub);
    defaultSub = Reverse(defaultSub);

    for (int i = 0; i < defaultSub.size(); ++i)
    {
        if (defaultSub.at(i) != appSub.at(i))
        {
            return QString();
        }
    }

    QStringList rootSub = appSub.mid(defaultSub.size());
    rootSub = Reverse(rootSub);

    return '/' + rootSub.join('/');
}
