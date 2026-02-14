/************************************************************************
 **
 **  @file   vtooltriangle.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#include "vtooltriangle.h"

#include <QLine>
#include <QLineF>
#include <QSharedPointer>
#include <QtMath>

#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../dialogs/tools/dialogtriangle.h"
#include "../../../../visualization/line/vistooltriangle.h"
#include "../../../../visualization/visualization.h"
#include "../../../vabstracttool.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/exception/vexception.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vtoolsinglepoint.h"

template <class T> class QSharedPointer;

const QString VToolTriangle::ToolType = QStringLiteral("triangle"); // NOLINT

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolTriangle constructor.
 * @param initData init data.
 * @param parent parent object.
 */
VToolTriangle::VToolTriangle(const VToolTriangleInitData &initData, QGraphicsItem *parent)
  : VToolSinglePoint(initData.doc, initData.data, initData.id, initData.notes, parent),
    axisP1Id(initData.axisP1Id),
    axisP2Id(initData.axisP2Id),
    firstPointId(initData.firstPointId),
    secondPointId(initData.secondPointId)
{
    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolTriangle::GatherToolChanges() const -> VAbstractPoint::ToolChanges
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogTriangle> dialogTool = qobject_cast<DialogTriangle *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    return {.pointId = m_id, .oldLabel = name(), .newLabel = dialogTool->GetPointName()};
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolTriangle::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogTriangle> dialogTool = qobject_cast<DialogTriangle *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetAxisP1Id(axisP1Id);
    dialogTool->SetAxisP2Id(axisP2Id);
    dialogTool->SetFirstPointId(firstPointId);
    dialogTool->SetSecondPointId(secondPointId);
    dialogTool->SetPointName(p->name());
    dialogTool->SetNotes(m_notes);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool from GUI.
 * @param dialog dialog.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 * @return the created tool
 */
auto VToolTriangle::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                           VContainer *data) -> VToolTriangle *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogTriangle> dialogTool = qobject_cast<DialogTriangle *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolTriangleInitData initData;
    initData.axisP1Id = dialogTool->GetAxisP1Id();
    initData.axisP2Id = dialogTool->GetAxisP2Id();
    initData.firstPointId = dialogTool->GetFirstPointId();
    initData.secondPointId = dialogTool->GetSecondPointId();
    initData.name = dialogTool->GetPointName();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();

    VToolTriangle *point = Create(initData);
    if (point != nullptr)
    {
        point->m_dialog = dialog;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create helps to create the tool.
 * @param initData init data.
 * @return the created tool
 */
auto VToolTriangle::Create(VToolTriangleInitData initData) -> VToolTriangle *
{
    const QSharedPointer<VPointF> axisP1 = initData.data->GeometricObject<VPointF>(initData.axisP1Id);
    const QSharedPointer<VPointF> axisP2 = initData.data->GeometricObject<VPointF>(initData.axisP2Id);
    const QSharedPointer<VPointF> firstPoint = initData.data->GeometricObject<VPointF>(initData.firstPointId);
    const QSharedPointer<VPointF> secondPoint = initData.data->GeometricObject<VPointF>(initData.secondPointId);

    QPointF point;
    const bool success = FindPoint(static_cast<QPointF>(*axisP1), static_cast<QPointF>(*axisP2),
                                   static_cast<QPointF>(*firstPoint), static_cast<QPointF>(*secondPoint), &point);

    if (not success)
    {
        const QString errorMsg =
            tr("Error calculating point '%1'. Point of intersection cannot be found").arg(initData.name);
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VExceptionObjectError(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
    }

    auto *p = new VPointF(point, initData.name, initData.mx, initData.my);
    p->SetShowLabel(initData.showLabel);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->AddGObject(p);
    }
    else
    {
        initData.data->UpdateGObject(initData.id, p);
    }

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->AddVertex(initData.id, VNodeType::TOOL, initData.doc->PatternBlockMapper()->GetActiveId());

    patternGraph->AddEdge(initData.axisP1Id, initData.id);
    patternGraph->AddEdge(initData.axisP2Id, initData.id);
    patternGraph->AddEdge(initData.firstPointId, initData.id);
    patternGraph->AddEdge(initData.secondPointId, initData.id);

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::Triangle, initData.doc);
        auto *point = new VToolTriangle(initData);
        initData.scene->addItem(point);
        InitToolConnections(initData.scene, point);
        VAbstractPattern::AddTool(initData.id, point);
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FindPoint find point intersection two foots right triangle.
 * @param axisP1 first axis point.
 * @param axisP2 second axis point.
 * @param firstPoint first triangle point, what lies on the hypotenuse.
 * @param secondPoint second triangle point, what lies on the hypotenuse.
 * @param intersectionPoint point intersection two foots right triangle
 * @return true if the intersection exists.
 */
auto VToolTriangle::FindPoint(const QPointF &axisP1, const QPointF &axisP2, const QPointF &firstPoint,
                              const QPointF &secondPoint, QPointF *intersectionPoint) -> bool
{
    SCASSERT(intersectionPoint != nullptr)

    QLineF const axis(axisP1, axisP2);
    QLineF const hypotenuse(firstPoint, secondPoint);

    QPointF startPoint;
    QLineF::IntersectType const intersect = axis.intersects(hypotenuse, &startPoint);

    if (intersect != QLineF::UnboundedIntersection && intersect != QLineF::BoundedIntersection)
    {
        return false;
    }
    if (VFuzzyComparePossibleNulls(axis.angle(), hypotenuse.angle()) ||
        VFuzzyComparePossibleNulls(qAbs(axis.angle() - hypotenuse.angle()), 180))
    {
        return false;
    }

    qreal const step = 1;

    QLineF line;
    line.setP1(startPoint);
    line.setAngle(axis.angle());
    line.setLength(step);

    qint64 const c = qFloor(hypotenuse.length());
    while (true)
    {
        line.setLength(line.length() + step);
        qint64 const a = qFloor(QLineF(line.p2(), firstPoint).length());
        qint64 const b = qFloor(QLineF(line.p2(), secondPoint).length());
        if (c * c <= (a * a + b * b))
        {
            *intersectionPoint = line.p2();
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolTriangle::AxisP1Name() const -> QString
{
    return VAbstractTool::data.GetGObject(axisP1Id)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolTriangle::AxisP2Name() const -> QString
{
    return VAbstractTool::data.GetGObject(axisP2Id)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolTriangle::FirstPointName() const -> QString
{
    return VAbstractTool::data.GetGObject(firstPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolTriangle::SecondPointName() const -> QString
{
    return VAbstractTool::data.GetGObject(secondPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolTriangle::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogTriangle> dialogTool = qobject_cast<DialogTriangle *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrName, dialogTool->GetPointName());
    doc->SetAttribute(domElement, AttrAxisP1, QString().setNum(dialogTool->GetAxisP1Id()));
    doc->SetAttribute(domElement, AttrAxisP2, QString().setNum(dialogTool->GetAxisP2Id()));
    doc->SetAttribute(domElement, AttrFirstPoint, QString().setNum(dialogTool->GetFirstPointId()));
    doc->SetAttribute(domElement, AttrSecondPoint, QString().setNum(dialogTool->GetSecondPointId()));

    const QString notes = dialogTool->GetNotes();
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrNotes,
                                         notes,
                                         [](const QString &notes) noexcept -> bool { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolTriangle::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrAxisP1, axisP1Id);
    doc->SetAttribute(tag, AttrAxisP2, axisP2Id);
    doc->SetAttribute(tag, AttrFirstPoint, firstPointId);
    doc->SetAttribute(tag, AttrSecondPoint, secondPointId);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolTriangle::ReadToolAttributes(const QDomElement &domElement)
{
    VToolSinglePoint::ReadToolAttributes(domElement);

    axisP1Id = VAbstractPattern::GetParametrUInt(domElement, AttrAxisP1, NULL_ID_STR);
    axisP2Id = VAbstractPattern::GetParametrUInt(domElement, AttrAxisP2, NULL_ID_STR);
    firstPointId = VAbstractPattern::GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
    secondPointId = VAbstractPattern::GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolTriangle::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolTriangle *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetObject1Id(axisP1Id);
        visual->SetObject2Id(axisP2Id);
        visual->SetHypotenuseP1Id(firstPointId);
        visual->SetHypotenuseP2Id(secondPointId);
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolTriangle::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    ProcessPointToolOptions(oldDomElement, newDomElement, GatherToolChanges());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolTriangle::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogTriangle>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolTriangle::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolTriangle>(show);
}
