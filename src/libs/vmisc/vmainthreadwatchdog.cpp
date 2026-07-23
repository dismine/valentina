/************************************************************************
 **
 **  @file   vmainthreadwatchdog.cpp
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
#include "vmainthreadwatchdog.h"

#if defined(Q_OS_WIN)
#define NOMINMAX
#include <windows.h>
#endif

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QThread>
#include <QTimer>

#include <atomic>

#ifdef CRASH_REPORTING
#include <string>

#include <client/annotation.h>
#include <client/crashpad_client.h>
#include <client/simulate_crash.h>
#endif

Q_LOGGING_CATEGORY(vWatchdog, "v.watchdog") // NOLINT

namespace
{
constexpr int heartbeatIntervalMs = 1000;
// Windows stamps a window "Not Responding" after 5 s of an unresponsive message pump. Matching it means the log says
// what the user saw.
constexpr qint64 stallThresholdMs = 5000;
// One pathological machine must not flood the crash backend. Stalls keep being logged after the cap.
constexpr int maxDumpsPerSession = 3;

std::atomic<qint64> heartbeat{0}; // Written by the GUI thread, read by the monitor.
std::atomic<bool> monitoring{false};

QString logPath; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables) - set once before the monitor starts.

//---------------------------------------------------------------------------------------------------------------------
// Append the stall directly to the log file rather than going through qCInfo(). The message handler marshals logging
// from a worker thread onto the GUI thread, which is the very thread that is stuck, so a logged line does not reach the
// file until the stall ends - long after the crash report was captured and its copy of the log attached. Writing here
// is safe precisely because the GUI thread is blocked: it cannot be touching its own QTextStream while we append.
void AppendStallToLog(const QString &line)
{
    if (logPath.isEmpty())
    {
        return;
    }

    if (QFile file(logPath); file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
    {
        QTextStream stream(&file);
        stream << line << Qt::endl;
    }
}

#if defined(Q_OS_WIN)
HANDLE mainThreadHandle = nullptr; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

//---------------------------------------------------------------------------------------------------------------------
void CaptureMainThreadHandle()
{
    // GetCurrentThread() hands back a pseudo-handle that always refers to whichever thread uses it, so the monitor
    // thread cannot be given it directly. Duplicate it into a real handle while we are still on the main thread.
    if (not DuplicateHandle(GetCurrentProcess(),
                            GetCurrentThread(),
                            GetCurrentProcess(),
                            &mainThreadHandle,
                            THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME,
                            FALSE,
                            0))
    {
        mainThreadHandle = nullptr;
    }
}
#endif

//---------------------------------------------------------------------------------------------------------------------
void DumpStalledMainThread(qint64 elapsed)
{
#ifdef CRASH_REPORTING
    // Safe when the user has crash reporting switched off: InitializeCrashpad() returns before starting a handler, and
    // crashpad's DumpWithoutCrash() bails out when no handler is connected.
    static crashpad::StringAnnotation<8> reportType("report_type");  // NOLINT
    static crashpad::StringAnnotation<24> stallDuration("stall_ms"); // NOLINT

    // Per-report, unlike the annotations handed to StartHandler(), which are fixed for the life of the process.
    // NOTE: verified NOT to reach the minidump in the Linux/GCC build - AnnotationList::Register() and
    // CrashpadInfo::GetCrashpadInfo() are not linked in and the binary has no crashpad_info section, so the registry
    // the handler reads does not exist. Left in because PE builds carry that section differently and it costs nothing,
    // but the marker that is actually relied on is the WATCHDOG line written into the attached log above.
    reportType.Set("hang");
    stallDuration.Set(std::to_string(elapsed).c_str());

#if defined(Q_OS_WIN)
    if (mainThreadHandle != nullptr)
    {
        CONTEXT context = {};
        context.ContextFlags = CONTEXT_FULL;

        // Report the *main* thread rather than this one. CRASHPAD_SIMULATE_CRASH() captures its caller, so every hang
        // would group under this function and bury the stack we are after.
        if (SuspendThread(mainThreadHandle) != static_cast<DWORD>(-1))
        {
            const bool captured = GetThreadContext(mainThreadHandle, &context);
            ResumeThread(mainThreadHandle);

            if (captured)
            {
                crashpad::CrashpadClient::DumpWithoutCrash(context);
                return;
            }
        }
    }
#endif

    // Fallback: dumps this thread instead, but the minidump still carries every thread's stack.
    CRASHPAD_SIMULATE_CRASH();
#else
    Q_UNUSED(elapsed)
#endif
}

//---------------------------------------------------------------------------------------------------------------------
void MonitorMainThread()
{
    bool stalled = false;
    qint64 stallStart = 0;
    int dumpsSent = 0;

    while (monitoring.load(std::memory_order_relaxed))
    {
        QThread::msleep(heartbeatIntervalMs);

        const qint64 beat = heartbeat.load(std::memory_order_relaxed);
        const qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - beat;

        if (elapsed >= stallThresholdMs)
        {
            if (not stalled)
            {
                stalled = true;
                stallStart = beat;

                const QString since = QDateTime::fromMSecsSinceEpoch(beat).toString(
                    QStringLiteral("yyyy.MM.dd hh:mm:ss"));

                // Written straight to the file so it is already there when the dump below attaches a copy of the log.
                AppendStallToLog(QStringLiteral("[%1:WATCHDOG] Main thread stalled for %2 ms, since %3.")
                                     .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy.MM.dd hh:mm:ss")))
                                     .arg(elapsed)
                                     .arg(since));

                if (dumpsSent < maxDumpsPerSession)
                {
                    ++dumpsSent;
                    DumpStalledMainThread(elapsed);
                }
            }
        }
        else if (stalled)
        {
            stalled = false;
            // Safe to go through the normal logging path now - the GUI thread is pumping events again.
            qCInfo(vWatchdog, "Main thread recovered after %lld ms.", beat - stallStart);
        }
    }
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
void StartMainThreadWatchdog(const QString &logFilePath)
{
    if (monitoring.exchange(true))
    {
        return;
    }

    logPath = logFilePath;

    heartbeat.store(QDateTime::currentMSecsSinceEpoch(), std::memory_order_relaxed);

#if defined(Q_OS_WIN)
    CaptureMainThreadHandle();
#endif

    auto *pulse = new QTimer(QCoreApplication::instance());
    pulse->setTimerType(Qt::CoarseTimer);
    QObject::connect(pulse,
                     &QTimer::timeout,
                     pulse,
                     []() { heartbeat.store(QDateTime::currentMSecsSinceEpoch(), std::memory_order_relaxed); });
    pulse->start(heartbeatIntervalMs);

    QThread *monitor = QThread::create(MonitorMainThread);
    monitor->setObjectName(QStringLiteral("watchdog"));
    QObject::connect(monitor, &QThread::finished, monitor, &QObject::deleteLater);
    QObject::connect(QCoreApplication::instance(),
                     &QCoreApplication::aboutToQuit,
                     monitor,
                     [monitor]()
                     {
                         monitoring.store(false, std::memory_order_relaxed);
                         monitor->wait(3000);
                     });
    monitor->start();
}
