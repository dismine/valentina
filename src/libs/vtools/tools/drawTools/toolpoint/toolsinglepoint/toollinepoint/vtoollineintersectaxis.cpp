/************************************************************************
 **
 **  @file   vtoollineintersectaxis.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   19 10, 2014
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

#include "vtoollineintersectaxis.h"

#include <QLine>
#include <QSharedPointer>
#include <new>

#include "../../../../../dialogs/tools/dialoglineintersectaxis.h"
#include "../../../../../dialogs/tools/dialogtool.h"
#include "../../../../../visualization/line/vistoollineintersectaxis.h"
#include "../../../../../visualization/visualization.h"
#include "../../../../vabstracttool.h"
#include "../../../vdrawtool.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vtoollinepoint.h"

using namespace Qt::Literals::StringLiterals;

template <class T> class QSharedPointer;

const QString VToolLineIntersectAxis::ToolType = QStringLiteral("lineIntersectAxis");

//---------------------------------------------------------------------------------------------------------------------
VToolLineIntersectAxis::VToolLineIntersectAxis(const VToolLineIntersectAxisInitData &initData, QGraphicsItem *parent)
  : VToolLinePoint(initData.doc, initData.data, initData.id, initData.typeLine, initData.lineColor, QString(),
                   initData.basePointId, 0, initData.notes, parent),
    formulaAngle(initData.formulaAngle),
    firstPointId(initData.firstPointId),
    secondPointId(initData.secondPointId)
{
    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLineIntersectAxis::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    m_dialog->setModal(true);
    const QPointer<DialogLineIntersectAxis> dialogTool = qobject_cast<DialogLineIntersectAxis *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->SetTypeLine(m_lineType);
    dialogTool->SetLineColor(lineColor);
    dialogTool->SetAngle(formulaAngle);
    dialogTool->SetBasePointId(basePointId);
    dialogTool->SetFirstPointId(firstPointId);
    dialogTool->SetSecondPointId(secondPointId);
    dialogTool->SetPointName(p->name());
    dialogTool->SetNotes(m_notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolLineIntersectAxis::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene,
                                    VAbstractPattern *doc, VContainer *data) -> VToolLineIntersectAxis *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogLineIntersectAxis> dialogTool = qobject_cast<DialogLineIntersectAxis *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolLineIntersectAxisInitData initData;
    initData.formulaAngle = dialogTool->GetAngle();
    initData.basePointId = dialogTool->GetBasePointId();
    initData.firstPointId = dialogTool->GetFirstPointId();
    initData.secondPointId = dialogTool->GetSecondPointId();
    initData.typeLine = dialogTool->GetTypeLine();
    initData.lineColor = dialogTool->GetLineColor();
    initData.name = dialogTool->GetPointName();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();

    VToolLineIntersectAxis *point = Create(initData);
    if (point != nullptr)
    {
        point->m_dialog = dialog;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolLineIntersectAxis::Create(VToolLineIntersectAxisInitData &initData) -> VToolLineIntersectAxis *
{
    const QSharedPointer<VPointF> basePoint = initData.data->GeometricObject<VPointF>(initData.basePointId);
    QLineF axis = QLineF(static_cast<QPointF>(*basePoint), QPointF(basePoint->x() + 100, basePoint->y()));
    axis.setAngle(CheckFormula(initData.id, initData.formulaAngle, initData.data));

    const QSharedPointer<VPointF> firstPoint = initData.data->GeometricObject<VPointF>(initData.firstPointId);
    const QSharedPointer<VPointF> secondPoint = initData.data->GeometricObject<VPointF>(initData.secondPointId);
    QLineF line(static_cast<QPointF>(*firstPoint), static_cast<QPointF>(*secondPoint));

    QPointF fPoint;
    const bool success = FindPoint(axis, line, &fPoint);

    if (not success)
    {
        const QString errorMsg = tr("Error calculating point '%1'. Line (%2;%3) doesn't have intersection with axis "
                                    "through point '%4' and angle %5°")
                                     .arg(initData.name, firstPoint->name(), secondPoint->name(), basePoint->name())
                                     .arg(axis.angle());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VExceptionObjectError(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
    }

    VPointF *p = new VPointF(fPoint, initData.name, initData.mx, initData.my);
    p->SetShowLabel(initData.showLabel);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->AddGObject(p);
        initData.data->AddLine(initData.basePointId, initData.id);
        initData.data->AddLine(initData.firstPointId, initData.id);
        initData.data->AddLine(initData.id, initData.secondPointId);
    }
    else
    {
        initData.data->UpdateGObject(initData.id, p);
        initData.data->AddLine(initData.basePointId, initData.id);
        initData.data->AddLine(initData.firstPointId, initData.id);
        initData.data->AddLine(initData.id, initData.secondPointId);
        if (initData.parse != Document::FullParse)
        {
            initData.doc->UpdateToolData(initData.id, initData.data);
        }
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::LineIntersectAxis, initData.doc);
        VToolLineIntersectAxis *point = new VToolLineIntersectAxis(initData);
        initData.scene->addItem(point);
        InitToolConnections(initData.scene, point);
        VAbstractPattern::AddTool(initData.id, point);
        initData.doc->IncrementReferens(basePoint->getIdTool());
        initData.doc->IncrementReferens(firstPoint->getIdTool());
        initData.doc->IncrementReferens(secondPoint->getIdTool());
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolLineIntersectAxis::FindPoint(const QLineF &axis, const QLineF &line, QPointF *intersectionPoint) -> bool
{
    SCASSERT(intersectionPoint != nullptr)

    QPointF fPoint;
    QLineF::IntersectType intersect = Intersects(axis, line, &fPoint);

    if (intersect == QLineF::UnboundedIntersection || intersect == QLineF::BoundedIntersection)
    {
        if (VFuzzyComparePossibleNulls(axis.angle(), line.angle()) ||
            VFuzzyComparePossibleNulls(qAbs(axis.angle() - line.angle()), 180))
        {
            return false;
        }
        else
        {
            *intersectionPoint = fPoint;
            return true;
        }
    }
    else
    {
        return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolLineIntersectAxis::FirstLinePoint() const -> QString
{
    return VAbstractTool::data.GetGObject(firstPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolLineIntersectAxis::SecondLinePoint() const -> QString
{
    return VAbstractTool::data.GetGObject(secondPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolLineIntersectAxis::GetFormulaAngle() const -> VFormula
{
    VFormula fAngle(formulaAngle, getData());
    fAngle.setCheckZero(false);
    fAngle.setToolId(m_id);
    fAngle.setPostfix(degreeSymbol);
    fAngle.Eval();
    return fAngle;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLineIntersectAxis::SetFormulaAngle(const VFormula &value)
{
    if (value.error() == false)
    {
        formulaAngle = value.GetFormula(FormulaType::FromUser);

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLineIntersectAxis::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolLineIntersectAxis>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLineIntersectAxis::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogLineIntersectAxis>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLineIntersectAxis::SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies,
                                        QList<quint32> &newDependencies)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogLineIntersectAxis> dialogTool = qobject_cast<DialogLineIntersectAxis *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    AddDependence(oldDependencies, basePointId);
    AddDependence(oldDependencies, firstPointId);
    AddDependence(oldDependencies, secondPointId);
    AddDependence(newDependencies, dialogTool->GetBasePointId());
    AddDependence(newDependencies, dialogTool->GetFirstPointId());
    AddDependence(newDependencies, dialogTool->GetSecondPointId());

    doc->SetAttribute(domElement, AttrName, dialogTool->GetPointName());
    doc->SetAttribute(domElement, AttrTypeLine, dialogTool->GetTypeLine());
    doc->SetAttribute(domElement, AttrLineColor, dialogTool->GetLineColor());
    doc->SetAttribute(domElement, AttrAngle, dialogTool->GetAngle());
    doc->SetAttribute(domElement, AttrBasePoint, QString().setNum(dialogTool->GetBasePointId()));
    doc->SetAttribute(domElement, AttrP1Line, QString().setNum(dialogTool->GetFirstPointId()));
    doc->SetAttribute(domElement, AttrP2Line, QString().setNum(dialogTool->GetSecondPointId()));
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrNotes, dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLineIntersectAxis::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolLinePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrAngle, formulaAngle);
    doc->SetAttribute(tag, AttrBasePoint, basePointId);
    doc->SetAttribute(tag, AttrP1Line, firstPointId);
    doc->SetAttribute(tag, AttrP2Line, secondPointId);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLineIntersectAxis::ReadToolAttributes(const QDomElement &domElement)
{
    VToolLinePoint::ReadToolAttributes(domElement);

    m_lineType = doc->GetParametrString(domElement, AttrTypeLine, TypeLineLine);
    lineColor = doc->GetParametrString(domElement, AttrLineColor, ColorBlack);
    basePointId = doc->GetParametrUInt(domElement, AttrBasePoint, NULL_ID_STR);
    firstPointId = doc->GetParametrUInt(domElement, AttrP1Line, NULL_ID_STR);
    secondPointId = doc->GetParametrUInt(domElement, AttrP2Line, NULL_ID_STR);
    formulaAngle = doc->GetParametrString(domElement, AttrAngle, QString());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLineIntersectAxis::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolLineIntersectAxis *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetPoint1Id(firstPointId);
        visual->SetPoint2Id(secondPointId);
        visual->SetAxisPointId(basePointId);
        visual->SetAngle(VAbstractApplication::VApp()->TrVars()->FormulaToUser(
            formulaAngle, VAbstractApplication::VApp()->Settings()->GetOsSeparator()));
        visual->SetLineStyle(LineStyleToPenStyle(m_lineType));
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolLineIntersectAxis::MakeToolTip() const -> QString
{
    const QSharedPointer<VPointF> basePoint = VAbstractTool::data.GeometricObject<VPointF>(basePointId);
    const QSharedPointer<VPointF> firstPoint = VAbstractTool::data.GeometricObject<VPointF>(firstPointId);
    const QSharedPointer<VPointF> secondPoint = VAbstractTool::data.GeometricObject<VPointF>(secondPointId);
    const QSharedPointer<VPointF> current = VAbstractTool::data.GeometricObject<VPointF>(m_id);

    const QLineF curLine(static_cast<QPointF>(*basePoint), static_cast<QPointF>(*current));
    const QLineF firstToCur(static_cast<QPointF>(*firstPoint), static_cast<QPointF>(*current));
    const QLineF curToSecond(static_cast<QPointF>(*current), static_cast<QPointF>(*secondPoint));

    const QString toolTip = u"<table>"
                            u"<tr> <td><b>%10:</b> %11</td> </tr>"
                            u"<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                            u"<tr> <td><b>%4:</b> %5°</td> </tr>"
                            u"<tr> <td><b>%6:</b> %7 %3</td> </tr>"
                            u"<tr> <td><b>%8:</b> %9 %3</td> </tr>"
                            u"</table>"_s.arg(tr("Length"))
                                .arg(VAbstractValApplication::VApp()->fromPixel(curLine.length()))
                                .arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true), tr("Angle"))
                                .arg(curLine.angle())
                                .arg(u"%1->%2"_s.arg(firstPoint->name(), current->name()))
                                .arg(VAbstractValApplication::VApp()->fromPixel(firstToCur.length()))
                                .arg(u"%1->%2"_s.arg(current->name(), secondPoint->name()))
                                .arg(VAbstractValApplication::VApp()->fromPixel(curToSecond.length()))
                                .arg(tr("Label"), current->name());
    return toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLineIntersectAxis::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    VToolLinePoint::hoverEnterEvent(event);
    mainLine->SetBoldLine(false);
}
