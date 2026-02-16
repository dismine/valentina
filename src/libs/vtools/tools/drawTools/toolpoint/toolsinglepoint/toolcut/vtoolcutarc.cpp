/************************************************************************
 **
 **  @file   vtoolcutarc.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 1, 2014
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

#include "vtoolcutarc.h"

#include <QPointF>
#include <QSharedPointer>

#include "../../../../../dialogs/tools/dialogcutarc.h"
#include "../../../../../dialogs/tools/dialogtool.h"
#include "../../../../../visualization/path/vistoolcutarc.h"
#include "../../../../../visualization/visualization.h"
#include "../../../../vabstracttool.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vgeometry/vabstractarc.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vpointf.h"
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

const QString VToolCutArc::ToolType = QStringLiteral("cutArc");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolCutArc constructor.
 * @param initData init data.
 * @param parent parent object.
 */
VToolCutArc::VToolCutArc(const VToolCutInitData &initData, QGraphicsItem *parent)
  : VToolCut(initData, parent)
{
    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCutArc::GatherToolChanges() const -> VToolCut::ToolChanges
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogCutArc> dialogTool = qobject_cast<DialogCutArc *>(m_dialog);
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
void VToolCutArc::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogCutArc> dialogTool = qobject_cast<DialogCutArc *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetFormula(formula);
    dialogTool->setArcId(baseCurveId);
    dialogTool->SetPointName(point->name());
    dialogTool->SetNotes(m_notes);
    dialogTool->SetAliasSuffix1(m_aliasSuffix1);
    dialogTool->SetAliasSuffix2(m_aliasSuffix2);
    dialogTool->SetName1(GetName1());
    dialogTool->SetName2(GetName2());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool form GUI.
 * @param dialog dialog.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 */
auto VToolCutArc::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                         VContainer *data) -> VToolCutArc *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogCutArc> dialogTool = qobject_cast<DialogCutArc *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolCutInitData initData;
    initData.formula = dialogTool->GetFormula();
    initData.baseCurveId = dialogTool->getArcId();
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

    VToolCutArc *point = Create(initData);
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
auto VToolCutArc::Create(VToolCutInitData &initData) -> VToolCutArc *
{
    const QSharedPointer<VAbstractArc> arc = initData.data->GeometricObject<VAbstractArc>(initData.baseCurveId);

    // Declare special variable "CurrentLength"
    auto *length =
        new VCurveLength(initData.baseCurveId, initData.baseCurveId, arc.data(), *initData.data->GetPatternUnit());
    length->SetName(currentLength);
    initData.data->AddVariable(length);

    const qreal result = CheckFormula(initData.id, initData.formula, initData.data);

    QPointF cutPoint;
    QSharedPointer<VAbstractArc> a1;
    QSharedPointer<VAbstractArc> a2;

    if (arc->getType() == GOType::Arc)
    {
        VArc arc1;
        VArc arc2;
        cutPoint = arc->CutArc(VAbstractValApplication::VApp()->toPixel(result), &arc1, &arc2, initData.name);

        a1 = QSharedPointer<VArc>(new VArc(arc1));
        a2 = QSharedPointer<VArc>(new VArc(arc2));
    }
    else if (arc->getType() == GOType::EllipticalArc)
    {
        VEllipticalArc arc1;
        VEllipticalArc arc2;
        cutPoint = arc->CutArc(VAbstractValApplication::VApp()->toPixel(result), &arc1, &arc2, initData.name);

        a1 = QSharedPointer<VEllipticalArc>(new VEllipticalArc(arc1));
        a2 = QSharedPointer<VEllipticalArc>(new VEllipticalArc(arc2));
    }

    a1->SetDerivative(true);
    a2->SetDerivative(true);

    a1->SetAliasSuffix(initData.aliasSuffix1);
    a2->SetAliasSuffix(initData.aliasSuffix2);

    // These checks can be removed since name1 and name2 no longer should be empty
    Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
    if (initData.name1.isEmpty())
    {
        initData.name1 = a1->HeadlessName();
    }

    if (initData.name2.isEmpty())
    {
        initData.name2 = a2->HeadlessName();
    }

    a1->SetNameSuffix(initData.name1);
    a2->SetNameSuffix(initData.name2);

    auto *p = new VPointF(cutPoint, initData.name, initData.mx, initData.my);
    p->SetShowLabel(initData.showLabel);

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->AddVertex(initData.id, VNodeType::TOOL, initData.doc->PatternBlockMapper()->GetActiveId());

    const auto varData = initData.data->DataDependencyVariables();
    initData.doc->FindFormulaDependencies(initData.formula, initData.id, varData);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->AddGObject(p);

        a1->setId(initData.data->getNextId());
        initData.data->RegisterUniqueName(a1);
        initData.data->AddArc(a1, NULL_ID, initData.id);

        a2->setId(initData.data->getNextId());
        initData.data->RegisterUniqueName(a2);
        initData.data->AddArc(a2, NULL_ID, initData.id);
    }
    else
    {
        initData.data->UpdateGObject(initData.id, p);

        a1->setId(initData.id + 1);
        initData.data->RegisterUniqueName(a1);
        initData.data->AddArc(a1, NULL_ID, initData.id);

        a2->setId(initData.id + 2);
        initData.data->RegisterUniqueName(a2);
        initData.data->AddArc(a2, NULL_ID, initData.id);
    }

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

    VToolCutArc *tool = nullptr;
    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::CutArc, initData.doc);
        tool = new VToolCutArc(initData);
        initData.scene->addItem(tool);
        InitToolConnections(initData.scene, tool);
        VAbstractPattern::AddTool(initData.id, tool);
    }
    // Very important to delete it. Only this tool need this special variable.
    initData.data->RemoveVariable(currentLength);
    return tool;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutArc::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolCutArc>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutArc::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogCutArc>(event, id);
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
void VToolCutArc::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogCutArc> dialogTool = qobject_cast<DialogCutArc *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrName, dialogTool->GetPointName());
    doc->SetAttribute(domElement, AttrLength, dialogTool->GetFormula());
    doc->SetAttribute(domElement, AttrArc, QString().setNum(dialogTool->getArcId()));
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

    const QString notes = dialogTool->GetNotes();
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrNotes,
                                         notes,
                                         [](const QString &notes) noexcept -> bool { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutArc::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolCut::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrLength, formula);
    doc->SetAttribute(tag, AttrArc, baseCurveId);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutArc::ReadToolAttributes(const QDomElement &domElement)
{
    VToolCut::ReadToolAttributes(domElement);

    formula = VDomDocument::GetParametrString(domElement, AttrLength, QString());
    baseCurveId = VAbstractPattern::GetParametrUInt(domElement, AttrArc, NULL_ID_STR);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutArc::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolCutArc *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetArcId(baseCurveId);
        visual->SetLength(VAbstractApplication::VApp()->TrVars()->FormulaToUser(
            formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator()));

        const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(baseCurveId);
        visual->SetLineStyle(LineStyleToPenStyle(curve->GetPenStyle()));
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCutArc::MakeToolTip() const -> QString
{
    const QSharedPointer<VAbstractArc> arc = VAbstractTool::data.GeometricObject<VAbstractArc>(baseCurveId);

    const QString expression = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    const qreal length = Visualization::FindValFromUser(expression, VAbstractTool::data.DataVariables());

    const QString arcStr = QCoreApplication::translate("VToolCutArc", "Arc");
    const QString elArcStr = QCoreApplication::translate("VToolCutArc", "Elliptical arc");
    const QString lengthStr = QCoreApplication::translate("VToolCutArc", "length");
    const QString startAngleStr = QCoreApplication::translate("VToolCutArc", "start angle");
    const QString endAngleStr = QCoreApplication::translate("VToolCutArc", "end angle");
    const QString radiusStr = QCoreApplication::translate("VToolCutArc", "radius");
    const QString labelStr = QCoreApplication::translate("VToolCutArc", "label");
    const QString radius1Str = QCoreApplication::translate("VToolCutArc", "radius1");
    const QString radius2Str = QCoreApplication::translate("VToolCutArc", "radius2");

    const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(m_id);

    if (arc->getType() == GOType::Arc)
    {
        auto ArcToolTip =
            [arcStr, lengthStr, startAngleStr, endAngleStr, radiusStr, labelStr](QString toolTip,
                                                                                 const VArc &arc,
                                                                                 const QString &arcNumber) -> QString
        {
            toolTip += u"<tr> <td><b>%10:</b> %11</td> </tr>"
                       u"<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                       u"<tr> <td><b>%4:</b> %5 %3</td> </tr>"
                       u"<tr> <td><b>%6:</b> %7°</td> </tr>"
                       u"<tr> <td><b>%8:</b> %9°</td> </tr>"_s
                           .arg(arcStr + arcNumber + " "_L1 + lengthStr)                           // %1
                           .arg(VAbstractValApplication::VApp()->fromPixel(arc.GetLength()))       // %2
                           .arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true), // %3
                                arcStr + arcNumber + " "_L1 + radiusStr)                           // %4
                           .arg(arc.GetRadius())                                                   // %5
                           .arg(arcStr + arcNumber + " "_L1 + startAngleStr)                       // %6
                           .arg(arc.GetStartAngle())                                               // %7
                           .arg(arcStr + arcNumber + " "_L1 + endAngleStr)                         // %8
                           .arg(arc.GetEndAngle())                                                 // %9
                           .arg(arcStr + arcNumber + " "_L1 + labelStr,                            // %10
                                arc.ObjectName());                                                 // %11
            return toolTip;
        };

        VArc ar1;
        VArc ar2;
        arc->CutArc(VAbstractValApplication::VApp()->toPixel(length), &ar1, &ar2, point->name());

        ar1.setId(m_id + 1);
        ar1.SetNameSuffix(m_name1);
        ar1.SetAliasSuffix(m_aliasSuffix1);

        ar2.setId(m_id + 2);
        ar2.SetNameSuffix(m_name2);
        ar2.SetAliasSuffix(m_aliasSuffix2);

        return ArcToolTip(ArcToolTip(QStringLiteral("<table>"), ar1, QChar('1')), ar2, QChar('2'))
               + QStringLiteral("</table>");
    }

    if (arc->getType() == GOType::EllipticalArc)
    {
        auto ElArcToolTip = [elArcStr,
                             lengthStr,
                             startAngleStr,
                             endAngleStr,
                             radius1Str,
                             radius2Str,
                             labelStr](QString toolTip, const VEllipticalArc &arc, const QString &arcNumber) -> QString
        {
            toolTip += u"<tr> <td><b>%12:</b> %13</td> </tr>"
                       u"<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                       u"<tr> <td><b>%4:</b> %5 %3</td> </tr>"
                       u"<tr> <td><b>%6:</b> %7 %3</td> </tr>"
                       u"<tr> <td><b>%8:</b> %9°</td> </tr>"
                       u"<tr> <td><b>%10:</b> %11°</td> </tr>"_s
                           .arg(elArcStr + arcNumber + QChar(QChar::Space) + lengthStr)            // %1
                           .arg(VAbstractValApplication::VApp()->fromPixel(arc.GetLength()))       // %2
                           .arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true), // %3
                                elArcStr + arcNumber + QChar(QChar::Space) + radius1Str)           // %4
                           .arg(VAbstractValApplication::VApp()->fromPixel(arc.GetRadius1()))      // %5
                           .arg(elArcStr + arcNumber + QChar(QChar::Space) + radius2Str)           // %6
                           .arg(VAbstractValApplication::VApp()->fromPixel(arc.GetRadius2()))      // %7
                           .arg(elArcStr + arcNumber + QChar(QChar::Space) + startAngleStr)        // %8
                           .arg(arc.GetStartAngle())                                               // %9
                           .arg(elArcStr + arcNumber + QChar(QChar::Space) + endAngleStr)          // %10
                           .arg(arc.GetEndAngle())                                                 // %11
                           .arg(elArcStr + arcNumber + QChar(QChar::Space) + labelStr,             // %12
                                arc.ObjectName());                                                 // %13
            return toolTip;
        };

        VEllipticalArc ar1;
        VEllipticalArc ar2;
        arc->CutArc(VAbstractValApplication::VApp()->toPixel(length), &ar1, &ar2, point->name());

        ar1.setId(m_id + 1);
        ar1.SetNameSuffix(m_name1);
        ar1.SetAliasSuffix(m_aliasSuffix1);

        ar2.setId(m_id + 2);
        ar2.SetNameSuffix(m_name2);
        ar2.SetAliasSuffix(m_aliasSuffix2);

        return ElArcToolTip(ElArcToolTip(QStringLiteral("<table>"), ar1, QChar('1')), ar2, QChar('2'))
               + QStringLiteral("</table>");
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutArc::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    ProcessToolCutOptions(oldDomElement, newDomElement, GatherToolChanges());
}
