/************************************************************************
 **
 **  @file   vdependencytreemodel.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   1 1, 2026
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
#include "vdependencytreemodel.h"

#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vpatterndb/vcontainer.h"
#include "../vtools/tools/drawTools/vtoolline.h"

#include <QStringLiteral>

namespace
{
auto GetPieceName(vidtype id, const VContainer &patternData, const QString &defaultName) -> QString
{
    try
    {
        return patternData.GetPiece(id).GetName();
    }
    catch (const VExceptionBadId &)
    {
        return defaultName;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto FormatObjectName(const QSharedPointer<VGObject> &obj) -> QString
{
    QString objName = obj->name();
    if (!obj->GetAlias().isEmpty())
    {
        objName = QStringLiteral("%1 (%2)").arg(objName, obj->GetAlias());
    }
    return objName;
}

//---------------------------------------------------------------------------------------------------------------------
auto GetObjectName(vidtype id, const VContainer &patternData, const QString &defaultName) -> QString
{
    try
    {
        const QSharedPointer<VGObject> obj = patternData.GetGObject(id);
        return FormatObjectName(obj);
    }
    catch (const VExceptionBadId &)
    {
        return defaultName;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto IsGeometricTool(Tool toolType) -> bool
{
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64, "List of tools changed.");

    switch (toolType)
    {
        case Tool::BasePoint:
        case Tool::EndLine:
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
        case Tool::EllipticalArc:
        case Tool::EllipticalArcWithLength:
            return true;
        default:
            return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto GetGeometricToolName(vidtype id, const VContainer &patternData, const QString &defaultName) -> QString
{
    try
    {
        const QSharedPointer<VGObject> obj = patternData.GetGObject(id);
        return FormatObjectName(obj);
    }
    catch (const VExceptionBadId &)
    {
        return defaultName;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto FindChildIndex(VDependencyNode *parent, vidtype objectId) -> int
{
    for (int i = 0; i < parent->children.size(); ++i)
    {
        if (parent->children.at(i)->objectId == objectId)
        {
            return i;
        }
    }
    return -1;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VDependencyTreeModel::VDependencyTreeModel(QObject *parent)
  : QAbstractItemModel{parent},
    m_rootNode(QSharedPointer<VDependencyNode>::create())
{
    m_rootNode->objectId = 0; // Root has ID 0
    m_rootNode->childrenLoaded = true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyTreeModel::index(int row, int column, const QModelIndex &parent) const -> QModelIndex
{
    if (!hasIndex(row, column, parent))
    {
        return {};
    }

    VDependencyNode *parentNode = GetNode(parent);

    if (row >= 0 && row < parentNode->children.size())
    {
        return createIndex(row, column, parentNode->children[row].get());
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyTreeModel::parent(const QModelIndex &child) const -> QModelIndex
{
    if (!child.isValid())
    {
        return {};
    }

    const VDependencyNode *childNode = GetNode(child);
    const VDependencyNode *parentNode = childNode->parent;

    if ((parentNode == nullptr) || parentNode == m_rootNode.get())
    {
        return {};
    }

    VDependencyNode const *grandParent = parentNode->parent;
    if (grandParent == nullptr)
    {
        return {};
    }

    int row = 0;
    for (const auto &c : grandParent->children)
    {
        if (c.get() == parentNode)
        {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            return createIndex(row, 0, parentNode);
#else
            return createIndex(row, 0, const_cast<VDependencyNode *>(parentNode));
#endif
        }
        row++;
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyTreeModel::rowCount(const QModelIndex &parent) const -> int
{
    VDependencyNode const *parentNode = GetNode(parent);
    return static_cast<int>(parentNode->children.size());
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyTreeModel::columnCount(const QModelIndex &parent) const -> int
{
    Q_UNUSED(parent);
    return 1;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyTreeModel::data(const QModelIndex &index, int role) const -> QVariant
{
    if (!index.isValid())
    {
        return {};
    }

    VDependencyNode const *node = GetNode(index);

    if (role == Qt::DisplayRole)
    {
        return node->displayName;
    }

    if (role == Qt::ToolTipRole)
    {
        return GetDisplayToolTipForObject(node->objectId);
    }

    if (role == Qt::UserRole)
    {
        return node->objectId; // Store objectId for easy access
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyTreeModel::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0)
    {
        return tr("Dependency tree");
    }
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyTreeModel::hasChildren(const QModelIndex &parent) const -> bool
{
    if (const VDependencyNode *node = GetNode(parent); node->childrenLoaded)
    {
        return !node->children.isEmpty();
    }

    // Assume all unloaded nodes can have children
    // The expand arrow disappears after loading if no children found
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyTreeModel::canFetchMore(const QModelIndex &parent) const -> bool
{
    const VDependencyNode *node = GetNode(parent);
    return !node->childrenLoaded;
}

//---------------------------------------------------------------------------------------------------------------------
void VDependencyTreeModel::fetchMore(const QModelIndex &parent)
{
    VDependencyNode *node = GetNode(parent);

    if (node->childrenLoaded)
    {
        return;
    }

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);

    LoadDependencies(parent, node);

    QGuiApplication::restoreOverrideCursor();
}

//---------------------------------------------------------------------------------------------------------------------
void VDependencyTreeModel::SetRootObjects(const QVector<vidtype> &objectIds)
{
    beginResetModel();

    m_rootNode->children.clear();

    for (vidtype const objId : objectIds)
    {
        auto node = QSharedPointer<VDependencyNode>::create();
        node->objectId = objId;
        node->displayName = GetDisplayNameForObject(objId);
        node->parent = m_rootNode.get();
        node->childrenLoaded = false;

        m_rootNode->children.append(node);
    }

    endResetModel();
}

//---------------------------------------------------------------------------------------------------------------------
void VDependencyTreeModel::ClearModel()
{
    beginResetModel();
    m_rootNode->children.clear();
    m_expandedNodes.clear();
    endResetModel();
}

//---------------------------------------------------------------------------------------------------------------------
void VDependencyTreeModel::SetCurrentPattern(const VAbstractPattern *doc)
{
    m_doc = doc;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyTreeModel::FindRootIndexByObjectId(vidtype objectId) const -> QModelIndex
{
    for (int i = 0; i < m_rootNode->children.size(); ++i)
    {
        if (m_rootNode->children.at(i)->objectId == objectId)
        {
            return index(i, 0, QModelIndex());
        }
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
void VDependencyTreeModel::UpdateTree(const QVector<vidtype> &newRootObjects)
{
    // Build sets for comparison
    QSet<vidtype> currentIds;
    QVector<vidtype> currentOrder;
    for (const auto &child : std::as_const(m_rootNode->children))
    {
        currentIds.insert(child->objectId);
        currentOrder.append(child->objectId);
    }

    QSet<vidtype> const newIds = QSet<vidtype>(newRootObjects.begin(), newRootObjects.end());

    // Find what to remove and what to add
    QSet<vidtype> const toRemove = currentIds - newIds;
    QSet<vidtype> const toAdd = newIds - currentIds;

    // Remove obsolete nodes
    for (vidtype const id : std::as_const(toRemove))
    {
        RemoveRootObject(id);
    }

    // Add new nodes
    for (vidtype const id : std::as_const(toAdd))
    {
        AddRootObject(id);
    }

    // Check if reordering is needed (only for objects that weren't added/removed)
    QSet<vidtype> const remaining = currentIds - toRemove;
    bool needsReorder = false;

    if (remaining.size() > 1) // Only check if there are multiple items
    {
        // Build current order of remaining items
        QVector<vidtype> currentRemainingOrder;
        for (vidtype const id : std::as_const(currentOrder))
        {
            if (remaining.contains(id))
            {
                currentRemainingOrder.append(id);
            }
        }

        // Build expected order from newRootObjects
        QVector<vidtype> newRemainingOrder;
        for (vidtype const id : std::as_const(newRootObjects))
        {
            if (remaining.contains(id))
            {
                newRemainingOrder.append(id);
            }
        }

        // Check if order changed
        needsReorder = (currentRemainingOrder != newRemainingOrder);
    }

    if (needsReorder)
    {
        // Reorder the children to match newRootObjects
        ReorderRootObjects(newRootObjects);
    }

    // Refresh existing nodes if their children were loaded
    for (auto &child : m_rootNode->children)
    {
        if (child->childrenLoaded)
        {
            QVector<vidtype> const newDeps = FetchDependenciesForObject(child->objectId);
            UpdateNodeChildren(child.get(), newDeps);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDependencyTreeModel::AddRootObject(vidtype objectId)
{
    // Check if already exists
    for (const auto &child : std::as_const(m_rootNode->children))
    {
        if (child->objectId == objectId)
        {
            return;
        }
    }

    int const row = static_cast<int>(m_rootNode->children.size());
    beginInsertRows(QModelIndex(), row, row);

    auto node = QSharedPointer<VDependencyNode>::create();
    node->objectId = objectId;
    node->displayName = GetDisplayNameForObject(objectId);
    node->parent = m_rootNode.get();
    node->childrenLoaded = false;

    m_rootNode->children.append(node);

    endInsertRows();
}

//---------------------------------------------------------------------------------------------------------------------
void VDependencyTreeModel::RemoveRootObject(vidtype objectId)
{
    int const index = FindChildIndex(m_rootNode.get(), objectId);
    if (index >= 0)
    {
        RemoveChildAt(m_rootNode.get(), index);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDependencyTreeModel::RefreshNode(const QString &objectPath)
{
    VDependencyNode *node = FindNodeByPath(objectPath);
    if ((node == nullptr) || !node->childrenLoaded)
    {
        return;
    }

    QVector<vidtype> const newDependencies = FetchDependenciesForObject(node->objectId);
    UpdateNodeChildren(node, newDependencies);
}

//---------------------------------------------------------------------------------------------------------------------
void VDependencyTreeModel::ReorderRootObjects(const QVector<vidtype> &newOrder)
{
    // Build map for quick lookup
    QHash<vidtype, int> currentPositions;
    for (int i = 0; i < m_rootNode->children.size(); ++i)
    {
        currentPositions[m_rootNode->children[i]->objectId] = i;
    }

    // Move items one by one to match new order
    for (int targetPos = 0; targetPos < newOrder.size(); ++targetPos)
    {
        vidtype const id = newOrder[targetPos];

        if (!currentPositions.contains(id))
        {
            continue; // Skip items that don't exist
        }

        int currentPos = -1;
        for (int i = 0; i < m_rootNode->children.size(); ++i)
        {
            if (m_rootNode->children[i]->objectId == id)
            {
                currentPos = i;
                break;
            }
        }

        if (currentPos != targetPos && currentPos != -1)
        {
            // Move from currentPos to targetPos
            int destPos = targetPos;
            if (currentPos < targetPos)
            {
                destPos++; // Qt adjustment for moving down
            }

            beginMoveRows(QModelIndex(), currentPos, currentPos, QModelIndex(), destPos);
            m_rootNode->children.move(currentPos, targetPos);
            endMoveRows();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDependencyTreeModel::SetExpandedNodes(const QSet<QString> &expandedPaths)
{
    m_expandedNodes = expandedPaths;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyTreeModel::GetExpandedNodes() const -> QSet<QString>
{
    return m_expandedNodes;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyTreeModel::GetNode(const QModelIndex &index) const -> VDependencyNode *
{
    if (index.isValid())
    {
        // Map proxy index to source model index
        QModelIndex sourceIndex = index;

        // Check if we're dealing with a proxy model
        if (const auto *proxyModel = qobject_cast<const QAbstractProxyModel *>(index.model()))
        {
            sourceIndex = proxyModel->mapToSource(index);
        }

        return static_cast<VDependencyNode *>(sourceIndex.internalPointer());
    }
    return m_rootNode.get();
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyTreeModel::FindNodeByPath(const QString &path, VDependencyNode *root) -> VDependencyNode *
{
    if (root == nullptr)
    {
        root = m_rootNode.get();
    }

    QStringList parts = path.split('/');
    VDependencyNode *current = root;

    for (const QString &part : std::as_const(parts))
    {
        bool ok = false;
        vidtype const id = part.toUInt(&ok);
        if (!ok)
        {
            return nullptr;
        }

        bool found = false;
        for (auto &child : current->children)
        {
            if (child->objectId == id)
            {
                current = child.get();
                found = true;
                break;
            }
        }
        if (!found)
        {
            return nullptr;
        }
    }

    return current;
}

//---------------------------------------------------------------------------------------------------------------------
void VDependencyTreeModel::LoadDependencies(const QModelIndex &parentIndex, VDependencyNode *node)
{
    if ((node == nullptr) || node->childrenLoaded)
    {
        return;
    }

    if (const QVector<vidtype> dependencies = FetchDependenciesForObject(node->objectId); !dependencies.isEmpty())
    {
        beginInsertRows(parentIndex, 0, static_cast<int>(dependencies.size() - 1));

        for (vidtype const depId : std::as_const(dependencies))
        {
            auto childNode = QSharedPointer<VDependencyNode>::create();
            childNode->objectId = depId;
            childNode->displayName = GetDisplayNameForObject(depId);
            childNode->parent = node;
            childNode->childrenLoaded = false;

            node->children.append(childNode);
        }

        endInsertRows();
    }

    node->childrenLoaded = true;

    // Notify the view to re-evaluate hasChildren and refresh the expand arrow
    emit dataChanged(parentIndex, parentIndex, {Qt::DisplayRole});
}

//---------------------------------------------------------------------------------------------------------------------
void VDependencyTreeModel::UpdateNodeChildren(VDependencyNode *node, const QVector<vidtype> &newDependencies)
{
    if (node == nullptr)
    {
        return;
    }

    // Build current and new sets
    QSet<vidtype> currentIds;
    for (const auto &child : std::as_const(node->children))
    {
        currentIds.insert(child->objectId);
    }

    QSet<vidtype> const newIds = QSet<vidtype>(newDependencies.begin(), newDependencies.end());

    // Find what to remove
    QSet<vidtype> const toRemove = currentIds - newIds;

    // Remove obsolete children (iterate backwards to avoid index issues)
    for (int i = static_cast<int>(node->children.size() - 1); i >= 0; --i)
    {
        if (toRemove.contains(node->children[i]->objectId))
        {
            RemoveChildAt(node, i);
        }
    }

    // Add new children (maintain order from newDependencies)
    for (int i = 0; i < newDependencies.size(); ++i)
    {
        vidtype const depId = newDependencies.at(i);

        // Check if this child already exists
        int const existingIndex = FindChildIndex(node, depId);

        if (existingIndex < 0)
        {
            // New child - insert at correct position
            InsertChild(node, i, depId);
        }
        else if (existingIndex != i)
        {
            // Child exists but at wrong position - move it
            // For simplicity, we'll just leave it (or implement move logic)
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VDependencyTreeModel::RemoveChildAt(VDependencyNode *parent, int index)
{
    if (index < 0 || index >= parent->children.size())
    {
        return;
    }

    QModelIndex parentIndex;
    if (parent == m_rootNode.get())
    {
        parentIndex = QModelIndex();
    }
    else
    {
        // Find parent's index
        if (parent->parent != nullptr)
        {
            int row = 0;
            for (const auto &child : std::as_const(parent->parent->children))
            {
                if (child.get() == parent)
                {
                    parentIndex = createIndex(row, 0, parent);
                    break;
                }
                row++;
            }
        }
    }

    beginRemoveRows(parentIndex, index, index);
    parent->children.removeAt(index);
    endRemoveRows();
}

//---------------------------------------------------------------------------------------------------------------------
void VDependencyTreeModel::InsertChild(VDependencyNode *parent, int index, vidtype objectId)
{
    QModelIndex parentIndex;
    if (parent == m_rootNode.get())
    {
        parentIndex = QModelIndex();
    }
    else
    {
        if (parent->parent != nullptr)
        {
            int row = 0;
            for (const auto &child : std::as_const(parent->parent->children))
            {
                if (child.get() == parent)
                {
                    parentIndex = createIndex(row, 0, parent);
                    break;
                }
                row++;
            }
        }
    }

    beginInsertRows(parentIndex, index, index);

    auto childNode = QSharedPointer<VDependencyNode>::create();
    childNode->objectId = objectId;
    childNode->displayName = GetDisplayNameForObject(objectId);
    childNode->parent = parent;
    childNode->childrenLoaded = false;

    parent->children.insert(index, childNode);

    endInsertRows();
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyTreeModel::FetchDependenciesForObject(vidtype objectId) const -> QVector<vidtype>
{
    if (m_doc == nullptr)
    {
        return {};
    }

    const VPatternGraph *graph = m_doc->PatternGraph();
    const QVector<vidtype> dependencies = graph->GetNeighbors(objectId);

    QVector<vidtype> visibleDependencies;
    visibleDependencies.reserve(dependencies.size());

    for (const auto id : dependencies)
    {
        if (const auto node = graph->GetVertex(id);
            node && (node->type == VNodeType::MODELING_TOOL || node->type == VNodeType::MODELING_OBJECT))
        {
            auto Filter = [](const auto &node) -> auto { return node.type == VNodeType::PIECE; };
            const QVector<VNode> nodeDependencies = graph->GetDependentNodes(id, Filter);

            for (const auto pieceNode : nodeDependencies)
            {
                if (!visibleDependencies.contains(pieceNode.id))
                {
                    visibleDependencies.append(pieceNode.id);
                }
            }
        }
        else
        {
            visibleDependencies.append(id);
        }
    }

    return visibleDependencies;
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyTreeModel::GetDisplayNameForObject(vidtype objectId) const -> QString
{
    QString defaultName = QStringLiteral("Object_%1").arg(objectId);

    if (m_doc == nullptr)
    {
        return defaultName;
    }

    VPatternGraph const *graph = m_doc->PatternGraph();

    auto node = graph->GetVertex(objectId);
    if (!node)
    {
        return defaultName;
    }

    if (node->indexPatternBlock < 0)
    {
        return defaultName;
    }

    VContainer const patternData = m_doc->GetCompletePPData(
        m_doc->PatternBlockMapper()->FindName(node->indexPatternBlock));

    switch (node->type)
    {
        case VNodeType::PIECE:
            return GetPieceName(node->id, patternData, defaultName);
        case VNodeType::OBJECT:
            return GetObjectName(node->id, patternData, defaultName);
        case VNodeType::TOOL:
            return GetToolName(node->id, patternData, defaultName);
        default:
            return defaultName;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyTreeModel::GetDisplayToolTipForObject(vidtype objectId) const -> QString
{
    if (m_doc == nullptr)
    {
        return {};
    }

    VPatternGraph const *graph = m_doc->PatternGraph();

    auto node = graph->GetVertex(objectId);
    if (!node)
    {
        return {};
    }

    VContainer const patternData = m_doc->GetCompleteData();

    switch (node->type)
    {
        case VNodeType::PIECE:
            return tr("Piece '%1'").arg(GetPieceName(node->id, patternData, QString()));
        case VNodeType::TOOL:
            return GetToolToolTip(node->id);
        case VNodeType::OBJECT:
        default:
            return {};
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyTreeModel::FindToolRecord(vidtype id) const -> VToolRecord
{
    const QVector<VToolRecord> *history = m_doc->getHistory();

    for (const auto &record : *history)
    {
        if (id == record.GetId())
        {
            return record;
        }
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyTreeModel::GetToolName(vidtype id, const VContainer &patternData, const QString &defaultName) const
    -> QString
{
    const VToolRecord tool = FindToolRecord(id);

    if (tool.GetToolType() == Tool::LAST_ONE_DO_NOT_USE)
    {
        return defaultName;
    }

    if (tool.GetToolType() == Tool::Line)
    {
        return GetLineToolName(tool.GetId(), defaultName);
    }

    if (IsGeometricTool(tool.GetToolType()))
    {
        return GetGeometricToolName(tool.GetId(), patternData, defaultName);
    }

    return GetOperationToolName(tool.GetToolType(), defaultName);
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyTreeModel::GetOperationToolName(Tool toolType, const QString &defaultName) const -> QString
{
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64, "List of tools changed.");

    switch (toolType)
    {
        case Tool::TrueDarts:
            return tr("True darts", "operation");
        case Tool::Rotation:
            return tr("Rotation", "operation");
        case Tool::FlippingByLine:
            return tr("Flipping by line", "operation");
        case Tool::FlippingByAxis:
            return tr("Flipping by axis", "operation");
        case Tool::Move:
            return tr("Move", "operation");
        default:
            return defaultName;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyTreeModel::GetToolToolTip(vidtype id) const -> QString
{
    const VToolRecord tool = FindToolRecord(id);

    if (tool.GetToolType() == Tool::LAST_ONE_DO_NOT_USE)
    {
        return {};
    }

    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64, "List of tools changed.");

    switch (tool.GetToolType())
    {
        case Tool::BasePoint:
            return tr("Base point");
        case Tool::EndLine:
            return tr("Point at distance and angle");
        case Tool::AlongLine:
            return tr("Point along line");
        case Tool::ShoulderPoint:
            return tr("Shoulder point");
        case Tool::Normal:
            return tr("Point along perpendicular");
        case Tool::Bisector:
            return tr("Bisector");
        case Tool::LineIntersect:
            return tr("Point at line intersection");
        case Tool::Spline:
        case Tool::CubicBezier:
            return tr("Spline");
        case Tool::Arc:
            return tr("Arc");
        case Tool::ArcWithLength:
            return tr("Elliptical arc with given length");
        case Tool::ParallelCurve:
            return tr("Parallel curve");
        case Tool::GraduatedCurve:
            return tr("Graduated curve");
        case Tool::SplinePath:
        case Tool::CubicBezierPath:
            return tr("Spline path");
        case Tool::PointOfContact:
            return tr("Point of intersection circle and segment");
        case Tool::Height:
            return tr("Perpendicular point along line");
        case Tool::Triangle:
            return tr("Point located at the triangle's right-angled corner, lying along the axis");
        case Tool::PointOfIntersection:
            return tr("Point from X and Y of two other points");
        case Tool::CutArc:
            return tr("Point segmenting an arc");
        case Tool::CutSpline:
            return tr("Point segmenting a spline");
        case Tool::CutSplinePath:
            return tr("Point segmenting a spline path");
        case Tool::LineIntersectAxis:
            return tr("Point of intersection line and axis");
        case Tool::CurveIntersectAxis:
            return tr("Point of intersection curve and axis");
        case Tool::PointOfIntersectionArcs:
            return tr("Point of intersection arcs");
        case Tool::PointOfIntersectionCircles:
            return tr("Point of intersection circles");
        case Tool::PointOfIntersectionCurves:
            return tr("Point of intersection curves");
        case Tool::PointFromCircleAndTangent:
            return tr("Point from circle and tangent");
        case Tool::PointFromArcAndTangent:
            return tr("Point from arc and tangent");
        case Tool::EllipticalArc:
            return tr("Elliptical arc");
        case Tool::EllipticalArcWithLength:
            return tr("Elliptical arc with given length");
        case Tool::Line:
            return tr("Line connecting two points");
        default:
            return {};
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyTreeModel::GetLineToolName(vidtype id, const QString &defaultName) const -> QString
{
    try
    {
        auto *tool = qobject_cast<VToolLine *>(VAbstractPattern::getTool(id));
        if (tool == nullptr)
        {
            return defaultName;
        }

        return tr("Line_%1_%2").arg(tool->FirstPointName(), tool->SecondPointName());
    }
    catch (VExceptionBadId &)
    {
        return defaultName;
    }
}

// ============================================================================
// VTreeStateManager
// ============================================================================
VTreeStateManager::VTreeStateManager(QTreeView *view, VDependencyTreeModel *model, QObject *parent)
  : QObject(parent),
    m_view(view),
    m_model(model)
{
    connect(m_view, &QTreeView::expanded, this, &VTreeStateManager::OnExpanded);
    connect(m_view, &QTreeView::collapsed, this, &VTreeStateManager::OnCollapsed);
}

//---------------------------------------------------------------------------------------------------------------------
void VTreeStateManager::SaveState()
{
    m_expandedPaths.clear();
    SaveExpandedState(QModelIndex());
}

//---------------------------------------------------------------------------------------------------------------------
void VTreeStateManager::RestoreState()
{
    for (const QString &path : std::as_const(m_expandedPaths))
    {
        if (QModelIndex const index = FindIndexByPath(path); index.isValid())
        {
            m_view->expand(index);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VTreeStateManager::OnExpanded(const QModelIndex &index)
{
    if (index.isValid())
    {
        // Map proxy index to source model index
        QModelIndex sourceIndex = index;

        // Check if we're dealing with a proxy model
        if (const auto *proxyModel = qobject_cast<const QAbstractProxyModel *>(index.model()))
        {
            sourceIndex = proxyModel->mapToSource(index);
        }

        const auto *node = static_cast<VDependencyNode *>(sourceIndex.internalPointer());
        m_expandedPaths.insert(node->GetPath());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VTreeStateManager::OnCollapsed(const QModelIndex &index)
{
    if (index.isValid())
    {
        // Map proxy index to source model index
        QModelIndex sourceIndex = index;

        // Check if we're dealing with a proxy model
        if (const auto *proxyModel = qobject_cast<const QAbstractProxyModel *>(index.model()))
        {
            sourceIndex = proxyModel->mapToSource(index);
        }

        const auto *node = static_cast<VDependencyNode *>(sourceIndex.internalPointer());
        m_expandedPaths.remove(node->GetPath());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VTreeStateManager::SaveExpandedState(const QModelIndex &parent)
{
    for (int i = 0; i < m_model->rowCount(parent); ++i)
    {
        if (QModelIndex const index = m_model->index(i, 0, parent); m_view->isExpanded(index))
        {
            // Map proxy index to source model index
            QModelIndex sourceIndex = index;

            // Check if we're dealing with a proxy model
            if (const auto *proxyModel = qobject_cast<const QAbstractProxyModel *>(index.model()))
            {
                sourceIndex = proxyModel->mapToSource(index);
            }

            const auto *node = static_cast<VDependencyNode *>(sourceIndex.internalPointer());
            m_expandedPaths.insert(node->GetPath());
            SaveExpandedState(index);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VTreeStateManager::FindIndexByPath(const QString &path) -> QModelIndex
{
    QStringList parts = path.split('/');
    QModelIndex current;

    for (const QString &part : std::as_const(parts))
    {
        bool ok = false;
        const vidtype id = part.toUInt(&ok);
        if (!ok)
        {
            return {};
        }

        bool found = false;
        for (int i = 0; i < m_model->rowCount(current); ++i)
        {
            QModelIndex const index = m_model->index(i, 0, current);

            // Map proxy index to source model index
            QModelIndex sourceIndex = index;

            // Check if we're dealing with a proxy model
            if (const auto *proxyModel = qobject_cast<const QAbstractProxyModel *>(index.model()))
            {
                sourceIndex = proxyModel->mapToSource(index);
            }

            if (auto *node = static_cast<VDependencyNode *>(sourceIndex.internalPointer()); node->objectId == id)
            {
                current = index;
                found = true;
                break;
            }
        }
        if (!found)
        {
            return {};
        }
    }

    return current;
}

// ============================================================================
// VDependencyFilterProxyModel
// ============================================================================
VDependencyFilterProxyModel::VDependencyFilterProxyModel(QObject *parent)
  : QSortFilterProxyModel(parent)
{
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    setRecursiveFilteringEnabled(false); // Only filter root level
}

//---------------------------------------------------------------------------------------------------------------------
auto VDependencyFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const -> bool
{
    // Only filter root level
    if (sourceParent.isValid())
    {
        return true;
    }

    // Filter first-level items
    QModelIndex const index = sourceModel()->index(sourceRow, 0, sourceParent);
    QString const displayName = sourceModel()->data(index, Qt::DisplayRole).toString();

    return displayName.contains(filterRegularExpression());
}
