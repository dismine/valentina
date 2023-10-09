/************************************************************************
 **
 **  @file   vtoolellipticalarc.cpp
 **  @author Valentina Zhuravska <zhuravska19(at)gmail.com>
 **  @date   20 10, 2016
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

#include "vtoolellipticalarc.h"

#include <QPen>
#include <QSharedPointer>
#include <new>

#include "../../../dialogs/tools/dialogellipticalarc.h"
#include "../../../dialogs/tools/dialogtool.h"
#include "../../../visualization/path/vistoolellipticalarc.h"
#include "../../../visualization/visualization.h"
#include "../../vabstracttool.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vdomdocument.h"
#include "../vdrawtool.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vformula.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vabstractspline.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

const QString VToolEllipticalArc::ToolType = QStringLiteral("simple");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolEllipticalArc constuctor.
 * @param initData init data
 * @param parent parent object
 */
VToolEllipticalArc::VToolEllipticalArc(const VToolEllipticalArcInitData &initData, QGraphicsItem *parent)
  : VToolAbstractArc(initData.doc, initData.data, initData.id, initData.notes, parent)
{
    SetSceneType(SceneObject::ElArc);

    this->setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus

    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolEllipticalArc::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogEllipticalArc> dialogTool = qobject_cast<DialogEllipticalArc *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VEllipticalArc> elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    dialogTool->SetCenter(elArc->GetCenter().id());
    dialogTool->SetF1(elArc->GetFormulaF1());
    dialogTool->SetF2(elArc->GetFormulaF2());
    dialogTool->SetRadius1(elArc->GetFormulaRadius1());
    dialogTool->SetRadius2(elArc->GetFormulaRadius2());
    dialogTool->SetRotationAngle(elArc->GetFormulaRotationAngle());
    dialogTool->SetColor(elArc->GetColor());
    dialogTool->SetPenStyle(elArc->GetPenStyle());
    dialogTool->SetNotes(m_notes);
    dialogTool->SetAliasSuffix(elArc->GetAliasSuffix());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool
 * @param dialog dialog options.
 * @param scene pointer to scene.
 * @param doc dom document container
 * @param data container with variables
 */
auto VToolEllipticalArc::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                                VContainer *data) -> VToolEllipticalArc *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogEllipticalArc> dialogTool = qobject_cast<DialogEllipticalArc *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolEllipticalArcInitData initData;
    initData.center = dialogTool->GetCenter();
    initData.radius1 = dialogTool->GetRadius1();
    initData.radius2 = dialogTool->GetRadius2();
    initData.f1 = dialogTool->GetF1();
    initData.f2 = dialogTool->GetF2();
    initData.rotationAngle = dialogTool->GetRotationAngle();
    initData.color = dialogTool->GetColor();
    initData.penStyle = dialogTool->GetPenStyle();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.approximationScale = dialogTool->GetApproximationScale();
    initData.notes = dialogTool->GetNotes();
    initData.aliasSuffix = dialogTool->GetAliasSuffix();
    // initData.approximationScale = dialogTool->GetApproximationScale(); // For future use

    VToolEllipticalArc *point = Create(initData);
    if (point != nullptr)
    {
        point->m_dialog = dialog;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool form GUI.
 * @param initData init data.
 */
auto VToolEllipticalArc::Create(VToolEllipticalArcInitData &initData) -> VToolEllipticalArc *
{
    qreal calcRadius1 = 0, calcRadius2 = 0, calcF1 = 0, calcF2 = 0, calcRotationAngle = 0;

    calcRadius1 = VAbstractValApplication::VApp()->toPixel(CheckFormula(initData.id, initData.radius1, initData.data));
    calcRadius2 = VAbstractValApplication::VApp()->toPixel(CheckFormula(initData.id, initData.radius2, initData.data));

    calcF1 = CheckFormula(initData.id, initData.f1, initData.data);
    calcF2 = CheckFormula(initData.id, initData.f2, initData.data);
    calcRotationAngle = CheckFormula(initData.id, initData.rotationAngle, initData.data);

    const VPointF c = *initData.data->GeometricObject<VPointF>(initData.center);
    VEllipticalArc *elArc =
        new VEllipticalArc(c, calcRadius1, calcRadius2, initData.radius1, initData.radius2, calcF1, initData.f1, calcF2,
                           initData.f2, calcRotationAngle, initData.rotationAngle);
    elArc->SetColor(initData.color);
    elArc->SetPenStyle(initData.penStyle);
    elArc->SetApproximationScale(initData.approximationScale);
    elArc->SetAliasSuffix(initData.aliasSuffix);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->AddGObject(elArc);
        initData.data->AddArc(initData.data->GeometricObject<VEllipticalArc>(initData.id), initData.id);
    }
    else
    {
        initData.data->UpdateGObject(initData.id, elArc);
        initData.data->AddArc(initData.data->GeometricObject<VEllipticalArc>(initData.id), initData.id);
        if (initData.parse != Document::FullParse)
        {
            initData.doc->UpdateToolData(initData.id, initData.data);
        }
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::EllipticalArc, initData.doc);
        auto *toolEllipticalArc = new VToolEllipticalArc(initData);
        initData.scene->addItem(toolEllipticalArc);
        InitElArcToolConnections(initData.scene, toolEllipticalArc);
        VAbstractPattern::AddTool(initData.id, toolEllipticalArc);
        initData.doc->IncrementReferens(c.getIdTool());
        return toolEllipticalArc;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolEllipticalArc::getTagName() const -> QString
{
    return VAbstractPattern::TagElArc;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolEllipticalArc::GetFormulaRadius1() const -> VFormula
{
    QSharedPointer<VEllipticalArc> elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    SCASSERT(elArc.isNull() == false)

    VFormula radius1(elArc->GetFormulaRadius1(), getData());
    radius1.setCheckZero(true);
    radius1.setToolId(m_id);
    radius1.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    radius1.Eval();
    return radius1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArc::SetFormulaRadius1(const VFormula &value)
{
    if (value.error() == false)
    {
        if (value.getDoubleValue() > 0) // Formula don't check this, but radius1 can't be 0 or negative
        {
            QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
            QSharedPointer<VEllipticalArc> elArc = qSharedPointerDynamicCast<VEllipticalArc>(obj);
            elArc->SetFormulaRadius1(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
            SaveOption(obj);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolEllipticalArc::GetFormulaRadius2() const -> VFormula
{
    QSharedPointer<VEllipticalArc> elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    SCASSERT(elArc.isNull() == false)

    VFormula radius2(elArc->GetFormulaRadius2(), getData());
    radius2.setCheckZero(true);
    radius2.setToolId(m_id);
    radius2.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    radius2.Eval();
    return radius2;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArc::SetFormulaRadius2(const VFormula &value)
{
    if (value.error() == false)
    {
        if (value.getDoubleValue() > 0) // Formula don't check this, but radius2 can't be 0 or negative
        {
            QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
            QSharedPointer<VEllipticalArc> elArc = qSharedPointerDynamicCast<VEllipticalArc>(obj);
            elArc->SetFormulaRadius2(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
            SaveOption(obj);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolEllipticalArc::GetFormulaF1() const -> VFormula
{
    QSharedPointer<VEllipticalArc> elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    SCASSERT(elArc.isNull() == false)

    VFormula f1(elArc->GetFormulaF1(), getData());
    f1.setCheckZero(false);
    f1.setToolId(m_id);
    f1.setPostfix(degreeSymbol);
    f1.Eval();
    return f1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArc::SetFormulaF1(const VFormula &value)
{
    if (value.error() == false)
    {
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        QSharedPointer<VEllipticalArc> elArc = qSharedPointerDynamicCast<VEllipticalArc>(obj);
        elArc->SetFormulaF1(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolEllipticalArc::GetFormulaF2() const -> VFormula
{
    QSharedPointer<VEllipticalArc> elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    SCASSERT(elArc.isNull() == false)

    VFormula f2(elArc->GetFormulaF2(), getData());
    f2.setCheckZero(false);
    f2.setToolId(m_id);
    f2.setPostfix(degreeSymbol);
    f2.Eval();
    return f2;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArc::SetFormulaF2(const VFormula &value)
{
    if (value.error() == false)
    {
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        QSharedPointer<VEllipticalArc> elArc = qSharedPointerDynamicCast<VEllipticalArc>(obj);
        elArc->SetFormulaF2(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolEllipticalArc::GetFormulaRotationAngle() const -> VFormula
{
    QSharedPointer<VEllipticalArc> elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    SCASSERT(elArc.isNull() == false)

    VFormula rotationAngle(elArc->GetFormulaRotationAngle(), getData());
    rotationAngle.setCheckZero(false);
    rotationAngle.setToolId(m_id);
    rotationAngle.setPostfix(degreeSymbol);
    rotationAngle.Eval();
    return rotationAngle;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArc::SetFormulaRotationAngle(const VFormula &value)
{
    if (value.error() == false)
    {
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        QSharedPointer<VEllipticalArc> elArc = qSharedPointerDynamicCast<VEllipticalArc>(obj);
        elArc->SetFormulaRotationAngle(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolEllipticalArc::GetApproximationScale() const -> qreal
{
    QSharedPointer<VEllipticalArc> arc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    SCASSERT(arc.isNull() == false)

    return arc->GetApproximationScale();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArc::SetApproximationScale(qreal value)
{
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    QSharedPointer<VEllipticalArc> arc = qSharedPointerDynamicCast<VEllipticalArc>(obj);
    arc->SetApproximationScale(value);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArc::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolEllipticalArc>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArc::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    Q_UNUSED(id)
    try
    {
        ContextMenu<DialogEllipticalArc>(event);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RemoveReferens decrement value of reference.
 */
void VToolEllipticalArc::RemoveReferens()
{
    const auto elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    doc->DecrementReferens(elArc->GetCenter().getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolEllipticalArc::SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies,
                                    QList<quint32> &newDependencies)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogEllipticalArc> dialogTool = qobject_cast<DialogEllipticalArc *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    const auto elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
    SCASSERT(elArc.isNull() == false)
    AddDependence(oldDependencies, elArc->GetCenter().id());
    AddDependence(newDependencies, dialogTool->GetCenter());

    doc->SetAttribute(domElement, AttrCenter, QString().setNum(dialogTool->GetCenter()));
    doc->SetAttribute(domElement, AttrRadius1, dialogTool->GetRadius1());
    doc->SetAttribute(domElement, AttrRadius2, dialogTool->GetRadius2());
    doc->SetAttribute(domElement, AttrAngle1, dialogTool->GetF1());
    doc->SetAttribute(domElement, AttrAngle2, dialogTool->GetF2());
    doc->SetAttribute(domElement, AttrRotationAngle, dialogTool->GetRotationAngle());
    doc->SetAttribute(domElement, AttrColor, dialogTool->GetColor());
    doc->SetAttribute(domElement, AttrPenStyle, dialogTool->GetPenStyle());
    doc->SetAttribute(domElement, AttrAScale, dialogTool->GetApproximationScale());
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrAlias, dialogTool->GetAliasSuffix(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrNotes, dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArc::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VAbstractSpline::SaveOptions(tag, obj);

    QSharedPointer<VEllipticalArc> elArc = qSharedPointerDynamicCast<VEllipticalArc>(obj);
    SCASSERT(elArc.isNull() == false)

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrCenter, elArc->GetCenter().id());
    doc->SetAttribute(tag, AttrRadius1, elArc->GetFormulaRadius1());
    doc->SetAttribute(tag, AttrRadius2, elArc->GetFormulaRadius2());
    doc->SetAttribute(tag, AttrAngle1, elArc->GetFormulaF1());
    doc->SetAttribute(tag, AttrAngle2, elArc->GetFormulaF2());
    doc->SetAttribute(tag, AttrRotationAngle, elArc->GetFormulaRotationAngle());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolEllipticalArc::SetVisualization()
{
    if (not vis.isNull())
    {
        const QSharedPointer<VEllipticalArc> elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);
        auto *visual = qobject_cast<VisToolEllipticalArc *>(vis);
        SCASSERT(visual != nullptr)

        const bool osSeparator = VAbstractApplication::VApp()->Settings()->GetOsSeparator();
        const VTranslateVars *trVars = VAbstractApplication::VApp()->TrVars();

        visual->SetCenterId(elArc->GetCenter().id());
        visual->SetRadius1(trVars->FormulaToUser(elArc->GetFormulaRadius1(), osSeparator));
        visual->SetRadius2(trVars->FormulaToUser(elArc->GetFormulaRadius2(), osSeparator));
        visual->SetF1(trVars->FormulaToUser(elArc->GetFormulaF1(), osSeparator));
        visual->SetF2(trVars->FormulaToUser(elArc->GetFormulaF2(), osSeparator));
        visual->SetRotationAngle(trVars->FormulaToUser(elArc->GetFormulaRotationAngle(), osSeparator));
        visual->SetLineStyle(LineStyleToPenStyle(elArc->GetPenStyle()));
        visual->SetApproximationScale(elArc->GetApproximationScale());
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolEllipticalArc::MakeToolTip() const -> QString
{
    const QSharedPointer<VEllipticalArc> elArc = VAbstractTool::data.GeometricObject<VEllipticalArc>(m_id);

    const QString toolTip = u"<table>"
                            u"<tr> <td><b>%12:</b> %13</td> </tr>"
                            u"<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                            u"<tr> <td><b>%4:</b> %5 %3</td> </tr>"
                            u"<tr> <td><b>%6:</b> %7 %3</td> </tr>"
                            u"<tr> <td><b>%8:</b> %9°</td> </tr>"
                            u"<tr> <td><b>%10:</b> %11°</td> </tr>"
                            u"<tr> <td><b>%14:</b> %15°</td> </tr>"
                            u"</table>"_s
                                .arg(tr("Length"))                                                      // 1
                                .arg(VAbstractValApplication::VApp()->fromPixel(elArc->GetLength()))    // 2
                                .arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true), // 3
                                     tr("Radius") + '1'_L1)                                             // 4
                                .arg(VAbstractValApplication::VApp()->fromPixel(elArc->GetRadius1()))   // 5
                                .arg(tr("Radius") + '2'_L1)                                             // 6
                                .arg(VAbstractValApplication::VApp()->fromPixel(elArc->GetRadius2()))   // 7
                                .arg(tr("Start angle"))                                                 // 8
                                .arg(elArc->GetStartAngle())                                            // 9
                                .arg(tr("End angle"))                                                   // 10
                                .arg(elArc->GetEndAngle())                                              // 11
                                .arg(tr("Label"),                                                       // 12
                                     elArc->ObjectName(),                                               // 13
                                     tr("Rotation"))                                                    // 14
                                .arg(elArc->GetRotationAngle());                                        // 15
    return toolTip;
}
