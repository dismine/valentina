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
#include <new>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolnormal.h"
#include "../vgeometry/vpointf.h"
#include "../visualization.h"
#include "../vmisc/vmodifierkey.h"
#include "../vpatterndb/vcontainer.h"
#include "visline.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolNormal::VisToolNormal(const VContainer *data, QGraphicsItem *parent)
  : VisLine(data, parent)
{
    m_lineP1 = InitPoint(VColorRole::VisSupportColor, this);
    m_lineP2 = InitPoint(VColorRole::VisSupportColor, this); //-V656
    m_line = InitItem<VScaledLine>(VColorRole::VisSupportColor, this);

    m_point = InitPoint(VColorRole::VisMainColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolNormal::RefreshGeometry()
{
    if (m_object1Id > NULL_ID)
    {
        const QSharedPointer<VPointF> first = GetData()->GeometricObject<VPointF>(m_object1Id);
        DrawPoint(m_lineP1, static_cast<QPointF>(*first));

        if (m_object2Id <= NULL_ID)
        {
            QLineF line_mouse(static_cast<QPointF>(*first), ScenePos());
            DrawLine(m_line, line_mouse);

            QLineF normal = line_mouse.normalVector();
            QPointF endRay = Ray(normal.p1(), normal.angle());
            DrawLine(this, QLineF(normal.p1(), endRay));
        }
        else
        {
            const QSharedPointer<VPointF> second = GetData()->GeometricObject<VPointF>(m_object2Id);
            DrawPoint(m_lineP2, static_cast<QPointF>(*second));

            QLineF line_mouse(static_cast<QPointF>(*first), static_cast<QPointF>(*second));
            DrawLine(m_line, line_mouse);

            if (not qFuzzyIsNull(m_length))
            {
                QPointF fPoint = VToolNormal::FindPoint(static_cast<QPointF>(*first), static_cast<QPointF>(*second),
                                                        m_length, m_angle);
                QLineF mainLine = QLineF(static_cast<QPointF>(*first), fPoint);
                DrawLine(this, mainLine, LineStyle());

                DrawPoint(m_point, mainLine.p2());
            }
            else if (GetMode() == Mode::Creation)
            {
                QLineF cursorLine(static_cast<QPointF>(*first), ScenePos());
                QLineF normal = line_mouse.normalVector();

                qreal len = cursorLine.length();
                qreal angleTo = normal.angleTo(cursorLine);
                if (angleTo > 90 && angleTo < 270)
                {
                    len *= -1;
                }

                QPointF fPoint =
                    VToolNormal::FindPoint(static_cast<QPointF>(*first), static_cast<QPointF>(*second), len, m_angle);
                QLineF mainLine = QLineF(static_cast<QPointF>(*first), fPoint);
                DrawLine(this, mainLine, LineStyle());

                DrawPoint(m_point, mainLine.p2());

                const QString prefix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
                SetToolTip(tr("Length = %1%2; "
                              "<b>Mouse click</b> - finish selecting the length, "
                              "<b>%3</b> - skip")
                               .arg(LengthToUser(len), prefix, VModifierKey::EnterKey()));
            }
            else
            {
                QLineF normal = line_mouse.normalVector();
                QPointF endRay = Ray(normal.p1(), normal.angle());
                DrawLine(this, QLineF(normal.p1(), endRay));
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolNormal::VisualMode(quint32 id)
{
    m_object1Id = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolNormal::SetLength(const QString &expression)
{
    m_length = FindLengthFromUser(expression, GetData()->DataVariables());
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
