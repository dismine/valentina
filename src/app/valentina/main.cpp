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
#include <QTimer>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <xercesc/util/PlatformUtils.hpp>
#endif

#ifdef CRASH_REPORTING
#include "../vmisc/crashhandler/crashhandler.h"
#include "version.h"
#endif

// Fix bug in Qt. Deprecation warning in QMessageBox::critical.
#if QT_VERSION >= QT_VERSION_CHECK(6, 2, 0)
#undef QT_REQUIRE_VERSION
#define QT_REQUIRE_VERSION(argc, argv, str)                                                                            \
    {                                                                                                                  \
        QString s = QString::fromLatin1(str);                                                                          \
        QString sq = QString::fromLatin1(qVersion());                                                                  \
        if ((sq.section(QChar::fromLatin1('.'), 0, 0).toInt() << 16) +                                                 \
                (sq.section(QChar::fromLatin1('.'), 1, 1).toInt() << 8) +                                              \
                sq.section(QChar::fromLatin1('.'), 2, 2).toInt() <                                                     \
            (s.section(QChar::fromLatin1('.'), 0, 0).toInt() << 16) +                                                  \
                (s.section(QChar::fromLatin1('.'), 1, 1).toInt() << 8) +                                               \
                s.section(QChar::fromLatin1('.'), 2, 2).toInt())                                                       \
        {                                                                                                              \
            if (!qApp)                                                                                                 \
            {                                                                                                          \
                new QApplication(argc, argv);                                                                          \
            }                                                                                                          \
            QString s = QApplication::tr("Executable '%1' requires Qt %2, found Qt %3.")                               \
                            .arg(qAppName())                                                                           \
                            .arg(QString::fromLatin1(str))                                                             \
                            .arg(QString::fromLatin1(qVersion()));                                                     \
            QMessageBox::critical(0, QApplication::tr("Incompatible Qt Library Error"), s, QMessageBox::Abort,         \
                                  static_cast<QMessageBox::StandardButton>(0));                                        \
            qFatal("%s", s.toLatin1().data());                                                                         \
        }                                                                                                              \
    }
#endif

//---------------------------------------------------------------------------------------------------------------------
auto main(int argc, char *argv[]) -> int
{
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
#endif
    Q_INIT_RESOURCE(win_light_theme); // NOLINT
    Q_INIT_RESOURCE(win_dark_theme);  // NOLINT

#ifdef CRASH_REPORTING
    InitializeCrashpad(QStringLiteral(VER_PRODUCTNAME_STR).toLower());
#endif

#if defined(Q_OS_WIN)
    VAbstractApplication::WinAttachConsole();
#endif

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

    auto Terminate = qScopeGuard([]() { XERCES_CPP_NAMESPACE::XMLPlatformUtils::Terminate(); });
#endif

    VApplication app(argc, argv);
    app.InitOptions();

    QT_REQUIRE_VERSION(argc, argv, "5.15.0") // clazy:exclude=qstring-arg,qstring-allocations NOLINT

    VApplication::setDesktopFileName(QStringLiteral("ua.com.smart-pattern.valentina.desktop"));

    if (VApplication::IsGUIMode() && VAbstractApplication::VApp()->Settings()->IsAutomaticallyCheckUpdates())
    {
        // Set feed URL before doing anything else
        FvUpdater::sharedUpdater()->SetFeedURL(FvUpdater::CurrentFeedURL());

        // Check for updates automatically
        FvUpdater::sharedUpdater()->CheckForUpdatesSilent();
    }

    MainWindow w;
#if !defined(Q_OS_MAC)
    VApplication::setWindowIcon(QIcon(":/icon/64x64/icon64x64.png"));
#endif // !defined(Q_OS_MAC)
    app.setMainWindow(&w);

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
