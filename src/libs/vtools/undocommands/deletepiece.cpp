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

#include "deletepiece.h"

#include <QDomElement>
#include <QHash>

#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vdomdocument.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vmisc/def.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/vpiecepath.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "vundocommand.h"

//---------------------------------------------------------------------------------------------------------------------
DeletePiece::DeletePiece(
    VAbstractPattern *doc, quint32 id, const VContainer &data, VMainGraphicsScene *scene, QUndoCommand *parent)
  : VUndoCommand(doc, id, parent),
    m_detail(data.GetPiece(id)),
    m_data(data),
    m_scene(scene),
    m_record(VAbstractTool::GetRecord(id, Tool::Piece, doc))
{
    setText(tr("delete tool"));
    QDomElement const domElement = doc->FindElementById(id, VAbstractPattern::TagDetail);
    if (!domElement.isElement())
    {
        qCDebug(vUndo, "Can't get detail by id = %u.", id);
        return;
    }

    SetElement(domElement.cloneNode().toElement());
    m_parentNode = domElement.parentNode();
    QDomNode const previousDetail = domElement.previousSibling();
    if (previousDetail.isNull())
    {
        m_siblingId = NULL_ID;
    }
    else
    {
        // Better save id of previous detail instead of reference to node.
        m_siblingId = VAbstractPattern::GetParametrUInt(previousDetail.toElement(), VDomDocument::AttrId, NULL_ID_STR);
    }

    VPatternGraph const *patternGraph = doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    if (const auto node = patternGraph->GetVertex(id))
    {
        m_indexPatternBlock = node->indexPatternBlock;
    }
}

//---------------------------------------------------------------------------------------------------------------------
DeletePiece::~DeletePiece()
{
    delete m_tool.data();
}

//---------------------------------------------------------------------------------------------------------------------
void DeletePiece::undo()
{
    qCDebug(vUndo, "Undo.");

    UndoDeleteAfterSibling(m_parentNode, m_siblingId, VAbstractPattern::TagDetail);

    VAbstractPattern::AddTool(ElementId(), m_tool);
    m_data.UpdatePiece(ElementId(), m_detail);

    VPatternGraph *patternGraph = Doc()->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->AddVertex(ElementId(), VNodeType::PIECE, m_indexPatternBlock);

    const auto varData = m_data.DataDependencyVariables();
    VToolSeamAllowance::AddPieceDependencies(ElementId(), m_detail, Doc(), varData);

    m_tool->ReinitInternals(m_detail, m_scene);

    VAbstractTool::AddRecord(m_record, Doc());
    m_scene->addItem(m_tool);
    m_tool->ConnectOutsideSignals();
    m_tool->show();
    VMainGraphicsView::NewSceneRect(m_scene, VAbstractValApplication::VApp()->getSceneView(), m_tool);
    m_tool.clear();
    emit Doc()->UpdateInLayoutList();
}

//---------------------------------------------------------------------------------------------------------------------
void DeletePiece::redo()
{
    qCDebug(vUndo, "Redo.");

    QDomElement const domElement = Doc()->FindElementById(ElementId(), VAbstractPattern::TagDetail);
    if (!domElement.isElement())
    {
        qCDebug(vUndo, "Can't get detail by id = %u.", ElementId());
        return;
    }

    m_parentNode.removeChild(domElement);

    m_tool = qobject_cast<VToolSeamAllowance *>(VAbstractPattern::getTool(ElementId()));
    SCASSERT(not m_tool.isNull());
    m_tool->DisconnectOutsideSignals();
    m_tool->EnableToolMove(true);
    m_tool->hide();
    m_tool->CancelLabelRendering();

    m_scene->removeItem(m_tool);

    VAbstractPattern::RemoveTool(ElementId());
    m_data.RemovePiece(ElementId());
    Doc()->getHistory()->removeOne(m_record);

    VPatternGraph *patternGraph = Doc()->PatternGraph();
    SCASSERT(patternGraph != nullptr)
    patternGraph->RemoveVertex(ElementId());

    emit Doc()->UpdateInLayoutList();
}
