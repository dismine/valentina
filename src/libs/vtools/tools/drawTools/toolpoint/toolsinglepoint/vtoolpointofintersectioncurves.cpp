/************************************************************************
 **
 **  @file   vtoolpointofintersectioncurves.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 1, 2016
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

#include "vtoolpointofintersectioncurves.h"

#include <QLineF>
#include <QSharedPointer>
#include <QUndoStack>

#include "../../../../dialogs/tools/dialogpointofintersectioncurves.h"
#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../undocommands/label/movesegmentlabel.h"
#include "../../../../undocommands/renameobject.h"
#include "../../../../undocommands/savetooloptions.h"
#include "../../../../visualization/path/vistoolpointofintersectioncurves.h"
#include "../../../vabstracttool.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/exception/vexception.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vsegmentlabel.h"
#include "vtoolsinglepoint.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

template <class T> class QSharedPointer;

const QString VToolPointOfIntersectionCurves::ToolType = QStringLiteral("pointOfIntersectionCurves");

//---------------------------------------------------------------------------------------------------------------------
VToolPointOfIntersectionCurves::VToolPointOfIntersectionCurves(const VToolPointOfIntersectionCurvesInitData &initData,
                                                               QGraphicsItem *parent)
  : VToolSinglePoint(initData.doc, initData.data, initData.id, initData.notes, parent),
    firstCurveId(initData.firstCurveId),
    secondCurveId(initData.secondCurveId),
    vCrossPoint(initData.vCrossPoint),
    hCrossPoint(initData.hCrossPoint),
    m_curve1Name1(initData.curve1Name1),
    m_curve1Name2(initData.curve1Name2),
    m_curve2Name1(initData.curve2Name1),
    m_curve2Name2(initData.curve2Name2),
    m_curve1AliasSuffix1(initData.curve1AliasSuffix1),
    m_curve1AliasSuffix2(initData.curve1AliasSuffix2),
    m_curve2AliasSuffix1(initData.curve2AliasSuffix1),
    m_curve2AliasSuffix2(initData.curve2AliasSuffix2),
    m_curve1Segment1Id(initData.curve1Segment1Id),
    m_curve1Segment2Id(initData.curve1Segment2Id),
    m_curve2Segment1Id(initData.curve2Segment1Id),
    m_curve2Segment2Id(initData.curve2Segment2Id),
    m_curve1Segment1Mx(initData.curve1Segment1Mx),
    m_curve1Segment1My(initData.curve1Segment1My),
    m_curve1Segment2Mx(initData.curve1Segment2Mx),
    m_curve1Segment2My(initData.curve1Segment2My),
    m_curve2Segment1Mx(initData.curve2Segment1Mx),
    m_curve2Segment1My(initData.curve2Segment1My),
    m_curve2Segment2Mx(initData.curve2Segment2Mx),
    m_curve2Segment2My(initData.curve2Segment2My)
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

    m_curve1Segment1Label = CreateSegmentLabel(m_curve1Segment1Id,
                                               m_curve1Segment1Mx,
                                               m_curve1Segment1My,
                                               &VToolPointOfIntersectionCurves::SegmentChoosed,
                                               &VToolPointOfIntersectionCurves::PointSelected,
                                               &VToolPointOfIntersectionCurves::Curve1Segment1LabelPositionChanged);

    m_curve1Segment2Label = CreateSegmentLabel(m_curve1Segment2Id,
                                               m_curve1Segment2Mx,
                                               m_curve1Segment2My,
                                               &VToolPointOfIntersectionCurves::SegmentChoosed,
                                               &VToolPointOfIntersectionCurves::PointSelected,
                                               &VToolPointOfIntersectionCurves::Curve1Segment2LabelPositionChanged);

    m_curve2Segment1Label = CreateSegmentLabel(m_curve2Segment1Id,
                                               m_curve2Segment1Mx,
                                               m_curve2Segment1My,
                                               &VToolPointOfIntersectionCurves::SegmentChoosed,
                                               &VToolPointOfIntersectionCurves::PointSelected,
                                               &VToolPointOfIntersectionCurves::Curve2Segment1LabelPositionChanged);

    m_curve2Segment2Label = CreateSegmentLabel(m_curve2Segment2Id,
                                               m_curve2Segment2Mx,
                                               m_curve2Segment2My,
                                               &VToolPointOfIntersectionCurves::SegmentChoosed,
                                               &VToolPointOfIntersectionCurves::PointSelected,
                                               &VToolPointOfIntersectionCurves::Curve2Segment2LabelPositionChanged);

    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::ProcessToolOptions(const QDomElement &oldDomElement,
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

    if (changes.LabelChanged())
    {
        const auto *renameLabel = new RenameLabel(changes.oldLabel, changes.newLabel, doc, m_id, newGroup);
        if (!changes.Curve1Name1Changed() && !changes.Curve1Name2Changed() && !changes.Curve2Name1Changed()
            && !changes.Curve2Name2Changed() && !changes.Curve1AliasSuffix1Changed()
            && !changes.Curve1AliasSuffix2Changed() && !changes.Curve2AliasSuffix1Changed()
            && !changes.Curve2AliasSuffix2Changed())
        {
            connect(renameLabel, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    const QSharedPointer<VAbstractCurve> curve1 = VAbstractTool::data.GeometricObject<VAbstractCurve>(firstCurveId);
    const CurveAliasType curve1Type = RenameAlias::CurveType(curve1->getType());

    if (changes.Curve1Name1Changed())
    {
        const auto *renameName = new RenameAlias(curve1Type,
                                                 changes.oldCurve1Name1,
                                                 changes.newCurve1Name1,
                                                 doc,
                                                 m_curve1Segment1Id,
                                                 newGroup);
        if (!changes.Curve1Name2Changed() && !changes.Curve2Name1Changed() && !changes.Curve2Name2Changed()
            && !changes.Curve1AliasSuffix1Changed() && !changes.Curve1AliasSuffix2Changed()
            && !changes.Curve2AliasSuffix1Changed() && !changes.Curve2AliasSuffix2Changed())
        {
            connect(renameName, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    if (changes.Curve1Name2Changed())
    {
        const auto *renameName = new RenameAlias(curve1Type,
                                                 changes.oldCurve1Name2,
                                                 changes.newCurve1Name2,
                                                 doc,
                                                 m_curve1Segment2Id,
                                                 newGroup);
        if (!changes.Curve2Name1Changed() && !changes.Curve2Name2Changed() && !changes.Curve1AliasSuffix1Changed()
            && !changes.Curve1AliasSuffix2Changed() && !changes.Curve2AliasSuffix1Changed()
            && !changes.Curve2AliasSuffix2Changed())
        {
            connect(renameName, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    if (changes.Curve1AliasSuffix1Changed())
    {
        const auto *renameAlias = new RenameAlias(curve1Type,
                                                  changes.oldCurve1AliasSuffix1,
                                                  changes.newCurve1AliasSuffix1,
                                                  doc,
                                                  m_curve1Segment1Id,
                                                  newGroup);
        if (!changes.Curve2Name1Changed() && !changes.Curve2Name2Changed() && !changes.Curve1AliasSuffix2Changed()
            && !changes.Curve2AliasSuffix1Changed() && !changes.Curve2AliasSuffix2Changed())
        {
            connect(renameAlias, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    if (changes.Curve1AliasSuffix2Changed())
    {
        const auto *renameAlias = new RenameAlias(curve1Type,
                                                  changes.oldCurve1AliasSuffix1,
                                                  changes.newCurve1AliasSuffix1,
                                                  doc,
                                                  m_curve1Segment2Id,
                                                  newGroup);
        if (!changes.Curve2Name1Changed() && !changes.Curve2Name2Changed() && !changes.Curve2AliasSuffix1Changed()
            && !changes.Curve2AliasSuffix2Changed())
        {
            connect(renameAlias, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    const QSharedPointer<VAbstractCurve> curve2 = VAbstractTool::data.GeometricObject<VAbstractCurve>(secondCurveId);
    const CurveAliasType curve2Type = RenameAlias::CurveType(curve1->getType());

    if (changes.Curve2Name1Changed())
    {
        const auto *renameName = new RenameAlias(curve2Type,
                                                 changes.oldCurve2Name1,
                                                 changes.newCurve2Name1,
                                                 doc,
                                                 m_curve2Segment1Id,
                                                 newGroup);
        if (!changes.Curve2Name2Changed() && !changes.Curve2AliasSuffix1Changed()
            && !changes.Curve2AliasSuffix2Changed())
        {
            connect(renameName, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    if (changes.Curve2Name2Changed())
    {
        const auto *renameName = new RenameAlias(curve2Type,
                                                 changes.oldCurve2Name2,
                                                 changes.newCurve2Name2,
                                                 doc,
                                                 m_curve2Segment2Id,
                                                 newGroup);
        if (!changes.Curve2AliasSuffix1Changed() && !changes.Curve2AliasSuffix2Changed())
        {
            connect(renameName, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    if (changes.Curve2AliasSuffix1Changed())
    {
        const auto *renameAlias = new RenameAlias(curve2Type,
                                                  changes.oldCurve2AliasSuffix1,
                                                  changes.newCurve2AliasSuffix1,
                                                  doc,
                                                  m_curve2Segment1Id,
                                                  newGroup);
        if (!changes.Curve2AliasSuffix2Changed())
        {
            connect(renameAlias, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    if (changes.Curve2AliasSuffix2Changed())
    {
        const auto *renameAlias = new RenameAlias(curve2Type,
                                                  changes.oldCurve2AliasSuffix1,
                                                  changes.newCurve2AliasSuffix1,
                                                  doc,
                                                  m_curve2Segment2Id,
                                                  newGroup);

        connect(renameAlias, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    }

    undoStack->push(newGroup);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::GetFieldMetadata(VToolPointOfIntersectionCurvesNameField field)
    -> VToolPointOfIntersectionCurvesFieldMetadata
{
    switch (field)
    {
        case VToolPointOfIntersectionCurvesNameField::Curve1Name1:
            return {.curveId = firstCurveId, .isName = true, .memberPtr = &m_curve1Name1};
        case VToolPointOfIntersectionCurvesNameField::Curve1Name2:
            return {.curveId = firstCurveId, .isName = true, .memberPtr = &m_curve1Name2};
        case VToolPointOfIntersectionCurvesNameField::Curve2Name1:
            return {.curveId = secondCurveId, .isName = true, .memberPtr = &m_curve2Name1};
        case VToolPointOfIntersectionCurvesNameField::Curve2Name2:
            return {.curveId = secondCurveId, .isName = true, .memberPtr = &m_curve2Name2};
        case VToolPointOfIntersectionCurvesNameField::Curve1AliasSuffix1:
            return {.curveId = firstCurveId, .isName = false, .memberPtr = &m_curve1AliasSuffix1};
        case VToolPointOfIntersectionCurvesNameField::Curve1AliasSuffix2:
            return {.curveId = firstCurveId, .isName = false, .memberPtr = &m_curve1AliasSuffix2};
        case VToolPointOfIntersectionCurvesNameField::Curve2AliasSuffix1:
            return {.curveId = secondCurveId, .isName = false, .memberPtr = &m_curve2AliasSuffix1};
        case VToolPointOfIntersectionCurvesNameField::Curve2AliasSuffix2:
            return {.curveId = secondCurveId, .isName = false, .memberPtr = &m_curve2AliasSuffix2};
        default:
            break;
    }

    Q_UNREACHABLE();
    return {}; // Should never reach
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::HasConflict(const QString &value,
                                                 VToolPointOfIntersectionCurvesNameField currentField) const -> bool
{
    if (value.isEmpty())
    {
        return false;
    }

    // Check all name fields
    if (currentField != VToolPointOfIntersectionCurvesNameField::Curve1Name1 && value == m_curve1Name1)
    {
        return true;
    }
    if (currentField != VToolPointOfIntersectionCurvesNameField::Curve1Name2 && value == m_curve1Name2)
    {
        return true;
    }
    if (currentField != VToolPointOfIntersectionCurvesNameField::Curve2Name1 && value == m_curve2Name1)
    {
        return true;
    }
    if (currentField != VToolPointOfIntersectionCurvesNameField::Curve2Name2 && value == m_curve2Name2)
    {
        return true;
    }

    // Check non-empty alias fields
    if (currentField != VToolPointOfIntersectionCurvesNameField::Curve1AliasSuffix1 && !m_curve1AliasSuffix1.isEmpty()
        && value == m_curve1AliasSuffix1)
    {
        return true;
    }
    if (currentField != VToolPointOfIntersectionCurvesNameField::Curve1AliasSuffix2 && !m_curve1AliasSuffix2.isEmpty()
        && value == m_curve1AliasSuffix2)
    {
        return true;
    }
    if (currentField != VToolPointOfIntersectionCurvesNameField::Curve2AliasSuffix1 && !m_curve2AliasSuffix1.isEmpty()
        && value == m_curve2AliasSuffix1)
    {
        return true;
    }
    if (currentField != VToolPointOfIntersectionCurvesNameField::Curve2AliasSuffix2 && !m_curve2AliasSuffix2.isEmpty()
        && value == m_curve2AliasSuffix2)
    {
        return true;
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::UpdateNameField(VToolPointOfIntersectionCurvesNameField field, const QString &value)
{
    VToolPointOfIntersectionCurvesFieldMetadata const metadata = GetFieldMetadata(field);

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
    ToolChanges const changes
        = {.oldLabel = label,
           .newLabel = label,
           .oldCurve1Name1 = m_curve1Name1,
           .newCurve1Name1 = (field == VToolPointOfIntersectionCurvesNameField::Curve1Name1) ? value : m_curve1Name1,
           .oldCurve1Name2 = m_curve1Name2,
           .newCurve1Name2 = (field == VToolPointOfIntersectionCurvesNameField::Curve1Name2) ? value : m_curve1Name2,
           .oldCurve2Name1 = m_curve2Name1,
           .newCurve2Name1 = (field == VToolPointOfIntersectionCurvesNameField::Curve2Name1) ? value : m_curve2Name1,
           .oldCurve2Name2 = m_curve2Name2,
           .newCurve2Name2 = (field == VToolPointOfIntersectionCurvesNameField::Curve2Name2) ? value : m_curve2Name2,
           .oldCurve1AliasSuffix1 = m_curve1AliasSuffix1,
           .newCurve1AliasSuffix1 = (field == VToolPointOfIntersectionCurvesNameField::Curve1AliasSuffix1)
                                        ? value
                                        : m_curve1AliasSuffix1,
           .oldCurve1AliasSuffix2 = m_curve1AliasSuffix2,
           .newCurve1AliasSuffix2 = (field == VToolPointOfIntersectionCurvesNameField::Curve1AliasSuffix2)
                                        ? value
                                        : m_curve1AliasSuffix2,
           .oldCurve2AliasSuffix1 = m_curve2AliasSuffix1,
           .newCurve2AliasSuffix1 = (field == VToolPointOfIntersectionCurvesNameField::Curve2AliasSuffix1)
                                        ? value
                                        : m_curve2AliasSuffix1,
           .oldCurve2AliasSuffix2 = m_curve2AliasSuffix2,
           .newCurve2AliasSuffix2 = (field == VToolPointOfIntersectionCurvesNameField::Curve2AliasSuffix2)
                                        ? value
                                        : m_curve2AliasSuffix2};

    // Update the appropriate member variable
    *(metadata.memberPtr) = value;

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    QDomElement newDomElement = oldDomElement.cloneNode().toElement();
    SaveOptions(newDomElement, obj);
    ProcessToolOptions(oldDomElement, newDomElement, changes);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetSegmentLabelVisible(quint32 curveId,
                                                            bool visible,
                                                            int &refCount,
                                                            VSegmentLabel *seg1Label,
                                                            VSegmentLabel *seg2Label,
                                                            std::initializer_list<GOType> types) const
{
    const QSharedPointer<VGObject> curve = VAbstractTool::data.GetGObject(curveId);
    const GOType type = curve->getType();

    if (const bool matches = std::any_of(types.begin(), types.end(), [type](GOType t) { return t == type; }); !matches)
    {
        return;
    }

    refCount += visible ? 1 : -1;
    refCount = qMax(refCount, 0);

    seg1Label->SetLabelVisible(visible);
    seg2Label->SetLabelVisible(visible);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetSegmentLabelHover(quint32 curveId,
                                                          bool enabled,
                                                          VSegmentLabel *seg1Label,
                                                          VSegmentLabel *seg2Label,
                                                          std::initializer_list<GOType> types) const
{
    const QSharedPointer<VGObject> curve = VAbstractTool::data.GetGObject(curveId);
    const GOType type = curve->getType();

    if (const bool matches = std::any_of(types.begin(), types.end(), [type](GOType t) { return t == type; }); !matches)
    {
        return;
    }

    seg1Label->AllowLabelHover(enabled);
    seg2Label->AllowLabelHover(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::GatherToolChanges() const -> VToolPointOfIntersectionCurves::ToolChanges
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogPointOfIntersectionCurves> dialogTool = qobject_cast<DialogPointOfIntersectionCurves *>(
        m_dialog);
    SCASSERT(not dialogTool.isNull())

    return {.oldLabel = VAbstractTool::data.GeometricObject<VPointF>(m_id)->name(),
            .newLabel = dialogTool->GetPointName(),
            .oldCurve1Name1 = m_curve1Name1,
            .newCurve1Name1 = dialogTool->GetCurve1Name1(),
            .oldCurve1Name2 = m_curve1Name2,
            .newCurve1Name2 = dialogTool->GetCurve1Name2(),
            .oldCurve2Name1 = m_curve2Name1,
            .newCurve2Name1 = dialogTool->GetCurve2Name1(),
            .oldCurve2Name2 = m_curve2Name2,
            .newCurve2Name2 = dialogTool->GetCurve2Name2(),
            .oldCurve1AliasSuffix1 = m_curve1AliasSuffix1,
            .newCurve1AliasSuffix1 = dialogTool->GetCurve1AliasSuffix1(),
            .oldCurve1AliasSuffix2 = m_curve1AliasSuffix2,
            .newCurve1AliasSuffix2 = dialogTool->GetCurve1AliasSuffix2(),
            .oldCurve2AliasSuffix1 = m_curve2AliasSuffix1,
            .newCurve2AliasSuffix1 = dialogTool->GetCurve2AliasSuffix1(),
            .oldCurve2AliasSuffix2 = m_curve2AliasSuffix2,
            .newCurve2AliasSuffix2 = dialogTool->GetCurve2AliasSuffix2()};
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    auto *dialogTool = qobject_cast<DialogPointOfIntersectionCurves *>(m_dialog);
    SCASSERT(dialogTool != nullptr)
    auto p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetFirstCurveId(firstCurveId);
    dialogTool->SetSecondCurveId(secondCurveId);
    dialogTool->SetVCrossPoint(vCrossPoint);
    dialogTool->SetHCrossPoint(hCrossPoint);
    dialogTool->SetPointName(p->name());
    dialogTool->SetNotes(m_notes);
    dialogTool->SetCurve1Name1(m_curve1Name1);
    dialogTool->SetCurve1Name2(m_curve1Name2);
    dialogTool->SetCurve2Name1(m_curve2Name1);
    dialogTool->SetCurve2Name2(m_curve2Name2);
    dialogTool->SetCurve1AliasSuffix1(m_curve1AliasSuffix1);
    dialogTool->SetCurve1AliasSuffix2(m_curve1AliasSuffix2);
    dialogTool->SetCurve2AliasSuffix1(m_curve2AliasSuffix1);
    dialogTool->SetCurve2AliasSuffix2(m_curve2AliasSuffix2);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene,
                                            VAbstractPattern *doc, VContainer *data) -> VToolPointOfIntersectionCurves *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogPointOfIntersectionCurves> dialogTool =
        qobject_cast<DialogPointOfIntersectionCurves *>(dialog);
    SCASSERT(not dialogTool.isNull())

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    VToolPointOfIntersectionCurvesInitData initData;

    QT_WARNING_POP

    initData.firstCurveId = dialogTool->GetFirstCurveId();
    initData.secondCurveId = dialogTool->GetSecondCurveId();
    initData.vCrossPoint = dialogTool->GetVCrossPoint();
    initData.hCrossPoint = dialogTool->GetHCrossPoint();
    initData.name = dialogTool->GetPointName();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();
    initData.curve1Name1 = dialogTool->GetCurve1Name1();
    initData.curve1Name2 = dialogTool->GetCurve1Name2();
    initData.curve2Name1 = dialogTool->GetCurve2Name1();
    initData.curve2Name2 = dialogTool->GetCurve2Name2();
    initData.curve1AliasSuffix1 = dialogTool->GetCurve1AliasSuffix1();
    initData.curve1AliasSuffix2 = dialogTool->GetCurve1AliasSuffix2();
    initData.curve2AliasSuffix1 = dialogTool->GetCurve2AliasSuffix1();
    initData.curve2AliasSuffix2 = dialogTool->GetCurve2AliasSuffix2();

    VToolPointOfIntersectionCurves *point = Create(initData);
    if (point != nullptr)
    {
        point->m_dialog = dialog;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::Create(VToolPointOfIntersectionCurvesInitData initData)
    -> VToolPointOfIntersectionCurves *
{
    auto curve1 = initData.data->GeometricObject<VAbstractCurve>(initData.firstCurveId);
    auto curve2 = initData.data->GeometricObject<VAbstractCurve>(initData.secondCurveId);

    QPointF fPoint;
    if (!VToolPointOfIntersectionCurves::FindPoint(curve1->GetPoints(),
                                                   curve2->GetPoints(),
                                                   initData.vCrossPoint,
                                                   initData.hCrossPoint,
                                                   &fPoint))
    {
        const QString errorMsg = tr("Error calculating point '%1'. Curves '%2' and '%3' have no point of intersection")
                                     .arg(initData.name, curve1->name(), curve2->name());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VExceptionObjectError(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
    }

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

    SegmentDetails curve1Details{.typeCreation = initData.typeCreation,
                                 .curveType = curve1->getType(),
                                 .segLength = curve1->GetLengthByPoint(fPoint),
                                 .p = *p,
                                 .curveId = initData.firstCurveId,
                                 .data = initData.data,
                                 .doc = initData.doc,
                                 .name1 = initData.curve1Name1,
                                 .name2 = initData.curve1Name2,
                                 .alias1 = initData.curve1AliasSuffix1,
                                 .alias2 = initData.curve1AliasSuffix2,
                                 .id = initData.id,
                                 .name1AttrName = AttrCurve1Name1,
                                 .name2AttrName = AttrCurve1Name2,
                                 .segment1Id = initData.curve1Segment1Id,
                                 .segment2Id = initData.curve1Segment2Id};
    VToolSinglePoint::InitSegments(curve1Details);

    initData.curve1Name1 = curve1Details.name1;
    initData.curve1Name2 = curve1Details.name2;

    SegmentDetails curve2Details{.typeCreation = initData.typeCreation,
                                 .curveType = curve2->getType(),
                                 .segLength = curve2->GetLengthByPoint(fPoint),
                                 .p = *p,
                                 .curveId = initData.secondCurveId,
                                 .data = initData.data,
                                 .doc = initData.doc,
                                 .name1 = initData.curve2Name1,
                                 .name2 = initData.curve2Name2,
                                 .alias1 = initData.curve2AliasSuffix1,
                                 .alias2 = initData.curve2AliasSuffix2,
                                 .id = initData.id,
                                 .name1AttrName = AttrCurve2Name1,
                                 .name2AttrName = AttrCurve2Name2,
                                 .segment1Id = initData.curve2Segment1Id,
                                 .segment2Id = initData.curve2Segment2Id};
    VToolSinglePoint::InitSegments(curve2Details);

    initData.curve2Name1 = curve2Details.name1;
    initData.curve2Name2 = curve2Details.name2;

    patternGraph->AddEdge(initData.firstCurveId, initData.id);
    patternGraph->AddEdge(initData.secondCurveId, initData.id);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.curve1Segment1Id = curve1Details.segment1Id;
        initData.curve1Segment2Id = curve1Details.segment2Id;
        initData.curve2Segment1Id = curve2Details.segment1Id;
        initData.curve2Segment2Id = curve2Details.segment2Id;
    }

    patternGraph->AddVertex(initData.curve1Segment1Id,
                            VNodeType::OBJECT,
                            initData.doc->PatternBlockMapper()->GetActiveId());
    patternGraph->AddVertex(initData.curve1Segment2Id,
                            VNodeType::OBJECT,
                            initData.doc->PatternBlockMapper()->GetActiveId());
    patternGraph->AddVertex(initData.curve2Segment1Id,
                            VNodeType::OBJECT,
                            initData.doc->PatternBlockMapper()->GetActiveId());
    patternGraph->AddVertex(initData.curve2Segment2Id,
                            VNodeType::OBJECT,
                            initData.doc->PatternBlockMapper()->GetActiveId());

    patternGraph->AddEdge(initData.id, initData.curve1Segment1Id);
    patternGraph->AddEdge(initData.id, initData.curve1Segment2Id);
    patternGraph->AddEdge(initData.id, initData.curve2Segment1Id);
    patternGraph->AddEdge(initData.id, initData.curve2Segment2Id);

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::PointOfIntersectionCurves, initData.doc);
        auto *point = new VToolPointOfIntersectionCurves(initData);
        initData.scene->addItem(point);
        InitPointToolConnections(initData.scene, point);
        InitUniqueSegmentConnections(initData.scene, point, curve1->getType());
        InitUniqueSegmentConnections(initData.scene, point, curve2->getType());
        VAbstractPattern::AddTool(initData.id, point);
        return point;
    }

    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::FindPoint(const QVector<QPointF> &curve1Points,
                                               const QVector<QPointF> &curve2Points, VCrossCurvesPoint vCrossPoint,
                                               HCrossCurvesPoint hCrossPoint, QPointF *intersectionPoint) -> bool
{
    SCASSERT(intersectionPoint != nullptr)

    if (curve1Points.isEmpty() || curve2Points.isEmpty())
    {
        return false;
    }

    QVector<QPointF> intersections;
    for (auto i = 0; i < curve1Points.count() - 1; ++i)
    {
        intersections << VAbstractCurve::CurveIntersectLine(curve2Points,
                                                            QLineF(curve1Points.at(i), curve1Points.at(i + 1)));
    }

    if (intersections.isEmpty())
    {
        return false;
    }

    if (intersections.size() == 1)
    {
        *intersectionPoint = intersections.at(0);
        return true;
    }

    QVector<QPointF> vIntersections;
    if (vCrossPoint == VCrossCurvesPoint::HighestPoint)
    {
        qreal minY = intersections.at(0).y();
        vIntersections.append(intersections.at(0));

        for (auto i = 1; i < intersections.count(); ++i)
        {
            const QPointF p = intersections.at(i);
            if (p.y() < minY)
            {
                minY = p.y();
                vIntersections.clear();
                vIntersections.append(p);
            }
            else if (VFuzzyComparePossibleNulls(p.y(), minY))
            {
                vIntersections.append(p);
            }
        }
    }
    else
    {
        qreal maxY = intersections.at(0).y();
        vIntersections.append(intersections.at(0));

        for (auto i = 1; i < intersections.count(); ++i)
        {
            const QPointF p = intersections.at(i);
            if (p.y() > maxY)
            {
                maxY = p.y();
                vIntersections.clear();
                vIntersections.append(p);
            }
            else if (VFuzzyComparePossibleNulls(p.y(), maxY))
            {
                vIntersections.append(p);
            }
        }
    }

    if (vIntersections.isEmpty())
    {
        return false;
    }

    if (vIntersections.size() == 1)
    {
        *intersectionPoint = vIntersections.at(0);
        return true;
    }

    QPointF crossPoint = vIntersections.at(0);

    if (hCrossPoint == HCrossCurvesPoint::RightmostPoint)
    {
        qreal maxX = vIntersections.at(0).x();

        for (auto i = 1; i < vIntersections.count(); ++i)
        {
            const QPointF p = vIntersections.at(i);
            if (p.x() > maxX)
            {
                maxX = p.x();
                crossPoint = p;
            }
        }
    }
    else
    {
        qreal minX = vIntersections.at(0).x();

        for (auto i = 1; i < vIntersections.count(); ++i)
        {
            const QPointF p = vIntersections.at(i);
            if (p.x() < minX)
            {
                minX = p.x();
                crossPoint = p;
            }
        }
    }

    *intersectionPoint = crossPoint;
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::FirstCurveName() const -> QString
{
    return VAbstractTool::data.GetGObject(firstCurveId)->ObjectName();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::SecondCurveName() const -> QString
{
    return VAbstractTool::data.GetGObject(secondCurveId)->ObjectName();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::GetCurve1Name1() const -> QString
{
    return m_curve1Name1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetCurve1Name1(const QString &name)
{
    UpdateNameField(VToolPointOfIntersectionCurvesNameField::Curve1Name1, name);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::GetCurve1Name2() const -> QString
{
    return m_curve1Name2;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetCurve1Name2(const QString &name)
{
    UpdateNameField(VToolPointOfIntersectionCurvesNameField::Curve1Name2, name);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::GetCurve2Name1() const -> QString
{
    return m_curve2Name1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetCurve2Name1(const QString &name)
{
    UpdateNameField(VToolPointOfIntersectionCurvesNameField::Curve2Name1, name);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::GetCurve2Name2() const -> QString
{
    return m_curve2Name2;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetCurve2Name2(const QString &name)
{
    UpdateNameField(VToolPointOfIntersectionCurvesNameField::Curve2Name2, name);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::GetCurve1AliasSuffix1() const -> QString
{
    return m_curve1AliasSuffix1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetCurve1AliasSuffix1(const QString &alias)
{
    UpdateNameField(VToolPointOfIntersectionCurvesNameField::Curve1AliasSuffix1, alias);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::GetCurve1AliasSuffix2() const -> QString
{
    return m_curve1AliasSuffix2;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetCurve1AliasSuffix2(const QString &alias)
{
    UpdateNameField(VToolPointOfIntersectionCurvesNameField::Curve1AliasSuffix2, alias);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::GetCurve2AliasSuffix1() const -> QString
{
    return m_curve2AliasSuffix1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetCurve2AliasSuffix1(const QString &alias)
{
    UpdateNameField(VToolPointOfIntersectionCurvesNameField::Curve2AliasSuffix1, alias);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::GetCurve2AliasSuffix2() const -> QString
{
    return m_curve2AliasSuffix2;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetCurve2AliasSuffix2(const QString &alias)
{
    UpdateNameField(VToolPointOfIntersectionCurvesNameField::Curve2AliasSuffix2, alias);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::GetVCrossPoint() const -> VCrossCurvesPoint
{
    return vCrossPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetVCrossPoint(VCrossCurvesPoint value)
{
    vCrossPoint = value;

    auto obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::GetHCrossPoint() const -> HCrossCurvesPoint
{
    return hCrossPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetHCrossPoint(HCrossCurvesPoint value)
{
    hCrossPoint = value;

    auto obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolPointOfIntersectionCurves>(show);
    m_curve1Segment1Label->SetLabelVisible(show);
    m_curve1Segment2Label->SetLabelVisible(show);
    m_curve2Segment1Label->SetLabelVisible(show);
    m_curve2Segment2Label->SetLabelVisible(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::ChangeSegmentLabelPosition(SegmentLabel segment, const QPointF &pos)
{
    switch (segment)
    {
        case SegmentLabel::Segment1:
            m_curve1Segment1Label->SetLabelPosition(pos);
            break;
        case SegmentLabel::Segment2:
            m_curve1Segment2Label->SetLabelPosition(pos);
            break;
        case SegmentLabel::Segment3:
            m_curve2Segment1Label->SetLabelPosition(pos);
            break;
        case SegmentLabel::Segment4:
            m_curve2Segment2Label->SetLabelPosition(pos);
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::IsRemovable() const -> RemoveStatus
{
    if (!doc->IsPatternGraphComplete())
    {
        return RemoveStatus::Pending; // Data not ready yet
    }

    VPatternGraph const *patternGraph = doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    auto Filter = [](const auto &node) -> auto
    { return node.type != VNodeType::MODELING_TOOL && node.type != VNodeType::MODELING_OBJECT; };

    auto const curve1Segment1Dependecies = patternGraph->TryGetDependentNodes(m_curve1Segment1Id, 500, Filter);
    if (!curve1Segment1Dependecies)
    {
        return RemoveStatus::Pending; // Lock timeout
    }

    if (!curve1Segment1Dependecies->isEmpty())
    {
        return RemoveStatus::Blocked;
    }

    auto const curve1Segment2Dependecies = patternGraph->TryGetDependentNodes(m_curve1Segment2Id, 500, Filter);
    if (!curve1Segment2Dependecies)
    {
        return RemoveStatus::Pending; // Lock timeout
    }

    if (!curve1Segment2Dependecies->isEmpty())
    {
        return RemoveStatus::Blocked;
    }

    auto const curve2Segment1Dependecies = patternGraph->TryGetDependentNodes(m_curve2Segment1Id, 500, Filter);
    if (!curve2Segment1Dependecies)
    {
        return RemoveStatus::Pending; // Lock timeout
    }

    if (!curve2Segment1Dependecies->isEmpty())
    {
        return RemoveStatus::Blocked;
    }

    auto const curve2Segment2Dependecies = patternGraph->TryGetDependentNodes(m_curve2Segment2Id, 500, Filter);
    if (!curve2Segment2Dependecies)
    {
        return RemoveStatus::Pending; // Lock timeout
    }

    if (!curve2Segment2Dependecies->isEmpty())
    {
        return RemoveStatus::Blocked;
    }

    return RemoveStatus::Removable;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogPointOfIntersectionCurves>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::Enable()
{
    const bool enabled = m_indexActivePatternBlock == doc->PatternBlockMapper()->GetActiveId();
    m_curve1Segment1Label->SetEnabledState(enabled);
    m_curve1Segment2Label->SetEnabledState(enabled);
    m_curve2Segment1Label->SetEnabledState(enabled);
    m_curve2Segment2Label->SetEnabledState(enabled);

    VToolSinglePoint::Enable();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::EnableToolMove(bool move)
{
    m_curve1Segment1Label->SetLabelMovable(move);
    m_curve1Segment2Label->SetLabelMovable(move);
    m_curve2Segment1Label->SetLabelMovable(move);
    m_curve2Segment2Label->SetLabelMovable(move);

    VToolSinglePoint::EnableToolMove(move);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::AllowArcSegmentHover(bool enabled)
{
    SetSegmentLabelHover(firstCurveId, enabled, m_curve1Segment1Label, m_curve1Segment2Label, {GOType::Arc});
    SetSegmentLabelHover(secondCurveId, enabled, m_curve2Segment1Label, m_curve2Segment2Label, {GOType::Arc});
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::AllowElArcSegmentHover(bool enabled)
{
    SetSegmentLabelHover(firstCurveId, enabled, m_curve1Segment1Label, m_curve1Segment2Label, {GOType::EllipticalArc});
    SetSegmentLabelHover(secondCurveId, enabled, m_curve2Segment1Label, m_curve2Segment2Label, {GOType::EllipticalArc});
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::AllowSplineSegmentHover(bool enabled)
{
    SetSegmentLabelHover(firstCurveId,
                         enabled,
                         m_curve1Segment1Label,
                         m_curve1Segment2Label,
                         {GOType::CubicBezier, GOType::Spline});
    SetSegmentLabelHover(secondCurveId,
                         enabled,
                         m_curve2Segment1Label,
                         m_curve2Segment2Label,
                         {GOType::CubicBezier, GOType::Spline});
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::AllowSplinePathSegmentHover(bool enabled)
{
    SetSegmentLabelHover(firstCurveId,
                         enabled,
                         m_curve1Segment1Label,
                         m_curve1Segment2Label,
                         {GOType::CubicBezierPath, GOType::SplinePath});
    SetSegmentLabelHover(secondCurveId,
                         enabled,
                         m_curve2Segment1Label,
                         m_curve2Segment2Label,
                         {GOType::CubicBezierPath, GOType::SplinePath});
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::ToolSelectionType(const SelectionType &selectionType)
{
    m_curve1Segment1Label->LabelSelectionType(selectionType);
    m_curve1Segment2Label->LabelSelectionType(selectionType);
    m_curve2Segment1Label->LabelSelectionType(selectionType);
    m_curve2Segment2Label->LabelSelectionType(selectionType);

    VToolSinglePoint::ToolSelectionType(selectionType);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetArcSegmentLabelVisible(bool visible)
{
    SetSegmentLabelVisible(firstCurveId,
                           visible,
                           segLableVisRefCount,
                           m_curve1Segment1Label,
                           m_curve1Segment2Label,
                           {GOType::Arc});
    SetSegmentLabelVisible(secondCurveId,
                           visible,
                           segLableVisRefCount,
                           m_curve2Segment1Label,
                           m_curve2Segment2Label,
                           {GOType::Arc});
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetElArcSegmentLabelVisible(bool visible)
{
    SetSegmentLabelVisible(firstCurveId,
                           visible,
                           segLableVisRefCount,
                           m_curve1Segment1Label,
                           m_curve1Segment2Label,
                           {GOType::EllipticalArc});
    SetSegmentLabelVisible(secondCurveId,
                           visible,
                           segLableVisRefCount,
                           m_curve2Segment1Label,
                           m_curve2Segment2Label,
                           {GOType::EllipticalArc});
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetSplineSegmentLabelVisible(bool visible)
{
    SetSegmentLabelVisible(firstCurveId,
                           visible,
                           segLableVisRefCount,
                           m_curve1Segment1Label,
                           m_curve1Segment2Label,
                           {GOType::CubicBezier, GOType::Spline});
    SetSegmentLabelVisible(secondCurveId,
                           visible,
                           segLableVisRefCount,
                           m_curve2Segment1Label,
                           m_curve2Segment2Label,
                           {GOType::CubicBezier, GOType::Spline});
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetSplinePathSegmentLabelVisible(bool visible)
{
    SetSegmentLabelVisible(firstCurveId,
                           visible,
                           segLableVisRefCount,
                           m_curve1Segment1Label,
                           m_curve1Segment2Label,
                           {GOType::CubicBezierPath, GOType::SplinePath});
    SetSegmentLabelVisible(secondCurveId,
                           visible,
                           segLableVisRefCount,
                           m_curve2Segment1Label,
                           m_curve2Segment2Label,
                           {GOType::CubicBezierPath, GOType::SplinePath});
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::AllowLabelSelecting(bool enabled)
{
    m_curve1Segment1Label->SetLabelSelectable(enabled);
    m_curve1Segment2Label->SetLabelSelectable(enabled);
    m_curve2Segment1Label->SetLabelSelectable(enabled);
    m_curve2Segment2Label->SetLabelSelectable(enabled);

    VToolSinglePoint::AllowLabelSelecting(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    auto *dialogTool = qobject_cast<DialogPointOfIntersectionCurves *>(m_dialog);
    SCASSERT(dialogTool != nullptr)

    doc->SetAttribute(domElement, AttrName, dialogTool->GetPointName());
    doc->SetAttribute(domElement, AttrCurve1, QString().setNum(dialogTool->GetFirstCurveId()));
    doc->SetAttribute(domElement, AttrCurve2, QString().setNum(dialogTool->GetSecondCurveId()));
    doc->SetAttribute(domElement, AttrVCrossPoint, QString().setNum(static_cast<int>(dialogTool->GetVCrossPoint())));
    doc->SetAttribute(domElement, AttrHCrossPoint, QString().setNum(static_cast<int>(dialogTool->GetHCrossPoint())));
    doc->SetAttribute(domElement, AttrCurve1Name1, dialogTool->GetCurve1Name1());
    doc->SetAttribute(domElement, AttrCurve1Name2, dialogTool->GetCurve1Name2());
    doc->SetAttribute(domElement, AttrCurve2Name1, dialogTool->GetCurve2Name1());
    doc->SetAttribute(domElement, AttrCurve2Name2, dialogTool->GetCurve2Name2());
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrCurve1Alias1, dialogTool->GetCurve1AliasSuffix1(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrCurve1Alias2, dialogTool->GetCurve1AliasSuffix2(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrCurve2Alias1, dialogTool->GetCurve2AliasSuffix1(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrCurve2Alias2, dialogTool->GetCurve2AliasSuffix2(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrNotes, dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrCurve1, firstCurveId);
    doc->SetAttribute(tag, AttrCurve2, secondCurveId);
    doc->SetAttribute(tag, AttrVCrossPoint, static_cast<int>(vCrossPoint));
    doc->SetAttribute(tag, AttrHCrossPoint, static_cast<int>(hCrossPoint));
    doc->SetAttribute(tag, AttrCurve1Name1, m_curve1Name1);
    doc->SetAttribute(tag, AttrCurve1Name2, m_curve1Name2);
    doc->SetAttribute(tag, AttrCurve2Name1, m_curve2Name1);
    doc->SetAttribute(tag, AttrCurve2Name2, m_curve2Name2);
    doc->SetAttributeOrRemoveIf<QString>(tag, AttrCurve1Alias1, m_curve1AliasSuffix1,
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(tag, AttrCurve1Alias2, m_curve1AliasSuffix2,
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(tag, AttrCurve2Alias1, m_curve2AliasSuffix1,
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(tag, AttrCurve2Alias2, m_curve2AliasSuffix2,
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttribute(tag, AttrSegment1Id, m_curve1Segment1Id);
    doc->SetAttribute(tag, AttrSegment2Id, m_curve1Segment2Id);
    doc->SetAttribute(tag, AttrSegment3Id, m_curve2Segment1Id);
    doc->SetAttribute(tag, AttrSegment4Id, m_curve2Segment2Id);

    const VAbstractValApplication *app = VAbstractValApplication::VApp();
    doc->SetAttribute(tag, AttrSegment1Mx, app->fromPixel(m_curve1Segment1Mx));
    doc->SetAttribute(tag, AttrSegment1My, app->fromPixel(m_curve1Segment1My));
    doc->SetAttribute(tag, AttrSegment2Mx, app->fromPixel(m_curve1Segment2Mx));
    doc->SetAttribute(tag, AttrSegment2My, app->fromPixel(m_curve1Segment2Mx));
    doc->SetAttribute(tag, AttrSegment3Mx, app->fromPixel(m_curve2Segment1Mx));
    doc->SetAttribute(tag, AttrSegment3My, app->fromPixel(m_curve2Segment1My));
    doc->SetAttribute(tag, AttrSegment4Mx, app->fromPixel(m_curve2Segment2Mx));
    doc->SetAttribute(tag, AttrSegment4My, app->fromPixel(m_curve2Segment2My));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::ReadToolAttributes(const QDomElement &domElement)
{
    VToolSinglePoint::ReadToolAttributes(domElement);

    firstCurveId = VAbstractPattern::GetParametrUInt(domElement, AttrCurve1, NULL_ID_STR);
    secondCurveId = VAbstractPattern::GetParametrUInt(domElement, AttrCurve2, NULL_ID_STR);
    vCrossPoint =
        static_cast<VCrossCurvesPoint>(VAbstractPattern::GetParametrUInt(domElement, AttrVCrossPoint, QChar('1')));
    hCrossPoint =
        static_cast<HCrossCurvesPoint>(VAbstractPattern::GetParametrUInt(domElement, AttrHCrossPoint, QChar('1')));
    m_curve1Name1 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurve1Name1);
    m_curve1Name2 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurve1Name2);
    m_curve2Name1 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurve2Name1);
    m_curve2Name2 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurve2Name2);
    m_curve1AliasSuffix1 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurve1Alias1);
    m_curve1AliasSuffix2 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurve1Alias2);
    m_curve2AliasSuffix1 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurve2Alias1);
    m_curve2AliasSuffix2 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurve2Alias2);
    m_curve1Segment1Id = VAbstractPattern::GetParametrId(domElement, AttrSegment1Id);
    m_curve1Segment2Id = VAbstractPattern::GetParametrId(domElement, AttrSegment2Id);
    m_curve2Segment1Id = VAbstractPattern::GetParametrId(domElement, AttrSegment3Id);
    m_curve2Segment2Id = VAbstractPattern::GetParametrId(domElement, AttrSegment4Id);

    const VAbstractValApplication *app = VAbstractValApplication::VApp();

    const QString labelMXStr = QString::number(app->fromPixel(labelMX));
    const QString labelMYStr = QString::number(app->fromPixel(labelMY));

    m_curve1Segment1Mx = app->toPixel(VAbstractPattern::GetParametrDouble(domElement, AttrSegment1Mx, labelMXStr));
    m_curve1Segment1My = app->toPixel(VAbstractPattern::GetParametrDouble(domElement, AttrSegment1My, labelMYStr));
    m_curve1Segment2Mx = app->toPixel(VAbstractPattern::GetParametrDouble(domElement, AttrSegment2Mx, labelMXStr));
    m_curve1Segment2My = app->toPixel(VAbstractPattern::GetParametrDouble(domElement, AttrSegment2My, labelMYStr));
    m_curve2Segment1Mx = app->toPixel(VAbstractPattern::GetParametrDouble(domElement, AttrSegment3Mx, labelMXStr));
    m_curve2Segment1My = app->toPixel(VAbstractPattern::GetParametrDouble(domElement, AttrSegment3My, labelMYStr));
    m_curve2Segment2Mx = app->toPixel(VAbstractPattern::GetParametrDouble(domElement, AttrSegment4Mx, labelMXStr));
    m_curve2Segment2My = app->toPixel(VAbstractPattern::GetParametrDouble(domElement, AttrSegment4My, labelMYStr));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolPointOfIntersectionCurves *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetCurve1Id(firstCurveId);
        visual->SetCurve2Id(secondCurveId);
        visual->SetVCrossPoint(vCrossPoint);
        visual->SetHCrossPoint(hCrossPoint);
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::MakeToolTip() const -> QString
{
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);

    QSharedPointer<VGObject> const curve1Segment1 = VAbstractTool::data.GetGObject(m_curve1Segment1Id);
    QSharedPointer<VGObject> const curve1Segment2 = VAbstractTool::data.GetGObject(m_curve1Segment2Id);
    QSharedPointer<VGObject> const curve2Segment1 = VAbstractTool::data.GetGObject(m_curve1Segment1Id);
    QSharedPointer<VGObject> const curve2Segment2 = VAbstractTool::data.GetGObject(m_curve1Segment2Id);

    const QString toolTip = u"<table>"
                            u"<tr> <td><b>%1:</b> %2</td> </tr>"
                            u"<tr> <td><b>%3:</b> %4</td> </tr>"
                            u"<tr> <td><b>%5:</b> %6</td> </tr>"
                            u"<tr> <td><b>%7:</b> %8</td> </tr>"
                            u"<tr> <td><b>%9:</b> %10</td> </tr>"
                            u"</table>"_s
                                .arg(tr("Label"),
                                     p->name(), /* 1, 2 */
                                     tr("Curve 1 segment 1"),
                                     curve1Segment1->ObjectName(), /* 3, 4 */
                                     tr("Curve 1 segment 2"),
                                     curve1Segment2->ObjectName()) /* 5, 6 */
                                .arg(tr("Curve 2 segment 1"),
                                     curve2Segment1->ObjectName(), /* 7, 8 */
                                     tr("Curve 2 segment 2"),
                                     curve2Segment2->ObjectName()); /* 9, 10 */

    return toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogPointOfIntersectionCurves> dialogTool = qobject_cast<DialogPointOfIntersectionCurves *>(
        m_dialog);
    SCASSERT(not dialogTool.isNull())

    ProcessToolOptions(oldDomElement, newDomElement, GatherToolChanges());
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant
{
    if (change == QGraphicsItem::ItemSelectedHasChanged && !m_selectedFromChild && segLableVisRefCount == 0)
    {
        {
            const QSignalBlocker blocker(m_curve1Segment1Label);
            m_curve1Segment1Label->SetLabelVisible(value.toBool());
            m_curve1Segment1Label->setSelected(value.toBool());
        }

        {
            const QSignalBlocker blocke(m_curve1Segment2Label);
            m_curve1Segment2Label->setSelected(value.toBool());
            m_curve1Segment2Label->SetLabelVisible(value.toBool());
        }

        {
            const QSignalBlocker blocker(m_curve2Segment1Label);
            m_curve2Segment1Label->SetLabelVisible(value.toBool());
            m_curve2Segment1Label->setSelected(value.toBool());
        }

        {
            const QSignalBlocker blocker(m_curve2Segment2Label);
            m_curve2Segment2Label->SetLabelVisible(value.toBool());
            m_curve2Segment2Label->setSelected(value.toBool());
        }
    }

    return VToolSinglePoint::itemChange(change, value);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::RefreshGeometry()
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

    UpdateSegmentLabel(m_curve1Segment1Id, m_curve1Segment1Mx, m_curve1Segment1My, m_curve1Segment1Label);
    UpdateSegmentLabel(m_curve1Segment2Id, m_curve1Segment2Mx, m_curve1Segment2My, m_curve1Segment2Label);
    UpdateSegmentLabel(m_curve2Segment1Id, m_curve2Segment1Mx, m_curve2Segment1My, m_curve2Segment1Label);
    UpdateSegmentLabel(m_curve2Segment2Id, m_curve2Segment2Mx, m_curve2Segment2My, m_curve2Segment2Label);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SegmentChoosed(quint32 id, SceneObject type)
{
    emit ChoosedTool(id, type);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::Curve1Segment1LabelPositionChanged(const QPointF &pos)
{
    VAbstractApplication::VApp()->getUndoStack()->push(new MoveSegmentLabel(doc, pos, m_id, SegmentLabel::Segment1));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::Curve1Segment2LabelPositionChanged(const QPointF &pos)
{
    VAbstractApplication::VApp()->getUndoStack()->push(new MoveSegmentLabel(doc, pos, m_id, SegmentLabel::Segment2));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::Curve2Segment1LabelPositionChanged(const QPointF &pos)
{
    VAbstractApplication::VApp()->getUndoStack()->push(new MoveSegmentLabel(doc, pos, m_id, SegmentLabel::Segment3));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::Curve2Segment2LabelPositionChanged(const QPointF &pos)
{
    VAbstractApplication::VApp()->getUndoStack()->push(new MoveSegmentLabel(doc, pos, m_id, SegmentLabel::Segment4));
}
