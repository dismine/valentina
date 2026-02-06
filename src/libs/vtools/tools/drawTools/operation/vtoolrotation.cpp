/************************************************************************
 **
 **  @file   vtoolrotation.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 4, 2016
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

#include "vtoolrotation.h"

#include <QColor>
#include <QDomNode>
#include <QDomNodeList>
#include <QMapIterator>
#include <QPoint>
#include <QSharedPointer>
#include <QUndoStack>
#include <climits>

#include "../../../dialogs/tools/dialogrotation.h"
#include "../../../dialogs/tools/dialogtool.h"
#include "../../../visualization/line/operation/vistoolrotation.h"
#include "../../../visualization/visualization.h"
#include "../../vabstracttool.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepath.h"
#include "../vmisc/exception/vexception.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vformula.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vmaingraphicsscene.h"

class QDomElement;
class QGraphicsSceneContextMenuEvent;
class QPainter;
class QStyleOptionGraphicsItem;
class QWidget;
template <class T> class QSharedPointer;

const QString VToolRotation::ToolType = QStringLiteral("rotation");

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto CreatePoint(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle, VContainer *data)
    -> DestinationItem
{
    const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(sItem.id);
    VPointF rotated = point->Rotate(origin, angle, sItem.name);
    rotated.setIdObject(idTool);

    return {.id = data->AddGObject(new VPointF(rotated)),
            .mx = rotated.mx(),
            .my = rotated.my(),
            .showLabel = rotated.IsShowLabel()};
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
auto CreateItem(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle, VContainer *data)
    -> DestinationItem
{
    const QSharedPointer<Item> i = data->GeometricObject<Item>(sItem.id);
    Item rotated = i->Rotate(origin, angle, sItem.name);
    rotated.setIdObject(idTool);

    if (sItem.penStyle != TypeLineDefault)
    {
        rotated.SetPenStyle(sItem.penStyle);
    }

    if (sItem.color != ColorDefault)
    {
        rotated.SetColor(sItem.color);
    }

    return {.id = data->AddGObject(new Item(rotated))};
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
auto CreateArc(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle, VContainer *data)
    -> DestinationItem
{
    const DestinationItem item = CreateItem<Item>(idTool, sItem, origin, angle, data);
    data->AddArc(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
auto CreateCurve(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle, VContainer *data)
    -> DestinationItem
{
    const DestinationItem item = CreateItem<Item>(idTool, sItem, origin, angle, data);
    data->AddSpline(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
auto CreateCurveWithSegments(
    quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle, VContainer *data) -> DestinationItem
{
    const DestinationItem item = CreateItem<Item>(idTool, sItem, origin, angle, data);
    data->AddCurveWithSegments(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
auto CreateDestinationObject(
    quint32 id, const SourceItem &object, GOType type, qreal calcAngle, const QPointF &oPoint, VContainer *data)
    -> DestinationItem
{
    // This check helps to find missed objects in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 8, "Not all objects were handled.");

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wswitch-default")
    QT_WARNING_DISABLE_CLANG("-Wswitch-default")

    switch (type)
    {
        case GOType::Point:
            return CreatePoint(id, object, oPoint, calcAngle, data);
        case GOType::Arc:
            return CreateArc<VArc>(id, object, oPoint, calcAngle, data);
        case GOType::EllipticalArc:
            return CreateArc<VEllipticalArc>(id, object, oPoint, calcAngle, data);
        case GOType::Spline:
            return CreateCurve<VSpline>(id, object, oPoint, calcAngle, data);
        case GOType::SplinePath:
            return CreateCurveWithSegments<VSplinePath>(id, object, oPoint, calcAngle, data);
        case GOType::CubicBezier:
            return CreateCurve<VCubicBezier>(id, object, oPoint, calcAngle, data);
        case GOType::CubicBezierPath:
            return CreateCurveWithSegments<VCubicBezierPath>(id, object, oPoint, calcAngle, data);
        case GOType::Unknown:
        case GOType::PlaceLabel:
            Q_UNREACHABLE();
            break;
    }

    QT_WARNING_POP

    Q_UNREACHABLE();
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
void CreateDestinationObjects(VToolRotationInitData &initData, qreal calcAngle, const QPointF &oPoint)
{
    for (const auto &object : std::as_const(initData.source))
    {
        const QSharedPointer<VGObject> obj = initData.data->GetGObject(object.id);

        const DestinationItem item
            = CreateDestinationObject(initData.id, object, obj->getType(), calcAngle, oPoint, initData.data);
        initData.destination.append(item);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void UpdatePoint(quint32 idTool,
                 const SourceItem &sItem,
                 const QPointF &origin,
                 qreal angle,
                 VContainer *data,
                 const DestinationItem &item)
{
    const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(sItem.id);
    VPointF rotated = point->Rotate(origin, angle, sItem.name);
    rotated.setIdObject(idTool);
    rotated.setMx(item.mx);
    rotated.setMy(item.my);
    rotated.SetShowLabel(item.showLabel);

    data->UpdateGObject(item.id, new VPointF(rotated));
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
void UpdateItem(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle, VContainer *data, quint32 id)
{
    const QSharedPointer<Item> i = data->GeometricObject<Item>(sItem.id);
    Item rotated = i->Rotate(origin, angle, sItem.name);
    rotated.setIdObject(idTool);

    if (sItem.penStyle != TypeLineDefault)
    {
        rotated.SetPenStyle(sItem.penStyle);
    }

    if (sItem.color != ColorDefault)
    {
        rotated.SetColor(sItem.color);
    }

    data->UpdateGObject(id, new Item(rotated));
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
void UpdateArc(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle, VContainer *data, quint32 id)
{
    UpdateItem<Item>(idTool, sItem, origin, angle, data, id);
    data->AddArc(data->GeometricObject<Item>(id), id);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
void UpdateCurve(
    quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle, VContainer *data, quint32 id)
{
    UpdateItem<Item>(idTool, sItem, origin, angle, data, id);
    data->AddSpline(data->GeometricObject<Item>(id), id);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
void UpdateCurveWithSegments(
    quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle, VContainer *data, quint32 id)
{
    UpdateItem<Item>(idTool, sItem, origin, angle, data, id);
    data->AddCurveWithSegments(data->GeometricObject<Item>(id), id);
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateDestinationObject(quint32 id,
                             const SourceItem &object,
                             GOType type,
                             qreal calcAngle,
                             const QPointF &oPoint,
                             VContainer *data,
                             const DestinationItem &destination)
{
    // This check helps to find missed objects in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 8, "Not all objects were handled.");

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wswitch-default")
    QT_WARNING_DISABLE_CLANG("-Wswitch-default")

    switch (type)
    {
        case GOType::Point:
            UpdatePoint(id, object, oPoint, calcAngle, data, destination);
            break;
        case GOType::Arc:
            UpdateArc<VArc>(id, object, oPoint, calcAngle, data, destination.id);
            break;
        case GOType::EllipticalArc:
            UpdateArc<VEllipticalArc>(id, object, oPoint, calcAngle, data, destination.id);
            break;
        case GOType::Spline:
            UpdateCurve<VSpline>(id, object, oPoint, calcAngle, data, destination.id);
            break;
        case GOType::SplinePath:
            UpdateCurveWithSegments<VSplinePath>(id, object, oPoint, calcAngle, data, destination.id);
            break;
        case GOType::CubicBezier:
            UpdateCurve<VCubicBezier>(id, object, oPoint, calcAngle, data, destination.id);
            break;
        case GOType::CubicBezierPath:
            UpdateCurveWithSegments<VCubicBezierPath>(id, object, oPoint, calcAngle, data, destination.id);
            break;
        case GOType::Unknown:
        case GOType::PlaceLabel:
            Q_UNREACHABLE();
            break;
    }

    QT_WARNING_POP
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateDestinationObjects(VToolRotationInitData &initData, qreal calcAngle, const QPointF &oPoint)
{
    for (int i = 0; i < initData.source.size(); ++i)
    {
        const SourceItem object = initData.source.at(i);
        const QSharedPointer<VGObject> obj = initData.data->GetGObject(object.id);
        UpdateDestinationObject(initData.id,
                                object,
                                obj->getType(),
                                calcAngle,
                                oPoint,
                                initData.data,
                                initData.destination.at(i));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void CreateDestination(VToolRotationInitData &initData)
{
    qreal const calcAngle = VAbstractTool::CheckFormula(initData.id, initData.angle, initData.data);

    const auto originPoint = *initData.data->GeometricObject<VPointF>(initData.origin);
    const auto oPoint = static_cast<QPointF>(originPoint);

    if (initData.typeCreation == Source::FromGui)
    {
        CreateDestinationObjects(initData, calcAngle, oPoint);
    }
    else
    {
        UpdateDestinationObjects(initData, calcAngle, oPoint);
    }
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VToolRotation::VToolRotation(const VToolRotationInitData &initData, QGraphicsItem *parent)
  : VAbstractOperation(initData, parent),
    origPointId(initData.origin),
    formulaAngle(initData.angle)
{
    InitOperatedObjects();
    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolRotation::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogRotation> dialogTool = qobject_cast<DialogRotation *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetOrigPointId(origPointId);
    dialogTool->SetAngle(formulaAngle);
    dialogTool->SetNotes(m_notes);
    dialogTool->SetSourceObjects(source);

    SetDialogVisibilityGroupData(dialogTool);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolRotation::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                           VContainer *data) -> VToolRotation *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogRotation> dialogTool = qobject_cast<DialogRotation *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolRotationInitData initData;
    initData.origin = dialogTool->GetOrigPointId();
    initData.angle = dialogTool->GetAngle();
    initData.source = dialogTool->GetSourceObjects();
    initData.hasLinkedVisibilityGroup = dialogTool->HasLinkedVisibilityGroup();
    initData.visibilityGroupName = dialogTool->GetVisibilityGroupName();
    initData.visibilityGroupTags = dialogTool->GetVisibilityGroupTags();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();

    VToolRotation *operation = Create(initData);
    if (operation != nullptr)
    {
        operation->m_dialog = dialog;
    }
    return operation;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolRotation::Create(VToolRotationInitData &initData) -> VToolRotation *
{
    if (initData.typeCreation == Source::FromGui)
    {
        initData.destination.clear(); // Try to avoid mistake, value must be empty

        initData.id = initData.data->getNextId(); // Just reserve id for tool
    }
    else
    {
        PrepareNames(initData);
    }

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->AddVertex(initData.id, VNodeType::TOOL, initData.doc->PatternBlockMapper()->GetActiveId());

    const auto varData = initData.data->DataDependencyVariables();
    initData.doc->FindFormulaDependencies(initData.angle, initData.id, varData);

    CreateDestination(initData);

    patternGraph->AddEdge(initData.origin, initData.id);

    for (const auto &object : std::as_const(initData.source))
    {
        patternGraph->AddEdge(object.id, initData.id);
    }

    for (const auto &object : std::as_const(initData.destination))
    {
        patternGraph->AddVertex(object.id, VNodeType::OBJECT, initData.doc->PatternBlockMapper()->GetActiveId());
        patternGraph->AddEdge(initData.id, object.id);
    }

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        if (initData.typeCreation == Source::FromGui && initData.hasLinkedVisibilityGroup)
        {
            VAbstractApplication::VApp()->getUndoStack()->beginMacro(tr("rotate"));
        }

        VAbstractTool::AddRecord(initData.id, Tool::Rotation, initData.doc);
        auto *tool = new VToolRotation(initData);
        initData.scene->addItem(tool);
        InitOperationToolConnections(initData.scene, tool);
        VAbstractPattern::AddTool(initData.id, tool);

        if (initData.typeCreation == Source::FromGui && initData.hasLinkedVisibilityGroup)
        {
            VAbstractOperation::CreateVisibilityGroup(initData);
            VAbstractApplication::VApp()->getUndoStack()->endMacro();
        }

        return tool;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolRotation::OriginPointName() const -> QString
{
    return VAbstractTool::data.GetGObject(origPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolRotation::GetFormulaAngle() const -> VFormula
{
    VFormula fAngle(formulaAngle, getData());
    fAngle.setToolId(m_id);
    fAngle.setPostfix(degreeSymbol);
    fAngle.Eval();
    return fAngle;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolRotation::SetFormulaAngle(const VFormula &value)
{
    if (!value.error())
    {
        formulaAngle = value.GetFormula(FormulaType::FromUser);

        QSharedPointer<VGObject> obj = VContainer::GetFakeGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolRotation::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolRotation>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolRotation::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogRotation>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolRotation::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolRotation *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetObjects(SourceToObjects(source));
        visual->SetOriginPointId(origPointId);
        visual->SetAngle(VAbstractApplication::VApp()->TrVars()->FormulaToUser(
            formulaAngle, VAbstractApplication::VApp()->Settings()->GetOsSeparator()));
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolRotation::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogRotation> dialogTool = qobject_cast<DialogRotation *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrCenter, QString().setNum(dialogTool->GetOrigPointId()));
    doc->SetAttribute(domElement, AttrAngle, dialogTool->GetAngle());
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrNotes,
                                         dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept -> bool { return notes.isEmpty(); });

    // We no longer need to handle suffix attribute here. The code can be removed.
    Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
    if (domElement.hasAttribute(AttrSuffix))
    {
        domElement.removeAttribute(AttrSuffix);
    }

    source = dialogTool->GetSourceObjects();
    SaveSourceDestination(domElement);

    // Save visibility data for later use
    SaveVisibilityGroupData(dialogTool);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolRotation::ReadToolAttributes(const QDomElement &domElement)
{
    VAbstractOperation::ReadToolAttributes(domElement);

    origPointId = VAbstractPattern::GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);
    formulaAngle = VDomDocument::GetParametrString(domElement, AttrAngle, QChar('0'));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolRotation::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VAbstractOperation::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrCenter, QString().setNum(origPointId));
    doc->SetAttribute(tag, AttrAngle, formulaAngle);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolRotation::MakeToolTip() const -> QString
{
    return QStringLiteral("<tr> <td><b>%1:</b> %2</td> </tr>"
                          "<tr> <td><b>%3:</b> %4°</td> </tr>"
                          "%5")
        .arg(tr("Origin point"), OriginPointName(), tr("Rotation angle")) // 1, 2, 3
        .arg(GetFormulaAngle().getDoubleValue())                          // 4
        .arg(VisibilityGroupToolTip());                                   // 5
}

//---------------------------------------------------------------------------------------------------------------------
void VToolRotation::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogRotation> dialogTool = qobject_cast<DialogRotation *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    ProcessOperationToolOptions(oldDomElement, newDomElement, dialogTool->GetSourceObjects());
}
