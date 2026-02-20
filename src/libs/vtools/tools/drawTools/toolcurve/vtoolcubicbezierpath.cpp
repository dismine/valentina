/************************************************************************
 **
 **  @file   vtoolcubicbezierpath.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 3, 2016
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

#include "vtoolcubicbezierpath.h"

#include <QDomElement>
#include <QPen>
#include <QSharedPointer>

#include "../../../dialogs/tools/dialogcubicbezierpath.h"
#include "../../../dialogs/tools/dialogtool.h"
#include "../../../visualization/path/vistoolcubicbezierpath.h"
#include "../../../visualization/visualization.h"
#include "../../vabstracttool.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vdomdocument.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vgeometry/vabstractcubicbezierpath.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/exception/vexception.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vtoolabstractcurve.h"

const QString VToolCubicBezierPath::ToolType = QStringLiteral("cubicBezierPath");

//---------------------------------------------------------------------------------------------------------------------
VToolCubicBezierPath::VToolCubicBezierPath(const VToolCubicBezierPathInitData &initData, QGraphicsItem *parent)
  : VToolAbstractBezier(initData.doc, initData.data, initData.id, initData.notes, parent)
{
    SetSceneType(SceneObject::SplinePath);

    this->setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus

    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierPath::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    auto *dialogTool = qobject_cast<DialogCubicBezierPath *>(m_dialog);
    SCASSERT(dialogTool != nullptr)
    const QSharedPointer<VCubicBezierPath> splPath = VAbstractTool::data.GeometricObject<VCubicBezierPath>(m_id);
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetPath(*splPath);
    dialogTool->SetNotes(m_notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCubicBezierPath::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                                  VContainer *data) -> VToolCubicBezierPath *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogCubicBezierPath> dialogTool = qobject_cast<DialogCubicBezierPath *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolCubicBezierPathInitData initData;
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();
    initData.path = new VCubicBezierPath(dialogTool->GetPath());

    VToolCubicBezierPath *spl = Create(initData);
    if (spl != nullptr)
    {
        spl->m_dialog = dialog;
    }
    return spl;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCubicBezierPath::Create(VToolCubicBezierPathInitData initData) -> VToolCubicBezierPath *
{
    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->AddGObject(initData.path);
    }
    else
    {
        initData.data->UpdateGObject(initData.id, initData.path);
    }

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->AddVertex(initData.id, VNodeType::TOOL, initData.doc->PatternBlockMapper()->GetActiveId());

    initData.data->AddCurveWithSegments(initData.data->GeometricObject<VAbstractCubicBezierPath>(initData.id),
                                        initData.id);

    for (qint32 i = 0; i < initData.path->CountPoints(); ++i)
    {
        patternGraph->AddEdge((*initData.path)[i].id(), initData.id);
    }

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::CubicBezierPath, initData.doc);
        auto *spl = new VToolCubicBezierPath(initData);
        initData.scene->addItem(spl);
        InitSplinePathToolConnections(initData.scene, spl);
        VAbstractPattern::AddTool(initData.id, spl);
        return spl;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierPath::UpdatePathPoints(VAbstractPattern *doc, QDomElement &element, const VCubicBezierPath &path)
{
    VDomDocument::RemoveAllChildren(element);
    for (qint32 i = 0; i < path.CountPoints(); ++i)
    {
        AddPathPoint(doc, element, path.at(i));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCubicBezierPath::getSplinePath() const -> VCubicBezierPath
{
    QSharedPointer<VCubicBezierPath> const splPath = VAbstractTool::data.GeometricObject<VCubicBezierPath>(m_id);
    return *splPath.data();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierPath::setSplinePath(const VCubicBezierPath &splPath)
{
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    QSharedPointer<VCubicBezierPath> const splinePath = qSharedPointerDynamicCast<VCubicBezierPath>(obj);
    *splinePath.data() = splPath;
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierPath::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolCubicBezierPath>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierPath::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    Q_UNUSED(id)
    try
    {
        ContextMenu<DialogCubicBezierPath>(event);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierPath::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    auto *const dialogTool = qobject_cast<DialogCubicBezierPath *>(m_dialog);
    SCASSERT(dialogTool != nullptr)

    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrNotes, dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });

    SetSplinePathAttributes(domElement, dialogTool->GetPath());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierPath::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolAbstractBezier::SaveOptions(tag, obj);

    QSharedPointer<VCubicBezierPath> const splPath = qSharedPointerDynamicCast<VCubicBezierPath>(obj);
    SCASSERT(splPath.isNull() == false)

    SetSplinePathAttributes(tag, *splPath);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierPath::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolCubicBezierPath *>(vis);
        SCASSERT(visual != nullptr)

        QSharedPointer<VCubicBezierPath> const splPath = VAbstractTool::data.GeometricObject<VCubicBezierPath>(m_id);
        visual->SetPath(*splPath.data());
        visual->SetLineStyle(LineStyleToPenStyle(splPath->GetPenStyle()));
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierPath::RefreshGeometry()
{
    QSharedPointer<VCubicBezierPath> const splPath = VAbstractTool::data.GeometricObject<VCubicBezierPath>(m_id);
    setPath(splPath->GetPath());

    SetVisualization();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierPath::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    ProcessSplinePathToolOptions(oldDomElement, newDomElement, GatherToolChanges());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierPath::AddPathPoint(VAbstractPattern *doc, QDomElement &domElement, const VPointF &splPoint)
{
    SCASSERT(doc != nullptr)
    QDomElement pathPoint = doc->createElement(TagPathPoint);
    doc->SetAttribute(pathPoint, AttrPSpline, splPoint.id());
    domElement.appendChild(pathPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezierPath::SetSplinePathAttributes(QDomElement &domElement, const VCubicBezierPath &path)
{
    doc->SetAttribute(domElement, AttrType, ToolType);
    doc->SetAttributeOrRemoveIf<quint32>(domElement, AttrDuplicate, path.GetDuplicate(),
                                         [](quint32 duplicate) noexcept { return duplicate <= 0; });
    doc->SetAttribute(domElement, AttrColor, path.GetColor());
    doc->SetAttribute(domElement, AttrPenStyle, path.GetPenStyle());
    doc->SetAttribute(domElement, AttrAScale, path.GetApproximationScale());

    UpdatePathPoints(doc, domElement, path);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCubicBezierPath::GatherToolChanges() const -> VToolAbstractBezier::ToolChanges
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogCubicBezierPath> dialogTool = qobject_cast<DialogCubicBezierPath *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    const VCubicBezierPath newCurve = dialogTool->GetPath();
    const auto oldCurve = VAbstractTool::data.GeometricObject<VAbstractCubicBezierPath>(m_id);

    return {.oldP1Label = oldCurve->FirstPoint().name(),
            .newP1Label = newCurve.FirstPoint().name(),
            .oldP4Label = oldCurve->LastPoint().name(),
            .newP4Label = newCurve.LastPoint().name(),
            .oldAliasSuffix = oldCurve->GetAliasSuffix(),
            .newAliasSuffix = newCurve.GetAliasSuffix()};
}
