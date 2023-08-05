/************************************************************************
 **
 **  @file   vistoolarcwithlength.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 6, 2015
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

#include "vistoolarcwithlength.h"

#include <QPainterPath>
#include <QPointF>
#include <QSharedPointer>
#include <QtMath>
#include <new>

#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vpointf.h"
#include "../visualization.h"
#include "../vmisc/vmodifierkey.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/scalesceneitems.h"
#include "vispath.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolArcWithLength::VisToolArcWithLength(const VContainer *data, QGraphicsItem *parent)
  : VisPath(data, parent)
{
    m_arcCenter = InitPoint(VColorRole::VisSupportColor, this);
    m_f1Point = InitPoint(VColorRole::VisSupportColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolArcWithLength::RefreshGeometry()
{
    if (m_centerId > NULL_ID)
    {
        m_f1Point->setVisible(false);

        const QSharedPointer<VPointF> first = GetData()->GeometricObject<VPointF>(m_centerId);
        DrawPoint(m_arcCenter, static_cast<QPointF>(*first));

        if (GetMode() == Mode::Creation)
        {
            QLineF r = QLineF(static_cast<QPointF>(*first), ScenePos());

            auto Angle = [r]()
            {
                if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
                {
                    QLineF correction = r;
                    correction.setAngle(CorrectAngle(correction.angle()));
                    return correction.angle();
                }

                return r.angle();
            };

            static const QString prefix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);

            if (qFuzzyIsNull(m_radius))
            {
                VArc arc = VArc(*first, r.length(), r.angle(), r.angle());
                arc.SetApproximationScale(ApproximationScale());
                DrawPath(this, arc.GetPath(), QVector<DirectionArrow>(), Qt::DashLine, Qt::RoundCap);

                SetToolTip(tr("<b>Arc</b>: radius = %1%2; "
                              "<b>Mouse click</b> - finish selecting the radius, "
                              "<b>%3</b> - skip")
                               .arg(LengthToUser(r.length()), prefix, VModifierKey::EnterKey()));
            }
            else if (m_f1 < 0)
            {
                qreal f1Angle = Angle();
                VArc arc = VArc(*first, m_radius, f1Angle, f1Angle);
                arc.SetApproximationScale(ApproximationScale());
                DrawPath(this, arc.GetPath(), QVector<DirectionArrow>(), Qt::DashLine, Qt::RoundCap);

                QLineF f1Line = r;
                f1Line.setLength(m_radius);
                f1Line.setAngle(f1Angle);

                DrawPoint(m_f1Point, f1Line.p2());

                SetToolTip(tr("<b>Arc</b>: radius = %1%2, first angle = %3°; "
                              "<b>Mouse click</b> - finish selecting the first angle, "
                              "<b>%4</b> - sticking angle, "
                              "<b>%5</b> - skip")
                               .arg(LengthToUser(m_radius), prefix, AngleToUser(f1Angle), VModifierKey::Shift(),
                                    VModifierKey::EnterKey()));
            }
            else if (m_f1 >= 0)
            {
                VArc arc = VArc(*first, m_radius, m_f1, r.angle());
                arc.SetApproximationScale(ApproximationScale());
                DrawPath(this, arc.GetPath(), arc.DirectionArrows(), LineStyle(), Qt::RoundCap);

                SetToolTip(tr("<b>Arc</b>: radius = %1%2, first angle = %3°, arc length = %4%2; "
                              "<b>Mouse click</b> - finish creating, "
                              "<b>%5</b> - skip")
                               .arg(LengthToUser(m_radius), prefix, AngleToUser(m_f1), LengthToUser(arc.GetLength()),
                                    VModifierKey::EnterKey()));
            }
        }
        else
        {
            if (not qFuzzyIsNull(m_radius) && m_f1 >= 0 && not qFuzzyIsNull(m_length))
            {
                VArc arc = VArc(m_length, *first, m_radius, m_f1);
                arc.SetApproximationScale(ApproximationScale());
                DrawPath(this, arc.GetPath(), arc.DirectionArrows(), LineStyle(), Qt::RoundCap);
            }
            else
            {
                DrawPath(this, QPainterPath(), QVector<DirectionArrow>(), LineStyle(), Qt::RoundCap);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolArcWithLength::VisualMode(quint32 id)
{
    m_centerId = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolArcWithLength::SetRadius(const QString &expression)
{
    m_radius = FindLengthFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolArcWithLength::SetF1(const QString &expression)
{
    m_f1 = FindValFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolArcWithLength::SetLength(const QString &expression)
{
    m_length = FindLengthFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolArcWithLength::CorrectAngle(qreal angle) -> qreal
{
    qreal ang = angle;
    if (angle > 360)
    {
        ang = angle - 360.0 * qFloor(angle / 360);
    }

    return (qFloor(qAbs(ang) / 5.)) * 5;
}
