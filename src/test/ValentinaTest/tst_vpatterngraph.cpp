/************************************************************************
 **
 **  @file   tst_vpatterngraph.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   19 5, 2026
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

#include "tst_vpatterngraph.h"
#include "../ifc/xml/vpatterngraph.h"

#include <QtTest>

namespace
{
// Stable ID constants used across tests so the relationships are obvious when reading.
constexpr vidtype kSourceNode = 1;
constexpr vidtype kUnionTool = 10;
constexpr vidtype kChild1 = 20;
constexpr vidtype kChild2 = 21;
constexpr vidtype kModelNode1 = 30;
constexpr vidtype kModelNode2 = 31;
constexpr vidtype kPiece = 100;

//---------------------------------------------------------------------------------------------------------------------
auto PieceFilter(const VNode &node) -> bool
{
    return node.type == VNodeType::PIECE;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
TST_VPatternGraph::TST_VPatternGraph(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
// GetDependentNodes returns empty when no PIECE is reachable from the given node.
void TST_VPatternGraph::TestGetDependentNodes_NoPieceDescendant()
{
    VPatternGraph graph;
    graph.AddVertex(kChild1, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kModelNode1, VNodeType::MODELING_OBJECT, 0);
    graph.AddEdge(kChild1, kModelNode1);

    const auto deps = graph.GetDependentNodes(kChild1, PieceFilter);
    QVERIFY(deps.isEmpty());
}

//---------------------------------------------------------------------------------------------------------------------
// GetDependentNodes returns the piece when child → model_node → piece.
void TST_VPatternGraph::TestGetDependentNodes_WithDirectPiece()
{
    VPatternGraph graph;
    graph.AddVertex(kChild1, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kModelNode1, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kPiece, VNodeType::PIECE, 0);
    graph.AddEdge(kChild1, kModelNode1);
    graph.AddEdge(kModelNode1, kPiece);

    const auto deps = graph.GetDependentNodes(kChild1, PieceFilter);
    QCOMPARE(deps.size(), 1);
    QCOMPARE(deps.first().id, kPiece);
    QCOMPARE(deps.first().type, VNodeType::PIECE);
}

//---------------------------------------------------------------------------------------------------------------------
// GetDependentNodes traverses arbitrarily deep chains to find PIECE nodes.
void TST_VPatternGraph::TestGetDependentNodes_WithChainedPiece()
{
    // child1 → intermediate → model_node → piece
    constexpr vidtype kIntermediate = 25;

    VPatternGraph graph;
    graph.AddVertex(kChild1, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kIntermediate, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kModelNode1, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kPiece, VNodeType::PIECE, 0);
    graph.AddEdge(kChild1, kIntermediate);
    graph.AddEdge(kIntermediate, kModelNode1);
    graph.AddEdge(kModelNode1, kPiece);

    const auto deps = graph.GetDependentNodes(kChild1, PieceFilter);
    QCOMPARE(deps.size(), 1);
    QCOMPARE(deps.first().id, kPiece);
}

//---------------------------------------------------------------------------------------------------------------------
// GetDependentNodes returns only PIECE-typed nodes, not intermediate MODELING_OBJECT ones.
void TST_VPatternGraph::TestGetDependentNodes_OnlyReturnsPieceNodes()
{
    VPatternGraph graph;
    graph.AddVertex(kChild1, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kModelNode1, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kPiece, VNodeType::PIECE, 0);
    graph.AddEdge(kChild1, kModelNode1);
    graph.AddEdge(kModelNode1, kPiece);

    const auto deps = graph.GetDependentNodes(kChild1, PieceFilter);

    for (const VNode &node : deps)
    {
        QCOMPARE(node.type, VNodeType::PIECE);
    }
}

//---------------------------------------------------------------------------------------------------------------------
// Simulates GCStaleUnionTools: union tool with two children, neither reaching a PIECE.
// All GetDependentNodes calls return empty → all_stale = true.
void TST_VPatternGraph::TestUnionToolStaleness_AllChildrenOrphaned()
{
    VPatternGraph graph;
    graph.AddVertex(kUnionTool, VNodeType::MODELING_TOOL, 0);
    graph.AddVertex(kChild1, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kChild2, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kModelNode1, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kModelNode2, VNodeType::MODELING_OBJECT, 0);
    graph.AddEdge(kUnionTool, kChild1);
    graph.AddEdge(kUnionTool, kChild2);
    graph.AddEdge(kChild1, kModelNode1);
    graph.AddEdge(kChild2, kModelNode2);
    // No PIECE vertex — neither chain reaches a piece.

    const bool child1Stale = graph.GetDependentNodes(kChild1, PieceFilter).isEmpty();
    const bool child2Stale = graph.GetDependentNodes(kChild2, PieceFilter).isEmpty();

    QVERIFY(child1Stale);
    QVERIFY(child2Stale);
    QVERIFY(child1Stale && child2Stale); // all_stale → tool is GC candidate
}

//---------------------------------------------------------------------------------------------------------------------
// Simulates GCStaleUnionTools: one live child prevents the whole tool from being GC'd.
void TST_VPatternGraph::TestUnionToolStaleness_OneLiveChildPreventsGC()
{
    VPatternGraph graph;
    graph.AddVertex(kUnionTool, VNodeType::MODELING_TOOL, 0);
    graph.AddVertex(kChild1, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kChild2, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kModelNode1, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kModelNode2, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kPiece, VNodeType::PIECE, 0);
    graph.AddEdge(kUnionTool, kChild1);
    graph.AddEdge(kUnionTool, kChild2);
    graph.AddEdge(kChild1, kModelNode1);
    graph.AddEdge(kModelNode1, kPiece); // child1 is live
    graph.AddEdge(kChild2, kModelNode2); // child2 has no piece

    const bool child1Stale = graph.GetDependentNodes(kChild1, PieceFilter).isEmpty();
    const bool child2Stale = graph.GetDependentNodes(kChild2, PieceFilter).isEmpty();

    QVERIFY(!child1Stale);  // child1 is live
    QVERIFY(child2Stale);   // child2 is orphaned

    const bool allStale = child1Stale && child2Stale;
    QVERIFY(!allStale); // tool must NOT be GC'd
}

//---------------------------------------------------------------------------------------------------------------------
// Three children: two stale, one live — not all stale, tool survives.
void TST_VPatternGraph::TestUnionToolStaleness_MixedChildrenNotAllStale()
{
    constexpr vidtype kChild3     = 22;
    constexpr vidtype kModelNode3 = 32;

    VPatternGraph graph;
    graph.AddVertex(kUnionTool, VNodeType::MODELING_TOOL, 0);
    graph.AddVertex(kChild1, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kChild2, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kChild3, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kModelNode1, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kModelNode2, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kModelNode3, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kPiece, VNodeType::PIECE, 0);
    graph.AddEdge(kUnionTool, kChild1);
    graph.AddEdge(kUnionTool, kChild2);
    graph.AddEdge(kUnionTool, kChild3);
    graph.AddEdge(kChild1, kModelNode1);
    graph.AddEdge(kChild2, kModelNode2);
    graph.AddEdge(kChild3, kModelNode3);
    graph.AddEdge(kModelNode2, kPiece); // only child2 is live

    const QVector<vidtype> children = {kChild1, kChild2, kChild3};
    const bool allStale = std::ranges::all_of(children,
                                               [&graph](vidtype childId)
                                               { return graph.GetDependentNodes(childId, PieceFilter).isEmpty(); });

    QVERIFY(!allStale);
}

//---------------------------------------------------------------------------------------------------------------------
// After removing the PIECE vertex, a previously live child becomes stale.
void TST_VPatternGraph::TestUnionToolStaleness_PieceRemovedMakesChildStale()
{
    VPatternGraph graph;
    graph.AddVertex(kChild1, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kModelNode1, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kPiece, VNodeType::PIECE, 0);
    graph.AddEdge(kChild1, kModelNode1);
    graph.AddEdge(kModelNode1, kPiece);

    QVERIFY(!graph.GetDependentNodes(kChild1, PieceFilter).isEmpty()); // live before removal

    graph.RemoveVertex(kPiece);

    QVERIFY(graph.GetDependentNodes(kChild1, PieceFilter).isEmpty()); // stale after removal
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPatternGraph::TestGetVerticesByType_ModelingTool()
{
    VPatternGraph graph;
    graph.AddVertex(kSourceNode, VNodeType::OBJECT, 0);
    graph.AddVertex(kUnionTool, VNodeType::MODELING_TOOL, 0);
    graph.AddVertex(kChild1, VNodeType::MODELING_OBJECT, 0);
    graph.AddVertex(kPiece, VNodeType::PIECE, 0);

    const QVector<vidtype> tools = graph.GetVerticesByType(VNodeType::MODELING_TOOL);
    QCOMPARE(tools.size(), 1);
    QCOMPARE(tools.first(), kUnionTool);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPatternGraph::TestGetVerticesByType_EmptyGraph()
{
    VPatternGraph graph;
    QVERIFY(graph.GetVerticesByType(VNodeType::MODELING_TOOL).isEmpty());
}
