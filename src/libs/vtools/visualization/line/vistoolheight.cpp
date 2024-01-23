/************************************************************************
 **
 **  @file   vistoolheight.cpp
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

#include "vistoolheight.h"

#include <QGraphicsLineItem>
#include <QLine>
#include <QPointF>
#include <QSharedPointer>
#include <new>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolheight.h"
#include "../vgeometry/vpointf.h"
#include "../visualization.h"
#include "../vmisc/compatibility.h"
#include "../vpatterndb/vcontainer.h"
#include "visline.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolHeight::VisToolHeight(const VContainer *data, QGraphicsItem *parent)
  : VisLine(data, parent)
{
    m_basePoint = InitPoint(VColorRole::VisSupportColor, this);
    m_lineP1 = InitPoint(VColorRole::VisSupportColor, this);
    m_lineP2 = InitPoint(VColorRole::VisSupportColor, this);
    m_line = InitItem<VScaledLine>(VColorRole::VisSupportColor, this);
    m_lineIntersection = InitItem<VScaledLine>(VColorRole::VisSupportColor, this); //-V656

    m_point = InitPoint(VColorRole::VisMainColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolHeight::RefreshGeometry()
{
    if (m_basePointId > NULL_ID)
    {
        const QSharedPointer<VPointF> first = GetData()->GeometricObject<VPointF>(m_basePointId);
        DrawPoint(m_basePoint, static_cast<QPointF>(*first));

        if (m_lineP1Id <= NULL_ID)
        {
            DrawLine(this, QLineF(static_cast<QPointF>(*first), ScenePos()));
        }
        else
        {
            const QSharedPointer<VPointF> second = GetData()->GeometricObject<VPointF>(m_lineP1Id);
            DrawPoint(m_lineP1, static_cast<QPointF>(*second));

            QLineF base_line;
            if (m_lineP2Id <= NULL_ID)
            {
                base_line = QLineF(static_cast<QPointF>(*second), ScenePos());
                DrawLine(m_line, base_line);
            }
            else
            {
                const QSharedPointer<VPointF> third = GetData()->GeometricObject<VPointF>(m_lineP2Id);
                DrawPoint(m_lineP2, static_cast<QPointF>(*third));

                base_line = QLineF(static_cast<QPointF>(*second), static_cast<QPointF>(*third));
            }

            DrawLine(m_line, base_line);

            QPointF height = VToolHeight::FindPoint(base_line, static_cast<QPointF>(*first));
            DrawPoint(m_point, height);

            QLineF height_line(static_cast<QPointF>(*first), height);
            DrawLine(this, height_line, LineStyle());

            ShowIntersection(height_line, base_line);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolHeight::VisualMode(quint32 id)
{
    m_basePointId = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolHeight::ShowIntersection(const QLineF &height_line, const QLineF &base_line)
{
    QPointF p;
    QLineF::IntersectType intersect = height_line.intersects(base_line, &p);

    if (intersect == QLineF::UnboundedIntersection)
    {
        m_lineIntersection->setVisible(true);
        DrawLine(m_lineIntersection, QLineF(base_line.p1(), height_line.p2()), Qt::DashLine);
    }
    else if (intersect == QLineF::BoundedIntersection)
    {
        m_lineIntersection->setVisible(false);
    }
}
