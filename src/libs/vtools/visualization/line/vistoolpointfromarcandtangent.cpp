/************************************************************************
 **
 **  @file   vistoolpointfromarcandtangent.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "vistoolpointfromarcandtangent.h"

#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QLineF>
#include <QPainterPath>
#include <QSharedPointer>
#include <new>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/vtoolpointfromarcandtangent.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../visualization.h"
#include "../vpatterndb/vcontainer.h"
#include "visline.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolPointFromArcAndTangent::VisToolPointFromArcAndTangent(const VContainer *data, QGraphicsItem *parent)
  : VisLine(data, parent)
{
    SetColorRole(VColorRole::VisSupportColor);

    m_arcPath = InitItem<VCurvePathItem>(VColorRole::VisSupportColor2, this);
    m_point = InitPoint(VColorRole::VisMainColor, this);
    m_tangent = InitPoint(VColorRole::VisSupportColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointFromArcAndTangent::RefreshGeometry()
{
    if (m_pointId > NULL_ID) // tangent point
    {
        const QSharedPointer<VPointF> tan = GetData()->GeometricObject<VPointF>(m_pointId);
        DrawPoint(m_tangent, static_cast<QPointF>(*tan));

        if (m_arcId > NULL_ID) // circle center
        {
            const QSharedPointer<VArc> arc = GetData()->GeometricObject<VArc>(m_arcId);
            DrawPath(m_arcPath, arc->GetPath(), arc->DirectionArrows(), Qt::SolidLine, Qt::RoundCap);

            FindRays(static_cast<QPointF>(*tan), arc.data());

            QPointF fPoint;
            VToolPointFromArcAndTangent::FindPoint(static_cast<QPointF>(*tan), arc.data(), m_crossPoint, &fPoint);
            DrawPoint(m_point, fPoint);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointFromArcAndTangent::VisualMode(quint32 id)
{
    m_pointId = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointFromArcAndTangent::FindRays(const QPointF &p, const VArc *arc)
{
    QPointF p1, p2;
    const auto center = static_cast<QPointF>(arc->GetCenter());
    const qreal radius = arc->GetRadius();
    const int res = VGObject::ContactPoints(p, center, radius, p1, p2);

    switch (res)
    {
        case 2:
        {
            QLineF r1Arc(center, p1);
            r1Arc.setLength(radius + 10);

            QLineF r2Arc(center, p2);
            r2Arc.setLength(radius + 10);

            int localRes = 0;
            bool flagP1 = false;

            if (arc->IsIntersectLine(r1Arc))
            {
                ++localRes;
                flagP1 = true;
            }

            if (arc->IsIntersectLine(r2Arc))
            {
                ++localRes;
            }

            switch (localRes)
            {
                case 2:
                    DrawRay(this, p, m_crossPoint == CrossCirclesPoint::FirstPoint ? p1 : p2, Qt::DashLine);
                    break;
                case 1:
                    DrawRay(this, p, flagP1 ? p1 : p2, Qt::DashLine);
                    break;
                case 0:
                default:
                    this->setVisible(false);
                    break;
            }

            break;
        }
        case 1:
            DrawRay(this, p, p1, Qt::DashLine);
            break;
        case 3:
        case 0:
        default:
            this->setVisible(false);
            break;
    }
}
