/************************************************************************
 **
 **  @file   vtapeshortcutmanager.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 10, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#ifndef VTAPESHORTCUTMANAGER_H
#define VTAPESHORTCUTMANAGER_H

#include "../vmisc/vabstractshortcutmanager.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif

class VTapeShortcutManager : public VAbstractShortcutManager
{
    Q_OBJECT // NOLINT

public:
    explicit VTapeShortcutManager(QObject *parent = nullptr);
    ~VTapeShortcutManager() override = default;

private:
    Q_DISABLE_COPY_MOVE(VTapeShortcutManager) // NOLINT
};

#endif // VTAPESHORTCUTMANAGER_H
