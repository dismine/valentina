/************************************************************************
 **
 **  @file   vistoolellipticalarc.cpp
 **  @author Valentina Zhuravska <zhuravska19(at)gmail.com>
 **  @date   24 10, 2016
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
#include "vistoolellipticalarc.h"
#include <QPainterPath>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <new>

#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "../vwidgets/scalesceneitems.h"
#include "vispath.h"
#include "../vmisc/vmodifierkey.h"

namespace
{
auto Angle(const QLineF &radius) -> qreal
{
    if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
    {
        QLineF correction = radius;
        correction.setAngle(Visualization::CorrectAngle(correction.angle()));
        return correction.angle();
    }

    return radius.angle();
}
}  // namespace

//---------------------------------------------------------------------------------------------------------------------
VisToolEllipticalArc::VisToolEllipticalArc(const VContainer *data, QGraphicsItem *parent)
    :VisPath(data, parent)
{
    m_arcCenter = InitPoint(Color(VColor::MainColor), this);
    m_radius1Line = InitItem<VScaledLine>(Color(VColor::SupportColor), this);
    m_radius2Line = InitItem<VScaledLine>(Color(VColor::SupportColor), this);
    m_f1Point = InitPoint(Color(VColor::SupportColor), this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArc::RefreshGeometry()
{
    if (m_centerId > NULL_ID)
    {
        const QSharedPointer<VPointF> first = GetData()->GeometricObject<VPointF>(m_centerId);
        DrawPoint(m_arcCenter, static_cast<QPointF>(*first), Color(VColor::SupportColor));

        static const QString prefix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
        QLineF radius (static_cast<QPointF>(*first), ScenePos());
        auto center = static_cast<QPointF>(*first);

        if (GetMode() == Mode::Creation)
        {
            if (qFuzzyIsNull(m_radius1))
            {
                DrawRadius1Line(center, radius.length());

                SetToolTip(tr("<b>Elliptical arc</b>: radius1 = %1%2; "
                              "<b>Mouse click</b> - finish selecting the first radius, "
                              "<b>%3</b> - skip").arg(LengthToUser(radius.length()), prefix, VModifierKey::EnterKey()));
            }
            else if (qFuzzyIsNull(m_radius2))
            {
                DrawRadius2Line(center, m_radius1);

                QLineF radius2Line(center.x(), center.y(), center.x(), center.y() - 100);
                QPointF p = VGObject::ClosestPoint(radius2Line, ScenePos());
                radius = QLineF(static_cast<QPointF>(*first), p);

                DrawRadius2Line(center, radius.length());
                DrawElArc(*first, m_radius1, radius.length(), 0, 0);

                SetToolTip(tr("<b>Elliptical arc</b>: radius1 = %1%2, "
                              "radius2 = %3%2; "
                              "<b>Mouse click</b> - finish selecting the second radius, "
                              "<b>%4</b> - skip")
                               .arg(LengthToUser(m_radius1), prefix, LengthToUser(radius.length()),
                                    VModifierKey::EnterKey()));
            }
            else if (m_f1 < 0)
            {
                DrawRadius1Line(center, m_radius1);
                DrawRadius2Line(center, m_radius2);

                qreal f1Angle = Angle(radius);
                VEllipticalArc elArc = DrawElArc(*first, m_radius1, m_radius2, f1Angle, f1Angle);

                DrawPoint(m_f1Point, elArc.GetP1(), Color(VColor::SupportColor));

                SetToolTip(tr("<b>Elliptical arc</b>: radius1 = %1%2, "
                              "radius2 = %3%2, angle1 = %4°; "
                              "<b>Mouse click</b> - finish selecting the second radius, "
                              "<b>%5</b> - sticking angle, "
                              "<b>%6</b> - skip")
                               .arg(LengthToUser(m_radius1), prefix, LengthToUser(m_radius2),
                                    AngleToUser(f1Angle), VModifierKey::Shift(), VModifierKey::EnterKey()));
            }
            else if (m_f2 < 0)
            {
                DrawRadius1Line(center, m_radius1);
                DrawRadius2Line(center, m_radius2);

                const qreal f2Angle = Angle(radius);
                VEllipticalArc elArc = DrawElArc(*first, m_radius1, m_radius2, m_f1, f2Angle);

                DrawPoint(m_f1Point, elArc.GetP1(), Color(VColor::SupportColor));

                SetToolTip(tr("<b>Elliptical arc</b>: radius1 = %1%2, "
                              "radius2 = %3%2, angle1 = %4°, angle2 = %5°; "
                              "<b>Mouse click</b> - finish selecting the second radius, "
                              "<b>%6</b> - sticking angle, "
                              "<b>%7</b> - skip")
                               .arg(LengthToUser(m_radius1), prefix, LengthToUser(m_radius2),
                                    AngleToUser(m_f1), AngleToUser(f2Angle), VModifierKey::Shift(),
                                    VModifierKey::EnterKey()));
            }
            else if (VFuzzyComparePossibleNulls(m_rotationAngle, INT_MAX))
            {
                if (VFuzzyComparePossibleNulls(m_startingRotationAngle, INT_MAX))
                {
                    m_startingRotationAngle = radius.angle();
                }

                qreal rotationAngle = Angle(radius) - m_startingRotationAngle;

                DrawRadius1Line(center, m_radius1, rotationAngle);
                DrawRadius2Line(center, m_radius2, rotationAngle);
                VEllipticalArc elArc = DrawElArc(*first, m_radius1, m_radius2, m_f1, m_f2, rotationAngle);

                DrawPoint(m_f1Point, elArc.GetP1(), Color(VColor::SupportColor));

                SetToolTip(tr("<b>Elliptical arc</b>: radius1 = %1%2, "
                              "radius2 = %3%2, angle1 = %4°, angle2 = %5°, rotation = %6°; "
                              "<b>Mouse click</b> - finish selecting the second radius, "
                              "<b>%7</b> - sticking angle, "
                              "<b>%8</b> - skip")
                               .arg(LengthToUser(m_radius1), prefix, LengthToUser(radius.length()),
                                    AngleToUser(m_f1), AngleToUser(m_f2), AngleToUser(rotationAngle),
                                    VModifierKey::Shift(), VModifierKey::EnterKey()));
            }
        }
        else if (not qFuzzyIsNull(m_radius1) && not qFuzzyIsNull(m_radius2) && m_f1 >= 0 && m_f2 >= 0 &&
                 not VFuzzyComparePossibleNulls(m_rotationAngle, INT_MAX))
        {
            DrawElArc(*first, m_radius1, m_radius2, m_f1, m_f2, m_rotationAngle);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArc::VisualMode(quint32 id)
{
    m_centerId = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArc::SetRadius1(const QString &expression)
{
    m_radius1 = FindLengthFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArc::SetRadius2(const QString &expression)
{
    m_radius2 = FindLengthFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArc::SetF1(const QString &expression)
{
    m_f1 = FindValFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArc::SetF2(const QString &expression)
{
    m_f2 = FindValFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArc::SetRotationAngle(const QString &expression)
{
    m_rotationAngle = FindValFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArc::DrawRadius1Line(const QPointF &center, qreal radius, qreal rotationAngle)
{
    QLineF radiusLine(center.x(), center.y(), center.x() + 100, center.y());
    radiusLine.setLength(radius);
    radiusLine.setAngle(radiusLine.angle() + rotationAngle);
    DrawLine(m_radius1Line, radiusLine, Color(VColor::SupportColor));
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArc::DrawRadius2Line(const QPointF &center, qreal radius, qreal rotationAngle)
{
    QLineF radiusLine(center.x(), center.y(), center.x(), center.y() - 100);
    radiusLine.setLength(radius);
    radiusLine.setAngle(radiusLine.angle() + rotationAngle);
    DrawLine(m_radius2Line, radiusLine, Color(VColor::SupportColor));
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolEllipticalArc::DrawElArc(const VPointF &center, qreal radius1, qreal radius2, qreal f1, qreal f2,
                                     qreal rotationAngle) -> VEllipticalArc
{
    VEllipticalArc elArc(center, radius1, radius2, f1, f2, rotationAngle);
    elArc.SetApproximationScale(ApproximationScale());
    DrawPath(this, elArc.GetPath(), elArc.DirectionArrows(), Color(VColor::MainColor), LineStyle(), Qt::RoundCap);

    return elArc;
}
