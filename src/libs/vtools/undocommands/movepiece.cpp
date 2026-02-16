/************************************************************************
 **
 **  @file   movedetail.cpp
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

#include "movepiece.h"

#include <QDomElement>

#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../tools/vtoolseamallowance.h"
#include "../vmisc/def.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "vundocommand.h"

//---------------------------------------------------------------------------------------------------------------------
MovePiece::MovePiece(VAbstractPattern *doc,
                     const double &x,
                     const double &y,
                     const quint32 &id,
                     QGraphicsScene *scene,
                     QUndoCommand *parent)
  : VUndoCommand(doc, id, parent),
    m_oldX(0.0),
    m_oldY(0.0),
    m_newX(x),
    m_newY(y),
    m_scene(scene)
{
    setText(QObject::tr("move detail"));

    SCASSERT(scene != nullptr)
    QDomElement const domElement = doc->FindElementById(id, VAbstractPattern::TagDetail);
    if (domElement.isElement())
    {
        m_oldX = VAbstractValApplication::VApp()->toPixel(VDomDocument::GetParametrDouble(domElement, AttrMx, "0.0"));
        m_oldY = VAbstractValApplication::VApp()->toPixel(VDomDocument::GetParametrDouble(domElement, AttrMy, "0.0"));
    }
    else
    {
        qCDebug(vUndo, "Can't find detail with id = %u.", id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MovePiece::undo()
{
    qCDebug(vUndo, "Undo.");
    Do(m_oldX, m_oldY);
}

//---------------------------------------------------------------------------------------------------------------------
void MovePiece::redo()
{
    qCDebug(vUndo, "Redo.");
    Do(m_newX, m_newY);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
auto MovePiece::mergeWith(const QUndoCommand *command) -> bool
{
    const auto *moveCommand = static_cast<const MovePiece *>(command);
    SCASSERT(moveCommand != nullptr)

    if (moveCommand->ElementId() != ElementId())
    {
        return false;
    }

    m_newX = moveCommand->m_newX;
    m_newY = moveCommand->m_newY;
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto MovePiece::id() const -> int
{
    return static_cast<int>(UndoCommand::MovePiece);
}

//---------------------------------------------------------------------------------------------------------------------
void MovePiece::Do(qreal x, qreal y)
{
    qCDebug(vUndo, "Do.");

    QDomElement domElement = Doc()->FindElementById(ElementId(), VAbstractPattern::TagDetail);
    if (domElement.isElement())
    {
        SaveCoordinates(domElement, x, y);

        auto *tool = qobject_cast<VToolSeamAllowance *>(VAbstractPattern::getTool(ElementId()));
        if (tool != nullptr)
        {
            tool->Move(x, y);
        }
        VMainGraphicsView::NewSceneRect(m_scene, VAbstractValApplication::VApp()->getSceneView(), tool);
    }
    else
    {
        qCDebug(vUndo, "Can't find detail with id = %u.", ElementId());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MovePiece::SaveCoordinates(QDomElement &domElement, double x, double y)
{
    Doc()->SetAttribute(domElement, AttrMx, QString().setNum(VAbstractValApplication::VApp()->fromPixel(x)));
    Doc()->SetAttribute(domElement, AttrMy, QString().setNum(VAbstractValApplication::VApp()->fromPixel(y)));
}
