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
#include <new>
#include <QtMath>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/vtooltriangle.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "visline.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolTriangle::VisToolTriangle(const VContainer *data, QGraphicsItem *parent)
    :VisLine(data, parent)
{
    m_axisP1 = InitPoint(Color(VColor::SupportColor), this);
    m_axisP2 = InitPoint(Color(VColor::SupportColor), this);
    m_axis = InitItem<VCurvePathItem>(Color(VColor::SupportColor), this);
    m_hypotenuseP1 = InitPoint(Color(VColor::SupportColor), this);
    m_hypotenuseP2 = InitPoint(Color(VColor::SupportColor), this);
    m_foot1 = InitItem<VScaledLine>(Color(VColor::SupportColor), this);
    m_foot2 = InitItem<VScaledLine>(Color(VColor::SupportColor), this); //-V656

    m_point = InitPoint(Color(VColor::MainColor), this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolTriangle::RefreshGeometry()
{
    if (m_object1Id > NULL_ID)
    {
        const QSharedPointer<VPointF> first = GetData()->GeometricObject<VPointF>(m_object1Id);
        DrawPoint(m_axisP1, static_cast<QPointF>(*first), Color(VColor::SupportColor));

        if (m_object2Id <= NULL_ID)
        {
            DrawAimedAxis(m_axis, QLineF(static_cast<QPointF>(*first), ScenePos()), Color(VColor::SupportColor));
        }
        else
        {
            const QSharedPointer<VPointF> second = GetData()->GeometricObject<VPointF>(m_object2Id);
            DrawPoint(m_axisP2, static_cast<QPointF>(*second), Color(VColor::SupportColor));

            DrawAimedAxis(m_axis, QLineF(static_cast<QPointF>(*first), static_cast<QPointF>(*second)),
                          Color(VColor::SupportColor));

            if (m_hypotenuseP1Id > NULL_ID)
            {
                const QSharedPointer<VPointF> third = GetData()->GeometricObject<VPointF>(m_hypotenuseP1Id);
                DrawPoint(m_hypotenuseP1, static_cast<QPointF>(*third), Color(VColor::SupportColor));

                if (m_hypotenuseP2Id <= NULL_ID)
                {
                    DrawLine(this, QLineF(static_cast<QPointF>(*third), ScenePos()), Color(VColor::SupportColor),
                             Qt::DashLine);

                    QPointF trPoint;
                    VToolTriangle::FindPoint(static_cast<QPointF>(*first), static_cast<QPointF>(*second),
                                             static_cast<QPointF>(*third), ScenePos(), &trPoint);
                    DrawPoint(m_point, trPoint, Color(VColor::MainColor));

                    DrawLine(m_foot1, QLineF(static_cast<QPointF>(*third), trPoint), Color(VColor::SupportColor),
                             Qt::DashLine);
                    DrawLine(m_foot2, QLineF(ScenePos(), trPoint), Color(VColor::SupportColor), Qt::DashLine);
                }
                else
                {
                    const QSharedPointer<VPointF> forth = GetData()->GeometricObject<VPointF>(m_hypotenuseP2Id);
                    DrawPoint(m_hypotenuseP2, static_cast<QPointF>(*forth), Color(VColor::SupportColor));

                    DrawLine(this, QLineF(static_cast<QPointF>(*third), static_cast<QPointF>(*forth)),
                             Color(VColor::SupportColor), Qt::DashLine);

                    QPointF trPoint;
                    VToolTriangle::FindPoint(static_cast<QPointF>(*first), static_cast<QPointF>(*second),
                                             static_cast<QPointF>(*third), static_cast<QPointF>(*forth), &trPoint);
                    DrawPoint(m_point, trPoint, Color(VColor::MainColor));

                    DrawLine(m_foot1, QLineF(static_cast<QPointF>(*third), trPoint), Color(VColor::SupportColor),
                             Qt::DashLine);
                    DrawLine(m_foot2, QLineF(static_cast<QPointF>(*forth), trPoint), Color(VColor::SupportColor),
                             Qt::DashLine);
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
void VisToolTriangle::DrawAimedAxis(VCurvePathItem *item, const QLineF &line, const QColor &color,
                                    Qt::PenStyle style)
{
    SCASSERT (item != nullptr)

    QPen visPen = item->pen();
    visPen.setColor(color);
    visPen.setStyle(style);

    item->setPen(visPen);

    QPainterPath path;
    path.moveTo(line.p1());
    path.lineTo(line.p2());

    qreal arrow_step = 60;
    qreal arrow_size = 10;

    if (line.length() < arrow_step)
    {
        DrawArrow(line, path, arrow_size);
    }

    QLineF axis;
    axis.setP1(line.p1());
    axis.setAngle(line.angle());
    axis.setLength(arrow_step);

    int steps = qFloor(line.length()/arrow_step);
    for (int i=0; i<steps; ++i)
    {
        DrawArrow(axis, path, arrow_size);
        axis.setLength(axis.length()+arrow_step);
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
    arrowPart1.setAngle(axis.angle()+180+35);

    path.moveTo(arrowPart1.p1());
    path.lineTo(arrowPart1.p2());

    QLineF arrowPart2;
    arrowPart2.setP1(axis.p2());
    arrowPart2.setLength(arrow_size);
    arrowPart2.setAngle(axis.angle()+180-35);

    path.moveTo(arrowPart2.p1());
    path.lineTo(arrowPart2.p2());
}
