/************************************************************************
 **
 **  @file   vistoolnormal.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 8, 2014
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

#include "vistoolnormal.h"

#include <QColor>
#include <QGraphicsLineItem>
#include <QLineF>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <new>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolnormal.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "visline.h"
#include "../vmisc/vmodifierkey.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolNormal::VisToolNormal(const VContainer *data, QGraphicsItem *parent)
    : VisLine(data, parent)
{
    this->mainColor = Qt::red;

    m_lineP1 = InitPoint(supportColor, this);
    m_lineP2 = InitPoint(supportColor, this); //-V656
    m_line = InitItem<VScaledLine>(supportColor, this);

    m_point = InitPoint(mainColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolNormal::RefreshGeometry()
{
    if (object1Id > NULL_ID)
    {
        const QSharedPointer<VPointF> first = Visualization::data->GeometricObject<VPointF>(object1Id);
        DrawPoint(m_lineP1, static_cast<QPointF>(*first), supportColor);

        if (m_object2Id <= NULL_ID)
        {
            QLineF line_mouse(static_cast<QPointF>(*first), Visualization::scenePos);
            DrawLine(m_line, line_mouse, supportColor);

            QLineF normal = line_mouse.normalVector();
            QPointF endRay = Ray(normal.p1(), normal.angle());
            DrawLine(this, QLineF(normal.p1(), endRay), mainColor);
        }
        else
        {
            const QSharedPointer<VPointF> second = Visualization::data->GeometricObject<VPointF>(m_object2Id);
            DrawPoint(m_lineP2, static_cast<QPointF>(*second), supportColor);

            QLineF line_mouse(static_cast<QPointF>(*first), static_cast<QPointF>(*second));
            DrawLine(m_line, line_mouse, supportColor);

            if (not qFuzzyIsNull(m_length))
            {
                QPointF fPoint = VToolNormal::FindPoint(static_cast<QPointF>(*first), static_cast<QPointF>(*second),
                                                        m_length, m_angle);
                QLineF mainLine = QLineF(static_cast<QPointF>(*first), fPoint);
                DrawLine(this, mainLine, mainColor, lineStyle);

                DrawPoint(m_point, mainLine.p2(), mainColor);
            }
            else if (mode == Mode::Creation)
            {
                QLineF cursorLine (static_cast<QPointF>(*first), Visualization::scenePos);
                QLineF normal = line_mouse.normalVector();

                qreal len = cursorLine.length();
                qreal angleTo = normal.angleTo(cursorLine);
                if (angleTo > 90 && angleTo < 270)
                {
                    len *= -1;
                }

                QPointF fPoint = VToolNormal::FindPoint(static_cast<QPointF>(*first), static_cast<QPointF>(*second),
                                                        len, m_angle);
                QLineF mainLine = QLineF(static_cast<QPointF>(*first), fPoint);
                DrawLine(this, mainLine, mainColor, lineStyle);

                DrawPoint(m_point, mainLine.p2(), mainColor);

                const QString prefix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
                Visualization::toolTip = tr("Length = %1%2; "
                                            "<b>Mouse click</b> - finish selecting the length, "
                                            "<b>%3</b> - skip")
                                             .arg(NumberToUser(len), prefix, VModifierKey::EnterKey());
            }
            else
            {
                QLineF normal = line_mouse.normalVector();
                QPointF endRay = Ray(normal.p1(), normal.angle());
                DrawLine(this, QLineF(normal.p1(), endRay), mainColor);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolNormal::setObject2Id(const quint32 &value)
{
    m_object2Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolNormal::setLength(const QString &expression)
{
    m_length = FindLengthFromUser(expression, Visualization::data->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolNormal::GetAngle() const -> qreal
{
    return m_angle;
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolNormal::SetAngle(const qreal &value)
{
    m_angle = value;
}
