/************************************************************************
 **
 **  @file   addtocalc.cpp
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

#include "addtocalc.h"

#include <QDomNode>

#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../vmisc/customevents.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "vundocommand.h"

//---------------------------------------------------------------------------------------------------------------------
AddToCalc::AddToCalc(const QDomElement &xml, VAbstractPattern *doc, QUndoCommand *parent)
  : VUndoCommand(xml, doc, parent),
    m_indexActiveBlock(doc->PatternBlockMapper()->GetActiveId())
{
    setText(tr("add object"));
    SetElementId(VAbstractPattern::GetParametrId(xml));
}

//---------------------------------------------------------------------------------------------------------------------
void AddToCalc::undo()
{
    qCDebug(vUndo, "Undo.");

    // Without this user will not see this change
    Doc()->ShowPatternBlock(Doc()->PatternBlockMapper()->FindName(m_indexActiveBlock));

    QDomElement calcElement;
    if (Doc()->GetActivNodeElement(VAbstractPattern::TagCalculation, calcElement))
    {
        QDomElement const domElement = Doc()->FindElementById(ElementId());
        if (domElement.isElement())
        {
            if (calcElement.removeChild(domElement).isNull())
            {
                qCDebug(vUndo, "Can't delete node.");
                return;
            }
        }
        else
        {
            qCDebug(vUndo, "Can't get tool by id = %u.", ElementId());
            return;
        }
    }
    else
    {
        qCDebug(vUndo, "Can't find tag Calculation.");
        return;
    }
    emit NeedFullParsing();
    VMainGraphicsView::NewSceneRect(VAbstractValApplication::VApp()->getCurrentScene(),
                                    VAbstractValApplication::VApp()->getSceneView());
    if (VAbstractValApplication::VApp()->GetDrawMode() == Draw::Calculation)
    {
        // Return current pattern piece after undo
        emit Doc()->ShowPatternBlock(Doc()->PatternBlockMapper()->FindName(m_indexActiveBlock));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void AddToCalc::redo()
{
    qCDebug(vUndo, "Redo.");

    // Without this user will not see this change
    Doc()->ShowPatternBlock(Doc()->PatternBlockMapper()->FindName(m_indexActiveBlock));

    QDomElement calcElement;
    if (Doc()->GetActivNodeElement(VAbstractPattern::TagCalculation, calcElement))
    {
        calcElement.appendChild(GetElement());
    }
    else
    {
        qCDebug(vUndo, "Can't find tag Calculation.");
        return;
    }
    RedoFullParsing();
    VMainGraphicsView::NewSceneRect(VAbstractValApplication::VApp()->getCurrentScene(),
                                    VAbstractValApplication::VApp()->getSceneView());
}

//---------------------------------------------------------------------------------------------------------------------
void AddToCalc::RedoFullParsing()
{
    if (RedoFlag())
    {
        emit NeedFullParsing();
        if (VAbstractValApplication::VApp()->GetDrawMode() == Draw::Calculation)
        {
            // Return current pattern piece after undo
            emit Doc()->ShowPatternBlock(Doc()->PatternBlockMapper()->FindName(m_indexActiveBlock));
        }
    }
    else
    {
        QApplication::postEvent(Doc(), new LiteParseEvent());
    }
    SetRedoFlag(true);
}
