/************************************************************************
 **
 **  @file   vtoolpointofintersectionarcs.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 5, 2015
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

#include "vtoolpointofintersectionarcs.h"

#include <QLineF>
#include <QSharedPointer>
#include <QUndoCommand>

#include "../../../../dialogs/tools/dialogpointofintersectionarcs.h"
#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../undocommands/label/movesegmentlabel.h"
#include "../../../../undocommands/renameobject.h"
#include "../../../../undocommands/savetooloptions.h"
#include "../../../../visualization/line/vistoolpointofintersectionarcs.h"
#include "../../../../visualization/visualization.h"
#include "../../../vabstracttool.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/exception/vexception.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vsegmentlabel.h"
#include "vtoolsinglepoint.h"

template <class T> class QSharedPointer;

const QString VToolPointOfIntersectionArcs::ToolType = QStringLiteral("pointOfIntersectionArcs");

//---------------------------------------------------------------------------------------------------------------------
VToolPointOfIntersectionArcs::VToolPointOfIntersectionArcs(const VToolPointOfIntersectionArcsInitData &initData,
                                                           QGraphicsItem *parent)
  : VToolSinglePoint(initData.doc, initData.data, initData.id, initData.notes, parent),
    firstArcId(initData.firstArcId),
    secondArcId(initData.secondArcId),
    crossPoint(initData.pType),
    m_arc1Name1(initData.arc1Name1),
    m_arc1Name2(initData.arc1Name2),
    m_arc2Name1(initData.arc2Name1),
    m_arc2Name2(initData.arc2Name2),
    m_arc1AliasSuffix1(initData.arc1AliasSuffix1),
    m_arc1AliasSuffix2(initData.arc1AliasSuffix2),
    m_arc2AliasSuffix1(initData.arc2AliasSuffix1),
    m_arc2AliasSuffix2(initData.arc2AliasSuffix2),
    m_arc1Segment1Id(initData.arc1Segment1Id),
    m_arc1Segment2Id(initData.arc1Segment2Id),
    m_arc2Segment1Id(initData.arc2Segment1Id),
    m_arc2Segment2Id(initData.arc2Segment2Id),
    m_arc1Segment1Mx(initData.arc1Segment1Mx),
    m_arc1Segment1My(initData.arc1Segment1My),
    m_arc1Segment2Mx(initData.arc1Segment2Mx),
    m_arc1Segment2My(initData.arc1Segment2My),
    m_arc2Segment1Mx(initData.arc2Segment1Mx),
    m_arc2Segment1My(initData.arc2Segment1My),
    m_arc2Segment2Mx(initData.arc2Segment2Mx),
    m_arc2Segment2My(initData.arc2Segment2My)
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

    m_arc1Segment1Label = CreateSegmentLabel(m_arc1Segment1Id,
                                             m_arc1Segment1Mx,
                                             m_arc1Segment1My,
                                             &VToolPointOfIntersectionArcs::SegmentChoosed,
                                             &VToolPointOfIntersectionArcs::PointSelected,
                                             &VToolPointOfIntersectionArcs::Arc1Segment1LabelPositionChanged);

    m_arc1Segment2Label = CreateSegmentLabel(m_arc1Segment2Id,
                                             m_arc1Segment2Mx,
                                             m_arc1Segment2My,
                                             &VToolPointOfIntersectionArcs::SegmentChoosed,
                                             &VToolPointOfIntersectionArcs::PointSelected,
                                             &VToolPointOfIntersectionArcs::Arc1Segment2LabelPositionChanged);

    m_arc2Segment1Label = CreateSegmentLabel(m_arc2Segment1Id,
                                             m_arc2Segment1Mx,
                                             m_arc2Segment1My,
                                             &VToolPointOfIntersectionArcs::SegmentChoosed,
                                             &VToolPointOfIntersectionArcs::PointSelected,
                                             &VToolPointOfIntersectionArcs::Arc2Segment1LabelPositionChanged);

    m_arc2Segment2Label = CreateSegmentLabel(m_arc2Segment2Id,
                                             m_arc2Segment2Mx,
                                             m_arc2Segment2My,
                                             &VToolPointOfIntersectionArcs::SegmentChoosed,
                                             &VToolPointOfIntersectionArcs::PointSelected,
                                             &VToolPointOfIntersectionArcs::Arc2Segment2LabelPositionChanged);

    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::ProcessToolOptions(const QDomElement &oldDomElement,
                                                      const QDomElement &newDomElement,
                                                      const ToolChanges &changes)
{
    if (!changes.HasChanges())
    {
        VToolSinglePoint::ApplyToolOptions(oldDomElement, newDomElement);
        return;
    }

    QUndoStack *undoStack = VAbstractApplication::VApp()->getUndoStack();
    auto *newGroup = new QUndoCommand(); // an empty command
    newGroup->setText(tr("save tool options"));

    auto *saveOptions = new SaveToolOptions(oldDomElement, newDomElement, doc, m_id, newGroup);
    saveOptions->SetInGroup(true);
    connect(saveOptions, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);

    const QSharedPointer<VAbstractCurve> curve1 = VAbstractTool::data.GeometricObject<VAbstractCurve>(firstArcId);
    const CurveAliasType curve1Type = RenameAlias::CurveType(curve1->getType());

    const QSharedPointer<VAbstractCurve> curve2 = VAbstractTool::data.GeometricObject<VAbstractCurve>(secondArcId);
    const CurveAliasType curve2Type = RenameAlias::CurveType(curve1->getType());

    // Collect rename commands in order; only the last one triggers a reparse.
    QVector<const AbstractObjectRename *> renameCommands;
    renameCommands.resize(9);

    if (changes.LabelChanged())
    {
        renameCommands << new RenameLabel(changes.oldLabel, changes.newLabel, doc, m_id, newGroup);
    }

    if (changes.Arc1Name1Changed())
    {
        renameCommands << new RenameAlias(curve1Type,
                                          changes.oldArc1Name1,
                                          changes.newArc1Name1,
                                          doc,
                                          m_arc1Segment1Id,
                                          newGroup);
    }

    if (changes.Arc1Name2Changed())
    {
        renameCommands << new RenameAlias(curve1Type,
                                          changes.oldArc1Name2,
                                          changes.newArc1Name2,
                                          doc,
                                          m_arc1Segment2Id,
                                          newGroup);
    }

    if (changes.Arc1AliasSuffix1Changed())
    {
        renameCommands << new RenameAlias(curve1Type,
                                          changes.oldArc1AliasSuffix1,
                                          changes.newArc1AliasSuffix1,
                                          doc,
                                          m_arc1Segment1Id,
                                          newGroup);
    }

    if (changes.Arc1AliasSuffix2Changed())
    {
        renameCommands << new RenameAlias(curve1Type,
                                          changes.oldArc1AliasSuffix1,
                                          changes.newArc1AliasSuffix1,
                                          doc,
                                          m_arc1Segment2Id,
                                          newGroup);
    }

    if (changes.Arc2Name1Changed())
    {
        renameCommands << new RenameAlias(curve2Type,
                                          changes.oldArc2Name1,
                                          changes.newArc2Name1,
                                          doc,
                                          m_arc2Segment1Id,
                                          newGroup);
    }

    if (changes.Arc2Name2Changed())
    {
        renameCommands << new RenameAlias(curve2Type,
                                          changes.oldArc2Name2,
                                          changes.newArc2Name2,
                                          doc,
                                          m_arc2Segment2Id,
                                          newGroup);
    }

    if (changes.Arc2AliasSuffix1Changed())
    {
        renameCommands << new RenameAlias(curve2Type,
                                          changes.oldArc2AliasSuffix1,
                                          changes.newArc2AliasSuffix1,
                                          doc,
                                          m_arc2Segment1Id,
                                          newGroup);
    }

    if (changes.Arc2AliasSuffix2Changed())
    {
        renameCommands << new RenameAlias(curve2Type,
                                          changes.oldArc2AliasSuffix1,
                                          changes.newArc2AliasSuffix1,
                                          doc,
                                          m_arc2Segment2Id,
                                          newGroup);
    }

    if (!renameCommands.isEmpty())
    {
        connect(renameCommands.constLast(),
                &AbstractObjectRename::NeedLiteParsing,
                doc,
                &VAbstractPattern::LiteParseTree);
    }

    undoStack->push(newGroup);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionArcs::GetFieldMetadata(VToolPointOfIntersectionArcsNameField field)
    -> VToolPointOfIntersectionArcsFieldMetadata
{
    switch (field)
    {
        case VToolPointOfIntersectionArcsNameField::Arc1Name1:
            return {.curveId = firstArcId, .isName = true, .memberPtr = &m_arc1Name1};
        case VToolPointOfIntersectionArcsNameField::Arc1Name2:
            return {.curveId = firstArcId, .isName = true, .memberPtr = &m_arc1Name2};
        case VToolPointOfIntersectionArcsNameField::Arc2Name1:
            return {.curveId = secondArcId, .isName = true, .memberPtr = &m_arc2Name1};
        case VToolPointOfIntersectionArcsNameField::Arc2Name2:
            return {.curveId = secondArcId, .isName = true, .memberPtr = &m_arc2Name2};
        case VToolPointOfIntersectionArcsNameField::Arc1AliasSuffix1:
            return {.curveId = firstArcId, .isName = false, .memberPtr = &m_arc1AliasSuffix1};
        case VToolPointOfIntersectionArcsNameField::Arc1AliasSuffix2:
            return {.curveId = firstArcId, .isName = false, .memberPtr = &m_arc1AliasSuffix2};
        case VToolPointOfIntersectionArcsNameField::Arc2AliasSuffix1:
            return {.curveId = secondArcId, .isName = false, .memberPtr = &m_arc2AliasSuffix1};
        case VToolPointOfIntersectionArcsNameField::Arc2AliasSuffix2:
            return {.curveId = secondArcId, .isName = false, .memberPtr = &m_arc2AliasSuffix2};
        default:
            break;
    }

    Q_UNREACHABLE();
    return {}; // Should never reach
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionArcs::HasConflict(const QString &value,
                                               VToolPointOfIntersectionArcsNameField currentField) const -> bool
{
    if (value.isEmpty())
    {
        return false;
    }

    // Check all name fields
    if (currentField != VToolPointOfIntersectionArcsNameField::Arc1Name1 && value == m_arc1Name1)
    {
        return true;
    }
    if (currentField != VToolPointOfIntersectionArcsNameField::Arc1Name2 && value == m_arc1Name2)
    {
        return true;
    }
    if (currentField != VToolPointOfIntersectionArcsNameField::Arc2Name1 && value == m_arc2Name1)
    {
        return true;
    }
    if (currentField != VToolPointOfIntersectionArcsNameField::Arc2Name2 && value == m_arc2Name2)
    {
        return true;
    }

    // Check non-empty alias fields
    if (currentField != VToolPointOfIntersectionArcsNameField::Arc1AliasSuffix1 && !m_arc1AliasSuffix1.isEmpty()
        && value == m_arc1AliasSuffix1)
    {
        return true;
    }
    if (currentField != VToolPointOfIntersectionArcsNameField::Arc1AliasSuffix2 && !m_arc1AliasSuffix2.isEmpty()
        && value == m_arc1AliasSuffix2)
    {
        return true;
    }
    if (currentField != VToolPointOfIntersectionArcsNameField::Arc2AliasSuffix1 && !m_arc2AliasSuffix1.isEmpty()
        && value == m_arc2AliasSuffix1)
    {
        return true;
    }
    if (currentField != VToolPointOfIntersectionArcsNameField::Arc2AliasSuffix2 && !m_arc2AliasSuffix2.isEmpty()
        && value == m_arc2AliasSuffix2)
    {
        return true;
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::UpdateNameField(VToolPointOfIntersectionArcsNameField field, const QString &value)
{
    VToolPointOfIntersectionArcsFieldMetadata const metadata = GetFieldMetadata(field);

    // Validation - name fields require non-empty values
    if (metadata.isName && value.isEmpty())
    {
        return; // Name is required
    }

    // Validate format and uniqueness for non-empty values
    if (!value.isEmpty())
    {
        QSharedPointer<VAbstractCurve> const curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(
            metadata.curveId);
        const QString fullName = curve->GetTypeHead() + value;

        if (QRegularExpression const rx(NameRegExp()); !rx.match(fullName).hasMatch())
        {
            return; // Invalid format
        }

        if (!VAbstractTool::data.IsUnique(fullName))
        {
            return; // Not unique in data
        }

        if (HasConflict(value, field))
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
    ToolChanges const changes = {
        .oldLabel = label,
        .newLabel = label,
        .oldArc1Name1 = m_arc1Name1,
        .newArc1Name1 = (field == VToolPointOfIntersectionArcsNameField::Arc1Name1) ? value : m_arc1Name1,
        .oldArc1Name2 = m_arc1Name2,
        .newArc1Name2 = (field == VToolPointOfIntersectionArcsNameField::Arc1Name2) ? value : m_arc1Name2,
        .oldArc2Name1 = m_arc2Name1,
        .newArc2Name1 = (field == VToolPointOfIntersectionArcsNameField::Arc2Name1) ? value : m_arc2Name1,
        .oldArc2Name2 = m_arc2Name2,
        .newArc2Name2 = (field == VToolPointOfIntersectionArcsNameField::Arc2Name2) ? value : m_arc2Name2,
        .oldArc1AliasSuffix1 = m_arc1AliasSuffix1,
        .newArc1AliasSuffix1 = (field == VToolPointOfIntersectionArcsNameField::Arc1AliasSuffix1) ? value
                                                                                                  : m_arc1AliasSuffix1,
        .oldArc1AliasSuffix2 = m_arc1AliasSuffix2,
        .newArc1AliasSuffix2 = (field == VToolPointOfIntersectionArcsNameField::Arc1AliasSuffix2) ? value
                                                                                                  : m_arc1AliasSuffix2,
        .oldArc2AliasSuffix1 = m_arc2AliasSuffix1,
        .newArc2AliasSuffix1 = (field == VToolPointOfIntersectionArcsNameField::Arc2AliasSuffix1) ? value
                                                                                                  : m_arc2AliasSuffix1,
        .oldArc2AliasSuffix2 = m_arc2AliasSuffix2,
        .newArc2AliasSuffix2 = (field == VToolPointOfIntersectionArcsNameField::Arc2AliasSuffix2) ? value
                                                                                                  : m_arc2AliasSuffix2};

    // Update the appropriate member variable
    *(metadata.memberPtr) = value;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    QDomElement newDomElement = oldDomElement.cloneNode().toElement();
    SaveOptions(newDomElement, obj);
    ProcessToolOptions(oldDomElement, newDomElement, changes);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionArcs::GatherToolChanges() const -> VToolPointOfIntersectionArcs::ToolChanges
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogPointOfIntersectionArcs> dialogTool = qobject_cast<DialogPointOfIntersectionArcs *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    return {.pointId = m_id,
            .oldLabel = name(),
            .newLabel = dialogTool->GetPointName(),
            .oldArc1Name1 = m_arc1Name1,
            .newArc1Name1 = dialogTool->GetArc1Name1(),
            .oldArc1Name2 = m_arc1Name2,
            .newArc1Name2 = dialogTool->GetArc1Name2(),
            .oldArc2Name1 = m_arc2Name1,
            .newArc2Name1 = dialogTool->GetArc2Name1(),
            .oldArc2Name2 = m_arc2Name2,
            .newArc2Name2 = dialogTool->GetArc2Name2(),
            .oldArc1AliasSuffix1 = m_arc1AliasSuffix1,
            .newArc1AliasSuffix1 = dialogTool->GetArc1AliasSuffix1(),
            .oldArc1AliasSuffix2 = m_arc1AliasSuffix2,
            .newArc1AliasSuffix2 = dialogTool->GetArc1AliasSuffix2(),
            .oldArc2AliasSuffix1 = m_arc2AliasSuffix1,
            .newArc2AliasSuffix1 = dialogTool->GetArc2AliasSuffix1(),
            .oldArc2AliasSuffix2 = m_arc2AliasSuffix2,
            .newArc2AliasSuffix2 = dialogTool->GetArc2AliasSuffix2()};
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogPointOfIntersectionArcs> dialogTool = qobject_cast<DialogPointOfIntersectionArcs *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetFirstArcId(firstArcId);
    dialogTool->SetSecondArcId(secondArcId);
    dialogTool->SetCrossArcPoint(crossPoint);
    dialogTool->SetPointName(p->name());
    dialogTool->SetNotes(m_notes);
    dialogTool->SetArc1Name1(m_arc1Name1);
    dialogTool->SetArc1Name2(m_arc1Name2);
    dialogTool->SetArc2Name1(m_arc2Name1);
    dialogTool->SetArc2Name2(m_arc2Name2);
    dialogTool->SetArc1AliasSuffix1(m_arc1AliasSuffix1);
    dialogTool->SetArc1AliasSuffix2(m_arc1AliasSuffix2);
    dialogTool->SetArc2AliasSuffix1(m_arc2AliasSuffix1);
    dialogTool->SetArc2AliasSuffix2(m_arc2AliasSuffix2);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionArcs::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene,
                                          VAbstractPattern *doc, VContainer *data) -> VToolPointOfIntersectionArcs *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogPointOfIntersectionArcs> dialogTool = qobject_cast<DialogPointOfIntersectionArcs *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolPointOfIntersectionArcsInitData initData;
    initData.firstArcId = dialogTool->GetFirstArcId();
    initData.secondArcId = dialogTool->GetSecondArcId();
    initData.pType = dialogTool->GetCrossArcPoint();
    initData.name = dialogTool->GetPointName();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();
    initData.arc1Name1 = dialogTool->GetArc1Name1();
    initData.arc1Name2 = dialogTool->GetArc1Name2();
    initData.arc2Name1 = dialogTool->GetArc2Name1();
    initData.arc2Name2 = dialogTool->GetArc2Name2();
    initData.arc1AliasSuffix1 = dialogTool->GetArc1AliasSuffix1();
    initData.arc1AliasSuffix2 = dialogTool->GetArc1AliasSuffix2();
    initData.arc2AliasSuffix1 = dialogTool->GetArc2AliasSuffix1();
    initData.arc2AliasSuffix2 = dialogTool->GetArc2AliasSuffix2();

    VToolPointOfIntersectionArcs *point = Create(initData);
    if (point != nullptr)
    {
        point->m_dialog = dialog;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionArcs::Create(VToolPointOfIntersectionArcsInitData initData)
    -> VToolPointOfIntersectionArcs *
{
    const QSharedPointer<VArc> firstArc = initData.data->GeometricObject<VArc>(initData.firstArcId);
    const QSharedPointer<VArc> secondArc = initData.data->GeometricObject<VArc>(initData.secondArcId);

    QPointF point;
    if (!FindPoint(firstArc.data(), secondArc.data(), initData.pType, &point))
    {
        const QString errorMsg = tr("Error calculating point '%1'. Arcs '%2' and '%3' have no point of intersection")
                                     .arg(initData.name, firstArc->ObjectName(), secondArc->ObjectName());
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

    SegmentDetails curve1Details{.typeCreation = initData.typeCreation,
                                 .curveType = firstArc->getType(),
                                 .segLength = firstArc->GetLengthByPoint(point),
                                 .p = *p,
                                 .curveId = initData.firstArcId,
                                 .data = initData.data,
                                 .doc = initData.doc,
                                 .name1 = initData.arc1Name1,
                                 .name2 = initData.arc1Name2,
                                 .alias1 = initData.arc1AliasSuffix1,
                                 .alias2 = initData.arc1AliasSuffix2,
                                 .id = initData.id,
                                 .name1AttrName = AttrCurve1Name1,
                                 .name2AttrName = AttrCurve1Name2,
                                 .segment1Id = initData.arc1Segment1Id,
                                 .segment2Id = initData.arc1Segment2Id};
    VToolSinglePoint::InitSegments(curve1Details);

    initData.arc1Name1 = curve1Details.name1;
    initData.arc1Name2 = curve1Details.name2;

    SegmentDetails curve2Details{.typeCreation = initData.typeCreation,
                                 .curveType = secondArc->getType(),
                                 .segLength = secondArc->GetLengthByPoint(point),
                                 .p = *p,
                                 .curveId = initData.secondArcId,
                                 .data = initData.data,
                                 .doc = initData.doc,
                                 .name1 = initData.arc2Name1,
                                 .name2 = initData.arc2Name2,
                                 .alias1 = initData.arc2AliasSuffix1,
                                 .alias2 = initData.arc2AliasSuffix2,
                                 .id = initData.id,
                                 .name1AttrName = AttrCurve2Name1,
                                 .name2AttrName = AttrCurve2Name2,
                                 .segment1Id = initData.arc2Segment1Id,
                                 .segment2Id = initData.arc2Segment2Id};
    VToolSinglePoint::InitSegments(curve2Details);

    initData.arc2Name1 = curve2Details.name1;
    initData.arc2Name2 = curve2Details.name2;

    patternGraph->AddEdge(initData.firstArcId, initData.id);
    patternGraph->AddEdge(initData.secondArcId, initData.id);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.arc1Segment1Id = curve1Details.segment1Id;
        initData.arc1Segment2Id = curve1Details.segment2Id;
        initData.arc2Segment1Id = curve2Details.segment1Id;
        initData.arc2Segment2Id = curve2Details.segment2Id;
    }

    patternGraph->AddVertex(initData.arc1Segment1Id,
                            VNodeType::OBJECT,
                            initData.doc->PatternBlockMapper()->GetActiveId());
    patternGraph->AddVertex(initData.arc1Segment2Id,
                            VNodeType::OBJECT,
                            initData.doc->PatternBlockMapper()->GetActiveId());
    patternGraph->AddVertex(initData.arc2Segment1Id,
                            VNodeType::OBJECT,
                            initData.doc->PatternBlockMapper()->GetActiveId());
    patternGraph->AddVertex(initData.arc2Segment2Id,
                            VNodeType::OBJECT,
                            initData.doc->PatternBlockMapper()->GetActiveId());

    patternGraph->AddEdge(initData.id, initData.arc1Segment1Id);
    patternGraph->AddEdge(initData.id, initData.arc1Segment2Id);
    patternGraph->AddEdge(initData.id, initData.arc2Segment1Id);
    patternGraph->AddEdge(initData.id, initData.arc2Segment2Id);

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::PointOfIntersectionArcs, initData.doc);
        auto *point = new VToolPointOfIntersectionArcs(initData);
        initData.scene->addItem(point);
        InitPointToolConnections(initData.scene, point);
        InitSegmentConnections(initData.scene, point, GOType::Arc);
        VAbstractPattern::AddTool(initData.id, point);
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionArcs::FindPoint(const VArc *arc1,
                                             const VArc *arc2,
                                             CrossCirclesPoint pType,
                                             QPointF *intersectionPoint) -> bool
{
    SCASSERT(intersectionPoint != nullptr)

    QPointF p1, p2;
    const auto centerArc1 = static_cast<QPointF>(arc1->GetCenter());
    const auto centerArc2 = static_cast<QPointF>(arc2->GetCenter());
    const int res = VGObject::IntersectionCircles(centerArc1, arc1->GetRadius(), centerArc2, arc2->GetRadius(), p1, p2);

    QLineF r1Arc1(centerArc1, p1);
    r1Arc1.setLength(r1Arc1.length() + 10);

    QLineF r1Arc2(centerArc2, p1);
    r1Arc2.setLength(r1Arc2.length() + 10);

    QLineF r2Arc1(centerArc1, p2);
    r2Arc1.setLength(r2Arc1.length() + 10);

    QLineF r2Arc2(centerArc2, p2);
    r2Arc2.setLength(r2Arc2.length() + 10);

    switch (res)
    {
        case 2:
        {
            int localRes = 0;
            bool flagP1 = false;

            if (arc1->IsIntersectLine(r1Arc1) && arc2->IsIntersectLine(r1Arc2))
            {
                ++localRes;
                flagP1 = true;
            }

            if (arc1->IsIntersectLine(r2Arc1) && arc2->IsIntersectLine(r2Arc2))
            {
                ++localRes;
            }

            switch (localRes)
            {
                case 2:
                    if (pType == CrossCirclesPoint::FirstPoint)
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
                    if (flagP1)
                    {
                        *intersectionPoint = p1;
                        return true;
                    }
                    else
                    {
                        *intersectionPoint = p2;
                        return true;
                    }
                case 0:
                default:
                    return false;
            }

            break;
        }
        case 1:
            if (arc1->IsIntersectLine(r1Arc1) && arc2->IsIntersectLine(r1Arc2))
            {
                *intersectionPoint = p1;
                return true;
            }
            else
            {
                return false;
            }
        case 3:
        case 0:
        default:
            break;
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionArcs::FirstArcName() const -> QString
{
    return VAbstractTool::data.GetGObject(firstArcId)->ObjectName();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionArcs::SecondArcName() const -> QString
{
    return VAbstractTool::data.GetGObject(secondArcId)->ObjectName();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionArcs::GetArc1Name1() const -> QString
{
    return m_arc1Name1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SetArc1Name1(const QString &name)
{
    UpdateNameField(VToolPointOfIntersectionArcsNameField::Arc1Name1, name);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionArcs::GetArc1Name2() const -> QString
{
    return m_arc1Name2;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SetArc1Name2(const QString &name)
{
    UpdateNameField(VToolPointOfIntersectionArcsNameField::Arc1Name2, name);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionArcs::GetArc2Name1() const -> QString
{
    return m_arc2Name1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SetArc2Name1(const QString &name)
{
    UpdateNameField(VToolPointOfIntersectionArcsNameField::Arc2Name1, name);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionArcs::GetArc2Name2() const -> QString
{
    return m_arc2Name2;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SetArc2Name2(const QString &name)
{
    UpdateNameField(VToolPointOfIntersectionArcsNameField::Arc2Name2, name);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionArcs::GetArc1AliasSuffix1() const -> QString
{
    return m_arc1AliasSuffix1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SetArc1AliasSuffix1(const QString &alias)
{
    UpdateNameField(VToolPointOfIntersectionArcsNameField::Arc1AliasSuffix1, alias);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionArcs::GetArc1AliasSuffix2() const -> QString
{
    return m_arc1AliasSuffix2;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SetArc1AliasSuffix2(const QString &alias)
{
    UpdateNameField(VToolPointOfIntersectionArcsNameField::Arc1AliasSuffix2, alias);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionArcs::GetArc2AliasSuffix1() const -> QString
{
    return m_arc2AliasSuffix1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SetArc2AliasSuffix1(const QString &alias)
{
    UpdateNameField(VToolPointOfIntersectionArcsNameField::Arc2AliasSuffix1, alias);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionArcs::GetArc2AliasSuffix2() const -> QString
{
    return m_arc2AliasSuffix2;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SetArc2AliasSuffix2(const QString &alias)
{
    UpdateNameField(VToolPointOfIntersectionArcsNameField::Arc2AliasSuffix2, alias);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionArcs::GetCrossCirclesPoint() const -> CrossCirclesPoint
{
    return crossPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SetCrossCirclesPoint(const CrossCirclesPoint &value)
{
    crossPoint = value;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolPointOfIntersectionArcs>(show);
    m_arc1Segment1Label->SetLabelVisible(show);
    m_arc1Segment2Label->SetLabelVisible(show);
    m_arc2Segment1Label->SetLabelVisible(show);
    m_arc2Segment2Label->SetLabelVisible(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::ChangeSegmentLabelPosition(SegmentLabel segment, const QPointF &pos)
{
    switch (segment)
    {
        case SegmentLabel::Segment1:
            m_arc1Segment1Label->SetLabelPosition(pos);
            break;
        case SegmentLabel::Segment2:
            m_arc1Segment2Label->SetLabelPosition(pos);
            break;
        case SegmentLabel::Segment3:
            m_arc2Segment1Label->SetLabelPosition(pos);
            break;
        case SegmentLabel::Segment4:
            m_arc2Segment2Label->SetLabelPosition(pos);
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::Enable()
{
    const bool enabled = m_indexActivePatternBlock == doc->PatternBlockMapper()->GetActiveId();
    m_arc1Segment1Label->SetEnabledState(enabled);
    m_arc1Segment2Label->SetEnabledState(enabled);
    m_arc2Segment1Label->SetEnabledState(enabled);
    m_arc2Segment2Label->SetEnabledState(enabled);

    VToolSinglePoint::Enable();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::EnableToolMove(bool move)
{
    m_arc1Segment1Label->SetLabelMovable(move);
    m_arc1Segment2Label->SetLabelMovable(move);
    m_arc2Segment1Label->SetLabelMovable(move);
    m_arc2Segment2Label->SetLabelMovable(move);

    VToolSinglePoint::EnableToolMove(move);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::AllowSegmentHover(bool enabled)
{
    m_arc1Segment1Label->AllowLabelHover(enabled);
    m_arc1Segment2Label->AllowLabelHover(enabled);
    m_arc2Segment1Label->AllowLabelHover(enabled);
    m_arc2Segment2Label->AllowLabelHover(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::ToolSelectionType(const SelectionType &selectionType)
{
    m_arc1Segment1Label->LabelSelectionType(selectionType);
    m_arc1Segment2Label->LabelSelectionType(selectionType);
    m_arc2Segment1Label->LabelSelectionType(selectionType);
    m_arc2Segment2Label->LabelSelectionType(selectionType);

    VToolSinglePoint::ToolSelectionType(selectionType);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SetSegmentLabelVisible(bool visible)
{
    m_arc1Segment1Label->SetLabelVisible(visible);
    m_arc1Segment2Label->SetLabelVisible(visible);
    m_arc2Segment1Label->SetLabelVisible(visible);
    m_arc2Segment2Label->SetLabelVisible(visible);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::AllowLabelSelecting(bool enabled)
{
    m_arc1Segment1Label->SetLabelSelectable(enabled);
    m_arc1Segment2Label->SetLabelSelectable(enabled);
    m_arc2Segment1Label->SetLabelSelectable(enabled);
    m_arc2Segment2Label->SetLabelSelectable(enabled);

    VToolSinglePoint::AllowLabelSelecting(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogPointOfIntersectionArcs>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogPointOfIntersectionArcs> dialogTool = qobject_cast<DialogPointOfIntersectionArcs *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrName, dialogTool->GetPointName());
    doc->SetAttribute(domElement, AttrFirstArc, QString().setNum(dialogTool->GetFirstArcId()));
    doc->SetAttribute(domElement, AttrSecondArc, QString().setNum(dialogTool->GetSecondArcId()));
    doc->SetAttribute(domElement, AttrCrossPoint, QString().setNum(static_cast<int>(dialogTool->GetCrossArcPoint())));
    doc->SetAttribute(domElement, AttrCurve1Name1, dialogTool->GetArc1Name1());
    doc->SetAttribute(domElement, AttrCurve1Name2, dialogTool->GetArc1Name2());
    doc->SetAttribute(domElement, AttrCurve2Name1, dialogTool->GetArc2Name1());
    doc->SetAttribute(domElement, AttrCurve2Name2, dialogTool->GetArc2Name2());
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrCurve1Alias1,
                                         dialogTool->GetArc1AliasSuffix1(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrCurve1Alias2,
                                         dialogTool->GetArc1AliasSuffix2(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrCurve2Alias1,
                                         dialogTool->GetArc2AliasSuffix1(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrCurve2Alias2,
                                         dialogTool->GetArc2AliasSuffix2(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrNotes,
                                         dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrFirstArc, firstArcId);
    doc->SetAttribute(tag, AttrSecondArc, secondArcId);
    doc->SetAttribute(tag, AttrCrossPoint, static_cast<int>(crossPoint));
    doc->SetAttribute(tag, AttrCurve1Name1, m_arc1Name1);
    doc->SetAttribute(tag, AttrCurve1Name2, m_arc1Name2);
    doc->SetAttribute(tag, AttrCurve2Name1, m_arc2Name1);
    doc->SetAttribute(tag, AttrCurve2Name2, m_arc2Name2);
    doc->SetAttributeOrRemoveIf<QString>(tag,
                                         AttrCurve1Alias1,
                                         m_arc1AliasSuffix1,
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(tag,
                                         AttrCurve1Alias2,
                                         m_arc1AliasSuffix2,
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(tag,
                                         AttrCurve2Alias1,
                                         m_arc2AliasSuffix1,
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(tag,
                                         AttrCurve2Alias2,
                                         m_arc2AliasSuffix2,
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttribute(tag, AttrSegment1Id, m_arc1Segment1Id);
    doc->SetAttribute(tag, AttrSegment2Id, m_arc1Segment2Id);
    doc->SetAttribute(tag, AttrSegment3Id, m_arc2Segment1Id);
    doc->SetAttribute(tag, AttrSegment4Id, m_arc2Segment2Id);

    const VAbstractValApplication *app = VAbstractValApplication::VApp();
    doc->SetAttribute(tag, AttrSegment1Mx, app->fromPixel(m_arc1Segment1Mx));
    doc->SetAttribute(tag, AttrSegment1My, app->fromPixel(m_arc1Segment1My));
    doc->SetAttribute(tag, AttrSegment2Mx, app->fromPixel(m_arc1Segment2Mx));
    doc->SetAttribute(tag, AttrSegment2My, app->fromPixel(m_arc1Segment2Mx));
    doc->SetAttribute(tag, AttrSegment3Mx, app->fromPixel(m_arc2Segment1Mx));
    doc->SetAttribute(tag, AttrSegment3My, app->fromPixel(m_arc2Segment1My));
    doc->SetAttribute(tag, AttrSegment4Mx, app->fromPixel(m_arc2Segment2Mx));
    doc->SetAttribute(tag, AttrSegment4My, app->fromPixel(m_arc2Segment2My));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::ReadToolAttributes(const QDomElement &domElement)
{
    VToolSinglePoint::ReadToolAttributes(domElement);

    firstArcId = VAbstractPattern::GetParametrUInt(domElement, AttrFirstArc, NULL_ID_STR);
    secondArcId = VAbstractPattern::GetParametrUInt(domElement, AttrSecondArc, NULL_ID_STR);
    crossPoint = static_cast<CrossCirclesPoint>(
        VAbstractPattern::GetParametrUInt(domElement, AttrCrossPoint, QChar('1')));
    m_arc1Name1 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurve1Name1);
    m_arc1Name2 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurve1Name2);
    m_arc2Name1 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurve2Name1);
    m_arc2Name2 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurve2Name2);
    m_arc1AliasSuffix1 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurve1Alias1);
    m_arc1AliasSuffix2 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurve1Alias2);
    m_arc2AliasSuffix1 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurve2Alias1);
    m_arc2AliasSuffix2 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurve2Alias2);
    m_arc1Segment1Id = VAbstractPattern::GetParametrId(domElement, AttrSegment1Id);
    m_arc1Segment2Id = VAbstractPattern::GetParametrId(domElement, AttrSegment2Id);
    m_arc2Segment1Id = VAbstractPattern::GetParametrId(domElement, AttrSegment3Id);
    m_arc2Segment2Id = VAbstractPattern::GetParametrId(domElement, AttrSegment4Id);

    const VAbstractValApplication *app = VAbstractValApplication::VApp();

    const QString labelMXStr = QString::number(app->fromPixel(labelMX));
    const QString labelMYStr = QString::number(app->fromPixel(labelMY));

    m_arc1Segment1Mx = app->toPixel(VAbstractPattern::GetParametrDouble(domElement, AttrSegment1Mx, labelMXStr));
    m_arc1Segment1My = app->toPixel(VAbstractPattern::GetParametrDouble(domElement, AttrSegment1My, labelMYStr));
    m_arc1Segment2Mx = app->toPixel(VAbstractPattern::GetParametrDouble(domElement, AttrSegment2Mx, labelMXStr));
    m_arc1Segment2My = app->toPixel(VAbstractPattern::GetParametrDouble(domElement, AttrSegment2My, labelMYStr));
    m_arc2Segment1Mx = app->toPixel(VAbstractPattern::GetParametrDouble(domElement, AttrSegment3Mx, labelMXStr));
    m_arc2Segment1My = app->toPixel(VAbstractPattern::GetParametrDouble(domElement, AttrSegment3My, labelMYStr));
    m_arc2Segment2Mx = app->toPixel(VAbstractPattern::GetParametrDouble(domElement, AttrSegment4Mx, labelMXStr));
    m_arc2Segment2My = app->toPixel(VAbstractPattern::GetParametrDouble(domElement, AttrSegment4My, labelMYStr));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolPointOfIntersectionArcs *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetArc1Id(firstArcId);
        visual->SetArc2Id(secondArcId);
        visual->SetCrossPoint(crossPoint);
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    ProcessToolOptions(oldDomElement, newDomElement, GatherToolChanges());
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionArcs::itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant
{
    if (change == QGraphicsItem::ItemSelectedHasChanged && !m_selectedFromChild && segLableVisRefCount == 0)
    {
        {
            const QSignalBlocker blocker(m_arc1Segment1Label);
            m_arc1Segment1Label->SetLabelVisible(value.toBool());
            m_arc1Segment1Label->setSelected(value.toBool());
        }

        {
            const QSignalBlocker blocke(m_arc1Segment2Label);
            m_arc1Segment2Label->setSelected(value.toBool());
            m_arc1Segment2Label->SetLabelVisible(value.toBool());
        }

        {
            const QSignalBlocker blocker(m_arc2Segment1Label);
            m_arc2Segment1Label->SetLabelVisible(value.toBool());
            m_arc2Segment1Label->setSelected(value.toBool());
        }

        {
            const QSignalBlocker blocker(m_arc2Segment2Label);
            m_arc2Segment2Label->SetLabelVisible(value.toBool());
            m_arc2Segment2Label->setSelected(value.toBool());
        }
    }

    return VToolSinglePoint::itemChange(change, value);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::RefreshGeometry()
{
    VToolSinglePoint::RefreshGeometry();

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

    UpdateSegmentLabel(m_arc1Segment1Id, m_arc1Segment1Mx, m_arc1Segment1My, m_arc1Segment1Label);
    UpdateSegmentLabel(m_arc1Segment2Id, m_arc1Segment2Mx, m_arc1Segment2My, m_arc1Segment2Label);
    UpdateSegmentLabel(m_arc2Segment1Id, m_arc2Segment1Mx, m_arc2Segment1My, m_arc2Segment1Label);
    UpdateSegmentLabel(m_arc2Segment2Id, m_arc2Segment2Mx, m_arc2Segment2My, m_arc2Segment2Label);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::SegmentChoosed(quint32 id, SceneObject type)
{
    emit ChoosedTool(id, type);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::Arc1Segment1LabelPositionChanged(const QPointF &pos)
{
    VAbstractApplication::VApp()->getUndoStack()->push(new MoveSegmentLabel(doc, pos, m_id, SegmentLabel::Segment1));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::Arc1Segment2LabelPositionChanged(const QPointF &pos)
{
    VAbstractApplication::VApp()->getUndoStack()->push(new MoveSegmentLabel(doc, pos, m_id, SegmentLabel::Segment2));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::Arc2Segment1LabelPositionChanged(const QPointF &pos)
{
    VAbstractApplication::VApp()->getUndoStack()->push(new MoveSegmentLabel(doc, pos, m_id, SegmentLabel::Segment3));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionArcs::Arc2Segment2LabelPositionChanged(const QPointF &pos)
{
    VAbstractApplication::VApp()->getUndoStack()->push(new MoveSegmentLabel(doc, pos, m_id, SegmentLabel::Segment4));
}
