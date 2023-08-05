/************************************************************************
 **
 **  @file   vistoolpointofintersectioncircles.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 5, 2015
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

#include "vistoolpointofintersectioncircles.h"

#include <QGraphicsEllipseItem>
#include <QPen>
#include <QPointF>
#include <QSharedPointer>
#include <new>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersectioncircles.h"
#include "../vgeometry/vpointf.h"
#include "../visualization.h"
#include "../vmisc/vmodifierkey.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/global.h"
#include "visline.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolPointOfIntersectionCircles::VisToolPointOfIntersectionCircles(const VContainer *data, QGraphicsItem *parent)
  : VisLine(data, parent)
{
    this->setPen(QPen(Qt::NoPen)); // don't use parent this time

    m_c1Path = InitItem<VScaledEllipse>(VColorRole::VisSupportColor2, this);
    m_c1Path->SetPointMode(false);

    m_c2Path = InitItem<VScaledEllipse>(VColorRole::VisSupportColor4, this);
    m_c2Path->SetPointMode(false);

    m_point = InitPoint(VColorRole::VisMainColor, this);
    m_point->setZValue(1);

    m_c1Center = InitPoint(VColorRole::VisSupportColor, this);
    m_c2Center = InitPoint(VColorRole::VisSupportColor, this); //-V656
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfIntersectionCircles::RefreshGeometry()
{
    if (m_circle1Id > NULL_ID)
    {
        const QSharedPointer<VPointF> first = GetData()->GeometricObject<VPointF>(m_circle1Id);
        DrawPoint(m_c1Center, static_cast<QPointF>(*first));

        if (m_c1Radius > 0)
        {
            m_c1Path->setRect(PointRect(m_c1Radius));
            DrawPoint(m_c1Path, static_cast<QPointF>(*first), Qt::DashLine);

            if (m_circle2Id > NULL_ID)
            {
                const QSharedPointer<VPointF> second = GetData()->GeometricObject<VPointF>(m_circle2Id);
                DrawPoint(m_c2Center, static_cast<QPointF>(*second));

                if (m_c2Radius > 0)
                {
                    m_c2Path->setRect(PointRect(m_c2Radius));
                    DrawPoint(m_c2Path, static_cast<QPointF>(*second), Qt::DashLine);

                    QPointF fPoint;
                    VToolPointOfIntersectionCircles::FindPoint(static_cast<QPointF>(*first),
                                                               static_cast<QPointF>(*second), m_c1Radius, m_c2Radius,
                                                               m_crossPoint, &fPoint);
                    DrawPoint(m_point, fPoint);
                }
                else if (GetMode() == Mode::Creation)
                {
                    QLineF radiusLine(static_cast<QPointF>(*second), ScenePos());
                    const qreal length = radiusLine.length();

                    m_c2Path->setRect(PointRect(length));
                    DrawPoint(m_c2Path, static_cast<QPointF>(*second), Qt::DashLine);

                    QPointF fPoint;
                    VToolPointOfIntersectionCircles::FindPoint(static_cast<QPointF>(*first),
                                                               static_cast<QPointF>(*second), m_c1Radius, length,
                                                               m_crossPoint, &fPoint);
                    DrawPoint(m_point, fPoint);

                    const QString prefix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
                    SetToolTip(tr("Radius = %1%2; "
                                  "<b>Mouse click</b> - finish selecting the second radius, "
                                  "<b>%3</b> - skip")
                                   .arg(LengthToUser(length), prefix, VModifierKey::EnterKey()));
                }
            }
        }
        else if (GetMode() == Mode::Creation && VAbstractValApplication::VApp()->Settings()->IsInteractiveTools())
        {
            QLineF radiusLine(static_cast<QPointF>(*first), ScenePos());
            const qreal length = radiusLine.length();

            m_c1Path->setRect(PointRect(length));
            DrawPoint(m_c1Path, static_cast<QPointF>(*first), Qt::DashLine);

            const QString prefix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
            SetToolTip(tr("Radius = %1%2; "
                          "<b>Mouse click</b> - finish selecting the first radius, "
                          "<b>%3</b> - skip")
                           .arg(LengthToUser(length), prefix, VModifierKey::EnterKey()));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfIntersectionCircles::VisualMode(quint32 id)
{
    m_circle1Id = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfIntersectionCircles::SetC1Radius(const QString &value)
{
    m_c1Radius = FindLengthFromUser(value, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfIntersectionCircles::SetC2Radius(const QString &value)
{
    m_c2Radius = FindLengthFromUser(value, GetData()->DataVariables());
}
