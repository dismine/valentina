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
#include "../tools/vtoolseamallowance.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/def.h"
#include "../vpatterndb/vpiecenode.h"
#include "vundocommand.h"

//---------------------------------------------------------------------------------------------------------------------
SavePieceOptions::SavePieceOptions(const VPiece &oldDet, const VPiece &newDet, VAbstractPattern *doc, quint32 id,
                                   QUndoCommand *parent)
  : VUndoCommand(QDomElement(), doc, parent),
    m_oldDet(oldDet),
    m_newDet(newDet)
{
    setText(tr("save detail options"));
    nodeId = id;
}

//---------------------------------------------------------------------------------------------------------------------
void SavePieceOptions::undo()
{
    qCDebug(vUndo, "Undo.");

    QDomElement domElement = doc->elementById(nodeId, VAbstractPattern::TagDetail);
    if (domElement.isElement())
    {
        VToolSeamAllowance::AddAttributes(doc, domElement, nodeId, m_oldDet);
        VAbstractPattern::RemoveAllChildren(domElement); // Very important to clear before rewrite
        VToolSeamAllowance::AddPatternPieceData(doc, domElement, m_oldDet);
        VToolSeamAllowance::AddPatternInfo(doc, domElement, m_oldDet);
        VToolSeamAllowance::AddGrainline(doc, domElement, m_oldDet);
        VToolSeamAllowance::AddNodes(doc, domElement, m_oldDet);
        VToolSeamAllowance::AddCSARecords(doc, domElement, m_oldDet.GetCustomSARecords());
        VToolSeamAllowance::AddInternalPaths(doc, domElement, m_oldDet.GetInternalPaths());
        VToolSeamAllowance::AddPins(doc, domElement, m_oldDet.GetPins());
        VToolSeamAllowance::AddPlaceLabels(doc, domElement, m_oldDet.GetPlaceLabels());
        VToolSeamAllowance::AddMirrorLine(doc, domElement, m_oldDet);

        DecrementReferences(m_newDet.MissingNodes(m_oldDet));
        IncrementReferences(m_oldDet.MissingNodes(m_newDet));

        DecrementReferences(m_newDet.MissingCSAPath(m_oldDet));
        IncrementReferences(m_oldDet.MissingCSAPath(m_newDet));

        DecrementReferences(m_newDet.MissingInternalPaths(m_oldDet));
        IncrementReferences(m_oldDet.MissingInternalPaths(m_newDet));

        DecrementReferences(m_newDet.MissingPins(m_oldDet));
        IncrementReferences(m_oldDet.MissingPins(m_newDet));

        DecrementReferences(m_newDet.MissingPlaceLabels(m_oldDet));
        IncrementReferences(m_oldDet.MissingPlaceLabels(m_newDet));

        if (auto *tool = qobject_cast<VToolSeamAllowance *>(VAbstractPattern::getTool(nodeId)))
        {
            tool->Update(m_oldDet);
        }

        emit UpdateGroups();
    }
    else
    {
        qCDebug(vUndo, "Can't find detail with id = %u.", nodeId);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void SavePieceOptions::redo()
{
    qCDebug(vUndo, "Redo.");

    QDomElement domElement = doc->elementById(nodeId, VAbstractPattern::TagDetail);
    if (domElement.isElement())
    {
        VToolSeamAllowance::AddAttributes(doc, domElement, nodeId, m_newDet);
        VAbstractPattern::RemoveAllChildren(domElement); // Very important to clear before rewrite
        VToolSeamAllowance::AddPatternPieceData(doc, domElement, m_newDet);
        VToolSeamAllowance::AddPatternInfo(doc, domElement, m_newDet);
        VToolSeamAllowance::AddGrainline(doc, domElement, m_newDet);
        VToolSeamAllowance::AddNodes(doc, domElement, m_newDet);
        VToolSeamAllowance::AddCSARecords(doc, domElement, m_newDet.GetCustomSARecords());
        VToolSeamAllowance::AddInternalPaths(doc, domElement, m_newDet.GetInternalPaths());
        VToolSeamAllowance::AddPins(doc, domElement, m_newDet.GetPins());
        VToolSeamAllowance::AddPlaceLabels(doc, domElement, m_newDet.GetPlaceLabels());
        VToolSeamAllowance::AddMirrorLine(doc, domElement, m_newDet);

        DecrementReferences(m_oldDet.MissingNodes(m_newDet));
        IncrementReferences(m_newDet.MissingNodes(m_oldDet));

        DecrementReferences(m_oldDet.MissingCSAPath(m_newDet));
        IncrementReferences(m_newDet.MissingCSAPath(m_oldDet));

        DecrementReferences(m_oldDet.MissingInternalPaths(m_newDet));
        IncrementReferences(m_newDet.MissingInternalPaths(m_oldDet));

        DecrementReferences(m_oldDet.MissingPins(m_newDet));
        IncrementReferences(m_newDet.MissingPins(m_oldDet));

        DecrementReferences(m_oldDet.MissingPlaceLabels(m_newDet));
        IncrementReferences(m_newDet.MissingPlaceLabels(m_oldDet));

        if (auto *tool = qobject_cast<VToolSeamAllowance *>(VAbstractPattern::getTool(nodeId)))
        {
            tool->Update(m_newDet);
        }

        emit UpdateGroups();
    }
    else
    {
        qCDebug(vUndo, "Can't find detail with id = %u.", nodeId);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto SavePieceOptions::mergeWith(const QUndoCommand *command) -> bool
{
    const auto *saveCommand = static_cast<const SavePieceOptions *>(command);
    SCASSERT(saveCommand != nullptr);

    if (saveCommand->DetId() != nodeId)
    {
        return false;
    }

    const VPiece candidate = saveCommand->NewDet();

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

    m_newDet = saveCommand->NewDet();
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto SavePieceOptions::id() const -> int
{
    return static_cast<int>(UndoCommand::SavePieceOptions);
}
