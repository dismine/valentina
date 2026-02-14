/************************************************************************
 **
 **  @file   vtoolpointofintersectioncircles.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 5, 2015
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

#include "vtoolpointofintersectioncircles.h"

#include <QSharedPointer>

#include "../../../../dialogs/tools/dialogpointofintersectioncircles.h"
#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../visualization/line/vistoolpointofintersectioncircles.h"
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

const QString VToolPointOfIntersectionCircles::ToolType = QStringLiteral("pointOfIntersectionCircles");

//---------------------------------------------------------------------------------------------------------------------
VToolPointOfIntersectionCircles::VToolPointOfIntersectionCircles(
    const VToolPointOfIntersectionCirclesInitData &initData, QGraphicsItem *parent)
  : VToolSinglePoint(initData.doc, initData.data, initData.id, initData.notes, parent),
    firstCircleCenterId(initData.firstCircleCenterId),
    secondCircleCenterId(initData.secondCircleCenterId),
    firstCircleRadius(initData.firstCircleRadius),
    secondCircleRadius(initData.secondCircleRadius),
    crossPoint(initData.crossPoint)
{
    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCircles::GatherToolChanges() const -> VAbstractPoint::ToolChanges
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogPointOfIntersectionCircles> dialogTool = qobject_cast<DialogPointOfIntersectionCircles *>(
        m_dialog);
    SCASSERT(not dialogTool.isNull())

    return {.pointId = m_id, .oldLabel = name(), .newLabel = dialogTool->GetPointName()};
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCircles::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogPointOfIntersectionCircles> dialogTool =
        qobject_cast<DialogPointOfIntersectionCircles *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetFirstCircleCenterId(firstCircleCenterId);
    dialogTool->SetSecondCircleCenterId(secondCircleCenterId);
    dialogTool->SetFirstCircleRadius(firstCircleRadius);
    dialogTool->SetSecondCircleRadius(secondCircleRadius);
    dialogTool->SetCrossCirclesPoint(crossPoint);
    dialogTool->SetPointName(p->name());
    dialogTool->SetNotes(m_notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCircles::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene,
                                             VAbstractPattern *doc, VContainer *data)
    -> VToolPointOfIntersectionCircles *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogPointOfIntersectionCircles> dialogTool =
        qobject_cast<DialogPointOfIntersectionCircles *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolPointOfIntersectionCirclesInitData initData;
    initData.firstCircleCenterId = dialogTool->GetFirstCircleCenterId();
    initData.secondCircleCenterId = dialogTool->GetSecondCircleCenterId();
    initData.firstCircleRadius = dialogTool->GetFirstCircleRadius();
    initData.secondCircleRadius = dialogTool->GetSecondCircleRadius();
    initData.crossPoint = dialogTool->GetCrossCirclesPoint();
    initData.name = dialogTool->GetPointName();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();

    VToolPointOfIntersectionCircles *point = Create(initData);
    if (point != nullptr)
    {
        point->m_dialog = dialog;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCircles::Create(VToolPointOfIntersectionCirclesInitData &initData)
    -> VToolPointOfIntersectionCircles *
{
    const qreal calcC1Radius =
        VAbstractValApplication::VApp()->toPixel(CheckFormula(initData.id, initData.firstCircleRadius, initData.data));
    const qreal calcC2Radius =
        VAbstractValApplication::VApp()->toPixel(CheckFormula(initData.id, initData.secondCircleRadius, initData.data));

    const VPointF c1Point = *initData.data->GeometricObject<VPointF>(initData.firstCircleCenterId);
    const VPointF c2Point = *initData.data->GeometricObject<VPointF>(initData.secondCircleCenterId);

    QPointF point;
    const bool success = FindPoint(static_cast<QPointF>(c1Point), static_cast<QPointF>(c2Point), calcC1Radius,
                                   calcC2Radius, initData.crossPoint, &point);

    if (not success)
    {
        const QString errorMsg = tr("Error calculating point '%1'. Circles with centers in points '%2' and '%3' have "
                                    "no point of intersection")
                                     .arg(initData.name, c1Point.name(), c2Point.name());
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
    initData.doc->FindFormulaDependencies(initData.firstCircleRadius, initData.id, varData);
    initData.doc->FindFormulaDependencies(initData.secondCircleRadius, initData.id, varData);

    patternGraph->AddEdge(initData.firstCircleCenterId, initData.id);
    patternGraph->AddEdge(initData.secondCircleCenterId, initData.id);

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::PointOfIntersectionCircles, initData.doc);
        auto *point = new VToolPointOfIntersectionCircles(initData);
        initData.scene->addItem(point);
        InitToolConnections(initData.scene, point);
        VAbstractPattern::AddTool(initData.id, point);
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCircles::FindPoint(const QPointF &c1Point, const QPointF &c2Point, qreal c1Radius,
                                                qreal c2Radius, const CrossCirclesPoint crossPoint,
                                                QPointF *intersectionPoint) -> bool
{
    SCASSERT(intersectionPoint != nullptr)

    QPointF p1, p2;
    const int res = VGObject::IntersectionCircles(c1Point, qAbs(c1Radius), c2Point, qAbs(c2Radius), p1, p2);

    switch (res)
    {
        case 2:
            if (crossPoint == CrossCirclesPoint::FirstPoint)
            {
                *intersectionPoint = p1;
                return true;
            }
            else
            {
                *intersectionPoint = p2;
                return true;
            }
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
auto VToolPointOfIntersectionCircles::FirstCircleCenterPointName() const -> QString
{
    return VAbstractTool::data.GetGObject(firstCircleCenterId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCircles::SecondCircleCenterPointName() const -> QString
{
    return VAbstractTool::data.GetGObject(secondCircleCenterId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCircles::GetFirstCircleRadius() const -> VFormula
{
    VFormula radius(firstCircleRadius, getData());
    radius.setToolId(m_id);
    radius.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    radius.Eval();
    return radius;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCircles::SetFirstCircleRadius(const VFormula &value)
{
    if (!value.error())
    {
        firstCircleRadius = value.GetFormula(FormulaType::FromUser);
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCircles::GetSecondCircleRadius() const -> VFormula
{
    VFormula radius(secondCircleRadius, getData());
    radius.setToolId(m_id);
    radius.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    radius.Eval();
    return radius;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCircles::SetSecondCircleRadius(const VFormula &value)
{
    if (!value.error())
    {
        secondCircleRadius = value.GetFormula(FormulaType::FromUser);
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCircles::GetCrossCirclesPoint() const -> CrossCirclesPoint
{
    return crossPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCircles::SetCrossCirclesPoint(const CrossCirclesPoint &value)
{
    crossPoint = value;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCircles::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolPointOfIntersectionCircles>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCircles::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogPointOfIntersectionCircles>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCircles::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogPointOfIntersectionCircles> dialogTool =
        qobject_cast<DialogPointOfIntersectionCircles *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrName, dialogTool->GetPointName());
    doc->SetAttribute(domElement, AttrC1Center, QString().setNum(dialogTool->GetFirstCircleCenterId()));
    doc->SetAttribute(domElement, AttrC2Center, QString().setNum(dialogTool->GetSecondCircleCenterId()));
    doc->SetAttribute(domElement, AttrC1Radius, dialogTool->GetFirstCircleRadius());
    doc->SetAttribute(domElement, AttrC2Radius, dialogTool->GetSecondCircleRadius());
    doc->SetAttribute(domElement, AttrCrossPoint,
                      QString().setNum(static_cast<int>(dialogTool->GetCrossCirclesPoint())));
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrNotes,
                                         dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept -> bool { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCircles::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrC1Center, firstCircleCenterId);
    doc->SetAttribute(tag, AttrC2Center, secondCircleCenterId);
    doc->SetAttribute(tag, AttrC1Radius, firstCircleRadius);
    doc->SetAttribute(tag, AttrC2Radius, secondCircleRadius);
    doc->SetAttribute(tag, AttrCrossPoint, static_cast<int>(crossPoint));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCircles::ReadToolAttributes(const QDomElement &domElement)
{
    VToolSinglePoint::ReadToolAttributes(domElement);

    firstCircleCenterId = VAbstractPattern::GetParametrUInt(domElement, AttrC1Center, NULL_ID_STR);
    secondCircleCenterId = VAbstractPattern::GetParametrUInt(domElement, AttrC2Center, NULL_ID_STR);
    firstCircleRadius = VDomDocument::GetParametrString(domElement, AttrC1Radius);
    secondCircleRadius = VDomDocument::GetParametrString(domElement, AttrC2Radius);
    crossPoint = static_cast<CrossCirclesPoint>(
        VAbstractPattern::GetParametrUInt(domElement, AttrCrossPoint, QChar('1')));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCircles::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolPointOfIntersectionCircles *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetCircle1Id(firstCircleCenterId);
        visual->SetCircle2Id(secondCircleCenterId);

        const bool osSeparator = VAbstractApplication::VApp()->Settings()->GetOsSeparator();
        const VTranslateVars *trVars = VAbstractApplication::VApp()->TrVars();

        visual->SetC1Radius(trVars->FormulaToUser(firstCircleRadius, osSeparator));
        visual->SetC2Radius(trVars->FormulaToUser(secondCircleRadius, osSeparator));
        visual->SetCrossPoint(crossPoint);
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCircles::ApplyToolOptions(const QDomElement &oldDomElement,
                                                       const QDomElement &newDomElement)
{
    ProcessPointToolOptions(oldDomElement, newDomElement, GatherToolChanges());
}
