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

#include "vpapplication.h"

#include <QMessageBox> // For QT_REQUIRE_VERSION
#include <QTimer>

#ifdef Q_OS_WIN
#include <clocale>
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

#include "vmainbase.h"

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

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#if defined(SHARED_ICU_DATA)
#if defined(Q_OS_MACOS)
    const QString icuDataPath = QCoreApplication::applicationDirPath() + "/../Resources/icu"_L1;
    u_setDataDirectory(icuDataPath.toUtf8().constData());
#endif
#endif // defined(SHARED_ICU_DATA)
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

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
    const VXercesGuard xercesGuard;
#endif

    VPApplication app(argc, argv);
    app.InitOptions();

    VAbstractApplication::LogScreenInfo();

    QT_REQUIRE_VERSION(argc, argv, "5.15.0") // clazy:exclude=qstring-arg,qstring-allocations NOLINT

    VPApplication::setDesktopFileName(QStringLiteral("ua.com.smart-pattern.puzzle"));

    QTimer::singleShot(0, &app, &VPApplication::ProcessCMD);

    return VPApplication::exec();
}
