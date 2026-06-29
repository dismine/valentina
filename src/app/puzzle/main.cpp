/************************************************************************
 **
 **  @file   main.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 2, 2020
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

#include <QDebug>
#include <QMessageBox> // For QT_REQUIRE_VERSION
#include <QScreen>
#include <QTimer>

#include "vpapplication.h"

#ifdef Q_OS_WIN
#include <clocale>
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <xercesc/util/PlatformUtils.hpp>
#endif

#ifdef CRASH_REPORTING
#include "../vmisc/crashhandler/crashhandler.h"
#include "version.h"
#endif

// Fix bug in Qt. Deprecation warning in QMessageBox::critical.
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0) && QT_VERSION < QT_VERSION_CHECK(6, 6, 0)
#undef QT_REQUIRE_VERSION
#define QT_REQUIRE_VERSION(argc, argv, str) \
    { \
        QString s = QString::fromLatin1(str); \
        QString sq = QString::fromLatin1(qVersion()); \
        if ((sq.section(QChar::fromLatin1('.'), 0, 0).toInt() << 16) \
                + (sq.section(QChar::fromLatin1('.'), 1, 1).toInt() << 8) \
                + sq.section(QChar::fromLatin1('.'), 2, 2).toInt() \
            < (s.section(QChar::fromLatin1('.'), 0, 0).toInt() << 16) \
                  + (s.section(QChar::fromLatin1('.'), 1, 1).toInt() << 8) \
                  + s.section(QChar::fromLatin1('.'), 2, 2).toInt()) \
        { \
            if (!qApp) \
            { \
                new QApplication(argc, argv); \
            } \
            QString s = QApplication::tr("Executable '%1' requires Qt %2, found Qt %3.") \
                            .arg(qAppName()) \
                            .arg(QString::fromLatin1(str)) \
                            .arg(QString::fromLatin1(qVersion())); \
            QMessageBox::critical(nullptr, \
                                  QApplication::tr("Incompatible Qt Library Error"), \
                                  s, \
                                  QMessageBox::Abort, \
                                  static_cast<QMessageBox::StandardButton>(0)); \
            qFatal("%s", s.toLatin1().data()); \
        } \
    }
#endif

auto main(int argc, char *argv[]) -> int
{
#ifdef Q_OS_WIN
    std::setlocale(LC_ALL, ".UTF8");
#endif

    Q_INIT_RESOURCE(puzzleicon);   // NOLINT
    Q_INIT_RESOURCE(icon);         // NOLINT
    Q_INIT_RESOURCE(schema);       // NOLINT
    Q_INIT_RESOURCE(flags);        // NOLINT
    Q_INIT_RESOURCE(breeze);       // NOLINT
    Q_INIT_RESOURCE(puzzlecursor); // NOLINT
    Q_INIT_RESOURCE(scenestyle);   // NOLINT
    Q_INIT_RESOURCE(cursor);       // NOLINT
#if defined(Q_OS_MACX)
    Q_INIT_RESOURCE(mac_light_theme); // NOLINT
    Q_INIT_RESOURCE(mac_dark_theme);  // NOLINT
#else
    Q_INIT_RESOURCE(win_light_theme); // NOLINT
    Q_INIT_RESOURCE(win_dark_theme);  // NOLINT
#endif

#ifdef CRASH_REPORTING
    InitializeCrashpad(QStringLiteral(VER_PRODUCTNAME_STR));
#endif

#if defined(Q_OS_WIN)
    VAbstractApplication::WinAttachConsole();
#endif

#ifndef Q_OS_MAC // supports natively
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    VAbstractApplication::InitHighDpiScaling(argc, argv);
#endif
#endif // ndef Q_OS_MAC

#ifdef Q_OS_MAC
#if MACOS_LAYER_BACKING_AFFECTED
    MacosEnableLayerBacking();
#endif // MACOS_LAYER_BACKING_AFFECTED
#endif // Q_OS_MAC

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    XERCES_CPP_NAMESPACE::XMLPlatformUtils::Initialize();

    auto Terminate = qScopeGuard([]() -> void { XERCES_CPP_NAMESPACE::XMLPlatformUtils::Terminate(); });
#endif

    VPApplication app(argc, argv);
    app.InitOptions();

    // === Diagnostic logging for DPI/multi-monitor crash investigation ===
    // Crashes have been observed during screen/display reconfiguration (e.g. macOS
    // wake-from-sleep, plugging/unplugging a monitor, or changing display scaling). The
    // crash stack trace lands inside the platform style painting with no application frames,
    // so the log is the only signal. During the failing transition a transient QScreen with
    // an empty name() briefly appears, so logging only name() is useless. Dump the full
    // screen identity and follow every add/remove/primary-change plus per-screen DPI and
    // geometry change, to capture the exact sequence leading to the crash.
    auto DumpScreen = [](const QScreen *screen) -> QString
    {
        if (screen == nullptr)
        {
            return QStringLiteral("<null screen>");
        }

        QString info;
        QDebug dbg(&info);
        dbg.nospace() << "name=" << screen->name() << " manufacturer=" << screen->manufacturer()
                      << " model=" << screen->model() << " serial=" << screen->serialNumber()
                      << " geometry=" << screen->geometry() << " available=" << screen->availableGeometry()
                      << " DPR=" << screen->devicePixelRatio() << " logicalDPI=" << screen->logicalDotsPerInch()
                      << " physicalDPI=" << screen->physicalDotsPerInch() << " depth=" << screen->depth()
                      << " refreshRate=" << screen->refreshRate();
        return info;
    };

    auto ConnectScreenSignals = [DumpScreen](QScreen *screen) -> void
    {
        if (screen == nullptr)
        {
            return;
        }

        QObject::connect(screen,
                         &QScreen::geometryChanged,
                         screen,
                         [DumpScreen, screen](const QRect &geometry)
                         { qDebug() << "Screen geometry changed:" << geometry << "for" << DumpScreen(screen); });
        QObject::connect(screen,
                         &QScreen::logicalDotsPerInchChanged,
                         screen,
                         [DumpScreen, screen](qreal dpi)
                         { qDebug() << "Screen logical DPI changed:" << dpi << "for" << DumpScreen(screen); });
        QObject::connect(screen,
                         &QScreen::physicalDotsPerInchChanged,
                         screen,
                         [DumpScreen, screen](qreal dpi)
                         { qDebug() << "Screen physical DPI changed:" << dpi << "for" << DumpScreen(screen); });
    };

    qDebug() << "=== Screen Information ===";
    const QList<QScreen *> screens = QGuiApplication::screens();
    qDebug() << "Screens:" << screens.size();
    for (auto *screen : screens)
    {
        qDebug() << "  -" << DumpScreen(screen);
        ConnectScreenSignals(screen);
    }

    QObject::connect(qApp,
                     &QGuiApplication::screenAdded,
                     [DumpScreen, ConnectScreenSignals](QScreen *screen) -> void
                     {
                         qDebug() << "Screen added:" << DumpScreen(screen)
                                  << "(total:" << QGuiApplication::screens().size() << ")";
                         ConnectScreenSignals(screen);
                     });

    QObject::connect(qApp,
                     &QGuiApplication::screenRemoved,
                     [DumpScreen](QScreen *screen) -> void
                     {
                         qDebug() << "Screen removed:" << DumpScreen(screen)
                                  << "(remaining:" << QGuiApplication::screens().size() << ")";
                     });

    QObject::connect(qApp,
                     &QGuiApplication::primaryScreenChanged,
                     [DumpScreen](QScreen *screen) -> void
                     { qDebug() << "Primary screen changed to:" << DumpScreen(screen); });

    QT_REQUIRE_VERSION(argc, argv, "5.15.0") // clazy:exclude=qstring-arg,qstring-allocations NOLINT

    VPApplication::setDesktopFileName(QStringLiteral("ua.com.smart-pattern.puzzle"));

    QTimer::singleShot(0, &app, &VPApplication::ProcessCMD);

    return VPApplication::exec();
}
