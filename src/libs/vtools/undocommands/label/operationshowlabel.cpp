/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 9, 2017
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

#include "operationshowlabel.h"

#include <QDomElement>

#include "../ifc/xml/vabstractpattern.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "../vtools/tools/drawTools/vdrawtool.h"

//---------------------------------------------------------------------------------------------------------------------
OperationShowLabel::OperationShowLabel(
    VAbstractPattern *doc, quint32 idTool, quint32 idPoint, bool visible, QUndoCommand *parent)
  : VUndoCommand(doc, idPoint, parent),
    m_visible(visible),
    m_oldVisible(not visible),
    m_scene(VAbstractValApplication::VApp()->getCurrentScene()),
    m_idTool(idTool)
{
    qCDebug(vUndo, "Point id %u", idPoint);

    setText(tr("toggle label"));

    qCDebug(vUndo, "Tool id %u", m_idTool);

    const QDomElement element = GetDestinationObject(m_idTool, idPoint);
    if (element.isElement())
    {
        m_oldVisible = VDomDocument::GetParametrBool(element, AttrShowLabel, trueStr);
    }
    else
    {
        qCDebug(vUndo, "Can't find point with id = %u.", idPoint);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void OperationShowLabel::undo()
{
    qCDebug(vUndo, "Undo.");

    Do(m_oldVisible);
}

//---------------------------------------------------------------------------------------------------------------------
void OperationShowLabel::redo()
{
    qCDebug(vUndo, "Redo.");

    Do(m_visible);
}

//---------------------------------------------------------------------------------------------------------------------
void OperationShowLabel::Do(bool visible)
{
    QDomElement domElement = GetDestinationObject(m_idTool, ElementId());
    if (not domElement.isNull() && domElement.isElement())
    {
        Doc()->SetAttribute<bool>(domElement, AttrShowLabel, visible);

        if (auto *tool = qobject_cast<VDrawTool *>(VAbstractPattern::getTool(m_idTool)))
        {
            tool->SetLabelVisible(ElementId(), visible);
        }
        VMainGraphicsView::NewSceneRect(m_scene, VAbstractValApplication::VApp()->getSceneView());
    }
    else
    {
        qCDebug(vUndo, "Can't find point with id = %u.", ElementId());
    }
}

