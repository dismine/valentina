/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   17 9, 2017
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

#include "showlabel.h"

#include <QDomElement>

#include "../ifc/xml/vabstractpattern.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "../vmisc/vabstractapplication.h"
#include "../vtools/tools/drawTools/vdrawtool.h"

//---------------------------------------------------------------------------------------------------------------------
ShowLabel::ShowLabel(VAbstractPattern *doc, quint32 id, bool visible, QUndoCommand *parent)
    : VUndoCommand(QDomElement(), doc, parent),
      m_visible(visible),
      m_oldVisible(true),
      m_scene(VAbstractValApplication::VApp()->getCurrentScene())
{
    setText(tr("toggle label"));

    nodeId = id;

    QDomElement const domElement = doc->FindElementById(nodeId, VAbstractPattern::TagPoint);
    if (domElement.isElement())
    {
        m_oldVisible = VDomDocument::GetParametrBool(domElement, AttrShowLabel, trueStr);
    }
    else
    {
        qCDebug(vUndo, "Can't find point with id = %u.", nodeId);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ShowLabel::undo()
{
    qCDebug(vUndo, "Undo.");

    Do(m_oldVisible);
}

//---------------------------------------------------------------------------------------------------------------------
void ShowLabel::redo()
{
    qCDebug(vUndo, "Redo.");

    Do(m_visible);
}

//---------------------------------------------------------------------------------------------------------------------
void ShowLabel::Do(bool visible)
{
    QDomElement domElement = doc->FindElementById(nodeId, VAbstractPattern::TagPoint);
    if (domElement.isElement())
    {
        doc->SetAttribute<bool>(domElement, AttrShowLabel, visible);

        if (auto *tool = qobject_cast<VAbstractTool *>(VAbstractPattern::getTool(nodeId)))
        {
            tool->SetLabelVisible(nodeId, visible);
        }
    }
    else
    {
        qCDebug(vUndo, "Can't find point with id = %u.", nodeId);
    }
}
