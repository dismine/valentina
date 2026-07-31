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
#include <QDir>
#include <QFile>
#include <QFileInfo>
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
// Write straight to the file rather than going through qCInfo(). The message handler marshals logging from a worker
// thread onto the GUI thread, which is the very thread that is stuck, so a logged line does not reach the file until
// the stall ends - long after the crash report was captured and its copy of the log attached.
//
// This goes to the watchdog's own file, never the application log. Sharing that file loses the data: the GUI thread's
// logger keeps its own handle and file position, so everything appended during a stall is overwritten as soon as the
// GUI thread resumes and writes at the offset it remembers.
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
DWORD mainThreadId = 0;            // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)

//---------------------------------------------------------------------------------------------------------------------
void CaptureMainThreadHandle()
{
    mainThreadId = GetCurrentThreadId();

    // GetCurrentThread() hands back a pseudo-handle that always refers to whichever thread uses it, so the monitor
    // thread cannot be given it directly. Duplicate it into a real handle while we are still on the main thread.
    if (not DuplicateHandle(GetCurrentProcess(),
                            GetCurrentThread(),
                            GetCurrentProcess(),
                            &mainThreadHandle,
                            THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME | THREAD_QUERY_INFORMATION,
                            FALSE,
                            0))
    {
        mainThreadHandle = nullptr;
    }
}

//---------------------------------------------------------------------------------------------------------------------
// How much CPU the main thread has actually burned, split kernel/user, in ms. A stack says where the thread is; it
// cannot say why it is slow, and the three candidates want opposite fixes:
//
//   user time climbing    - the thread is computing. Our allocation volume is the problem.
//   kernel time climbing  - it is in the kernel: page faults, memory decompression, syscalls. The machine is.
//   neither climbing      - it is blocked, waiting on I/O or a lock.
//
// Summing the two hides the distinction that matters most here, so keep them apart.
struct MainThreadCpu
{
    qint64 kernelMs{-1};
    qint64 userMs{-1};
};

auto SampleMainThreadCpu() -> MainThreadCpu
{
    FILETIME creation = {};
    FILETIME exit = {};
    FILETIME kernel = {};
    FILETIME user = {};

    if (mainThreadHandle == nullptr || not GetThreadTimes(mainThreadHandle, &creation, &exit, &kernel, &user))
    {
        return {};
    }

    auto ToMs = [](const FILETIME &ft)
    {
        // FILETIME counts 100-nanosecond intervals, split across two 32-bit halves.
        const auto ticks = (static_cast<quint64>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
        return static_cast<qint64>(ticks / 10000);
    };

    return {ToMs(kernel), ToMs(user)};
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

//---------------------------------------------------------------------------------------------------------------------
// Is the main thread really failing to pump messages, or is it merely parked in one of Windows' own modal loops?
//
// The heartbeat rides Qt's event loop, and a native file dialog, an open menu or a window being dragged all run a
// message loop of their own that never gets back to Qt's. A user reading an Open dialog for ten seconds therefore
// looks exactly like a freeze - which is how a session that was never stuck spends its whole dump budget before the
// real freeze arrives. One report already came in this way: the log ends at getOpenFileName(), the stack sits in
// comctl32's text drawing, and the thread burned no CPU at all.
//
// Windows draws the distinction already. IsHungAppWindow() is what makes Explorer stamp a window "Not Responding",
// and it is true only once the owning thread has stopped answering messages - which a healthy dialog loop still does.
// Ask it rather than trying to recognise a dialog from the stack.
auto MainThreadHung() -> bool
{
    GUITHREADINFO info = {};
    info.cbSize = sizeof(info);

    if (mainThreadId == 0 || not GetGUIThreadInfo(mainThreadId, &info))
    {
        return true;
    }

    // Any window owned by the thread answers the question. There may be none to ask about - the app sits in the
    // background with nothing active or focused - and then we report, because a missed freeze costs more than a
    // spurious dump the cap already limits to three.
    const HWND window = info.hwndActive != nullptr ? info.hwndActive : info.hwndFocus;
    return window == nullptr || IsHungAppWindow(window);
}
#else
//---------------------------------------------------------------------------------------------------------------------
// Only Windows runs modal loops that starve Qt's event loop while the thread itself stays responsive, and only Windows
// can be asked about it. Everywhere else a stopped heartbeat is a stall.
constexpr auto MainThreadHung() -> bool
{
    return true;
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
    MainThreadCpu lastCpu{};
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

        // Checked every pass rather than once when the stall starts: a dialog opened during a genuine stall must not
        // silence the rest of it, and a dialog closed during one must not keep it silenced.
        if (elapsed >= stallThresholdMs && MainThreadHung())
        {
            const bool stallJustStarted = not stalled;

            if (stallJustStarted)
            {
                stalled = true;
                stallStart = beat;
#if defined(Q_OS_WIN)
                samplesTaken = 0;
                lastCpu = SampleMainThreadCpu(); // Baseline, so the first sample's delta is a real measurement.
#endif

                const QString since = QDateTime::fromMSecsSinceEpoch(beat).toString(
                    QStringLiteral("yyyy.MM.dd hh:mm:ss"));

                // Written straight to the file so it is already there when the dump below attaches a copy of the log.
                AppendStallToLog(QStringLiteral("[%1:WATCHDOG] Main thread stalled for %2 ms, since %3.")
                                     .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy.MM.dd hh:mm:ss")))
                                     .arg(elapsed)
                                     .arg(since));
            }

#if defined(Q_OS_WIN)
            // Before the dump, not after: crashpad attaches its copy of the log at the moment the dump is taken, so a
            // sample written afterwards only reaches the backend with the *next* stall's report - one freeze late, and
            // never at all for a session that stalls once.
            if (samplesTaken < maxSamplesPerStall)
            {
                ++samplesTaken;

                const MainThreadCpu cpu = SampleMainThreadCpu();
                const bool haveCpu = cpu.kernelMs >= 0 && lastCpu.kernelMs >= 0;
                const qint64 kernelDelta = haveCpu ? cpu.kernelMs - lastCpu.kernelMs : -1;
                const qint64 userDelta = haveCpu ? cpu.userMs - lastCpu.userMs : -1;
                lastCpu = cpu;

                if (const QString stack = WalkMainThreadStack(); not stack.isEmpty())
                {
                    AppendStallToLog(
                        QStringLiteral("[%1:WATCHDOG] Stalled, kernel +%2 ms user +%3 ms per %4 ms wall, at %5")
                            .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy.MM.dd hh:mm:ss")))
                            .arg(kernelDelta)
                            .arg(userDelta)
                            .arg(heartbeatIntervalMs)
                            .arg(stack));
                }
            }
#endif

            if (stallJustStarted && dumpsSent < maxDumpsPerSession)
            {
                ++dumpsSent;
                DumpStalledMainThread(elapsed);
            }
        }
        else if (stalled && elapsed < stallThresholdMs)
        {
            // Only a beat that actually moved is a recovery. Falling out of the branch above because the thread stopped
            // looking hung is not one, and would report a recovery after 0 ms.
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
auto MainThreadWatchdogLogPath(const QString &logFilePath) -> QString
{
    if (logFilePath.isEmpty())
    {
        return {};
    }

    QFileInfo const info(logFilePath);
    const QString suffix = info.suffix();
    const QString name = info.completeBaseName() + QStringLiteral("-watchdog")
                         + (suffix.isEmpty() ? QString() : QLatin1Char('.') + suffix);

    return QDir::toNativeSeparators(info.absoluteDir().filePath(name));
}

//---------------------------------------------------------------------------------------------------------------------
void StartMainThreadWatchdog(const QString &logFilePath)
{
    if (monitoring.exchange(true))
    {
        return;
    }

    logPath = MainThreadWatchdogLogPath(logFilePath);

    // Create it now, empty sessions included. Crashpad lists this file as an attachment when the handler starts, and a
    // path that does not exist is not worth finding out about at crash time. It also makes "the watchdog was running"
    // an observable fact rather than an assumption.
    AppendStallToLog(QStringLiteral("[%1:WATCHDOG] Watchdog armed.")
                         .arg(QDateTime::currentDateTime().toString(QStringLiteral("yyyy.MM.dd hh:mm:ss"))));

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
