/************************************************************************
 **
 **  @file   vtoolspline.cpp
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

#include "vtoolspline.h"

#include <QDomElement>
#include <QEvent>
#include <QFlags>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QList>
#include <QPen>
#include <QPoint>
#include <QRectF>
#include <QSharedPointer>
#include <QVector>
#include <QtMath>

#include "../../../dialogs/tools/dialogspline.h"
#include "../../../dialogs/tools/dialogtool.h"
#include "../../../undocommands/movespline.h"
#include "../../../visualization/path/vistoolspline.h"
#include "../../../visualization/visualization.h"
#include "../../vabstracttool.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../qmuparser/qmutokenparser.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vmisc/exception/vexception.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/global.h"
#include "../vwidgets/vcontrolpointspline.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vtoolabstractcurve.h"

const QString VToolSpline::ToolType = QStringLiteral("simpleInteractive"); // NOLINT
const QString VToolSpline::OldToolType = QStringLiteral("simple");         // NOLINT

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolSpline constructor.
 * @param initData init data.
 * @param parent parent object.
 */
VToolSpline::VToolSpline(const VToolSplineInitData &initData, QGraphicsItem *parent)
  : VToolAbstractBezier(initData.doc, initData.data, initData.id, initData.notes, parent)
{
    SetSceneType(SceneObject::Spline);

    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    this->setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus

    const auto spl = VAbstractTool::data.GeometricObject<VSpline>(initData.id);

    auto InitControlPoint = [this](VControlPointSpline *cPoint) -> void
    {
        connect(cPoint, &VControlPointSpline::ControlPointChangePosition, this,
                &VToolSpline::ControlPointChangePosition);
        connect(this, &VToolSpline::setEnabledPoint, cPoint, &VControlPointSpline::setEnabledPoint);
        connect(cPoint, &VControlPointSpline::ShowContextMenu, this, &VToolSpline::contextMenuEvent);
        connect(cPoint, &VControlPointSpline::Released, this, &VToolSpline::CurveReleased);
        connect(cPoint, &VControlPointSpline::Selected, this, &VToolSpline::CurveSelected);
        controlPoints.append(cPoint);
    };

    const bool freeAngle1 = qmu::QmuTokenParser::IsSingle(spl->GetStartAngleFormula());
    const bool freeLength1 = qmu::QmuTokenParser::IsSingle(spl->GetC1LengthFormula());

    auto *controlPoint1 = new VControlPointSpline(1, SplinePointPosition::FirstPoint,
                                                  static_cast<QPointF>(spl->GetP2()), freeAngle1, freeLength1, this);
    InitControlPoint(controlPoint1);

    const bool freeAngle2 = qmu::QmuTokenParser::IsSingle(spl->GetEndAngleFormula());
    const bool freeLength2 = qmu::QmuTokenParser::IsSingle(spl->GetC2LengthFormula());

    auto *controlPoint2 = new VControlPointSpline(1, SplinePointPosition::LastPoint, static_cast<QPointF>(spl->GetP3()),
                                                  freeAngle2, freeLength2, this);
    InitControlPoint(controlPoint2);

    VToolSpline::RefreshCtrlPoints();

    VToolSpline::ShowHandles(IsDetailsMode());

    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolSpline::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogSpline> dialogTool = qobject_cast<DialogSpline *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const auto spl = VAbstractTool::data.GeometricObject<VSpline>(m_id);
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetSpline(*spl);
    dialogTool->SetNotes(m_notes);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool from GUI.
 * @param dialog dialog.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 * @return the created tool
 */
auto VToolSpline::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                         VContainer *data) -> VToolSpline *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogSpline> dialogTool = qobject_cast<DialogSpline *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VSpline const spline = dialogTool->GetSpline();

    VToolSplineInitData initData;
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();
    initData.point1 = spline.GetP1().id();
    initData.point4 = spline.GetP4().id();

    auto *spl = Create(initData, new VSpline(spline));

    if (spl != nullptr)
    {
        spl->m_dialog = dialog;
    }
    return spl;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool.
 * @param initData init data.
 * @param spline spline.
 * @return the created tool
 */
auto VToolSpline::Create(VToolSplineInitData &initData, VSpline *spline) -> VToolSpline *
{
    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->AddGObject(spline);
    }
    else
    {
        initData.data->UpdateGObject(initData.id, spline);
    }

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->AddVertex(initData.id, VNodeType::TOOL, initData.doc->PatternBlockMapper()->GetActiveId());

    const auto varData = initData.data->DataDependencyVariables();
    initData.doc->FindFormulaDependencies(initData.a1, initData.id, varData);
    initData.doc->FindFormulaDependencies(initData.a2, initData.id, varData);
    initData.doc->FindFormulaDependencies(initData.l1, initData.id, varData);
    initData.doc->FindFormulaDependencies(initData.l2, initData.id, varData);

    initData.data->AddSpline(initData.data->GeometricObject<VAbstractBezier>(initData.id), initData.id);

    patternGraph->AddEdge(initData.point1, initData.id);
    patternGraph->AddEdge(initData.point4, initData.id);

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::Spline, initData.doc);
        auto *_spl = new VToolSpline(initData);
        initData.scene->addItem(_spl);
        InitSplineToolConnections(initData.scene, _spl);
        VAbstractPattern::AddTool(initData.id, _spl);
        return _spl;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSpline::Create(VToolSplineInitData &initData) -> VToolSpline *
{
    const qreal calcAngle1 = CheckFormula(initData.id, initData.a1, initData.data);
    const qreal calcAngle2 = CheckFormula(initData.id, initData.a2, initData.data);

    const qreal calcLength1 =
        VAbstractValApplication::VApp()->toPixel(CheckFormula(initData.id, initData.l1, initData.data));
    const qreal calcLength2 =
        VAbstractValApplication::VApp()->toPixel(CheckFormula(initData.id, initData.l2, initData.data));

    auto p1 = initData.data->GeometricObject<VPointF>(initData.point1);
    auto p4 = initData.data->GeometricObject<VPointF>(initData.point4);

    auto *spline = new VSpline(*p1, *p4, calcAngle1, initData.a1, calcAngle2, initData.a2, calcLength1, initData.l1,
                               calcLength2, initData.l2);
    if (initData.duplicate > 0)
    {
        spline->SetDuplicate(initData.duplicate);
    }

    spline->SetColor(initData.color);
    spline->SetPenStyle(initData.penStyle);
    spline->SetApproximationScale(initData.approximationScale);
    spline->SetAliasSuffix(initData.aliasSuffix);

    return VToolSpline::Create(initData, spline);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSpline::getSpline() const -> VSpline
{
    auto spline = VAbstractTool::data.GeometricObject<VSpline>(m_id);
    return *spline.data();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::setSpline(const VSpline &spl)
{
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    QSharedPointer<VSpline> const spline = qSharedPointerDynamicCast<VSpline>(obj);
    *spline.data() = spl;
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolSpline>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::ShowHandles(bool show)
{
    for (auto *point : std::as_const(controlPoints))
    {
        point->setVisible(show);
    }
    VToolAbstractBezier::ShowHandles(show);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ControlPointChangePosition handle change position control point.
 * @param indexSpline position spline in spline list.
 * @param position position point in spline.
 * @param pos new position.
 */
void VToolSpline::ControlPointChangePosition(const qint32 &indexSpline, const SplinePointPosition &position,
                                             const QPointF &pos)
{
    Q_UNUSED(indexSpline)
    const QSharedPointer<VSpline> spline = VAbstractTool::data.GeometricObject<VSpline>(m_id);

    if (VAbstractApplication::VApp()->Settings()->IsFreeCurveMode() && not moved)
    {
        oldMoveSpline = QSharedPointer<VSpline>::create(*spline);
        moved = true;
    }

    const VSpline spl = CorrectedSpline(*spline, position, pos);

    if (not VAbstractApplication::VApp()->Settings()->IsFreeCurveMode())
    {
        UndoCommandMove(*spline, spl);
    }
    else
    {
        newMoveSpline = QSharedPointer<VSpline>::create(spl);
        VAbstractTool::data.UpdateGObject(m_id, newMoveSpline);
        RefreshGeometry();

        if (QGraphicsScene *sc = scene())
        {
            VMainGraphicsView::NewSceneRect(sc, VAbstractValApplication::VApp()->getSceneView(), this);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::EnableToolMove(bool move)
{
    this->setFlag(QGraphicsItem::ItemIsMovable, move);

    for (auto *point : std::as_const(controlPoints))
    {
        point->setFlag(QGraphicsItem::ItemIsMovable, move);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::AllowHover(bool enabled)
{
    VToolAbstractBezier::AllowHover(enabled);

    for (auto *point : std::as_const(controlPoints))
    {
        point->setAcceptHoverEvents(enabled);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::AllowSelecting(bool enabled)
{
    VToolAbstractBezier::AllowSelecting(enabled);

    for (auto *point : std::as_const(controlPoints))
    {
        point->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolSpline::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    auto *dialogTool = qobject_cast<DialogSpline *>(m_dialog);
    SCASSERT(dialogTool != nullptr)

    const VSpline spl = dialogTool->GetSpline();

    {
        const QSignalBlocker blocker0(controlPoints[0]);
        const QSignalBlocker blocker1(controlPoints[1]);

        controlPoints[0]->setPos(static_cast<QPointF>(spl.GetP2()));
        controlPoints[1]->setPos(static_cast<QPointF>(spl.GetP3()));
    }

    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrNotes, dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });

    SetSplineAttributes(domElement, spl);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolAbstractBezier::SaveOptions(tag, obj);

    auto spl = qSharedPointerDynamicCast<VSpline>(obj);
    SCASSERT(spl.isNull() == false)
    SetSplineAttributes(tag, *spl);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (flags() & QGraphicsItem::ItemIsMovable && event->button() == Qt::LeftButton &&
        event->type() != QEvent::GraphicsSceneMouseDoubleClick && IsMovable())
    {
        SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
        oldPosition = event->scenePos();
        event->accept();
    }
    VToolAbstractBezier::mousePressEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (flags() & QGraphicsItem::ItemIsMovable && event->button() == Qt::LeftButton &&
        event->type() != QEvent::GraphicsSceneMouseDoubleClick && IsMovable())
    {
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);

        CurveReleased();
    }
    VToolAbstractBezier::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!IsMovable())
    {
        return;
    }

    // Don't need check if left mouse button was pressed. According to the Qt documentation "If you do receive this
    // event, you can be certain that this item also received a mouse press event, and that this item is the current
    // mouse grabber.".

    // Magic Bezier Drag Equations follow!
    // "weight" describes how the influence of the drag should be distributed
    // among the handles; 0 = front handle only, 1 = back handle only.

    const auto spline = VAbstractTool::data.GeometricObject<VSpline>(m_id);

    if (VAbstractApplication::VApp()->Settings()->IsFreeCurveMode() && not moved)
    {
        oldMoveSpline = QSharedPointer<VSpline>::create(*spline);
        moved = true;
    }

    const qreal t = spline->ParamT(oldPosition);

    if (qFloor(t) == -1)
    {
        moved = false;
        return;
    }

    double weight;
    if (t <= 1.0 / 6.0)
    {
        weight = 0;
    }
    else if (t <= 0.5)
    {
        weight = (pow((6 * t - 1) / 2.0, 3)) / 2;
    }
    else if (t <= 5.0 / 6.0)
    {
        weight = (1 - pow((6 * (1 - t) - 1) / 2.0, 3)) / 2 + 0.5;
    }
    else
    {
        weight = 1;
    }

    const QPointF delta = event->scenePos() - oldPosition;
    const QPointF offset0 = ((1 - weight) / (3 * t * (1 - t) * (1 - t))) * delta;
    const QPointF offset1 = (weight / (3 * t * t * (1 - t))) * delta;

    const auto p2 = static_cast<QPointF>(spline->GetP2()) + offset0;
    const auto p3 = static_cast<QPointF>(spline->GetP3()) + offset1;

    oldPosition = event->scenePos(); // Now mouse here

    newMoveSpline = QSharedPointer<VSpline>::create(spline->GetP1(), p2, p3, spline->GetP4());

    if (not VAbstractApplication::VApp()->Settings()->IsFreeCurveMode())
    {
        UndoCommandMove(*spline, *newMoveSpline);
    }
    else
    {
        VAbstractTool::data.UpdateGObject(m_id, newMoveSpline);
        RefreshGeometry();

        if (QGraphicsScene *sc = scene())
        {
            VMainGraphicsView::NewSceneRect(sc, VAbstractValApplication::VApp()->getSceneView(), this);
        }
    }

    // Each time we move something we call recalculation scene rect. In some cases this can cause moving
    // objects positions. And this cause infinite redrawing. That's why we wait the finish of saving the last move.
    static bool changeFinished = true;
    if (changeFinished)
    {
        changeFinished = false;

        const QList<QGraphicsView *> viewList = scene()->views();
        if (not viewList.isEmpty())
        {
            if (auto *view = qobject_cast<VMainGraphicsView *>(viewList.at(0)))
            {
                auto *currentScene = qobject_cast<VMainGraphicsScene *>(scene());
                SCASSERT(currentScene)
                const QPointF cursorPosition = currentScene->getScenePos();
                const qreal scale = SceneScale(scene());
                view->EnsureVisibleWithDelay(
                    QRectF(cursorPosition.x() - 5 / scale, cursorPosition.y() - 5 / scale, 10 / scale, 10 / scale),
                    VMainGraphicsView::scrollDelay);
            }
        }
        changeFinished = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (GetAcceptHoverEvents())
    {
        if (flags() & QGraphicsItem::ItemIsMovable)
        {
            if (IsMovable())
            {
                SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
            }
            else
            {
                setCursor(VAbstractValApplication::VApp()->getSceneView()->viewport()->cursor());
            }
        }
        else
        {
            setCursor(VAbstractValApplication::VApp()->getSceneView()->viewport()->cursor());
        }

        VToolAbstractBezier::hoverEnterEvent(event);
    }
    else
    {
        setCursor(VAbstractValApplication::VApp()->getSceneView()->viewport()->cursor());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (GetAcceptHoverEvents())
    {
        VToolAbstractBezier::hoverLeaveEvent(event);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolSpline *>(vis);
        SCASSERT(visual != nullptr)

        const QSharedPointer<VSpline> spl = VAbstractTool::data.GeometricObject<VSpline>(m_id);
        visual->SetPoint1Id(spl->GetP1().id());
        visual->SetPoint4Id(spl->GetP4().id());
        visual->SetAngle1(spl->GetStartAngle());
        visual->SetAngle2(spl->GetEndAngle());
        visual->SetKAsm1(spl->GetKasm1());
        visual->SetKAsm2(spl->GetKasm2());
        visual->SetKCurve(spl->GetKcurve());
        visual->SetLineStyle(LineStyleToPenStyle(spl->GetPenStyle()));
        visual->SetApproximationScale(spl->GetApproximationScale());
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSpline::IsMovable() const -> bool
{
    const auto spline = VAbstractTool::data.GeometricObject<VSpline>(m_id);

    return qmu::QmuTokenParser::IsSingle(spline->GetStartAngleFormula()) &&
           qmu::QmuTokenParser::IsSingle(spline->GetEndAngleFormula()) &&
           qmu::QmuTokenParser::IsSingle(spline->GetC1LengthFormula()) &&
           qmu::QmuTokenParser::IsSingle(spline->GetC2LengthFormula());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::RefreshCtrlPoints()
{
    // Very important to disable control points. Without it the pogram can't move the curve.
    for (auto *point : std::as_const(controlPoints))
    {
        point->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    }

    const auto spl = VAbstractTool::data.GeometricObject<VSpline>(m_id);

    {
        const QSignalBlocker blocker0(controlPoints[0]);
        const QSignalBlocker blocker1(controlPoints[1]);

        {
            const bool freeAngle1 = qmu::QmuTokenParser::IsSingle(spl->GetStartAngleFormula());
            const bool freeLength1 = qmu::QmuTokenParser::IsSingle(spl->GetC1LengthFormula());

            const auto splinePoint = static_cast<QPointF>(
                *VAbstractTool::data.GeometricObject<VPointF>(spl->GetP1().id()));
            controlPoints[0]->RefreshCtrlPoint(1,
                                               SplinePointPosition::FirstPoint,
                                               static_cast<QPointF>(spl->GetP2()),
                                               static_cast<QPointF>(splinePoint),
                                               freeAngle1,
                                               freeLength1);
        }

        {
            const bool freeAngle2 = qmu::QmuTokenParser::IsSingle(spl->GetEndAngleFormula());
            const bool freeLength2 = qmu::QmuTokenParser::IsSingle(spl->GetC2LengthFormula());

            const auto splinePoint = static_cast<QPointF>(
                *VAbstractTool::data.GeometricObject<VPointF>(spl->GetP4().id()));
            controlPoints[1]->RefreshCtrlPoint(1,
                                               SplinePointPosition::LastPoint,
                                               static_cast<QPointF>(spl->GetP3()),
                                               static_cast<QPointF>(splinePoint),
                                               freeAngle2,
                                               freeLength2);
        }
    }

    for (auto *point : std::as_const(controlPoints))
    {
        point->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    ProcessSplineToolOptions(oldDomElement, newDomElement, GatherToolChanges());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::CurveReleased()
{
    if (VAbstractApplication::VApp()->Settings()->IsFreeCurveMode() && moved)
    {
        UndoCommandMove(*oldMoveSpline, *newMoveSpline);

        oldMoveSpline.clear();
        newMoveSpline.clear();

        moved = false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    Q_UNUSED(id)
    try
    {
        ContextMenu<DialogSpline>(event);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::CurveSelected(bool selected)
{
    setSelected(selected);

    for (auto *point : std::as_const(controlPoints))
    {
        const QSignalBlocker blocker(point);
        point->setSelected(selected);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::SetSplineAttributes(QDomElement &domElement, const VSpline &spl)
{
    SCASSERT(doc != nullptr)

    doc->SetAttribute(domElement, AttrType, ToolType);
    doc->SetAttribute(domElement, AttrPoint1, spl.GetP1().id());
    doc->SetAttribute(domElement, AttrPoint4, spl.GetP4().id());
    doc->SetAttribute(domElement, AttrAngle1, spl.GetStartAngleFormula());
    doc->SetAttribute(domElement, AttrAngle2, spl.GetEndAngleFormula());
    doc->SetAttribute(domElement, AttrLength1, spl.GetC1LengthFormula());
    doc->SetAttribute(domElement, AttrLength2, spl.GetC2LengthFormula());
    doc->SetAttribute(domElement, AttrColor, spl.GetColor());
    doc->SetAttribute(domElement, AttrPenStyle, spl.GetPenStyle());
    doc->SetAttribute(domElement, AttrAScale, spl.GetApproximationScale());
    doc->SetAttributeOrRemoveIf<quint32>(domElement, AttrDuplicate, spl.GetDuplicate(),
                                         [](quint32 duplicate) noexcept { return duplicate == 0; });
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrAlias, spl.GetAliasSuffix(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });

    if (domElement.hasAttribute(AttrKCurve))
    {
        domElement.removeAttribute(AttrKCurve);
    }

    if (domElement.hasAttribute(AttrKAsm1))
    {
        domElement.removeAttribute(AttrKAsm1);
    }

    if (domElement.hasAttribute(AttrKAsm2))
    {
        domElement.removeAttribute(AttrKAsm2);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSpline::UndoCommandMove(const VSpline &oldSpl, const VSpline &newSpl)
{
    auto *moveSpl = new MoveSpline(doc, oldSpl, newSpl, m_id);
    connect(moveSpl, &MoveSpline::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    VAbstractApplication::VApp()->getUndoStack()->push(moveSpl);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSpline::GatherToolChanges() const -> VToolAbstractBezier::ToolChanges
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogSpline> dialogTool = qobject_cast<DialogSpline *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    const VSpline newCurve = dialogTool->GetSpline();
    const auto oldCurve = VAbstractTool::data.GeometricObject<VAbstractCubicBezier>(m_id);

    return {.oldP1Label = oldCurve->GetP1().name(),
            .newP1Label = newCurve.GetP1().name(),
            .oldP4Label = oldCurve->GetP4().name(),
            .newP4Label = newCurve.GetP4().name(),
            .oldAliasSuffix = oldCurve->GetAliasSuffix(),
            .newAliasSuffix = newCurve.GetAliasSuffix()};
}
