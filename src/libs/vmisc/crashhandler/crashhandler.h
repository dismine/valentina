/************************************************************************
 **
 **  @file   crashhandler.h
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
#ifndef CRASHHANDLER_H
#define CRASHHANDLER_H

#include <QString>

/**
 * @brief InitializeCrashpad start the crash reporter.
 *
 * @param productName the product reports are filed under in BugSplat. This is "Valentina" for all three applications
 * and changing it would send their reports somewhere else.
 * @param appName this application - "Valentina", "Tape" or "Puzzle". Picks the settings file to read the opt-in from
 * and the log file to attach, both of which are per application. Must match QCoreApplication::applicationName(), which
 * is not set yet when this runs.
 */
auto InitializeCrashpad(const QString &productName, const QString &appName) -> bool;

#endif // CRASHHANDLER_H
