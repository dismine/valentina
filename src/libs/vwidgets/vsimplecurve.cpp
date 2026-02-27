/************************************************************************
 **
 **  @file   vsimplecurve.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 5, 2016
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

#include "vsimplecurve.h"

#include <QBrush>
#include <QFlags>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QMessageLogger>
#include <QPainterPath>
#include <QPen>
#include <QtDebug>

#include "../ifc/ifcdef.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vmisc/theme/vscenestylesheet.h"
#include "../vmisc/vabstractapplication.h"
#include "global.h"

template <class T> class QSharedPointer;

//---------------------------------------------------------------------------------------------------------------------
VSimpleCurve::VSimpleCurve(quint32 id, const QSharedPointer<VAbstractCurve> &curve, QObject *parent)
  : VAbstractSimple(id, parent),
    VCurvePathItem(VColorRole::CustomColor),
    m_curve(curve),
    m_isHovered(false)
{
    setBrush(QBrush(Qt::NoBrush));
    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus
}

//---------------------------------------------------------------------------------------------------------------------
void VSimpleCurve::RefreshGeometry(const QSharedPointer<VAbstractCurve> &curve)
{
    m_curve = curve;

    if (not m_curve.isNull())
    {
        m_isHovered ? SetDirectionArrows(m_curve->DirectionArrows()) : SetDirectionArrows(QVector<DirectionArrow>());
        setPath(m_curve->GetPath());
        SetPoints(m_curve->GetPoints());
    }
    else
    {
        qWarning() << "VSimpleCurve::RefreshGeometry: pointer to curve is null.";
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VSimpleCurve::CurveChoosed()
{
    emit Choosed(id);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimpleCurve::CurveSelected(bool selected)
{
    emit Selected(selected, id);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimpleCurve::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Special for not selectable item first need to call standard mousePressEvent then accept event
    QGraphicsPathItem::mousePressEvent(event);

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
            emit Choosed(id);
            event->accept();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VSimpleCurve::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_selectionType == SelectionType::ByMouseRelease && IsSelectedByReleaseEvent(this, event))
    {
        emit Choosed(id);
    }
    QGraphicsPathItem::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimpleCurve::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = true;
    if (not m_curve.isNull())
    {
        SetDirectionArrows(m_curve->DirectionArrows());
    }
    QGraphicsPathItem::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimpleCurve::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_isHovered = false;
    SetDirectionArrows(QVector<DirectionArrow>());
    QGraphicsPathItem::hoverLeaveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
auto VSimpleCurve::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) -> QVariant
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        emit Selected(value.toBool(), id);
    }

    return QGraphicsPathItem::itemChange(change, value);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimpleCurve::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    emit ShowContextMenu(event, id);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimpleCurve::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Delete:
            emit Delete();
            return; // Leave this method immediately after call!!!
        default:
            break;
    }
    QGraphicsPathItem::keyReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VSimpleCurve::ScalePenWidth()
{
    qreal width = m_isHovered ? VAbstractApplication::VApp()->Settings()->WidthMainLine()
                              : VAbstractApplication::VApp()->Settings()->WidthHairLine();
    width = ScaleWidth(width, SceneScale(scene()));

    QPen curvePen = pen();
    curvePen.setColor(
        VSceneStylesheet::CorrectToolColor(this, VSceneStylesheet::CorrectToolColorForDarkTheme(m_curve->GetColor())));
    curvePen.setWidthF(width);
    curvePen.setStyle(LineStyleToPenStyle(m_curve->GetPenStyle()));
    setPen(curvePen);
}
