/************************************************************************
 **
 **  @file   main.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#include "../fervor/fvupdater.h"
#include "../vpatterndb/vpiecenode.h"
#include "core/vapplication.h"
#include "mainwindow.h"
#include "vabstractapplication.h"

#include <QMessageBox> // For QT_REQUIRE_VERSION
#include <QScopeGuard>
#include <QScreen>
#include <QTimer>

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

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QDir>
#if defined(SHARED_ICU_DATA)
#include <unicode/putil.h>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;
#endif
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

//---------------------------------------------------------------------------------------------------------------------
auto main(int argc, char *argv[]) -> int
{
#ifdef Q_OS_WIN
    std::setlocale(LC_ALL, ".UTF8");
#endif

    Q_INIT_RESOURCE(cursor);     // NOLINT
    Q_INIT_RESOURCE(toolcursor); // NOLINT
    Q_INIT_RESOURCE(icon);       // NOLINT
    Q_INIT_RESOURCE(schema);     // NOLINT
    Q_INIT_RESOURCE(flags);      // NOLINT
    Q_INIT_RESOURCE(icons);      // NOLINT
    Q_INIT_RESOURCE(toolicon);   // NOLINT
    Q_INIT_RESOURCE(breeze);     // NOLINT
    Q_INIT_RESOURCE(scenestyle); // NOLINT
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

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#if defined(SHARED_ICU_DATA)
#if defined(Q_OS_MACOS)
    const QString icuDataPath = QCoreApplication::applicationDirPath() + "/../Resources/icu"_L1;
    u_setDataDirectory(icuDataPath.toUtf8().constData());
#endif
#endif // defined(SHARED_ICU_DATA)
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

    // Need to internally move a node inside a piece main path
    REGISTER_META_TYPE_STREAM_OPERATORS(VPieceNode);
    // Need to internally move a node inside a custom seam allowance path
    REGISTER_META_TYPE_STREAM_OPERATORS(CustomSARecord);

#ifndef Q_OS_MAC // supports natively
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    VAbstractApplication::InitHighDpiScaling(argc, argv);
#endif
#endif // Q_OS_MAC

#ifdef Q_OS_MAC
#if MACOS_LAYER_BACKING_AFFECTED
    MacosEnableLayerBacking();
#endif // MACOS_LAYER_BACKING_AFFECTED
#endif // Q_OS_MAC

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    XERCES_CPP_NAMESPACE::XMLPlatformUtils::Initialize();

    auto Terminate = qScopeGuard([]() -> void { XERCES_CPP_NAMESPACE::XMLPlatformUtils::Terminate(); });
#endif

    VApplication app(argc, argv);
    app.InitOptions();

    // === Diagnostic logging for DPI/multi-monitor crash investigation ===
    // This logs screen configuration to help diagnose crashes that only occur
    // on specific user systems, particularly related to:
    // - Multi-monitor setups with different DPI scaling (e.g., 100% + 150%)
    // - High-DPI displays (4K, 5K) with scaling > 100%
    // - QPainter "engine == 0" errors caused by invalid paint device dimensions
    // The crash stack trace shows UxTheme.dll + GetSystemMetricsForDpi, indicating
    // Windows is calculating UI metrics based on DPI, which may fail if Qt and
    // Windows disagree about DPI handling.
    qDebug() << "=== Screen Information ===";
    qDebug() << "Screens:";
    for (auto *screen : QGuiApplication::screens())
    {
        qDebug() << "  -" << screen->name() << "DPI:" << screen->logicalDotsPerInch()
                 << "Ratio:" << screen->devicePixelRatio() << "Geometry:" << screen->geometry();
    }

    // Monitor for screen configuration changes during runtime
    // These events can trigger repainting with new DPI values, which may expose
    // the crash if Qt/Windows DPI coordination fails during the transition
    QObject::connect(qApp,
                     &QGuiApplication::screenAdded,
                     [](QScreen *screen) -> void
                     {
                         qDebug() << "Screen added:" << screen->name();
                         // User plugged in external monitor - may trigger DPI recalculation
                     });

    QObject::connect(qApp,
                     &QGuiApplication::primaryScreenChanged,
                     [](QScreen *screen) -> void
                     {
                         qDebug() << "Primary screen changed to:" << screen->name();
                         // User moved window to different monitor or changed primary display
                         // This can cause widgets to repaint with different DPI values
                     });

    QT_REQUIRE_VERSION(argc, argv, "5.15.0") // clazy:exclude=qstring-arg,qstring-allocations NOLINT

    VApplication::setDesktopFileName(QStringLiteral("ua.com.smart-pattern.valentina"));

    MainWindow w;
#if !defined(Q_OS_MAC)
    VApplication::setWindowIcon(QIcon(":/icon/64x64/icon64x64.png"));
#endif // !defined(Q_OS_MAC)
    app.setMainWindow(&w);

    if (VApplication::IsGUIMode() && VAbstractApplication::VApp()->Settings()->IsAutomaticallyCheckUpdates())
    {
        // Set feed URL before doing anything else
        FvUpdater::sharedUpdater()->SetFeedURL(FvUpdater::CurrentFeedURL());

        FvUpdater::sharedUpdater()->SetMainWindow(&w);

        // Check for updates automatically
        FvUpdater::sharedUpdater()->CheckForUpdatesSilent();
    }

    int msec = 0;
    // Before we load pattern show window.
    if (VApplication::IsGUIMode())
    {
        w.show();
        msec = 15; // set delay for correct the first fitbest zoom
    }

    QTimer::singleShot(msec, &w, &MainWindow::ProcessCMD);

    return VApplication::exec();
}
