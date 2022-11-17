/************************************************************************
 **
 **  @file   vislineintersectaxis.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   19 10, 2014
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

#include "vistoollineintersectaxis.h"

#include <QColor>
#include <QGraphicsLineItem>
#include <QLine>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <new>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoollineintersectaxis.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "visline.h"
#include "../vmisc/vmodifierkey.h"
#include "../vmisc/compatibility.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolLineIntersectAxis::VisToolLineIntersectAxis(const VContainer *data, QGraphicsItem *parent)
    : VisLine(data, parent)
{
    SetMainColor(Qt::red);

    m_lineP1 = InitPoint(Color(VColor::SupportColor), this);
    m_lineP2 = InitPoint(Color(VColor::SupportColor), this);
    m_basePoint = InitPoint(Color(VColor::SupportColor), this);
    m_baseLine = InitItem<VScaledLine>(Color(VColor::SupportColor), this);
    m_axisLine = InitItem<VScaledLine>(Color(VColor::SupportColor), this);
    m_lineIntersection = InitItem<VScaledLine>(Color(VColor::SupportColor), this);
    m_point = InitPoint(Color(VColor::MainColor), this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolLineIntersectAxis::RefreshGeometry()
{
    if (m_point1Id > NULL_ID)
    {
        const QSharedPointer<VPointF> first = GetData()->GeometricObject<VPointF>(m_point1Id);
        DrawPoint(m_lineP1, static_cast<QPointF>(*first), Color(VColor::SupportColor));

        if (m_point2Id <= NULL_ID)
        {
            DrawLine(m_baseLine, QLineF(static_cast<QPointF>(*first), ScenePos()), Color(VColor::SupportColor));
        }
        else
        {
            const QSharedPointer<VPointF> second = GetData()->GeometricObject<VPointF>(m_point2Id);
            DrawPoint(m_lineP2, static_cast<QPointF>(*second), Color(VColor::SupportColor));

            const QLineF base_line(static_cast<QPointF>(*first), static_cast<QPointF>(*second));
            DrawLine(m_baseLine, base_line, Color(VColor::SupportColor));

            if (m_axisPointId > NULL_ID)
            {
                QLineF axis;
                const QSharedPointer<VPointF> third = GetData()->GeometricObject<VPointF>(m_axisPointId);
                DrawPoint(m_basePoint, static_cast<QPointF>(*third), Color(VColor::MainColor));

                if (VFuzzyComparePossibleNulls(m_angle, -1))
                {
                    if (GetMode() == Mode::Show)
                    {
                        return;
                    }
                    axis = Axis(static_cast<QPointF>(*third), ScenePos());
                }
                else
                {
                    axis = Axis(static_cast<QPointF>(*third), m_angle);
                }

                DrawLine(m_axisLine, axis, Color(VColor::SupportColor), Qt::DashLine);

                QPointF p;
                VToolLineIntersectAxis::FindPoint(axis, base_line, &p);
                QLineF axis_line(static_cast<QPointF>(*third), p);
                if (not axis_line.isNull())
                {
                    DrawLine(this, axis_line, Color(VColor::MainColor), LineStyle());
                }

                DrawPoint(m_point, p, Color(VColor::MainColor));
                ShowIntersection(axis_line, base_line);

                if (GetMode() == Mode::Creation)
                {
                    SetToolTip(tr("<b>Intersection line and axis</b>: angle = %1°; <b>%2</b> - "
                                  "sticking angle, <b>%3</b> - finish creation")
                                   .arg(AngleToUser(this->line().angle()), VModifierKey::Shift(),
                                        VModifierKey::EnterKey()));
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolLineIntersectAxis::VisualMode(quint32 id)
{
    m_point1Id = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolLineIntersectAxis::Angle() const -> QString
{
    return QString::number(this->line().angle());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolLineIntersectAxis::SetAngle(const QString &expression)
{
    m_angle = FindValFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolLineIntersectAxis::ShowIntersection(const QLineF &axis_line, const QLineF &base_line)
{
    QPointF p;
    QLineF::IntersectType intersect = Intersects(axis_line, base_line, &p);

    if (intersect == QLineF::UnboundedIntersection)
    {
        m_lineIntersection->setVisible(true);
        DrawLine(m_lineIntersection, QLineF(base_line.p1(), axis_line.p2()), Color(VColor::SupportColor), Qt::DashLine);
    }
    else if (intersect == QLineF::BoundedIntersection || intersect == QLineF::NoIntersection)
    {
        m_lineIntersection->setVisible(false);
    }
}
