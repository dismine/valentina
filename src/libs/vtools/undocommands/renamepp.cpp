/************************************************************************
 **
 **  @file   renamepp.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   17 7, 2014
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

#include "renamepp.h"

#include <utility>
#include <QComboBox>
#include <QDomElement>

#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../vmisc/def.h"
#include "vundocommand.h"

//---------------------------------------------------------------------------------------------------------------------
RenamePP::RenamePP(VAbstractPattern *doc, QString newPatternBlockName, QComboBox *combo, QUndoCommand *parent)
  : VUndoCommand(doc, parent),
    combo(combo),
    m_newPatternBlockName(std::move(newPatternBlockName)),
    m_oldPatternBlockName(doc->PatternBlockMapper()->GetActive())
{
    setText(tr("rename pattern block"));
    SCASSERT(combo != nullptr)
}

//---------------------------------------------------------------------------------------------------------------------
void RenamePP::undo()
{
    qCDebug(vUndo, "Undo.");

    ChangeName(m_newPatternBlockName, m_oldPatternBlockName);
}

//---------------------------------------------------------------------------------------------------------------------
void RenamePP::redo()
{
    qCDebug(vUndo, "Redo.");

    ChangeName(m_oldPatternBlockName, m_newPatternBlockName);
}

//---------------------------------------------------------------------------------------------------------------------
auto RenamePP::mergeWith(const QUndoCommand *command) -> bool
{
    const auto *renameCommand = static_cast<const RenamePP *>(command);
    SCASSERT(renameCommand != nullptr)

    if (m_newPatternBlockName != renameCommand->GetOldPatternBlockName())
    {
        return false;
    }

    m_newPatternBlockName = renameCommand->GetNewPatternBlockName();
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto RenamePP::id() const -> int
{
    return static_cast<int>(UndoCommand::RenamePP);
}

//---------------------------------------------------------------------------------------------------------------------
void RenamePP::ChangeName(const QString &oldName, const QString &newName)
{
    if (Doc()->PatternBlockMapper()->Rename(oldName, newName))
    {
        combo->setItemText(combo->findText(oldName), newName);
    }
    else
    {
        qCWarning(vUndo, "Can't change pattern piece name");
    }
}
