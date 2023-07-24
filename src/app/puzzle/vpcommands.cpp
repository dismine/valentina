/************************************************************************
 **
 **  @file   vpcommands.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 4, 2020
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
#include "vpcommands.h"

#include <QStringList>

const QString LONG_OPTION_RAW_LAYOUT = QStringLiteral("rawLayout"); // NOLINT
const QString SINGLE_OPTION_RAW_LAYOUT = QStringLiteral("r");       // NOLINT
const QString LONG_OPTION_STYLE = QStringLiteral("style");          // NOLINT

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AllKeys return list with all command line keys (short and long forms). Used for testing on conflicts.
 * @return list with all command line keys
 */
auto AllKeys() -> QStringList
{
    return {LONG_OPTION_RAW_LAYOUT, SINGLE_OPTION_RAW_LAYOUT, LONG_OPTION_STYLE};
}
