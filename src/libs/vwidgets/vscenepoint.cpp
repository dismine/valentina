/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 6, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#include "vscenepoint.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/def.h"
#include "../vmisc/theme/vscenestylesheet.h"
#include "../vmisc/vabstractapplication.h"
#include "global.h"
#include "scalesceneitems.h"
#include "vgraphicssimpletextitem.h"

#include <QBrush>
#include <QFont>
#include <QPainter>
#include <QPen>
#include <QStyle>
#include <QStyleOptionGraphicsItem>

#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
#include "../vmisc/backport/qpainterstateguard.h"
#else
#include <QPainterStateGuard>
#endif

//---------------------------------------------------------------------------------------------------------------------
VScenePoint::VScenePoint(VColorRole role, QGraphicsItem *parent)
  : QGraphicsEllipseItem(parent),
    m_namePoint(new VGraphicsSimpleTextItem(VColorRole::DraftLabelColor, VColorRole::DraftLabelHoverColor, this)),
    m_lineName(new VScaledLine(VColorRole::DraftLabelLineColor, this)),
    m_role(role)
{
    m_lineName->SetBoldLine(false);
    m_lineName->setLine(QLineF(0, 0, 1, 0));
    m_lineName->setVisible(false);

    setBrush(QBrush(Qt::NoBrush));
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus
}

//---------------------------------------------------------------------------------------------------------------------
void VScenePoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    const qreal scale = SceneScale(scene());

    ScaleMainPenWidth(scale);
    ScaleCircleSize(this, scale);

    const VCommonSettings *settings = VAbstractApplication::VApp()->Settings();

    if (settings->GetShowAccuracyRadius())
    {
        QPainterStateGuard const guard(painter);

        QPen pen = painter->pen();
        pen.setWidthF(accuracyPointOnLine / 15);
        pen.setStyle(Qt::DashLine);
        pen.setColor(VSceneStylesheet::ToolStyle().AccuracyRadiusColor());

        painter->setPen(pen);
        painter->drawEllipse(PointRect(accuracyPointOnLine));
    }

    if (settings->GetPatternLabelFontSize() * scale < minVisibleFontSize || settings->GetHideLabels())
    {
        m_namePoint->setVisible(false);
        m_lineName->setVisible(false);
    }
    else
    {
        if (not m_onlyPoint)
        {
            m_namePoint->setVisible(m_showLabel);
            RefreshLine();
        }
    }

    PaintWithFixItemHighlightSelected<QGraphicsEllipseItem>(this, painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
void VScenePoint::RefreshPointGeometry(const VPointF &point)
{
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
    setPos(static_cast<QPointF>(point));
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    m_showLabel = point.IsShowLabel();

    {
        const QSignalBlocker blocker(m_namePoint);
        m_namePoint->setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
        m_namePoint->SetRealPos(QPointF(point.mx(), point.my()));
        m_namePoint->setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    }

    m_namePoint->setText(point.name());
    m_namePoint->setVisible(m_showLabel);

    RefreshLine();
}

//---------------------------------------------------------------------------------------------------------------------
auto VScenePoint::boundingRect() const -> QRectF
{
    QRectF rect = QGraphicsEllipseItem::boundingRect();

    if (VAbstractApplication::VApp()->Settings()->GetShowAccuracyRadius())
    {
        rect = rect.united(PointRect(accuracyPointOnLine));
    }

    return rect;
}

//---------------------------------------------------------------------------------------------------------------------
void VScenePoint::SetOnlyPoint(bool value)
{
    m_onlyPoint = value;
    m_namePoint->setVisible(not m_onlyPoint);
    m_lineName->setVisible(not m_onlyPoint);
}

//---------------------------------------------------------------------------------------------------------------------
auto VScenePoint::IsOnlyPoint() const -> bool
{
    return m_onlyPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VScenePoint::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = true;
    QGraphicsEllipseItem::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VScenePoint::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = false;
    QGraphicsEllipseItem::hoverLeaveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VScenePoint::RefreshLine()
{
    QRectF nRec = m_namePoint->sceneBoundingRect();
    nRec.translate(-scenePos());
    if (not rect().intersects(nRec))
    {
        const QRectF nameRec = m_namePoint->sceneBoundingRect();
        QPointF p1;
        QPointF p2;
        VGObject::LineIntersectCircle(QPointF(), ScaledRadius(SceneScale(scene())),
                                      QLineF(QPointF(), nameRec.center() - scenePos()), p1, p2);
        const QPointF pRec = VGObject::LineIntersectRect(nameRec, QLineF(scenePos(), nameRec.center()));

        if (QLineF(p1, pRec - scenePos()).length() <= ToPixel(4 / qMax(1.0, SceneScale(scene())), Unit::Mm))
        {
            m_lineName->setVisible(false);
        }
        else
        {
            m_lineName->setLine(QLineF(p1, pRec - scenePos()));
            m_lineName->setVisible(m_showLabel);
        }
    }
    else
    {
        m_lineName->setVisible(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VScenePoint::GetLabelLineColorRole() const -> VColorRole
{
    return m_lineName->GetColorRole();
}

//---------------------------------------------------------------------------------------------------------------------
void VScenePoint::SetLabelLineColorRole(VColorRole role)
{
    m_lineName->SetColorRole(role);
}

//---------------------------------------------------------------------------------------------------------------------
auto VScenePoint::GetLabelTextColorRole() const -> VColorRole
{
    return m_namePoint->GetTextColor();
}

//---------------------------------------------------------------------------------------------------------------------
void VScenePoint::SetLabelTextColorRole(VColorRole role)
{
    m_namePoint->SetTextColor(role);
}

//---------------------------------------------------------------------------------------------------------------------
auto VScenePoint::GetLabelTextHoverColorRole() const -> VColorRole
{
    return m_namePoint->GetTextHoverColor();
}

//---------------------------------------------------------------------------------------------------------------------
void VScenePoint::SetLabelTextHoverColorRole(VColorRole role)
{
    m_namePoint->SetTextHoverColor(role);
}

//---------------------------------------------------------------------------------------------------------------------
void VScenePoint::ScaleMainPenWidth(qreal scale)
{
    const qreal width = ScaleWidth(m_isHovered ? VAbstractApplication::VApp()->Settings()->WidthMainLine()
                                               : VAbstractApplication::VApp()->Settings()->WidthHairLine(),
                                   scale);

    QPen pointPen = pen();
    if (m_role != VColorRole::CustomColor)
    {
        pointPen.setColor(VSceneStylesheet::CorrectToolColor(this, VSceneStylesheet::Color(m_role)));
    }
    pointPen.setWidthF(width);
    setPen(pointPen);
}
