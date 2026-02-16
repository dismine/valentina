/************************************************************************
 **
 **  @file   deletepatternpiece.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 6, 2014
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

#include "deletepatternpiece.h"

#include <QDomNode>
#include <QDomNodeList>

#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../vmisc/vabstractvalapplication.h"
#include "vundocommand.h"

//---------------------------------------------------------------------------------------------------------------------
DeletePatternPiece::DeletePatternPiece(VAbstractPattern *doc, int indexPatternBlock, QUndoCommand *parent)
  : VUndoCommand(doc, parent),
    m_indexPatternBlock(indexPatternBlock)
{
    SCASSERT(m_indexPatternBlock >= 0)
    setText(tr("delete pattern block"));

    const QDomElement patternBlockElement = doc->PatternBlockMapper()->GetElementById(m_indexPatternBlock);
    m_patternBlock = patternBlockElement.cloneNode().toElement();

    if (const QDomElement previousPatternBlockElement = patternBlockElement.previousSibling().toElement();
        not previousPatternBlockElement.isNull() && previousPatternBlockElement.tagName() == VAbstractPattern::TagDraw)
    {
        m_previousPatternBlockName = VAbstractPattern::GetParametrString(previousPatternBlockElement,
                                                                         VAbstractPattern::AttrName,
                                                                         QString());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DeletePatternPiece::undo()
{
    qCDebug(vUndo, "Undo.");

    const VPatternBlockMapper *blocks = Doc()->PatternBlockMapper();
    QDomElement rootElement = Doc()->documentElement();

    if (!m_previousPatternBlockName.isEmpty())
    { // not first in the list, add after tag draw
        const QDomNode previousPatternBlock = blocks->GetElement(m_previousPatternBlockName);
        rootElement.insertAfter(m_patternBlock, previousPatternBlock);
    }
    else
    { // first in the list, add before tag draw
        QDomElement draw;

        if (const QDomNodeList list = rootElement.elementsByTagName(VAbstractPattern::TagDraw); not list.isEmpty())
        {
            draw = list.at(0).toElement();
        }

        Q_ASSERT_X(not draw.isNull(), Q_FUNC_INFO, "Couldn't' find tag draw");
        rootElement.insertBefore(m_patternBlock, draw);
    }

    emit NeedFullParsing();
    if (VAbstractValApplication::VApp()->GetDrawMode() == Draw::Calculation)
    {
        emit Doc()->ShowPatternBlock(
            blocks->FindName(m_indexPatternBlock)); // Without this user will not see this change
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DeletePatternPiece::redo()
{
    qCDebug(vUndo, "Redo.");

    const VPatternBlockMapper *blocks = Doc()->PatternBlockMapper();

    if (VAbstractValApplication::VApp()->GetDrawMode() == Draw::Calculation)
    {
        emit Doc()->ShowPatternBlock(
            blocks->FindName(m_indexPatternBlock)); // Without this user will not see this change
    }

    QDomElement rootElement = Doc()->documentElement();
    const QDomElement patternPieceElement = blocks->GetElementById(m_indexPatternBlock);
    rootElement.removeChild(patternPieceElement);
    emit NeedFullParsing();
}
