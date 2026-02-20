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
  : VUndoCommand(doc, id, parent),
    m_oldLabel(std::move(oldLabel)),
    m_newLabel(std::move(newLabel)),
    m_data(data),
    m_pieceId(pieceId)
{
    setText(tr("save place label options"));
}

//---------------------------------------------------------------------------------------------------------------------
void SavePlaceLabelOptions::undo()
{
    qCDebug(vUndo, "Undo.");

    QDomElement domElement = Doc()->FindElementById(ElementId(), VAbstractPattern::TagPoint);
    if (!domElement.isElement())
    {
        qCDebug(vUndo, "Can't find place label with id = %u.", ElementId());
        return;
    }

    VToolPlaceLabel::AddAttributes(Doc(), domElement, ElementId(), m_oldLabel);

    VPatternGraph *patternGraph = Doc()->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->RemoveIncomingEdges(ElementId());
    patternGraph->AddEdge(m_oldLabel.GetCenterPoint(), ElementId());

    SCASSERT(m_data);
    const auto varData = m_data->DataDependencyVariables();
    Doc()->FindFormulaDependencies(m_oldLabel.GetWidthFormula(), ElementId(), varData);
    Doc()->FindFormulaDependencies(m_oldLabel.GetHeightFormula(), ElementId(), varData);
    Doc()->FindFormulaDependencies(m_oldLabel.GetAngleFormula(), ElementId(), varData);
    Doc()->FindFormulaDependencies(m_oldLabel.GetVisibilityTrigger(), ElementId(), varData);

    m_data->UpdateGObject(ElementId(), new VPlaceLabelItem(m_oldLabel));

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

    QDomElement domElement = Doc()->FindElementById(ElementId(), VAbstractPattern::TagPoint);
    if (!domElement.isElement())
    {
        qCDebug(vUndo, "Can't find path with id = %u.", ElementId());
        return;
    }

    VToolPlaceLabel::AddAttributes(Doc(), domElement, ElementId(), m_newLabel);

    VPatternGraph *patternGraph = Doc()->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->RemoveIncomingEdges(ElementId());
    patternGraph->AddEdge(m_newLabel.GetCenterPoint(), ElementId());

    SCASSERT(m_data);
    const auto varData = m_data->DataDependencyVariables();
    Doc()->FindFormulaDependencies(m_newLabel.GetWidthFormula(), ElementId(), varData);
    Doc()->FindFormulaDependencies(m_newLabel.GetHeightFormula(), ElementId(), varData);
    Doc()->FindFormulaDependencies(m_newLabel.GetAngleFormula(), ElementId(), varData);
    Doc()->FindFormulaDependencies(m_newLabel.GetVisibilityTrigger(), ElementId(), varData);

    m_data->UpdateGObject(ElementId(), new VPlaceLabelItem(m_newLabel));

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

    if (saveCommand->ElementId() != ElementId()
        || m_newLabel.GetCenterPoint() != saveCommand->m_newLabel.GetCenterPoint())
    {
        return false;
    }

    m_newLabel = saveCommand->m_newLabel;
    return true;
}
