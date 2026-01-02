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

#include <QGraphicsItem>

//---------------------------------------------------------------------------------------------------------------------
VWidgetDependencies::VWidgetDependencies(VAbstractPattern *doc, QWidget *parent)
  : QWidget(parent),
    ui(new Ui::VWidgetDependencies),
    m_doc(doc),
    m_model(new VDependencyTreeModel(this)),
    m_proxyModel(new VDependencyFilterProxyModel(this))
{
    m_model->SetCurrentPattern(m_doc);

    m_proxyModel->setSourceModel(m_model);

    ui->setupUi(this);
    ui->treeView->header()->hide();
    ui->treeView->setModel(m_proxyModel);

    // NOLINTNEXTLINE(cppcoreguidelines-prefer-member-initializer)
    m_stateManager = new VTreeStateManager(ui->treeView, m_model, this);

    connect(ui->lineEditFilter,
            &QLineEdit::textChanged,
            this,
            [this](const QString &text) -> void { m_proxyModel->setFilterFixedString(text); });
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
        m_model->ClearModel();

        m_model->SetRootObjects(RootTools());
    }
    else
    {
        m_stateManager->SaveState();
        m_model->UpdateTree(RootTools());
        m_stateManager->RestoreState();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VWidgetDependencies::ShowDependency(QGraphicsItem *item)
{
    if (item != nullptr && not item->isEnabled())
    {
        return;
    }
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
