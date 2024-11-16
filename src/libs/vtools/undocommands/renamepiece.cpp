/************************************************************************
 **
 **  @file   renamepiece.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 11, 2024
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2024 Valentina project
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

#include <utility>

#include "../tools/vtoolseamallowance.h"
#include "../vmisc/vabstractvalapplication.h"
#include "renamepiece.h"

//---------------------------------------------------------------------------------------------------------------------
RenamePiece::RenamePiece(VAbstractPattern *doc, QString newName, quint32 id, QUndoCommand *parent)
  : VUndoCommand(QDomElement(), doc, parent),
    m_newName(std::move(newName))
{
    setText(QObject::tr("rename detail"));
    nodeId = id;

    QDomElement const domElement = doc->elementById(id, VAbstractPattern::TagDetail);
    if (domElement.isElement())
    {
        m_oldName = VAbstractPattern::GetParametrString(domElement, AttrName, tr("Detail"));
    }
    else
    {
        qCDebug(vUndo, "Can't find detail with id = %u.", nodeId);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void RenamePiece::undo()
{
    qCDebug(vUndo, "Undo.");
    Do(m_oldName);
}

//---------------------------------------------------------------------------------------------------------------------
void RenamePiece::redo()
{
    qCDebug(vUndo, "Redo.");
    Do(m_newName);
}

//---------------------------------------------------------------------------------------------------------------------
auto RenamePiece::mergeWith(const QUndoCommand *command) -> bool
{
    const auto *renameCommand = static_cast<const RenamePiece *>(command);
    SCASSERT(renameCommand != nullptr)
    const quint32 id = renameCommand->getDetId();

    if (id != nodeId)
    {
        return false;
    }

    m_newName = renameCommand->getNewName();
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto RenamePiece::id() const -> int
{
    return static_cast<int>(UndoCommand::RenamePiece);
}

//---------------------------------------------------------------------------------------------------------------------
void RenamePiece::Do(const QString &name)
{
    qCDebug(vUndo, "Do.");

    if (QDomElement domElement = doc->elementById(nodeId, VAbstractPattern::TagDetail); domElement.isElement())
    {
        doc->SetAttribute(domElement, AttrName, name);

        if (auto *tool = qobject_cast<VToolSeamAllowance *>(VAbstractPattern::getTool(nodeId)); tool != nullptr)
        {
            tool->SetName(name);
        }

        emit UpdateList();
    }
    else
    {
        qCDebug(vUndo, "Can't find detail with id = %u.", nodeId);
    }
}
