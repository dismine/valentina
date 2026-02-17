/************************************************************************
 **
 **  @file   vdependencytreemodel.h
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
#ifndef VDEPENDENCYTREEMODEL_H
#define VDEPENDENCYTREEMODEL_H

#include "../vmisc/def.h"
#include "../vmisc/typedef.h"

#include <QAbstractItemModel>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QString>
#include <QTreeView>
#include <QVector>

class VAbstractPattern;
class VToolRecord;
class VContainer;

// Represents a single object in dependency tree
struct VDependencyNode
{
    vidtype objectId{0};
    QString displayName{};
    QVector<QSharedPointer<VDependencyNode>> children{};
    bool childrenLoaded{false};
    VDependencyNode *parent{nullptr};

    // Helper to build unique path for state tracking
    auto GetPath() const -> QString
    {
        QString path = QString::number(objectId);
        const VDependencyNode *p = parent;
        while (p != nullptr && p->objectId != 0)
        {
            path = QString::number(p->objectId) + '/' + path;
            p = p->parent;
        }
        return path;
    }
};

class VDependencyTreeModel : public QAbstractItemModel
{
    Q_OBJECT // NOLINT

public:
    explicit VDependencyTreeModel(QObject *parent = nullptr);
    ~VDependencyTreeModel() override = default;

    // Required QAbstractItemModel interface
    auto index(int row, int column, const QModelIndex &parent = QModelIndex()) const -> QModelIndex override;
    auto parent(const QModelIndex &child) const -> QModelIndex override;
    auto rowCount(const QModelIndex &parent = QModelIndex()) const -> int override;
    auto columnCount(const QModelIndex &parent = QModelIndex()) const -> int override;
    auto data(const QModelIndex &index, int role = Qt::DisplayRole) const -> QVariant override;
    auto headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const -> QVariant override;

    // For lazy loading
    auto hasChildren(const QModelIndex &parent = QModelIndex()) const -> bool override;
    auto canFetchMore(const QModelIndex &parent) const -> bool override;
    void fetchMore(const QModelIndex &parent) override;

    // Public API
    void SetRootObjects(const QVector<vidtype> &objectIds);
    void ClearModel();
    void SetCurrentPattern(const VAbstractPattern *doc);
    auto FindRootIndexByObjectId(vidtype objectId) const -> QModelIndex;

    // Update operations that preserve state
    void UpdateTree(const QVector<vidtype> &newRootObjects);
    void AddRootObject(vidtype objectId);
    void RemoveRootObject(vidtype objectId);
    void RefreshNode(const QString &objectPath); // Refresh specific node's children
    void ReorderRootObjects(const QVector<vidtype> &newOrder);

    // State management (to be used with QTreeView)
    void SetExpandedNodes(const QSet<QString> &expandedPaths);
    auto GetExpandedNodes() const -> QSet<QString>;

private:
    Q_DISABLE_COPY_MOVE(VDependencyTreeModel)

    QSharedPointer<VDependencyNode> m_rootNode;
    const VAbstractPattern *m_doc{nullptr};
    QSet<QString> m_expandedNodes{}; // Track expanded state internally

    auto GetNode(const QModelIndex &index) const -> VDependencyNode *;
    auto FindNodeByPath(const QString &path, VDependencyNode *root = nullptr) -> VDependencyNode *;
    void LoadDependencies(const QModelIndex &parentIndex, VDependencyNode *node);
    auto FetchDependenciesForObject(vidtype objectId) const -> QVector<vidtype>;
    auto GetDisplayNameForObject(vidtype objectId) const -> QString; // Fetch display name
    auto GetDisplayToolTipForObject(vidtype objectId) const -> QString;

    // Smart update helpers
    void UpdateNodeChildren(VDependencyNode *node, const QVector<vidtype> &newDependencies);
    void RemoveChildAt(VDependencyNode *parent, int index);
    void InsertChild(VDependencyNode *parent, int index, vidtype objectId);

    auto FindToolRecord(vidtype id) const -> VToolRecord;
    auto GetToolName(vidtype id, const VContainer &patternData, const QString &defaultName) const -> QString;
    auto GetOperationToolName(Tool toolType, const QString &defaultName) const -> QString;
    auto GetToolToolTip(vidtype id) const -> QString;
    auto GetLineToolName(vidtype id, const QString &defaultName) const -> QString;
};

// ============================================================================
// HELPER CLASS: TreeStateManager - manages expanded state automatically
// ============================================================================

class VTreeStateManager : public QObject
{
    Q_OBJECT // NOLINT

public:
    VTreeStateManager(QTreeView *view, VDependencyTreeModel *model, QObject *parent = nullptr);
    ~VTreeStateManager() override = default;

    void SaveState();
    void RestoreState();

private slots:
    void OnExpanded(const QModelIndex &index);
    void OnCollapsed(const QModelIndex &index);

private:
    Q_DISABLE_COPY_MOVE(VTreeStateManager)

    QTreeView *m_view;
    VDependencyTreeModel *m_model;
    QSet<QString> m_expandedPaths{};

    void SaveExpandedState(const QModelIndex &parent);

    auto FindIndexByPath(const QString &path) -> QModelIndex;
};

// ============================================================================
// HELPER CLASS: VDependencyFilterProxyModel - sorting dependency tree
// ============================================================================

class VDependencyFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT // NOLINT

public:
    explicit VDependencyFilterProxyModel(QObject *parent = nullptr);
    ~VDependencyFilterProxyModel() override = default;

protected:
    auto filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const -> bool override;

private:
    Q_DISABLE_COPY_MOVE(VDependencyFilterProxyModel)
};

#endif // VDEPENDENCYTREEMODEL_H
