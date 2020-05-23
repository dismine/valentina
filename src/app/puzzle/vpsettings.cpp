/************************************************************************
 **
 **  @file   vpsettings.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 2, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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
#include "vpsettings.h"

namespace
{
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingDockWidgetPropertiesActive, (QLatin1String("dockWidget/properties")))
Q_GLOBAL_STATIC_WITH_ARGS(const QString, settingDockPropertiesContentsActive, (QLatin1String("dockWidget/contents")))
}

//---------------------------------------------------------------------------------------------------------------------
VPSettings::VPSettings(Format format, Scope scope, const QString &organization, const QString &application,
                                 QObject *parent)
    : VCommonSettings(format, scope, organization, application, parent)
{}

//---------------------------------------------------------------------------------------------------------------------
VPSettings::VPSettings(const QString &fileName, QSettings::Format format, QObject *parent)
    : VCommonSettings(fileName, format, parent)
{}

//---------------------------------------------------------------------------------------------------------------------
bool VPSettings::IsDockWidgetPropertiesActive() const
{
    return value(*settingDockWidgetPropertiesActive, GetDefDockWidgetPropertiesActive()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VPSettings::GetDefDockWidgetPropertiesActive()
{
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSettings::SetDockWidgetPropertiesActive(bool value)
{
    setValue(*settingDockWidgetPropertiesActive, value);
}

//---------------------------------------------------------------------------------------------------------------------
bool VPSettings::IsDockWidgetPropertiesContentsActive() const
{
    return value(*settingDockWidgetPropertiesActive, GetDefDockWidgetPropertiesActive()).toBool();
}

//---------------------------------------------------------------------------------------------------------------------
bool VPSettings::GetDefDockWidgetPropertiesContentsActive()
{
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSettings::SetDockWidgetPropertiesContentsActive(bool value)
{
    setValue(*settingDockPropertiesContentsActive, value);
}
