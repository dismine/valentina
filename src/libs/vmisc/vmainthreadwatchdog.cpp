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

// Needed to read the stalled thread's context, both for the crash report and for the stack sampling below. MinGW
// already defines NOMINMAX in os_defines.h, so guard the define instead of repeating it unconditionally.
#if defined(Q_OS_WIN)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

#include <QCoreApplication>
#include <QDateTime>
#include <QFile>
#include <QStringList>
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
// Enough samples to show which frames stay put and which move; a longer stall is already described by the first few.
constexpr int maxSamplesPerStall = 20;
constexpr int maxStackFrames = 24;

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

//---------------------------------------------------------------------------------------------------------------------
auto CaptureMainThreadContext(CONTEXT *context) -> bool
{
    if (mainThreadHandle == nullptr)
    {
        return false;
    }

    *context = {};
    context->ContextFlags = CONTEXT_FULL;

    if (SuspendThread(mainThreadHandle) == static_cast<DWORD>(-1))
    {
        return false;
    }

    // Nothing but the single call inside the suspend window. Anything that can take a lock - the loader lock in
    // particular - risks deadlocking against a main thread that already holds it, and unlike an ordinary deadlock this
    // one would leave the GUI suspended for good.
    const bool captured = GetThreadContext(mainThreadHandle, context);
    ResumeThread(mainThreadHandle);
    return captured;
}

//---------------------------------------------------------------------------------------------------------------------
auto ModuleOffset(quintptr address) -> QString
{
    HMODULE module = nullptr;
    if (not GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               reinterpret_cast<LPCWSTR>(address), // NOLINT(performance-no-int-to-ptr)
                               &module))
    {
        return QStringLiteral("0x%1").arg(address, 0, 16);
    }

    wchar_t path[MAX_PATH] = {};
    const DWORD length = GetModuleFileNameW(module, path, MAX_PATH);
    const QString name = QString::fromWCharArray(path, static_cast<int>(length)).section(QLatin1Char('\\'), -1);

    return QStringLiteral("%1+0x%2").arg(name).arg(address - reinterpret_cast<quintptr>(module), 0, 16);
}

//---------------------------------------------------------------------------------------------------------------------
// Where does the main thread actually sit during a stall? The uploaded minidumps cannot answer that: below the top few
// ntdll frames their stacks are recovered by scanning, which is why one shows ucrtbase!realloc_base at two
// non-adjacent levels and repeats three Qt6Core addresses. Unwinding here, in-process, uses the real unwind tables, so
// the frames are ordered and true. Sampling it once a second turns a stall into a profile: the frames that stay put
// across samples are the loop.
auto WalkMainThreadStack() -> QString
{
    CONTEXT context;
    if (not CaptureMainThreadContext(&context))
    {
        return {};
    }

#if defined(_WIN64)
    QStringList frames;

    // Unwind after resuming, never during the suspend: RtlLookupFunctionEntry can take the loader lock. A thread stuck
    // in one long operation has a stable stack, so an occasional torn sample costs nothing - the frames that matter are
    // the ones that repeat.
    for (int i = 0; i < maxStackFrames && context.Rip != 0; ++i)
    {
        frames.append(ModuleOffset(static_cast<quintptr>(context.Rip)));

        DWORD64 imageBase = 0;
        PRUNTIME_FUNCTION const function = RtlLookupFunctionEntry(context.Rip, &imageBase, nullptr);
        if (function == nullptr)
        {
            break; // Leaf function: no unwind data, and guessing the return address is how scanned stacks go wrong.
        }

        PVOID handlerData = nullptr;
        DWORD64 establisherFrame = 0;
        RtlVirtualUnwind(UNW_FLAG_NHANDLER,
                         imageBase,
                         context.Rip,
                         function,
                         &context,
                         &handlerData,
                         &establisherFrame,
                         nullptr);
    }

    return frames.join(QStringLiteral(" < "));
#else
    return ModuleOffset(static_cast<quintptr>(context.Eip)); // No RtlVirtualUnwind on 32-bit.
#endif
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
    // Report the *main* thread rather than this one. CRASHPAD_SIMULATE_CRASH() captures its caller, so every hang would
    // group under this function and bury the stack we are after.
    if (CONTEXT context; CaptureMainThreadContext(&context))
    {
        crashpad::CrashpadClient::DumpWithoutCrash(context);
        return;
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
#if defined(Q_OS_WIN)
    int samplesTaken = 0;
#endif

    while (monitoring.load(std::memory_order_relaxed))
    {
        QThread::msleep(static_cast<unsigned long>(heartbeatIntervalMs));

        const qint64 beat = heartbeat.load(std::memory_order_relaxed);

        // Nothing to compare against until the GUI thread has published its first beat. The heartbeat is a QTimer, so
        // it cannot fire before the event loop starts, while the watchdog is armed earlier - back in main(), before the
        // main window is even constructed. Measuring that window reports the whole of startup as a stall and burns a
        // dump slot on it every launch.
        if (beat == 0)
        {
            continue;
        }

        const qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - beat;

        if (elapsed >= stallThresholdMs)
        {
            if (not stalled)
            {
                stalled = true;
                stallStart = beat;
#if defined(Q_OS_WIN)
                samplesTaken = 0;
#endif

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

#if defined(Q_OS_WIN)
            if (samplesTaken < maxSamplesPerStall)
            {
                ++samplesTaken;
                if (const QString stack = WalkMainThreadStack(); not stack.isEmpty())
                {
                    AppendStallToLog(
                        QStringLiteral("[%1:WATCHDOG] Stalled at %2")
                            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy.MM.dd hh:mm:ss")), stack));
                }
            }
#endif
        }
        else if (stalled)
        {
            stalled = false;
            // Safe to go through the normal logging path now - the GUI thread is pumping events again.
            qCInfo(vWatchdog,
                   "%s",
                   qUtf8Printable(QStringLiteral("Main thread recovered after %1 ms.").arg(beat - stallStart)));
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

    // Left at 0 deliberately: the monitor treats that as "not armed yet" and waits for the timer below to publish the
    // first real beat, which cannot happen until the event loop is running. See MonitorMainThread().

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
