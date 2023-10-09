/************************************************************************
 **
 **  @file   vtoolarc.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#include "vtoolarc.h"

#include <QPen>
#include <QSharedPointer>

#include "../../../dialogs/tools/dialogarc.h"
#include "../../../dialogs/tools/dialogtool.h"
#include "../../../visualization/path/vistoolarc.h"
#include "../../../visualization/visualization.h"
#include "../../vabstracttool.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vdomdocument.h"
#include "../vdrawtool.h"
#include "../vgeometry/varc.h"
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

const QString VToolArc::ToolType = QStringLiteral("simple"); // NOLINT

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolArc constuctor.
 * @param initData init data
 */
VToolArc::VToolArc(const VToolArcInitData &initData, QGraphicsItem *parent)
  : VToolAbstractArc(initData.doc, initData.data, initData.id, initData.notes, parent)
{
    SetSceneType(SceneObject::Arc);

    this->setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus

    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolArc::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogArc> dialogTool = qobject_cast<DialogArc *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VArc> arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    dialogTool->SetCenter(arc->GetCenter().id());
    dialogTool->SetF1(arc->GetFormulaF1());
    dialogTool->SetF2(arc->GetFormulaF2());
    dialogTool->SetRadius(arc->GetFormulaRadius());
    dialogTool->SetColor(arc->GetColor());
    dialogTool->SetPenStyle(arc->GetPenStyle());
    dialogTool->SetApproximationScale(arc->GetApproximationScale());
    dialogTool->SetNotes(m_notes);
    dialogTool->SetAliasSuffix(arc->GetAliasSuffix());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool
 * @param dialog dialog options.
 * @param scene pointer to scene.
 * @param doc dom document container
 * @param data container with variables
 */
auto VToolArc::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                      VContainer *data) -> VToolArc *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogArc> dialogTool = qobject_cast<DialogArc *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolArcInitData initData;
    initData.center = dialogTool->GetCenter();
    initData.radius = dialogTool->GetRadius();
    initData.f1 = dialogTool->GetF1();
    initData.f2 = dialogTool->GetF2();
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

    VToolArc *point = Create(initData);
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
auto VToolArc::Create(VToolArcInitData &initData) -> VToolArc *
{
    qreal calcRadius = 0, calcF1 = 0, calcF2 = 0;

    calcRadius = VAbstractValApplication::VApp()->toPixel(CheckFormula(initData.id, initData.radius, initData.data));

    calcF1 = CheckFormula(initData.id, initData.f1, initData.data);
    calcF2 = CheckFormula(initData.id, initData.f2, initData.data);

    const VPointF c = *initData.data->GeometricObject<VPointF>(initData.center);
    VArc *arc = new VArc(c, calcRadius, initData.radius, calcF1, initData.f1, calcF2, initData.f2);
    arc->SetColor(initData.color);
    arc->SetPenStyle(initData.penStyle);
    arc->SetApproximationScale(initData.approximationScale);
    arc->SetAliasSuffix(initData.aliasSuffix);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->AddGObject(arc);
        initData.data->AddArc(initData.data->GeometricObject<VArc>(initData.id), initData.id);
    }
    else
    {
        initData.data->UpdateGObject(initData.id, arc);
        initData.data->AddArc(initData.data->GeometricObject<VArc>(initData.id), initData.id);
        if (initData.parse != Document::FullParse)
        {
            initData.doc->UpdateToolData(initData.id, initData.data);
        }
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::Arc, initData.doc);
        auto *toolArc = new VToolArc(initData);
        initData.scene->addItem(toolArc);
        InitArcToolConnections(initData.scene, toolArc);
        VAbstractPattern::AddTool(initData.id, toolArc);
        initData.doc->IncrementReferens(c.getIdTool());
        return toolArc;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolArc::getTagName() const -> QString
{
    return VAbstractPattern::TagArc;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolArc::GetFormulaRadius() const -> VFormula
{
    QSharedPointer<VArc> arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    SCASSERT(arc.isNull() == false)

    VFormula radius(arc->GetFormulaRadius(), getData());
    radius.setCheckZero(true);
    radius.setToolId(m_id);
    radius.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    radius.Eval();
    return radius;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArc::SetFormulaRadius(const VFormula &value)
{
    if (!value.error())
    {
        if (value.getDoubleValue() > 0) // Formula don't check this, but radius can't be 0 or negative
        {
            QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
            QSharedPointer<VArc> arc = qSharedPointerDynamicCast<VArc>(obj);
            arc->SetFormulaRadius(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
            SaveOption(obj);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolArc::GetFormulaF1() const -> VFormula
{
    QSharedPointer<VArc> arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    SCASSERT(arc.isNull() == false)

    VFormula f1(arc->GetFormulaF1(), getData());
    f1.setCheckZero(false);
    f1.setToolId(m_id);
    f1.setPostfix(degreeSymbol);
    f1.Eval();
    return f1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArc::SetFormulaF1(const VFormula &value)
{
    if (!value.error())
    {
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        QSharedPointer<VArc> arc = qSharedPointerDynamicCast<VArc>(obj);

        arc->SetFormulaF1(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolArc::GetFormulaF2() const -> VFormula
{
    QSharedPointer<VArc> arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    SCASSERT(arc.isNull() == false)

    VFormula f2(arc->GetFormulaF2(), getData());
    f2.setCheckZero(false);
    f2.setToolId(m_id);
    f2.setPostfix(degreeSymbol);
    f2.Eval();
    return f2;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArc::SetFormulaF2(const VFormula &value)
{
    if (!value.error())
    {
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        QSharedPointer<VArc> arc = qSharedPointerDynamicCast<VArc>(obj);
        arc->SetFormulaF2(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolArc::GetApproximationScale() const -> qreal
{
    QSharedPointer<VArc> arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    SCASSERT(arc.isNull() == false)

    return arc->GetApproximationScale();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArc::SetApproximationScale(qreal value)
{
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    QSharedPointer<VArc> arc = qSharedPointerDynamicCast<VArc>(obj);
    arc->SetApproximationScale(value);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArc::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolArc>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArc::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    Q_UNUSED(id)
    try
    {
        ContextMenu<DialogArc>(event);
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
void VToolArc::RemoveReferens()
{
    const auto arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    doc->DecrementReferens(arc->GetCenter().getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolArc::SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies, QList<quint32> &newDependencies)
{
    SCASSERT(not m_dialog.isNull())
    QPointer<DialogArc> dialogTool = qobject_cast<DialogArc *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    QSharedPointer<VArc> arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    SCASSERT(arc.isNull() == false)
    AddDependence(oldDependencies, arc->GetCenter().id());
    AddDependence(newDependencies, dialogTool->GetCenter());

    doc->SetAttribute(domElement, AttrCenter, QString().setNum(dialogTool->GetCenter()));
    doc->SetAttribute(domElement, AttrRadius, dialogTool->GetRadius());
    doc->SetAttribute(domElement, AttrAngle1, dialogTool->GetF1());
    doc->SetAttribute(domElement, AttrAngle2, dialogTool->GetF2());
    doc->SetAttribute(domElement, AttrColor, dialogTool->GetColor());
    doc->SetAttribute(domElement, AttrPenStyle, dialogTool->GetPenStyle());
    doc->SetAttribute(domElement, AttrAScale, dialogTool->GetApproximationScale());
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrAlias, dialogTool->GetAliasSuffix(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrNotes, dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArc::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VAbstractSpline::SaveOptions(tag, obj);

    QSharedPointer<VArc> arc = qSharedPointerDynamicCast<VArc>(obj);
    SCASSERT(arc.isNull() == false)

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrCenter, arc->GetCenter().id());
    doc->SetAttribute(tag, AttrRadius, arc->GetFormulaRadius());
    doc->SetAttribute(tag, AttrAngle1, arc->GetFormulaF1());
    doc->SetAttribute(tag, AttrAngle2, arc->GetFormulaF2());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArc::SetVisualization()
{
    if (not vis.isNull())
    {
        const QSharedPointer<VArc> arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
        auto *visual = qobject_cast<VisToolArc *>(vis);
        SCASSERT(visual != nullptr)

        const bool osSeparator = VAbstractApplication::VApp()->Settings()->GetOsSeparator();
        const VTranslateVars *trVars = VAbstractApplication::VApp()->TrVars();

        visual->SetCenterId(arc->GetCenter().id());
        visual->SetRadius(trVars->FormulaToUser(arc->GetFormulaRadius(), osSeparator));
        visual->SetF1(trVars->FormulaToUser(arc->GetFormulaF1(), osSeparator));
        visual->SetF2(trVars->FormulaToUser(arc->GetFormulaF2(), osSeparator));
        visual->SetLineStyle(LineStyleToPenStyle(arc->GetPenStyle()));
        visual->SetApproximationScale(arc->GetApproximationScale());
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolArc::MakeToolTip() const -> QString
{
    const QSharedPointer<VArc> arc = VAbstractTool::data.GeometricObject<VArc>(m_id);

    const QString toolTip = u"<table>"
                            u"<tr> <td><b>%10:</b> %11</td> </tr>"
                            u"<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                            u"<tr> <td><b>%4:</b> %5 %3</td> </tr>"
                            u"<tr> <td><b>%6:</b> %7°</td> </tr>"
                            u"<tr> <td><b>%8:</b> %9°</td> </tr>"
                            u"</table>"_s.arg(tr("Length"))
                                .arg(VAbstractValApplication::VApp()->fromPixel(arc->GetLength()))
                                .arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true), tr("Radius"))
                                .arg(VAbstractValApplication::VApp()->fromPixel(arc->GetRadius()))
                                .arg(tr("Start angle"))
                                .arg(arc->GetStartAngle())
                                .arg(tr("End angle"))
                                .arg(arc->GetEndAngle())
                                .arg(tr("Label"), arc->ObjectName());
    return toolTip;
}
