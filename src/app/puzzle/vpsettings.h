/************************************************************************
 **
 **  @file   vpsettings.h
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
#ifndef VPSETTINGS_H
#define VPSETTINGS_H

#include <QByteArray>

#include "vcommonsettings.h"

class VPSettings : public VCommonSettings
{
    Q_OBJECT
public:
    VPSettings(Format format, Scope scope, const QString &organization, const QString &application = QString(),
                    QObject *parent = nullptr);
    VPSettings(const QString &fileName, Format format, QObject *parent = nullptr);

    bool IsDockWidgetPropertiesActive() const;
    static bool GetDefDockWidgetPropertiesActive();
    void SetDockWidgetPropertiesActive(bool value);

    bool IsDockWidgetPropertiesContentsActive() const;
    static bool GetDefDockWidgetPropertiesContentsActive();
    void SetDockWidgetPropertiesContentsActive(bool value);

private:
    Q_DISABLE_COPY(VPSettings)
};

#endif // VPSETTINGS_H
