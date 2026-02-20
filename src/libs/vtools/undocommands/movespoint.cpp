/************************************************************************
 **
 **  @file   movespoint.cpp
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

#include "movespoint.h"

#include <QDomElement>

#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/ifcdef.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vmisc/def.h"
#include "vundocommand.h"

//---------------------------------------------------------------------------------------------------------------------
MoveSPoint::MoveSPoint(VAbstractPattern *doc,
                       const double &x,
                       const double &y,
                       const quint32 &id,
                       QGraphicsScene *scene,
                       QUndoCommand *parent)
  : VUndoCommand(doc, id, parent),
    oldX(0.0),
    oldY(0.0),
    newX(x),
    newY(y),
    scene(scene)
{
    setText(tr("move single point"));
    qCDebug(vUndo, "SPoint id %u", id);

    qCDebug(vUndo, "SPoint newX %f", newX);
    qCDebug(vUndo, "SPoint newY %f", newY);

    SCASSERT(scene != nullptr)
    QDomElement const domElement = doc->FindElementById(id, VAbstractPattern::TagPoint);
    if (domElement.isElement())
    {
        oldX = VAbstractValApplication::VApp()->toPixel(VDomDocument::GetParametrDouble(domElement, AttrX, "0.0"));
        oldY = VAbstractValApplication::VApp()->toPixel(VDomDocument::GetParametrDouble(domElement, AttrY, "0.0"));

        qCDebug(vUndo, "SPoint oldX %f", oldX);
        qCDebug(vUndo, "SPoint oldY %f", oldY);
    }
    else
    {
        qCDebug(vUndo, "Can't find spoint with id = %u.", id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MoveSPoint::undo()
{
    qCDebug(vUndo, "Undo.");

    Do(oldX, oldY);
}

//---------------------------------------------------------------------------------------------------------------------
void MoveSPoint::redo()
{
    qCDebug(vUndo, "Redo.");

    Do(newX, newY);
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveSPoint::mergeWith(const QUndoCommand *command) -> bool
{
    const auto *moveCommand = static_cast<const MoveSPoint *>(command);
    SCASSERT(moveCommand != nullptr)

    qCDebug(vUndo, "Mergin.");
    if (moveCommand->ElementId() != ElementId())
    {
        qCDebug(vUndo, "Merging canceled.");
        return false;
    }

    qCDebug(vUndo, "Mergin undo.");
    newX = moveCommand->newX;
    newY = moveCommand->newY;
    qCDebug(vUndo, "SPoint newX %f", newX);
    qCDebug(vUndo, "SPoint newY %f", newY);
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveSPoint::id() const -> int
{
    return static_cast<int>(UndoCommand::MoveSPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void MoveSPoint::Do(double x, double y)
{
    qCDebug(vUndo, "Move to x %f", x);
    qCDebug(vUndo, "Move to y %f", y);

    QDomElement domElement = Doc()->FindElementById(ElementId(), VAbstractPattern::TagPoint);
    if (domElement.isElement())
    {
        Doc()->SetAttribute(domElement, AttrX, QString().setNum(VAbstractValApplication::VApp()->fromPixel(x)));
        Doc()->SetAttribute(domElement, AttrY, QString().setNum(VAbstractValApplication::VApp()->fromPixel(y)));

        emit NeedLiteParsing(Document::LitePPParse);
    }
    else
    {
        qCDebug(vUndo, "Can't find spoint with id = %u.", ElementId());
    }
}
