/************************************************************************
 **
 **  @file   vwidgetdependencies.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   31 12, 2025
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
#include "vwidgetdependencies.h"
#include "dialogmovesteps.h"
#include "ui_vwidgetdependencies.h"

#include "../core/vdependencytreemodel.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vtools/tools/vabstracttool.h"
#include "../vtools/undocommands/movetool.h"
#include "../vwidgets/vcontrolpointspline.h"
#include "../vwidgets/vgraphicssimpletextitem.h"
#include "../vwidgets/vsimplecurve.h"
#include "../vwidgets/vsimplepoint.h"

#include <QClipboard>
#include <QGraphicsItem>
#include <QMenu>

#include <../vwidgets/vmaingraphicsview.h>

namespace
{
auto FindItemTool_r(QGraphicsItem *item) -> QGraphicsItem *
{
    if (item == nullptr)
    {
        return item;
    }

    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 64, "List of tools changed.");

    switch (item->type())
    {
        case VGraphicsSimpleTextItem::Type:
        case VControlPointSpline::Type:
        case VSimplePoint::Type:
        case VSimpleCurve::Type:
            return FindItemTool_r(item->parentItem());
        default:
            break;
    }

    return item;
}

//---------------------------------------------------------------------------------------------------------------------
auto ItemBoundingRect(QGraphicsItem *item) -> QRectF
{
    if (item == nullptr)
    {
        return {};
    }

    QRectF childrenRect = item->childrenBoundingRect();
    // map to scene coordinate.
    childrenRect.translate(item->scenePos());

    QRectF itemRect = item->boundingRect();
    // map to scene coordinate.
    itemRect.translate(item->scenePos());

    QRectF boundingRect;
    boundingRect = boundingRect.united(itemRect);
    boundingRect = boundingRect.united(childrenRect);
    return boundingRect;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VWidgetDependencies::VWidgetDependencies(VAbstractPattern *doc, QWidget *parent)
  : QWidget(parent),
    ui(new Ui::VWidgetDependencies),
    m_doc(doc),
    m_dependencyModel(new VDependencyTreeModel(this)),
    m_proxyModel(new VDependencyFilterProxyModel(this)),
    m_historyManager(doc)
{
    m_dependencyModel->SetCurrentPattern(m_doc);

    m_proxyModel->setSourceModel(m_dependencyModel);

    ui->setupUi(this);
    ui->treeView->header()->hide();
    ui->treeView->setModel(m_proxyModel);

    ui->toolButtonTop->setIcon(FromTheme(VThemeIcon::ObjectOrderFront));
    ui->toolButtonUp->setIcon(FromTheme(VThemeIcon::ObjectOrderRaise));
    ui->toolButtonDown->setIcon(FromTheme(VThemeIcon::ObjectOrderLower));
    ui->toolButtonBottom->setIcon(FromTheme(VThemeIcon::ObjectOrderBack));

    connect(ui->toolButtonTop, &QToolButton::clicked, this, &VWidgetDependencies::MoveTop);
    connect(ui->toolButtonUp, &QToolButton::clicked, this, &VWidgetDependencies::MoveUp);
    connect(ui->toolButtonDown, &QToolButton::clicked, this, &VWidgetDependencies::MoveDown);
    connect(ui->toolButtonBottom, &QToolButton::clicked, this, &VWidgetDependencies::MoveBottom);

    // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
    m_stateManager = new VTreeStateManager(ui->treeView, m_dependencyModel, this);

    connect(ui->lineEditFilter,
            &QLineEdit::textChanged,
            this,
            [this](const QString &text) -> void { m_proxyModel->setFilterFixedString(text); });
    connect(ui->treeView->selectionModel(),
            &QItemSelectionModel::currentChanged,
            this,
            &VWidgetDependencies::OnNodeSelectionChanged);

    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, &QTreeView::customContextMenuRequested, this, &VWidgetDependencies::OnContextMenuRequested);
}

//---------------------------------------------------------------------------------------------------------------------
VWidgetDependencies::~VWidgetDependencies()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetDependencies::UpdateDependencies()
{
    if (m_doc == nullptr)
    {
        return;
    }

    m_historyManager.RebuildIndex();

    if (VPatternBlockMapper const *blocks = m_doc->PatternBlockMapper(); m_indexPatternBlock != blocks->GetActiveId())
    {
        m_indexPatternBlock = blocks->GetActiveId();
        m_dependencyModel->ClearModel();

        m_dependencyModel->SetRootObjects(m_historyManager.RootTools());
    }
    else
    {
        m_stateManager->SaveState();
        m_dependencyModel->UpdateTree(m_historyManager.RootTools());
        m_stateManager->RestoreState();
    }

    EnableMoveButtons(ui->treeView->currentIndex());
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetDependencies::ShowDependency(QGraphicsItem *item)
{
    if (item == nullptr || !item->isEnabled())
    {
        if (m_activeTool != NULL_ID)
        {
            HighlightTool(m_activeTool, false);
            m_activeTool = NULL_ID;
        }
        return;
    }

    auto *toolItem = FindItemTool_r(item);

    const auto *tool = dynamic_cast<VAbstractTool *>(toolItem);
    if (tool == nullptr)
    {
        return;
    }

    vidtype const objectId = tool->getId();

    // Find in source model
    const QModelIndex sourceIndex = m_dependencyModel->FindRootIndexByObjectId(objectId);

    if (!sourceIndex.isValid())
    {
        return; // Object not found
    }

    // Map to proxy model
    const QModelIndex proxyIndex = m_proxyModel->mapFromSource(sourceIndex);

    if (!proxyIndex.isValid())
    {
        // Object is filtered out
        return;
    }

    // Select and scroll to the item
    QSignalBlocker const blocker(ui->treeView->selectionModel());
    ui->treeView->setCurrentIndex(proxyIndex);
    ui->treeView->scrollTo(proxyIndex);
    ui->treeView->expand(proxyIndex);
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetDependencies::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VWidgetDependencies::HighlightObject(vidtype id, int indexPatternBlock, bool show) const -> vidtype
{
    if (indexPatternBlock < 0)
    {
        return NULL_ID;
    }

    VContainer const patternData = m_doc->GetCompletePPData(m_doc->PatternBlockMapper()->FindName(indexPatternBlock));

    try
    {
        const QSharedPointer<VGObject> obj = patternData.GetGObject(id);
        return HighlightTool(obj->getIdTool(), show);
    }
    catch (const VExceptionBadId &)
    {
        // do nothing
    }
    return NULL_ID;
}

//---------------------------------------------------------------------------------------------------------------------
auto VWidgetDependencies::ObjectId(const QModelIndex &index) const -> vidtype
{
    if (!index.isValid())
    {
        // No selection
        return NULL_ID;
    }

    // Map proxy index to source index
    QModelIndex const sourceIndex = m_proxyModel->mapToSource(index);

    if (!sourceIndex.isValid())
    {
        return NULL_ID;
    }

    // Get the node
    const auto *node = static_cast<VDependencyNode *>(sourceIndex.internalPointer());
    if (node == nullptr)
    {
        return NULL_ID;
    }

    return node->objectId;
}

//---------------------------------------------------------------------------------------------------------------------
auto VWidgetDependencies::CurrentObjectId() const -> vidtype
{
    const QModelIndex currentModel = ui->treeView->currentIndex();
    if (!currentModel.isValid() || currentModel.parent().isValid())
    {
        return NULL_ID;
    }

    return ObjectId(currentModel);
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetDependencies::ExpandAllChildren(const QModelIndex &index)
{
    if (!index.isValid())
    {
        return;
    }

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);

    ui->treeView->expand(index);

    int const rows = index.model()->rowCount(index);
    for (int i = 0; i < rows; ++i)
    {
        const QModelIndex child = index.model()->index(i, 0, index);
        ExpandAllChildren(child);
    }

    QGuiApplication::restoreOverrideCursor();
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetDependencies::CollapseAllChildren(const QModelIndex &index)
{
    if (!index.isValid())
    {
        return;
    }

    int const rows = index.model()->rowCount(index);
    for (int i = 0; i < rows; ++i)
    {
        const QModelIndex child = index.model()->index(i, 0, index);
        CollapseAllChildren(child);
    }

    ui->treeView->collapse(index);
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetDependencies::GoToObject(vidtype id) const
{
    if (id == NULL_ID)
    {
        return;
    }

    VDataTool *tool = nullptr;

    try
    {
        tool = VAbstractPattern::getTool(id);
    }
    catch (const VExceptionBadId &)
    {
        return;
    }

    if (auto *sceneItem = dynamic_cast<QGraphicsItem *>(tool))
    {
        bool makeInvisible = false;
        if (!sceneItem->isVisible())
        {
            sceneItem->setVisible(true);
            makeInvisible = true;
        }

        const QRectF rect = ItemBoundingRect(sceneItem);

        if (makeInvisible)
        {
            sceneItem->setVisible(false);
        }

        if (rect.isEmpty())
        {
            return;
        }

        emit ShowTool(rect);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetDependencies::EnableMoveButtons(const QModelIndex &current)
{
    ui->toolButtonTop->setDisabled(true);
    ui->toolButtonUp->setDisabled(true);
    ui->toolButtonDown->setDisabled(true);
    ui->toolButtonBottom->setDisabled(true);

    if (!current.isValid() || current.parent().isValid())
    {
        return;
    }

    if (m_doc == nullptr || !m_doc->IsPatternGraphComplete())
    {
        return;
    }

    const vidtype objectId = ObjectId(current);

    if (m_historyManager.CanMoveUp(objectId))
    {
        ui->toolButtonTop->setEnabled(true);
        ui->toolButtonUp->setEnabled(true);
    }

    if (m_historyManager.CanMoveDown(objectId))
    {
        ui->toolButtonDown->setEnabled(true);
        ui->toolButtonBottom->setEnabled(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetDependencies::OnNodeSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (m_doc == nullptr)
    {
        return;
    }

    EnableMoveButtons(current);

    VPatternGraph const *graph = m_doc->PatternGraph();

    if (const vidtype previousObjectId = ObjectId(previous); previousObjectId != NULL_ID)
    {
        auto previousVertex = graph->GetVertex(previousObjectId);
        if (!previousVertex)
        {
            return;
        }

        switch (previousVertex->type)
        {
            case VNodeType::OBJECT:
                HighlightObject(previousVertex->id, previousVertex->indexPatternBlock, false);
                break;
            case VNodeType::TOOL:
                HighlightTool(previousVertex->id, false);
                break;
            default:
                break;
        }
    }

    const vidtype currentObjectId = ObjectId(current);
    if (currentObjectId == NULL_ID)
    {
        // No selection
        return;
    }

    auto currentVertex = graph->GetVertex(currentObjectId);
    if (!currentVertex)
    {
        return;
    }

    switch (currentVertex->type)
    {
        case VNodeType::OBJECT:
            m_activeTool = HighlightObject(currentVertex->id, currentVertex->indexPatternBlock, true);
            break;
        case VNodeType::TOOL:
            m_activeTool = HighlightTool(currentVertex->id, true);
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetDependencies::OnContextMenuRequested(const QPoint &pos)
{
    QModelIndex const proxyIndex = ui->treeView->indexAt(pos);
    if (!proxyIndex.isValid())
    {
        return; // No item at this position
    }

    // Map to source model
    QModelIndex const sourceIndex = m_proxyModel->mapToSource(proxyIndex);
    const auto *node = static_cast<VDependencyNode *>(sourceIndex.internalPointer());

    if (node == nullptr)
    {
        return;
    }

    // Create context menu
    QMenu contextMenu(this);

    QAction const *actionExpand = contextMenu.addAction(tr("Expand All"));
    QAction const *actionCollapse = contextMenu.addAction(tr("Collapse All"));
    contextMenu.addSeparator();
    QAction const *actionCopyId = contextMenu.addAction(tr("Copy ID"));
    contextMenu.addSeparator();
    QAction *actionGoToObject = contextMenu.addAction(FromTheme(VThemeIcon::SystemSearch), tr("Go to Object"));

    if (m_doc != nullptr)
    {
        if (auto vertex = m_doc->PatternGraph()->GetVertex(node->objectId); vertex)
        {
            actionGoToObject->setEnabled(vertex->type != VNodeType::PIECE);
        }
    }

    // Show menu and get selected action
    QAction const *selectedAction = contextMenu.exec(ui->treeView->viewport()->mapToGlobal(pos));

    if (selectedAction == actionExpand)
    {
        ExpandAllChildren(proxyIndex);
    }
    else if (selectedAction == actionCollapse)
    {
        CollapseAllChildren(proxyIndex);
    }
    else if (selectedAction == actionCopyId)
    {
        QApplication::clipboard()->setText(QString::number(node->objectId));
    }
    else if (selectedAction == actionGoToObject)
    {
        GoToObject(node->objectId);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetDependencies::MoveTop()
{
    if (m_doc == nullptr)
    {
        return;
    }

    const vidtype currentObjectId = CurrentObjectId();
    if (currentObjectId == NULL_ID)
    {
        return;
    }

    const vidtype topId = m_historyManager.CalculateTopId(currentObjectId);
    if (topId != NULL_ID)
    {
        auto *moveUp = new MoveToolUp(m_doc, currentObjectId, topId);
        connect(moveUp, &MoveToolUp::NeedFullParsing, m_doc, &VAbstractPattern::NeedFullParsing);
        VAbstractApplication::VApp()->getUndoStack()->push(moveUp);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetDependencies::MoveUp()
{
    if (m_doc == nullptr)
    {
        return;
    }

    const vidtype currentObjectId = CurrentObjectId();
    if (currentObjectId == NULL_ID)
    {
        return;
    }

    const vsizetype maxSteps = m_historyManager.CalculateMaxUpSteps(currentObjectId);
    if (maxSteps == 0)
    {
        return;
    }

    DialogMoveSteps dialog(static_cast<int>(maxSteps), this);
    dialog.SetMoveSteps(1);
    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    if (const vidtype upId = m_historyManager.CalculateUpId(currentObjectId, dialog.GetMoveSteps()); upId != NULL_ID)
    {
        auto *moveUp = new MoveToolUp(m_doc, currentObjectId, upId);
        connect(moveUp, &MoveToolUp::NeedFullParsing, m_doc, &VAbstractPattern::NeedFullParsing);
        VAbstractApplication::VApp()->getUndoStack()->push(moveUp);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetDependencies::MoveDown()
{
    if (m_doc == nullptr)
    {
        return;
    }

    const vidtype currentObjectId = CurrentObjectId();
    if (currentObjectId == NULL_ID)
    {
        return;
    }

    const vsizetype maxSteps = m_historyManager.CalculateMaxDownSteps(currentObjectId);
    if (maxSteps == 0)
    {
        return;
    }

    DialogMoveSteps dialog(static_cast<int>(maxSteps), this);
    dialog.SetMoveSteps(1);
    if (dialog.exec() == QDialog::Rejected)
    {
        return;
    }

    const vidtype downId = m_historyManager.CalculateDownId(currentObjectId, dialog.GetMoveSteps());
    auto *moveDown = new MoveToolDown(m_doc, currentObjectId, downId);
    connect(moveDown, &MoveToolDown::NeedFullParsing, m_doc, &VAbstractPattern::NeedFullParsing);
    VAbstractApplication::VApp()->getUndoStack()->push(moveDown);
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetDependencies::MoveBottom()
{
    if (m_doc == nullptr)
    {
        return;
    }

    const vidtype currentObjectId = CurrentObjectId();
    if (currentObjectId == NULL_ID)
    {
        return;
    }

    const vidtype bottomId = m_historyManager.CalculateBottomId(currentObjectId);
    auto *moveDown = new MoveToolDown(m_doc, currentObjectId, bottomId);
    connect(moveDown, &MoveToolDown::NeedFullParsing, m_doc, &VAbstractPattern::NeedFullParsing);
    VAbstractApplication::VApp()->getUndoStack()->push(moveDown);
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetDependencies::ShowToolProperties(VAbstractTool *tool, bool show) const
{
    if (auto *sceneItem = dynamic_cast<QGraphicsItem *>(tool))
    {
        emit ShowProperties(show ? sceneItem : nullptr);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VWidgetDependencies::HighlightTool(vidtype id, bool show) const -> vidtype
{
    try
    {
        if (auto *tool = qobject_cast<VAbstractTool *>(VAbstractPattern::getTool(id)))
        {
            tool->ShowVisualization(show);
            ShowToolProperties(tool, show);
            return id;
        }
    }
    catch (const VExceptionBadId &)
    {
        // do nothing
    }

    return NULL_ID;
}
