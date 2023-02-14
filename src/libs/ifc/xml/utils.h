/************************************************************************
 **
 **  @file   utils.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 1, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Valentina project
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
#ifndef UTILS_H
#define UTILS_H

#include <QtGlobal>
#include <QtCore/qcontainerfwd.h>

class QMimeType;
class QString;
class QMimeType;
class QByteArray;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QStringList;
#endif

auto IsMimeTypeImage(const QMimeType &mime) -> bool;
auto SplitString(QString str) -> QStringList;
auto MimeTypeFromByteArray(const QByteArray &data) -> QMimeType;
auto PrepareImageFilters() -> QString;

#endif // UTILS_H
