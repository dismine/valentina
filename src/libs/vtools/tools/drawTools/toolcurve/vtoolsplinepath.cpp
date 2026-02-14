/************************************************************************
 **
 **  @file   vtoolsplinepath.cpp
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

#include "vtoolsplinepath.h"

#include <QDomElement>
#include <QEvent>
#include <QFlags>
#include <QGraphicsScene>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QList>
#include <QPen>
#include <QPoint>
#include <QRectF>
#include <QSharedPointer>
#include <QUndoStack>
#include <QtMath>

#include "../../../dialogs/tools/dialogsplinepath.h"
#include "../../../dialogs/tools/dialogtool.h"
#include "../../../undocommands/movesplinepath.h"
#include "../../../undocommands/vundocommand.h"
#include "../../../visualization/path/vistoolsplinepath.h"
#include "../../../visualization/visualization.h"
#include "../../vabstracttool.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vdomdocument.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../qmuparser/qmutokenparser.h"
#include "../vgeometry/vabstractcubicbezierpath.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepoint.h"
#include "../vmisc/exception/vexception.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/../vgeometry/vsplinepath.h"
#include "../vwidgets/global.h"
#include "../vwidgets/vcontrolpointspline.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vtoolabstractcurve.h"

const QString VToolSplinePath::ToolType = QStringLiteral("pathInteractive");
const QString VToolSplinePath::OldToolType = QStringLiteral("path");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolSplinePath constructor.
 * @param initData init data.
 * @param parent parent object.
 */
VToolSplinePath::VToolSplinePath(const VToolSplinePathInitData &initData, QGraphicsItem *parent)
  : VToolAbstractBezier(initData.doc, initData.data, initData.id, initData.notes, parent)
{
    SetSceneType(SceneObject::SplinePath);

    this->setFlag(QGraphicsItem::ItemIsMovable, true);
    this->setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus

    const QSharedPointer<VSplinePath> splPath = initData.data->GeometricObject<VSplinePath>(initData.id);
    InitControlPoints(splPath.data());
    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolSplinePath::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogSplinePath> dialogTool = qobject_cast<DialogSplinePath *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VSplinePath> splPath = VAbstractTool::data.GeometricObject<VSplinePath>(m_id);
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetPath(*splPath);
    dialogTool->SetNotes(m_notes);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool from GUI.
 * @param dialog dialog.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 */
auto VToolSplinePath::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                             VContainer *data) -> VToolSplinePath *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogSplinePath> dialogTool = qobject_cast<DialogSplinePath *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolSplinePathInitData initData;
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();

    auto *path = new VSplinePath(dialogTool->GetPath());

    initData.points.reserve(path->CountPoints());
    for (qint32 i = 0; i < path->CountPoints(); ++i)
    {
        VPointF const point = (*path)[i].P();
        initData.points.append(point.id());
    }

    VToolSplinePath *spl = Create(initData, path);
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
 * @param path spline path.
 */
auto VToolSplinePath::Create(VToolSplinePathInitData &initData, VSplinePath *path) -> VToolSplinePath *
{
    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->AddGObject(path);
    }
    else
    {
        initData.data->UpdateGObject(initData.id, path);
    }

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->AddVertex(initData.id, VNodeType::TOOL, initData.doc->PatternBlockMapper()->GetActiveId());

    const auto varData = initData.data->DataDependencyVariables();

    for (int i = 0; i < initData.points.size(); ++i)
    {
        initData.doc->FindFormulaDependencies(initData.a1.value(i), initData.id, varData);
        initData.doc->FindFormulaDependencies(initData.a2.value(i), initData.id, varData);
        initData.doc->FindFormulaDependencies(initData.l1.value(i), initData.id, varData);
        initData.doc->FindFormulaDependencies(initData.l2.value(i), initData.id, varData);
    }

    initData.data->AddCurveWithSegments(initData.data->GeometricObject<VAbstractCubicBezierPath>(initData.id),
                                        initData.id);

    for (const auto &pId : std::as_const(initData.points))
    {
        patternGraph->AddEdge(pId, initData.id);
    }

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::SplinePath, initData.doc);
        auto *spl = new VToolSplinePath(initData);
        initData.scene->addItem(spl);
        InitSplinePathToolConnections(initData.scene, spl);
        VAbstractPattern::AddTool(initData.id, spl);
        return spl;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSplinePath::Create(VToolSplinePathInitData &initData) -> VToolSplinePath *
{
    auto *path = new VSplinePath();

    if (initData.duplicate > 0)
    {
        path->SetDuplicate(initData.duplicate);
    }

    for (int i = 0; i < initData.points.size(); ++i)
    {
        const qreal calcAngle1 = CheckFormula(initData.id, initData.a1[i], initData.data);
        const qreal calcAngle2 = CheckFormula(initData.id, initData.a2[i], initData.data);

        const qreal calcLength1 =
            VAbstractValApplication::VApp()->toPixel(CheckFormula(initData.id, initData.l1[i], initData.data));
        const qreal calcLength2 =
            VAbstractValApplication::VApp()->toPixel(CheckFormula(initData.id, initData.l2[i], initData.data));

        const auto p = *initData.data->GeometricObject<VPointF>(initData.points.at(i));

        path->append(VSplinePoint(p, calcAngle1, initData.a1.at(i), calcAngle2, initData.a2.at(i), calcLength1,
                                  initData.l1.at(i), calcLength2, initData.l2.at(i)));
    }

    path->SetColor(initData.color);
    path->SetPenStyle(initData.penStyle);
    path->SetApproximationScale(initData.approximationScale);
    path->SetAliasSuffix(initData.aliasSuffix);

    return VToolSplinePath::Create(initData, path);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief ControlPointChangePosition handle change position control point.
 * @param indexSpline position spline in spline list.
 * @param position position point in spline.
 * @param pos new position.
 */
void VToolSplinePath::ControlPointChangePosition(const qint32 &indexSpline, const SplinePointPosition &position,
                                                 const QPointF &pos)
{
    const QSharedPointer<VSplinePath> oldSplPath = VAbstractTool::data.GeometricObject<VSplinePath>(m_id);

    if (VAbstractApplication::VApp()->Settings()->IsFreeCurveMode() && not moved)
    {
        oldMoveSplinePath = QSharedPointer<VSplinePath>::create(*oldSplPath);
        moved = true;
    }

    auto newSplPath = QSharedPointer<VSplinePath>::create(*oldSplPath);
    const VSpline spl = CorrectedSpline(newSplPath->GetSpline(indexSpline), position, pos);

    UpdateControlPoints(spl, newSplPath, indexSpline);

    if (not VAbstractApplication::VApp()->Settings()->IsFreeCurveMode())
    {
        UndoCommandMove(*oldSplPath, *newSplPath);
    }
    else
    {
        newMoveSplinePath = newSplPath;
        VAbstractTool::data.UpdateGObject(m_id, newMoveSplinePath);
        RefreshGeometry();

        if (QGraphicsScene *sc = scene())
        {
            VMainGraphicsView::NewSceneRect(sc, VAbstractValApplication::VApp()->getSceneView(), this);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSplinePath::EnableToolMove(bool move)
{
    this->setFlag(QGraphicsItem::ItemIsMovable, move);

    for (auto *point : std::as_const(controlPoints))
    {
        point->setFlag(QGraphicsItem::ItemIsMovable, move);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSplinePath::AllowHover(bool enabled)
{
    VToolAbstractBezier::AllowHover(enabled);

    for (auto *point : std::as_const(controlPoints))
    {
        point->setAcceptHoverEvents(enabled);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSplinePath::AllowSelecting(bool enabled)
{
    VToolAbstractBezier::AllowSelecting(enabled);

    for (auto *point : std::as_const(controlPoints))
    {
        point->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSplinePath::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    Q_UNUSED(id)
    try
    {
        ContextMenu<DialogSplinePath>(event);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSplinePath::CurveSelected(bool selected)
{
    setSelected(selected);

    for (auto *point : std::as_const(controlPoints))
    {
        const QSignalBlocker blocker(point);
        point->setSelected(selected);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UpdateControlPoints update position control points in file.
 * @param spl spline that was changed.
 * @param splPath spline path.
 * @param indexSpline index spline in spline path.
 */
void VToolSplinePath::UpdateControlPoints(const VSpline &spl, QSharedPointer<VSplinePath> &splPath,
                                          qint32 indexSpline) const
{
    VSplinePoint p = splPath->GetSplinePoint(indexSpline, SplinePointPosition::FirstPoint);

    if (indexSpline > 1)
    {
        VSpline const prevSpline = splPath->GetSpline(indexSpline - 1);
        if (qmu::QmuTokenParser::IsSingle(prevSpline.GetEndAngleFormula()))
        {
            p.SetAngle2(spl.GetStartAngle(), spl.GetStartAngleFormula());
        }
    }
    else
    {
        p.SetAngle2(spl.GetStartAngle(), spl.GetStartAngleFormula());
    }

    p.SetLength2(spl.GetC1Length(), spl.GetC1LengthFormula());
    splPath->UpdatePoint(indexSpline, SplinePointPosition::FirstPoint, p);

    p = splPath->GetSplinePoint(indexSpline, SplinePointPosition::LastPoint);

    if (indexSpline < splPath->CountSubSpl())
    {
        VSpline const nextSpline = splPath->GetSpline(indexSpline + 1);
        if (qmu::QmuTokenParser::IsSingle(nextSpline.GetStartAngleFormula()))
        {
            p.SetAngle1(spl.GetEndAngle(), spl.GetEndAngleFormula());
        }
    }
    else
    {
        p.SetAngle1(spl.GetEndAngle(), spl.GetEndAngleFormula());
    }

    p.SetLength1(spl.GetC2Length(), spl.GetC2LengthFormula());
    splPath->UpdatePoint(indexSpline, SplinePointPosition::LastPoint, p);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSplinePath::SetSplinePathAttributes(QDomElement &domElement, const VSplinePath &path)
{
    doc->SetAttribute(domElement, AttrType, ToolType);
    doc->SetAttributeOrRemoveIf<quint32>(domElement, AttrDuplicate, path.GetDuplicate(),
                                         [](quint32 duplicate) noexcept { return duplicate == 0; });

    if (domElement.hasAttribute(AttrKCurve))
    {
        domElement.removeAttribute(AttrKCurve);
    }

    doc->SetAttribute(domElement, AttrColor, path.GetColor());
    doc->SetAttribute(domElement, AttrPenStyle, path.GetPenStyle());
    doc->SetAttribute(domElement, AttrAScale, path.GetApproximationScale());
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrAlias, path.GetAliasSuffix(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });

    UpdatePathPoints(doc, domElement, path);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSplinePath::UndoCommandMove(const VSplinePath &oldPath, const VSplinePath &newPath)
{
    auto *moveSplPath = new MoveSplinePath(doc, oldPath, newPath, m_id);
    connect(moveSplPath, &VUndoCommand::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    VAbstractApplication::VApp()->getUndoStack()->push(moveSplPath);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSplinePath::InitControlPoints(const VSplinePath *splPath)
{
    auto InitControlPoint = [this](VControlPointSpline *cPoint)
    {
        connect(cPoint, &VControlPointSpline::ControlPointChangePosition, this,
                &VToolSplinePath::ControlPointChangePosition);
        connect(this, &VToolSplinePath::setEnabledPoint, cPoint, &VControlPointSpline::setEnabledPoint);
        connect(cPoint, &VControlPointSpline::ShowContextMenu, this, &VToolSplinePath::contextMenuEvent);
        connect(cPoint, &VControlPointSpline::Released, this, &VToolSplinePath::CurveReleased);
        connect(cPoint, &VControlPointSpline::Selected, this, &VToolSplinePath::CurveSelected);
        controlPoints.append(cPoint);
    };

    for (qint32 i = 1; i <= splPath->CountSubSpl(); ++i)
    {
        const VSpline spl = splPath->GetSpline(i);

        bool freeAngle1 = true;

        if (i > 1)
        {
            const VSpline prevSpl = splPath->GetSpline(i - 1);
            freeAngle1 = qmu::QmuTokenParser::IsSingle(spl.GetStartAngleFormula()) &&
                         qmu::QmuTokenParser::IsSingle(prevSpl.GetEndAngleFormula());
        }
        else
        {
            freeAngle1 = qmu::QmuTokenParser::IsSingle(spl.GetStartAngleFormula());
        }

        const bool freeLength1 = qmu::QmuTokenParser::IsSingle(spl.GetC1LengthFormula());

        auto *controlPoint = new VControlPointSpline(i, SplinePointPosition::FirstPoint,
                                                     static_cast<QPointF>(spl.GetP2()), freeAngle1, freeLength1, this);
        InitControlPoint(controlPoint);

        bool freeAngle2 = true;

        if (i < splPath->CountSubSpl())
        {
            const VSpline nextSpl = splPath->GetSpline(i + 1);
            freeAngle2 = qmu::QmuTokenParser::IsSingle(nextSpl.GetStartAngleFormula()) &&
                         qmu::QmuTokenParser::IsSingle(spl.GetEndAngleFormula());
        }
        else
        {
            freeAngle2 = qmu::QmuTokenParser::IsSingle(spl.GetEndAngleFormula());
        }

        const bool freeLength2 = qmu::QmuTokenParser::IsSingle(spl.GetC2LengthFormula());

        controlPoint = new VControlPointSpline(i, SplinePointPosition::LastPoint, static_cast<QPointF>(spl.GetP3()),
                                               freeAngle2, freeLength2, this);
        InitControlPoint(controlPoint);
    }

    VToolSplinePath::RefreshCtrlPoints();
    VToolSplinePath::ShowHandles(IsDetailsMode());
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSplinePath::GatherToolChanges() const -> VToolAbstractBezier::ToolChanges
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogSplinePath> dialogTool = qobject_cast<DialogSplinePath *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    const VSplinePath newCurve = dialogTool->GetPath();
    const auto oldCurve = VAbstractTool::data.GeometricObject<VAbstractCubicBezierPath>(m_id);

    return {.oldP1Label = oldCurve->FirstPoint().name(),
            .newP1Label = newCurve.FirstPoint().name(),
            .oldP4Label = oldCurve->LastPoint().name(),
            .newP4Label = newCurve.LastPoint().name(),
            .oldAliasSuffix = oldCurve->GetAliasSuffix(),
            .newAliasSuffix = newCurve.GetAliasSuffix()};
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UpdatePathPoints update spline path in pattern file.
 * @param doc dom document container.
 * @param element tag in file.
 * @param path spline path.
 */
void VToolSplinePath::UpdatePathPoints(VAbstractPattern *doc, QDomElement &element, const VSplinePath &path)
{
    VDomDocument::RemoveAllChildren(element);
    for (qint32 i = 0; i < path.CountPoints(); ++i)
    {
        AddPathPoint(doc, element, path.at(i));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSplinePath::getSplinePath() const -> VSplinePath
{
    QSharedPointer<VSplinePath> const splPath = VAbstractTool::data.GeometricObject<VSplinePath>(m_id);
    return *splPath.data();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSplinePath::setSplinePath(const VSplinePath &splPath)
{
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    QSharedPointer<VSplinePath> const splinePath = qSharedPointerDynamicCast<VSplinePath>(obj);
    *splinePath.data() = splPath;
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSplinePath::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolSplinePath>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSplinePath::ShowHandles(bool show)
{
    for (auto *point : std::as_const(controlPoints))
    {
        point->setVisible(show);
    }
    VToolAbstractBezier::ShowHandles(show);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddPathPoint write path point to pattern file.
 * @param domElement dom element.
 * @param splPoint spline path point.
 */
void VToolSplinePath::AddPathPoint(VAbstractPattern *doc, QDomElement &domElement, const VSplinePoint &splPoint)
{
    SCASSERT(doc != nullptr)
    QDomElement pathPoint = doc->createElement(TagPathPoint);

    doc->SetAttribute(pathPoint, AttrPSpline, splPoint.P().id());
    doc->SetAttribute(pathPoint, AttrLength1, splPoint.Length1Formula());
    doc->SetAttribute(pathPoint, AttrLength2, splPoint.Length2Formula());
    doc->SetAttribute(pathPoint, AttrAngle1, splPoint.Angle1Formula());
    doc->SetAttribute(pathPoint, AttrAngle2, splPoint.Angle2Formula());

    if (domElement.hasAttribute(AttrKAsm1))
    {
        domElement.removeAttribute(AttrKAsm1);
    }

    if (domElement.hasAttribute(AttrKAsm2))
    {
        domElement.removeAttribute(AttrKAsm2);
    }

    if (domElement.hasAttribute(AttrAngle))
    {
        domElement.removeAttribute(AttrAngle);
    }

    domElement.appendChild(pathPoint);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolSplinePath::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogSplinePath> dialogTool = qobject_cast<DialogSplinePath *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    const VSplinePath splPath = dialogTool->GetPath();

    qDeleteAll(controlPoints);
    controlPoints.clear();
    InitControlPoints(&splPath);

    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrNotes, dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });

    SetSplinePathAttributes(domElement, splPath);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSplinePath::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolAbstractBezier::SaveOptions(tag, obj);

    QSharedPointer<VSplinePath> const splPath = qSharedPointerDynamicCast<VSplinePath>(obj);
    SCASSERT(splPath.isNull() == false)

    SetSplinePathAttributes(tag, *splPath);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSplinePath::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (flags() & QGraphicsItem::ItemIsMovable && event->button() == Qt::LeftButton &&
        event->type() != QEvent::GraphicsSceneMouseDoubleClick)
    {
        oldPosition = event->scenePos();
        const auto splPath = VAbstractTool::data.GeometricObject<VSplinePath>(m_id);
        splIndex = splPath->Segment(oldPosition);
        if (IsMovable(splIndex))
        {
            SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
            event->accept();
        }
    }
    VToolAbstractBezier::mousePressEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSplinePath::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (flags() & QGraphicsItem::ItemIsMovable && event->button() == Qt::LeftButton &&
        event->type() != QEvent::GraphicsSceneMouseDoubleClick)
    {
        oldPosition = event->scenePos();
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);

        CurveReleased();
    }
    VToolAbstractBezier::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSplinePath::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // Don't need to check if left mouse button was pressed. According to the Qt documentation "If you do receive this
    // event, you can be certain that this item also received a mouse press event, and that this item is the current
    // mouse grabber.".

    if (IsMovable(splIndex))
    {
        const auto oldSplPath = VAbstractTool::data.GeometricObject<VSplinePath>(m_id);

        if (VAbstractApplication::VApp()->Settings()->IsFreeCurveMode() && not moved)
        {
            oldMoveSplinePath = QSharedPointer<VSplinePath>::create(*oldSplPath);
            moved = true;
        }

        newMoveSplinePath = QSharedPointer<VSplinePath>::create(*oldSplPath);

        VSpline const spline = newMoveSplinePath->GetSpline(splIndex);
        const qreal t = spline.ParamT(oldPosition);

        if (qFloor(t) == -1)
        {
            return;
        }

        // Magic Bezier Drag Equations follow!
        // "weight" describes how the influence of the drag should be distributed
        // among the handles; 0 = front handle only, 1 = back handle only.

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

        const auto p2 = static_cast<QPointF>(spline.GetP2()) + offset0;
        const auto p3 = static_cast<QPointF>(spline.GetP3()) + offset1;

        oldPosition = event->scenePos(); // Now mouse here

        const auto spl = VSpline(spline.GetP1(), p2, p3, spline.GetP4());

        UpdateControlPoints(spl, newMoveSplinePath, splIndex);

        if (not VAbstractApplication::VApp()->Settings()->IsFreeCurveMode())
        {
            UndoCommandMove(*oldSplPath, *newMoveSplinePath);
        }
        else
        {
            VAbstractTool::data.UpdateGObject(m_id, newMoveSplinePath);
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
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSplinePath::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (GetAcceptHoverEvents())
    {
        if (flags() & QGraphicsItem::ItemIsMovable)
        {
            oldPosition = event->scenePos();
            const auto splPath = VAbstractTool::data.GeometricObject<VSplinePath>(m_id);
            splIndex = splPath->Segment(oldPosition);
            if (IsMovable(splIndex))
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
void VToolSplinePath::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (GetAcceptHoverEvents())
    {
        VToolAbstractBezier::hoverLeaveEvent(event);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSplinePath::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolSplinePath *>(vis);
        SCASSERT(visual != nullptr)

        QSharedPointer<VSplinePath> const splPath = VAbstractTool::data.GeometricObject<VSplinePath>(m_id);
        visual->SetPath(*splPath.data());
        visual->SetLineStyle(LineStyleToPenStyle(splPath->GetPenStyle()));
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSplinePath::IsMovable(int index) const -> bool
{
    const auto splPath = VAbstractTool::data.GeometricObject<VSplinePath>(m_id);

    if (index < 1 || index > splPath->CountSubSpl())
    {
        return false;
    }

    const VSplinePoint p1 = splPath->GetSplinePoint(index, SplinePointPosition::FirstPoint);
    const VSplinePoint p2 = splPath->GetSplinePoint(index, SplinePointPosition::LastPoint);

    return p1.IsMovable() && p2.IsMovable();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSplinePath::RefreshCtrlPoints()
{
    // Very important to disable control points. Without it the pogram can't move the curve.
    for (auto *point : std::as_const(controlPoints))
    {
        point->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    }

    const auto splPath = VAbstractTool::data.GeometricObject<VSplinePath>(m_id);

    for (qint32 i = 1; i <= splPath->CountSubSpl(); ++i)
    {
        const qint32 j = i * 2;

        if (j - 2 >= controlPoints.size())
        {
            break;
        }

        const QSignalBlocker blockerJ2(controlPoints[j - 2]);
        const QSignalBlocker blockerJ1(controlPoints[j - 1]);

        const auto spl = splPath->GetSpline(i);

        {
            bool freeAngle1 = true;

            if (i > 1)
            {
                const VSpline prevSpl = splPath->GetSpline(i - 1);
                freeAngle1 = qmu::QmuTokenParser::IsSingle(spl.GetStartAngleFormula()) &&
                             qmu::QmuTokenParser::IsSingle(prevSpl.GetEndAngleFormula());
            }
            else
            {
                freeAngle1 = qmu::QmuTokenParser::IsSingle(spl.GetStartAngleFormula());
            }

            const bool freeLength1 = qmu::QmuTokenParser::IsSingle(spl.GetC1LengthFormula());

            const auto splinePoint = spl.GetP1();
            controlPoints[j - 2]->RefreshCtrlPoint(i, SplinePointPosition::FirstPoint,
                                                   static_cast<QPointF>(spl.GetP2()), static_cast<QPointF>(splinePoint),
                                                   freeAngle1, freeLength1);
        }

        {
            bool freeAngle2 = true;

            if (i < splPath->CountSubSpl())
            {
                const VSpline nextSpl = splPath->GetSpline(i + 1);
                freeAngle2 = qmu::QmuTokenParser::IsSingle(nextSpl.GetStartAngleFormula()) &&
                             qmu::QmuTokenParser::IsSingle(spl.GetEndAngleFormula());
            }
            else
            {
                freeAngle2 = qmu::QmuTokenParser::IsSingle(spl.GetEndAngleFormula());
            }

            const bool freeLength2 = qmu::QmuTokenParser::IsSingle(spl.GetC2LengthFormula());

            const auto splinePoint = spl.GetP4();
            controlPoints[j - 1]->RefreshCtrlPoint(i, SplinePointPosition::LastPoint, static_cast<QPointF>(spl.GetP3()),
                                                   static_cast<QPointF>(splinePoint), freeAngle2, freeLength2);
        }
    }

    for (auto *point : std::as_const(controlPoints))
    {
        point->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSplinePath::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    ProcessSplinePathToolOptions(oldDomElement, newDomElement, GatherToolChanges());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSplinePath::CurveReleased()
{
    if (VAbstractApplication::VApp()->Settings()->IsFreeCurveMode() && moved)
    {
        UndoCommandMove(*oldMoveSplinePath, *newMoveSplinePath);

        oldMoveSplinePath.clear();
        newMoveSplinePath.clear();

        moved = false;
    }
}
