/************************************************************************
 **
 **  @file   vistoolcubicbezier.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 3, 2016
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

#include "vistoolcubicbezier.h"

#include <QGraphicsLineItem>
#include <QLineF>
#include <QPainterPath>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <new>

#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "vispath.h"
#include "../vwidgets/scalesceneitems.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolCubicBezier::VisToolCubicBezier(const VContainer *data, QGraphicsItem *parent)
    : VisPath(data, parent)
{
    m_helpLine1 = InitItem<VScaledLine>(Color(VColor::MainColor), this);
    m_helpLine2 = InitItem<VScaledLine>(Color(VColor::MainColor), this);

    m_point1 = InitPoint(Color(VColor::SupportColor), this);
    m_point2 = InitPoint(Color(VColor::SupportColor), this); //-V656
    m_point3 = InitPoint(Color(VColor::SupportColor), this); //-V656
    m_point4 = InitPoint(Color(VColor::SupportColor), this); //-V656
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCubicBezier::RefreshGeometry()
{
    if (m_point1Id > NULL_ID)
    {
        const auto first = GetData()->GeometricObject<VPointF>(m_point1Id);
        DrawPoint(m_point1, static_cast<QPointF>(*first), Qt::DashLine);

        if (m_point2Id <= NULL_ID)
        {
            DrawLine(m_helpLine1, QLineF(static_cast<QPointF>(*first), ScenePos()), Color(VColor::MainColor),
                     Qt::DashLine);
        }
        else
        {
            const auto second = GetData()->GeometricObject<VPointF>(m_point2Id);
            DrawPoint(m_point2, static_cast<QPointF>(*second), Color(VColor::SupportColor));
            DrawLine(m_helpLine1, QLineF(static_cast<QPointF>(*first), static_cast<QPointF>(*second)),
                     Color(VColor::MainColor), Qt::DashLine);

            if (m_point3Id <= NULL_ID)
            {
                VCubicBezier spline(*first, *second, VPointF(ScenePos()), VPointF(ScenePos()));
                spline.SetApproximationScale(ApproximationScale());
                DrawPath(this, spline.GetPath(), Color(VColor::MainColor), LineStyle(), Qt::RoundCap);
            }
            else
            {
                const auto third = GetData()->GeometricObject<VPointF>(m_point3Id);
                DrawPoint(m_point3, static_cast<QPointF>(*third), Color(VColor::SupportColor));

                if (m_point4Id <= NULL_ID)
                {
                    VCubicBezier spline(*first, *second, *third,  VPointF(ScenePos()));
                    spline.SetApproximationScale(ApproximationScale());
                    DrawPath(this, spline.GetPath(), Color(VColor::MainColor), LineStyle(), Qt::RoundCap);
                    DrawLine(m_helpLine2, QLineF(static_cast<QPointF>(*third), ScenePos()), Color(VColor::MainColor),
                             Qt::DashLine);
                }
                else
                {
                    const auto fourth = GetData()->GeometricObject<VPointF>(m_point4Id);
                    DrawPoint(m_point4, static_cast<QPointF>(*fourth), Color(VColor::SupportColor));
                    DrawLine(m_helpLine2, QLineF(static_cast<QPointF>(*fourth), static_cast<QPointF>(*third)),
                             Color(VColor::MainColor), Qt::DashLine);

                    VCubicBezier spline(*first, *second, *third,  *fourth);
                    spline.SetApproximationScale(ApproximationScale());
                    DrawPath(this, spline.GetPath(), spline.DirectionArrows(), Color(VColor::MainColor), LineStyle(),
                             Qt::RoundCap);
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCubicBezier::VisualMode(quint32 id)
{
    m_point1Id = id;
    StartVisualMode();
}
