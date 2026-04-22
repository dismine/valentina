/************************************************************************
 **
 **  @file   vtoolcurveintersectaxis.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 10, 2014
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

#include "vtoolcurveintersectaxis.h"

#include <climits>
#include <QLineF>
#include <QMap>
#include <QRectF>
#include <QSharedPointer>
#include <QUndoStack>
#include <QVector>

#include "../../../../../dialogs/tools/dialogcurveintersectaxis.h"
#include "../../../../../dialogs/tools/dialogtool.h"
#include "../../../../../undocommands/label/movesegmentlabel.h"
#include "../../../../../undocommands/renameobject.h"
#include "../../../../../undocommands/savetooloptions.h"
#include "../../../../vabstracttool.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../toolcut/vtoolcutsplinepath.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/exception/vexception.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vtools/visualization/line/vistoolcurveintersectaxis.h"
#include "../vtools/visualization/visualization.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vsegmentlabel.h"
#include "vtoollinepoint.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

template <class T> class QSharedPointer;

const QString VToolCurveIntersectAxis::ToolType = QStringLiteral("curveIntersectAxis");

//---------------------------------------------------------------------------------------------------------------------
VToolCurveIntersectAxis::VToolCurveIntersectAxis(const VToolCurveIntersectAxisInitData &initData, QGraphicsItem *parent)
  : VToolLinePoint(initData.doc,
                   initData.data,
                   initData.id,
                   initData.typeLine,
                   initData.lineColor,
                   QString(),
                   initData.basePointId,
                   0,
                   initData.notes,
                   parent),
    formulaAngle(initData.formulaAngle),
    curveId(initData.curveId),
    m_name1(initData.name1),
    m_name2(initData.name2),
    m_aliasSuffix1(initData.aliasSuffix1),
    m_aliasSuffix2(initData.aliasSuffix2),
    m_segment1Id(initData.segment1Id),
    m_segment2Id(initData.segment2Id),
    m_segment1Mx(initData.segment1Mx),
    m_segment1My(initData.segment1My),
    m_segment2Mx(initData.segment2Mx),
    m_segment2My(initData.segment2My)
{
    auto const CreateSegmentLabel =
        [this](quint32 segmentId, qreal mx, qreal my, auto chooseSlot, auto selectSlot, auto posChangedSlot)
        -> VSegmentLabel *
    {
        QSharedPointer<VAbstractCurve> const curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(segmentId);
        VPointF pos = curve->GetMidpoint();
        pos.setMx(mx);
        pos.setMy(my);
        pos.setX(pos.x() - this->pos().x());
        pos.setY(pos.y() - this->pos().y());

        auto *label = new VSegmentLabel(pos, curve, this);
        connect(label, &VSegmentLabel::SegmentChoosed, this, chooseSlot);
        connect(label, &VSegmentLabel::SegmentSelected, this, selectSlot);
        connect(label, &VSegmentLabel::LabelPositionChanged, this, posChangedSlot);
        return label;
    };

    m_segment1Label = CreateSegmentLabel(m_segment1Id,
                                         m_segment1Mx,
                                         m_segment1My,
                                         &VToolCurveIntersectAxis::SegmentChoosed,
                                         &VToolCurveIntersectAxis::PointSelected,
                                         &VToolCurveIntersectAxis::Segment1LabelPositionChanged);

    m_segment2Label = CreateSegmentLabel(m_segment2Id,
                                         m_segment2Mx,
                                         m_segment2My,
                                         &VToolCurveIntersectAxis::SegmentChoosed,
                                         &VToolCurveIntersectAxis::PointSelected,
                                         &VToolCurveIntersectAxis::Segment2LabelPositionChanged);

    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::ProcessToolOptions(const QDomElement &oldDomElement,
                                                 const QDomElement &newDomElement,
                                                 const ToolChanges &changes)
{
    if (!changes.HasChanges())
    {
        VToolLinePoint::ApplyToolOptions(oldDomElement, newDomElement);
        return;
    }

    QUndoStack *undoStack = VAbstractApplication::VApp()->getUndoStack();
    auto *newGroup = new QUndoCommand(); // an empty command
    newGroup->setText(tr("save tool options"));

    auto *saveOptions = new SaveToolOptions(oldDomElement, newDomElement, doc, m_id, newGroup);
    saveOptions->SetInGroup(true);
    connect(saveOptions, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);

    if (changes.LabelChanged())
    {
        const auto *renameLabel = new RenameLabel(changes.oldLabel, changes.newLabel, doc, m_id, newGroup);
        if (!changes.Name1Changed() && !changes.Name2Changed() && !changes.AliasSuffix1Changed()
            && !changes.AliasSuffix2Changed())
        {
            connect(renameLabel, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(curveId);
    const CurveAliasType curveType = RenameAlias::CurveType(curve->getType());

    if (changes.Name1Changed())
    {
        const auto *renameName
            = new RenameAlias(curveType, changes.oldName1, changes.newName1, doc, m_segment1Id, newGroup);
        if (!changes.Name2Changed() && !changes.AliasSuffix1Changed() && !changes.AliasSuffix2Changed())
        {
            connect(renameName, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    if (changes.Name2Changed())
    {
        const auto *renameName
            = new RenameAlias(curveType, changes.oldName2, changes.newName2, doc, m_segment2Id, newGroup);
        if (!changes.AliasSuffix1Changed() && !changes.AliasSuffix2Changed())
        {
            connect(renameName, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    if (changes.AliasSuffix1Changed())
    {
        const auto *renameAlias
            = new RenameAlias(curveType, changes.oldAliasSuffix1, changes.newAliasSuffix1, doc, m_segment1Id, newGroup);
        if (!changes.AliasSuffix2Changed())
        {
            connect(renameAlias, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    if (changes.AliasSuffix2Changed())
    {
        const auto *renameAlias
            = new RenameAlias(curveType, changes.oldAliasSuffix1, changes.newAliasSuffix1, doc, m_segment2Id, newGroup);
        connect(renameAlias, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    }

    undoStack->push(newGroup);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::UpdateNameField(VToolCurveIntersectAxisNameField field, const QString &value)
{
    // Validation - name fields require non-empty values
    if ((field == VToolCurveIntersectAxisNameField::Name1 || field == VToolCurveIntersectAxisNameField::Name2)
        && value.isEmpty())
    {
        return; // Name is required
    }

    // Validate format and uniqueness for non-empty values
    if (!value.isEmpty())
    {
        QSharedPointer<VAbstractCurve> const curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(curveId);
        const QString fullName = curve->GetTypeHead() + value;

        if (QRegularExpression const rx(NameRegExp()); !rx.match(fullName).hasMatch())
        {
            return; // Invalid format
        }

        if (!VAbstractTool::data.IsUnique(fullName))
        {
            return; // Not unique in data
        }

        // Check conflicts with other identifiers
        if (value == GetFieldValue(VToolCurveIntersectAxisNameField::Name1, field)
            || value == GetFieldValue(VToolCurveIntersectAxisNameField::Name2, field)
            || value == GetFieldValue(VToolCurveIntersectAxisNameField::AliasSuffix1, field)
            || value == GetFieldValue(VToolCurveIntersectAxisNameField::AliasSuffix2, field))
        {
            return; // Conflicts with other identifiers
        }
    }

    QDomElement const oldDomElement = doc->FindElementById(m_id, getTagName());
    if (!oldDomElement.isElement())
    {
        qDebug("Can't find tool with id = %u", m_id);
        return;
    }

    const QString label = VAbstractTool::data.GeometricObject<VPointF>(m_id)->name();

    // Build changes struct
    ToolChanges const changes
        = {.oldLabel = label,
           .newLabel = label,
           .oldName1 = m_name1,
           .newName1 = (field == VToolCurveIntersectAxisNameField::Name1) ? value : m_name1,
           .oldName2 = m_name2,
           .newName2 = (field == VToolCurveIntersectAxisNameField::Name2) ? value : m_name2,
           .oldAliasSuffix1 = m_aliasSuffix1,
           .newAliasSuffix1 = (field == VToolCurveIntersectAxisNameField::AliasSuffix1) ? value : m_aliasSuffix1,
           .oldAliasSuffix2 = m_aliasSuffix2,
           .newAliasSuffix2 = (field == VToolCurveIntersectAxisNameField::AliasSuffix2) ? value : m_aliasSuffix2};

    // Update the appropriate member variable
    switch (field)
    {
        case VToolCurveIntersectAxisNameField::Name1:
            m_name1 = value;
            break;
        case VToolCurveIntersectAxisNameField::Name2:
            m_name2 = value;
            break;
        case VToolCurveIntersectAxisNameField::AliasSuffix1:
            m_aliasSuffix1 = value;
            break;
        case VToolCurveIntersectAxisNameField::AliasSuffix2:
            m_aliasSuffix2 = value;
            break;
        default:
            break;
    }

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    QDomElement newDomElement = oldDomElement.cloneNode().toElement();
    SaveOptions(newDomElement, obj);
    ProcessToolOptions(oldDomElement, newDomElement, changes);
}

//---------------------------------------------------------------------------------------------------------------------
// Helper to get field values, excluding the current field being updated
auto VToolCurveIntersectAxis::GetFieldValue(VToolCurveIntersectAxisNameField field,
                                            VToolCurveIntersectAxisNameField excludeField) const -> QString
{
    if (field == excludeField)
    {
        return {}; // Don't check against self
    }

    switch (field)
    {
        case VToolCurveIntersectAxisNameField::Name1:
            return m_name1;
        case VToolCurveIntersectAxisNameField::Name2:
            return m_name2;
        case VToolCurveIntersectAxisNameField::AliasSuffix1:
            return m_aliasSuffix1;
        case VToolCurveIntersectAxisNameField::AliasSuffix2:
            return m_aliasSuffix2;
        default:
            break;
    }
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCurveIntersectAxis::GatherToolChanges() const -> VToolCurveIntersectAxis::ToolChanges
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogCurveIntersectAxis> dialogTool = qobject_cast<DialogCurveIntersectAxis *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    return {.oldLabel = VAbstractTool::data.GeometricObject<VPointF>(m_id)->name(),
            .newLabel = dialogTool->GetPointName(),
            .oldName1 = GetName1(),
            .newName1 = dialogTool->GetName1(),
            .oldName2 = GetName2(),
            .newName2 = dialogTool->GetName2(),
            .oldAliasSuffix1 = m_aliasSuffix1,
            .newAliasSuffix1 = dialogTool->GetAliasSuffix1(),
            .oldAliasSuffix2 = m_aliasSuffix2,
            .newAliasSuffix2 = dialogTool->GetAliasSuffix2()};
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    m_dialog->setModal(true);
    const QPointer<DialogCurveIntersectAxis> dialogTool = qobject_cast<DialogCurveIntersectAxis *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetTypeLine(m_lineType);
    dialogTool->SetLineColor(lineColor);
    dialogTool->SetAngle(formulaAngle);
    dialogTool->SetBasePointId(basePointId);
    dialogTool->setCurveId(curveId);
    dialogTool->SetPointName(p->name());
    dialogTool->SetNotes(m_notes);
    dialogTool->SetAliasSuffix1(m_aliasSuffix1);
    dialogTool->SetAliasSuffix2(m_aliasSuffix2);
    dialogTool->SetName1(GetName1());
    dialogTool->SetName2(GetName2());
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCurveIntersectAxis::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene,
                                     VAbstractPattern *doc, VContainer *data) -> VToolCurveIntersectAxis *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogCurveIntersectAxis> dialogTool = qobject_cast<DialogCurveIntersectAxis *>(dialog);
    SCASSERT(not dialogTool.isNull())

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    VToolCurveIntersectAxisInitData initData;

    QT_WARNING_POP

    initData.formulaAngle = dialogTool->GetAngle();
    initData.basePointId = dialogTool->GetBasePointId();
    initData.curveId = dialogTool->getCurveId();
    initData.typeLine = dialogTool->GetTypeLine();
    initData.lineColor = dialogTool->GetLineColor();
    initData.name = dialogTool->GetPointName();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();
    initData.aliasSuffix1 = dialogTool->GetAliasSuffix1();
    initData.aliasSuffix2 = dialogTool->GetAliasSuffix2();
    initData.name1 = dialogTool->GetName1();
    initData.name2 = dialogTool->GetName2();

    VToolCurveIntersectAxis *point = Create(initData);
    if (point != nullptr)
    {
        point->m_dialog = dialog;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCurveIntersectAxis::Create(VToolCurveIntersectAxisInitData &initData) -> VToolCurveIntersectAxis *
{
    const QSharedPointer<VPointF> basePoint = initData.data->GeometricObject<VPointF>(initData.basePointId);
    const qreal angle = CheckFormula(initData.id, initData.formulaAngle, initData.data);
    const QSharedPointer<VAbstractCurve> curve = initData.data->GeometricObject<VAbstractCurve>(initData.curveId);

    QPointF fPoint;
    const bool success = FindPoint(static_cast<QPointF>(*basePoint), angle, curve->GetPoints(), &fPoint);

    if (not success)
    {
        const QString errorMsg = tr("Error calculating point '%1'. There is no intersection with curve '%2' and axis"
                                    " through point '%3' with angle %4°")
                                     .arg(initData.name, curve->ObjectName(), basePoint->name())
                                     .arg(angle);
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VExceptionObjectError(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
    }

    const qreal segLength = curve->GetLengthByPoint(fPoint);

    auto *p = new VPointF(fPoint, initData.name, initData.mx, initData.my);
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

    initData.data->AddLine(initData.basePointId, initData.id);

    SegmentDetails details{.typeCreation = initData.typeCreation,
                           .curveType = curve->getType(),
                           .segLength = segLength,
                           .p = *p,
                           .curveId = initData.curveId,
                           .data = initData.data,
                           .doc = initData.doc,
                           .name1 = initData.name1,
                           .name2 = initData.name2,
                           .alias1 = initData.aliasSuffix1,
                           .alias2 = initData.aliasSuffix2,
                           .id = initData.id,
                           .name1AttrName = AttrName1,
                           .name2AttrName = AttrName2,
                           .segment1Id = initData.segment1Id,
                           .segment2Id = initData.segment2Id};

    VToolSinglePoint::InitSegments(details);

    initData.name1 = details.name1;
    initData.name2 = details.name2;

    patternGraph->AddEdge(initData.basePointId, initData.id);
    patternGraph->AddEdge(initData.curveId, initData.id);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.segment1Id = details.segment1Id;
        initData.segment2Id = details.segment2Id;
    }

    patternGraph->AddVertex(initData.segment1Id, VNodeType::OBJECT, initData.doc->PatternBlockMapper()->GetActiveId());
    patternGraph->AddVertex(initData.segment2Id, VNodeType::OBJECT, initData.doc->PatternBlockMapper()->GetActiveId());

    patternGraph->AddEdge(initData.id, initData.segment1Id);
    patternGraph->AddEdge(initData.id, initData.segment2Id);

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::CurveIntersectAxis, initData.doc);
        auto *point = new VToolCurveIntersectAxis(initData);
        initData.scene->addItem(point);
        InitPointToolConnections(initData.scene, point);
        InitSegmentConnections(initData.scene, point, curve->getType());
        VAbstractPattern::AddTool(initData.id, point);
        return point;
    }

    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCurveIntersectAxis::FindPoint(const QPointF &point, qreal angle, const QVector<QPointF> &curvePoints,
                                        QPointF *intersectionPoint) -> bool
{
    return VAbstractCurve::CurveIntersectAxis(point, angle, curvePoints, intersectionPoint);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCurveIntersectAxis::GetFormulaAngle() const -> VFormula
{
    VFormula fAngle(formulaAngle, getData());
    fAngle.setToolId(m_id);
    fAngle.setPostfix(degreeSymbol);
    fAngle.Eval();
    return fAngle;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SetFormulaAngle(const VFormula &value)
{
    if (!value.error())
    {
        formulaAngle = value.GetFormula(FormulaType::FromUser);

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCurveIntersectAxis::GetName1() const -> QString
{
    return m_name1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SetName1(const QString &name)
{
    UpdateNameField(VToolCurveIntersectAxisNameField::Name1, name);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCurveIntersectAxis::GetName2() const -> QString
{
    return m_name2;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SetName2(const QString &name)
{
    UpdateNameField(VToolCurveIntersectAxisNameField::Name2, name);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCurveIntersectAxis::GetAliasSuffix1() const -> QString
{
    return m_aliasSuffix1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SetAliasSuffix1(const QString &alias)
{
    UpdateNameField(VToolCurveIntersectAxisNameField::AliasSuffix1, alias);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCurveIntersectAxis::GetAliasSuffix2() const -> QString
{
    return m_aliasSuffix2;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SetAliasSuffix2(const QString &alias)
{
    UpdateNameField(VToolCurveIntersectAxisNameField::AliasSuffix2, alias);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCurveIntersectAxis::CurveName() const -> QString
{
    return VAbstractTool::data.GetGObject(curveId)->ObjectName();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolCurveIntersectAxis>(show);
    m_segment1Label->SetLabelVisible(show);
    m_segment2Label->SetLabelVisible(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::ChangeSegmentLabelPosition(SegmentLabel segment, const QPointF &pos)
{
    if (segment == SegmentLabel::Segment1)
    {
        m_segment1Label->SetLabelPosition(pos);
    }
    else if (segment == SegmentLabel::Segment2)
    {
        m_segment2Label->SetLabelPosition(pos);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCurveIntersectAxis::IsRemovable() const -> RemoveStatus
{
    if (!doc->IsPatternGraphComplete())
    {
        return RemoveStatus::Pending; // Data not ready yet
    }

    VPatternGraph const *patternGraph = doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    auto Filter = [](const auto &node) -> auto
    { return node.type != VNodeType::MODELING_TOOL && node.type != VNodeType::MODELING_OBJECT; };

    auto const segment1Dependecies = patternGraph->TryGetDependentNodes(m_segment1Id, 500, Filter);
    if (!segment1Dependecies)
    {
        return RemoveStatus::Pending; // Lock timeout
    }

    if (!segment1Dependecies->isEmpty())
    {
        return RemoveStatus::Blocked;
    }

    auto const segment2Dependecies = patternGraph->TryGetDependentNodes(m_segment2Id, 500, Filter);
    if (!segment2Dependecies)
    {
        return RemoveStatus::Pending; // Lock timeout
    }

    if (!segment2Dependecies->isEmpty())
    {
        return RemoveStatus::Blocked;
    }

    return RemoveStatus::Removable;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogCurveIntersectAxis>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogCurveIntersectAxis> dialogTool = qobject_cast<DialogCurveIntersectAxis *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrName, dialogTool->GetPointName());
    doc->SetAttribute(domElement, AttrTypeLine, dialogTool->GetTypeLine());
    doc->SetAttribute(domElement, AttrLineColor, dialogTool->GetLineColor());
    doc->SetAttribute(domElement, AttrAngle, dialogTool->GetAngle());
    doc->SetAttribute(domElement, AttrBasePoint, QString().setNum(dialogTool->GetBasePointId()));
    doc->SetAttribute(domElement, AttrCurve, QString().setNum(dialogTool->getCurveId()));
    doc->SetAttribute(domElement, AttrName1, dialogTool->GetName1());
    doc->SetAttribute(domElement, AttrName2, dialogTool->GetName2());
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrAlias1,
                                         dialogTool->GetAliasSuffix1(),
                                         [](const QString &suffix) noexcept -> bool { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrAlias2,
                                         dialogTool->GetAliasSuffix2(),
                                         [](const QString &suffix) noexcept -> bool { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrNotes,
                                         dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept -> bool { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolLinePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrAngle, formulaAngle);
    doc->SetAttribute(tag, AttrBasePoint, basePointId);
    doc->SetAttribute(tag, AttrCurve, curveId);
    doc->SetAttribute(tag, AttrName1, m_name1);
    doc->SetAttribute(tag, AttrName2, m_name2);
    doc->SetAttributeOrRemoveIf<QString>(tag,
                                         AttrAlias1,
                                         m_aliasSuffix1,
                                         [](const QString &suffix) noexcept -> bool { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(tag,
                                         AttrAlias2,
                                         m_aliasSuffix2,
                                         [](const QString &suffix) noexcept -> bool { return suffix.isEmpty(); });
    doc->SetAttribute(tag, AttrSegment1Id, m_segment1Id);
    doc->SetAttribute(tag, AttrSegment2Id, m_segment2Id);

    const VAbstractValApplication *app = VAbstractValApplication::VApp();
    doc->SetAttribute(tag, AttrSegment1Mx, app->fromPixel(m_segment1Mx));
    doc->SetAttribute(tag, AttrSegment1My, app->fromPixel(m_segment1My));
    doc->SetAttribute(tag, AttrSegment2Mx, app->fromPixel(m_segment2Mx));
    doc->SetAttribute(tag, AttrSegment2My, app->fromPixel(m_segment2Mx));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::ReadToolAttributes(const QDomElement &domElement)
{
    VToolLinePoint::ReadToolAttributes(domElement);

    m_lineType = VAbstractPattern::GetParametrString(domElement, AttrTypeLine, TypeLineLine);
    lineColor = VAbstractPattern::GetParametrString(domElement, AttrLineColor, ColorBlack);
    basePointId = VAbstractPattern::GetParametrUInt(domElement, AttrBasePoint, NULL_ID_STR);
    curveId = VAbstractPattern::GetParametrUInt(domElement, AttrCurve, NULL_ID_STR);
    formulaAngle = VAbstractPattern::GetParametrString(domElement, AttrAngle, QString());
    m_name1 = VAbstractPattern::GetParametrEmptyString(domElement, AttrName1);
    m_name2 = VAbstractPattern::GetParametrEmptyString(domElement, AttrName2);
    m_aliasSuffix1 = VAbstractPattern::GetParametrEmptyString(domElement, AttrAlias1);
    m_aliasSuffix2 = VAbstractPattern::GetParametrEmptyString(domElement, AttrAlias2);
    m_segment1Id = VAbstractPattern::GetParametrId(domElement, AttrSegment1Id);
    m_segment2Id = VAbstractPattern::GetParametrId(domElement, AttrSegment2Id);

    const VAbstractValApplication *app = VAbstractValApplication::VApp();

    const QString labelMXStr = QString::number(app->fromPixel(labelMX));
    const QString labelMYStr = QString::number(app->fromPixel(labelMY));

    m_segment1Mx = app->toPixel(VAbstractPattern::GetParametrDouble(domElement, AttrSegment1Mx, labelMXStr));
    m_segment1My = app->toPixel(VAbstractPattern::GetParametrDouble(domElement, AttrSegment1My, labelMYStr));
    m_segment2Mx = app->toPixel(VAbstractPattern::GetParametrDouble(domElement, AttrSegment2Mx, labelMXStr));
    m_segment2My = app->toPixel(VAbstractPattern::GetParametrDouble(domElement, AttrSegment2My, labelMYStr));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolCurveIntersectAxis *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetCurveId(curveId);
        visual->setAxisPointId(basePointId);
        visual->SetAngle(VAbstractApplication::VApp()->TrVars()->FormulaToUser(
            formulaAngle, VAbstractApplication::VApp()->Settings()->GetOsSeparator()));
        visual->SetLineStyle(LineStyleToPenStyle(m_lineType));
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCurveIntersectAxis::MakeToolTip() const -> QString
{
    const QSharedPointer<VPointF> first = VAbstractTool::data.GeometricObject<VPointF>(basePointId);
    const QSharedPointer<VPointF> second = VAbstractTool::data.GeometricObject<VPointF>(m_id);

    const QLineF line(static_cast<QPointF>(*first), static_cast<QPointF>(*second));

    QSharedPointer<VGObject> const segment1 = VAbstractTool::data.GetGObject(m_segment1Id);
    QSharedPointer<VGObject> const segment2 = VAbstractTool::data.GetGObject(m_segment2Id);

    const QString toolTip = u"<table>"
                            u"<tr> <td><b>%6:</b> %7</td> </tr>"
                            u"<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                            u"<tr> <td><b>%4:</b> %5°</td> </tr>"
                            u"<tr> <td><b>%8:</b> %9</td> </tr>"
                            u"<tr> <td><b>%10:</b> %11</td> </tr>"
                            u"</table>"_s
                                .arg(tr("Length"))                                              // 1
                                .arg(VAbstractValApplication::VApp()->fromPixel(line.length())) // 2
                                .arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true),
                                     tr("Angle"))  // 3, 4
                                .arg(line.angle()) // 5
                                .arg(tr("Label"),
                                     second->name(), /* 6, 7 */
                                     tr("Segment 1"),
                                     segment1->ObjectName(), /* 8, 9 */
                                     tr("Segment 2"),
                                     segment2->ObjectName()); /* 10, 11 */
    return toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    VToolLinePoint::hoverEnterEvent(event);
    mainLine->SetBoldLine(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    ProcessToolOptions(oldDomElement, newDomElement, GatherToolChanges());
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCurveIntersectAxis::itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant
{
    if (change == QGraphicsItem::ItemSelectedHasChanged && !m_selectedFromChild)
    {
        if (m_segment1Id != NULL_ID)
        {
            const QSignalBlocker blockerSegment1Label(m_segment1Label);
            m_segment1Label->SetLabelVisible(value.toBool());
            m_segment1Label->setSelected(value.toBool());
        }

        if (m_segment1Id != NULL_ID)
        {
            const QSignalBlocker blockeSegment2Label(m_segment2Label);
            m_segment2Label->setSelected(value.toBool());
            m_segment2Label->SetLabelVisible(value.toBool());
        }
    }

    return VToolLinePoint::itemChange(change, value);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::RefreshGeometry()
{
    VToolLinePoint::RefreshGeometry();

    auto const UpdateSegmentLabel = [this](quint32 segmentId, qreal mx, qreal my, VSegmentLabel *label)
    {
        QSharedPointer<VAbstractCurve> const curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(segmentId);

        VPointF pos = curve->GetMidpoint();
        pos.setMx(mx);
        pos.setMy(my);
        pos.setX(pos.x() - this->pos().x());
        pos.setY(pos.y() - this->pos().y());

        label->SetLabelData(pos);
        label->SetSegmentShape(curve);
    };

    UpdateSegmentLabel(m_segment1Id, m_segment1Mx, m_segment1My, m_segment1Label);
    UpdateSegmentLabel(m_segment2Id, m_segment2Mx, m_segment2My, m_segment2Label);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SegmentChoosed(quint32 id, SceneObject type)
{
    emit ChoosedTool(id, type);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::Segment1LabelPositionChanged(const QPointF &pos)
{
    VAbstractApplication::VApp()->getUndoStack()->push(new MoveSegmentLabel(doc, pos, m_id, SegmentLabel::Segment1));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::Segment2LabelPositionChanged(const QPointF &pos)
{
    VAbstractApplication::VApp()->getUndoStack()->push(new MoveSegmentLabel(doc, pos, m_id, SegmentLabel::Segment2));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::Enable()
{
    const bool enabled = m_indexActivePatternBlock == doc->PatternBlockMapper()->GetActiveId();
    m_segment1Label->SetEnabledState(enabled);
    m_segment2Label->SetEnabledState(enabled);

    VToolLinePoint::Enable();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::EnableToolMove(bool move)
{
    m_segment1Label->SetLabelMovable(move);
    m_segment2Label->SetLabelMovable(move);

    VToolLinePoint::EnableToolMove(move);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::AllowSegmentHover(bool enabled)
{
    m_segment1Label->AllowLabelHover(enabled);
    m_segment2Label->AllowLabelHover(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::ToolSelectionType(const SelectionType &selectionType)
{
    m_segment1Label->LabelSelectionType(selectionType);
    m_segment2Label->LabelSelectionType(selectionType);

    VToolLinePoint::ToolSelectionType(selectionType);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SetSegmentLabelVisible(bool visible)
{
    m_segment1Label->SetLabelVisible(visible);
    m_segment2Label->SetLabelVisible(visible);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::AllowLabelSelecting(bool enabled)
{
    m_segment1Label->SetLabelSelectable(enabled);
    m_segment2Label->SetLabelSelectable(enabled);

    VToolLinePoint::AllowLabelSelecting(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SetDetailsMode(bool mode)
{
    m_segment1Label->ShowExplicitly(mode);
    m_segment2Label->ShowExplicitly(mode);
}
