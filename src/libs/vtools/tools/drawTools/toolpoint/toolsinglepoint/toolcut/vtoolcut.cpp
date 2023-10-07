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
#include <new>

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
    m_aliasSuffix1(initData.aliasSuffix1),
    m_aliasSuffix2(initData.aliasSuffix2)
{
    Q_ASSERT_X(initData.baseCurveId != 0, Q_FUNC_INFO, "curveCutId == 0"); //-V654 //-V712
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::Disable(bool disable, const QString &namePP)
{
    VToolSinglePoint::Disable(disable, namePP);
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
auto VToolCut::GetFormulaLength() const -> VFormula
{
    VFormula val(formula, getData());
    val.setCheckZero(true);
    val.setToolId(m_id);
    val.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    val.Eval();
    return val;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::SetFormulaLength(const VFormula &value)
{
    if (value.error() == false)
    {
        formula = value.GetFormula(FormulaType::FromUser);

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCut::GetAliasSuffix1() const -> QString
{
    return m_aliasSuffix1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::SetAliasSuffix1(QString alias)
{
    QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(baseCurveId);

    const QString oldAliasSuffix = curve->GetAliasSuffix();
    alias = alias.simplified().replace(QChar(QChar::Space), '_'_L1);
    curve->SetAliasSuffix(alias);

    QRegularExpression rx(NameRegExp());

    if (alias.isEmpty() || (rx.match(curve->GetAlias()).hasMatch() && VAbstractTool::data.IsUnique(curve->GetAlias())))
    {
        m_aliasSuffix1 = alias;
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
    else
    {
        curve->SetAliasSuffix(oldAliasSuffix);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCut::GetAliasSuffix2() const -> QString
{
    return m_aliasSuffix2;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::SetAliasSuffix2(QString alias)
{
    QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(baseCurveId);

    const QString oldAliasSuffix = curve->GetAliasSuffix();
    alias = alias.simplified().replace(QChar(QChar::Space), '_'_L1);
    curve->SetAliasSuffix(alias);

    QRegularExpression rx(NameRegExp());

    if (alias.isEmpty() || (rx.match(curve->GetAlias()).hasMatch() && VAbstractTool::data.IsUnique(curve->GetAlias())))
    {
        m_aliasSuffix2 = alias;
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
    else
    {
        curve->SetAliasSuffix(oldAliasSuffix);
    }
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
/**
 * @brief RemoveReferens decrement referens value for used objects.
 */
void VToolCut::RemoveReferens()
{
    const auto curve = VAbstractTool::data.GetGObject(baseCurveId);

    doc->DecrementReferens(curve->getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    doc->SetAttributeOrRemoveIf<QString>(tag, AttrAlias1, m_aliasSuffix1,
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(tag, AttrAlias2, m_aliasSuffix2,
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCut::ReadToolAttributes(const QDomElement &domElement)
{
    VToolSinglePoint::ReadToolAttributes(domElement);

    m_aliasSuffix1 = VAbstractPattern::GetParametrEmptyString(domElement, AttrAlias1);
    m_aliasSuffix2 = VAbstractPattern::GetParametrEmptyString(domElement, AttrAlias2);
}
