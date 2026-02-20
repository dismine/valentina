/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 11, 2016
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

#include "savepieceoptions.h"

#include <QDebug>
#include <QDomElement>
#include <QPointF>
#include <QUndoCommand>

#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../tools/vtoolseamallowance.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/def.h"
#include "../vpatterndb/vpiecenode.h"

//---------------------------------------------------------------------------------------------------------------------
SavePieceOptions::SavePieceOptions(VPiece oldDet, VPiece newDet, VAbstractPattern *doc, quint32 id, QUndoCommand *parent)
  : VUndoCommand(doc, id, parent),
    m_oldDet(std::move(oldDet)),
    m_newDet(std::move(newDet))
{
    setText(tr("save detail options"));
}

//---------------------------------------------------------------------------------------------------------------------
void SavePieceOptions::undo()
{
    qCDebug(vUndo, "Undo.");

    QDomElement domElement = Doc()->FindElementById(ElementId(), VAbstractPattern::TagDetail);
    if (!domElement.isElement())
    {
        qCDebug(vUndo, "Can't find detail with id = %u.", ElementId());
        return;
    }

    VToolSeamAllowance::AddAttributes(Doc(), domElement, ElementId(), m_oldDet);
    VAbstractPattern::RemoveAllChildren(domElement); // Very important to clear before rewrite
    VToolSeamAllowance::AddPatternPieceData(Doc(), domElement, m_oldDet);
    VToolSeamAllowance::AddPatternInfo(Doc(), domElement, m_oldDet);
    VToolSeamAllowance::AddGrainline(Doc(), domElement, m_oldDet);
    VToolSeamAllowance::AddNodes(Doc(), domElement, m_oldDet);
    VToolSeamAllowance::AddCSARecords(Doc(), domElement, m_oldDet.GetCustomSARecords());
    VToolSeamAllowance::AddInternalPaths(Doc(), domElement, m_oldDet.GetInternalPaths());
    VToolSeamAllowance::AddPins(Doc(), domElement, m_oldDet.GetPins());
    VToolSeamAllowance::AddPlaceLabels(Doc(), domElement, m_oldDet.GetPlaceLabels());
    VToolSeamAllowance::AddMirrorLine(Doc(), domElement, m_oldDet);

    VPatternGraph *patternGraph = Doc()->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->RemoveIncomingEdges(ElementId());

    auto *tool = qobject_cast<VToolSeamAllowance *>(VAbstractPattern::getTool(ElementId()));
    SCASSERT(tool != nullptr)

    const auto varData = tool->getData()->DataDependencyVariables();
    VToolSeamAllowance::AddPieceDependencies(ElementId(), m_oldDet, Doc(), varData);

    DisablePieceNodes(m_newDet.GetPath());
    EnablePieceNodes(m_oldDet.GetPath());

    // Just disable is enough here. Piece will reactivate active paths
    DisableInternalPaths(m_newDet.GetInternalPaths());

    tool->Update(m_oldDet);

    emit UpdateGroups();
}

//---------------------------------------------------------------------------------------------------------------------
void SavePieceOptions::redo()
{
    qCDebug(vUndo, "Redo.");

    QDomElement domElement = Doc()->FindElementById(ElementId(), VAbstractPattern::TagDetail);
    if (!domElement.isElement())
    {
        qCDebug(vUndo, "Can't find detail with id = %u.", ElementId());
        return;
    }

    VToolSeamAllowance::AddAttributes(Doc(), domElement, ElementId(), m_newDet);
    VAbstractPattern::RemoveAllChildren(domElement); // Very important to clear before rewrite
    VToolSeamAllowance::AddPatternPieceData(Doc(), domElement, m_newDet);
    VToolSeamAllowance::AddPatternInfo(Doc(), domElement, m_newDet);
    VToolSeamAllowance::AddGrainline(Doc(), domElement, m_newDet);
    VToolSeamAllowance::AddNodes(Doc(), domElement, m_newDet);
    VToolSeamAllowance::AddCSARecords(Doc(), domElement, m_newDet.GetCustomSARecords());
    VToolSeamAllowance::AddInternalPaths(Doc(), domElement, m_newDet.GetInternalPaths());
    VToolSeamAllowance::AddPins(Doc(), domElement, m_newDet.GetPins());
    VToolSeamAllowance::AddPlaceLabels(Doc(), domElement, m_newDet.GetPlaceLabels());
    VToolSeamAllowance::AddMirrorLine(Doc(), domElement, m_newDet);

    VPatternGraph *patternGraph = Doc()->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->RemoveIncomingEdges(ElementId());

    auto *tool = qobject_cast<VToolSeamAllowance *>(VAbstractPattern::getTool(ElementId()));
    SCASSERT(tool != nullptr)

    const auto varData = tool->getData()->DataDependencyVariables();
    VToolSeamAllowance::AddPieceDependencies(ElementId(), m_newDet, Doc(), varData);

    DisablePieceNodes(m_oldDet.GetPath());
    EnablePieceNodes(m_newDet.GetPath());

    // Just disable is enough here. Piece will reactivate active paths
    DisableInternalPaths(m_oldDet.GetInternalPaths());

    tool->Update(m_newDet);

    emit UpdateGroups();
}

//---------------------------------------------------------------------------------------------------------------------
auto SavePieceOptions::mergeWith(const QUndoCommand *command) -> bool
{
    const auto *saveCommand = static_cast<const SavePieceOptions *>(command);
    SCASSERT(saveCommand != nullptr);

    if (saveCommand->ElementId() != ElementId())
    {
        return false;
    }

    const VPiece candidate = saveCommand->m_newDet;

    auto currentSet = ConvertToSet(m_newDet.Dependencies());
    auto candidateSet = ConvertToSet(candidate.Dependencies());

    if (currentSet != candidateSet)
    {
        return false;
    }

    const QVector<VPieceNode> nodes = m_newDet.GetPath().GetNodes();
    const QVector<VPieceNode> candidateNodes = candidate.GetPath().GetNodes();

    if (nodes.size() != candidateNodes.size())
    {
        return false;
    }

    for (int i = 0; i < nodes.size(); ++i)
    {
        if (nodes.at(i).IsExcluded() != candidateNodes.at(i).IsExcluded() ||
            nodes.at(i).IsCheckUniqueness() != candidateNodes.at(i).IsCheckUniqueness() ||
            nodes.at(i).IsPassmark() != candidateNodes.at(i).IsPassmark())
        {
            return false;
        }
    }

    m_newDet = saveCommand->m_newDet;
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto SavePieceOptions::id() const -> int
{
    return static_cast<int>(UndoCommand::SavePieceOptions);
}
