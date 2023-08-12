/************************************************************************
 **
 **  @file   text.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 2, 2023
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
#ifndef TEXT_H
#define TEXT_H

#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
#include <QTextStream>

namespace Qt
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wunused-variable")
QT_WARNING_DISABLE_CLANG("-Wunused-variable")

// static auto endl Q_DECL_UNUSED = ::endl; // NOLINT
// static auto flush Q_DECL_UNUSED = ::flush; // NOLINT
static auto dec Q_DECL_UNUSED = ::dec; // NOLINT
// static auto SkipEmptyParts Q_DECL_UNUSED = QString::SkipEmptyParts; // NOLINT

QT_WARNING_POP
} // namespace Qt

#endif

#endif // TEXT_H
