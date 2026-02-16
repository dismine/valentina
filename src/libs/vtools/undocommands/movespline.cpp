/************************************************************************
 **
 **  @file   movespline.cpp
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

#include "movespline.h"

#include <QDomElement>

#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vmisc/def.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "vundocommand.h"

//---------------------------------------------------------------------------------------------------------------------
MoveSpline::MoveSpline(
    VAbstractPattern *doc, const VSpline &oldSpl, const VSpline &newSpl, const quint32 &id, QUndoCommand *parent)
  : VUndoCommand(doc, id, parent),
    oldSpline(oldSpl),
    newSpline(newSpl),
    scene(VAbstractValApplication::VApp()->getCurrentScene())
{
    setText(tr("move spline"));

    SCASSERT(scene != nullptr)
}

//---------------------------------------------------------------------------------------------------------------------
void MoveSpline::undo()
{
    qCDebug(vUndo, "Undo.");

    Do(oldSpline);
}

//---------------------------------------------------------------------------------------------------------------------
void MoveSpline::redo()
{
    qCDebug(vUndo, "Redo.");

    Do(newSpline);
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveSpline::mergeWith(const QUndoCommand *command) -> bool
{
    const auto *moveCommand = static_cast<const MoveSpline *>(command);
    SCASSERT(moveCommand != nullptr)

    if (moveCommand->ElementId() != ElementId())
    {
        return false;
    }

    newSpline = moveCommand->newSpline;
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveSpline::id() const -> int
{
    return static_cast<int>(UndoCommand::MoveSpline);
}

//---------------------------------------------------------------------------------------------------------------------
void MoveSpline::Do(const VSpline &spl)
{
    if (QDomElement domElement = Doc()->FindElementById(ElementId(), VAbstractPattern::TagSpline);
        domElement.isElement())
    {
        Doc()->SetAttribute(domElement, AttrPoint1, spl.GetP1().id());
        Doc()->SetAttribute(domElement, AttrPoint4, spl.GetP4().id());
        Doc()->SetAttribute(domElement, AttrAngle1, spl.GetStartAngleFormula());
        Doc()->SetAttribute(domElement, AttrAngle2, spl.GetEndAngleFormula());
        Doc()->SetAttribute(domElement, AttrLength1, spl.GetC1LengthFormula());
        Doc()->SetAttribute(domElement, AttrLength2, spl.GetC2LengthFormula());

        emit NeedLiteParsing(Document::LiteParse);
    }
    else
    {
        qCDebug(vUndo, "Can't find spline with id = %u.", ElementId());
    }
}
