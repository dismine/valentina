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

#include <QMessageBox> // For QT_REQUIRE_VERSION
#include <QTimer>

#include "../vmisc/def.h"
#include "vpapplication.h"

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

auto main(int argc, char *argv[]) -> int
{
#if defined(APPIMAGE) && defined(Q_OS_LINUX)
    /* Fix path to ICU_DATA when run AppImage.*/
    char *exe_dir = IcuDataPath("/../share/icu");
    auto FreeMemory = qScopeGuard([exe_dir] { free(exe_dir); });
#endif // defined(APPIMAGE) && defined(Q_OS_LINUX)

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
#endif
    Q_INIT_RESOURCE(win_light_theme); // NOLINT
    Q_INIT_RESOURCE(win_dark_theme);  // NOLINT

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

    auto Terminate = qScopeGuard([]() { XERCES_CPP_NAMESPACE::XMLPlatformUtils::Terminate(); });
#endif

    VPApplication app(argc, argv);
    app.InitOptions();

    QT_REQUIRE_VERSION(argc, argv, "5.6.0") // clazy:exclude=qstring-arg,qstring-allocations NOLINT

#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
    VPApplication::setDesktopFileName(QStringLiteral("ua.com.smart-pattern.puzzle.desktop"));
#endif

    QTimer::singleShot(0, &app, &VPApplication::ProcessCMD);

#if defined(APPIMAGE) && defined(Q_OS_LINUX)
    if (exe_dir)
    {
        qDebug() << "Path to ICU folder:" << exe_dir;
    }
#endif // defined(APPIMAGE) && defined(Q_OS_LINUX)

    return VPApplication::exec();
}
