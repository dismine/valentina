/************************************************************************
 **
 **  @file   moverotationlabel.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 5, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#include "operationmovelabel.h"

#include <QDomNode>
#include <QDomNodeList>

#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/ifcdef.h"
#include "../vmisc/def.h"
#include "../vundocommand.h"
#include "moveabstractlabel.h"
#include "../vtools/tools/drawTools/vdrawtool.h"

//---------------------------------------------------------------------------------------------------------------------
OperationMoveLabel::OperationMoveLabel(quint32 idTool, VAbstractPattern *doc, const QPointF &pos, quint32 idPoint,
                                       QUndoCommand *parent)
    : MoveAbstractLabel(doc, idPoint, pos, parent),
      m_idTool(idTool),
      m_scene(VAbstractValApplication::VApp()->getCurrentScene())
{
    setText(tr("move point label"));

    qCDebug(vUndo, "Tool id %u", m_idTool);

    const QDomElement element = GetDestinationObject(m_idTool, idPoint);
    if (element.isElement())
    {
        m_oldPos.rx() = VAbstractValApplication::VApp()->toPixel(VDomDocument::GetParametrDouble(element, AttrMx, "0.0"));
        m_oldPos.ry() = VAbstractValApplication::VApp()->toPixel(VDomDocument::GetParametrDouble(element, AttrMy, "0.0"));

        qCDebug(vUndo, "Label old Mx %f", m_oldPos.x());
        qCDebug(vUndo, "Label old My %f", m_oldPos.y());
    }
    else
    {
        qCDebug(vUndo, "Can't find point with id = %u.", idPoint);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto OperationMoveLabel::mergeWith(const QUndoCommand *command) -> bool
{
    const auto *moveCommand = static_cast<const OperationMoveLabel *>(command);
    SCASSERT(moveCommand != nullptr)

    if (moveCommand->m_idTool != m_idTool && moveCommand->ElementId() != ElementId())
    {
        return false;
    }

    qCDebug(vUndo, "Mergin undo.");
    m_newPos = moveCommand->m_newPos;
    qCDebug(vUndo, "Label new Mx %f", m_newPos.x());
    qCDebug(vUndo, "Label new My %f", m_newPos.y());
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto OperationMoveLabel::id() const -> int
{
    return static_cast<int>(UndoCommand::RotationMoveLabel);
}

//---------------------------------------------------------------------------------------------------------------------
void OperationMoveLabel::Do(const QPointF &pos)
{
    qCDebug(vUndo, "New mx %f", pos.x());
    qCDebug(vUndo, "New my %f", pos.y());

    QDomElement domElement = GetDestinationObject(m_idTool, ElementId());
    if (not domElement.isNull() && domElement.isElement())
    {
        Doc()->SetAttribute(domElement, AttrMx, QString().setNum(VAbstractValApplication::VApp()->fromPixel(pos.x())));
        Doc()->SetAttribute(domElement, AttrMy, QString().setNum(VAbstractValApplication::VApp()->fromPixel(pos.y())));

        if (auto *tool = qobject_cast<VDrawTool *>(VAbstractPattern::getTool(m_idTool)))
        {
            tool->ChangeLabelPosition(ElementId(), pos);
        }
    }
    else
    {
        qCDebug(vUndo, "Can't find point with id = %u.", ElementId());
    }
}
