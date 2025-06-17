/************************************************************************
 **
 **  @file   vistooltriangle.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 8, 2014
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

#include "vistooltriangle.h"

#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPen>
#include <QPointF>
#include <QSharedPointer>
#include <QtMath>
#include <new>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/vtooltriangle.h"
#include "../vgeometry/vpointf.h"
#include "../visualization.h"
#include "../vpatterndb/vcontainer.h"
#include "visline.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolTriangle::VisToolTriangle(const VContainer *data, QGraphicsItem *parent)
  : VisLine(data, parent)
{
    SetColorRole(VColorRole::VisSupportColor);

    m_axisP1 = InitPoint(VColorRole::VisSupportColor, this);
    m_axisP2 = InitPoint(VColorRole::VisSupportColor, this);
    m_axis = InitItem<VCurvePathItem>(VColorRole::VisSupportColor, this);
    m_hypotenuseP1 = InitPoint(VColorRole::VisSupportColor, this);
    m_hypotenuseP2 = InitPoint(VColorRole::VisSupportColor, this);
    m_foot1 = InitItem<VScaledLine>(VColorRole::VisSupportColor, this);
    m_foot2 = InitItem<VScaledLine>(VColorRole::VisSupportColor, this); //-V656

    m_point = InitPoint(VColorRole::VisMainColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolTriangle::RefreshGeometry()
{
    if (m_object1Id > NULL_ID)
    {
        const QSharedPointer<VPointF> first = GetData()->GeometricObject<VPointF>(m_object1Id);
        DrawPoint(m_axisP1, static_cast<QPointF>(*first));

        if (m_object2Id <= NULL_ID)
        {
            DrawAimedAxis(m_axis, QLineF(static_cast<QPointF>(*first), ScenePos()));
        }
        else
        {
            const QSharedPointer<VPointF> second = GetData()->GeometricObject<VPointF>(m_object2Id);
            DrawPoint(m_axisP2, static_cast<QPointF>(*second));

            DrawAimedAxis(m_axis, QLineF(static_cast<QPointF>(*first), static_cast<QPointF>(*second)));

            if (m_hypotenuseP1Id > NULL_ID)
            {
                const QSharedPointer<VPointF> third = GetData()->GeometricObject<VPointF>(m_hypotenuseP1Id);
                DrawPoint(m_hypotenuseP1, static_cast<QPointF>(*third));

                if (m_hypotenuseP2Id <= NULL_ID)
                {
                    DrawLine(this, QLineF(static_cast<QPointF>(*third), ScenePos()), Qt::DashLine);

                    QPointF trPoint;
                    VToolTriangle::FindPoint(static_cast<QPointF>(*first), static_cast<QPointF>(*second),
                                             static_cast<QPointF>(*third), ScenePos(), &trPoint);
                    DrawPoint(m_point, trPoint);

                    DrawLine(m_foot1, QLineF(static_cast<QPointF>(*third), trPoint), Qt::DashLine);
                    DrawLine(m_foot2, QLineF(ScenePos(), trPoint), Qt::DashLine);
                }
                else
                {
                    const QSharedPointer<VPointF> forth = GetData()->GeometricObject<VPointF>(m_hypotenuseP2Id);
                    DrawPoint(m_hypotenuseP2, static_cast<QPointF>(*forth));

                    DrawLine(this, QLineF(static_cast<QPointF>(*third), static_cast<QPointF>(*forth)), Qt::DashLine);

                    QPointF trPoint;
                    VToolTriangle::FindPoint(static_cast<QPointF>(*first), static_cast<QPointF>(*second),
                                             static_cast<QPointF>(*third), static_cast<QPointF>(*forth), &trPoint);
                    DrawPoint(m_point, trPoint);

                    DrawLine(m_foot1, QLineF(static_cast<QPointF>(*third), trPoint), Qt::DashLine);
                    DrawLine(m_foot2, QLineF(static_cast<QPointF>(*forth), trPoint), Qt::DashLine);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolTriangle::VisualMode(quint32 id)
{
    m_object1Id = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolTriangle::DrawAimedAxis(VCurvePathItem *item, const QLineF &line, Qt::PenStyle style)
{
    SCASSERT(item != nullptr)

    QPen visPen = item->pen();
    visPen.setStyle(style);

    item->setPen(visPen);

    QPainterPath path;
    path.moveTo(line.p1());
    path.lineTo(line.p2());

    qreal const arrow_step = 60;
    qreal const arrow_size = 10;

    if (line.length() < arrow_step)
    {
        DrawArrow(line, path, arrow_size);
    }

    QLineF axis;
    axis.setP1(line.p1());
    axis.setAngle(line.angle());
    axis.setLength(arrow_step);

    int const steps = qFloor(line.length() / arrow_step);
    for (int i = 0; i < steps; ++i)
    {
        DrawArrow(axis, path, arrow_size);
        axis.setLength(axis.length() + arrow_step);
    }
    item->setPath(path);
    item->setVisible(true);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolTriangle::DrawArrow(const QLineF &axis, QPainterPath &path, const qreal &arrow_size)
{
    QLineF arrowPart1;
    arrowPart1.setP1(axis.p2());
    arrowPart1.setLength(arrow_size);
    arrowPart1.setAngle(axis.angle() + 180 + 35);

    path.moveTo(arrowPart1.p1());
    path.lineTo(arrowPart1.p2());

    QLineF arrowPart2;
    arrowPart2.setP1(axis.p2());
    arrowPart2.setLength(arrow_size);
    arrowPart2.setAngle(axis.angle() + 180 - 35);

    path.moveTo(arrowPart2.p1());
    path.lineTo(arrowPart2.p2());
}
