/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 11, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#include "savepiecepathoptions.h"

#include <QDebug>
#include <QDomElement>
#include <QUndoCommand>

#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../tools/nodeDetails/vtoolpiecepath.h"
#include "../tools/vtoolseamallowance.h"
#include "../vmisc/compatibility.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiecenode.h"

//---------------------------------------------------------------------------------------------------------------------
SavePiecePathOptions::SavePiecePathOptions(quint32 pieceId,
                                           VPiecePath oldPath,
                                           VPiecePath newPath,
                                           VAbstractPattern *doc,
                                           VContainer *data,
                                           quint32 id,
                                           QUndoCommand *parent)
  : VUndoCommand(doc, id, parent),
    m_oldPath(std::move(oldPath)),
    m_newPath(std::move(newPath)),
    m_data(data),
    m_pieceId(pieceId)
{
    setText(tr("save path options"));
}

//---------------------------------------------------------------------------------------------------------------------
void SavePiecePathOptions::undo()
{
    qCDebug(vUndo, "Undo.");

    QDomElement domElement = Doc()->FindElementById(ElementId(), VAbstractPattern::TagPath);
    if (!domElement.isElement())
    {
        qCDebug(vUndo, "Can't find path with id = %u.", ElementId());
        return;
    }

    VToolPiecePath::AddAttributes(Doc(), domElement, ElementId(), m_oldPath);
    VDomDocument::RemoveAllChildren(domElement); //Very important to clear before rewrite
    VToolPiecePath::AddNodes(Doc(), domElement, m_oldPath);

    VPatternGraph *patternGraph = Doc()->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->RemoveIncomingEdges(ElementId());

    DisablePieceNodes(m_newPath);
    EnablePieceNodes(m_oldPath);

    SCASSERT(m_data);

    if (m_newPath.GetType() == PiecePathType::InternalPath)
    {
        const auto varData = m_data->DataDependencyVariables();
        Doc()->FindFormulaDependencies(m_oldPath.GetVisibilityTrigger(), ElementId(), varData);
    }

    for (int i = 0; i < m_oldPath.CountNodes(); ++i)
    {
        patternGraph->AddEdge(m_oldPath.at(i).GetId(), ElementId());
    }

    m_data->UpdatePiecePath(ElementId(), m_oldPath);

    if (m_pieceId != NULL_ID)
    {
        if (auto *tool = qobject_cast<VToolSeamAllowance *>(VAbstractPattern::getTool(m_pieceId)))
        {
            tool->RefreshGeometry();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void SavePiecePathOptions::redo()
{
    qCDebug(vUndo, "Redo.");

    QDomElement domElement = Doc()->FindElementById(ElementId(), VAbstractPattern::TagPath);
    if (!domElement.isElement())
    {
        qCDebug(vUndo, "Can't find path with id = %u.", ElementId());
        return;
    }

    VToolPiecePath::AddAttributes(Doc(), domElement, ElementId(), m_newPath);
    VDomDocument::RemoveAllChildren(domElement); //Very important to clear before rewrite
    VToolPiecePath::AddNodes(Doc(), domElement, m_newPath);

    VPatternGraph *patternGraph = Doc()->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->RemoveIncomingEdges(ElementId());

    DisablePieceNodes(m_oldPath);
    EnablePieceNodes(m_newPath);

    SCASSERT(m_data);

    if (m_newPath.GetType() == PiecePathType::InternalPath)
    {
        const auto varData = m_data->DataDependencyVariables();
        Doc()->FindFormulaDependencies(m_newPath.GetVisibilityTrigger(), ElementId(), varData);
    }

    for (int i = 0; i < m_newPath.CountNodes(); ++i)
    {
        patternGraph->AddEdge(m_newPath.at(i).GetId(), ElementId());
    }

    m_data->UpdatePiecePath(ElementId(), m_newPath);

    if (m_pieceId != NULL_ID)
    {
        if (auto *tool = qobject_cast<VToolSeamAllowance *>(VAbstractPattern::getTool(m_pieceId)))
        {
            tool->RefreshGeometry();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto SavePiecePathOptions::mergeWith(const QUndoCommand *command) -> bool
{
    const auto *saveCommand = static_cast<const SavePiecePathOptions *>(command);
    SCASSERT(saveCommand != nullptr);

    if (saveCommand->ElementId() != ElementId())
    {
        return false;
    }

    const VPiecePath candidate = saveCommand->m_newPath;

    auto currentSet = ConvertToSet(m_newPath.Dependencies());
    auto candidateSet = ConvertToSet(candidate.Dependencies());

    if (currentSet != candidateSet)
    {
        return false;
    }

    const QVector<VPieceNode> nodes = m_newPath.GetNodes();
    const QVector<VPieceNode> candidateNodes = candidate.GetNodes();

    if (nodes.size() == candidateNodes.size())
    {
        return false;
    }

    for (int i = 0; i < nodes.size(); ++i)
    {
        if (nodes.at(i).IsExcluded() != candidateNodes.at(i).IsExcluded()
            || nodes.at(i).IsCheckUniqueness() != candidateNodes.at(i).IsCheckUniqueness())
        {
            return false;
        }
    }

    m_newPath = saveCommand->m_newPath;
    return true;
}
