/************************************************************************
 **
 **  @file   vistoollineintersect.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 8, 2014
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

#include "vistoollineintersect.h"

#include <QGraphicsLineItem>
#include <QLine>
#include <QLineF>
#include <QPointF>
#include <QSharedPointer>
#include <new>

#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "visline.h"
#include "../vmisc/compatibility.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolLineIntersect::VisToolLineIntersect(const VContainer *data, QGraphicsItem *parent)
    :VisLine(data, parent)
{
    m_line1P1 = InitPoint(Color(VColor::SupportColor), this);
    m_line1P2 = InitPoint(Color(VColor::SupportColor), this);
    m_line1 = InitItem<VScaledLine>(Color(VColor::SupportColor), this);

    m_line2P1 = InitPoint(Color(VColor::SupportColor), this);
    m_line2P2 = InitPoint(Color(VColor::SupportColor), this);

    m_point = InitPoint(Color(VColor::MainColor), this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolLineIntersect::RefreshGeometry()
{
    if (m_line1P1Id > NULL_ID)
    {
        const QSharedPointer<VPointF> first = GetData()->GeometricObject<VPointF>(m_line1P1Id);
        DrawPoint(m_line1P1, static_cast<QPointF>(*first), Color(VColor::SupportColor));

        if (m_line1P2Id <= NULL_ID)
        {
            DrawLine(m_line1, QLineF(static_cast<QPointF>(*first), ScenePos()), Color(VColor::SupportColor));
        }
        else
        {
            const QSharedPointer<VPointF> second = GetData()->GeometricObject<VPointF>(m_line1P2Id);
            DrawPoint(m_line1P2, static_cast<QPointF>(*second), Color(VColor::SupportColor));

            DrawLine(m_line1, QLineF(static_cast<QPointF>(*first), static_cast<QPointF>(*second)),
                     Color(VColor::SupportColor));

            if (m_line2P1Id > NULL_ID)
            {
                const QSharedPointer<VPointF> third = GetData()->GeometricObject<VPointF>(m_line2P1Id);
                DrawPoint(m_line2P1, static_cast<QPointF>(*third), Color(VColor::SupportColor));

                if (m_line2P2Id <= NULL_ID)
                {
                    DrawLine(this, QLineF(static_cast<QPointF>(*third), ScenePos()),
                             Color(VColor::SupportColor));

                    QLineF l1(static_cast<QPointF>(*first), static_cast<QPointF>(*second));
                    QLineF l2(static_cast<QPointF>(*third), ScenePos());
                    QPointF fPoint;
                    QLineF::IntersectType intersect = Intersects(l1, l2, &fPoint);

                    if (intersect == QLineF::UnboundedIntersection || intersect == QLineF::BoundedIntersection)
                    {
                        DrawPoint(m_point, fPoint, Color(VColor::MainColor));
                    }
                }
                else
                {
                    const QSharedPointer<VPointF> forth = GetData()->GeometricObject<VPointF>(m_line2P2Id);
                    DrawPoint(m_line2P2, static_cast<QPointF>(*forth), Color(VColor::SupportColor));

                    DrawLine(this, QLineF(static_cast<QPointF>(*third), static_cast<QPointF>(*forth)),
                             Color(VColor::SupportColor));

                    QLineF l1(static_cast<QPointF>(*first), static_cast<QPointF>(*second));
                    QLineF l2(static_cast<QPointF>(*third), static_cast<QPointF>(*forth));
                    QPointF fPoint;
                    QLineF::IntersectType intersect = Intersects(l1, l2, &fPoint);

                    if (intersect == QLineF::UnboundedIntersection || intersect == QLineF::BoundedIntersection)
                    {
                        DrawPoint(m_point, fPoint, Color(VColor::MainColor));
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolLineIntersect::VisualMode(quint32 id)
{
    m_line1P1Id = id;
    StartVisualMode();
}
