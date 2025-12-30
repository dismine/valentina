/************************************************************************
 **
 **  @file   vundocommand.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 7, 2014
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

#include "vundocommand.h"

#include <QApplication>
#include <QDomNode>

#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../tools/drawTools/operation/vabstractoperation.h"
#include "../tools/nodeDetails/vnodepoint.h"
#include "../tools/nodeDetails/vtoolpiecepath.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/customevents.h"
#include "../vmisc/def.h"
#include "../vpatterndb/vpiecenode.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(vUndo, "v.undo") // NOLINT

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
VUndoCommand::VUndoCommand(const QDomElement &xml, VAbstractPattern *doc, QUndoCommand *parent)
  : QObject(),
    QUndoCommand(parent),
    xml(xml),
    doc(doc),
    nodeId(NULL_ID),
    redoFlag(false)
{
    SCASSERT(doc != nullptr)
}

//---------------------------------------------------------------------------------------------------------------------
void VUndoCommand::RedoFullParsing()
{
    if (redoFlag)
    {
        emit NeedFullParsing();
    }
    else
    {
        QApplication::postEvent(doc, new LiteParseEvent());
    }
    redoFlag = true;
}

//---------------------------------------------------------------------------------------------------------------------
void VUndoCommand::UndoDeleteAfterSibling(QDomNode &parentNode, quint32 siblingId, const QString &tagName) const
{
    if (siblingId == NULL_ID)
    {
        parentNode.appendChild(xml);
    }
    else
    {
        const QDomElement refElement = doc->NodeById(siblingId, tagName);
        parentNode.insertAfter(xml, refElement);
        doc->RefreshElementIdCache();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VUndoCommand::AddEdges(const QVector<quint32> &nodes) const
{
    VPatternGraph *patternGraph = doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    for (auto id : nodes)
    {
        patternGraph->AddEdge(id, nodeId);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VUndoCommand::RemoveEdges(const QVector<quint32> &nodes) const
{
    VPatternGraph *patternGraph = doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    for (auto id : nodes)
    {
        patternGraph->RemoveEdge(id, nodeId);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VUndoCommand::AddEdges(const QVector<CustomSARecord> &nodes) const
{
    QVector<quint32> n;
    n.reserve(nodes.size());

    for (const auto &node : nodes)
    {
        n.append(node.path);
    }

    AddEdges(n);
}

//---------------------------------------------------------------------------------------------------------------------
void VUndoCommand::RemoveEdges(const QVector<CustomSARecord> &nodes) const
{
    QVector<quint32> n;
    n.reserve(nodes.size());

    for (const auto &node : nodes)
    {
        n.append(node.path);
    }

    RemoveEdges(n);
}

//---------------------------------------------------------------------------------------------------------------------
void VUndoCommand::AddEdges(const QVector<VPieceNode> &nodes) const
{
    QVector<quint32> n;
    n.reserve(nodes.size());

    for (const auto &node : nodes)
    {
        n.append(node.GetId());
    }

    AddEdges(n);
}

//---------------------------------------------------------------------------------------------------------------------
void VUndoCommand::RemoveEdges(const QVector<VPieceNode> &nodes) const
{
    QVector<quint32> n;
    n.reserve(nodes.size());

    for (const auto &node : nodes)
    {
        n.append(node.GetId());
    }

    RemoveEdges(n);
}

//---------------------------------------------------------------------------------------------------------------------
auto VUndoCommand::GetDestinationObject(quint32 idTool, quint32 idPoint) const -> QDomElement
{
    if (const QDomElement tool = doc->FindElementById(idTool, VAbstractPattern::TagOperation); tool.isElement())
    {
        QDomElement correctDest;
        const QDomNodeList nodeList = tool.childNodes();
        QDOM_LOOP(nodeList, i)
        {
            if (const QDomElement dest = QDOM_ELEMENT(nodeList, i).toElement();
                not dest.isNull() && dest.isElement() && dest.tagName() == VAbstractOperation::TagDestination)
            {
                correctDest = dest;
                break;
            }
        }

        if (not correctDest.isNull())
        {
            const QDomNodeList destObjects = correctDest.childNodes();
            QDOM_LOOP(destObjects, i)
            {
                if (const QDomElement obj = QDOM_ELEMENT(destObjects, i).toElement();
                    not obj.isNull() && obj.isElement())
                {
                    if (const quint32 id = VAbstractPattern::GetParametrUInt(obj, AttrIdObject, NULL_ID_STR);
                        idPoint == id)
                    {
                        return obj;
                    }
                }
            }
        }
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
void VUndoCommand::DisablePieceNodes(const VPiecePath &path)
{
    for (int i = 0; i < path.CountNodes(); ++i)
    {
        const VPieceNode &node = path.at(i);
        if (node.GetTypeTool() != Tool::NodePoint)
        {
            continue;
        }
        try
        {
            if (auto *tool = qobject_cast<VNodePoint *>(VAbstractPattern::getTool(node.GetId())))
            {
                tool->setVisible(false);
            }
        }
        catch (const VExceptionBadId &)
        {
            // ignore
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VUndoCommand::EnablePieceNodes(const VPiecePath &path)
{
    for (int i = 0; i < path.CountNodes(); ++i)
    {
        const VPieceNode &node = path.at(i);
        if (node.GetTypeTool() != Tool::NodePoint)
        {
            continue;
        }

        try
        {
            if (auto *tool = qobject_cast<VNodePoint *>(VAbstractPattern::getTool(node.GetId())))
            {
                tool->setVisible(!node.IsExcluded());
            }
        }
        catch (const VExceptionBadId &)
        {
            // ignore
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VUndoCommand::DisableInternalPaths(const QVector<quint32> &paths)
{
    for (auto path : paths)
    {
        try
        {
            if (auto *tool = qobject_cast<VToolPiecePath *>(VAbstractPattern::getTool(path)))
            {
                tool->setVisible(false);
            }
        }
        catch (const VExceptionBadId &)
        {
            // ignore
        }
    }
}
