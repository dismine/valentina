/************************************************************************
 **
 **  @file   vhistorymanager.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   5 1, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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
#include "vhistorymanager.h"

#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/defglobal.h"
#include "../vpatterndb/vcontainer.h"

#include <QSet>

//---------------------------------------------------------------------------------------------------------------------
VHistoryManager::VHistoryManager(VAbstractPattern *doc)
  : m_doc(doc)
{
    RebuildIndex();
}

//---------------------------------------------------------------------------------------------------------------------
auto VHistoryManager::CanMoveUp(vidtype objectId) const -> bool
{
    return CalculateMaxUpSteps(objectId) > 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VHistoryManager::CanMoveDown(vidtype objectId) const -> bool
{
    return CalculateMaxDownSteps(objectId) > 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VHistoryManager::CalculateTopId(vidtype objectId) const -> vidtype
{
    const vsizetype currentIdx = m_idToIndex.value(objectId, -1);
    if (currentIdx == -1)
    {
        return NULL_ID;
    }

    const vsizetype maxSteps = CalculateMaxUpSteps(objectId);
    if (maxSteps == 0)
    {
        return NULL_ID;
    }

    const vsizetype index = currentIdx - maxSteps - 1;
    if (index < 0)
    {
        return NULL_ID;
    }

    return m_objects.at(index);
}

//---------------------------------------------------------------------------------------------------------------------
auto VHistoryManager::CalculateBottomId(vidtype objectId) const -> vidtype
{
    const vsizetype currentIdx = m_idToIndex.value(objectId, -1);
    if (currentIdx == -1)
    {
        return NULL_ID;
    }

    const vsizetype maxSteps = CalculateMaxDownSteps(objectId);
    if (maxSteps == 0)
    {
        return NULL_ID;
    }

    const vsizetype index = currentIdx + maxSteps + 1;
    if (index >= m_objects.size())
    {
        return NULL_ID;
    }

    return m_objects.at(index);
}

//---------------------------------------------------------------------------------------------------------------------
auto VHistoryManager::CalculateMaxUpSteps(vidtype objectId) const -> vsizetype
{
    if (objectId == NULL_ID)
    {
        return 0;
    }

    vsizetype const currentIdx = m_idToIndex.value(objectId, -1);
    if (currentIdx <= 0) // Early exit: at beginning or invalid
    {
        return 0;
    }

    if (!IsSameBlock(objectId))
    {
        return 0;
    }

    const QVector<vidtype> predecessors = PredecessorTools(objectId);
    if (predecessors.isEmpty())
    {
        return 0; // Should be impossible, at least base point must exist before any tool or it is a base point
    }

    vsizetype maxDependencyIdx = -1;

    for (const vidtype depId : std::as_const(predecessors))
    {
        if (const vsizetype depIdx = m_idToIndex.value(depId, -1); depIdx != -1)
        {
            maxDependencyIdx = qMax(maxDependencyIdx, depIdx);
        }
    }

    // Must be placed after the last dependency
    const vsizetype topPosition = maxDependencyIdx + 1;

    return (topPosition < currentIdx) ? (currentIdx - topPosition) : 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VHistoryManager::CalculateMaxDownSteps(vidtype objectId) const -> vsizetype
{
    if (objectId == NULL_ID)
    {
        return 0;
    }

    const vsizetype currentIdx = m_idToIndex.value(objectId, -1);
    const vsizetype lastIdx = m_objects.size() - 1;
    if (currentIdx == -1 || currentIdx == 0 || currentIdx == lastIdx) // Early exit: base point, at end or invalid
    {
        return 0;
    }

    if (!IsSameBlock(objectId))
    {
        return 0;
    }

    const QVector<vidtype> dependencies = DependencyTools(objectId);
    if (dependencies.isEmpty())
    {
        return lastIdx - currentIdx; // Can move all the way to the bottom
    }

    vsizetype minDependentIdx = m_objects.size();
    for (const vidtype depId : std::as_const(dependencies))
    {
        if (const vsizetype depIdx = m_idToIndex.value(depId, -1); depIdx != -1)
        {
            minDependentIdx = qMin(minDependentIdx, depIdx);
        }
    }

    // Must be placed before the first dependent
    const vsizetype bottomPosition = minDependentIdx - 1;

    return (bottomPosition > currentIdx) ? (bottomPosition - currentIdx) : 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VHistoryManager::CalculateUpId(vidtype objectId, vsizetype steps) const -> vidtype
{
    if (steps <= 0)
    {
        return NULL_ID;
    }

    const vsizetype currentIdx = m_idToIndex.value(objectId, -1);
    if (currentIdx == -1)
    {
        return NULL_ID;
    }

    const vsizetype maxSteps = CalculateMaxUpSteps(objectId);
    if (maxSteps == 0 || steps > maxSteps)
    {
        return NULL_ID;
    }

    const vsizetype index = currentIdx - steps - 1;
    if (index < 0)
    {
        return NULL_ID;
    }

    return m_objects.at(index);
}

//---------------------------------------------------------------------------------------------------------------------
auto VHistoryManager::CalculateDownId(vidtype objectId, vsizetype steps) const -> vidtype
{
    if (steps <= 0)
    {
        return NULL_ID;
    }

    const vsizetype currentIdx = m_idToIndex.value(objectId, -1);
    if (currentIdx == -1)
    {
        return NULL_ID;
    }

    const vsizetype maxSteps = CalculateMaxDownSteps(objectId);
    if (maxSteps == 0 || steps > maxSteps)
    {
        return NULL_ID;
    }

    const vsizetype index = currentIdx + steps + 1;
    if (index >= m_objects.size())
    {
        return NULL_ID;
    }

    return m_objects.at(index);
}

//---------------------------------------------------------------------------------------------------------------------
void VHistoryManager::RebuildIndex()
{
    m_objects = RootTools();

    m_idToIndex.clear();

    for (vsizetype i = 0; i < m_objects.size(); ++i)
    {
        m_idToIndex[m_objects.at(i)] = i;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VHistoryManager::RootTools() const -> QVector<vidtype>
{
    if (m_doc == nullptr)
    {
        return {};
    }

    QVector<VToolRecord> const history = m_doc->GetLocalHistory();
    QVector<vidtype> rootObjects;
    rootObjects.reserve(history.size());

    for (const auto &record : history)
    {
        // This check helps to find missed tools in the switch
        Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64, "List of tools changed.");

        switch (record.GetToolType())
        {
            case Tool::Arrow:
            case Tool::SinglePoint:
            case Tool::DoublePoint:
            case Tool::LinePoint:
            case Tool::AbstractSpline:
            case Tool::Cut:
            case Tool::Midpoint:         // Same as Tool::AlongLine, but tool will never has such type
            case Tool::ArcIntersectAxis: // Same as Tool::CurveIntersectAxis, but tool will never has such type
            case Tool::BackgroundImage:
            case Tool::BackgroundImageControls:
            case Tool::BackgroundPixmapImage:
            case Tool::BackgroundSVGImage:
            case Tool::ArcStart: // Same as Tool::CutArc, but tool will never has such type
            case Tool::ArcEnd:   // Same as Tool::CutArc, but tool will never has such type
            case Tool::LAST_ONE_DO_NOT_USE:
                Q_UNREACHABLE(); //-V501
                break;
            // Because "history" not only show history of pattern, but help restore current data for each pattern
            // block, we keep records about pieces and nodes, but don't show them.
            case Tool::UnionDetails:
            case Tool::NodeArc:
            case Tool::NodeElArc:
            case Tool::NodePoint:
            case Tool::NodeSpline:
            case Tool::NodeSplinePath:
            case Tool::Group:
            case Tool::PiecePath:
            case Tool::Pin:
            case Tool::PlaceLabel:
            case Tool::InsertNode:
            case Tool::DuplicateDetail:
            case Tool::Piece:
                break;
            // Regular tools dependencies which a user can see
            case Tool::BasePoint:
            case Tool::EndLine:
            case Tool::Line:
            case Tool::AlongLine:
            case Tool::ShoulderPoint:
            case Tool::Normal:
            case Tool::Bisector:
            case Tool::LineIntersect:
            case Tool::Spline:
            case Tool::CubicBezier:
            case Tool::Arc:
            case Tool::ArcWithLength:
            case Tool::ParallelCurve:
            case Tool::GraduatedCurve:
            case Tool::SplinePath:
            case Tool::CubicBezierPath:
            case Tool::PointOfContact:
            case Tool::Height:
            case Tool::Triangle:
            case Tool::PointOfIntersection:
            case Tool::CutArc:
            case Tool::CutSpline:
            case Tool::CutSplinePath:
            case Tool::LineIntersectAxis:
            case Tool::CurveIntersectAxis:
            case Tool::PointOfIntersectionArcs:
            case Tool::PointOfIntersectionCircles:
            case Tool::PointOfIntersectionCurves:
            case Tool::PointFromCircleAndTangent:
            case Tool::PointFromArcAndTangent:
            case Tool::TrueDarts:
            case Tool::EllipticalArc:
            case Tool::Rotation:
            case Tool::FlippingByLine:
            case Tool::FlippingByAxis:
            case Tool::Move:
            case Tool::EllipticalArcWithLength:
            default:
                rootObjects.append(record.GetId());
        }
    }

    return rootObjects;
}

//---------------------------------------------------------------------------------------------------------------------
auto VHistoryManager::IsSameBlock(vidtype objectId) const -> bool
{
    if (m_doc == nullptr)
    {
        return false;
    }

    const VPatternGraph *graph = m_doc->PatternGraph();

    const auto vertex = graph->GetVertex(objectId);
    return vertex && vertex->indexPatternBlock == m_doc->PatternBlockMapper()->GetActiveId();
}

//---------------------------------------------------------------------------------------------------------------------
auto VHistoryManager::PredecessorTools(vidtype objectId) const -> QVector<vidtype>
{
    if (m_doc == nullptr)
    {
        return {};
    }

    const VPatternGraph *graph = m_doc->PatternGraph();
    return FilterNodes(graph->GetPredecessors(objectId));
}

//---------------------------------------------------------------------------------------------------------------------
auto VHistoryManager::DependencyTools(vidtype objectId) const -> QVector<vidtype>
{
    if (m_doc == nullptr)
    {
        return {};
    }

    const VPatternGraph *graph = m_doc->PatternGraph();
    return FilterNodes(graph->GetNeighbors(objectId));
}

//---------------------------------------------------------------------------------------------------------------------
auto VHistoryManager::FilterNodes(const QVector<vidtype> &nodes) const -> QVector<vidtype>
{
    if (m_doc == nullptr)
    {
        return {};
    }

    QSet<vidtype> tools;
    tools.reserve(nodes.size());

    const int activeBlock = m_doc->PatternBlockMapper()->GetActiveId();
    VContainer const patternData = m_doc->GetCompletePPData(m_doc->PatternBlockMapper()->GetActive());
    const VPatternGraph *graph = m_doc->PatternGraph();

    for (auto nodeId : std::as_const(nodes))
    {
        const auto vertex = graph->GetVertex(nodeId);
        if (!vertex || vertex->indexPatternBlock != activeBlock)
        {
            continue;
        }

        if (vertex->type == VNodeType::TOOL)
        {
            tools.insert(nodeId);
        }
        else if (vertex->type == VNodeType::OBJECT)
        {
            try
            {
                const QSharedPointer<VGObject> obj = patternData.GetGObject(nodeId);
                tools.insert(obj->getIdTool());
            }
            catch (const VExceptionBadId &)
            {
                // do nothing
            }
        }
    }

    return ConvertToVector(tools);
}
