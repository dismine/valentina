/************************************************************************
 **
 **  @file   toolsdef.cpp
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

#include "toolsdef.h"

#include <QRegularExpression>
#include <QVector>

#include "../vgeometry/vgobject.h"
#include "../qmuparser/qmudef.h"
#include "../vpatterndb/vcontainer.h"

//---------------------------------------------------------------------------------------------------------------------
QVector<quint32> SourceToObjects(const QVector<SourceItem> &source)
{
    QVector<quint32> ids;
    ids.reserve(source.size());

    for (auto s: source)
    {
        ids.append(s.id);
    }

    return ids;
}

//---------------------------------------------------------------------------------------------------------------------
bool SourceAliasValid(const SourceItem &item, const QSharedPointer<VGObject> &obj, const VContainer *data,
                      const QString &originAlias)
{
    SCASSERT(data != nullptr)

    QRegularExpression rx(NameRegExp());

    QString alias;

    if (obj->getType() == GOType::Point)
    {
        alias = item.alias;
    }
    else
    {
        const QString oldAlias = obj->GetAliasSuffix();
        obj->SetAliasSuffix(item.alias);
        alias = obj->GetAlias();
        obj->SetAliasSuffix(oldAlias);
    }

    if (not alias.isEmpty() && originAlias != alias &&
        (not rx.match(alias).hasMatch() || not data->IsUnique(alias)))
    {
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
QString OriginAlias(quint32 id, const QVector<SourceItem> &source, const QSharedPointer<VGObject> &obj)
{
    auto item = std::find_if(source.begin(), source.end(),
                            [id](const SourceItem &sItem) { return sItem.id == id; });
    if (item != source.end())
    {
        if (obj->getType() == GOType::Point)
        {
            return item->alias;
        }
        else
        {
            const QString oldAlias = obj->GetAliasSuffix();
            obj->SetAliasSuffix(item->alias);
            QString alias = obj->GetAlias();
            obj->SetAliasSuffix(oldAlias);
            return alias;
        }
    }

    return QString();
}
