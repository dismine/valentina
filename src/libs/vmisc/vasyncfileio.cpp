/************************************************************************
 **
 **  @file   vasyncfileio.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 9, 2026
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

#include "vasyncfileio.h"

#include "vabstractapplication.h"

#include <QApplication>
#include <QEventLoop>
#include <QFutureWatcher>
#include <QGlobalStatic>
#include <QProgressDialog>
#include <QScopeGuard>
#include <QString>
#include <QThread>
#include <QThreadPool>
#include <QTimer>
#include <QtConcurrent>

#include <memory>
#include <utility>

namespace
{
//---------------------------------------------------------------------------------------------------------------------
// A dedicated pool, not the global one. The global pool already runs pattern parsing and layout nesting, and
// nesting submits nested QtConcurrent work into it - adding blocking file I/O there can starve that work on
// machines with few cores (issue #243). One thread also serializes file operations for free.
class VFileIOThreadPool : public QThreadPool
{
public:
    VFileIOThreadPool() { setMaxThreadCount(1); }
};

Q_GLOBAL_STATIC(VFileIOThreadPool, fileIOPool) // NOLINT

// Only ever touched on the GUI thread, inside RunFileOperationOnWorker(). A counter, not a bool: a nested event
// loop can pump another RunFileOperationOnWorker() call before the outer one finishes (e.g. the measurements sync
// timer firing mid-save opens a modal that triggers a second, nested file operation), so the flag must only clear
// once the outermost call completes.
int inFlightCount = 0; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

// Only ever touched on the GUI thread, via SetBusyIndicatorHandler() and RunFileOperationOnWorker().
VAsyncFileIO::BusyIndicatorHandler busyIndicatorHandler; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

// Below half a second a modal flash is more annoying than the freeze it replaces.
constexpr int busyDialogDelayMs = 500;
} // namespace

//---------------------------------------------------------------------------------------------------------------------
auto VAsyncFileIO::RunFileOperation(const QString &description, const std::function<bool()> &op) -> bool
{
    if (const VAbstractApplication *app = VAbstractApplication::VApp();
        app == nullptr || not app->IsAppInGUIMode() || QThread::currentThread() != app->thread())
    {
        return op(); // console mode, tests, or already off the GUI thread: no GUI to keep responsive
    }

    return RunFileOperationOnWorker(description, op);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAsyncFileIO::RunFileOperationOnWorker(const QString &description, const std::function<bool()> &op) -> bool
{
    QFutureWatcher<bool> watcher;
    QEventLoop loop;
    QObject::connect(&watcher, &QFutureWatcherBase::finished, &loop, &QEventLoop::quit);

    ++inFlightCount;
    auto resetFlag = qScopeGuard([]() { --inFlightCount; });

    watcher.setFuture(QtConcurrent::run(static_cast<QThreadPool *>(fileIOPool()), op));

    std::unique_ptr<QProgressDialog> dialog;
    bool busyShown = false;
    auto hideBusy = qScopeGuard(
        [&busyShown, &description]()
        {
            if (busyShown && busyIndicatorHandler)
            {
                busyIndicatorHandler(false, description);
            }
        });

    QTimer delay;
    delay.setSingleShot(true);
    QObject::connect(&delay,
                     &QTimer::timeout,
                     &delay,
                     [&dialog, &description, &busyShown]()
                     {
                         if (busyIndicatorHandler)
                         {
                             // The app registered its own way of showing "working" (e.g. Valentina reuses its
                             // status-bar progress bar) - the modal dialog only exists as a fallback.
                             busyShown = true;
                             busyIndicatorHandler(true, description);
                             return;
                         }

                         dialog = std::make_unique<QProgressDialog>(description,
                                                                    QString(),
                                                                    0,
                                                                    0, // 0..0 = indeterminate busy indicator
                                                                    QApplication::activeWindow());
                         dialog->setWindowModality(Qt::ApplicationModal);
                         dialog->setCancelButton(nullptr); // the stalled OS call cannot be interrupted
                         dialog->setMinimumDuration(0);
                         dialog->show();
                     });

    if (not description.isEmpty())
    {
        delay.start(busyDialogDelayMs);
    }

    // ExcludeUserInputEvents is what makes spinning a nested event loop safe here: paint events still run, so the
    // window stays alive, but the user cannot start a second save or edit the document being written. Timers do
    // still fire - see IsFileOperationRunning().
    if (not watcher.isFinished())
    {
        loop.exec(QEventLoop::ExcludeUserInputEvents);
    }

    return watcher.result();
}

//---------------------------------------------------------------------------------------------------------------------
auto VAsyncFileIO::IsFileOperationRunning() -> bool
{
    return inFlightCount > 0;
}

//---------------------------------------------------------------------------------------------------------------------
void VAsyncFileIO::WaitForFileOperations()
{
    if (not fileIOPool.isDestroyed())
    {
        fileIOPool()->waitForDone();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAsyncFileIO::SetBusyIndicatorHandler(BusyIndicatorHandler handler)
{
    busyIndicatorHandler = std::move(handler);
}
