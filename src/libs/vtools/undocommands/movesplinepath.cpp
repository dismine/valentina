/************************************************************************
 **
 **  @file   movesplinepath.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 6, 2014
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

#include "movesplinepath.h"

#include <QDomElement>

#include "../ifc/xml/vabstractpattern.h"
#include "../tools/drawTools/toolcurve/vtoolsplinepath.h"
#include "../vmisc/def.h"
#include "../vgeometry/vsplinepath.h"
#include "vundocommand.h"

//---------------------------------------------------------------------------------------------------------------------
MoveSplinePath::MoveSplinePath(VAbstractPattern *doc,
                               const VSplinePath &oldSplPath,
                               const VSplinePath &newSplPath,
                               const quint32 &id,
                               QUndoCommand *parent)
  : VUndoCommand(doc, id, parent),
    oldSplinePath(oldSplPath),
    newSplinePath(newSplPath),
    scene(VAbstractValApplication::VApp()->getCurrentScene())
{
    setText(tr("move spline path"));

    SCASSERT(scene != nullptr)
}

//---------------------------------------------------------------------------------------------------------------------
void MoveSplinePath::undo()
{
    qCDebug(vUndo, "Undo.");

    Do(oldSplinePath);
}

//---------------------------------------------------------------------------------------------------------------------
void MoveSplinePath::redo()
{
    qCDebug(vUndo, "Redo.");

    Do(newSplinePath);
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveSplinePath::mergeWith(const QUndoCommand *command) -> bool
{
    const auto *moveCommand = static_cast<const MoveSplinePath *>(command);
    SCASSERT(moveCommand != nullptr)

    if (moveCommand->ElementId() != ElementId())
    {
        return false;
    }

    newSplinePath = moveCommand->newSplinePath;
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveSplinePath::id() const -> int
{
    return static_cast<int>(UndoCommand::MoveSplinePath);
}

//---------------------------------------------------------------------------------------------------------------------
void MoveSplinePath::Do(const VSplinePath &splPath)
{
    QDomElement domElement = Doc()->FindElementById(ElementId(), VAbstractPattern::TagSpline);
    if (domElement.isElement())
    {
        VToolSplinePath::UpdatePathPoints(Doc(), domElement, splPath);

        emit NeedLiteParsing(Document::LiteParse);
    }
    else
    {
        qCDebug(vUndo, "Can't find spline path with id = %u.", ElementId());
    }
}
