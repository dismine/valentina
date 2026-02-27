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
#include <new>

#include "../../../../undocommands/label/movelabel.h"
#include "../../../../undocommands/label/showlabel.h"
#include "../../../vabstracttool.h"
#include "../../vdrawtool.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vabstractpoint.h"
#include "../vgeometry/vabstractcubicbezier.h"
#include "../vgeometry/vabstractcubicbezierpath.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepath.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/global.h"
#include "../vwidgets/scalesceneitems.h"
#include "../vwidgets/vgraphicssimpletextitem.h"
#include "toolcut/vtoolcutsplinepath.h"

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
    SetPointName(m_id, name);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolSinglePoint::SetEnabled(bool enabled)
{
    setEnabled(enabled);
    m_lineName->setEnabled(enabled);
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

    if (m_selectionType == SelectionType::ByMouseRelease)
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
void VToolSinglePoint::Disable(bool disable, const QString &namePP)
{
    const bool enabled = !CorrectDisable(disable, namePP);
    SetEnabled(enabled);
    m_namePoint->SetEnabledState(enabled);
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
    if (m_selectionType == SelectionType::ByMouseRelease && IsSelectedByReleaseEvent(this, event))
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
            m_namePoint->blockSignals(true);
            m_namePoint->setSelected(value.toBool());
            m_namePoint->blockSignals(false);
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
void VToolSinglePoint::ToolSelectionType(const SelectionType &selectionType)
{
    VAbstractTool::ToolSelectionType(selectionType);
    m_namePoint->LabelSelectionType(selectionType);
}

//---------------------------------------------------------------------------------------------------------------------

auto VToolSinglePoint::InitSegments(GOType curveType, qreal segLength, const VPointF *p, quint32 curveId,
                                    VContainer *data, const QString &alias1, const QString &alias2)
    -> QPair<QString, QString>
{
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wswitch-default")
    QT_WARNING_DISABLE_CLANG("-Wswitch-default")

    switch (curveType)
    {
        case GOType::EllipticalArc:
            return InitArc<VEllipticalArc>(data, segLength, p, curveId, alias1, alias2);
        case GOType::Arc:
            return InitArc<VArc>(data, segLength, p, curveId, alias1, alias2);
        case GOType::CubicBezier:
        case GOType::Spline:
        {
            QSharedPointer<VAbstractBezier> spline1;
            QSharedPointer<VAbstractBezier> spline2;

            const auto spl = data->GeometricObject<VAbstractCubicBezier>(curveId);
            QPointF spl1p2, spl1p3, spl2p2, spl2p3;
            if (not VFuzzyComparePossibleNulls(segLength, -1))
            {
                spl->CutSpline(segLength, spl1p2, spl1p3, spl2p2, spl2p3, p->name());
            }
            else
            {
                spl->CutSpline(0, spl1p2, spl1p3, spl2p2, spl2p3, p->name());
            }

            auto *spl1 = new VSpline(spl->GetP1(), spl1p2, spl1p3, *p);
            auto *spl2 = new VSpline(*p, spl2p2, spl2p3, spl->GetP4());

            if (not VFuzzyComparePossibleNulls(segLength, -1))
            {
                spline1 = QSharedPointer<VAbstractBezier>(spl1);
                spline2 = QSharedPointer<VAbstractBezier>(spl2);
            }
            else
            {
                spline1 = QSharedPointer<VAbstractBezier>(new VSpline());
                spline2 = QSharedPointer<VAbstractBezier>(new VSpline());

                // Take names for empty splines from donors.
                spline1->setName(spl1->name());
                spline2->setName(spl2->name());

                delete spl1;
                delete spl2;
            }

            spline1->SetAliasSuffix(alias1);
            spline2->SetAliasSuffix(alias2);

            data->RegisterUniqueName(spline1);
            data->AddSpline(spline1, NULL_ID, p->id());

            data->RegisterUniqueName(spline2);
            data->AddSpline(spline2, NULL_ID, p->id());

            // Because we don't store segments, but only data about them we must register the names manually
            data->RegisterUniqueName(spline1);
            data->RegisterUniqueName(spline2);

            return qMakePair(spline1->ObjectName(), spline2->ObjectName());
        }
        case GOType::CubicBezierPath:
        case GOType::SplinePath:
        {
            QSharedPointer<VAbstractBezier> splP1;
            QSharedPointer<VAbstractBezier> splP2;

            const auto splPath = data->GeometricObject<VAbstractCubicBezierPath>(curveId);
            VSplinePath *splPath1 = nullptr;
            VSplinePath *splPath2 = nullptr;
            if (not VFuzzyComparePossibleNulls(segLength, -1))
            {
                VPointF *pC = VToolCutSplinePath::CutSplinePath(segLength, splPath, p->name(), &splPath1, &splPath2);
                delete pC;
            }
            else
            {
                VPointF *pC = VToolCutSplinePath::CutSplinePath(0, splPath, p->name(), &splPath1, &splPath2);
                delete pC;
            }

            SCASSERT(splPath1 != nullptr)
            SCASSERT(splPath2 != nullptr)

            if (not VFuzzyComparePossibleNulls(segLength, -1))
            {
                splP1 = QSharedPointer<VAbstractBezier>(splPath1);
                splP2 = QSharedPointer<VAbstractBezier>(splPath2);
            }
            else
            {
                splP1 = QSharedPointer<VAbstractBezier>(new VSplinePath());
                splP2 = QSharedPointer<VAbstractBezier>(new VSplinePath());

                // Take names for empty spline paths from donors.
                splP1->setName(splPath1->name());
                splP2->setName(splPath2->name());

                delete splPath1;
                delete splPath2;
            }

            splP1->SetAliasSuffix(alias1);
            splP2->SetAliasSuffix(alias2);

            data->RegisterUniqueName(splP1);
            data->AddSpline(splP1, NULL_ID, p->id());

            data->RegisterUniqueName(splP2);
            data->AddSpline(splP2, NULL_ID, p->id());

            // Because we don't store segments, but only data about them we must register the names manually
            data->RegisterUniqueName(splP1);
            data->RegisterUniqueName(splP2);

            return qMakePair(splP1->ObjectName(), splP2->ObjectName());
        }
        case GOType::Point:
        case GOType::PlaceLabel:
        case GOType::Unknown:
            Q_UNREACHABLE();
            break;
    }

    QT_WARNING_POP

    return {};
}
