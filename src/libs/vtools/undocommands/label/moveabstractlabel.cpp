/************************************************************************
 **
 **  @file   moveabstractlabel.cpp
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

#include "moveabstractlabel.h"

#include <QDomElement>

#include "../vundocommand.h"

//---------------------------------------------------------------------------------------------------------------------
MoveAbstractLabel::MoveAbstractLabel(
    VAbstractPattern *doc, quint32 pointId, const QPointF &oldPos, const QPointF &newPos, QUndoCommand *parent)
  : VUndoCommand(doc, pointId, parent),
    m_oldPos(oldPos),
    m_newPos(newPos)
{
    qCDebug(vUndo, "Point id %u  old(%f;%f) → new(%f;%f)", pointId, oldPos.x(), oldPos.y(), newPos.x(), newPos.y());
}

//---------------------------------------------------------------------------------------------------------------------
void MoveAbstractLabel::undo()
{
    qCDebug(vUndo, "Undo.");
    WritePos(m_oldPos);
}

//---------------------------------------------------------------------------------------------------------------------
void MoveAbstractLabel::redo()
{
    qCDebug(vUndo, "Redo.");
    WritePos(m_newPos);
}
