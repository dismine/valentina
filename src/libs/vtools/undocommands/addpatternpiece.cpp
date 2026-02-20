/************************************************************************
 **
 **  @file   addpatternpiece.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 6, 2014
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

#include "addpatternpiece.h"

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "vundocommand.h"
#include "../ifc/xml/vpatternblockmapper.h"

//---------------------------------------------------------------------------------------------------------------------
AddPatternPiece::AddPatternPiece(const QDomElement &xml,
                                 VAbstractPattern *doc,
                                 int indexPatternBlock,
                                 QUndoCommand *parent)
  : VUndoCommand(xml, doc, parent),
    m_indexPatternBlock(indexPatternBlock)
{
    // cppcheck-suppress unknownMacro
    SCASSERT(m_indexPatternBlock >= 0)
    setText(tr("add pattern piece %1").arg(indexPatternBlock));
}

//---------------------------------------------------------------------------------------------------------------------
void AddPatternPiece::undo()
{
    qCDebug(vUndo, "Undo.");

    if (Doc()->CountPatternBlockTags() <= 1)
    {
        emit ClearScene();
    }
    else
    {
        QDomElement rootElement = Doc()->documentElement();
        QDomElement const patternBlock = Doc()->PatternBlockMapper()->GetElementById(m_indexPatternBlock);
        rootElement.removeChild(patternBlock);
        emit NeedFullParsing();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AddPatternPiece::redo()
{
    qCDebug(vUndo, "Redo.");

    if (Doc()->CountPatternBlockTags() == 0)
    {
        Doc()->CreateEmptyFile();
    }

    QDomElement rootElement = Doc()->documentElement();

    rootElement.appendChild(GetElement());

    RedoFullParsing();
}
