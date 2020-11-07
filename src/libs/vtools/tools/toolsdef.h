/************************************************************************
 **
 **  @file   toolsdef.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 11, 2020
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
#ifndef TOOLSDEF_H
#define TOOLSDEF_H

#include <QString>
#include <QMetaType>

class VGObject;
class VContainer;

struct SourceItem
{
    quint32 id{0};
    QString alias{};
};

Q_DECLARE_METATYPE(SourceItem)
Q_DECLARE_TYPEINFO(SourceItem, Q_MOVABLE_TYPE);

QVector<quint32> SourceToObjects(const QVector<SourceItem> &source);

QString OriginAlias(quint32 id, const QVector<SourceItem> &source, const QSharedPointer<VGObject> &obj);

bool SourceAliasValid(const SourceItem &item, const QSharedPointer<VGObject> &obj, const VContainer *data,
                      const QString &originAlias);

#endif // TOOLSDEF_H
