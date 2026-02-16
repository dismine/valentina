/************************************************************************
 **
 **  @file   deltool.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 6, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#include "deltool.h"

#include <QDomElement>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/vabstractvalapplication.h"
#include "vundocommand.h"
#include "../ifc/xml/vpatternblockmapper.h"

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto FixGroups(QMap<quint32, VGroupData> groups, const QMap<quint32, VGroupData> &fix) -> QMap<quint32, VGroupData>
{
    auto i = fix.constBegin();
    while (i != fix.constEnd())
    {
        groups.insert(i.key(), i.value());
        ++i;
    }

    return groups;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
DelTool::DelTool(VAbstractPattern *doc, quint32 id, QUndoCommand *parent)
  : VUndoCommand(doc, id, parent),
    parentNode(doc->ParentNodeById(id)),
    siblingId(doc->SiblingNodeId(id)),
    nameActivDraw(doc->PatternBlockMapper()->GetActive())
{
    setText(tr("delete tool"));
    SetElement(doc->CloneNodeById(id));

    QVector<QPair<vidtype, vidtype>> cleanItems;
    QMap<quint32, VGroupData> const groups = doc->GetGroups(nameActivDraw);
    auto i = groups.constBegin();
    while (i != groups.constEnd())
    {
        VGroupData groupData = i.value();
        auto itemRecord = std::find_if(groupData.items.begin(),
                                       groupData.items.end(),
                                       [id](const QPair<vidtype, vidtype> &item) -> bool { return item.second == id; });

        if (itemRecord != groupData.items.end())
        {
            m_groupsBefore.insert(i.key(), groupData);

            cleanItems.clear();
            cleanItems.reserve(groupData.items.size());

            for (auto item : std::as_const(groupData.items))
            {
                if (item.second != id)
                {
                    cleanItems.append(item);
                }
            }

            VGroupData cleanGroupData = groupData;
            cleanGroupData.items = cleanItems;
            m_groupsAfter.insert(i.key(), cleanGroupData);
        }
        ++i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DelTool::undo()
{
    qCDebug(vUndo, "Undo.");

    UndoDeleteAfterSibling(parentNode, siblingId);

    if (not m_groupsBefore.isEmpty())
    {
        UpdateGroups(FixGroups(Doc()->GetGroups(nameActivDraw), m_groupsBefore));
    }

    emit NeedFullParsing();

    if (VAbstractValApplication::VApp()->GetDrawMode() == Draw::Calculation)
    {                                          // Keep last!
        emit Doc()->ShowPatternBlock(nameActivDraw); // Without this user will not see this change
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DelTool::redo()
{
    qCDebug(vUndo, "Redo.");

    if (VAbstractValApplication::VApp()->GetDrawMode() == Draw::Calculation)
    {                                          // Keep first!
        emit Doc()->ShowPatternBlock(nameActivDraw); // Without this user will not see this change
    }
    QDomElement const domElement = Doc()->NodeById(ElementId());
    parentNode.removeChild(domElement);

    if (not m_groupsAfter.isEmpty())
    {
        UpdateGroups(FixGroups(Doc()->GetGroups(nameActivDraw), m_groupsAfter));
    }

    emit NeedFullParsing();
}

//---------------------------------------------------------------------------------------------------------------------
void DelTool::UpdateGroups(const QMap<quint32, VGroupData> &groups) const
{
    QDomElement groupsTag = Doc()->CreateGroups(nameActivDraw);
    if (not groupsTag.isNull())
    {
        VDomDocument::RemoveAllChildren(groupsTag);

        auto i = groups.constBegin();
        while (i != groups.constEnd())
        {
            QMap<vidtype, vidtype> groupMap;
            for (auto [first, second] : i.value().items)
            {
                groupMap.insert(first, second);
            }

            QDomElement group = Doc()->CreateGroup(i.key(), i.value().name, i.value().tags, groupMap, i.value().tool);
            Doc()->SetAttribute(group, VAbstractPattern::AttrVisible, i.value().visible);
            groupsTag.appendChild(group);

            ++i;
        }
    }
}
