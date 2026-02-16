/************************************************************************
 **
 **  @file   vtoolcutspline.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 12, 2013
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

#include "vtoolcutspline.h"

#include <QPointF>
#include <QSharedPointer>

#include "../../../../../dialogs/tools/dialogcutspline.h"
#include "../../../../../dialogs/tools/dialogtool.h"
#include "../../../../../visualization/path/vistoolcutspline.h"
#include "../../../../../visualization/visualization.h"
#include "../../../../vabstracttool.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vgeometry/vabstractcubicbezier.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vmisc/exception/vexception.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/variables/vcurvelength.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vtoolcut.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

template <class T> class QSharedPointer;

const QString VToolCutSpline::ToolType = QStringLiteral("cutSpline"); // NOLINT(cert-err58-cpp)
const QString VToolCutSpline::AttrSpline = QStringLiteral("spline");  // NOLINT(cert-err58-cpp)

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolCutSpline constructor.
 * @param initData init data.
 * @param parent parent object.
 */
VToolCutSpline::VToolCutSpline(const VToolCutInitData &initData, QGraphicsItem *parent)
  : VToolCut(initData, parent)
{
    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCutSpline::GatherToolChanges() const -> VToolCut::ToolChanges
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogCutSpline> dialogTool = qobject_cast<DialogCutSpline *>(m_dialog);
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
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolCutSpline::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    QPointer<DialogCutSpline> const dialogTool = qobject_cast<DialogCutSpline *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetFormula(formula);
    dialogTool->setSplineId(baseCurveId);
    dialogTool->SetPointName(point->name());
    dialogTool->SetNotes(m_notes);
    dialogTool->SetAliasSuffix1(m_aliasSuffix1);
    dialogTool->SetAliasSuffix2(m_aliasSuffix2);
    dialogTool->SetName1(GetName1());
    dialogTool->SetName2(GetName2());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool from GUI.
 * @param dialog dialog.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 */
auto VToolCutSpline::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                            VContainer *data) -> VToolCutSpline *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogCutSpline> dialogTool = qobject_cast<DialogCutSpline *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolCutInitData initData;
    initData.formula = dialogTool->GetFormula();
    initData.baseCurveId = dialogTool->getSplineId();
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

    VToolCutSpline *point = Create(initData);
    if (point != nullptr)
    {
        point->m_dialog = dialog;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool.
 * @param initData init data.
 */
auto VToolCutSpline::Create(VToolCutInitData &initData) -> VToolCutSpline *
{
    const auto spl = initData.data->GeometricObject<VAbstractCubicBezier>(initData.baseCurveId);

    // Declare special variable "CurrentLength"
    auto *length =
        new VCurveLength(initData.baseCurveId, initData.baseCurveId, spl.data(), *initData.data->GetPatternUnit());
    length->SetName(currentLength);
    initData.data->AddVariable(length);

    const qreal result = CheckFormula(initData.id, initData.formula, initData.data);

    QPointF spl1p2;
    QPointF spl1p3;
    QPointF spl2p2;
    QPointF spl2p3;
    QPointF const point =
        spl->CutSpline(VAbstractValApplication::VApp()->toPixel(result), spl1p2, spl1p3, spl2p2, spl2p3, initData.name);

    auto *p = new VPointF(point, initData.name, initData.mx, initData.my);
    p->SetShowLabel(initData.showLabel);

    auto spline1 = QSharedPointer<VAbstractBezier>(new VSpline(spl->GetP1(), spl1p2, spl1p3, *p));
    auto spline2 = QSharedPointer<VAbstractBezier>(new VSpline(*p, spl2p2, spl2p3, spl->GetP4()));

    spline1->SetDerivative(true);
    spline2->SetDerivative(true);

    spline1->SetApproximationScale(spl->GetApproximationScale());
    spline2->SetApproximationScale(spl->GetApproximationScale());

    spline1->SetAliasSuffix(initData.aliasSuffix1);
    spline2->SetAliasSuffix(initData.aliasSuffix2);

    // These checks can be removed since name1 and name2 no longer should be empty
    Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
    if (initData.name1.isEmpty())
    {
        initData.name1 = spline1->HeadlessName();
    }

    if (initData.name2.isEmpty())
    {
        initData.name2 = spline2->HeadlessName();
    }

    spline1->SetNameSuffix(initData.name1);
    spline2->SetNameSuffix(initData.name2);

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
    initData.doc->FindFormulaDependencies(initData.formula, initData.id, varData);

    initData.data->AddSpline(spline1, NULL_ID, initData.id);
    initData.data->AddSpline(spline2, NULL_ID, initData.id);

    initData.data->RegisterUniqueName(spline1);
    initData.data->RegisterUniqueName(spline2);

    // TODO: Add segments to graph when we start showing them for users
    // patternGraph->AddVertex(initData.segment1Id, VNodeType::OBJECT, initData.doc->PatternBlockMapper()->GetActiveId());
    // patternGraph->AddVertex(initData.segment2Id, VNodeType::OBJECT, initData.doc->PatternBlockMapper()->GetActiveId());

    // patternGraph->AddEdge(initData.id, initData.segment1Id);
    // patternGraph->AddEdge(initData.id, initData.segment2Id);

    patternGraph->AddEdge(initData.baseCurveId, initData.id);

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    VToolCutSpline *tool = nullptr;
    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::CutSpline, initData.doc);
        tool = new VToolCutSpline(initData);
        initData.scene->addItem(tool);
        InitToolConnections(initData.scene, tool);
        VAbstractPattern::AddTool(initData.id, tool);
    }
    // Very important to delete it. Only this tool need this special variable.
    initData.data->RemoveVariable(currentLength);
    return tool;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutSpline::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolCutSpline>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutSpline::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogCutSpline>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolCutSpline::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogCutSpline> dialogTool = qobject_cast<DialogCutSpline *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrName, dialogTool->GetPointName());
    doc->SetAttribute(domElement, AttrLength, dialogTool->GetFormula());
    doc->SetAttribute(domElement, AttrSpline, QString().setNum(dialogTool->getSplineId()));
    doc->SetAttribute(domElement, AttrCurveName1, dialogTool->GetName1());
    doc->SetAttribute(domElement, AttrCurveName2, dialogTool->GetName2());
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
void VToolCutSpline::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolCut::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrLength, formula);
    doc->SetAttribute(tag, AttrSpline, baseCurveId);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutSpline::ReadToolAttributes(const QDomElement &domElement)
{
    VToolCut::ReadToolAttributes(domElement);

    formula = VDomDocument::GetParametrString(domElement, AttrLength, QString());
    baseCurveId = VDomDocument::GetParametrUInt(domElement, AttrSpline, NULL_ID_STR);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutSpline::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolCutSpline *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetSplineId(baseCurveId);
        visual->SetLength(VAbstractApplication::VApp()->TrVars()->FormulaToUser(
            formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator()));

        const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(baseCurveId);
        visual->SetLineStyle(LineStyleToPenStyle(curve->GetPenStyle()));
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCutSpline::MakeToolTip() const -> QString
{
    const auto spl = VAbstractTool::data.GeometricObject<VAbstractCubicBezier>(baseCurveId);

    const QString expression = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    const qreal length = Visualization::FindValFromUser(expression, VAbstractTool::data.DataVariables());

    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);

    QPointF spl1p2;
    QPointF spl1p3;
    QPointF spl2p2;
    QPointF spl2p3;
    QPointF const point =
        spl->CutSpline(VAbstractValApplication::VApp()->toPixel(length), spl1p2, spl1p3, spl2p2, spl2p3, p->name());

    auto spline1 = VSpline(spl->GetP1(), spl1p2, spl1p3, VPointF(point));
    spline1.SetNameSuffix(m_name1);
    spline1.SetAliasSuffix(m_aliasSuffix1);

    auto spline2 = VSpline(VPointF(point), spl2p2, spl2p3, spl->GetP4());
    spline2.SetNameSuffix(m_name2);
    spline2.SetAliasSuffix(m_aliasSuffix2);

    const QString curveStr = QCoreApplication::translate("VToolCutSpline", "Curve");
    const QString lengthStr = QCoreApplication::translate("VToolCutSpline", "length");

    const QString toolTip = u"<table>"
                            u"<tr> <td><b>%6:</b> %7</td> </tr>"
                            u"<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                            u"<tr> <td><b>%8:</b> %9</td> </tr>"
                            u"<tr> <td><b>%4:</b> %5 %3</td> </tr>"
                            u"</table>"_s.arg(curveStr + " 1 "_L1 + lengthStr)
                                .arg(VAbstractValApplication::VApp()->fromPixel(spline1.GetLength()))
                                .arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true),
                                     curveStr + " 2 "_L1 + lengthStr)
                                .arg(VAbstractValApplication::VApp()->fromPixel(spline2.GetLength()))
                                .arg(curveStr + " 1 "_L1 + QCoreApplication::translate("VToolCutSpline", "label"),
                                     spline1.ObjectName(),
                                     curveStr + " 2 "_L1 + QCoreApplication::translate("VToolCutSpline", "label"),
                                     spline2.ObjectName());

    return toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutSpline::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    ProcessToolCutOptions(oldDomElement, newDomElement, GatherToolChanges());
}
