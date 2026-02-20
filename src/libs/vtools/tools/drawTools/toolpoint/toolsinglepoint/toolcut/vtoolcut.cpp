/************************************************************************
 **
 **  @file   vtoolcut.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 6, 2014
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

#include "vtoolcut.h"

#include <QDomElement>
#include <QSharedPointer>
#include <QUndoStack>

#include "../../../../../undocommands/renameobject.h"
#include "../../../../../undocommands/savetooloptions.h"
#include "../../../../vabstracttool.h"
#include "../../../vdrawtool.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vdomdocument.h"
#include "../qmuparser/qmudef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vformula.h"
#include "../vtoolsinglepoint.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
VToolCut::VToolCut(const VToolCutInitData &initData, QGraphicsItem *parent)
  : VToolSinglePoint(initData.doc, initData.data, initData.id, initData.notes, parent),
    formula(initData.formula),
    baseCurveId(initData.baseCurveId),
    detailsMode(VAbstractApplication::VApp()->Settings()->IsShowCurveDetails()),
    m_name1(initData.name1),
    m_name2(initData.name2),
    m_aliasSuffix1(initData.aliasSuffix1),
    m_aliasSuffix2(initData.aliasSuffix2)
{
    Q_ASSERT_X(initData.baseCurveId != 0, Q_FUNC_INFO, "curveCutId == 0"); //-V654 //-V712
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::SetDetailsMode(bool mode)
{
    detailsMode = mode;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FullUpdateFromFile update tool data form file.
 */
void VToolCut::FullUpdateFromFile()
{
    ReadAttributes();
    RefreshGeometry();
    SetVisualization();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::ProcessToolCutOptions(const QDomElement &oldDomElement,
                                     const QDomElement &newDomElement,
                                     const ToolChanges &changes)
{
    if (!changes.HasChanges())
    {
        VToolCut::ApplyToolOptions(oldDomElement, newDomElement);
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
        auto *renameLabel = new RenameLabel(changes.oldLabel, changes.newLabel, doc, m_id, newGroup);
        if (!changes.Name1Changed() && !changes.Name2Changed() && !changes.AliasSuffix1Changed()
            && !changes.AliasSuffix2Changed())
        {
            connect(renameLabel, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(baseCurveId);
    const CurveAliasType curveType = RenameAlias::CurveType(curve->getType());

    const quint32 subSpl1Id = m_id /*+ 1*/;
    const quint32 subSpl2Id = m_id /*+ 2*/;

    if (changes.Name1Changed())
    {
        auto *renameName = new RenameAlias(curveType, changes.oldName1, changes.newName1, doc, subSpl1Id, newGroup);
        if (!changes.Name2Changed() && !changes.AliasSuffix1Changed() && !changes.AliasSuffix2Changed())
        {
            connect(renameName, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    if (changes.Name2Changed())
    {
        auto *renameName = new RenameAlias(curveType, changes.oldName2, changes.newName2, doc, subSpl2Id, newGroup);
        if (!changes.AliasSuffix1Changed() && !changes.AliasSuffix2Changed())
        {
            connect(renameName, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    if (changes.AliasSuffix1Changed())
    {
        auto *renameAlias
            = new RenameAlias(curveType, changes.oldAliasSuffix1, changes.newAliasSuffix1, doc, subSpl1Id, newGroup);
        if (!changes.AliasSuffix2Changed())
        {
            connect(renameAlias, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    if (changes.AliasSuffix2Changed())
    {
        auto *renameAlias
            = new RenameAlias(curveType, changes.oldAliasSuffix1, changes.newAliasSuffix1, doc, subSpl2Id, newGroup);
        connect(renameAlias, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    }

    undoStack->push(newGroup);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCut::GetFormulaLength() const -> VFormula
{
    VFormula val(formula, getData());
    val.setToolId(m_id);
    val.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    val.Eval();
    return val;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::SetFormulaLength(const VFormula &value)
{
    if (!value.error())
    {
        formula = value.GetFormula(FormulaType::FromUser);

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCut::GetName1() const -> QString
{
    return m_name1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::SetName1(const QString &name)
{
    UpdateNameField(VToolCutNameField::Name1, name);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCut::GetName2() const -> QString
{
    return m_name2;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::SetName2(const QString &name)
{
    UpdateNameField(VToolCutNameField::Name2, name);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCut::GetAliasSuffix1() const -> QString
{
    return m_aliasSuffix1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::SetAliasSuffix1(const QString &alias)
{
    UpdateNameField(VToolCutNameField::AliasSuffix1, alias);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCut::GetAliasSuffix2() const -> QString
{
    return m_aliasSuffix2;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::SetAliasSuffix2(const QString &alias)
{
    UpdateNameField(VToolCutNameField::AliasSuffix2, alias);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCut::CurveName() const -> QString
{
    return VAbstractTool::data.GetGObject(baseCurveId)->ObjectName();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RefreshGeometry  refresh item on scene.
 */
void VToolCut::RefreshGeometry()
{
    VToolSinglePoint::RefreshPointGeometry(*VDrawTool::data.GeometricObject<VPointF>(m_id));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrCurveName1, m_name1);
    doc->SetAttribute(tag, AttrCurveName2, m_name2);
    doc->SetAttributeOrRemoveIf<QString>(tag,
                                         AttrAlias1,
                                         m_aliasSuffix1,
                                         [](const QString &suffix) noexcept -> bool { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(tag,
                                         AttrAlias2,
                                         m_aliasSuffix2,
                                         [](const QString &suffix) noexcept -> bool { return suffix.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::ReadToolAttributes(const QDomElement &domElement)
{
    VToolSinglePoint::ReadToolAttributes(domElement);

    m_name1 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurveName1);
    m_name2 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurveName2);
    m_aliasSuffix1 = VAbstractPattern::GetParametrEmptyString(domElement, AttrAlias1);
    m_aliasSuffix2 = VAbstractPattern::GetParametrEmptyString(domElement, AttrAlias2);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::UpdateNameField(VToolCutNameField field, const QString &value)
{
    // Validation - name fields require non-empty values
    if ((field == VToolCutNameField::Name1 || field == VToolCutNameField::Name2) && value.isEmpty())
    {
        return; // Name is required
    }

    // Validate format and uniqueness for non-empty values
    if (!value.isEmpty())
    {
        QSharedPointer<VAbstractCurve> const curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(baseCurveId);
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
        if (HasConflict(value, field))
        {
            return; // Conflicts with other identifiers
        }
    }

    QDomElement const oldDomElement = doc->FindElementById(m_id, getTagName());
    if (!oldDomElement.isElement())
    {
        qCDebug(vTool, "Can't find tool with id = %u", m_id);
        return;
    }

    const QString label = VAbstractTool::data.GeometricObject<VPointF>(m_id)->name();

    // Build changes struct
    ToolChanges const changes = {.oldLabel = label,
                                 .newLabel = label,
                                 .oldName1 = m_name1,
                                 .newName1 = (field == VToolCutNameField::Name1) ? value : m_name1,
                                 .oldName2 = m_name2,
                                 .newName2 = (field == VToolCutNameField::Name2) ? value : m_name2,
                                 .oldAliasSuffix1 = m_aliasSuffix1,
                                 .newAliasSuffix1 = (field == VToolCutNameField::AliasSuffix1) ? value : m_aliasSuffix1,
                                 .oldAliasSuffix2 = m_aliasSuffix2,
                                 .newAliasSuffix2 = (field == VToolCutNameField::AliasSuffix2) ? value : m_aliasSuffix2};

    // Update the appropriate member variable
    switch (field)
    {
        case VToolCutNameField::Name1:
            m_name1 = value;
            break;
        case VToolCutNameField::Name2:
            m_name2 = value;
            break;
        case VToolCutNameField::AliasSuffix1:
            m_aliasSuffix1 = value;
            break;
        case VToolCutNameField::AliasSuffix2:
            m_aliasSuffix2 = value;
            break;
        default:
            break;
    }

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    QDomElement newDomElement = oldDomElement.cloneNode().toElement();
    SaveOptions(newDomElement, obj);
    ProcessToolCutOptions(oldDomElement, newDomElement, changes);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCut::HasConflict(const QString &value, VToolCutNameField currentField) const -> bool
{
    if (value.isEmpty())
    {
        return false;
    }

    // Check name fields
    if (currentField != VToolCutNameField::Name1 && value == m_name1)
    {
        return true;
    }
    if (currentField != VToolCutNameField::Name2 && value == m_name2)
    {
        return true;
    }

    // Check non-empty alias fields
    if (currentField != VToolCutNameField::AliasSuffix1 && !m_aliasSuffix1.isEmpty() && value == m_aliasSuffix1)
    {
        return true;
    }
    if (currentField != VToolCutNameField::AliasSuffix2 && !m_aliasSuffix2.isEmpty() && value == m_aliasSuffix2)
    {
        return true;
    }

    return false;
}
