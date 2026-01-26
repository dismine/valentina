/************************************************************************
 **
 **  @file   vabstractpoint.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   19 6, 2015
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

#include "vabstractpoint.h"

#include <QSharedPointer>

#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../operation/vabstractoperation.h"
#include "../vdrawtool.h"
#include "../vgeometry/vgobject.h"
#include "../vmisc/exception/vexception.h"

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto IsRelevantToolType(Tool toolType) -> bool
{
    return toolType == Tool::FlippingByAxis || toolType == Tool::FlippingByLine || toolType == Tool::Rotation
           || toolType == Tool::Move;
}

//---------------------------------------------------------------------------------------------------------------------
auto ExtractSuffixIfContainsId(vidtype recordId, vidtype targetId) -> std::optional<QString>
{
    try
    {
        auto *tool = qobject_cast<VAbstractOperation *>(VAbstractPattern::getTool(recordId));
        if (tool == nullptr)
        {
            return std::nullopt;
        }

        const QVector<SourceItem> source = tool->SourceItems();
        const bool contains = std::any_of(source.cbegin(),
                                          source.cend(),
                                          [targetId](const auto &item) -> auto { return item.id == targetId; });

        return contains ? std::optional<QString>(tool->Suffix()) : std::nullopt;
    }
    catch (const VExceptionBadId &)
    {
        return std::nullopt;
    }
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VAbstractPoint::VAbstractPoint(VAbstractPattern *doc, VContainer *data, quint32 id, const QString &notes)
    :VDrawTool(doc, data, id, notes)
{}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPoint::getTagName() const -> QString
{
    return VAbstractPattern::TagPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPoint::DeleteFromLabel()
{
    try
    {
        DeleteToolWithConfirm();
    }
    catch(const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPoint::SetPointName(quint32 id, const QString &name)
{
    // Don't know if need check name here.
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(id);
    obj->setName(name);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPoint::GroupSuffixes(quint32 id) const -> QList<QString>
{
    const QVector<vidtype> neighbors = doc->PatternGraph()->GetNeighbors(id);

    QSet<QString> suffixes;

    const QVector<VToolRecord> *history = doc->getHistory();
    for (const auto &record : *history)
    {
        if (!IsRelevantToolType(record.GetToolType()) || !neighbors.contains(record.GetId()))
        {
            continue;
        }

        if (auto suffix = ExtractSuffixIfContainsId(record.GetId(), id))
        {
            suffixes.insert(*suffix);
        }
    }

    return ConvertToList(suffixes);
}
