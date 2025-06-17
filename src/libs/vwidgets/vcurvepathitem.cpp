/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 5, 2017
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

#include "vcurvepathitem.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vmisc/theme/vscenestylesheet.h"
#include "../vmisc/vabstractapplication.h"
#include "global.h"

#include <QPainter>

#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
#include "../vmisc/backport/qpainterstateguard.h"
#else
#include <QPainterStateGuard>
#endif

//---------------------------------------------------------------------------------------------------------------------
VCurvePathItem::VCurvePathItem(VColorRole role, QGraphicsItem *parent)
  : QGraphicsPathItem(parent),
    m_defaultWidth(VAbstractApplication::VApp()->Settings()->WidthMainLine()),
    m_role(role)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VCurvePathItem::shape() const -> QPainterPath
{
    QPainterPath itemPath;

    if (not m_points.isEmpty())
    {
        for (qint32 i = 0; i < m_points.count() - 1; ++i)
        {
            itemPath.moveTo(m_points.at(i));
            itemPath.lineTo(m_points.at(i + 1));
        }
    }
    else
    {
        itemPath = path();
    }

    const QPainterPath arrowsPath = VAbstractCurve::ShowDirection(
        m_directionArrows, ScaleWidth(VAbstractCurve::LengthCurveDirectionArrow(), SceneScale(scene())));

    if (arrowsPath != QPainterPath())
    {
        itemPath.addPath(arrowsPath);
    }
    itemPath.setFillRule(Qt::WindingFill);
    return ItemShapeFromPath(itemPath, pen());
}

//---------------------------------------------------------------------------------------------------------------------
void VCurvePathItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    ScalePenWidth();

    const QPainterPath arrowsPath = VAbstractCurve::ShowDirection(
        m_directionArrows, ScaleWidth(VAbstractCurve::LengthCurveDirectionArrow(), SceneScale(scene())));

    if (arrowsPath != QPainterPath())
    {
        QPainterStateGuard const guard(painter);

        QPen arrowPen(pen());

        if (m_role != VColorRole::CustomColor)
        {
            arrowPen.setColor(VSceneStylesheet::Color(m_role));
        }

        arrowPen.setStyle(Qt::SolidLine);

        painter->setPen(arrowPen);
        painter->setBrush(brush());
        painter->drawPath(arrowsPath);
    }

    PaintWithFixItemHighlightSelected<QGraphicsPathItem>(this, painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
void VCurvePathItem::SetDirectionArrows(const QVector<QPair<QLineF, QLineF>> &arrows)
{
    m_directionArrows = arrows;
}

//---------------------------------------------------------------------------------------------------------------------
void VCurvePathItem::SetPoints(const QVector<QPointF> &points)
{
    m_points = points;
}

//---------------------------------------------------------------------------------------------------------------------
void VCurvePathItem::SetWidth(qreal width)
{
    m_defaultWidth = width;
}

//---------------------------------------------------------------------------------------------------------------------
void VCurvePathItem::ScalePenWidth()
{
    const qreal width = ScaleWidth(m_defaultWidth, SceneScale(scene()));

    QPen toolPen = pen();
    toolPen.setWidthF(width);

    if (m_role != VColorRole::CustomColor)
    {
        toolPen.setColor(VSceneStylesheet::Color(m_role));
    }

    setPen(toolPen);
}
