/************************************************************************
 **
 **  @file   defglobal.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 11, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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
#ifndef DEFGLOBAL_H
#define DEFGLOBAL_H

#include <QtGlobal>

constexpr qreal PrintDPI = 96.0;

#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
// Since Qt 6.0 minimal requirement is C++17. There is no longer need for qAsConst.
#define qAsConst std::as_const
#endif

#ifndef Q_DISABLE_ASSIGN
#define Q_DISABLE_ASSIGN(Class) Class &operator=(const Class &) = delete;
#endif

#ifndef Q_DISABLE_ASSIGN_MOVE
#define Q_DISABLE_ASSIGN_MOVE(Class)                                                                                   \
    Q_DISABLE_ASSIGN(Class)                                                                                            \
    Class(Class &&) = delete;                                                                                          \
    Class &operator=(Class &&) = delete;
#endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
using vsizetype = qsizetype;
#else
using vsizetype = int;
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QTextCodec;
using VTextCodec = QTextCodec;
#endif

#endif // DEFGLOBAL_H
