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
#include <Qt>
#include <new>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersectioncircles.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../visualization.h"
#include "visline.h"
#include "../vmisc/vmodifierkey.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolPointOfIntersectionCircles::VisToolPointOfIntersectionCircles(const VContainer *data, QGraphicsItem *parent)
    : VisLine(data, parent)
{
    this->setPen(QPen(Qt::NoPen)); // don't use parent this time

    m_c1Path = InitItem<VScaledEllipse>(Qt::darkGreen, this);
    m_c1Path->SetPointMode(false);

    m_c2Path = InitItem<VScaledEllipse>(Qt::darkRed, this);
    m_c2Path->SetPointMode(false);

    m_point = InitPoint(mainColor, this);
    m_point->setZValue(1);

    m_c1Center = InitPoint(supportColor, this);
    m_c2Center = InitPoint(supportColor, this);  //-V656
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfIntersectionCircles::RefreshGeometry()
{
    if (object1Id > NULL_ID)
    {
        const QSharedPointer<VPointF> first = Visualization::data->GeometricObject<VPointF>(object1Id);
        DrawPoint(m_c1Center, static_cast<QPointF>(*first), supportColor);

        if (m_c1Radius > 0)
        {
            m_c1Path->setRect(PointRect(m_c1Radius));
            DrawPoint(m_c1Path, static_cast<QPointF>(*first), Qt::darkGreen, Qt::DashLine);

            if (m_object2Id > NULL_ID)
            {
                const QSharedPointer<VPointF> second = Visualization::data->GeometricObject<VPointF>(m_object2Id);
                DrawPoint(m_c2Center, static_cast<QPointF>(*second), supportColor);

                if (m_c2Radius > 0)
                {
                    m_c2Path->setRect(PointRect(m_c2Radius));
                    DrawPoint(m_c2Path, static_cast<QPointF>(*second), Qt::darkRed, Qt::DashLine);

                    QPointF fPoint;
                    VToolPointOfIntersectionCircles::FindPoint(static_cast<QPointF>(*first),
                                                               static_cast<QPointF>(*second),
                                                               m_c1Radius, m_c2Radius, m_crossPoint, &fPoint);
                    DrawPoint(m_point, fPoint, mainColor);
                }
                else if (mode == Mode::Creation)
                {
                    QLineF radiusLine (static_cast<QPointF>(*second), Visualization::scenePos);
                    const qreal length = radiusLine.length();

                    m_c2Path->setRect(PointRect(length));
                    DrawPoint(m_c2Path, static_cast<QPointF>(*second), Qt::darkRed, Qt::DashLine);

                    QPointF fPoint;
                    VToolPointOfIntersectionCircles::FindPoint(static_cast<QPointF>(*first),
                                                               static_cast<QPointF>(*second),
                                                               m_c1Radius, length, m_crossPoint, &fPoint);
                    DrawPoint(m_point, fPoint, mainColor);

                    const QString prefix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
                    Visualization::toolTip = tr("Radius = %1%2; "
                                                "<b>Mouse click</b> - finish selecting the second radius, "
                                                "<b>%3</b> - skip")
                                                 .arg(NumberToUser(length), prefix, VModifierKey::EnterKey());
                }
            }
        }
        else if (mode == Mode::Creation)
        {
            QLineF radiusLine (static_cast<QPointF>(*first), Visualization::scenePos);
            const qreal length = radiusLine.length();

            m_c1Path->setRect(PointRect(length));
            DrawPoint(m_c1Path, static_cast<QPointF>(*first), Qt::darkGreen, Qt::DashLine);

            const QString prefix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
            Visualization::toolTip = tr("Radius = %1%2; "
                                        "<b>Mouse click</b> - finish selecting the first radius, "
                                        "<b>%3</b> - skip")
                                         .arg(NumberToUser(length), prefix, VModifierKey::EnterKey());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfIntersectionCircles::VisualMode(const quint32 &id)
{
    auto *scene = qobject_cast<VMainGraphicsScene *>(VAbstractValApplication::VApp()->getCurrentScene());
    SCASSERT(scene != nullptr)

    this->object1Id = id;
    Visualization::scenePos = scene->getScenePos();
    RefreshGeometry();

    AddOnScene();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfIntersectionCircles::setObject2Id(const quint32 &value)
{
    m_object2Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfIntersectionCircles::setC1Radius(const QString &value)
{
    m_c1Radius = FindLengthFromUser(value, Visualization::data->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfIntersectionCircles::setC2Radius(const QString &value)
{
    m_c2Radius = FindLengthFromUser(value, Visualization::data->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfIntersectionCircles::setCrossPoint(const CrossCirclesPoint &value)
{
    m_crossPoint = value;
}
