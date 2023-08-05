/************************************************************************
 **
 **  @file   vistoolpointofintersection.cpp
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

#include "vistoolpointofintersection.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QLine>
#include <QPointF>
#include <QSharedPointer>
#include <new>

#include "../vgeometry/vpointf.h"
#include "../visualization.h"
#include "../vmisc/compatibility.h"
#include "../vpatterndb/vcontainer.h"
#include "visline.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolPointOfIntersection::VisToolPointOfIntersection(const VContainer *data, QGraphicsItem *parent)
  : VisLine(data, parent)
{
    SetColorRole(VColorRole::VisSupportColor);

    m_axisP1 = InitPoint(VColorRole::VisSupportColor, this);
    m_axisP2 = InitPoint(VColorRole::VisSupportColor, this); //-V656
    m_axis2 = InitItem<VScaledLine>(VColorRole::VisSupportColor, this);

    m_point = InitPoint(VColorRole::VisMainColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfIntersection::RefreshGeometry()
{
    QLineF axisL1;
    if (m_point1Id <= NULL_ID)
    {
        axisL1 = Axis(ScenePos(), 90);
        DrawLine(this, axisL1, Qt::DashLine);
    }
    else
    {
        const QSharedPointer<VPointF> first = GetData()->GeometricObject<VPointF>(m_point1Id);
        DrawPoint(m_axisP1, static_cast<QPointF>(*first));

        axisL1 = Axis(static_cast<QPointF>(*first), 90);
        DrawLine(this, axisL1, Qt::DashLine);

        QLineF axisL2;
        if (m_point2Id <= NULL_ID)
        {
            axisL2 = Axis(ScenePos(), 180);
            ShowIntersection(axisL1, axisL2);
        }
        else
        {
            const QSharedPointer<VPointF> second = GetData()->GeometricObject<VPointF>(m_point2Id);
            DrawPoint(m_axisP2, static_cast<QPointF>(*second));
            axisL2 = Axis(static_cast<QPointF>(*second), 180);
            ShowIntersection(axisL1, axisL2);
        }
        DrawLine(m_axis2, axisL2, Qt::DashLine);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfIntersection::VisualMode(quint32 id)
{
    m_point1Id = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfIntersection::ShowIntersection(const QLineF &axis1, const QLineF &axis2)
{
    QPointF p;
    QLineF::IntersectType intersect = Intersects(axis1, axis2, &p);

    if (intersect == QLineF::UnboundedIntersection || intersect == QLineF::BoundedIntersection)
    {
        m_point->setVisible(true);
        DrawPoint(m_point, p);
    }
    else
    {
        m_point->setVisible(false);
    }
}
