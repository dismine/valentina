/************************************************************************
 **
 **  @file   vtoolcurveintersectaxis.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 10, 2014
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

#include "vtoolcurveintersectaxis.h"

#include <QLineF>
#include <QMap>
#include <QRectF>
#include <QSharedPointer>
#include <QVector>
#include <climits>
#include <new>

#include "../../../../../dialogs/tools/dialogcurveintersectaxis.h"
#include "../../../../../dialogs/tools/dialogtool.h"
#include "../../../../vabstracttool.h"
#include "../../../vdrawtool.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/ifcdef.h"
#include "../toolcut/vtoolcutsplinepath.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vtools/visualization/line/vistoolcurveintersectaxis.h"
#include "../vtools/visualization/visualization.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vtoollinepoint.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

template <class T> class QSharedPointer;

const QString VToolCurveIntersectAxis::ToolType = QStringLiteral("curveIntersectAxis");

//---------------------------------------------------------------------------------------------------------------------
VToolCurveIntersectAxis::VToolCurveIntersectAxis(const VToolCurveIntersectAxisInitData &initData, QGraphicsItem *parent)
  : VToolLinePoint(initData.doc, initData.data, initData.id, initData.typeLine, initData.lineColor, QString(),
                   initData.basePointId, 0, initData.notes, parent),
    formulaAngle(initData.formulaAngle),
    curveId(initData.curveId),
    m_segments(initData.segments),
    m_aliasSuffix1(initData.aliasSuffix1),
    m_aliasSuffix2(initData.aliasSuffix2)
{
    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    m_dialog->setModal(true);
    const QPointer<DialogCurveIntersectAxis> dialogTool = qobject_cast<DialogCurveIntersectAxis *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->SetTypeLine(m_lineType);
    dialogTool->SetLineColor(lineColor);
    dialogTool->SetAngle(formulaAngle);
    dialogTool->SetBasePointId(basePointId);
    dialogTool->setCurveId(curveId);
    dialogTool->SetPointName(p->name());
    dialogTool->SetNotes(m_notes);
    dialogTool->SetAliasSuffix1(m_aliasSuffix1);
    dialogTool->SetAliasSuffix2(m_aliasSuffix2);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCurveIntersectAxis::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene,
                                     VAbstractPattern *doc, VContainer *data) -> VToolCurveIntersectAxis *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogCurveIntersectAxis> dialogTool = qobject_cast<DialogCurveIntersectAxis *>(dialog);
    SCASSERT(not dialogTool.isNull())

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    VToolCurveIntersectAxisInitData initData;

    QT_WARNING_POP

    initData.formulaAngle = dialogTool->GetAngle();
    initData.basePointId = dialogTool->GetBasePointId();
    initData.curveId = dialogTool->getCurveId();
    initData.typeLine = dialogTool->GetTypeLine();
    initData.lineColor = dialogTool->GetLineColor();
    initData.name = dialogTool->GetPointName();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();
    initData.aliasSuffix1 = dialogTool->GetAliasSuffix1();
    initData.aliasSuffix2 = dialogTool->GetAliasSuffix2();

    VToolCurveIntersectAxis *point = Create(initData);
    if (point != nullptr)
    {
        point->m_dialog = dialog;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCurveIntersectAxis::Create(VToolCurveIntersectAxisInitData &initData) -> VToolCurveIntersectAxis *
{
    const QSharedPointer<VPointF> basePoint = initData.data->GeometricObject<VPointF>(initData.basePointId);
    const qreal angle = CheckFormula(initData.id, initData.formulaAngle, initData.data);
    const QSharedPointer<VAbstractCurve> curve = initData.data->GeometricObject<VAbstractCurve>(initData.curveId);

    QPointF fPoint;
    const bool success = FindPoint(static_cast<QPointF>(*basePoint), angle, curve->GetPoints(), &fPoint);

    if (not success)
    {
        const QString errorMsg = tr("Error calculating point '%1'. There is no intersection with curve '%2' and axis"
                                    " through point '%3' with angle %4°")
                                     .arg(initData.name, curve->ObjectName(), basePoint->name())
                                     .arg(angle);
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VExceptionObjectError(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
    }

    const qreal segLength = curve->GetLengthByPoint(fPoint);

    auto *p = new VPointF(fPoint, initData.name, initData.mx, initData.my);
    p->SetShowLabel(initData.showLabel);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->AddGObject(p);
        initData.data->AddLine(initData.basePointId, initData.id);

        initData.data->getNextId();
        initData.data->getNextId();
        initData.segments = VToolSinglePoint::InitSegments(curve->getType(), segLength, p, initData.curveId,
                                                           initData.data, initData.aliasSuffix1, initData.aliasSuffix2);
    }
    else
    {
        initData.data->UpdateGObject(initData.id, p);
        initData.data->AddLine(initData.basePointId, initData.id);

        initData.segments = VToolSinglePoint::InitSegments(curve->getType(), segLength, p, initData.curveId,
                                                           initData.data, initData.aliasSuffix1, initData.aliasSuffix2);

        if (initData.parse != Document::FullParse)
        {
            initData.doc->UpdateToolData(initData.id, initData.data);
        }
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::CurveIntersectAxis, initData.doc);
        auto *point = new VToolCurveIntersectAxis(initData);
        initData.scene->addItem(point);
        InitToolConnections(initData.scene, point);
        VAbstractPattern::AddTool(initData.id, point);
        initData.doc->IncrementReferens(basePoint->getIdTool());
        initData.doc->IncrementReferens(curve->getIdTool());
        return point;
    }

    auto *tool = qobject_cast<VToolCurveIntersectAxis *>(VAbstractPattern::getTool(initData.id));
    tool->SetSegments(initData.segments);

    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCurveIntersectAxis::FindPoint(const QPointF &point, qreal angle, const QVector<QPointF> &curvePoints,
                                        QPointF *intersectionPoint) -> bool
{
    return VAbstractCurve::CurveIntersectAxis(point, angle, curvePoints, intersectionPoint);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCurveIntersectAxis::GetFormulaAngle() const -> VFormula
{
    VFormula fAngle(formulaAngle, getData());
    fAngle.setCheckZero(false);
    fAngle.setToolId(m_id);
    fAngle.setPostfix(degreeSymbol);
    fAngle.Eval();
    return fAngle;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SetFormulaAngle(const VFormula &value)
{
    if (value.error() == false)
    {
        formulaAngle = value.GetFormula(FormulaType::FromUser);

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCurveIntersectAxis::CurveName() const -> QString
{
    return VAbstractTool::data.GetGObject(curveId)->ObjectName();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolCurveIntersectAxis>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogCurveIntersectAxis>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies,
                                         QList<quint32> &newDependencies)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogCurveIntersectAxis> dialogTool = qobject_cast<DialogCurveIntersectAxis *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    AddDependence(oldDependencies, basePointId);
    AddDependence(oldDependencies, curveId);
    AddDependence(newDependencies, dialogTool->GetBasePointId());
    AddDependence(newDependencies, dialogTool->getCurveId());

    doc->SetAttribute(domElement, AttrName, dialogTool->GetPointName());
    doc->SetAttribute(domElement, AttrTypeLine, dialogTool->GetTypeLine());
    doc->SetAttribute(domElement, AttrLineColor, dialogTool->GetLineColor());
    doc->SetAttribute(domElement, AttrAngle, dialogTool->GetAngle());
    doc->SetAttribute(domElement, AttrBasePoint, QString().setNum(dialogTool->GetBasePointId()));
    doc->SetAttribute(domElement, AttrCurve, QString().setNum(dialogTool->getCurveId()));
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrAlias1, dialogTool->GetAliasSuffix1(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrAlias2, dialogTool->GetAliasSuffix2(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrNotes, dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolLinePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrAngle, formulaAngle);
    doc->SetAttribute(tag, AttrBasePoint, basePointId);
    doc->SetAttribute(tag, AttrCurve, curveId);
    doc->SetAttributeOrRemoveIf<QString>(tag, AttrAlias1, m_aliasSuffix1,
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(tag, AttrAlias2, m_aliasSuffix2,
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::ReadToolAttributes(const QDomElement &domElement)
{
    VToolLinePoint::ReadToolAttributes(domElement);

    m_lineType = VAbstractPattern::GetParametrString(domElement, AttrTypeLine, TypeLineLine);
    lineColor = VAbstractPattern::GetParametrString(domElement, AttrLineColor, ColorBlack);
    basePointId = VAbstractPattern::GetParametrUInt(domElement, AttrBasePoint, NULL_ID_STR);
    curveId = VAbstractPattern::GetParametrUInt(domElement, AttrCurve, NULL_ID_STR);
    formulaAngle = VAbstractPattern::GetParametrString(domElement, AttrAngle, QString());
    m_aliasSuffix1 = VAbstractPattern::GetParametrEmptyString(domElement, AttrAlias1);
    m_aliasSuffix2 = VAbstractPattern::GetParametrEmptyString(domElement, AttrAlias2);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolCurveIntersectAxis *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetCurveId(curveId);
        visual->setAxisPointId(basePointId);
        visual->SetAngle(VAbstractApplication::VApp()->TrVars()->FormulaToUser(
            formulaAngle, VAbstractApplication::VApp()->Settings()->GetOsSeparator()));
        visual->SetLineStyle(LineStyleToPenStyle(m_lineType));
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCurveIntersectAxis::MakeToolTip() const -> QString
{
    const QSharedPointer<VPointF> first = VAbstractTool::data.GeometricObject<VPointF>(basePointId);
    const QSharedPointer<VPointF> second = VAbstractTool::data.GeometricObject<VPointF>(m_id);

    const QLineF line(static_cast<QPointF>(*first), static_cast<QPointF>(*second));

    const QString toolTip =
        u"<table>"
        u"<tr> <td><b>%6:</b> %7</td> </tr>"
        u"<tr> <td><b>%1:</b> %2 %3</td> </tr>"
        u"<tr> <td><b>%4:</b> %5°</td> </tr>"
        u"<tr> <td><b>%8:</b> %9</td> </tr>"
        u"<tr> <td><b>%10:</b> %11</td> </tr>"
        u"</table>"_s
            .arg(tr("Length"))                                                                   // 1
            .arg(VAbstractValApplication::VApp()->fromPixel(line.length()))                      // 2
            .arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true), tr("Angle")) // 3, 4
            .arg(line.angle())                                                                   // 5
            .arg(tr("Label"), second->name(),                                                    /* 6, 7 */
                 tr("Segment 1"), m_segments.first,                                              /* 8, 9 */
                 tr("Segment 2"), m_segments.second);                                            /* 10, 11 */
    return toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    VToolLinePoint::hoverEnterEvent(event);
    mainLine->SetBoldLine(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCurveIntersectAxis::SetSegments(const QPair<QString, QString> &segments)
{
    m_segments = segments;
}
