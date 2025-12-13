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
    dialogTool->SetOrigPointId(origPointId);
    dialogTool->SetAngle(formulaAngle);
    dialogTool->SetSuffix(suffix);
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
    initData.suffix = dialogTool->GetSuffix();
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
    qreal calcAngle = 0;

    calcAngle = CheckFormula(initData.id, initData.angle, initData.data);

    const auto originPoint = *initData.data->GeometricObject<VPointF>(initData.origin);
    const auto oPoint = static_cast<QPointF>(originPoint);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.destination.clear(); // Try to avoid mistake, value must be empty

        initData.id = initData.data->getNextId(); // Just reserve id for tool

        for (const auto &object : qAsConst(initData.source))
        {
            const QSharedPointer<VGObject> obj = initData.data->GetGObject(object.id);

            // This check helps to find missed objects in the switch
            Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 8, "Not all objects were handled.");

            QT_WARNING_PUSH
            QT_WARNING_DISABLE_GCC("-Wswitch-default")
            QT_WARNING_DISABLE_CLANG("-Wswitch-default")

            switch (obj->getType())
            {
                case GOType::Point:
                    initData.destination.append(
                        CreatePoint(initData.id, object, oPoint, calcAngle, initData.suffix, initData.data));
                    break;
                case GOType::Arc:
                    initData.destination.append(
                        CreateArc<VArc>(initData.id, object, oPoint, calcAngle, initData.suffix, initData.data));
                    break;
                case GOType::EllipticalArc:
                    initData.destination.append(CreateArc<VEllipticalArc>(initData.id, object, oPoint, calcAngle,
                                                                          initData.suffix, initData.data));
                    break;
                case GOType::Spline:
                    initData.destination.append(
                        CreateCurve<VSpline>(initData.id, object, oPoint, calcAngle, initData.suffix, initData.data));
                    break;
                case GOType::SplinePath:
                    initData.destination.append(CreateCurveWithSegments<VSplinePath>(
                        initData.id, object, oPoint, calcAngle, initData.suffix, initData.data));
                    break;
                case GOType::CubicBezier:
                    initData.destination.append(CreateCurve<VCubicBezier>(initData.id, object, oPoint, calcAngle,
                                                                          initData.suffix, initData.data));
                    break;
                case GOType::CubicBezierPath:
                    initData.destination.append(CreateCurveWithSegments<VCubicBezierPath>(
                        initData.id, object, oPoint, calcAngle, initData.suffix, initData.data));
                    break;
                case GOType::Unknown:
                case GOType::PlaceLabel:
                    Q_UNREACHABLE();
                    break;
            }

            QT_WARNING_POP
        }
    }
    else
    {
        for (int i = 0; i < initData.source.size(); ++i)
        {
            const SourceItem object = initData.source.at(i);
            const QSharedPointer<VGObject> obj = initData.data->GetGObject(object.id);

            // This check helps to find missed objects in the switch
            Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 8, "Not all objects were handled.");

            QT_WARNING_PUSH
            QT_WARNING_DISABLE_GCC("-Wswitch-default")
            QT_WARNING_DISABLE_CLANG("-Wswitch-default")

            switch (obj->getType())
            {
                case GOType::Point:
                {
                    UpdatePoint(initData.id, object, oPoint, calcAngle, initData.suffix, initData.data,
                                initData.destination.at(i));
                    break;
                }
                case GOType::Arc:
                    UpdateArc<VArc>(initData.id, object, oPoint, calcAngle, initData.suffix, initData.data,
                                    initData.destination.at(i).id);
                    break;
                case GOType::EllipticalArc:
                    UpdateArc<VEllipticalArc>(initData.id, object, oPoint, calcAngle, initData.suffix, initData.data,
                                              initData.destination.at(i).id);
                    break;
                case GOType::Spline:
                    UpdateCurve<VSpline>(initData.id, object, oPoint, calcAngle, initData.suffix, initData.data,
                                         initData.destination.at(i).id);
                    break;
                case GOType::SplinePath:
                    UpdateCurveWithSegments<VSplinePath>(initData.id, object, oPoint, calcAngle, initData.suffix,
                                                         initData.data, initData.destination.at(i).id);
                    break;
                case GOType::CubicBezier:
                    UpdateCurve<VCubicBezier>(initData.id, object, oPoint, calcAngle, initData.suffix, initData.data,
                                              initData.destination.at(i).id);
                    break;
                case GOType::CubicBezierPath:
                    UpdateCurveWithSegments<VCubicBezierPath>(initData.id, object, oPoint, calcAngle, initData.suffix,
                                                              initData.data, initData.destination.at(i).id);
                    break;
                case GOType::Unknown:
                case GOType::PlaceLabel:
                    Q_UNREACHABLE();
                    break;
            }

            QT_WARNING_POP
        }
        if (initData.parse != Document::FullParse)
        {
            initData.doc->UpdateToolData(initData.id, initData.data);
        }
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
        initData.doc->IncrementReferens(originPoint.getIdTool());
        for (const auto &object : qAsConst(initData.source))
        {
            initData.doc->IncrementReferens(initData.data->GetGObject(object.id)->getIdTool());
        }

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
void VToolRotation::SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies,
                               QList<quint32> &newDependencies)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogRotation> dialogTool = qobject_cast<DialogRotation *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    AddDependence(oldDependencies, origPointId);
    AddDependence(newDependencies, dialogTool->GetOrigPointId());

    doc->SetAttribute(domElement, AttrCenter, QString().setNum(dialogTool->GetOrigPointId()));
    doc->SetAttribute(domElement, AttrAngle, dialogTool->GetAngle());
    doc->SetAttribute(domElement, AttrSuffix, dialogTool->GetSuffix());
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrNotes,
                                         dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept -> bool { return notes.isEmpty(); });

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
auto VToolRotation::CreatePoint(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle,
                                const QString &suffix, VContainer *data) -> DestinationItem
{
    const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(sItem.id);
    VPointF rotated = point->Rotate(origin, angle, suffix);
    rotated.setIdObject(idTool);

    if (not sItem.alias.isEmpty())
    {
        rotated.setName(sItem.alias);
    }

    DestinationItem item;
    item.mx = rotated.mx();
    item.my = rotated.my();
    item.showLabel = rotated.IsShowLabel();
    item.id = data->AddGObject(new VPointF(rotated));
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
auto VToolRotation::CreateItem(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle,
                               const QString &suffix, VContainer *data) -> DestinationItem
{
    const QSharedPointer<Item> i = data->GeometricObject<Item>(sItem.id);
    Item rotated = i->Rotate(origin, angle, suffix);
    rotated.setIdObject(idTool);

    if (not sItem.alias.isEmpty())
    {
        rotated.SetAliasSuffix(sItem.alias);
    }

    if (sItem.penStyle != TypeLineDefault)
    {
        rotated.SetPenStyle(sItem.penStyle);
    }

    if (sItem.color != ColorDefault)
    {
        rotated.SetColor(sItem.color);
    }

    DestinationItem item;
    item.id = data->AddGObject(new Item(rotated));
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
auto VToolRotation::CreateArc(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle,
                              const QString &suffix, VContainer *data) -> DestinationItem
{
    const DestinationItem item = CreateItem<Item>(idTool, sItem, origin, angle, suffix, data);
    data->AddArc(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
auto VToolRotation::CreateCurve(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle,
                                const QString &suffix, VContainer *data) -> DestinationItem
{
    const DestinationItem item = CreateItem<Item>(idTool, sItem, origin, angle, suffix, data);
    data->AddSpline(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
auto VToolRotation::CreateCurveWithSegments(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle,
                                            const QString &suffix, VContainer *data) -> DestinationItem
{
    const DestinationItem item = CreateItem<Item>(idTool, sItem, origin, angle, suffix, data);
    data->AddCurveWithSegments(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolRotation::UpdatePoint(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle,
                                const QString &suffix, VContainer *data, const DestinationItem &item)
{
    const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(sItem.id);
    VPointF rotated = point->Rotate(origin, angle, suffix);
    rotated.setIdObject(idTool);
    rotated.setMx(item.mx);
    rotated.setMy(item.my);
    rotated.SetShowLabel(item.showLabel);

    if (not sItem.alias.isEmpty())
    {
        rotated.setName(sItem.alias);
    }

    data->UpdateGObject(item.id, new VPointF(rotated));
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VToolRotation::UpdateItem(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle,
                               const QString &suffix, VContainer *data, quint32 id)
{
    const QSharedPointer<Item> i = data->GeometricObject<Item>(sItem.id);
    Item rotated = i->Rotate(origin, angle, suffix);
    rotated.setIdObject(idTool);

    if (not sItem.alias.isEmpty())
    {
        rotated.SetAliasSuffix(sItem.alias);
    }

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
template <class Item>
void VToolRotation::UpdateArc(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle,
                              const QString &suffix, VContainer *data, quint32 id)
{
    UpdateItem<Item>(idTool, sItem, origin, angle, suffix, data, id);
    data->AddArc(data->GeometricObject<Item>(id), id);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VToolRotation::UpdateCurve(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle,
                                const QString &suffix, VContainer *data, quint32 id)
{
    UpdateItem<Item>(idTool, sItem, origin, angle, suffix, data, id);
    data->AddSpline(data->GeometricObject<Item>(id), id);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VToolRotation::UpdateCurveWithSegments(quint32 idTool, const SourceItem &sItem, const QPointF &origin, qreal angle,
                                            const QString &suffix, VContainer *data, quint32 id)
{
    UpdateItem<Item>(idTool, sItem, origin, angle, suffix, data, id);
    data->AddCurveWithSegments(data->GeometricObject<Item>(id), id);
}
