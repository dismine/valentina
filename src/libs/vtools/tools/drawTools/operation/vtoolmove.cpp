/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   1 10, 2016
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

#include "vtoolmove.h"

#include <QColor>
#include <QDomNode>
#include <QDomNodeList>
#include <QMapIterator>
#include <QPoint>
#include <QSharedPointer>
#include <QUndoStack>
#include <climits>
#include <qiterator.h>

#include "../../../dialogs/tools/dialogmove.h"
#include "../../../dialogs/tools/dialogtool.h"
#include "../../../visualization/line/operation/vistoolmove.h"
#include "../../../visualization/visualization.h"
#include "../../vabstracttool.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vgeometry/vabstractcurve.h"
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

template <class T> class QSharedPointer;

const QString VToolMove::ToolType = QStringLiteral("moving");

namespace
{
auto GetOriginPoint(const QVector<SourceItem> &objects, const VContainer *data, qreal calcLength, qreal calcAngle)
    -> QPointF
{
    QPolygonF originObjects;

    for (const auto &object : objects)
    {
        const QSharedPointer<VGObject> obj = data->GetGObject(object.id);

        // This check helps to find missed objects in the switch
        Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 8, "Not all objects were handled.");

        switch (obj->getType())
        {
            case GOType::Point:
                originObjects.append(data->GeometricObject<VPointF>(object.id)->toQPointF());
                break;
            case GOType::Arc:
            case GOType::EllipticalArc:
            case GOType::Spline:
            case GOType::SplinePath:
            case GOType::CubicBezier:
            case GOType::CubicBezierPath:
                originObjects.append(data->GeometricObject<VAbstractCurve>(object.id)->GetPoints());
                break;
            case GOType::Unknown:
            case GOType::PlaceLabel:
                Q_UNREACHABLE();
                break;
            default:
                break;
        }
    }

    QPointF const rotationOrigin = originObjects.boundingRect().center();
    QLineF move(rotationOrigin, QPointF(rotationOrigin.x() + calcLength, rotationOrigin.y()));
    move.setAngle(calcAngle);
    return move.p2();
}

//---------------------------------------------------------------------------------------------------------------------
auto CreatePoint(quint32 idTool,
                 const SourceItem &sItem,
                 qreal angle,
                 qreal length,
                 qreal rotationAngle,
                 const QPointF &rotationOrigin,
                 VContainer *data) -> DestinationItem
{
    const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(sItem.id);
    VPointF moved = point->Move(length, angle, sItem.name).Rotate(rotationOrigin, rotationAngle, sItem.name);
    moved.setIdObject(idTool);

    return {.id = data->AddGObject(new VPointF(moved)),
            .mx = moved.mx(),
            .my = moved.my(),
            .showLabel = moved.IsShowLabel()};
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
auto CreateItem(quint32 idTool,
                const SourceItem &sItem,
                qreal angle,
                qreal length,
                qreal rotationAngle,
                const QPointF &rotationOrigin,
                VContainer *data) -> DestinationItem
{
    const QSharedPointer<Item> i = data->GeometricObject<Item>(sItem.id);
    Item moved = i->Move(length, angle, sItem.name).Rotate(rotationOrigin, rotationAngle, sItem.name);
    moved.setIdObject(idTool);

    if (sItem.penStyle != TypeLineDefault)
    {
        moved.SetPenStyle(sItem.penStyle);
    }

    if (sItem.color != ColorDefault)
    {
        moved.SetColor(sItem.color);
    }

    return {.id = data->AddGObject(new Item(moved))};
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
auto CreateArc(quint32 idTool,
               const SourceItem &sItem,
               qreal angle,
               qreal length,
               qreal rotationAngle,
               const QPointF &rotationOrigin,
               VContainer *data) -> DestinationItem
{
    const DestinationItem item = CreateItem<Item>(idTool, sItem, angle, length, rotationAngle, rotationOrigin, data);
    data->AddArc(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
auto CreateCurve(quint32 idTool,
                 const SourceItem &sItem,
                 qreal angle,
                 qreal length,
                 qreal rotationAngle,
                 const QPointF &rotationOrigin,
                 VContainer *data) -> DestinationItem
{
    const DestinationItem item = CreateItem<Item>(idTool, sItem, angle, length, rotationAngle, rotationOrigin, data);
    data->AddSpline(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
auto CreateCurveWithSegments(quint32 idTool,
                             const SourceItem &sItem,
                             qreal angle,
                             qreal length,
                             qreal rotationAngle,
                             const QPointF &rotationOrigin,
                             VContainer *data) -> DestinationItem
{
    const DestinationItem item = CreateItem<Item>(idTool, sItem, angle, length, rotationAngle, rotationOrigin, data);
    data->AddCurveWithSegments(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
auto CreateDestinationObject(quint32 id,
                             const SourceItem &object,
                             GOType type,
                             qreal calcAngle,
                             qreal calcLength,
                             qreal calcRotationAngle,
                             QPointF rotationOrigin,
                             VContainer *data) -> DestinationItem
{
    Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 8, "Not all objects were handled.");

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wswitch-default")
    QT_WARNING_DISABLE_CLANG("-Wswitch-default")

    switch (type)
    {
        case GOType::Point:
            return CreatePoint(id, object, calcAngle, calcLength, calcRotationAngle, rotationOrigin, data);
        case GOType::Arc:
            return CreateArc<VArc>(id, object, calcAngle, calcLength, calcRotationAngle, rotationOrigin, data);
        case GOType::EllipticalArc:
            return CreateArc<VEllipticalArc>(id, object, calcAngle, calcLength, calcRotationAngle, rotationOrigin, data);
        case GOType::Spline:
            return CreateCurve<VSpline>(id, object, calcAngle, calcLength, calcRotationAngle, rotationOrigin, data);
        case GOType::SplinePath:
            return CreateCurveWithSegments<VSplinePath>(id,
                                                        object,
                                                        calcAngle,
                                                        calcLength,
                                                        calcRotationAngle,
                                                        rotationOrigin,
                                                        data);
        case GOType::CubicBezier:
            return CreateCurve<VCubicBezier>(id, object, calcAngle, calcLength, calcRotationAngle, rotationOrigin, data);
        case GOType::CubicBezierPath:
            return CreateCurveWithSegments<VCubicBezierPath>(id,
                                                             object,
                                                             calcAngle,
                                                             calcLength,
                                                             calcRotationAngle,
                                                             rotationOrigin,
                                                             data);
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
void CreateDestinationObjects(
    VToolMoveInitData &initData, QPointF rotationOrigin, qreal calcAngle, qreal calcLength, qreal calcRotationAngle)
{
    for (const auto &object : std::as_const(initData.source))
    {
        const QSharedPointer<VGObject> obj = initData.data->GetGObject(object.id);
        const DestinationItem item = CreateDestinationObject(initData.id,
                                                             object,
                                                             obj->getType(),
                                                             calcAngle,
                                                             calcLength,
                                                             calcRotationAngle,
                                                             rotationOrigin,
                                                             initData.data);
        initData.destination.append(item);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void UpdatePoint(quint32 idTool,
                 const SourceItem &sItem,
                 qreal angle,
                 qreal length,
                 qreal rotationAngle,
                 const QPointF &rotationOrigin,
                 VContainer *data,
                 const DestinationItem &item)
{
    const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(sItem.id);
    VPointF moved = point->Move(length, angle, sItem.name).Rotate(rotationOrigin, rotationAngle, sItem.name);
    moved.setIdObject(idTool);
    moved.setMx(item.mx);
    moved.setMy(item.my);
    moved.SetShowLabel(item.showLabel);
    data->UpdateGObject(item.id, new VPointF(moved));
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
void UpdateItem(quint32 idTool,
                const SourceItem &sItem,
                qreal angle,
                qreal length,
                qreal rotationAngle,
                const QPointF &rotationOrigin,
                VContainer *data,
                quint32 id)
{
    const QSharedPointer<Item> i = data->GeometricObject<Item>(sItem.id);
    Item moved = i->Move(length, angle, sItem.name).Rotate(rotationOrigin, rotationAngle, sItem.name);
    moved.setIdObject(idTool);

    if (sItem.penStyle != TypeLineDefault)
    {
        moved.SetPenStyle(sItem.penStyle);
    }

    if (sItem.color != ColorDefault)
    {
        moved.SetColor(sItem.color);
    }

    data->UpdateGObject(id, new Item(moved));
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
void UpdateArc(quint32 idTool,
               const SourceItem &sItem,
               qreal angle,
               qreal length,
               qreal rotationAngle,
               const QPointF &rotationOrigin,
               VContainer *data,
               quint32 id)
{
    UpdateItem<Item>(idTool, sItem, angle, length, rotationAngle, rotationOrigin, data, id);
    data->AddArc(data->GeometricObject<Item>(id), id);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
void UpdateCurve(quint32 idTool,
                 const SourceItem &sItem,
                 qreal angle,
                 qreal length,
                 qreal rotationAngle,
                 const QPointF &rotationOrigin,
                 VContainer *data,
                 quint32 id)
{
    UpdateItem<Item>(idTool, sItem, angle, length, rotationAngle, rotationOrigin, data, id);
    data->AddSpline(data->GeometricObject<Item>(id), id);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
void UpdateCurveWithSegments(quint32 idTool,
                             const SourceItem &sItem,
                             qreal angle,
                             qreal length,
                             qreal rotationAngle,
                             const QPointF &rotationOrigin,
                             VContainer *data,
                             quint32 id)
{
    UpdateItem<Item>(idTool, sItem, angle, length, rotationAngle, rotationOrigin, data, id);
    data->AddCurveWithSegments(data->GeometricObject<Item>(id), id);
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateDestinationObject(quint32 id,
                             const SourceItem &object,
                             GOType type,
                             qreal calcAngle,
                             qreal calcLength,
                             qreal calcRotationAngle,
                             QPointF rotationOrigin,
                             VContainer *data,
                             const DestinationItem &destination)
{
    Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 8, "Not all objects were handled.");

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wswitch-default")
    QT_WARNING_DISABLE_CLANG("-Wswitch-default")

    switch (type)
    {
        case GOType::Point:
            UpdatePoint(id, object, calcAngle, calcLength, calcRotationAngle, rotationOrigin, data, destination);
            break;
        case GOType::Arc:
            UpdateArc<VArc>(id, object, calcAngle, calcLength, calcRotationAngle, rotationOrigin, data, destination.id);
            break;
        case GOType::EllipticalArc:
            UpdateArc<VEllipticalArc>(id,
                                      object,
                                      calcAngle,
                                      calcLength,
                                      calcRotationAngle,
                                      rotationOrigin,
                                      data,
                                      destination.id);
            break;
        case GOType::Spline:
            UpdateCurve<VSpline>(id,
                                 object,
                                 calcAngle,
                                 calcLength,
                                 calcRotationAngle,
                                 rotationOrigin,
                                 data,
                                 destination.id);
            break;
        case GOType::SplinePath:
            UpdateCurveWithSegments<VSplinePath>(id,
                                                 object,
                                                 calcAngle,
                                                 calcLength,
                                                 calcRotationAngle,
                                                 rotationOrigin,
                                                 data,
                                                 destination.id);
            break;
        case GOType::CubicBezier:
            UpdateCurve<VCubicBezier>(id,
                                      object,
                                      calcAngle,
                                      calcLength,
                                      calcRotationAngle,
                                      rotationOrigin,
                                      data,
                                      destination.id);
            break;
        case GOType::CubicBezierPath:
            UpdateCurveWithSegments<VCubicBezierPath>(id,
                                                      object,
                                                      calcAngle,
                                                      calcLength,
                                                      calcRotationAngle,
                                                      rotationOrigin,
                                                      data,
                                                      destination.id);
            break;
        case GOType::Unknown:
        case GOType::PlaceLabel:
            Q_UNREACHABLE();
            break;
    }

    QT_WARNING_POP
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateDestinationObjects(
    VToolMoveInitData &initData, QPointF rotationOrigin, qreal calcAngle, qreal calcLength, qreal calcRotationAngle)
{
    for (int i = 0; i < initData.source.size(); ++i)
    {
        const SourceItem &object = initData.source.at(i);
        const QSharedPointer<VGObject> obj = initData.data->GetGObject(object.id);
        UpdateDestinationObject(initData.id,
                                object,
                                obj->getType(),
                                calcAngle,
                                calcLength,
                                calcRotationAngle,
                                rotationOrigin,
                                initData.data,
                                initData.destination.at(i));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void CreateDestination(VToolMoveInitData &initData, QPointF rotationOrigin, qreal calcAngle, qreal calcLength)
{
    qreal const calcRotationAngle = VAbstractTool::CheckFormula(initData.id,
                                                                initData.formulaRotationAngle,
                                                                initData.data);

    if (initData.typeCreation == Source::FromGui)
    {
        CreateDestinationObjects(initData, rotationOrigin, calcAngle, calcLength, calcRotationAngle);
    }
    else
    {
        UpdateDestinationObjects(initData, rotationOrigin, calcAngle, calcLength, calcRotationAngle);
    }
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogMove> dialogTool = qobject_cast<DialogMove *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetAngle(formulaAngle);
    dialogTool->SetRotationAngle(formulaRotationAngle);
    dialogTool->SetLength(formulaLength);
    dialogTool->SetRotationOrigPointId(origPointId);
    dialogTool->SetNotes(m_notes);
    dialogTool->SetSourceObjects(source);

    SetDialogVisibilityGroupData(dialogTool);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolMove::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolMove *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogMove> dialogTool = qobject_cast<DialogMove *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolMoveInitData initData;
    initData.formulaAngle = dialogTool->GetAngle();
    initData.formulaRotationAngle = dialogTool->GetRotationAngle();
    initData.formulaLength = dialogTool->GetLength();
    initData.rotationOrigin = dialogTool->GetRotationOrigPointId();
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

    VToolMove *operation = Create(initData);
    if (operation != nullptr)
    {
        operation->m_dialog = dialog;
    }
    return operation;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolMove::Create(VToolMoveInitData &initData) -> VToolMove *
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
    initData.doc->FindFormulaDependencies(initData.formulaAngle, initData.id, varData);
    initData.doc->FindFormulaDependencies(initData.formulaLength, initData.id, varData);
    initData.doc->FindFormulaDependencies(initData.formulaRotationAngle, initData.id, varData);

    qreal const calcAngle = CheckFormula(initData.id, initData.formulaAngle, initData.data);

    qreal const calcLength = VAbstractValApplication::VApp()->toPixel(
        CheckFormula(initData.id, initData.formulaLength, initData.data));

    QPointF rotationOrigin;
    QSharedPointer<VPointF> originPoint;

    if (initData.rotationOrigin == NULL_ID)
    {
        rotationOrigin = GetOriginPoint(initData.source, initData.data, calcLength, calcAngle);
    }
    else
    {
        originPoint = initData.data->GeometricObject<VPointF>(initData.rotationOrigin);
        rotationOrigin = static_cast<QPointF>(*originPoint);
        patternGraph->AddEdge(initData.rotationOrigin, initData.id);
    }

    CreateDestination(initData, rotationOrigin, calcAngle, calcLength);

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
            VAbstractApplication::VApp()->getUndoStack()->beginMacro(tr("move"));
        }

        VAbstractTool::AddRecord(initData.id, Tool::Move, initData.doc);
        auto *tool = new VToolMove(initData);
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
auto VToolMove::GetFormulaAngle() const -> VFormula
{
    VFormula fAngle(formulaAngle, getData());
    fAngle.setToolId(m_id);
    fAngle.setPostfix(degreeSymbol);
    fAngle.Eval();
    return fAngle;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::SetFormulaAngle(const VFormula &value)
{
    if (!value.error())
    {
        formulaAngle = value.GetFormula(FormulaType::FromUser);

        QSharedPointer<VGObject> obj = VContainer::GetFakeGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolMove::GetFormulaRotationAngle() const -> VFormula
{
    VFormula fAngle(formulaRotationAngle, getData());
    fAngle.setToolId(m_id);
    fAngle.setPostfix(degreeSymbol);
    fAngle.Eval();
    return fAngle;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::SetFormulaRotationAngle(const VFormula &value)
{
    if (!value.error())
    {
        formulaRotationAngle = value.GetFormula(FormulaType::FromUser);

        QSharedPointer<VGObject> obj = VContainer::GetFakeGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolMove::GetFormulaLength() const -> VFormula
{
    VFormula fLength(formulaLength, getData());
    fLength.setToolId(m_id);
    fLength.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    fLength.Eval();
    return fLength;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::SetFormulaLength(const VFormula &value)
{
    if (!value.error())
    {
        formulaLength = value.GetFormula(FormulaType::FromUser);

        QSharedPointer<VGObject> obj = VContainer::GetFakeGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolMove::OriginPointName() const -> QString
{
    try
    {
        return VAbstractTool::data.GetGObject(origPointId)->name();
    }
    catch (const VExceptionBadId &)
    {
        return tr("Center point");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolMove>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogMove>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolMove *>(vis);
        SCASSERT(visual != nullptr)

        const bool osSeparator = VAbstractApplication::VApp()->Settings()->GetOsSeparator();
        const VTranslateVars *trVars = VAbstractApplication::VApp()->TrVars();

        visual->SetObjects(SourceToObjects(source));
        visual->SetAngle(trVars->FormulaToUser(formulaAngle, osSeparator));
        visual->SetRotationAngle(trVars->FormulaToUser(formulaRotationAngle, osSeparator));
        visual->SetLength(trVars->FormulaToUser(formulaLength, osSeparator));
        visual->SetRotationOriginPointId(origPointId);
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogMove> dialogTool = qobject_cast<DialogMove *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrAngle, dialogTool->GetAngle());
    doc->SetAttribute(domElement, AttrLength, dialogTool->GetLength());
    doc->SetAttribute(domElement, AttrCenter, QString().setNum(dialogTool->GetRotationOrigPointId()));
    doc->SetAttribute(domElement, AttrRotationAngle, dialogTool->GetRotationAngle());
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
void VToolMove::ReadToolAttributes(const QDomElement &domElement)
{
    VAbstractOperation::ReadToolAttributes(domElement);

    origPointId = VDomDocument::GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);
    formulaAngle = VDomDocument::GetParametrString(domElement, AttrAngle, QChar('0'));
    formulaRotationAngle = VDomDocument::GetParametrString(domElement, AttrRotationAngle, QChar('0'));
    formulaLength = VDomDocument::GetParametrString(domElement, AttrLength, QChar('0'));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VAbstractOperation::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrAngle, formulaAngle);
    doc->SetAttribute(tag, AttrRotationAngle, formulaRotationAngle);
    doc->SetAttribute(tag, AttrLength, formulaLength);
    doc->SetAttribute(tag, AttrCenter, QString().setNum(origPointId));
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolMove::MakeToolTip() const -> QString
{
    return QStringLiteral("<tr> <td><b>%1:</b> %2°</td> </tr>"
                          "<tr> <td><b>%3:</b> %4 %5</td> </tr>"
                          "<tr> <td><b>%6:</b> %7°</td> </tr>"
                          "<tr> <td><b>%8:</b> %9</td> </tr>"
                          "%10")
        .arg(tr("Angle"))                                                       // 1
        .arg(GetFormulaAngle().getDoubleValue())                                // 2
        .arg(tr("Length"))                                                      // 3
        .arg(GetFormulaLength().getDoubleValue())                               // 4
        .arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true), // 5
             tr("Rotation angle"))                                              // 6
        .arg(GetFormulaRotationAngle().getDoubleValue())                        // 7
        .arg(tr("Rotation origin point"),                                       // 8
             OriginPointName(),                                                 // 9
             VisibilityGroupToolTip());                                         // 10
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogMove> dialogTool = qobject_cast<DialogMove *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    ProcessOperationToolOptions(oldDomElement, newDomElement, dialogTool->GetSourceObjects());
}

//---------------------------------------------------------------------------------------------------------------------
VToolMove::VToolMove(const VToolMoveInitData &initData, QGraphicsItem *parent)
  : VAbstractOperation(initData, parent),
    formulaAngle(initData.formulaAngle),
    formulaRotationAngle(initData.formulaRotationAngle),
    formulaLength(initData.formulaLength),
    origPointId(initData.rotationOrigin)
{
    InitOperatedObjects();
    ToolCreation(initData.typeCreation);
}
