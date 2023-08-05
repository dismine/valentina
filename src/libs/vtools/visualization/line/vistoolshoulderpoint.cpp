/************************************************************************
 **
 **  @file   vistoolshoulderpoint.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 7, 2014
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

#include "vistoolshoulderpoint.h"

#include <QGraphicsLineItem>
#include <QLineF>
#include <QPointF>
#include <QSharedPointer>
#include <new>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolshoulderpoint.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../visualization.h"
#include "../vmisc/vmodifierkey.h"
#include "../vpatterndb/vcontainer.h"
#include "visline.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolShoulderPoint::VisToolShoulderPoint(const VContainer *data, QGraphicsItem *parent)
  : VisLine(data, parent)
{
    m_line1P1 = InitPoint(VColorRole::VisSupportColor, this);
    m_line1P2 = InitPoint(VColorRole::VisSupportColor, this); //-V656
    m_line1 = InitItem<VScaledLine>(VColorRole::VisSupportColor, this);

    m_line2P2 = InitPoint(VColorRole::VisSupportColor, this);
    m_line2 = InitItem<VScaledLine>(VColorRole::VisSupportColor, this);
    m_line3 = InitItem<VScaledLine>(VColorRole::VisSupportColor, this); //-V656

    m_point = InitPoint(VColorRole::VisMainColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolShoulderPoint::RefreshGeometry()
{
    if (m_point3Id > NULL_ID)
    {
        const QSharedPointer<VPointF> first = GetData()->GeometricObject<VPointF>(m_point3Id);
        DrawPoint(m_line1P1, static_cast<QPointF>(*first));

        if (m_lineP1Id <= NULL_ID)
        {
            DrawLine(m_line1, QLineF(static_cast<QPointF>(*first), ScenePos()));
        }
        else
        {
            const QSharedPointer<VPointF> second = GetData()->GeometricObject<VPointF>(m_lineP1Id);
            DrawPoint(m_line1P2, static_cast<QPointF>(*second));

            DrawLine(m_line1, QLineF(static_cast<QPointF>(*first), static_cast<QPointF>(*second)));

            if (m_lineP2Id <= NULL_ID)
            {
                DrawLine(m_line2, QLineF(static_cast<QPointF>(*second), ScenePos()));
            }
            else
            {
                const QSharedPointer<VPointF> third = GetData()->GeometricObject<VPointF>(m_lineP2Id);
                DrawPoint(m_line2P2, static_cast<QPointF>(*third));

                DrawLine(m_line2, QLineF(static_cast<QPointF>(*second), static_cast<QPointF>(*third)));

                if (not qFuzzyIsNull(m_length))
                {
                    QPointF fPoint =
                        VToolShoulderPoint::FindPoint(static_cast<QPointF>(*second), static_cast<QPointF>(*third),
                                                      static_cast<QPointF>(*first), m_length);
                    QLineF mainLine = QLineF(static_cast<QPointF>(*second), fPoint);
                    DrawLine(this, mainLine, LineStyle());

                    DrawPoint(m_point, mainLine.p2());
                    DrawLine(m_line3, QLineF(static_cast<QPointF>(*first), mainLine.p2()), Qt::DashLine);
                }
                else if (GetMode() == Mode::Creation)
                {
                    QLineF cursorLine(static_cast<QPointF>(*first), ScenePos());

                    qreal len = cursorLine.length();
                    QPointF fPoint = VToolShoulderPoint::FindPoint(
                        static_cast<QPointF>(*second), static_cast<QPointF>(*third), static_cast<QPointF>(*first), len);
                    QLineF mainLine = QLineF(static_cast<QPointF>(*second), fPoint);
                    DrawLine(this, mainLine, LineStyle());

                    DrawPoint(m_point, mainLine.p2());
                    DrawLine(m_line3, QLineF(static_cast<QPointF>(*first), mainLine.p2()), Qt::DashLine);

                    const QString prefix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
                    SetToolTip(tr("Length = %1%2; "
                                  "<b>Mouse click</b> - finish selecting the length, "
                                  "<b>%3</b> - skip")
                                   .arg(LengthToUser(len), prefix, VModifierKey::EnterKey()));
                }
                else
                {
                    qreal angle = QLineF(static_cast<QPointF>(*second), static_cast<QPointF>(*third)).angle();
                    QPointF endRay = Ray(static_cast<QPointF>(*second), angle);
                    QLineF mainLine = VGObject::BuildLine(
                        static_cast<QPointF>(*second), QLineF(static_cast<QPointF>(*second), endRay).length(), angle);
                    DrawLine(this, mainLine, LineStyle());
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolShoulderPoint::VisualMode(quint32 id)
{
    m_point3Id = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolShoulderPoint::SetLength(const QString &expression)
{
    m_length = FindLengthFromUser(expression, GetData()->DataVariables());
}
