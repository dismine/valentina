/************************************************************************
 **
 **  @file   vistoolspline.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 8, 2014
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

#include "vistoolspline.h"

#include <QLineF>
#include <QPainterPath>
#include <QSharedPointer>
#include <Qt>
#include <new>

#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vcontrolpointspline.h"
#include "../vwidgets/scalesceneitems.h"
#include "../vwidgets/global.h"
#include "../visualization.h"
#include "vispath.h"
#include "../vmisc/vmodifierkey.h"

const int EMPTY_ANGLE = -1;

namespace
{
inline auto TriggerRadius() -> qreal
{
    return ScaledRadius(SceneScale(VAbstractValApplication::VApp()->getCurrentScene()))*1.5;
}
}

//---------------------------------------------------------------------------------------------------------------------
VisToolSpline::VisToolSpline(const VContainer *data, QGraphicsItem *parent)
    : VisPath(data, parent),
      m_angle1(EMPTY_ANGLE),
      m_angle2(EMPTY_ANGLE)
{
    m_point1 = InitPoint(Color(VColor::SupportColor), this);
    m_point4 = InitPoint(Color(VColor::SupportColor), this); //-V656

    auto *controlPoint1 = new VControlPointSpline(1, SplinePointPosition::FirstPoint, this);
    controlPoint1->hide();
    m_controlPoints.append(controlPoint1);

    auto *controlPoint2 = new VControlPointSpline(1, SplinePointPosition::LastPoint, this);
    controlPoint2->hide();
    m_controlPoints.append(controlPoint2);
}

//---------------------------------------------------------------------------------------------------------------------
VisToolSpline::~VisToolSpline()
{
    emit ToolTip(QString());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolSpline::RefreshGeometry()
{
    if (m_point1Id > NULL_ID)
    {
        const auto first = GetData()->GeometricObject<VPointF>(m_point1Id);
        DrawPoint(m_point1, static_cast<QPointF>(*first), Color(VColor::SupportColor));

        if (GetMode() == Mode::Creation)
        {
            DragFirstControlPoint(static_cast<QPointF>(*first));
        }

        if (m_point4Id <= NULL_ID)
        {
            VSpline spline(*first, m_p2, ScenePos(), VPointF(ScenePos()));
            spline.SetApproximationScale(ApproximationScale());
            DrawPath(this, spline.GetPath(), Color(VColor::MainColor), LineStyle(), Qt::RoundCap);
        }
        else
        {
            const auto second = GetData()->GeometricObject<VPointF>(m_point4Id);
            DrawPoint(m_point4, static_cast<QPointF>(*second), Color(VColor::SupportColor));

            if (GetMode() == Mode::Creation)
            {
                DragLastControlPoint(static_cast<QPointF>(*second));
            }

            if (VFuzzyComparePossibleNulls(m_angle1, EMPTY_ANGLE) || VFuzzyComparePossibleNulls(m_angle2, EMPTY_ANGLE))
            {
                VSpline spline(*first, m_p2, m_p3, *second);
                spline.SetApproximationScale(ApproximationScale());
                DrawPath(this, spline.GetPath(), Color(VColor::MainColor), LineStyle(), Qt::RoundCap);
            }
            else
            {
                VSpline spline(*first, *second, m_angle1, m_angle2, m_kAsm1, m_kAsm2, m_kCurve);
                spline.SetApproximationScale(ApproximationScale());
                DrawPath(this, spline.GetPath(), spline.DirectionArrows(), Color(VColor::MainColor), LineStyle(),
                         Qt::RoundCap);
                SetToolTip(tr("Use <b>%1</b> for sticking angle!").arg(VModifierKey::Shift()));
                emit ToolTip(CurrentToolTip());
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolSpline::VisualMode(quint32 id)
{
    m_point1Id = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolSpline::MouseLeftPressed()
{
    if (GetMode() == Mode::Creation)
    {
        m_isLeftMousePressed = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolSpline::MouseLeftReleased()
{
    if (GetMode() == Mode::Creation)
    {
        m_isLeftMousePressed = false;
        RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolSpline::DragFirstControlPoint(const QPointF &point)
{
    if (m_isLeftMousePressed && not m_p2Selected)
    {
        m_p2 = ScenePos();
        m_controlPoints.at(0)->RefreshCtrlPoint(1, SplinePointPosition::FirstPoint, m_p2, point);

        if (not m_controlPoints.at(0)->isVisible())
        {
            if (QLineF(point, m_p2).length() > TriggerRadius())
            {
                m_controlPoints.at(0)->show();
            }
            else
            {
                m_p2 = point;
            }
        }
    }
    else
    {
        m_p2Selected = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolSpline::DragLastControlPoint(const QPointF &point)
{
    if (m_isLeftMousePressed && not m_p3Selected)
    {
        m_p3 = ScenePos();
        m_controlPoints.at(1)->RefreshCtrlPoint(1, SplinePointPosition::LastPoint, m_p3, point);

        if (not m_controlPoints.at(1)->isVisible())
        {
            if (QLineF(point, m_p3).length() > TriggerRadius())
            {
                m_controlPoints.at(1)->show();
            }
            else
            {
                m_p3 = point;
            }
        }
    }
    else
    {
        m_p3Selected = true;
    }
}
