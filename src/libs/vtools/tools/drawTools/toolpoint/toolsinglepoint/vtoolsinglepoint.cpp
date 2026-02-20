/************************************************************************
 **
 **  @file   vtoolsinglepoint.cpp
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

#include "vtoolsinglepoint.h"

#include <QBrush>
#include <QFlags>
#include <QFont>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QPen>
#include <QPoint>
#include <QRectF>
#include <QSharedPointer>
#include <QUndoStack>

#include "../../../../undocommands/label/movelabel.h"
#include "../../../../undocommands/label/showlabel.h"
#include "../../../vabstracttool.h"
#include "../../vdrawtool.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../vabstractpoint.h"
#include "../vgeometry/vabstractcubicbezier.h"
#include "../vgeometry/vabstractcubicbezierpath.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepath.h"
#include "../vmisc/exception/vexception.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/global.h"
#include "../vwidgets/scalesceneitems.h"
#include "../vwidgets/vgraphicssimpletextitem.h"
#include "toolcut/vtoolcutsplinepath.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(vToolSinglePoint, "v.toolSinglePoint") // NOLINT

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolSinglePoint constructor.
 * @param doc dom document container.
 * @param data container with variables.
 * @param id object id in container.
 * @param parent parent object.
 */
VToolSinglePoint::VToolSinglePoint(VAbstractPattern *doc, VContainer *data, quint32 id, const QString &notes,
                                   QGraphicsItem *parent)
  : VAbstractPoint(doc, data, id, notes),
    VScenePoint(VColorRole::PatternColor, parent)
{
    connect(m_namePoint, &VGraphicsSimpleTextItem::ShowContextMenu, this, &VToolSinglePoint::contextMenuEvent);
    connect(m_namePoint, &VGraphicsSimpleTextItem::DeleteTool, this, &VToolSinglePoint::DeleteFromLabel);
    connect(m_namePoint, &VGraphicsSimpleTextItem::PointChoosed, this, &VToolSinglePoint::PointChoosed);
    connect(m_namePoint, &VGraphicsSimpleTextItem::PointSelected, this, &VToolSinglePoint::PointSelected);
    connect(m_namePoint, &VGraphicsSimpleTextItem::NameChangePosition, this, &VToolSinglePoint::NameChangePosition);
    RefreshPointGeometry(*VAbstractTool::data.GeometricObject<VPointF>(id));
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSinglePoint::name() const -> QString
{
    return ObjectName<VPointF>(m_id);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::setName(const QString &name)
{
    UpdatePointName(m_id,
                    name,
                    [&](const QDomElement &oldElem, const QDomElement &newElem, const ToolChanges &changes) -> void
                    { ProcessPointToolOptions(oldElem, newElem, changes); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::GroupVisibility(quint32 object, bool visible)
{
    Q_UNUSED(object)
    setVisible(visible);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSinglePoint::IsLabelVisible(quint32 id) const -> bool
{
    if (m_id == id)
    {
        const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(id);
        return point->IsShowLabel();
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::SetLabelVisible(quint32 id, bool visible)
{
    if (m_id == id)
    {
        const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(id);
        point->SetShowLabel(visible);
        RefreshPointGeometry(*point);
        if (QGraphicsScene *sc = scene())
        {
            VMainGraphicsView::NewSceneRect(sc, VAbstractValApplication::VApp()->getSceneView(), this);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief NameChangePosition handle change posion point label.
 * @param pos new position.
 */
void VToolSinglePoint::NameChangePosition(const QPointF &pos)
{
    UpdateNamePosition(m_id, pos - this->pos());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief UpdateNamePosition save new position label to the pattern file.
 */
void VToolSinglePoint::UpdateNamePosition(quint32 id, const QPointF &pos)
{
    if (id == m_id)
    {
        VAbstractApplication::VApp()->getUndoStack()->push(new MoveLabel(doc, pos, id));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Special for not selectable item first need to call standard mousePressEvent then accept event
    VScenePoint::mousePressEvent(event);

    // Somehow clicking on notselectable object do not clean previous selections.
    if (not(flags() & ItemIsSelectable) && scene())
    {
        scene()->clearSelection();
    }

    if (selectionType == SelectionType::ByMouseRelease)
    {
        event
            ->accept(); // Special for not selectable item first need to call standard mousePressEvent then accept event
    }
    else
    {
        if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
        {
            PointChoosed();
            event->accept();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::Enable()
{
    const bool enabled = m_indexActivePatternBlock == doc->PatternBlockMapper()->GetActiveId();
    setEnabled(enabled);
    m_namePoint->SetEnabledState(enabled);
    m_lineName->setEnabled(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::EnableToolMove(bool move)
{
    m_namePoint->setFlag(QGraphicsItem::ItemIsMovable, move);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::PointChoosed()
{
    emit ChoosedTool(m_id, SceneObject::Point);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::PointSelected(bool selected)
{
    m_selectedFromChild = true;
    setSelected(selected);
    m_selectedFromChild = false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FullUpdateFromFile update tool data form file.
 */
void VToolSinglePoint::FullUpdateFromFile()
{
    ReadAttributes();
    RefreshPointGeometry(*VAbstractTool::data.GeometricObject<VPointF>(m_id));
    SetVisualization();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief mouseReleaseEvent  handle mouse release events.
 * @param event mouse release event.
 */
void VToolSinglePoint::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (selectionType == SelectionType::ByMouseRelease && IsSelectedByReleaseEvent(this, event))
    {
        PointChoosed();
    }
    VScenePoint::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setToolTip(MakeToolTip());
    VScenePoint::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief itemChange hadle item change.
 * @param change change.
 * @param value value.
 * @return value.
 */
auto VToolSinglePoint::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) -> QVariant
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        if (not m_selectedFromChild)
        {
            const QSignalBlocker blocker(m_namePoint);
            m_namePoint->setSelected(value.toBool());
        }

        emit ChangedToolSelection(value.toBool(), m_id, m_id);
    }

    return VScenePoint::itemChange(change, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief keyReleaseEvent handle key release events.
 * @param event key release event.
 */
void VToolSinglePoint::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Delete:
            try
            {
                DeleteToolWithConfirm();
            }
            catch (const VExceptionToolWasDeleted &e)
            {
                Q_UNUSED(e)
                return; // Leave this method immediately!!!
            }
            break;
        default:
            break;
    }
    VScenePoint::keyReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    ShowContextMenu(event, m_id);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VDrawTool::SaveOptions(tag, obj);

    QSharedPointer<VPointF> const point = qSharedPointerDynamicCast<VPointF>(obj);
    SCASSERT(point.isNull() == false)

    doc->SetAttribute(tag, AttrName, point->name());
    doc->SetAttribute(tag, AttrMx, VAbstractValApplication::VApp()->fromPixel(point->mx()));
    doc->SetAttribute(tag, AttrMy, VAbstractValApplication::VApp()->fromPixel(point->my()));
    doc->SetAttribute<bool>(tag, AttrShowLabel, point->IsShowLabel());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::ChangeLabelVisibility(quint32 id, bool visible)
{
    if (id == m_id)
    {
        VAbstractApplication::VApp()->getUndoStack()->push(new ShowLabel(doc, id, visible));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::ChangeLabelPosition(quint32 id, const QPointF &pos)
{
    if (id == m_id)
    {
        QSharedPointer<VPointF> const point = VAbstractTool::data.GeometricObject<VPointF>(id);
        point->setMx(pos.x());
        point->setMy(pos.y());
        m_namePoint->SetRealPos(pos);
        RefreshLine();

        if (QGraphicsScene *sc = scene())
        {
            VMainGraphicsView::NewSceneRect(sc, VAbstractValApplication::VApp()->getSceneView(), this);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::AllowHover(bool enabled)
{
    setAcceptHoverEvents(enabled);
    AllowLabelHover(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::AllowSelecting(bool enabled)
{
    setFlag(QGraphicsItem::ItemIsSelectable, enabled);
    AllowLabelSelecting(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::AllowLabelHover(bool enabled)
{
    m_namePoint->setAcceptHoverEvents(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::AllowLabelSelecting(bool enabled)
{
    m_namePoint->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::ToolSelectionType(const SelectionType &type)
{
    VAbstractTool::ToolSelectionType(type);
    m_namePoint->LabelSelectionType(type);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSinglePoint::InitSegments(SegmentDetails &details) -> QPair<QString, QString>
{
    // This check helps to find missed objects in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 8, "Not all objects were handled.");

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wswitch-default")
    QT_WARNING_DISABLE_CLANG("-Wswitch-default")

    switch (details.curveType)
    {
        case GOType::EllipticalArc:
            return InitArc<VEllipticalArc>(details);
        case GOType::Arc:
            return InitArc<VArc>(details);
        case GOType::CubicBezier:
        case GOType::Spline:
            return InitSpline(details);
        case GOType::CubicBezierPath:
        case GOType::SplinePath:
            return InitSplinePath(details);
        case GOType::Point:
        case GOType::PlaceLabel:
        case GOType::Unknown:
            Q_UNREACHABLE();
            break;
    }

    QT_WARNING_POP

    Q_UNREACHABLE();
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSinglePoint::InitSplinePath(SegmentDetails &details) -> QPair<QString, QString>
{
    const auto splPath = details.data->GeometricObject<VAbstractCubicBezierPath>(details.curveId);
    VSplinePath *splPath1 = nullptr;
    VSplinePath *splPath2 = nullptr;
    if (not VFuzzyComparePossibleNulls(details.segLength, -1))
    {
        QScopedPointer<VPointF> const pC(
            VToolCutSplinePath::CutSplinePath(details.segLength, splPath, details.p.name(), &splPath1, &splPath2));
    }
    else
    {
        QScopedPointer<VPointF> const pC(
            VToolCutSplinePath::CutSplinePath(0, splPath, details.p.name(), &splPath1, &splPath2));
    }

    SCASSERT(splPath1 != nullptr)
    SCASSERT(splPath2 != nullptr)

    QSharedPointer<VAbstractCubicBezierPath> splP1;
    QSharedPointer<VAbstractCubicBezierPath> splP2;

    if (not VFuzzyComparePossibleNulls(details.segLength, -1))
    {
        splP1 = QSharedPointer<VAbstractCubicBezierPath>(new VSplinePath(*splPath1));
        splP2 = QSharedPointer<VAbstractCubicBezierPath>(new VSplinePath(*splPath2));
    }
    else
    {
        splP1 = QSharedPointer<VAbstractCubicBezierPath>(new VSplinePath());
        splP2 = QSharedPointer<VAbstractCubicBezierPath>(new VSplinePath());
    }

    splP1->SetDerivative(true);
    splP2->SetDerivative(true);

    // This fix can be removed since name1 and name2 no longer should be empty
    Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
    FixSubCurveNames(details, splPath, splP1, splP2);

    splP1->SetNameSuffix(details.name1);
    splP2->SetNameSuffix(details.name2);

    splP1->SetAliasSuffix(details.alias1);
    splP2->SetAliasSuffix(details.alias2);

    details.data->AddSpline(splP1, NULL_ID, details.p.id());
    details.data->AddSpline(splP2, NULL_ID, details.p.id());

    return qMakePair(splP1->ObjectName(), splP2->ObjectName());
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolSinglePoint::InitSpline(SegmentDetails &details) -> QPair<QString, QString>
{
    QSharedPointer<VAbstractCubicBezier> spline1;
    QSharedPointer<VAbstractCubicBezier> spline2;

    const auto spl = details.data->GeometricObject<VAbstractCubicBezier>(details.curveId);
    QPointF spl1p2;
    QPointF spl1p3;
    QPointF spl2p2;
    QPointF spl2p3;
    if (not VFuzzyComparePossibleNulls(details.segLength, -1))
    {
        spl->CutSpline(details.segLength, spl1p2, spl1p3, spl2p2, spl2p3, details.p.name());
    }
    else
    {
        spl->CutSpline(0, spl1p2, spl1p3, spl2p2, spl2p3, details.p.name());
    }

    if (not VFuzzyComparePossibleNulls(details.segLength, -1))
    {
        spline1 = QSharedPointer<VAbstractCubicBezier>(new VSpline(spl->GetP1(), spl1p2, spl1p3, details.p));
        spline2 = QSharedPointer<VAbstractCubicBezier>(new VSpline(details.p, spl2p2, spl2p3, spl->GetP4()));
    }
    else
    {
        spline1 = QSharedPointer<VAbstractCubicBezier>(new VSpline());
        spline2 = QSharedPointer<VAbstractCubicBezier>(new VSpline());
    }

    spline1->SetDerivative(true);
    spline2->SetDerivative(true);

    // This fix can be removed since name1 and name2 no longer should be empty
    Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
    FixSubCurveNames(details, spl, spline1, spline2);

    spline1->SetNameSuffix(details.name1);
    spline2->SetNameSuffix(details.name2);

    spline1->SetAliasSuffix(details.alias1);
    spline2->SetAliasSuffix(details.alias2);

    details.data->AddSpline(spline1, NULL_ID, details.p.id());
    details.data->AddSpline(spline2, NULL_ID, details.p.id());

    return qMakePair(spline1->ObjectName(), spline2->ObjectName());
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T>
void VToolSinglePoint::FixSubCurveNames(SegmentDetails &details,
                                        const QSharedPointer<T> &baseCurve,
                                        const QSharedPointer<T> &leftSub,
                                        const QSharedPointer<T> &rightSub)
{
    bool fixName1 = false;
    bool fixName2 = false;
    if (details.name1.isEmpty())
    {
        if (!baseCurve->IsDerivative())
        {
            details.name1 = leftSub->HeadlessName();
        }
        else
        {
            details.name1 = GenerateUniqueCurveName(details.data,
                                                    leftSub->GetTypeHead(),
                                                    "LSubCurve"_L1,
                                                    details.p.name());
            fixName1 = true;
        }
    }

    if (details.name2.isEmpty())
    {
        if (!baseCurve->IsDerivative())
        {
            details.name2 = rightSub->HeadlessName();
        }
        else
        {
            details.name2 = GenerateUniqueCurveName(details.data,
                                                    rightSub->GetTypeHead(),
                                                    "RSubCurve"_L1,
                                                    details.p.name());
            fixName2 = true;
        }
    }

    if (fixName1 || fixName2)
    {
        VAbstractApplication::VApp()->getUndoStack()->push(
            new RenameSegmentCurves(RenameAlias::CurveType(baseCurve->getType()),
                                    details.p.name(),
                                    fixName1 ? details.name1 : QString(),
                                    fixName2 ? details.name2 : QString(),
                                    details.doc));

        QDomElement domElement = details.doc->FindElementById(details.id);
        if (!domElement.isElement())
        {
            qCDebug(vTool, "Can't find tool with id = %u", details.id);
            return;
        }

        if (fixName1)
        {
            details.doc->SetAttribute(domElement, details.name1AttrName, details.name1);
        }

        if (fixName2)
        {
            details.doc->SetAttribute(domElement, details.name2AttrName, details.name2);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
auto VToolSinglePoint::InitArc(SegmentDetails &details) -> QPair<QString, QString>
{
    const QSharedPointer<Item> arc = details.data->GeometricObject<Item>(details.curveId);
    Item arc1;
    Item arc2;

    if (not VFuzzyComparePossibleNulls(details.segLength, -1))
    {
        arc->CutArc(details.segLength, &arc1, &arc2, details.p.name());
    }
    else
    {
        arc->CutArc(0, &arc1, &arc2, details.p.name());
    }

    // Arc highly depend on id. Need for creating the name.
    arc1.setId(details.p.id() + 1);
    arc2.setId(details.p.id() + 2);

    QSharedPointer<Item> a1;
    QSharedPointer<Item> a2;

    if (not VFuzzyComparePossibleNulls(details.segLength, -1))
    {
        a1 = QSharedPointer<Item>::create(arc1);
        a2 = QSharedPointer<Item>::create(arc2);
    }
    else
    {
        a1 = QSharedPointer<Item>::create();
        a2 = QSharedPointer<Item>::create();
    }

    a1->SetDerivative(true);
    a2->SetDerivative(true);

    // This fix can be removed since name1 and name2 no longer should be empty
    Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
    FixSubCurveNames(details, arc, a1, a2);

    a1->SetNameSuffix(details.name1);
    a2->SetNameSuffix(details.name2);

    a1->SetAliasSuffix(details.alias1);
    a2->SetAliasSuffix(details.alias2);

    details.data->AddArc(a1, /*arc1.id()*/ NULL_ID, details.p.id());
    details.data->AddArc(a2, /*arc2.id()*/ NULL_ID, details.p.id());

    // Because we don't store segments, but only data about them we must register the names manually
    details.data->RegisterUniqueName(a1);
    details.data->RegisterUniqueName(a2);

    return qMakePair(arc1.ObjectName(), arc2.ObjectName());
}
