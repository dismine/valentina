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

#include <QCoreApplication>
#include <QString>
#include <QVector>
#include <unicode/putil.h>

#include "compatibility.h"
#include "def.h"

using namespace Qt::Literals::StringLiterals;

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
    qDebug() << "Application directory:" << applicationDir;
    qDebug() << "Default app directory:" << defaultAppDir;

    QStringList appSub = SplitFilePaths(applicationDir);
    QStringList defaultSub = SplitFilePaths(defaultAppDir);

    if (appSub.isEmpty() || defaultSub.isEmpty() || appSub.size() <= defaultSub.size())
    {
        return {};
    }

    appSub = Reverse(appSub);
    defaultSub = Reverse(defaultSub);

    for (int i = 0; i < defaultSub.size(); ++i)
    {
        if (defaultSub.at(i) != appSub.at(i))
        {
            return {};
        }
    }

    QStringList rootSub = appSub.mid(defaultSub.size());
    rootSub = Reverse(rootSub);

    return '/'_L1 + rootSub.join('/');
}
