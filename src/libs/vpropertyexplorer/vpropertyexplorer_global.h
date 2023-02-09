/************************************************************************
 **
 **  @file   vpropertyexplorer_global.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 8, 2014
 **
 **  @brief
 **  @copyright
 **  All rights reserved. This program and the accompanying materials
 **  are made available under the terms of the GNU Lesser General Public License
 **  (LGPL) version 2.1 which accompanies this distribution, and is available at
 **  http://www.gnu.org/licenses/lgpl-2.1.html
 **
 **  This library is distributed in the hope that it will be useful,
 **  but WITHOUT ANY WARRANTY; without even the implied warranty of
 **  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 **  Lesser General Public License for more details.
 **
 *************************************************************************/

#ifndef VPROPERTYEXPLORER_GLOBAL_H
#define VPROPERTYEXPLORER_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(VPROPERTYEXPLORER_LIBRARY)
#  define VPROPERTYEXPLORERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define VPROPERTYEXPLORERSHARED_EXPORT Q_DECL_IMPORT
#endif

#ifndef Q_DISABLE_ASSIGN
#define Q_DISABLE_ASSIGN(Class) \
    Class &operator=(const Class &) = delete;
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#define Q_DISABLE_COPY_MOVE(Class) \
    Q_DISABLE_COPY(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;
#endif

#ifndef Q_DISABLE_ASSIGN_MOVE
#define Q_DISABLE_ASSIGN_MOVE(Class) \
    Q_DISABLE_ASSIGN(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;
#endif

#endif // VPROPERTYEXPLORER_GLOBAL_H
