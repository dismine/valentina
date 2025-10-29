/************************************************************************
 **
 **  @file   vistoolellipticalarcwithlength.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 10, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
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

#include <QPointF>

#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vmodifierkey.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/scalesceneitems.h"
#include "vistoolellipticalarcwithlength.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolEllipticalArcWithLength::VisToolEllipticalArcWithLength(const VContainer *data, QGraphicsItem *parent)
  : VisPath(data, parent)
{
    m_arcCenter = InitPoint(VColorRole::VisSupportColor, this);
    m_radius1Line = InitItem<VScaledLine>(VColorRole::VisSupportColor, this);
    m_radius2Line = InitItem<VScaledLine>(VColorRole::VisSupportColor, this);
    m_f1Point = InitPoint(VColorRole::VisSupportColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArcWithLength::RefreshGeometry()
{
    if (m_centerId == NULL_ID)
    {
        return;
    }

    m_f1Point->setVisible(false);

    const QSharedPointer<VPointF> first = GetData()->GeometricObject<VPointF>(m_centerId);
    DrawPoint(m_arcCenter, static_cast<QPointF>(*first));

    static const QString prefix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
    QLineF radius(static_cast<QPointF>(*first), ScenePos());
    auto center = static_cast<QPointF>(*first);

    if (GetMode() == Mode::Creation)
    {
        auto Angle = [&radius]()
        {
            if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
            {
                QLineF correction = radius;
                correction.setAngle(Visualization::CorrectAngle(correction.angle()));
                return correction.angle();
            }

            return radius.angle();
        };

        if (qFuzzyIsNull(m_radius1))
        {
            DrawRadius1Line(center, radius.length());

            SetToolTip(tr("<b>Elliptical arc</b>: radius1 = %1%2; "
                          "<b>Mouse click</b> - finish selecting the first radius, "
                          "<b>%3</b> - skip")
                           .arg(LengthToUser(radius.length()), prefix, VModifierKey::EnterKey()));
        }
        else if (qFuzzyIsNull(m_radius2))
        {
            DrawRadius2Line(center, m_radius1);

            QLineF const radius2Line(center.x(), center.y(), center.x(), center.y() - 100);
            QPointF const p = VGObject::ClosestPoint(radius2Line, ScenePos());
            radius = QLineF(static_cast<QPointF>(*first), p);

            DrawRadius2Line(center, radius.length());
            DrawElArc(*first, m_radius1, radius.length(), 0, 0);

            SetToolTip(
                tr("<b>Elliptical arc</b>: radius1 = %1%2, "
                   "radius2 = %3%2; "
                   "<b>Mouse click</b> - finish selecting the second radius, "
                   "<b>%4</b> - skip")
                    .arg(LengthToUser(m_radius1), prefix, LengthToUser(radius.length()), VModifierKey::EnterKey()));
        }
        else if (m_f1 < 0)
        {
            DrawRadius1Line(center, m_radius1);
            DrawRadius2Line(center, m_radius2);

            qreal const f1Angle = Angle();
            VEllipticalArc const elArc = DrawElArc(*first, m_radius1, m_radius2, f1Angle, f1Angle);
            DrawPath(this, elArc.GetPath(), QVector<DirectionArrow>(), Qt::DashLine, Qt::RoundCap);

            DrawPoint(m_f1Point, elArc.GetP1());

            SetToolTip(tr("<b>Elliptical arc</b>: radius1 = %1%2, "
                          "radius2 = %3%2, angle1 = %4°; "
                          "<b>Mouse click</b> - finish selecting the first radius, "
                          "<b>%5</b> - sticking angle, "
                          "<b>%6</b> - skip")
                           .arg(LengthToUser(m_radius1),
                                prefix,
                                LengthToUser(m_radius2),
                                AngleToUser(f1Angle),
                                VModifierKey::Shift(),
                                VModifierKey::EnterKey()));
        }
        else if (qFuzzyIsNull(m_length))
        {
            DrawRadius1Line(center, m_radius1);
            DrawRadius2Line(center, m_radius2);

            qreal const f2Angle = Angle();
            VEllipticalArc const elArc = DrawElArc(*first, m_radius1, m_radius2, m_f1, f2Angle);
            DrawPath(this, elArc.GetPath(), QVector<DirectionArrow>(), Qt::DashLine, Qt::RoundCap);

            DrawPoint(m_f1Point, elArc.GetP1());

            SetToolTip(tr("<b>Elliptical arc</b>: radius1 = %1%2, "
                          "radius2 = %3%2, angle1 = %4°, arc length = %5%2; "
                          "<b>Mouse click</b> - finish selecting the arc length, "
                          "<b>%6</b> - sticking angle, "
                          "<b>%7</b> - skip")
                           .arg(LengthToUser(m_radius1),
                                prefix,
                                LengthToUser(m_radius2),
                                AngleToUser(m_f1),
                                LengthToUser(elArc.GetLength()),
                                VModifierKey::Shift(),
                                VModifierKey::EnterKey()));
        }
        else if (VFuzzyComparePossibleNulls(m_rotationAngle, INT_MAX))
        {
            if (VFuzzyComparePossibleNulls(m_startingRotationAngle, INT_MAX))
            {
                m_startingRotationAngle = radius.angle();
            }

            qreal const rotationAngle = Angle() - m_startingRotationAngle;

            DrawRadius1Line(center, m_radius1, rotationAngle);
            DrawRadius2Line(center, m_radius2, rotationAngle);
            VEllipticalArc const elArc = DrawElArc(m_length, *first, m_radius1, m_radius2, m_f1, rotationAngle);

            DrawPoint(m_f1Point, elArc.GetP1());

            SetToolTip(tr("<b>Elliptical arc</b>: radius1 = %1%2, "
                          "radius2 = %3%2, angle1 = %4°, arc length = %5%2, rotation = %6°; "
                          "<b>Mouse click</b> - finish creating, "
                          "<b>%7</b> - sticking angle, "
                          "<b>%8</b> - skip")
                           .arg(LengthToUser(m_radius1),
                                prefix,
                                LengthToUser(m_radius2),
                                AngleToUser(m_f1),
                                LengthToUser(elArc.GetLength()),
                                AngleToUser(rotationAngle),
                                VModifierKey::Shift(),
                                VModifierKey::EnterKey()));
        }
    }
    else
    {
        if (not qFuzzyIsNull(m_radius1) && not qFuzzyIsNull(m_radius2) && m_f1 >= 0 && not qFuzzyIsNull(m_length)
            && not VFuzzyComparePossibleNulls(m_rotationAngle, INT_MAX))
        {
            DrawElArc(m_length, *first, m_radius1, m_radius2, m_f1, m_rotationAngle);
        }
        else
        {
            DrawPath(this, QPainterPath(), QVector<DirectionArrow>(), LineStyle(), Qt::RoundCap);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArcWithLength::VisualMode(quint32 id)
{
    m_centerId = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArcWithLength::SetRadius1(const QString &expression)
{
    m_radius1 = FindLengthFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArcWithLength::SetRadius2(const QString &expression)
{
    m_radius2 = FindLengthFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArcWithLength::SetF1(const QString &expression)
{
    m_f1 = FindValFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArcWithLength::SetLength(const QString &expression)
{
    m_length = FindLengthFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArcWithLength::SetRotationAngle(const QString &expression)
{
    m_rotationAngle = FindValFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArcWithLength::DrawRadius1Line(const QPointF &center, qreal radius, qreal rotationAngle)
{
    QLineF radiusLine(center.x(), center.y(), center.x() + 100, center.y());
    radiusLine.setLength(radius);
    radiusLine.setAngle(radiusLine.angle() + rotationAngle);
    DrawLine(m_radius1Line, radiusLine);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArcWithLength::DrawRadius2Line(const QPointF &center, qreal radius, qreal rotationAngle)
{
    QLineF radiusLine(center.x(), center.y(), center.x(), center.y() - 100);
    radiusLine.setLength(radius);
    radiusLine.setAngle(radiusLine.angle() + rotationAngle);
    DrawLine(m_radius2Line, radiusLine);
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolEllipticalArcWithLength::DrawElArc(
    const VPointF &center, qreal radius1, qreal radius2, qreal f1, qreal f2, qreal rotationAngle) -> VEllipticalArc
{
    VEllipticalArc elArc(center, radius1, radius2, f1, f2, rotationAngle);
    elArc.SetApproximationScale(ApproximationScale());
    DrawPath(this, elArc.GetPath(), elArc.DirectionArrows(), LineStyle(), Qt::RoundCap);

    return elArc;
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolEllipticalArcWithLength::DrawElArc(
    qreal length, const VPointF &center, qreal radius1, qreal radius2, qreal f1, qreal rotationAngle) -> VEllipticalArc
{
    VEllipticalArc elArc(length, center, radius1, radius2, f1, rotationAngle);
    elArc.SetApproximationScale(ApproximationScale());
    DrawPath(this, elArc.GetPath(), elArc.DirectionArrows(), LineStyle(), Qt::RoundCap);

    return elArc;
}
