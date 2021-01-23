/************************************************************************
 **
 **  @file   vabstractvalapplication.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 10, 2020
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
#include "vabstractvalapplication.h"
#include "customevents.h"
#include "vsettings.h"

#include <QWidget>

//---------------------------------------------------------------------------------------------------------------------
VAbstractValApplication::VAbstractValApplication(int &argc, char **argv)
    : VAbstractApplication(argc, argv)
{}

//---------------------------------------------------------------------------------------------------------------------
double VAbstractValApplication::toPixel(double val) const
{
    return ToPixel(val, m_patternUnits);
}

//---------------------------------------------------------------------------------------------------------------------
double VAbstractValApplication::fromPixel(double pix) const
{
    return FromPixel(pix, m_patternUnits);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractValApplication::PostWarningMessage(const QString &message, QtMsgType severity) const
{
    QApplication::postEvent(mainWindow,
                            new WarningMessageEvent(VAbstractValApplication::ClearMessage(message), severity));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief OpenSettings get acsses to application settings.
 *
 * Because we can create object in constructor we open file separately.
 */
void VAbstractValApplication::OpenSettings()
{
    settings = new VSettings(QSettings::IniFormat, QSettings::UserScope, QCoreApplication::organizationName(),
                             QCoreApplication::applicationName(), this);
}

//---------------------------------------------------------------------------------------------------------------------
VSettings *VAbstractValApplication::ValentinaSettings()
{
    SCASSERT(settings != nullptr)
    return qobject_cast<VSettings *>(settings);
}
