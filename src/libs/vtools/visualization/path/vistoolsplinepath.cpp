/************************************************************************
 **
 **  @file   vistoolsplinepath.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 9, 2014
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

#include "vistoolsplinepath.h"

#include <QGraphicsPathItem>
#include <QLineF>
#include <QPainterPath>
#include <QPoint>
#include <Qt>
#include <QtAlgorithms>

#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepoint.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vcontrolpointspline.h"
#include "../vwidgets/scalesceneitems.h"
#include "../vwidgets/global.h"
#include "../visualization.h"
#include "vispath.h"
#include "../vmisc/vmodifierkey.h"

namespace
{
inline auto TriggerRadius() -> qreal
{
    return ScaledRadius(SceneScale(VAbstractValApplication::VApp()->getCurrentScene()))*1.5;
}
}

//---------------------------------------------------------------------------------------------------------------------
VisToolSplinePath::VisToolSplinePath(const VContainer *data, QGraphicsItem *parent)
    : VisPath(data, parent)
{
    m_newCurveSegment = InitItem<VCurvePathItem>(Color(VColor::MainColor), this);
}

//---------------------------------------------------------------------------------------------------------------------
VisToolSplinePath::~VisToolSplinePath()
{
    emit ToolTip(QString());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolSplinePath::RefreshGeometry()
{
    if (m_path.CountPoints() == 0)
    {
        return;
    }

    const QVector<VSplinePoint> pathPoints = m_path.GetSplinePath();
    const auto size = pathPoints.size();

    for (int i = 0; i < size; ++i)
    {
        VScaledEllipse *point = GetPoint(static_cast<unsigned>(i));
        DrawPoint(point, static_cast<QPointF>(pathPoints.at(i).P()), Color(VColor::SupportColor));
    }

    if (GetMode() == Mode::Creation)
    {
        if (size > 1)
        {
            for (vsizetype i = 1; i<=m_path.CountSubSpl(); ++i)
            {
                const auto preLastPoint = (m_path.CountSubSpl() - 1) * 2;
                const auto lastPoint = preLastPoint + 1;

                VSpline spl = m_path.GetSpline(i);

                m_ctrlPoints.at(preLastPoint)->RefreshCtrlPoint(i, SplinePointPosition::FirstPoint,
                                                                static_cast<QPointF>(spl.GetP2()),
                                                                static_cast<QPointF>(spl.GetP1()));
                m_ctrlPoints.at(lastPoint)->RefreshCtrlPoint(i, SplinePointPosition::LastPoint,
                                                             static_cast<QPointF>(spl.GetP3()),
                                                             static_cast<QPointF>(spl.GetP4()));
            }
        }

        Creating(static_cast<QPointF>(pathPoints.at(size-1).P()), size);
    }

    if (size == 1)
    {
        VSpline spline(pathPoints.at(0).P(), m_ctrlPoint, ScenePos(), VPointF(ScenePos()));
        spline.SetApproximationScale(ApproximationScale());
        DrawPath(this, spline.GetPath(), Color(VColor::MainColor), LineStyle(), Qt::RoundCap);
    }
    else if (size > 1)
    {
        DrawPath(this, m_path.GetPath(), m_path.DirectionArrows(), Color(VColor::MainColor), LineStyle(), Qt::RoundCap);
    }

    if (m_path.CountPoints() < 3)
    {
        SetToolTip(tr("<b>Curved path</b>: select three or more points"));
    }
    else
    {
        SetToolTip(tr("<b>Curved path</b>: select three or more points, "
                      "<b>%1</b> - finish creation").arg(VModifierKey::EnterKey()));
    }
    if (GetMode() == Mode::Show)
    {
        SetToolTip(tr("Use <b>%1</b> for sticking angle!").arg(VModifierKey::Shift()));
        emit ToolTip(CurrentToolTip());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolSplinePath::VisualMode(quint32 id)
{
    Q_UNUSED(id)
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolSplinePath::MouseLeftPressed()
{
    if (GetMode() == Mode::Creation)
    {
        m_isLeftMousePressed = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolSplinePath::MouseLeftReleased()
{
    if (GetMode() == Mode::Creation)
    {
        m_isLeftMousePressed = false;
        RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolSplinePath::GetPoint(quint32 i) -> VScaledEllipse *
{
    if (static_cast<quint32>(m_points.size() - 1) >= i && not m_points.isEmpty())
    {
        return m_points.at(static_cast<int>(i));
    }

    m_pointSelected = false;

    auto *point = InitPoint(Color(VColor::SupportColor), this);
    m_points.append(point);

    if (m_points.size() == 1)
    {
        auto *controlPoint1 = new VControlPointSpline(m_points.size(), SplinePointPosition::FirstPoint, this);
        controlPoint1->hide();
        m_ctrlPoints.append(controlPoint1);
    }
    else
    {
        auto *controlPoint1 = new VControlPointSpline(m_points.size()-1, SplinePointPosition::LastPoint, this);
        controlPoint1->hide();
        m_ctrlPoints.append(controlPoint1);

        auto *controlPoint2 = new VControlPointSpline(m_points.size(), SplinePointPosition::FirstPoint, this);
        controlPoint2->hide();
        m_ctrlPoints.append(controlPoint2);
    }

    return point;
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolSplinePath::DragControlPoint(vsizetype lastPoint, vsizetype preLastPoint, const QPointF &pSpl,
                                         vsizetype size)
{
    if (not m_ctrlPoints.at(lastPoint)->isVisible())
    {
        //Radius of point circle, but little bigger. Need handle with hover sizes.
        if (QLineF(pSpl, m_ctrlPoint).length() > TriggerRadius())
        {
            if (size == 1)
            {
                m_ctrlPoints.at(lastPoint)->show();
            }
            else
            {
                m_ctrlPoints.at(preLastPoint)->show();
                m_ctrlPoints.at(lastPoint)->show();
            }
        }
        else
        {
            m_ctrlPoint = pSpl;
        }
    }

    QLineF ctrlLine (pSpl, ScenePos());
    ctrlLine.setAngle(ctrlLine.angle()+180);

    if (size == 1)
    {
        m_ctrlPoints.at(lastPoint)->RefreshCtrlPoint(size, SplinePointPosition::FirstPoint, m_ctrlPoint, pSpl);
    }
    else
    {
        m_ctrlPoints.at(preLastPoint)->RefreshCtrlPoint(size-1, SplinePointPosition::LastPoint, ctrlLine.p2(), pSpl);
        m_ctrlPoints.at(lastPoint)->RefreshCtrlPoint(size, SplinePointPosition::FirstPoint, m_ctrlPoint, pSpl);
    }

    VSpline spline(VPointF(pSpl), m_ctrlPoint, ScenePos(), VPointF(ScenePos()));

    if (size == 1)
    {
        NewCurveSegment(spline, pSpl, size);
    }
    else
    {
        const VSpline spl = m_path.GetSpline(size - 1);
        VSpline preSpl(spl.GetP1(), static_cast<QPointF>(spl.GetP2()), ctrlLine.p2(), VPointF(pSpl));

        m_path[size-1].SetAngle2(spline.GetStartAngle(), spline.GetStartAngleFormula());
        if (m_ctrlPoint != pSpl)
        {
            m_path[size-1].SetLength1(preSpl.GetC2Length(), preSpl.GetC2LengthFormula());
            m_path[size-1].SetLength2(spline.GetC1Length(), spline.GetC1LengthFormula());
        }
        else
        {
            m_path[size-1].SetLength1(0, QChar('0'));
            m_path[size-1].SetLength2(0, QChar('0'));
        }
        emit PathChanged(m_path);
    }

    DrawPath(m_newCurveSegment, spline.GetPath(), Color(VColor::MainColor), Qt::SolidLine, Qt::RoundCap);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolSplinePath::NewCurveSegment(const VSpline &spline, const QPointF &pSpl, vsizetype size)
{
    m_path[size-1].SetAngle2(spline.GetStartAngle(), spline.GetStartAngleFormula());

    if (m_ctrlPoint != pSpl)
    {
        m_path[size-1].SetLength2(spline.GetC1Length(), spline.GetC1LengthFormula());
    }
    else
    {
        m_path[size-1].SetLength2(0, QChar('0'));
    }
    emit PathChanged(m_path);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolSplinePath::Creating(const QPointF &pSpl, vsizetype size)
{
    vsizetype lastPoint = 0;
    vsizetype preLastPoint = 0;

    if (size > 1)
    {
        lastPoint = (size - 1) * 2;
        preLastPoint = lastPoint - 1;
    }

    if (m_isLeftMousePressed && not m_pointSelected)
    {
        m_newCurveSegment->hide();
        m_ctrlPoint = ScenePos();

        DragControlPoint(lastPoint, preLastPoint, pSpl, size);
    }
    else
    {
        m_pointSelected = true;

        VSpline spline(VPointF(pSpl), m_ctrlPoint, ScenePos(), VPointF(ScenePos()));
        NewCurveSegment(spline, pSpl, size);
        DrawPath(m_newCurveSegment, spline.GetPath(), Color(VColor::MainColor), Qt::SolidLine, Qt::RoundCap);
    }
}
