/************************************************************************
 **
 **  @file   projectversion.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 7, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "projectversion.h"

#include <QCoreApplication>
#include <QLatin1Char>
#include <QLatin1String>
#include <QObject>
#include <QString>
#include <QSysInfo>
#include <QtGlobal>

#if !defined(LATEST_TAG_DISTANCE) && defined(QBS_BUILD)
#include <vcsRepoState.h>
#define LATEST_TAG_DISTANCE VCS_REPO_STATE_DISTANCE
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
auto AppVersionStr() -> const QString &
{
    static const QString appVersionStr =
        QStringLiteral("%1.%2.%3.%4").arg(MAJOR_VERSION).arg(MINOR_VERSION).arg(DEBUG_VERSION).arg(LATEST_TAG_DISTANCE);
    return appVersionStr;
}

//---------------------------------------------------------------------------------------------------------------------
auto compilerString() -> QString
{
#if defined(Q_CC_INTEL) // must be before GNU, Clang and MSVC because ICC/ICL claim to be them
    QString iccCompact;
#ifdef __INTEL_CLANG_COMPILER
    iccCompact = "Clang"_L1;
#elif defined(__INTEL_MS_COMPAT_LEVEL)
    iccCompact = "Microsoft"_L1;
#elif defined(__GNUC__)
    iccCompact = Q "GCC"_L1;
#else
    iccCompact = "no"_L1;
#endif
    QString iccVersion;
    if (__INTEL_COMPILER >= 1300)
    {
        iccVersion = QString::number(__INTEL_COMPILER / 100);
    }
    else
    {
        iccVersion = QLatin1String(__INTEL_COMPILER);
    }
#ifdef __INTEL_COMPILER_UPDATE
    return "Intel(R) C++ "_L1 + iccVersion + '.'_L1 + QLatin1String(__INTEL_COMPILER_UPDATE) + " build "_L1 +
           QLatin1String(__INTEL_COMPILER_BUILD_DATE) + " ["_L1 + QLatin1String(iccCompact) + " compatibility]"_L1;
#else
    return "Intel(R) C++ "_L1 + iccVersion + " build "_L1 + QLatin1String(__INTEL_COMPILER_BUILD_DATE) + " ["_L1 +
           iccCompact + " compatibility]"_L1;
#endif
#elif defined(Q_CC_CLANG)            // must be before GNU, because clang claims to be GNU too
    // cppcheck-suppress unassignedVariable
    QString isAppleString;
#if defined(__apple_build_version__) // Apple clang has other version numbers
    isAppleString = " (Apple)"_L1;
#endif
    return "Clang "_L1 + QString::number(__clang_major__) + '.'_L1 + QString::number(__clang_minor__) + isAppleString;
#elif defined(Q_CC_GNU)
    return "GCC "_L1 + QLatin1String(__VERSION__);
#elif defined(Q_CC_MSVC)
    if (_MSC_VER >= 1800) // 1800: MSVC 2013 (yearly release cycle)
    {
        return "MSVC "_L1 + QString::number(2008 + ((_MSC_VER / 100) - 13));
    }
    if (_MSC_VER >= 1500) // 1500: MSVC 2008, 1600: MSVC 2010, ... (2-year release cycle)
    {
        return "MSVC "_L1 + QString::number(2008 + 2 * ((_MSC_VER / 100) - 15));
    }
    return "MSVC <unknown version>"_L1;
#else
    return QStringLiteral("<unknown compiler>");
#endif
}

//---------------------------------------------------------------------------------------------------------------------
auto buildCompatibilityString() -> QString
{
    return QCoreApplication::tr("Based on Qt %1 (%2, %3 bit)")
        .arg(QLatin1String(qVersion()), compilerString(), QString::number(QSysInfo::WordSize));
}
