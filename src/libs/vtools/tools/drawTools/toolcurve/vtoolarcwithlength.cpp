/************************************************************************
 **
 **  @file   vtoolarcwithlength.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 6, 2015
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

#include "vtoolarcwithlength.h"

#include <QPen>
#include <QSharedPointer>
#include <new>

#include "../../../dialogs/tools/dialogarcwithlength.h"
#include "../../../dialogs/tools/dialogtool.h"
#include "../../../visualization/path/vistoolarcwithlength.h"
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

const QString VToolArcWithLength::ToolType = QStringLiteral("arcWithLength");

//---------------------------------------------------------------------------------------------------------------------
VToolArcWithLength::VToolArcWithLength(const VToolArcWithLengthInitData &initData, QGraphicsItem *parent)
  : VToolAbstractArc(initData.doc, initData.data, initData.id, initData.notes, parent)
{
    SetSceneType(SceneObject::Arc);

    this->setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus

    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArcWithLength::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogArcWithLength> dialogTool = qobject_cast<DialogArcWithLength *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VArc> arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    dialogTool->SetCenter(arc->GetCenter().id());
    dialogTool->SetF1(arc->GetFormulaF1());
    dialogTool->SetLength(arc->GetFormulaLength());
    dialogTool->SetRadius(arc->GetFormulaRadius());
    dialogTool->SetColor(arc->GetColor());
    dialogTool->SetPenStyle(arc->GetPenStyle());
    dialogTool->SetApproximationScale(arc->GetApproximationScale());
    dialogTool->SetNotes(m_notes);
    dialogTool->SetAliasSuffix(arc->GetAliasSuffix());
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolArcWithLength::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                                VContainer *data) -> VToolArcWithLength *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogArcWithLength> dialogTool = qobject_cast<DialogArcWithLength *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolArcWithLengthInitData initData;
    initData.center = dialogTool->GetCenter();
    initData.radius = dialogTool->GetRadius();
    initData.f1 = dialogTool->GetF1();
    initData.length = dialogTool->GetLength();
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

    VToolArcWithLength *point = Create(initData);
    if (point != nullptr)
    {
        point->m_dialog = dialog;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolArcWithLength::Create(VToolArcWithLengthInitData &initData) -> VToolArcWithLength *
{
    qreal calcRadius = 0, calcF1 = 0, calcLength = 0;

    calcRadius = VAbstractValApplication::VApp()->toPixel(CheckFormula(initData.id, initData.radius, initData.data));
    calcLength = VAbstractValApplication::VApp()->toPixel(CheckFormula(initData.id, initData.length, initData.data));
    calcF1 = CheckFormula(initData.id, initData.f1, initData.data);

    const VPointF c = *initData.data->GeometricObject<VPointF>(initData.center);
    auto *arc = new VArc(calcLength, initData.length, c, calcRadius, initData.radius, calcF1, initData.f1);
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
        VAbstractTool::AddRecord(initData.id, Tool::ArcWithLength, initData.doc);
        auto *toolArc = new VToolArcWithLength(initData);
        initData.scene->addItem(toolArc);
        InitArcToolConnections(initData.scene, toolArc);
        VAbstractPattern::AddTool(initData.id, toolArc);
        initData.doc->IncrementReferens(c.getIdTool());
        return toolArc;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolArcWithLength::getTagName() const -> QString
{
    return VAbstractPattern::TagArc;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolArcWithLength::GetFormulaRadius() const -> VFormula
{
    QSharedPointer<VArc> const arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    SCASSERT(arc.isNull() == false)

    VFormula radius(arc->GetFormulaRadius(), getData());
    radius.setToolId(m_id);
    radius.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    radius.Eval();
    return radius;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArcWithLength::SetFormulaRadius(const VFormula &value)
{
    if (value.error() == false)
    {
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        QSharedPointer<VArc> const arc = qSharedPointerDynamicCast<VArc>(obj);
        arc->SetFormulaRadius(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolArcWithLength::GetFormulaF1() const -> VFormula
{
    QSharedPointer<VArc> const arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    SCASSERT(arc.isNull() == false)

    VFormula f1(arc->GetFormulaF1(), getData());
    f1.setToolId(m_id);
    f1.setPostfix(degreeSymbol);
    f1.Eval();
    return f1;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArcWithLength::SetFormulaF1(const VFormula &value)
{
    if (value.error() == false)
    {
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        QSharedPointer<VArc> const arc = qSharedPointerDynamicCast<VArc>(obj);

        if (not VFuzzyComparePossibleNulls(value.getDoubleValue(), arc->GetEndAngle())) // Angles can't be equal
        {
            arc->SetFormulaF1(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
            SaveOption(obj);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolArcWithLength::GetFormulaLength() const -> VFormula
{
    QSharedPointer<VArc> const arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    SCASSERT(arc.isNull() == false)

    VFormula radius(arc->GetFormulaLength(), getData());
    radius.setCheckZero(true);
    radius.setToolId(m_id);
    radius.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    radius.Eval();
    return radius;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArcWithLength::SetFormulaLength(const VFormula &value)
{
    if (value.error() == false)
    {
        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        QSharedPointer<VArc> const arc = qSharedPointerDynamicCast<VArc>(obj);
        arc->SetFormulaLength(value.GetFormula(FormulaType::FromUser), value.getDoubleValue());
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolArcWithLength::GetApproximationScale() const -> qreal
{
    QSharedPointer<VArc> const arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    SCASSERT(arc.isNull() == false)

    return arc->GetApproximationScale();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArcWithLength::SetApproximationScale(qreal value)
{
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    QSharedPointer<VArc> const arc = qSharedPointerDynamicCast<VArc>(obj);
    arc->SetApproximationScale(value);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArcWithLength::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolArcWithLength>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArcWithLength::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    Q_UNUSED(id)
    try
    {
        ContextMenu<DialogArcWithLength>(event);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArcWithLength::RemoveReferens()
{
    const auto arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    doc->DecrementReferens(arc->GetCenter().getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArcWithLength::SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies,
                                    QList<quint32> &newDependencies)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogArcWithLength> dialogTool = qobject_cast<DialogArcWithLength *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    QSharedPointer<VArc> const arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
    SCASSERT(arc.isNull() == false)
    AddDependence(oldDependencies, arc->GetCenter().id());
    AddDependence(newDependencies, dialogTool->GetCenter());

    doc->SetAttribute(domElement, AttrCenter, QString().setNum(dialogTool->GetCenter()));
    doc->SetAttribute(domElement, AttrRadius, dialogTool->GetRadius());
    doc->SetAttribute(domElement, AttrAngle1, dialogTool->GetF1());
    doc->SetAttribute(domElement, AttrLength, dialogTool->GetLength());
    doc->SetAttribute(domElement, AttrColor, dialogTool->GetColor());
    doc->SetAttribute(domElement, AttrPenStyle, dialogTool->GetPenStyle());
    doc->SetAttribute(domElement, AttrAScale, dialogTool->GetApproximationScale());
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrAlias, dialogTool->GetAliasSuffix(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrNotes, dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArcWithLength::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VAbstractSpline::SaveOptions(tag, obj);

    QSharedPointer<VArc> const arc = qSharedPointerDynamicCast<VArc>(obj);
    SCASSERT(arc.isNull() == false)

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrCenter, arc->GetCenter().id());
    doc->SetAttribute(tag, AttrRadius, arc->GetFormulaRadius());
    doc->SetAttribute(tag, AttrAngle1, arc->GetFormulaF1());
    doc->SetAttribute(tag, AttrLength, arc->GetFormulaLength());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolArcWithLength::SetVisualization()
{
    if (not vis.isNull())
    {
        const QSharedPointer<VArc> arc = VAbstractTool::data.GeometricObject<VArc>(m_id);
        auto *visual = qobject_cast<VisToolArcWithLength *>(vis);
        SCASSERT(visual != nullptr)

        const bool osSeparator = VAbstractApplication::VApp()->Settings()->GetOsSeparator();
        const VTranslateVars *trVars = VAbstractApplication::VApp()->TrVars();

        visual->SetCenterId(arc->GetCenter().id());
        visual->SetRadius(trVars->FormulaToUser(arc->GetFormulaRadius(), osSeparator));
        visual->SetF1(trVars->FormulaToUser(arc->GetFormulaF1(), osSeparator));
        visual->SetLength(trVars->FormulaToUser(arc->GetFormulaLength(), osSeparator));
        visual->SetLineStyle(LineStyleToPenStyle(arc->GetPenStyle()));
        visual->SetApproximationScale(arc->GetApproximationScale());
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolArcWithLength::MakeToolTip() const -> QString
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
