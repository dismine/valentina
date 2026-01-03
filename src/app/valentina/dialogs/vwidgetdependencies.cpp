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
#include "ui_vwidgetdependencies.h"

#include "../core/vdependencytreemodel.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vtools/tools/vabstracttool.h"
#include "../vwidgets/vcontrolpointspline.h"
#include "../vwidgets/vgraphicssimpletextitem.h"
#include "../vwidgets/vsimplecurve.h"
#include "../vwidgets/vsimplepoint.h"

#include <QGraphicsItem>

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
void ShowToolProperties(VAbstractTool *tool)
{
    if (auto *sceneItem = dynamic_cast<QGraphicsItem *>(tool))
    {
        if (auto *scene = sceneItem->scene())
        {
            const QList<QGraphicsView *> viewList = scene->views();
            if (!viewList.isEmpty())
            {
                if (auto *view = qobject_cast<VMainGraphicsView *>(viewList.at(0)))
                {
                    emit view->itemClicked(sceneItem);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto HighlightTool(vidtype id, bool show) -> vidtype
{
    try
    {
        if (auto *tool = qobject_cast<VAbstractTool *>(VAbstractPattern::getTool(id)))
        {
            tool->ShowVisualization(show);

            if (show)
            {
                ShowToolProperties(tool);
            }

            return id;
        }
    }
    catch (const VExceptionBadId &)
    {
        // do nothing
    }

    return NULL_ID;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VWidgetDependencies::VWidgetDependencies(VAbstractPattern *doc, QWidget *parent)
  : QWidget(parent),
    ui(new Ui::VWidgetDependencies),
    m_doc(doc),
    m_dependencyModel(new VDependencyTreeModel(this)),
    m_proxyModel(new VDependencyFilterProxyModel(this))
{
    m_dependencyModel->SetCurrentPattern(m_doc);

    m_proxyModel->setSourceModel(m_dependencyModel);

    ui->setupUi(this);
    ui->treeView->header()->hide();
    ui->treeView->setModel(m_proxyModel);

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
}

//---------------------------------------------------------------------------------------------------------------------
VWidgetDependencies::~VWidgetDependencies()
{
    delete ui;
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetDependencies::UpdateDependencies()
{
    if (VPatternBlockMapper const *blocks = m_doc->PatternBlockMapper(); m_indexPatternBlock != blocks->GetActiveId())
    {
        m_indexPatternBlock = blocks->GetActiveId();
        m_dependencyModel->ClearModel();

        m_dependencyModel->SetRootObjects(RootTools());
    }
    else
    {
        m_stateManager->SaveState();
        m_dependencyModel->UpdateTree(RootTools());
        m_stateManager->RestoreState();
    }
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
auto VWidgetDependencies::RootTools() const -> QVector<vidtype>
{
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
void VWidgetDependencies::OnNodeSelectionChanged(const QModelIndex &current, const QModelIndex &previous)
{
    if (m_doc == nullptr)
    {
        return;
    }

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
