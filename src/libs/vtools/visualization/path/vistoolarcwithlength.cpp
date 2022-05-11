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
#include <Qt>
#include <new>

#include "../ifc/ifcdef.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "../vwidgets/scalesceneitems.h"
#include "vispath.h"
#include "../vmisc/vmodifierkey.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolArcWithLength::VisToolArcWithLength(const VContainer *data, QGraphicsItem *parent)
    :VisPath(data, parent)
{
    arcCenter = InitPoint(mainColor, this);
    f1Point = InitPoint(supportColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolArcWithLength::RefreshGeometry()
{
    if (object1Id > NULL_ID)
    {
        f1Point->setVisible(false);

        const QSharedPointer<VPointF> first = Visualization::data->GeometricObject<VPointF>(object1Id);
        DrawPoint(arcCenter, static_cast<QPointF>(*first), supportColor);

        if (mode == Mode::Creation)
        {
            QLineF r = QLineF(static_cast<QPointF>(*first), Visualization::scenePos);

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

            auto NumberToUser = [](qreal value)
            {
                return VAbstractApplication::VApp()->TrVars()
                        ->FormulaToUser(QString::number(VAbstractValApplication::VApp()->fromPixel(value)),
                                        VAbstractApplication::VApp()->Settings()->GetOsSeparator());
            };

            static const QString prefix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);

            if (qFuzzyIsNull(radius))
            {
                VArc arc = VArc (*first, r.length(), r.angle(), r.angle());
                arc.SetApproximationScale(m_approximationScale);
                DrawPath(this, arc.GetPath(), QVector<DirectionArrow>(), supportColor, Qt::DashLine, Qt::RoundCap);

                Visualization::toolTip = tr("<b>Arc</b>: radius = %1%2; "
                                            "<b>Mouse click</b> - finish selecting the radius, "
                                            "<b>%3</b> - skip")
                        .arg(NumberToUser(r.length()), prefix, VModifierKey::EnterKey());
            }
            else if (f1 < 0)
            {
                qreal f1Angle = Angle();
                VArc arc = VArc (*first, radius, f1Angle, f1Angle);
                arc.SetApproximationScale(m_approximationScale);
                DrawPath(this, arc.GetPath(), QVector<DirectionArrow>(), supportColor, Qt::DashLine, Qt::RoundCap);

                QLineF f1Line = r;
                f1Line.setLength(radius);
                f1Line.setAngle(f1Angle);

                DrawPoint(f1Point, f1Line.p2(), supportColor);

                Visualization::toolTip = tr("<b>Arc</b>: radius = %1%2, first angle = %3°; "
                                            "<b>Mouse click</b> - finish selecting the first angle, "
                                            "<b>%4</b> - sticking angle, "
                                            "<b>%5</b> - skip")
                        .arg(NumberToUser(radius), prefix)
                        .arg(f1Angle)
                        .arg(VModifierKey::Shift(), VModifierKey::EnterKey());
            }
            else if (f1 >= 0)
            {
                VArc arc = VArc (*first, radius, f1, r.angle());
                arc.SetApproximationScale(m_approximationScale);
                DrawPath(this, arc.GetPath(), arc.DirectionArrows(), mainColor, lineStyle, Qt::RoundCap);

                Visualization::toolTip = tr("<b>Arc</b>: radius = %1%2, first angle = %3°, arc length = %4%2; "
                                            "<b>Mouse click</b> - finish creating, "
                                            "<b>%5</b> - skip")
                        .arg(NumberToUser(radius), prefix)
                        .arg(f1)
                        .arg(NumberToUser(arc.GetLength()), VModifierKey::EnterKey());
            }
        }
        else
        {
            if (not qFuzzyIsNull(radius) && f1 >= 0 && not qFuzzyIsNull(length))
            {
                VArc arc = VArc (length, *first, radius, f1);
                arc.SetApproximationScale(m_approximationScale);
                DrawPath(this, arc.GetPath(), arc.DirectionArrows(), mainColor, lineStyle, Qt::RoundCap);
            }
            else
            {
                DrawPath(this, QPainterPath(), QVector<DirectionArrow>(), mainColor, lineStyle, Qt::RoundCap);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolArcWithLength::setRadius(const QString &expression)
{
    radius = FindLengthFromUser(expression, Visualization::data->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolArcWithLength::setF1(const QString &expression)
{
    f1 = FindValFromUser(expression, Visualization::data->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolArcWithLength::setLength(const QString &expression)
{
    length = FindLengthFromUser(expression, Visualization::data->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolArcWithLength::CorrectAngle(qreal angle) -> qreal
{
    qreal ang = angle;
    if (angle > 360)
    {
        ang = angle - 360.0 * qFloor(angle/360);
    }

    return (qFloor(qAbs(ang)/5.)) * 5;
}
