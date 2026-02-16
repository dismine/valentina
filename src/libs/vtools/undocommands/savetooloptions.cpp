/************************************************************************
 **
 **  @file   savetooloptions.cpp
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

#include "savetooloptions.h"

#include <QDomNode>

#include "../ifc/xml/vabstractpattern.h"
#include "vundocommand.h"

//---------------------------------------------------------------------------------------------------------------------
SaveToolOptions::SaveToolOptions(
    const QDomElement &oldXml, const QDomElement &newXml, VAbstractPattern *doc, quint32 id, QUndoCommand *parent)
  : VUndoCommand(doc, id, parent),
    oldXml(oldXml),
    newXml(newXml)
{
    setText(tr("save tool options"));
}

//---------------------------------------------------------------------------------------------------------------------
void SaveToolOptions::undo()
{
    qCDebug(vUndo, "Undo.");

    QDomElement const domElement = Doc()->FindElementById(ElementId());
    if (!domElement.isElement())
    {
        qCDebug(vUndo, "Can't find tool with id = %u.", ElementId());
        return;
    }

    domElement.parentNode().replaceChild(oldXml, domElement);

    emit NeedLiteParsing(Document::LiteParse);
}

//---------------------------------------------------------------------------------------------------------------------
void SaveToolOptions::redo()
{
    qCDebug(vUndo, "Redo.");

    QDomElement const domElement = Doc()->FindElementById(ElementId());
    if (!domElement.isElement())
    {
        qCDebug(vUndo, "Can't find tool with id = %u.", ElementId());
        return;
    }

    domElement.parentNode().replaceChild(newXml, domElement);

    if (!m_inGroup)
    {
        emit NeedLiteParsing(Document::LiteParse);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto SaveToolOptions::mergeWith(const QUndoCommand *command) -> bool
{
    const auto *saveCommand = static_cast<const SaveToolOptions *>(command);

    if (saveCommand->ElementId() != ElementId())
    {
        return false;
    }

    newXml = saveCommand->newXml;
    return true;
}
