/************************************************************************
 **
 **  @file   saveplacelabeloptions.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 10, 2017
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
#include "saveplacelabeloptions.h"

#include "../ifc/xml/vpatterngraph.h"
#include "../tools/nodeDetails/vtoolplacelabel.h"
#include "../tools/vtoolseamallowance.h"

//---------------------------------------------------------------------------------------------------------------------
SavePlaceLabelOptions::SavePlaceLabelOptions(quint32 pieceId,
                                             VPlaceLabelItem oldLabel,
                                             VPlaceLabelItem newLabel,
                                             VAbstractPattern *doc,
                                             VContainer *data,
                                             quint32 id,
                                             QUndoCommand *parent)
  : VUndoCommand(doc, parent),
    m_oldLabel(std::move(oldLabel)),
    m_newLabel(std::move(newLabel)),
    m_data(data),
    m_pieceId(pieceId)
{
    setText(tr("save place label options"));
    nodeId = id;
}

//---------------------------------------------------------------------------------------------------------------------
void SavePlaceLabelOptions::undo()
{
    qCDebug(vUndo, "Undo.");

    QDomElement domElement = doc->FindElementById(nodeId, VAbstractPattern::TagPoint);
    if (!domElement.isElement())
    {
        qCDebug(vUndo, "Can't find place label with id = %u.", nodeId);
        return;
    }

    VToolPlaceLabel::AddAttributes(doc, domElement, nodeId, m_oldLabel);

    VPatternGraph *patternGraph = doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->RemoveIncomingEdges(nodeId);
    patternGraph->AddEdge(m_oldLabel.GetCenterPoint(), nodeId);

    SCASSERT(m_data);
    const auto varData = m_data->DataDependencyVariables();
    doc->FindFormulaDependencies(m_oldLabel.GetWidthFormula(), nodeId, varData);
    doc->FindFormulaDependencies(m_oldLabel.GetHeightFormula(), nodeId, varData);
    doc->FindFormulaDependencies(m_oldLabel.GetAngleFormula(), nodeId, varData);
    doc->FindFormulaDependencies(m_oldLabel.GetVisibilityTrigger(), nodeId, varData);

    m_data->UpdateGObject(nodeId, new VPlaceLabelItem(m_oldLabel));

    if (m_pieceId != NULL_ID)
    {
        if (auto *tool = qobject_cast<VToolSeamAllowance *>(VAbstractPattern::getTool(m_pieceId)))
        {
            tool->RefreshGeometry();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void SavePlaceLabelOptions::redo()
{
    qCDebug(vUndo, "Redo.");

    QDomElement domElement = doc->FindElementById(nodeId, VAbstractPattern::TagPoint);
    if (!domElement.isElement())
    {
        qCDebug(vUndo, "Can't find path with id = %u.", nodeId);
        return;
    }

    VToolPlaceLabel::AddAttributes(doc, domElement, nodeId, m_newLabel);

    VPatternGraph *patternGraph = doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->RemoveIncomingEdges(nodeId);
    patternGraph->AddEdge(m_newLabel.GetCenterPoint(), nodeId);

    SCASSERT(m_data);
    const auto varData = m_data->DataDependencyVariables();
    doc->FindFormulaDependencies(m_newLabel.GetWidthFormula(), nodeId, varData);
    doc->FindFormulaDependencies(m_newLabel.GetHeightFormula(), nodeId, varData);
    doc->FindFormulaDependencies(m_newLabel.GetAngleFormula(), nodeId, varData);
    doc->FindFormulaDependencies(m_newLabel.GetVisibilityTrigger(), nodeId, varData);

    m_data->UpdateGObject(nodeId, new VPlaceLabelItem(m_newLabel));

    if (m_pieceId != NULL_ID)
    {
        if (auto *tool = qobject_cast<VToolSeamAllowance *>(VAbstractPattern::getTool(m_pieceId)))
        {
            tool->RefreshGeometry();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto SavePlaceLabelOptions::mergeWith(const QUndoCommand *command) -> bool
{
    const auto *saveCommand = static_cast<const SavePlaceLabelOptions *>(command);
    SCASSERT(saveCommand != nullptr);

    if (saveCommand->LabelId() != nodeId || m_newLabel.GetCenterPoint() != saveCommand->NewLabel().GetCenterPoint())
    {
        return false;
    }

    m_newLabel = saveCommand->NewLabel();
    return true;
}
