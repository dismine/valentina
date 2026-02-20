/************************************************************************
 **
 **  @file   vtoolpointfromcircleandtangent.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 6, 2015
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

#include "vtoolpointfromcircleandtangent.h"

#include <QSharedPointer>

#include "../../../../dialogs/tools/dialogpointfromcircleandtangent.h"
#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../visualization/line/vistoolpointfromcircleandtangent.h"
#include "../../../../visualization/visualization.h"
#include "../../../vabstracttool.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vdomdocument.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/exception/vexception.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vformula.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vtoolsinglepoint.h"

template <class T> class QSharedPointer;

const QString VToolPointFromCircleAndTangent::ToolType = QStringLiteral("pointFromCircleAndTangent");

//---------------------------------------------------------------------------------------------------------------------
VToolPointFromCircleAndTangent::VToolPointFromCircleAndTangent(const VToolPointFromCircleAndTangentInitData &initData,
                                                               QGraphicsItem *parent)
  : VToolSinglePoint(initData.doc, initData.data, initData.id, initData.notes, parent),
    circleCenterId(initData.circleCenterId),
    tangentPointId(initData.tangentPointId),
    circleRadius(initData.circleRadius),
    crossPoint(initData.crossPoint)
{
    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointFromCircleAndTangent::GatherToolChanges() const -> VAbstractPoint::ToolChanges
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogPointFromCircleAndTangent> dialogTool = qobject_cast<DialogPointFromCircleAndTangent *>(
        m_dialog);
    SCASSERT(not dialogTool.isNull())

    return {.pointId = m_id, .oldLabel = name(), .newLabel = dialogTool->GetPointName()};
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromCircleAndTangent::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogPointFromCircleAndTangent> dialogTool =
        qobject_cast<DialogPointFromCircleAndTangent *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetCircleCenterId(circleCenterId);
    dialogTool->SetCircleRadius(circleRadius);
    dialogTool->SetCrossCirclesPoint(crossPoint);
    dialogTool->SetTangentPointId(tangentPointId);
    dialogTool->SetPointName(p->name());
    dialogTool->SetNotes(m_notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointFromCircleAndTangent::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene,
                                            VAbstractPattern *doc, VContainer *data) -> VToolPointFromCircleAndTangent *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogPointFromCircleAndTangent> dialogTool =
        qobject_cast<DialogPointFromCircleAndTangent *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolPointFromCircleAndTangentInitData initData;
    initData.circleCenterId = dialogTool->GetCircleCenterId();
    initData.circleRadius = dialogTool->GetCircleRadius();
    initData.tangentPointId = dialogTool->GetTangentPointId();
    initData.crossPoint = dialogTool->GetCrossCirclesPoint();
    initData.name = dialogTool->GetPointName();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();

    VToolPointFromCircleAndTangent *point = Create(initData);
    if (point != nullptr)
    {
        point->m_dialog = dialog;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointFromCircleAndTangent::Create(VToolPointFromCircleAndTangentInitData &initData)
    -> VToolPointFromCircleAndTangent *
{
    const qreal radius =
        VAbstractValApplication::VApp()->toPixel(CheckFormula(initData.id, initData.circleRadius, initData.data));
    const VPointF cPoint = *initData.data->GeometricObject<VPointF>(initData.circleCenterId);
    const VPointF tPoint = *initData.data->GeometricObject<VPointF>(initData.tangentPointId);

    QPointF point;
    const bool success = VToolPointFromCircleAndTangent::FindPoint(
        static_cast<QPointF>(tPoint), static_cast<QPointF>(cPoint), radius, initData.crossPoint, &point);
    if (not success)
    {
        const QString errorMsg = tr("Error calculating point '%1'. Tangent to circle with center '%2' and radius '%3' "
                                    "from point '%4' cannot be found")
                                     .arg(initData.name, cPoint.name())
                                     .arg(radius)
                                     .arg(tPoint.name());

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

    const auto varData = initData.data->DataDependencyVariables();
    initData.doc->FindFormulaDependencies(initData.circleRadius, initData.id, varData);

    patternGraph->AddEdge(initData.circleCenterId, initData.id);
    patternGraph->AddEdge(initData.tangentPointId, initData.id);

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::PointFromCircleAndTangent, initData.doc);
        auto *point = new VToolPointFromCircleAndTangent(initData);
        initData.scene->addItem(point);
        InitToolConnections(initData.scene, point);
        VAbstractPattern::AddTool(initData.id, point);
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointFromCircleAndTangent::FindPoint(const QPointF &p, const QPointF &center, qreal radius,
                                               const CrossCirclesPoint crossPoint, QPointF *intersectionPoint) -> bool
{
    SCASSERT(intersectionPoint != nullptr)

    QPointF p1, p2;
    const int res = VGObject::ContactPoints(p, center, qAbs(radius), p1, p2);

    switch (res)
    {
        case 2:
            *intersectionPoint = (crossPoint == CrossCirclesPoint::FirstPoint ? p1 : p2);
            return true;
        case 1:
            *intersectionPoint = p1;
            return true;
        case 3:
        case 0:
        default:
            return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointFromCircleAndTangent::TangentPointName() const -> QString
{
    return VAbstractTool::data.GetGObject(tangentPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointFromCircleAndTangent::CircleCenterPointName() const -> QString
{
    return VAbstractTool::data.GetGObject(circleCenterId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointFromCircleAndTangent::GetCircleRadius() const -> VFormula
{
    VFormula radius(circleRadius, getData());
    radius.setCheckZero(true);
    radius.setToolId(m_id);
    radius.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    radius.Eval();
    return radius;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromCircleAndTangent::SetCircleRadius(const VFormula &value)
{
    if (value.error() == false)
    {
        circleRadius = value.GetFormula(FormulaType::FromUser);
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointFromCircleAndTangent::GetCrossCirclesPoint() const -> CrossCirclesPoint
{
    return crossPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromCircleAndTangent::SetCrossCirclesPoint(const CrossCirclesPoint &value)
{
    crossPoint = value;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromCircleAndTangent::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolPointFromCircleAndTangent>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromCircleAndTangent::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogPointFromCircleAndTangent>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromCircleAndTangent::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogPointFromCircleAndTangent> dialogTool =
        qobject_cast<DialogPointFromCircleAndTangent *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrName, dialogTool->GetPointName());
    doc->SetAttribute(domElement, AttrCCenter, QString().setNum(dialogTool->GetCircleCenterId()));
    doc->SetAttribute(domElement, AttrTangent, QString().setNum(dialogTool->GetTangentPointId()));
    doc->SetAttribute(domElement, AttrCRadius, dialogTool->GetCircleRadius());
    doc->SetAttribute(domElement, AttrCrossPoint,
                      QString().setNum(static_cast<int>(dialogTool->GetCrossCirclesPoint())));
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrNotes,
                                         dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept -> bool { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromCircleAndTangent::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrCCenter, circleCenterId);
    doc->SetAttribute(tag, AttrTangent, tangentPointId);
    doc->SetAttribute(tag, AttrCRadius, circleRadius);
    doc->SetAttribute(tag, AttrCrossPoint, static_cast<int>(crossPoint));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromCircleAndTangent::ReadToolAttributes(const QDomElement &domElement)
{
    VToolSinglePoint::ReadToolAttributes(domElement);

    circleCenterId = VAbstractPattern::GetParametrUInt(domElement, AttrCCenter, NULL_ID_STR);
    tangentPointId = VAbstractPattern::GetParametrUInt(domElement, AttrTangent, NULL_ID_STR);
    circleRadius = VAbstractPattern::GetParametrString(domElement, AttrCRadius);
    crossPoint = static_cast<CrossCirclesPoint>(
        VAbstractPattern::GetParametrUInt(domElement, AttrCrossPoint, QChar('1')));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromCircleAndTangent::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolPointFromCircleAndTangent *>(vis);
        SCASSERT(visual != nullptr)

        const bool osSeparator = VAbstractApplication::VApp()->Settings()->GetOsSeparator();
        const VTranslateVars *trVars = VAbstractApplication::VApp()->TrVars();

        visual->SetPointId(tangentPointId);
        visual->SetCenterId(circleCenterId);
        visual->SetCRadius(trVars->FormulaToUser(circleRadius, osSeparator));
        visual->SetCrossPoint(crossPoint);
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointFromCircleAndTangent::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    ProcessPointToolOptions(oldDomElement, newDomElement, GatherToolChanges());
}
