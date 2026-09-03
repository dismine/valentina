/************************************************************************
 **
 **  @file   vasyncfileio.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 9, 2026
 **
 **  @brief  Runs blocking file operations without freezing the GUI.
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

#ifndef VASYNCFILEIO_H
#define VASYNCFILEIO_H

#include <functional>

class QString;

// Save and copy operations block on the OS, not on our code: QSaveFile::commit() calls FlushFileBuffers and
// QFile::remove() calls DeleteFileW, and a cloud-sync or antivirus filter driver can hold either for seconds.
// Run on the GUI thread that reads as a hard freeze, indistinguishable from a crash. These helpers move the
// blocking call to a worker and keep the window repainting instead.
namespace VAsyncFileIO
{
/**
 * @brief Runs a blocking file operation without freezing the GUI.
 *
 * In console mode, in tests, or when already off the GUI thread, @a op is simply called inline - there is no GUI
 * to keep responsive. Otherwise it runs on a worker thread while a local event loop keeps the window painting.
 *
 * The call is synchronous either way: it returns only once @a op has finished, so @a op may safely capture the
 * caller's locals by reference.
 *
 * @param description shown in the busy dialog if the operation outlives the delay. Pass an empty string for
 *                    background work the user did not ask for (autosave), which must never raise a dialog.
 * @param op          the blocking operation. Runs on another thread: it must touch no GUI object and no DOM.
 * @return whatever @a op returned.
 */
auto RunFileOperation(const QString &description, const std::function<bool()> &op) -> bool;

/**
 * @brief The worker half of RunFileOperation(), always taking the threaded path. Exposed for tests.
 */
auto RunFileOperationOnWorker(const QString &description, const std::function<bool()> &op) -> bool;

/**
 * @brief True while a file operation started by RunFileOperation() is still in flight.
 *
 * The local event loop still delivers timer events, so periodic work (autosave) must check this and skip its
 * turn instead of stacking a second file operation behind the stalled one.
 */
auto IsFileOperationRunning() -> bool;

/**
 * @brief Blocks until any in-flight file operation has finished.
 *
 * Call on shutdown: quitting while a QSaveFile commit is still running would tear the worker down mid-write.
 */
void WaitForFileOperations();

/**
 * @brief Called with @c true when a file operation has outlived the busy delay and is still running, and with
 * @c false once it finishes. @a description is the same text passed to RunFileOperation() (may be empty).
 */
using BusyIndicatorHandler = std::function<void(bool busy, const QString &description)>;

/**
 * @brief Registers @a handler to replace the default modal QProgressDialog.
 *
 * Valentina, Tape and Puzzle each have their own way of showing "working" (Valentina reuses its status-bar
 * progress bar), so the default modal dialog only exists as a fallback for apps that register nothing. Call
 * once, e.g. from the application's main window constructor, and pass nullptr from its destructor to avoid
 * outliving the window. Not thread-safe: call only from the GUI thread.
 */
void SetBusyIndicatorHandler(BusyIndicatorHandler handler);
} // namespace VAsyncFileIO

#endif // VASYNCFILEIO_H
