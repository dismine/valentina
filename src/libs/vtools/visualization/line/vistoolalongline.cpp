/************************************************************************
 **
 **  @file   vistoolalongline.cpp
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

#include "vistoolalongline.h"

#include <QColor>
#include <QGraphicsLineItem>
#include <QLineF>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <new>

#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "visline.h"
#include "../vmisc/vmodifierkey.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolAlongLine::VisToolAlongLine(const VContainer *data, QGraphicsItem *parent)
    : VisLine(data, parent)
{
    SetMainColor(Qt::red);
    this->setZValue(2);// Show on top real tool

    m_lineP1 = InitPoint(Color(VColor::SupportColor), this);
    m_lineP2 = InitPoint(Color(VColor::SupportColor), this); //-V656
    m_line = InitItem<VScaledLine>(Color(VColor::SupportColor), this);
    m_point = InitPoint(Color(VColor::MainColor), this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolAlongLine::SetLength(const QString &expression)
{
    m_length = FindLengthFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolAlongLine::RefreshGeometry()
{
    if (m_point1Id > NULL_ID)
    {
        const QSharedPointer<VPointF> first = GetData()->GeometricObject<VPointF>(m_point1Id);
        DrawPoint(m_lineP1, static_cast<QPointF>(*first), Color(VColor::SupportColor));

        if (m_point2Id <= NULL_ID)
        {
            QLineF cursorLine (static_cast<QPointF>(*first), ScenePos());
            DrawLine(m_line, cursorLine, Color(VColor::SupportColor));

            if (m_midPointMode)
            {
                cursorLine.setLength(cursorLine.length()/2.0);
                DrawPoint(m_point,  cursorLine.p2(), Color(VColor::MainColor));
            }
        }
        else
        {
            const QSharedPointer<VPointF> second = GetData()->GeometricObject<VPointF>(m_point2Id);
            DrawPoint(m_lineP2, static_cast<QPointF>(*second), Color(VColor::SupportColor));

            QLineF baseLine(static_cast<QPointF>(*first), static_cast<QPointF>(*second));
            DrawLine(m_line, baseLine, Color(VColor::SupportColor));

            if (not qFuzzyIsNull(m_length))
            {
                QLineF mainLine = VGObject::BuildLine(static_cast<QPointF>(*first), m_length, m_line->line().angle());
                DrawLine(this, mainLine, Color(VColor::MainColor), LineStyle());

                DrawPoint(m_point, mainLine.p2(), Color(VColor::MainColor));
            }
            else if (GetMode() == Mode::Creation)
            {
                QLineF cursorLine (static_cast<QPointF>(*first), ScenePos());

                qreal len = cursorLine.length();
                qreal angleTo = baseLine.angleTo(cursorLine);
                if (angleTo > 90 && angleTo < 270)
                {
                    len *= -1;
                }

                QLineF mainLine = VGObject::BuildLine(static_cast<QPointF>(*first), len, m_line->line().angle());
                DrawLine(this, mainLine, Color(VColor::MainColor), LineStyle());

                DrawPoint(m_point, mainLine.p2(), Color(VColor::MainColor));

                const QString prefix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
                SetToolTip(tr("Length = %1%2; "
                              "<b>Mouse click</b> - finish selecting the length, "
                              "<b>%3</b> - skip")
                               .arg(NumberToUser(len), prefix, VModifierKey::EnterKey()));
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolAlongLine::VisualMode(quint32 id)
{
    m_point1Id = id;
    StartVisualMode();
}
