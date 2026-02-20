/************************************************************************
 **
 **  @file   vtoolellipticalarcwithlength.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 10, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
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
#include "vtoolellipticalarcwithlength.h"

#include "../../../dialogs/tools/dialogellipticalarcwithlength.h"
#include "../../../dialogs/tools/dialogtool.h"
#include "../../../visualization/path/vistoolellipticalarcwithlength.h"
#include "../../../visualization/visualization.h"
#include "../../vabstracttool.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vdomdocument.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/exception/vexception.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vformula.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vmaingraphicsscene.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

const QString VToolEllipticalArcWithLength::ToolType = QStringLiteral("ellipticalArcWithLength");

//---------------------------------------------------------------------------------------------------------------------
VToolEllipticalArcWithLength::VToolEllipticalArcWithLength(const VToolEllipticalArcWithLengthInitData &initData,
                                                           QGraphicsItem *parent)
  : VToolAbstractArc(initData.doc, initData.data, initData.id, initData.notes, parent)
{
    SetSceneType(SceneObject::ElArc);

    this->setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus

    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolEllipticalArcWithLength::GatherToolChanges() const -> VToolAbstractArc::ToolChanges
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogEllipticalArcWithLength> dialogTool = qobject_cast<DialogEllipticalArcWithLength *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    const QSharedPointer<VAbstractArc> arc = VAbstractTool::data.GeometricObject<VAbstractArc>(m_id);

    return {.oldCenterLabel = CenterPointName(),
            .newCenterLabel = VAbstractTool::data.GetGObject(dialogTool->GetCenter())->name(),
            .oldAliasSuffix = arc->GetAliasSuffix(),
            .newAliasSuffix = dialogTool->GetAliasSuffix()};
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArcWithLength::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogEllipticalArcWithLength> dialogTool = qobject_cast<DialogEllipticalArcWithLength *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VEllipticalArc> elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetCenter(elArc->GetCenter().id());
    dialogTool->SetF1(elArc->GetFormulaF1());
    dialogTool->SetLength(elArc->GetFormulaLength());
    dialogTool->SetRadius1(elArc->GetFormulaRadius1());
    dialogTool->SetRadius2(elArc->GetFormulaRadius2());
    dialogTool->SetRotationAngle(elArc->GetFormulaRotationAngle());
    dialogTool->SetColor(elArc->GetColor());
    dialogTool->SetPenStyle(elArc->GetPenStyle());
    dialogTool->SetApproximationScale(elArc->GetApproximationScale());
    dialogTool->SetNotes(m_notes);
    dialogTool->SetAliasSuffix(elArc->GetAliasSuffix());
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolEllipticalArcWithLength::Create(const QPointer<DialogTool> &dialog,
                                          VMainGraphicsScene *scene,
                                          VAbstractPattern *doc,
                                          VContainer *data) -> VToolEllipticalArcWithLength *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogEllipticalArcWithLength> dialogTool = qobject_cast<DialogEllipticalArcWithLength *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolEllipticalArcWithLengthInitData initData;
    initData.center = dialogTool->GetCenter();
    initData.radius1 = dialogTool->GetRadius1();
    initData.radius2 = dialogTool->GetRadius2();
    initData.f1 = dialogTool->GetF1();
    initData.length = dialogTool->GetLength();
    initData.rotationAngle = dialogTool->GetRotationAngle();
    initData.color = dialogTool->GetColor();
    initData.penStyle = dialogTool->GetPenStyle();
    initData.approximationScale = dialogTool->GetApproximationScale();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();
    initData.aliasSuffix = dialogTool->GetAliasSuffix();

    VToolEllipticalArcWithLength *point = Create(initData);
    if (point != nullptr)
    {
        point->m_dialog = dialog;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolEllipticalArcWithLength::Create(VToolEllipticalArcWithLengthInitData &initData)
    -> VToolEllipticalArcWithLength *
{
    qreal const calcRadius1 = VAbstractValApplication::VApp()->toPixel(
        CheckFormula(initData.id, initData.radius1, initData.data));
    qreal const calcRadius2 = VAbstractValApplication::VApp()->toPixel(
        CheckFormula(initData.id, initData.radius2, initData.data));
    qreal const calcLength = VAbstractValApplication::VApp()->toPixel(
        CheckFormula(initData.id, initData.length, initData.data));
    qreal const calcF1 = CheckFormula(initData.id, initData.f1, initData.data);
    qreal const calcRotationAngle = CheckFormula(initData.id, initData.rotationAngle, initData.data);

    const VPointF c = *initData.data->GeometricObject<VPointF>(initData.center);
    auto *arc = new VEllipticalArc(calcLength,
                                   initData.length,
                                   c,
                                   calcRadius1,
                                   calcRadius2,
                                   initData.radius1,
                                   initData.radius2,
                                   calcF1,
                                   initData.f1,
                                   calcRotationAngle,
                                   initData.rotationAngle);
    arc->SetColor(initData.color);
    arc->SetPenStyle(initData.penStyle);
    arc->SetApproximationScale(initData.approximationScale);
    arc->SetAliasSuffix(initData.aliasSuffix);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->AddGObject(arc);
    }
    else
    {
        initData.data->UpdateGObject(initData.id, arc);
    }

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->AddVertex(initData.id, VNodeType::TOOL, initData.doc->PatternBlockMapper()->GetActiveId());

    const auto varData = initData.data->DataDependencyVariables();
    initData.doc->FindFormulaDependencies(initData.radius1, initData.id, varData);
    initData.doc->FindFormulaDependencies(initData.radius2, initData.id, varData);
    initData.doc->FindFormulaDependencies(initData.length, initData.id, varData);
    initData.doc->FindFormulaDependencies(initData.f1, initData.id, varData);
    initData.doc->FindFormulaDependencies(initData.rotationAngle, initData.id, varData);

    initData.data->AddArc(initData.data->GeometricObject<VEllipticalArc>(initData.id), initData.id);

    patternGraph->AddEdge(initData.center, initData.id);

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::EllipticalArcWithLength, initData.doc);
        auto *toolArc = new VToolEllipticalArcWithLength(initData);
        initData.scene->addItem(toolArc);
        InitElArcToolConnections(initData.scene, toolArc);
        VAbstractPattern::AddTool(initData.id, toolArc);
        return toolArc;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolEllipticalArcWithLength::getTagName() const -> QString
{
    return VAbstractPattern::TagElArc;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolEllipticalArcWithLength::GetFormulaRadius1() const -> VFormula
{
    QSharedPointer<VEllipticalArc> const elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    SCASSERT(elArc.isNull() == false)

    VFormula radius(elArc->GetFormulaRadius1(), getData());
    radius.setToolId(m_id);
    radius.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    radius.Eval();
    return radius;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArcWithLength::SetFormulaRadius1(const VFormula &value)
{
    if (!value.error())
    {
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        QSharedPointer<VEllipticalArc> const elArc = qSharedPointerDynamicCast<VEllipticalArc>(obj);
        elArc->SetFormulaRadius1(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolEllipticalArcWithLength::GetFormulaRadius2() const -> VFormula
{
    QSharedPointer<VEllipticalArc> const elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    SCASSERT(elArc.isNull() == false)

    VFormula radius(elArc->GetFormulaRadius2(), getData());
    radius.setToolId(m_id);
    radius.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    radius.Eval();
    return radius;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArcWithLength::SetFormulaRadius2(const VFormula &value)
{
    if (!value.error())
    {
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        QSharedPointer<VEllipticalArc> const elArc = qSharedPointerDynamicCast<VEllipticalArc>(obj);
        elArc->SetFormulaRadius2(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolEllipticalArcWithLength::GetFormulaF1() const -> VFormula
{
    QSharedPointer<VEllipticalArc> const elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    SCASSERT(elArc.isNull() == false)

    VFormula f1(elArc->GetFormulaF1(), getData());
    f1.setToolId(m_id);
    f1.setPostfix(degreeSymbol);
    f1.Eval();
    return f1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArcWithLength::SetFormulaF1(const VFormula &value)
{
    if (!value.error())
    {
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        QSharedPointer<VEllipticalArc> const elArc = qSharedPointerDynamicCast<VEllipticalArc>(obj);

        if (not VFuzzyComparePossibleNulls(value.getDoubleValue(), elArc->GetEndAngle())) // Angles can't be equal
        {
            elArc->SetFormulaF1(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
            SaveOption(obj);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolEllipticalArcWithLength::GetFormulaLength() const -> VFormula
{
    QSharedPointer<VEllipticalArc> const elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    SCASSERT(elArc.isNull() == false)

    VFormula radius(elArc->GetFormulaLength(), getData());
    radius.setCheckZero(true);
    radius.setToolId(m_id);
    radius.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    radius.Eval();
    return radius;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArcWithLength::SetFormulaLength(const VFormula &value)
{
    if (!value.error())
    {
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        QSharedPointer<VEllipticalArc> const elArc = qSharedPointerDynamicCast<VEllipticalArc>(obj);
        elArc->SetFormulaLength(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolEllipticalArcWithLength::GetFormulaRotationAngle() const -> VFormula
{
    QSharedPointer<VEllipticalArc> const elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    SCASSERT(elArc.isNull() == false)

    VFormula rotationAngle(elArc->GetFormulaRotationAngle(), getData());
    rotationAngle.setToolId(m_id);
    rotationAngle.setPostfix(degreeSymbol);
    rotationAngle.Eval();
    return rotationAngle;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArcWithLength::SetFormulaRotationAngle(const VFormula &value)
{
    if (!value.error())
    {
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        QSharedPointer<VEllipticalArc> const elArc = qSharedPointerDynamicCast<VEllipticalArc>(obj);
        elArc->SetFormulaRotationAngle(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolEllipticalArcWithLength::GetApproximationScale() const -> qreal
{
    QSharedPointer<VEllipticalArc> const elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    SCASSERT(elArc.isNull() == false)

    return elArc->GetApproximationScale();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArcWithLength::SetApproximationScale(qreal value)
{
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    QSharedPointer<VEllipticalArc> const elArc = qSharedPointerDynamicCast<VEllipticalArc>(obj);
    elArc->SetApproximationScale(value);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArcWithLength::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolEllipticalArcWithLength>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArcWithLength::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    Q_UNUSED(id)
    try
    {
        ContextMenu<DialogEllipticalArcWithLength>(event);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArcWithLength::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogEllipticalArcWithLength> dialogTool = qobject_cast<DialogEllipticalArcWithLength *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrCenter, QString().setNum(dialogTool->GetCenter()));
    doc->SetAttribute(domElement, AttrRadius1, dialogTool->GetRadius1());
    doc->SetAttribute(domElement, AttrRadius2, dialogTool->GetRadius2());
    doc->SetAttribute(domElement, AttrAngle1, dialogTool->GetF1());
    doc->SetAttribute(domElement, AttrLength, dialogTool->GetLength());
    doc->SetAttribute(domElement, AttrRotationAngle, dialogTool->GetRotationAngle());
    doc->SetAttribute(domElement, AttrColor, dialogTool->GetColor());
    doc->SetAttribute(domElement, AttrPenStyle, dialogTool->GetPenStyle());
    doc->SetAttribute(domElement, AttrAScale, dialogTool->GetApproximationScale());
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrAlias,
                                         dialogTool->GetAliasSuffix(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrNotes,
                                         dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArcWithLength::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolAbstractArc::SaveOptions(tag, obj);

    QSharedPointer<VEllipticalArc> const elArc = qSharedPointerDynamicCast<VEllipticalArc>(obj);
    SCASSERT(elArc.isNull() == false)

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrCenter, elArc->GetCenter().id());
    doc->SetAttribute(tag, AttrRadius1, elArc->GetFormulaRadius1());
    doc->SetAttribute(tag, AttrRadius2, elArc->GetFormulaRadius2());
    doc->SetAttribute(tag, AttrAngle1, elArc->GetFormulaF1());
    doc->SetAttribute(tag, AttrLength, elArc->GetFormulaLength());
    doc->SetAttribute(tag, AttrRotationAngle, elArc->GetFormulaRotationAngle());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArcWithLength::SetVisualization()
{
    if (not vis.isNull())
    {
        const QSharedPointer<VEllipticalArc> elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
        auto *visual = qobject_cast<VisToolEllipticalArcWithLength *>(vis);
        SCASSERT(visual != nullptr)

        const bool osSeparator = VAbstractApplication::VApp()->Settings()->GetOsSeparator();
        const VTranslateVars *trVars = VAbstractApplication::VApp()->TrVars();

        visual->SetCenterId(elArc->GetCenter().id());
        visual->SetRadius1(trVars->FormulaToUser(elArc->GetFormulaRadius1(), osSeparator));
        visual->SetRadius2(trVars->FormulaToUser(elArc->GetFormulaRadius2(), osSeparator));
        visual->SetF1(trVars->FormulaToUser(elArc->GetFormulaF1(), osSeparator));
        visual->SetLength(trVars->FormulaToUser(elArc->GetFormulaLength(), osSeparator));
        visual->SetRotationAngle(trVars->FormulaToUser(elArc->GetFormulaRotationAngle(), osSeparator));
        visual->SetLineStyle(LineStyleToPenStyle(elArc->GetPenStyle()));
        visual->SetApproximationScale(elArc->GetApproximationScale());
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolEllipticalArcWithLength::MakeToolTip() const -> QString
{
    const QSharedPointer<VEllipticalArc> elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);

    const QString toolTip = u"<table>"
                            u"<tr> <td><b>%12:</b> %13</td> </tr>"
                            u"<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                            u"<tr> <td><b>%4:</b> %5 %3</td> </tr>"
                            u"<tr> <td><b>%6:</b> %7 %3</td> </tr>"
                            u"<tr> <td><b>%8:</b> %9°</td> </tr>"
                            u"<tr> <td><b>%10:</b> %11°</td> </tr>"
                            u"</table>"_s
                                .arg(tr("Length"))                                                      // %1
                                .arg(VAbstractValApplication::VApp()->fromPixel(elArc->GetLength()))    // %2
                                .arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true), // %3
                                     tr("Radius1"))                                                     // %4
                                .arg(VAbstractValApplication::VApp()->fromPixel(elArc->GetRadius1()))   // %5
                                .arg(tr("Radius2"))                                                     // %6
                                .arg(VAbstractValApplication::VApp()->fromPixel(elArc->GetRadius2()))   // %7
                                .arg(tr("Start angle"))                                                 // %8
                                .arg(elArc->GetStartAngle())                                            // %9
                                .arg(tr("End angle"))                                                   // %10
                                .arg(elArc->GetEndAngle())                                              // %11
                                .arg(tr("Label"),                                                       // %12
                                     elArc->ObjectName());                                              // %13
    return toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArcWithLength::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    ProcessArcToolOptions(oldDomElement, newDomElement, GatherToolChanges());
}
