/************************************************************************
 **
 **  @file   vpatterngraph.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 12, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
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
#ifndef VPATTERNGRAPH_H
#define VPATTERNGRAPH_H

#include <QHash>
#include <QReadWriteLock>
#include <QtGlobal>

#include "../graaflib/graph.h"
#include "../vmisc/typedef.h"

// Node type enumeration
enum class VNodeType : quint8
{
    TOOL,
    OBJECT
};

// Node structure
struct VNode
{
    vidtype id{NULL_ID};
    VNodeType type{VNodeType::TOOL};

    VNode() = default;
    VNode(vidtype nodeId, VNodeType nodeType)
      : id(nodeId),
        type(nodeType)
    {
    }

    auto operator==(const VNode &other) const -> bool { return id == other.id && type == other.type; }
};

// Empty edge type for unweighted graph
struct VEdge
{
    // Empty struct for unweighted edges
};

/**
 * @brief Thread-safe wrapper for directed unweighted graph
 * 
 * This class provides a thread-safe interface for managing a directed graph
 * with nodes containing an ID and type (TOOL or OBJECT).
 * Uses QReadWriteLock for optimal read-heavy performance.
 */
class VPatternGraph
{
public:
    using vertex_id_t = graaf::vertex_id_t;
    using Graph = graaf::directed_graph<VNode, VEdge>;

    VPatternGraph() = default;
    ~VPatternGraph() = default;

    auto AddVertex(vidtype id, VNodeType type) -> bool;
    auto AddVertex(VNode node) -> bool;

    auto AddEdge(vidtype fromId, vidtype toId) -> bool;

    auto RemoveVertex(vidtype id) -> bool;
    auto RemoveEdge(vidtype fromId, vidtype toId) -> bool;

    auto HasVertex(vidtype id) const -> bool;
    auto HasEdge(vidtype fromId, vidtype toId) const -> bool;

    auto GetVertex(vidtype id) const -> std::optional<VNode>;

    auto GetVertices() const -> QList<vidtype>;

    auto VertexCount() const -> std::size_t;
    auto EdgeCount() const -> std::size_t;

    auto GetNeighbors(vidtype id) const -> QVector<vidtype>;
    auto GetPredecessors(vidtype id) const -> QVector<vidtype>;
    auto GetVerticesByType(VNodeType type) const -> QVector<vidtype>;

    auto GetDegree(vidtype id) const -> std::size_t;
    auto GetInDegree(vidtype id) const -> std::size_t;
    auto GetOutDegree(vidtype id) const -> std::size_t;

    void Clear();

    template<typename Func>
    void WithReadLock(Func &&func) const;

    template<typename Func>
    void WithWriteLock(Func &&func);

    template<typename Func>
    auto TryWithReadLock(Func &&func, int timeout = 1000) const -> bool;

    template<typename Func>
    auto TryWithWriteLock(Func &&func, int timeout = 1000) -> bool;

    auto GetInternalVertexId(vidtype id) const -> std::optional<vertex_id_t>;
    auto GetNodeId(vertex_id_t vertexId) const -> std::optional<vidtype>;

private:
    Q_DISABLE_COPY_MOVE(VPatternGraph)

    Graph m_graph{};
    mutable QReadWriteLock m_lock; // mutable for const methods with locks

    // Fast bidirectional mapping between node IDs and internal vertex IDs
    QHash<vidtype, vertex_id_t> m_idToVertex{}; // node ID -> vertex_id_t
    QHash<vertex_id_t, vidtype> m_vertexToId{}; // vertex_id_t -> node ID
};

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Execute a function on the graph with read lock
 * @param func Function to execute with const graph reference
 * 
 * Note: When working with the internal graph directly, you'll need
 * to use the internal vertex_id_t. Use GetInternalVertexId() helper
 * inside the lambda if needed.
 */
template<typename Func>
inline void VPatternGraph::WithReadLock(Func &&func) const
{
    QReadLocker const locker(&m_lock);
    std::forward<Func>(func)(m_graph);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Execute a function on the graph with write lock
 * @param func Function to execute with mutable graph reference
 * 
 * Warning: Direct graph modifications may break ID mapping.
 * Use this only for read-only graaf algorithms or when you know
 * what you're doing.
 */
template<typename Func>
inline void VPatternGraph::WithWriteLock(Func &&func)
{
    QWriteLocker const locker(&m_lock);
    std::forward<Func>(func)(m_graph);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Try to acquire read lock with timeout
 * @param func Function to execute if lock is acquired
 * @param timeout Timeout in milliseconds
 * @return true if operation completed, false if timeout
 */
template<typename Func>
inline auto VPatternGraph::TryWithReadLock(Func &&func, int timeout) const -> bool
{
    if (m_lock.tryLockForRead(timeout))
    {
        std::forward<Func>(func)(m_graph);
        m_lock.unlock();
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Try to acquire write lock with timeout
 * @param func Function to execute if lock is acquired
 * @param timeout Timeout in milliseconds
 * @return true if operation completed, false if timeout
 */
template<typename Func>
inline auto VPatternGraph::TryWithWriteLock(Func &&func, int timeout) -> bool
{
    if (m_lock.tryLockForWrite(timeout))
    {
        std::forward<Func>(func)(m_graph);
        m_lock.unlock();
        return true;
    }
    return false;
}

#endif // VPATTERNGRAPH_H
