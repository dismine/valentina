/************************************************************************
 **
 **  @file   vtooltruedarts.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "vtooltruedarts.h"

#include <QLine>
#include <QLineF>
#include <QSharedPointer>

#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../dialogs/tools/dialogtruedarts.h"
#include "../../../../visualization/line/vistooltruedarts.h"
#include "../../../../visualization/visualization.h"
#include "../../../vabstracttool.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/exception/vexception.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vtooldoublepoint.h"

template <class T> class QSharedPointer;

const QString VToolTrueDarts::ToolType = QStringLiteral("trueDarts"); // NOLINT

//---------------------------------------------------------------------------------------------------------------------
VToolTrueDarts::VToolTrueDarts(const VToolTrueDartsInitData &initData, QGraphicsItem *parent)
  : VToolDoublePoint(initData.doc, initData.data, initData.id, initData.p1id, initData.p2id, initData.notes, parent),
    baseLineP1Id(initData.baseLineP1Id),
    baseLineP2Id(initData.baseLineP2Id),
    dartP1Id(initData.dartP1Id),
    dartP2Id(initData.dartP2Id),
    dartP3Id(initData.dartP3Id)
{
    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolTrueDarts::GatherToolChanges() const -> VToolDoublePoint::ToolChanges
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogTrueDarts> dialogTool = qobject_cast<DialogTrueDarts *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    return {
        .oldP1Label = VAbstractTool::data.GetGObject(p1id)->name(),
        .newP1Label = dialogTool->GetFirstNewDartPointName(),
        .oldP2Label = VAbstractTool::data.GetGObject(p2id)->name(),
        .newP2Label = dialogTool->GetSecondNewDartPointName(),
    };
}

//---------------------------------------------------------------------------------------------------------------------
void VToolTrueDarts::FindPoint(const QPointF &baseLineP1, const QPointF &baseLineP2, const QPointF &dartP1,
                               const QPointF &dartP2, const QPointF &dartP3, QPointF &p1, QPointF &p2)
{
    const QLineF d2d1(dartP2, dartP1);
    const QLineF d2d3(dartP2, dartP3);

    const qreal degrees = d2d3.angleTo(d2d1);

    QLineF d2blP2(dartP2, baseLineP2);
    d2blP2.setAngle(d2blP2.angle() + degrees);

    if (QLineF(baseLineP1, d2blP2.p2()).intersects(d2d1, &p1) == QLineF::NoIntersection)
    {
        p1 = QPointF(0, 0);
        p2 = QPointF(0, 0);
        return;
    }

    QLineF d2p1(dartP2, p1);
    d2p1.setAngle(d2p1.angle() - degrees);
    p2 = d2p1.p2();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolTrueDarts::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogTrueDarts> dialogTool = qobject_cast<DialogTrueDarts *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    const QSharedPointer<VPointF> p1 = VAbstractTool::data.GeometricObject<VPointF>(p1id);
    const QSharedPointer<VPointF> p2 = VAbstractTool::data.GeometricObject<VPointF>(p2id);

    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetChildrenId(p1id, p2id);
    dialogTool->SetNewDartPointNames(p1->name(), p2->name());
    dialogTool->SetFirstBasePointId(baseLineP1Id);
    dialogTool->SetSecondBasePointId(baseLineP2Id);
    dialogTool->SetFirstDartPointId(dartP1Id);
    dialogTool->SetSecondDartPointId(dartP2Id);
    dialogTool->SetThirdDartPointId(dartP3Id);
    dialogTool->SetNotes(m_notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolTrueDarts::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                            VContainer *data) -> VToolTrueDarts *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogTrueDarts> dialogTool = qobject_cast<DialogTrueDarts *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolTrueDartsInitData initData;
    initData.name1 = dialogTool->GetFirstNewDartPointName();
    initData.name2 = dialogTool->GetSecondNewDartPointName();
    initData.baseLineP1Id = dialogTool->GetFirstBasePointId();
    initData.baseLineP2Id = dialogTool->GetSecondBasePointId();
    initData.dartP1Id = dialogTool->GetFirstDartPointId();
    initData.dartP2Id = dialogTool->GetSecondDartPointId();
    initData.dartP3Id = dialogTool->GetThirdDartPointId();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();

    VToolTrueDarts *point = Create(initData);
    if (point != nullptr)
    {
        point->m_dialog = dialog;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolTrueDarts::Create(VToolTrueDartsInitData initData) -> VToolTrueDarts *
{
    const QSharedPointer<VPointF> baseLineP1 = initData.data->GeometricObject<VPointF>(initData.baseLineP1Id);
    const QSharedPointer<VPointF> baseLineP2 = initData.data->GeometricObject<VPointF>(initData.baseLineP2Id);
    const QSharedPointer<VPointF> dartP1 = initData.data->GeometricObject<VPointF>(initData.dartP1Id);
    const QSharedPointer<VPointF> dartP2 = initData.data->GeometricObject<VPointF>(initData.dartP2Id);
    const QSharedPointer<VPointF> dartP3 = initData.data->GeometricObject<VPointF>(initData.dartP3Id);

    QPointF fPoint1;
    QPointF fPoint2;
    VToolTrueDarts::FindPoint(static_cast<QPointF>(*baseLineP1), static_cast<QPointF>(*baseLineP2),
                              static_cast<QPointF>(*dartP1), static_cast<QPointF>(*dartP2),
                              static_cast<QPointF>(*dartP3), fPoint1, fPoint2);

    auto *p1 = new VPointF(fPoint1, initData.name1, initData.mx1, initData.my1, initData.id);
    p1->SetShowLabel(initData.showLabel1);

    auto *p2 = new VPointF(fPoint2, initData.name2, initData.mx2, initData.my2, initData.id);
    p2->SetShowLabel(initData.showLabel2);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->getNextId(); // Just reserve id for tool
        initData.p1id = initData.data->AddGObject(p1);
        initData.p2id = initData.data->AddGObject(p2);
    }
    else
    {
        initData.data->UpdateGObject(initData.p1id, p1);
        initData.data->UpdateGObject(initData.p2id, p2);
    }

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->AddVertex(initData.id, VNodeType::TOOL, initData.doc->PatternBlockMapper()->GetActiveId());

    patternGraph->AddEdge(initData.baseLineP1Id, initData.id);
    patternGraph->AddEdge(initData.baseLineP2Id, initData.id);
    patternGraph->AddEdge(initData.dartP1Id, initData.id);
    patternGraph->AddEdge(initData.dartP2Id, initData.id);
    patternGraph->AddEdge(initData.dartP3Id, initData.id);

    patternGraph->AddVertex(initData.p1id, VNodeType::OBJECT, initData.doc->PatternBlockMapper()->GetActiveId());
    patternGraph->AddVertex(initData.p2id, VNodeType::OBJECT, initData.doc->PatternBlockMapper()->GetActiveId());

    patternGraph->AddEdge(initData.id, initData.p1id);
    patternGraph->AddEdge(initData.id, initData.p2id);

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::TrueDarts, initData.doc);
        auto *points = new VToolTrueDarts(initData);
        initData.scene->addItem(points);
        InitToolConnections(initData.scene, points);
        VAbstractPattern::AddTool(initData.id, points);
        return points;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolTrueDarts::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolTrueDarts>(show);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolTrueDarts::BaseLineP1Name() const -> QString
{
    return VAbstractTool::data.GetGObject(baseLineP1Id)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolTrueDarts::BaseLineP2Name() const -> QString
{
    return VAbstractTool::data.GetGObject(baseLineP2Id)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolTrueDarts::DartP1Name() const -> QString
{
    return VAbstractTool::data.GetGObject(dartP1Id)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolTrueDarts::DartP2Name() const -> QString
{
    return VAbstractTool::data.GetGObject(dartP2Id)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolTrueDarts::DartP3Name() const -> QString
{
    return VAbstractTool::data.GetGObject(dartP3Id)->name();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolTrueDarts::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogTrueDarts>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolTrueDarts::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogTrueDarts> dialogTool = qobject_cast<DialogTrueDarts *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrName1, dialogTool->GetFirstNewDartPointName());
    doc->SetAttribute(domElement, AttrName2, dialogTool->GetSecondNewDartPointName());
    doc->SetAttribute(domElement, AttrBaseLineP1, QString().setNum(dialogTool->GetFirstBasePointId()));
    doc->SetAttribute(domElement, AttrBaseLineP2, QString().setNum(dialogTool->GetSecondBasePointId()));
    doc->SetAttribute(domElement, AttrDartP1, QString().setNum(dialogTool->GetFirstDartPointId()));
    doc->SetAttribute(domElement, AttrDartP2, QString().setNum(dialogTool->GetSecondDartPointId()));
    doc->SetAttribute(domElement, AttrDartP3, QString().setNum(dialogTool->GetThirdDartPointId()));
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrNotes,
                                         dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept -> bool { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolTrueDarts::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolDoublePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrBaseLineP1, baseLineP1Id);
    doc->SetAttribute(tag, AttrBaseLineP2, baseLineP2Id);
    doc->SetAttribute(tag, AttrDartP1, dartP1Id);
    doc->SetAttribute(tag, AttrDartP2, dartP2Id);
    doc->SetAttribute(tag, AttrDartP3, dartP3Id);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolTrueDarts::ReadToolAttributes(const QDomElement &domElement)
{
    VToolDoublePoint::ReadToolAttributes(domElement);

    baseLineP1Id = VAbstractPattern::GetParametrUInt(domElement, AttrBaseLineP1, NULL_ID_STR);
    baseLineP2Id = VAbstractPattern::GetParametrUInt(domElement, AttrBaseLineP2, NULL_ID_STR);
    dartP1Id = VAbstractPattern::GetParametrUInt(domElement, AttrDartP1, NULL_ID_STR);
    dartP2Id = VAbstractPattern::GetParametrUInt(domElement, AttrDartP2, NULL_ID_STR);
    dartP3Id = VAbstractPattern::GetParametrUInt(domElement, AttrDartP3, NULL_ID_STR);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolTrueDarts::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolTrueDarts *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetBaseLineP1Id(baseLineP1Id);
        visual->SetBaseLineP2Id(baseLineP2Id);
        visual->SetD1PointId(dartP1Id);
        visual->SetD2PointId(dartP2Id);
        visual->SetD3PointId(dartP3Id);
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolTrueDarts::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    ProcessTrueDartsToolOptions(oldDomElement, newDomElement, GatherToolChanges());
}
