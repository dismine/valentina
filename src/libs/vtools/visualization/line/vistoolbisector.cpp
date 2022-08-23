/************************************************************************
 **
 **  @file   vistoolbisector.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   24 7, 2014
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

#include "vistoolbisector.h"

#include <QGraphicsLineItem>
#include <QLineF>
#include <QPointF>
#include <QSharedPointer>
#include <new>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolbisector.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "visline.h"
#include "../vmisc/vmodifierkey.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolBisector::VisToolBisector(const VContainer *data, QGraphicsItem *parent)
    :VisLine(data, parent)
{
    m_line1P1 = InitPoint(Color(VColor::SupportColor), this);
    m_line1P2 = InitPoint(Color(VColor::SupportColor), this); //-V656
    m_line1 = InitItem<VScaledLine>(Color(VColor::SupportColor), this);

    m_line2P2 = InitPoint(Color(VColor::SupportColor), this);
    m_line2 = InitItem<VScaledLine>(Color(VColor::SupportColor), this);

    m_point = InitPoint(Color(VColor::MainColor), this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolBisector::SetLength(const QString &expression)
{
    m_length = FindLengthFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolBisector::RefreshGeometry()
{
    if (m_point1Id > NULL_ID)
    {
        const QSharedPointer<VPointF> first = GetData()->GeometricObject<VPointF>(m_point1Id);
        DrawPoint(m_line1P1, static_cast<QPointF>(*first), Color(VColor::SupportColor));

        if (m_point2Id <= NULL_ID)
        {
            DrawLine(m_line1, QLineF(static_cast<QPointF>(*first), ScenePos()), Color(VColor::SupportColor));
        }
        else
        {
            const QSharedPointer<VPointF> second = GetData()->GeometricObject<VPointF>(m_point2Id);
            DrawPoint(m_line1P2, static_cast<QPointF>(*second), Color(VColor::SupportColor));

            DrawLine(m_line1, QLineF(static_cast<QPointF>(*first), static_cast<QPointF>(*second)),
                     Color(VColor::SupportColor));

            if (m_point3Id <= NULL_ID)
            {
                DrawLine(m_line2, QLineF(static_cast<QPointF>(*second), ScenePos()), Color(VColor::SupportColor));
            }
            else
            {
                const QSharedPointer<VPointF> third = GetData()->GeometricObject<VPointF>(m_point3Id);
                DrawPoint(m_line2P2, static_cast<QPointF>(*third), Color(VColor::SupportColor));

                DrawLine(m_line2, QLineF(static_cast<QPointF>(*second), static_cast<QPointF>(*third)),
                         Color(VColor::SupportColor));

                if (not qFuzzyIsNull(m_length))
                {
                    qreal angle = VToolBisector::BisectorAngle(static_cast<QPointF>(*first),
                                                               static_cast<QPointF>(*second),
                                                               static_cast<QPointF>(*third));
                    QLineF mainLine = VGObject::BuildLine(static_cast<QPointF>(*second), m_length, angle);
                    DrawLine(this, mainLine, Color(VColor::MainColor), LineStyle());

                    DrawPoint(m_point, mainLine.p2(), Color(VColor::MainColor));
                }
                else if (GetMode() == Mode::Creation)
                {
                    QLineF cursorLine (static_cast<QPointF>(*second), ScenePos());
                    QLineF baseLine(static_cast<QPointF>(*second), static_cast<QPointF>(*third));

                    qreal angle = VToolBisector::BisectorAngle(static_cast<QPointF>(*first),
                                                               static_cast<QPointF>(*second),
                                                               static_cast<QPointF>(*third));

                    baseLine.setAngle(angle);

                    qreal len = cursorLine.length();
                    qreal angleTo = baseLine.angleTo(cursorLine);
                    if (angleTo > 90 && angleTo < 270)
                    {
                        len *= -1;
                    }

                    QLineF mainLine = VGObject::BuildLine(static_cast<QPointF>(*second), len, angle);

                    DrawLine(this, mainLine, Color(VColor::MainColor), LineStyle());

                    DrawPoint(m_point, mainLine.p2(), Color(VColor::MainColor));

                    const QString prefix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
                    SetToolTip(tr("Length = %1%2; "
                                  "<b>Mouse click</b> - finish selecting the length, "
                                  "<b>%3</b> - skip")
                                   .arg(NumberToUser(len), prefix, VModifierKey::EnterKey()));
                }
                else
                {
                    qreal angle = VToolBisector::BisectorAngle(static_cast<QPointF>(*first),
                                                               static_cast<QPointF>(*second),
                                                               static_cast<QPointF>(*third));
                    QPointF endRay = Ray(static_cast<QPointF>(*second), angle);
                    QLineF mainLine = VGObject::BuildLine(static_cast<QPointF>(*second),
                                                          QLineF(static_cast<QPointF>(*second), endRay).length(),
                                                          angle);
                    DrawLine(this, mainLine, Color(VColor::MainColor), LineStyle());
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolBisector::VisualMode(quint32 id)
{
    m_point1Id = id;
    StartVisualMode();
}
