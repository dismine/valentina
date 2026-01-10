/************************************************************************
 **
 **  @file   vabstractnode.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#include "vabstractnode.h"

#include <QSharedPointer>
#include <QUndoStack>

#include "../ifc/exception/vexceptionwrongid.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vabstracttool.h"
#include "../vmisc/def.h"
#include "../vpatterndb/vcontainer.h"

const QString VAbstractNode::AttrIdTool = QStringLiteral("idTool");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VAbstractNode constructor.
 * @param doc dom document container.
 * @param data container with variables.
 * @param id object id in container.
 * @param idNode object id in containerNode.
 * @param idTool id tool.
 * @param parent parent object.
 */
VAbstractNode::VAbstractNode(VAbstractPattern *doc,
                             VContainer *data,
                             quint32 id,
                             quint32 idNode,
                             const QString &drawName,
                             quint32 idTool,
                             QObject *parent)
  : VAbstractTool(doc, data, id, parent),
    parentType(ParentType::Item),
    idNode(idNode),
    idTool(idTool),
    m_drawName(drawName),
    m_exluded(false)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractNode::ShowVisualization(bool show)
{
    Q_UNUSED(show)
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractNode::GetParentType() const -> ParentType
{
    return parentType;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractNode::SetParentType(const ParentType &value)
{
    parentType = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractNode::GroupVisibility(quint32 object, bool visible)
{
    Q_UNUSED(object)
    Q_UNUSED(visible)
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractNode::IsExluded() const -> bool
{
    return m_exluded;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractNode::SetExluded(bool exluded)
{
    m_exluded = exluded;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractNode::IsRemovable() const -> RemoveStatus
{
    if (!doc->IsPatternGraphComplete())
    {
        return RemoveStatus::Pending; // Data not ready yet
    }

    VPatternGraph const *patternGraph = doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    auto Filter = [](const auto &) -> auto { return true; };

    auto const dependecies = patternGraph->TryGetDependentNodes(m_id, 1000, Filter);

    if (!dependecies)
    {
        return RemoveStatus::Pending; // Lock timeout
    }

    return dependecies->isEmpty() ? RemoveStatus::Removable : RemoveStatus::Blocked;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractNode::ToolCreation(const Source &typeCreation)
{
    if (typeCreation == Source::FromGui || typeCreation == Source::FromTool)
    {
        AddToFile();
    }
    else
    {
        RefreshDataInFile();
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddToModeling add tag to modeling tag current pattern peace.
 * @param domElement tag.
 */
void VAbstractNode::AddToModeling(const QDomElement &domElement)
{
    const QDomElement duplicate = doc->FindElementById(m_id);
    if (not duplicate.isNull())
    {
        throw VExceptionWrongId(tr("This id (%1) is not unique.").arg(m_id), duplicate);
    }

    QDomElement modeling;
    if (m_drawName.isEmpty())
    {
        doc->GetActivNodeElement(VAbstractPattern::TagModeling, modeling);
    }
    else
    {
        const VPatternBlockMapper *blocks = doc->PatternBlockMapper();
        modeling = blocks->GetElement(m_drawName).firstChildElement(VAbstractPattern::TagModeling);
    }
    modeling.appendChild(domElement);
}
