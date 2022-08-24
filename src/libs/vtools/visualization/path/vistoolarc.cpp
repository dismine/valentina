/************************************************************************
 **
 **  @file   vistoolarc.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 8, 2014
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

#include "vistoolarc.h"

#include <QPainterPath>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <new>
#include <QtMath>

#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "def.h"
#include "qnamespace.h"
#include "vgeometrydef.h"
#include "vispath.h"
#include "../vwidgets/scalesceneitems.h"
#include "../vmisc/vmodifierkey.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolArc::VisToolArc(const VContainer *data, QGraphicsItem *parent)
    :VisPath(data, parent)
{
    m_arcCenter = InitPoint(Color(VColor::MainColor), this);
    m_f1Point = InitPoint(Color(VColor::SupportColor), this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolArc::RefreshGeometry()
{
    if (m_centerId > NULL_ID)
    {
        m_f1Point->setVisible(false);

        const QSharedPointer<VPointF> first = GetData()->GeometricObject<VPointF>(m_centerId);
        DrawPoint(m_arcCenter, static_cast<QPointF>(*first), Color(VColor::SupportColor));

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
                VArc arc = VArc (*first, r.length(), r.angle(), r.angle());
                arc.SetApproximationScale(ApproximationScale());
                DrawPath(this, arc.GetPath(), QVector<DirectionArrow>(), Color(VColor::SupportColor), Qt::DashLine,
                         Qt::RoundCap);

                SetToolTip(tr("<b>Arc</b>: radius = %1%2; "
                              "<b>Mouse click</b> - finish selecting the radius, "
                              "<b>%3</b> - skip").arg(LengthToUser(r.length()), prefix, VModifierKey::EnterKey()));
            }
            else if (m_f1 < 0)
            {
                qreal f1Angle = Angle();
                VArc arc = VArc (*first, m_radius, f1Angle, f1Angle);
                arc.SetApproximationScale(ApproximationScale());
                DrawPath(this, arc.GetPath(), QVector<DirectionArrow>(), Color(VColor::SupportColor), Qt::DashLine,
                         Qt::RoundCap);

                QLineF f1Line = r;
                f1Line.setLength(m_radius);
                f1Line.setAngle(f1Angle);

                DrawPoint(m_f1Point, f1Line.p2(), Color(VColor::SupportColor));

                SetToolTip(tr("<b>Arc</b>: radius = %1%2, first angle = %3°; "
                              "<b>Mouse click</b> - finish selecting the first angle, "
                              "<b>%4</b> - sticking angle, "
                              "<b>%5</b> - skip")
                               .arg(LengthToUser(m_radius), prefix)
                               .arg(f1Angle)
                               .arg(VModifierKey::Shift(), VModifierKey::EnterKey()));
            }
            else if (m_f1 >= 0)
            {
                qreal f2Angle = StickyEnd(Angle());
                VArc arc = VArc (*first, m_radius, m_f1, f2Angle);
                arc.SetApproximationScale(ApproximationScale());
                DrawPath(this, arc.GetPath(), arc.DirectionArrows(), Color(VColor::MainColor), LineStyle(),
                         Qt::RoundCap);

                SetToolTip(tr("<b>Arc</b>: radius = %1%2, first angle = %3°, second angle = %4°; "
                              "<b>Mouse click</b> - finish creating, "
                              "<b>%5</b> - sticking angle, "
                              "<b>%6</b> - sticking end, "
                              "<b>%7</b> - skip")
                               .arg(LengthToUser(m_radius), prefix)
                               .arg(m_f1)
                               .arg(f2Angle)
                               .arg(VModifierKey::Shift(), VModifierKey::Control(), VModifierKey::EnterKey()));
            }
        }
        else
        {
            if (not qFuzzyIsNull(m_radius) && m_f1 >= 0 && m_f2 >= 0)
            {
                VArc arc = VArc (*first, m_radius, m_f1, m_f2);
                arc.SetApproximationScale(ApproximationScale());
                DrawPath(this, arc.GetPath(), arc.DirectionArrows(), Color(VColor::MainColor), LineStyle(),
                         Qt::RoundCap);
            }
            else
            {
                DrawPath(this, QPainterPath(), QVector<DirectionArrow>(), Color(VColor::MainColor), LineStyle(),
                         Qt::RoundCap);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolArc::SetRadius(const QString &expression)
{
    m_radius = FindLengthFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolArc::SetF1(const QString &expression)
{
    m_f1 = FindValFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolArc::SetF2(const QString &expression)
{
    m_f2 = FindValFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolArc::VisualMode(quint32 id)
{
    m_centerId = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolArc::StickyEnd(qreal angle) const -> qreal
{
    if (QGuiApplication::keyboardModifiers() == Qt::ControlModifier)
    {
        QLineF line(10, 10, 100, 10);
        line.setLength(m_radius);

        QLineF line2 = line;

        line.setAngle(m_f1);
        line2.setAngle(angle);

        if (VFuzzyComparePoints(line.p2(), line2.p2(), UnitConvertor(5, Unit::Mm, Unit::Px)))
        {
            return m_f1;
        }
    }

    return angle;
}
