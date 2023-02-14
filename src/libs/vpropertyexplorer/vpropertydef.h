/************************************************************************
 **
 **  @file   vpropertydef.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   1 2, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#ifndef VPROPERTYDEF_H
#define VPROPERTYDEF_H

#include "vpropertyexplorer_global.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtGlobal>
using vpesizetype = qsizetype;
#else
using vpesizetype = int;
#endif

VPROPERTYEXPLORERSHARED_EXPORT auto VPELocalePositiveSign(const QLocale &locale) -> QChar;
VPROPERTYEXPLORERSHARED_EXPORT auto VPELocaleNegativeSign(const QLocale &locale) -> QChar;
VPROPERTYEXPLORERSHARED_EXPORT auto VPELocaleSign0(const QLocale &locale) -> QChar;
VPROPERTYEXPLORERSHARED_EXPORT auto VPELocaleSign1(const QLocale &locale) -> QChar;
VPROPERTYEXPLORERSHARED_EXPORT auto VPELocaleSign2(const QLocale &locale) -> QChar;
VPROPERTYEXPLORERSHARED_EXPORT auto VPELocaleSign3(const QLocale &locale) -> QChar;
VPROPERTYEXPLORERSHARED_EXPORT auto VPELocaleSign4(const QLocale &locale) -> QChar;
VPROPERTYEXPLORERSHARED_EXPORT auto VPELocaleSign5(const QLocale &locale) -> QChar;
VPROPERTYEXPLORERSHARED_EXPORT auto VPELocaleSign6(const QLocale &locale) -> QChar;
VPROPERTYEXPLORERSHARED_EXPORT auto VPELocaleSign7(const QLocale &locale) -> QChar;
VPROPERTYEXPLORERSHARED_EXPORT auto VPELocaleSign8(const QLocale &locale) -> QChar;
VPROPERTYEXPLORERSHARED_EXPORT auto VPELocaleSign9(const QLocale &locale) -> QChar;
VPROPERTYEXPLORERSHARED_EXPORT auto VPELocaleExpUpper(const QLocale &locale) -> QChar;
VPROPERTYEXPLORERSHARED_EXPORT auto VPELocaleExpLower(const QLocale &locale) -> QChar;
VPROPERTYEXPLORERSHARED_EXPORT auto VPELocaleDecimalPoint(const QLocale &locale) -> QChar;
VPROPERTYEXPLORERSHARED_EXPORT auto VPELocaleGroupSeparator(const QLocale &locale) -> QChar;

#endif // VPROPERTYDEF_H
