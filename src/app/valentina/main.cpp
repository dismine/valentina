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
#include <QTimer>

#if defined(APPIMAGE) && defined(Q_OS_LINUX)
#if QT_VERSION < QT_VERSION_CHECK(5, 12, 0)
#include "../vmisc/backport/qscopeguard.h"
#else
#include <QScopeGuard>
#endif
#include "../vmisc/appimage.h"
#endif // defined(APPIMAGE) && defined(Q_OS_LINUX)

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <xercesc/util/PlatformUtils.hpp>
#endif

//---------------------------------------------------------------------------------------------------------------------

auto main(int argc, char *argv[]) -> int
{
#if defined(APPIMAGE) && defined(Q_OS_LINUX)
    /* Fix path to ICU_DATA when run AppImage.*/
    char *exe_dir = IcuDataPath("/../share/icu");
    auto FreeMemory = qScopeGuard([exe_dir] { free(exe_dir); });
#endif // defined(APPIMAGE) && defined(Q_OS_LINUX)

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

    QT_REQUIRE_VERSION(argc, argv, "5.6.0") // clazy:exclude=qstring-arg,qstring-allocations NOLINT

#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    VApplication::setDesktopFileName(QStringLiteral("ua.com.smart-pattern.valentina.desktop"));
#endif

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

#if defined(APPIMAGE) && defined(Q_OS_LINUX)
    if (exe_dir)
    {
        qDebug() << "Path to ICU folder:" << exe_dir;
    }
#endif // defined(APPIMAGE) && defined(Q_OS_LINUX)

    return VApplication::exec();
}
