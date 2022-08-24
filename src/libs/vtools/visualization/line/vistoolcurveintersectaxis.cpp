/************************************************************************
 **
 **  @file   vistoolcurveintersectaxis.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 10, 2014
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

#include "vistoolcurveintersectaxis.h"

#include <QColor>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QLineF>
#include <QPainterPath>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <new>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolcurveintersectaxis.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "visline.h"
#include "../vmisc/vmodifierkey.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolCurveIntersectAxis::VisToolCurveIntersectAxis(const VContainer *data, QGraphicsItem *parent)
    : VisLine(data, parent)
{
    SetMainColor(Qt::red);

    m_visCurve = InitItem<VCurvePathItem>(Qt::darkGreen, this);
    m_basePoint = InitPoint(Color(VColor::SupportColor), this);
    m_baseLine = InitItem<VScaledLine>(Color(VColor::SupportColor), this);
    m_axisLine = InitItem<VScaledLine>(Color(VColor::SupportColor), this); //-V656
    m_point = InitPoint(Color(VColor::MainColor), this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCurveIntersectAxis::RefreshGeometry()
{
    if (m_curveId > NULL_ID)
    {
        const QSharedPointer<VAbstractCurve> curve = GetData()->GeometricObject<VAbstractCurve>(m_curveId);
        DrawPath(m_visCurve, curve->GetPath(), curve->DirectionArrows(), Color(VColor::SupportColor), Qt::SolidLine,
                 Qt::RoundCap);

        if (m_axisPointId > NULL_ID)
        {
            QLineF axis;
            const QSharedPointer<VPointF> first = GetData()->GeometricObject<VPointF>(m_axisPointId);
            if (VFuzzyComparePossibleNulls(m_angle, -1))
            {
                axis = Axis(static_cast<QPointF>(*first), ScenePos());
            }
            else
            {
                axis = Axis(static_cast<QPointF>(*first), m_angle);
            }
            DrawPoint(m_basePoint, static_cast<QPointF>(*first), Color(VColor::MainColor));
            DrawLine(m_axisLine, axis, Color(VColor::SupportColor), Qt::DashLine);

            QPointF p;
            VToolCurveIntersectAxis::FindPoint(static_cast<QPointF>(*first), axis.angle(), curve->GetPoints(), &p);
            QLineF axis_line(static_cast<QPointF>(*first), p);
            DrawLine(this, axis_line, Color(VColor::MainColor), LineStyle());

            DrawPoint(m_point, p, Color(VColor::MainColor));

            SetToolTip(tr("<b>Intersection curve and axis</b>: angle = %1°; <b>%2</b> - "
                          "sticking angle, <b>%3</b> - finish creation")
                           .arg(AngleToUser(this->line().angle()), VModifierKey::Shift(), VModifierKey::EnterKey()));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCurveIntersectAxis::VisualMode(quint32 id)
{
    m_curveId = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolCurveIntersectAxis::Angle() const -> QString
{
    return QString::number(this->line().angle());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCurveIntersectAxis::SetAngle(const QString &expression)
{
    m_angle = FindValFromUser(expression, GetData()->DataVariables());
}
