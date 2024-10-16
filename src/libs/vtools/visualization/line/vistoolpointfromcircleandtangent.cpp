/************************************************************************
 **
 **  @file   vistoolpointfromcircleandtangent.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   5 6, 2015
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

#include "vistoolpointfromcircleandtangent.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QSharedPointer>
#include <new>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/vtoolpointfromcircleandtangent.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../visualization.h"
#include "../vmisc/vmodifierkey.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/global.h"
#include "visline.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolPointFromCircleAndTangent::VisToolPointFromCircleAndTangent(const VContainer *data, QGraphicsItem *parent)
  : VisLine(data, parent)
{
    SetColorRole(VColorRole::VisSupportColor);

    m_cPath = InitItem<VScaledEllipse>(VColorRole::VisSupportColor2, this);
    m_cPath->SetPointMode(false);
    m_point = InitPoint(VColorRole::VisMainColor, this);
    m_tangent = InitPoint(VColorRole::VisSupportColor, this);
    m_cCenter = InitPoint(VColorRole::VisSupportColor, this); //-V656
    m_tangent2 = InitItem<VScaledLine>(VColorRole::VisSupportColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointFromCircleAndTangent::RefreshGeometry()
{
    if (m_pointId > NULL_ID) // tangent point
    {
        const QSharedPointer<VPointF> tan = GetData()->GeometricObject<VPointF>(m_pointId);
        DrawPoint(m_tangent, static_cast<QPointF>(*tan));

        if (m_centerId > NULL_ID) // circle center
        {
            const QSharedPointer<VPointF> center = GetData()->GeometricObject<VPointF>(m_centerId);
            DrawPoint(m_cCenter, static_cast<QPointF>(*center));

            if (m_cRadius > 0)
            {
                m_cPath->setRect(PointRect(m_cRadius));
                DrawPoint(m_cPath, static_cast<QPointF>(*center), Qt::DashLine);

                FindRays(static_cast<QPointF>(*tan), static_cast<QPointF>(*center), m_cRadius);

                QPointF fPoint;
                VToolPointFromCircleAndTangent::FindPoint(static_cast<QPointF>(*tan), static_cast<QPointF>(*center),
                                                          m_cRadius, m_crossPoint, &fPoint);
                DrawPoint(m_point, fPoint);
            }
            else if (GetMode() == Mode::Creation)
            {
                QLineF const cursorLine(static_cast<QPointF>(*center), ScenePos());
                qreal const len = cursorLine.length();

                m_cPath->setRect(PointRect(len));
                DrawPoint(m_cPath, static_cast<QPointF>(*center), Qt::DashLine);

                FindRays(static_cast<QPointF>(*tan), static_cast<QPointF>(*center), len);

                QPointF fPoint;
                VToolPointFromCircleAndTangent::FindPoint(static_cast<QPointF>(*tan), static_cast<QPointF>(*center),
                                                          len, m_crossPoint, &fPoint);
                DrawPoint(m_point, fPoint);

                const QString prefix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
                SetToolTip(tr("Radius = %1%2; "
                              "<b>Mouse click</b> - finish selecting the radius, "
                              "<b>%3</b> - skip")
                               .arg(LengthToUser(len), prefix, VModifierKey::EnterKey()));
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointFromCircleAndTangent::VisualMode(quint32 id)
{
    m_pointId = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointFromCircleAndTangent::SetCRadius(const QString &value)
{
    m_cRadius = FindLengthFromUser(value, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointFromCircleAndTangent::FindRays(const QPointF &p, const QPointF &center, qreal radius)
{
    QPointF p1, p2;
    const int res = VGObject::ContactPoints(p, center, radius, p1, p2);

    switch (res)
    {
        case 2:
            DrawRay(this, p, p1, Qt::DashLine);
            DrawRay(m_tangent2, p, p2, Qt::DashLine);
            break;
        case 1:
            DrawRay(this, p, p1, Qt::DashLine);
            m_tangent2->setVisible(false);
            break;
        default:
            this->setVisible(false);
            m_tangent2->setVisible(false);
            break;
    }
}
