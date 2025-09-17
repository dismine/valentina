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
#include <new>
#include <qiterator.h>

#include "../../../dialogs/tools/dialogmove.h"
#include "../../../dialogs/tools/dialogtool.h"
#include "../../../visualization/line/operation/vistoolmove.h"
#include "../../../visualization/visualization.h"
#include "../../vabstracttool.h"
#include "../../vdatatool.h"
#include "../ifc/ifcdef.h"
#include "../vdrawtool.h"
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
} // namespace

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogMove> dialogTool = qobject_cast<DialogMove *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    dialogTool->SetAngle(formulaAngle);
    dialogTool->SetRotationAngle(formulaRotationAngle);
    dialogTool->SetLength(formulaLength);
    dialogTool->SetSuffix(suffix);
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
    qreal calcAngle = 0;
    qreal calcRotationAngle = 0;
    qreal calcLength = 0;

    calcAngle = CheckFormula(initData.id, initData.formulaAngle, initData.data);
    calcRotationAngle = CheckFormula(initData.id, initData.formulaRotationAngle, initData.data);
    calcLength =
        VAbstractValApplication::VApp()->toPixel(CheckFormula(initData.id, initData.formulaLength, initData.data));

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
    }

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
                    initData.destination.append(CreatePoint(initData.id, object, calcAngle, calcLength,
                                                            calcRotationAngle, rotationOrigin, initData.suffix,
                                                            initData.data));
                    break;
                case GOType::Arc:
                    initData.destination.append(CreateArc<VArc>(initData.id, object, calcAngle, calcLength,
                                                                calcRotationAngle, rotationOrigin, initData.suffix,
                                                                initData.data));
                    break;
                case GOType::EllipticalArc:
                    initData.destination.append(CreateArc<VEllipticalArc>(initData.id, object, calcAngle, calcLength,
                                                                          calcRotationAngle, rotationOrigin,
                                                                          initData.suffix, initData.data));
                    break;
                case GOType::Spline:
                    initData.destination.append(CreateCurve<VSpline>(initData.id, object, calcAngle, calcLength,
                                                                     calcRotationAngle, rotationOrigin, initData.suffix,
                                                                     initData.data));
                    break;
                case GOType::SplinePath:
                    initData.destination.append(CreateCurveWithSegments<VSplinePath>(
                        initData.id, object, calcAngle, calcLength, calcRotationAngle, rotationOrigin, initData.suffix,
                        initData.data));
                    break;
                case GOType::CubicBezier:
                    initData.destination.append(CreateCurve<VCubicBezier>(initData.id, object, calcAngle, calcLength,
                                                                          calcRotationAngle, rotationOrigin,
                                                                          initData.suffix, initData.data));
                    break;
                case GOType::CubicBezierPath:
                    initData.destination.append(CreateCurveWithSegments<VCubicBezierPath>(
                        initData.id, object, calcAngle, calcLength, calcRotationAngle, rotationOrigin, initData.suffix,
                        initData.data));
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
                    UpdatePoint(initData.id, object, calcAngle, calcLength, calcRotationAngle, rotationOrigin,
                                initData.suffix, initData.data, initData.destination.at(i));
                    break;
                case GOType::Arc:
                    UpdateArc<VArc>(initData.id, object, calcAngle, calcLength, calcRotationAngle, rotationOrigin,
                                    initData.suffix, initData.data, initData.destination.at(i).id);
                    break;
                case GOType::EllipticalArc:
                    UpdateArc<VEllipticalArc>(initData.id, object, calcAngle, calcLength, calcRotationAngle,
                                              rotationOrigin, initData.suffix, initData.data,
                                              initData.destination.at(i).id);
                    break;
                case GOType::Spline:
                    UpdateCurve<VSpline>(initData.id, object, calcAngle, calcLength, calcRotationAngle, rotationOrigin,
                                         initData.suffix, initData.data, initData.destination.at(i).id);
                    break;
                case GOType::SplinePath:
                    UpdateCurveWithSegments<VSplinePath>(initData.id, object, calcAngle, calcLength, calcRotationAngle,
                                                         rotationOrigin, initData.suffix, initData.data,
                                                         initData.destination.at(i).id);
                    break;
                case GOType::CubicBezier:
                    UpdateCurve<VCubicBezier>(initData.id, object, calcAngle, calcLength, calcRotationAngle,
                                              rotationOrigin, initData.suffix, initData.data,
                                              initData.destination.at(i).id);
                    break;
                case GOType::CubicBezierPath:
                    UpdateCurveWithSegments<VCubicBezierPath>(initData.id, object, calcAngle, calcLength,
                                                              calcRotationAngle, rotationOrigin, initData.suffix,
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
            VAbstractApplication::VApp()->getUndoStack()->beginMacro(tr("move"));
        }

        VAbstractTool::AddRecord(initData.id, Tool::Move, initData.doc);
        auto *tool = new VToolMove(initData);
        initData.scene->addItem(tool);
        InitOperationToolConnections(initData.scene, tool);
        VAbstractPattern::AddTool(initData.id, tool);

        if (not originPoint.isNull())
        {
            initData.doc->IncrementReferens(originPoint->getIdTool());
        }

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
    if (value.error() == false)
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
void VToolMove::SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies, QList<quint32> &newDependencies)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogMove> dialogTool = qobject_cast<DialogMove *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    AddDependence(oldDependencies, origPointId);
    AddDependence(newDependencies, dialogTool->GetRotationOrigPointId());

    doc->SetAttribute(domElement, AttrAngle, dialogTool->GetAngle());
    doc->SetAttribute(domElement, AttrLength, dialogTool->GetLength());
    doc->SetAttribute(domElement, AttrSuffix, dialogTool->GetSuffix());
    doc->SetAttribute(domElement, AttrCenter, QString().setNum(dialogTool->GetRotationOrigPointId()));
    doc->SetAttribute(domElement, AttrRotationAngle, dialogTool->GetRotationAngle());
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrNotes, dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });

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

//---------------------------------------------------------------------------------------------------------------------
auto VToolMove::CreatePoint(quint32 idTool, const SourceItem &sItem, qreal angle, qreal length, qreal rotationAngle,
                            const QPointF &rotationOrigin, const QString &suffix, VContainer *data) -> DestinationItem
{
    const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(sItem.id);
    VPointF moved = point->Move(length, angle, suffix).Rotate(rotationOrigin, rotationAngle);
    moved.setIdObject(idTool);

    if (not sItem.alias.isEmpty())
    {
        moved.setName(sItem.alias);
    }

    DestinationItem item;
    item.mx = moved.mx();
    item.my = moved.my();
    item.showLabel = moved.IsShowLabel();
    item.id = data->AddGObject(new VPointF(moved));
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
auto VToolMove::CreateArc(quint32 idTool, const SourceItem &sItem, qreal angle, qreal length, qreal rotationAngle,
                          const QPointF &rotationOrigin, const QString &suffix, VContainer *data) -> DestinationItem
{
    const DestinationItem item =
        CreateItem<Item>(idTool, sItem, angle, length, rotationAngle, rotationOrigin, suffix, data);
    data->AddArc(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolMove::UpdatePoint(quint32 idTool, const SourceItem &sItem, qreal angle, qreal length, qreal rotationAngle,
                            const QPointF &rotationOrigin, const QString &suffix, VContainer *data,
                            const DestinationItem &item)
{
    const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(sItem.id);
    VPointF moved = point->Move(length, angle, suffix).Rotate(rotationOrigin, rotationAngle);
    moved.setIdObject(idTool);
    moved.setMx(item.mx);
    moved.setMy(item.my);
    moved.SetShowLabel(item.showLabel);

    if (not sItem.alias.isEmpty())
    {
        moved.setName(sItem.alias);
    }

    data->UpdateGObject(item.id, new VPointF(moved));
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VToolMove::UpdateArc(quint32 idTool, const SourceItem &sItem, qreal angle, qreal length, qreal rotationAngle,
                          const QPointF &rotationOrigin, const QString &suffix, VContainer *data, quint32 id)
{
    UpdateItem<Item>(idTool, sItem, angle, length, rotationAngle, rotationOrigin, suffix, data, id);
    data->AddArc(data->GeometricObject<Item>(id), id);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
auto VToolMove::CreateItem(quint32 idTool, const SourceItem &sItem, qreal angle, qreal length, qreal rotationAngle,
                           const QPointF &rotationOrigin, const QString &suffix, VContainer *data) -> DestinationItem
{
    const QSharedPointer<Item> i = data->GeometricObject<Item>(sItem.id);
    Item moved = i->Move(length, angle, suffix).Rotate(rotationOrigin, rotationAngle);
    moved.setIdObject(idTool);

    if (not sItem.alias.isEmpty())
    {
        moved.SetAliasSuffix(sItem.alias);
    }

    if (sItem.penStyle != TypeLineDefault)
    {
        moved.SetPenStyle(sItem.penStyle);
    }

    if (sItem.color != ColorDefault)
    {
        moved.SetColor(sItem.color);
    }

    DestinationItem item;
    item.id = data->AddGObject(new Item(moved));
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
auto VToolMove::CreateCurve(quint32 idTool, const SourceItem &sItem, qreal angle, qreal length, qreal rotationAngle,
                            const QPointF &rotationOrigin, const QString &suffix, VContainer *data) -> DestinationItem
{
    const DestinationItem item =
        CreateItem<Item>(idTool, sItem, angle, length, rotationAngle, rotationOrigin, suffix, data);
    data->AddSpline(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
auto VToolMove::CreateCurveWithSegments(quint32 idTool, const SourceItem &sItem, qreal angle, qreal length,
                                        qreal rotationAngle, const QPointF &rotationOrigin, const QString &suffix,
                                        VContainer *data) -> DestinationItem
{
    const DestinationItem item =
        CreateItem<Item>(idTool, sItem, angle, length, rotationAngle, rotationOrigin, suffix, data);
    data->AddCurveWithSegments(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VToolMove::UpdateItem(quint32 idTool, const SourceItem &sItem, qreal angle, qreal length, qreal rotationAngle,
                           const QPointF &rotationOrigin, const QString &suffix, VContainer *data, quint32 id)
{
    const QSharedPointer<Item> i = data->GeometricObject<Item>(sItem.id);
    Item moved = i->Move(length, angle, suffix).Rotate(rotationOrigin, rotationAngle);
    moved.setIdObject(idTool);

    if (not sItem.alias.isEmpty())
    {
        moved.SetAliasSuffix(sItem.alias);
    }

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
template <class Item>
void VToolMove::UpdateCurve(quint32 idTool, const SourceItem &sItem, qreal angle, qreal length, qreal rotationAngle,
                            const QPointF &rotationOrigin, const QString &suffix, VContainer *data, quint32 id)
{
    UpdateItem<Item>(idTool, sItem, angle, length, rotationAngle, rotationOrigin, suffix, data, id);
    data->AddSpline(data->GeometricObject<Item>(id), id);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VToolMove::UpdateCurveWithSegments(quint32 idTool, const SourceItem &sItem, qreal angle, qreal length,
                                        qreal rotationAngle, const QPointF &rotationOrigin, const QString &suffix,
                                        VContainer *data, quint32 id)
{
    UpdateItem<Item>(idTool, sItem, angle, length, rotationAngle, rotationOrigin, suffix, data, id);
    data->AddCurveWithSegments(data->GeometricObject<Item>(id), id);
}
