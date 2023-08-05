/************************************************************************
 **
 **  @file   vistoolpointofcontact.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 8, 2014
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

#include "vistoolpointofcontact.h"

#include <QGraphicsEllipseItem>
#include <QLineF>
#include <QPointF>
#include <QSharedPointer>
#include <new>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofcontact.h"
#include "../vgeometry/vpointf.h"
#include "../visualization.h"
#include "../vmisc/vmodifierkey.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/global.h"
#include "visline.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolPointOfContact::VisToolPointOfContact(const VContainer *data, QGraphicsItem *parent)
  : VisLine(data, parent)
{
    SetColorRole(VColorRole::VisSupportColor);

    m_arcPoint = InitPoint(VColorRole::VisSupportColor, this);
    m_lineP1 = InitPoint(VColorRole::VisSupportColor, this);
    m_lineP2 = InitPoint(VColorRole::VisSupportColor, this);
    m_circle = InitItem<VScaledEllipse>(VColorRole::VisSupportColor, this);
    m_circle->SetPointMode(false);

    m_point = InitPoint(VColorRole::VisMainColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfContact::RefreshGeometry()
{
    if (m_lineP1Id > NULL_ID)
    {
        const QSharedPointer<VPointF> first = GetData()->GeometricObject<VPointF>(m_lineP1Id);
        DrawPoint(m_lineP1, static_cast<QPointF>(*first));

        if (m_lineP2Id <= NULL_ID)
        {
            DrawLine(this, QLineF(static_cast<QPointF>(*first), ScenePos()));
        }
        else
        {
            const QSharedPointer<VPointF> second = GetData()->GeometricObject<VPointF>(m_lineP2Id);
            DrawPoint(m_lineP2, static_cast<QPointF>(*second));
            DrawLine(this, QLineF(static_cast<QPointF>(*first), static_cast<QPointF>(*second)));

            if (m_radiusId > NULL_ID)
            {
                const QSharedPointer<VPointF> third = GetData()->GeometricObject<VPointF>(m_radiusId);
                DrawPoint(m_arcPoint, static_cast<QPointF>(*third));

                if (not qFuzzyIsNull(m_radius))
                {
                    QPointF fPoint;
                    VToolPointOfContact::FindPoint(m_radius, static_cast<QPointF>(*third), static_cast<QPointF>(*first),
                                                   static_cast<QPointF>(*second), &fPoint);
                    DrawPoint(m_point, fPoint);

                    m_circle->setRect(PointRect(m_radius));
                    DrawPoint(m_circle, static_cast<QPointF>(*third), Qt::DashLine);
                }
                else if (GetMode() == Mode::Creation)
                {
                    QLineF cursorLine(static_cast<QPointF>(*third), ScenePos());
                    qreal radius = cursorLine.length();

                    QPointF fPoint;
                    VToolPointOfContact::FindPoint(radius, static_cast<QPointF>(*third), static_cast<QPointF>(*first),
                                                   static_cast<QPointF>(*second), &fPoint);
                    DrawPoint(m_point, fPoint);

                    m_circle->setRect(PointRect(radius));
                    DrawPoint(m_circle, static_cast<QPointF>(*third), Qt::DashLine);

                    const QString prefix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
                    SetToolTip(tr("Radius = %1%2; "
                                  "<b>Mouse click</b> - finish selecting the radius, "
                                  "<b>%3</b> - skip")
                                   .arg(LengthToUser(radius), prefix, VModifierKey::EnterKey()));
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfContact::VisualMode(quint32 id)
{
    m_lineP1Id = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfContact::SetRadius(const QString &expression)
{
    m_radius = FindLengthFromUser(expression, GetData()->DataVariables());
}
