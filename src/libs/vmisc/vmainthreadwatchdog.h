/************************************************************************
 **
 **  @file   vmainthreadwatchdog.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 7, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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
#ifndef VMAINTHREADWATCHDOG_H
#define VMAINTHREADWATCHDOG_H

#include <QLoggingCategory>
#include <QString>

Q_DECLARE_LOGGING_CATEGORY(vWatchdog) // NOLINT

/**
 * @brief StartMainThreadWatchdog notice when the GUI thread stops pumping events.
 *
 * Users report the application freezing for minutes at a time on machines we cannot reproduce on, and nothing in the
 * application currently notices or records it. A background monitor watches a heartbeat published by the GUI thread;
 * when the heartbeat stops for longer than the threshold it writes the stall to the log and (where crash reporting is
 * built in and enabled) uploads a dump of the *stuck* thread, so the report shows where it is actually parked.
 *
 * Must be called from the main thread once QApplication exists. Calling it again is a no-op.
 */
void StartMainThreadWatchdog(const QString &logFilePath);

/**
 * @brief MainThreadWatchdogLogPath the file the watchdog writes stalls and stack samples to.
 *
 * Derived from the application log path by inserting "-watchdog" before the extension. The watchdog needs a file of its
 * own: it writes from the monitor thread while the GUI thread is wedged, and the GUI thread's logger holds its own open
 * handle with its own file position. Appending to that same file means the GUI thread overwrites the appended lines the
 * moment it resumes - which it did, destroying the samples from two ninety-second stalls before this was noticed.
 *
 * Returns an empty string for an empty input.
 */
auto MainThreadWatchdogLogPath(const QString &logFilePath) -> QString;

#endif // VMAINTHREADWATCHDOG_H
