/************************************************************************
 **
 **  @file   vabstractpoint.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   19 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "vabstractpoint.h"

#include <QSharedPointer>
#include <QUndoStack>

#include "../../../undocommands/renameobject.h"
#include "../../../undocommands/savetooloptions.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vdrawtool.h"
#include "../vgeometry/vgobject.h"
#include "../vmisc/exception/vexception.h"

//---------------------------------------------------------------------------------------------------------------------
VAbstractPoint::VAbstractPoint(VAbstractPattern *doc, VContainer *data, quint32 id, const QString &notes)
    :VDrawTool(doc, data, id, notes)
{}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPoint::getTagName() const -> QString
{
    return VAbstractPattern::TagPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPoint::DeleteFromLabel()
{
    try
    {
        DeleteToolWithConfirm();
    }
    catch(const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return;//Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPoint::UpdatePointName(
    quint32 pointId,
    const QString &name,
    const std::function<void(const QDomElement &, const QDomElement &, const ToolChanges &)> &ProcessOptions)
{
    QDomElement const oldDomElement = doc->FindElementById(m_id, getTagName());
    if (!oldDomElement.isElement())
    {
        qDebug("Can't find tool with id = %u", m_id);
        return;
    }

    if (name.isEmpty())
    {
        return; // Name is required
    }

    if (QRegularExpression const rx(NameRegExp()); !rx.match(name).hasMatch())
    {
        return; // Invalid format
    }

    if (!VAbstractTool::data.IsUnique(name))
    {
        return; // Not unique in data
    }

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(pointId);

    ToolChanges const changes = {.pointId = pointId, .oldLabel = obj->name(), .newLabel = name};

    obj->setName(name);

    QDomElement newDomElement = oldDomElement.cloneNode().toElement();
    SaveOptions(newDomElement, obj);
    ProcessOptions(oldDomElement, newDomElement, changes);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPoint::ProcessPointToolOptions(const QDomElement &oldDomElement,
                                             const QDomElement &newDomElement,
                                             const ToolChanges &changes)
{
    if (!changes.HasChanges())
    {
        VDrawTool::ApplyToolOptions(oldDomElement, newDomElement);
        return;
    }

    QUndoStack *undoStack = VAbstractApplication::VApp()->getUndoStack();
    auto *newGroup = new QUndoCommand(); // an empty command
    newGroup->setText(tr("save tool options"));

    auto *saveOptions = new SaveToolOptions(oldDomElement, newDomElement, doc, m_id, newGroup);
    saveOptions->SetInGroup(true);
    connect(saveOptions, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);

    auto *renameLabel = new RenameLabel(changes.oldLabel, changes.newLabel, doc, changes.pointId, newGroup);
    connect(renameLabel, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);

    undoStack->push(newGroup);
}
