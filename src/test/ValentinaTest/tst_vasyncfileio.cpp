/************************************************************************
 **
 **  @file   tst_vasyncfileio.cpp
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

#include "tst_vasyncfileio.h"

#include "../vmisc/vasyncfileio.h"

#include <QFile>
#include <QPair>
#include <QSaveFile>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>
#include <QTimer>
#include <QVector>

//---------------------------------------------------------------------------------------------------------------------
TST_VAsyncFileIO::TST_VAsyncFileIO(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
// TestVApplication::IsAppInGUIMode() returns false, so RunFileOperation() must degrade to a plain call: no thread,
// no event loop, no dialog. This is what keeps ValentinaTest and CollectionTest behaving exactly as before.
void TST_VAsyncFileIO::InlineWhenNotInGUIMode() const
{
    QThread *callerThread = QThread::currentThread();
    QThread *opThread = nullptr;

    const bool result = VAsyncFileIO::RunFileOperation(QStringLiteral("test"),
                                                       [&opThread]() -> bool
                                                       {
                                                           opThread = QThread::currentThread();
                                                           return true;
                                                       });

    QVERIFY2(result, "Inline operation did not propagate its result.");
    QCOMPARE(opThread, callerThread);
    QVERIFY2(not VAsyncFileIO::IsFileOperationRunning(), "Operation still marked as in flight.");
}

//---------------------------------------------------------------------------------------------------------------------
// The worker path is what the GUI actually uses. An empty description suppresses the busy dialog, so this stays
// headless-safe. The point of the whole feature is that the operation does not run on the calling thread.
void TST_VAsyncFileIO::WorkerRunsOffCallingThread() const
{
    const QThread *callerThread = QThread::currentThread();
    QThread *opThread = nullptr;

    const bool result = VAsyncFileIO::RunFileOperationOnWorker(QString(),
                                                               [&opThread]() -> bool
                                                               {
                                                                   opThread = QThread::currentThread();
                                                                   return true;
                                                               });

    QVERIFY2(result, "Worker operation did not propagate its result.");
    QVERIFY2(opThread != nullptr, "Operation never ran.");
    QVERIFY2(opThread != callerThread, "Operation ran on the calling thread.");
    QVERIFY2(not VAsyncFileIO::IsFileOperationRunning(), "Operation still marked as in flight.");
}

//---------------------------------------------------------------------------------------------------------------------
// A failed save must stay a failed save: false has to survive the trip through the worker, and anything the
// operation wrote into a captured error string must be visible once it returns.
void TST_VAsyncFileIO::WorkerPropagatesFailure() const
{
    QString error;

    const bool result = VAsyncFileIO::RunFileOperationOnWorker(QString(),
                                                               [&error]() noexcept -> bool
                                                               {
                                                                   error = QStringLiteral("disk on fire");
                                                                   return false;
                                                               });

    QVERIFY2(not result, "Failure was reported as success.");
    QCOMPARE(error, QStringLiteral("disk on fire"));
}

//---------------------------------------------------------------------------------------------------------------------
// QSaveFile only replaces the target on commit(). The other tests only prove the worker hop preserves a bool -
// this proves the real invariant the whole feature exists for: a failure that cancels the write (mirroring
// VDomDocument::SaveDocument's actual failure path) must leave the original file on disk completely untouched,
// even after a real hop through the worker thread.
void TST_VAsyncFileIO::WorkerFailureLeavesOriginalFileUnchanged() const
{
    QTemporaryDir tempDir;
    QVERIFY2(tempDir.isValid(), "Failed to create a temporary directory.");

    const QString filePath = tempDir.filePath(QStringLiteral("original.txt"));
    const auto goodContent = QByteArrayLiteral("good content");

    {
        QFile file(filePath);
        QVERIFY2(file.open(QIODevice::WriteOnly), "Failed to create the original file.");
        QCOMPARE(file.write(goodContent), goodContent.size());
    }

    const bool result = VAsyncFileIO::RunFileOperationOnWorker(
        QString(),
        [filePath]() -> bool
        {
            if (QSaveFile file(filePath); file.open(QIODevice::WriteOnly))
            {
                file.write(QByteArrayLiteral("corrupted"));
                file.cancelWriting(); // never commit(): mirrors VDomDocument::SaveDocument's failure path
            }
            return false;
        });

    QVERIFY2(not result, "Failed operation was reported as successful.");

    QFile file(filePath);
    QVERIFY2(file.open(QIODevice::ReadOnly), "Original file disappeared.");
    QCOMPARE(file.readAll(), goodContent);
}

//---------------------------------------------------------------------------------------------------------------------
// IsFileOperationRunning() is only ever useful when read from a timer callback firing mid-save (see
// MainWindow::AutoSavePattern). The other tests only check the flag before/after RunFileOperationOnWorker() -
// this proves it actually reads true while the nested event loop is spinning, not just that it resets afterwards.
void TST_VAsyncFileIO::WorkerReportsInFlightDuringOperation() const
{
    bool observedInFlight = false;
    QTimer::singleShot(0, [&observedInFlight]() { observedInFlight = VAsyncFileIO::IsFileOperationRunning(); });

    const bool result = VAsyncFileIO::RunFileOperationOnWorker(QString(),
                                                               []() -> bool
                                                               {
                                                                   QThread::msleep(50);
                                                                   return true;
                                                               });

    QVERIFY2(result, "Operation did not propagate its result.");
    QVERIFY2(observedInFlight, "IsFileOperationRunning() did not report true while the operation was in flight.");
}

//---------------------------------------------------------------------------------------------------------------------
// Apps that register their own "working" indicator (e.g. Valentina reuses its status-bar progress bar) must get
// it instead of the default modal dialog - this proves the handler actually fires, with the right busy/description
// pairing, in place of the dialog path.
void TST_VAsyncFileIO::WorkerUsesRegisteredBusyIndicatorHandler() const
{
    QVector<QPair<bool, QString>> calls;
    VAsyncFileIO::SetBusyIndicatorHandler([&calls](bool busy, const QString &description)
                                          { calls.append({busy, description}); });

    const bool result = VAsyncFileIO::RunFileOperationOnWorker(QStringLiteral("Doing work…"),
                                                               []() -> bool
                                                               {
                                                                   QThread::msleep(600); // outlive the busy delay
                                                                   return true;
                                                               });

    VAsyncFileIO::SetBusyIndicatorHandler(nullptr); // never leak a handler into the other tests

    QVERIFY2(result, "Operation did not propagate its result.");
    QCOMPARE(calls.size(), 2);
    QCOMPARE(calls.at(0), qMakePair(true, QStringLiteral("Doing work…")));
    QCOMPARE(calls.at(1), qMakePair(false, QStringLiteral("Doing work…")));
}
