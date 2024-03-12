/************************************************************************
 **
 **  @file   projectversion.h
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

#ifndef PROJECTVERSION_H
#define PROJECTVERSION_H

class QString;

constexpr unsigned MAJOR_VERSION = 0;
constexpr unsigned MINOR_VERSION = 7;
constexpr unsigned DEBUG_VERSION = 52;

auto AppVersionStr() -> const QString &;

constexpr inline auto FormatVersion(unsigned major, unsigned minor, unsigned patch) -> unsigned
{
    return ((major << 16u) | (minor << 8u) | patch);
}

constexpr inline auto AppVersion() -> unsigned
{
    return FormatVersion(MAJOR_VERSION, MINOR_VERSION, DEBUG_VERSION);
}

// Change version number in projectversion.cpp too.
// Synchronize valentina.nsi

#define VER_FILEVERSION 0, 7, 52
#define VER_FILEVERSION_STR "0.7.52\0"

#define V_PRERELEASE // Mark prerelease builds

#define VER_PRODUCTVERSION VER_FILEVERSION
#define VER_PRODUCTVERSION_STR VER_FILEVERSION_STR

#define VER_COMPANYNAME_STR "Valentina"
// #define VER_FILEDESCRIPTION_STR   "Patternmaking program." // Defined in program
// #define VER_INTERNALNAME_STR      "Valentina" // Defined in program
#define VER_LEGALCOPYRIGHT_STR "Copyright © 2014-2022 Valentina Team"
#define VER_LEGALTRADEMARKS1_STR "All Rights Reserved"
#define VER_LEGALTRADEMARKS2_STR VER_LEGALTRADEMARKS1_STR
// #define VER_ORIGINALFILENAME_STR  "valentina.exe" // Defined in program
// #define VER_PRODUCTNAME_STR       "Valentina" // Defined in program

#define VER_COMPANYDOMAIN_STR "smart-pattern.com.ua"

auto compilerString() -> QString;
auto buildCompatibilityString() -> QString;

#endif // PROJECTVERSION_H
