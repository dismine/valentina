/************************************************************************
 **
 **  @file   undogroup.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 3, 2020
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

#include "undogroup.h"

#include <QDomNode>
#include <QDomNodeList>

#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../vmisc/def.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vtools/tools/vdatatool.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "vundocommand.h"

//AddGroup
//---------------------------------------------------------------------------------------------------------------------
AddGroup::AddGroup(const QDomElement &xml, VAbstractPattern *doc, QUndoCommand *parent)
  : VUndoCommand(xml, doc, parent),
    m_indexPatternBlock(doc->PatternBlockMapper()->GetActiveId())
{
    setText(tr("add group"));
    SetElementId(VDomDocument::GetParametrId(xml));
}

//---------------------------------------------------------------------------------------------------------------------
void AddGroup::undo()
{
    qCDebug(vUndo, "Undo.");

    Doc()->PatternBlockMapper()->SetActiveById(m_indexPatternBlock); //Without this user will not see this change

    if (QDomElement groups = Doc()->CreateGroups(); not groups.isNull())
    {
        if (QDomElement group = Doc()->FindElementById(ElementId(), VAbstractPattern::TagGroup); group.isElement())
        {
            group.setAttribute(VAbstractPattern::AttrVisible, trueStr);
            Doc()->ParseGroups(groups);
            if (groups.removeChild(group).isNull())
            {
                qCDebug(vUndo, "Can't delete group.");
                return;
            }
            emit UpdateGroups();
        }
        else
        {
            if (groups.childNodes().isEmpty())
            {
                QDomNode parent = groups.parentNode();
                parent.removeChild(groups);
            }

            qCDebug(vUndo, "Can't get group by id = %u.", ElementId());
            return;
        }
    }
    else
    {
        qCDebug(vUndo, "Can't get tag Groups.");
        return;
    }

    VMainGraphicsView::NewSceneRect(VAbstractValApplication::VApp()->getCurrentScene(),
                                    VAbstractValApplication::VApp()->getSceneView());
    if (VAbstractValApplication::VApp()->GetDrawMode() == Draw::Calculation)
    {
        emit Doc()->ShowPatternBlock(
            Doc()->PatternBlockMapper()->FindName(m_indexPatternBlock)); //Return current pattern piece after undo
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AddGroup::redo()
{
    qCDebug(vUndo, "Redo.");

    Doc()->PatternBlockMapper()->SetActiveById(m_indexPatternBlock); //Without this user will not see this change

    if (QDomElement groups = Doc()->CreateGroups(); not groups.isNull())
    {
        groups.appendChild(GetElement());
        Doc()->ParseGroups(groups);
        emit UpdateGroups();
    }
    else
    {
        qCDebug(vUndo, "Can't get tag Groups.");
        return;
    }

    VMainGraphicsView::NewSceneRect(VAbstractValApplication::VApp()->getCurrentScene(),
                                    VAbstractValApplication::VApp()->getSceneView());
}

//RenameGroup
//---------------------------------------------------------------------------------------------------------------------
RenameGroup::RenameGroup(VAbstractPattern *doc, quint32 id, const QString &name, QUndoCommand *parent)
  : VUndoCommand(doc, id, parent),
    newName(name)
{
    setText(tr("rename group"));
    oldName = doc->GetGroupName(id);
}

//---------------------------------------------------------------------------------------------------------------------
void RenameGroup::undo()
{
    qCDebug(vUndo, "Undo.");
    Doc()->SetGroupName(ElementId(), oldName);
    emit UpdateGroups();
    emit Doc()->UpdateToolTip();
}

//---------------------------------------------------------------------------------------------------------------------
void RenameGroup::redo()
{
    qCDebug(vUndo, "Redo.");

    Doc()->SetGroupName(ElementId(), newName);
    emit UpdateGroups();
    emit Doc()->UpdateToolTip();
}

//ChangeGroupOptions
//---------------------------------------------------------------------------------------------------------------------
ChangeGroupOptions::ChangeGroupOptions(
    VAbstractPattern *doc, quint32 id, const QString &name, const QStringList &tags, QUndoCommand *parent)
  : VUndoCommand(doc, id, parent),
    newName(name),
    newTags(tags)
{
    setText(tr("rename group"));
    oldName = doc->GetGroupName(id);
    oldTags = doc->GetGroupTags(id);
}

//---------------------------------------------------------------------------------------------------------------------
void ChangeGroupOptions::undo()
{
    qCDebug(vUndo, "Undo.");
    Doc()->SetGroupName(ElementId(), oldName);
    Doc()->SetGroupTags(ElementId(), oldTags);
    emit UpdateGroups();
    emit Doc()->UpdateToolTip();
}

//---------------------------------------------------------------------------------------------------------------------
void ChangeGroupOptions::redo()
{
    qCDebug(vUndo, "Redo.");
    Doc()->SetGroupName(ElementId(), newName);
    Doc()->SetGroupTags(ElementId(), newTags);
    emit UpdateGroups();
    emit Doc()->UpdateToolTip();
}

//AddItemToGroup
//---------------------------------------------------------------------------------------------------------------------
AddItemToGroup::AddItemToGroup(const QDomElement &xml, VAbstractPattern *doc, quint32 groupId, QUndoCommand *parent)
  : VUndoCommand(xml, doc, parent),
    m_indexPatternBlock(doc->PatternBlockMapper()->GetActiveId())
{
    setText(tr("Add item to group"));
    SetElementId(groupId);
}

//---------------------------------------------------------------------------------------------------------------------
void AddItemToGroup::undo()
{
    qCDebug(vUndo, "Undo the add item to group");
    performUndoRedo(true);
}

//---------------------------------------------------------------------------------------------------------------------
void AddItemToGroup::redo()
{
    qCDebug(vUndo, "Redo the add item to group");
    performUndoRedo(false);
}

//---------------------------------------------------------------------------------------------------------------------
void AddItemToGroup::performUndoRedo(bool isUndo)
{
    Doc()->PatternBlockMapper()->SetActiveById(m_indexPatternBlock); //Without this user will not see this change

    if (QDomElement group = Doc()->FindElementById(ElementId(), VAbstractPattern::TagGroup); group.isElement())
    {
        if(isUndo)
        {
            if (group.removeChild(GetElement()).isNull())
            {
                qCDebug(vUndo, "Can't delete item.");
                return;
            }

            // set the item visible. Because if the undo is done when unvisible and it's not in any group after the
            // undo, it stays unvisible until the entire drawing is completly rerendered.
            quint32 const objectId = VAbstractPattern::GetParametrUInt(GetElement(),
                                                                       QStringLiteral("object"),
                                                                       NULL_ID_STR);
            quint32 const toolId = VAbstractPattern::GetParametrUInt(GetElement(), QStringLiteral("tool"), NULL_ID_STR);
            VDataTool* tool = VAbstractPattern::getTool(toolId);
            tool->GroupVisibility(objectId,true);
        }
        else // is redo
        {
            if (group.appendChild(GetElement()).isNull())
            {
                qCDebug(vUndo, "Can't add item.");
                return;
            }
        }

        Doc()->SetModified(true);
        emit VAbstractValApplication::VApp()->getCurrentDocument()->patternChanged(false);

        if (QDomElement const groups = Doc()->CreateGroups(); not groups.isNull())
        {
            Doc()->ParseGroups(groups);
        }
        else
        {
            qCDebug(vUndo, "Can't get tag Groups.");
            return;
        }

        emit UpdateGroups();
    }
    else
    {
        qCDebug(vUndo, "Can't get group by id = %u.", ElementId());
        return;
    }

    VMainGraphicsView::NewSceneRect(VAbstractValApplication::VApp()->getCurrentScene(),
                                    VAbstractValApplication::VApp()->getSceneView());
    if (VAbstractValApplication::VApp()->GetDrawMode() == Draw::Calculation)
    {
        //Return current pattern piece after undo
        emit Doc()->ShowPatternBlock(Doc()->PatternBlockMapper()->FindName(m_indexPatternBlock));
    }
}

//RemoveItemFromGroup
//---------------------------------------------------------------------------------------------------------------------
RemoveItemFromGroup::RemoveItemFromGroup(const QDomElement &xml,
                                         VAbstractPattern *doc,
                                         quint32 groupId,
                                         QUndoCommand *parent)
  : VUndoCommand(xml, doc, parent),
    m_indexPatternBlock(doc->PatternBlockMapper()->GetActiveId())
{
    setText(tr("Remove item from group"));
    SetElementId(groupId);
}

//---------------------------------------------------------------------------------------------------------------------
void RemoveItemFromGroup::undo()
{
    qCDebug(vUndo, "Undo the remove item from group");
    performUndoRedo(true);
}

//---------------------------------------------------------------------------------------------------------------------
void RemoveItemFromGroup::redo()
{
    qCDebug(vUndo, "Redo the add item to group");
    performUndoRedo(false);
}

//---------------------------------------------------------------------------------------------------------------------
void RemoveItemFromGroup::performUndoRedo(bool isUndo)
{
    Doc()->PatternBlockMapper()->SetActiveById(m_indexPatternBlock); //Without this user will not see this change

    if (QDomElement group = Doc()->FindElementById(ElementId(), VAbstractPattern::TagGroup); group.isElement())
    {
        if(isUndo)
        {
            if (group.appendChild(GetElement()).isNull())
            {
                qCDebug(vUndo, "Can't add the item.");
                return;
            }
        }
        else // is redo
        {
            if (group.removeChild(GetElement()).isNull())
            {
                qCDebug(vUndo, "Can't delete item.");
                return;
            }

            // set the item visible. Because if the undo is done when unvisibile and it's not in any group after the
            // undo, it stays unvisible until the entire drawing is completly rerendered.
            quint32 const objectId = VAbstractPattern::GetParametrUInt(GetElement(),
                                                                       QStringLiteral("object"),
                                                                       NULL_ID_STR);
            quint32 const toolId = VAbstractPattern::GetParametrUInt(GetElement(), QStringLiteral("tool"), NULL_ID_STR);
            VDataTool* tool = VAbstractPattern::getTool(toolId);
            tool->GroupVisibility(objectId,true);
        }

        Doc()->SetModified(true);
        emit VAbstractValApplication::VApp()->getCurrentDocument()->patternChanged(false);

        if (QDomElement const groups = Doc()->CreateGroups(); not groups.isNull())
        {
            Doc()->ParseGroups(groups);
        }
        else
        {
            qCDebug(vUndo, "Can't get tag Groups.");
            return;
        }

        emit UpdateGroups();
    }
    else
    {
        qCDebug(vUndo, "Can't get group by id = %u.", ElementId());
        return;
    }

    VMainGraphicsView::NewSceneRect(VAbstractValApplication::VApp()->getCurrentScene(),
                                    VAbstractValApplication::VApp()->getSceneView());

    if (VAbstractValApplication::VApp()->GetDrawMode() == Draw::Calculation)
    {
        //Return current pattern piece after undo
        emit Doc()->ShowPatternBlock(Doc()->PatternBlockMapper()->FindName(m_indexPatternBlock));
    }
}

//ChangeGroupVisibility
//---------------------------------------------------------------------------------------------------------------------
ChangeGroupVisibility::ChangeGroupVisibility(VAbstractPattern *doc, vidtype id, bool visible, QUndoCommand *parent)
  : VUndoCommand(doc, id, parent),
    m_newVisibility(visible),
    m_indexPatternBlock(doc->PatternBlockMapper()->GetActiveId())
{
    setText(tr("change group visibility"));
    if (QDomElement const group = doc->FindElementById(id, VAbstractPattern::TagGroup); group.isElement())
    {
        m_oldVisibility = VDomDocument::GetParametrBool(group, VAbstractPattern::AttrVisible, trueStr);
    }
    else
    {
        qDebug("Can't get group by id = %u.", id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ChangeGroupVisibility::undo()
{
    qCDebug(vUndo, "Undo.");

    Do(m_oldVisibility);
}

//---------------------------------------------------------------------------------------------------------------------
void ChangeGroupVisibility::redo()
{
    qCDebug(vUndo, "Redo.");

    Do(m_newVisibility);
}

//---------------------------------------------------------------------------------------------------------------------
void ChangeGroupVisibility::Do(bool visible)
{
    Doc()->PatternBlockMapper()->SetActiveById(m_indexPatternBlock); //Without this user will not see this change

    if (QDomElement group = Doc()->FindElementById(ElementId(), VAbstractPattern::TagGroup); group.isElement())
    {
        Doc()->SetAttribute(group, VAbstractPattern::AttrVisible, visible);

        if (QDomElement const groups = Doc()->CreateGroups(); not groups.isNull())
        {
            Doc()->ParseGroups(groups);
        }

        emit UpdateGroup(ElementId(), visible);

        VMainGraphicsView::NewSceneRect(VAbstractValApplication::VApp()->getCurrentScene(),
                                        VAbstractValApplication::VApp()->getSceneView());
    }
    else
    {
        qDebug("Can't get group by id = %u.", ElementId());
    }
}

//ChangeMultipleGroupsVisibility
//---------------------------------------------------------------------------------------------------------------------
ChangeMultipleGroupsVisibility::ChangeMultipleGroupsVisibility(VAbstractPattern *doc,
                                                               const QVector<vidtype> &groups,
                                                               bool visible,
                                                               QUndoCommand *parent)
  : VUndoCommand(doc, parent),
    m_groups(groups),
    m_newVisibility(visible),
    m_indexPatternBlock(doc->PatternBlockMapper()->GetActiveId())
{
    setText(tr("change multiple groups visibility"));

    for(auto & groupId : m_groups)
    {
        if (QDomElement const group = doc->FindElementById(groupId, VAbstractPattern::TagGroup); group.isElement())
        {
            m_oldVisibility.insert(groupId,
                                   VDomDocument::GetParametrBool(group, VAbstractPattern::AttrVisible, trueStr));
        }
        else
        {
            qDebug("Can't get group by id = %u.", groupId);
        }
    }

}

//---------------------------------------------------------------------------------------------------------------------
void ChangeMultipleGroupsVisibility::undo()
{
    qCDebug(vUndo, "Undo.");

    Doc()->PatternBlockMapper()->SetActiveById(m_indexPatternBlock); //Without this user will not see this change

    QMap<vidtype, bool> groupsState;

    QMap<vidtype, bool>::const_iterator i = m_oldVisibility.constBegin();
    while (i != m_oldVisibility.constEnd())
    {
        if (QDomElement group = Doc()->FindElementById(i.key(), VAbstractPattern::TagGroup); group.isElement())
        {
            Doc()->SetAttribute(group, VAbstractPattern::AttrVisible, i.value());
            groupsState.insert(i.key(), i.value());
        }
        else
        {
            qDebug("Can't get group by id = %u.", i.key());
        }
        ++i;
    }

    if (not groupsState.isEmpty())
    {
        if (QDomElement const groups = Doc()->CreateGroups(); not groups.isNull())
        {
            Doc()->ParseGroups(groups);
        }

        VMainGraphicsView::NewSceneRect(VAbstractValApplication::VApp()->getCurrentScene(),
                                        VAbstractValApplication::VApp()->getSceneView());

        emit UpdateMultipleGroups(groupsState);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ChangeMultipleGroupsVisibility::redo()
{
    qCDebug(vUndo, "ChangeMultipleGroupsVisibility::redo");

    Doc()->PatternBlockMapper()->SetActiveById(m_indexPatternBlock); //Without this user will not see this change

    QMap<vidtype, bool> groupsState;

    for (auto& groupId : m_groups)
    {
        if (QDomElement group = Doc()->FindElementById(groupId, VAbstractPattern::TagGroup); group.isElement())
        {
            Doc()->SetAttribute(group, VAbstractPattern::AttrVisible, m_newVisibility);
            groupsState.insert(groupId, m_newVisibility);
        }
        else
        {
            qDebug("Can't get group by id = %u.", groupId);
        }
    }

    if (not groupsState.isEmpty())
    {
        if (QDomElement const groups = Doc()->CreateGroups(); not groups.isNull())
        {
            Doc()->ParseGroups(groups);
        }

        VMainGraphicsView::NewSceneRect(VAbstractValApplication::VApp()->getCurrentScene(),
                                        VAbstractValApplication::VApp()->getSceneView());

        emit UpdateMultipleGroups(groupsState);
    }
}

//DelGroup
//---------------------------------------------------------------------------------------------------------------------
DelGroup::DelGroup(VAbstractPattern *doc, quint32 id, QUndoCommand *parent)
  : VUndoCommand(doc, id, parent),
    m_indexPatternBlock(doc->PatternBlockMapper()->GetActiveId())
{
    setText(tr("delete group"));
    SetElement(doc->CloneNodeById(id));
}

//---------------------------------------------------------------------------------------------------------------------
void DelGroup::undo()
{
    qCDebug(vUndo, "Undo.");

    if (VAbstractValApplication::VApp()->GetDrawMode() == Draw::Calculation)
    {
        //Without this user will not see this change
        emit Doc()->ShowPatternBlock(Doc()->PatternBlockMapper()->FindName(m_indexPatternBlock));
    }

    if (QDomElement groups = Doc()->CreateGroups(); not groups.isNull())
    {
        groups.appendChild(GetElement());
        Doc()->ParseGroups(groups);
        emit UpdateGroups();
    }
    else
    {
        qCDebug(vUndo, "Can't get tag Groups.");
        return;
    }

    VMainGraphicsView::NewSceneRect(VAbstractValApplication::VApp()->getCurrentScene(),
                                    VAbstractValApplication::VApp()->getSceneView());
}

//---------------------------------------------------------------------------------------------------------------------
void DelGroup::redo()
{
    qCDebug(vUndo, "Redo.");

    if (VAbstractValApplication::VApp()->GetDrawMode() == Draw::Calculation)
    {//Keep first!
        //Without this user will not see this change
        emit Doc()->ShowPatternBlock(Doc()->PatternBlockMapper()->FindName(m_indexPatternBlock));
    }

    if (QDomElement groups = Doc()->CreateGroups(); not groups.isNull())
    {
        if (QDomElement group = Doc()->FindElementById(ElementId(), VAbstractPattern::TagGroup); group.isElement())
        {
            group.setAttribute(VAbstractPattern::AttrVisible, trueStr);
            Doc()->ParseGroups(groups);
            if (groups.removeChild(group).isNull())
            {
                qCDebug(vUndo, "Can't delete group.");
                return;
            }
            emit UpdateGroups();

            if (groups.childNodes().isEmpty())
            {
                QDomNode parent = groups.parentNode();
                parent.removeChild(groups);
            }
        }
        else
        {
            qCDebug(vUndo, "Can't get group by id = %u.", ElementId());
            return;
        }
    }
    else
    {
        qCDebug(vUndo, "Can't get tag Groups.");
        return;
    }

    VMainGraphicsView::NewSceneRect(VAbstractValApplication::VApp()->getCurrentScene(),
                                    VAbstractValApplication::VApp()->getSceneView());
}
