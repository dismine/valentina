/************************************************************************
 **
 **  @file   vistooltruedarts.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 6, 2015
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

#include "vistooltruedarts.h"

#include <QGraphicsLineItem>
#include <QLineF>
#include <QPointF>
#include <QSharedPointer>
#include <new>

#include "../../tools/drawTools/toolpoint/tooldoublepoint/vtooltruedarts.h"
#include "../vgeometry/vpointf.h"
#include "../visualization.h"
#include "../vpatterndb/vcontainer.h"
#include "visline.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolTrueDarts::VisToolTrueDarts(const VContainer *data, QGraphicsItem *parent)
  : VisLine(data, parent)
{
    SetColorRole(VColorRole::VisSupportColor);

    m_baseLineP1 = InitPoint(VColorRole::VisSupportColor, this);
    m_baseLineP2 = InitPoint(VColorRole::VisSupportColor, this);
    m_dartP1 = InitPoint(VColorRole::VisSupportColor, this);
    m_dartP2 = InitPoint(VColorRole::VisSupportColor, this);
    m_dartP3 = InitPoint(VColorRole::VisSupportColor, this);

    m_lineblP1P1 = InitItem<VScaledLine>(VColorRole::VisSupportColor, this);
    m_lineblP2P2 = InitItem<VScaledLine>(VColorRole::VisSupportColor, this);
    m_p1d2 = InitItem<VScaledLine>(VColorRole::VisSupportColor, this);
    m_d2p2 = InitItem<VScaledLine>(VColorRole::VisSupportColor, this);

    m_point1 = InitPoint(VColorRole::VisMainColor, this);
    m_point2 = InitPoint(VColorRole::VisMainColor, this); //-V656
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolTrueDarts::RefreshGeometry()
{
    if (m_baseLineP1Id > NULL_ID)
    {
        const QSharedPointer<VPointF> blP1 = GetData()->GeometricObject<VPointF>(m_baseLineP1Id);
        DrawPoint(m_baseLineP1, static_cast<QPointF>(*blP1));

        if (m_baseLineP2Id <= NULL_ID)
        {
            DrawLine(this, QLineF(static_cast<QPointF>(*blP1), ScenePos()), Qt::DashLine);
        }
        else
        {
            const QSharedPointer<VPointF> blP2 = GetData()->GeometricObject<VPointF>(m_baseLineP2Id);
            DrawPoint(m_baseLineP2, static_cast<QPointF>(*blP2));
            DrawLine(this, QLineF(static_cast<QPointF>(*blP1), static_cast<QPointF>(*blP2)), Qt::DashLine);

            if (m_dartP1Id > NULL_ID)
            {
                const QSharedPointer<VPointF> d1 = GetData()->GeometricObject<VPointF>(m_dartP1Id);
                DrawPoint(m_dartP1, static_cast<QPointF>(*d1));

                if (m_dartP2Id <= NULL_ID)
                {
                    DrawLine(m_p1d2, QLineF(static_cast<QPointF>(*d1), ScenePos()));
                }
                else
                {
                    const QSharedPointer<VPointF> d2 = GetData()->GeometricObject<VPointF>(m_dartP2Id);
                    DrawPoint(m_dartP2, static_cast<QPointF>(*d2));
                    DrawLine(m_p1d2, QLineF(static_cast<QPointF>(*d1), static_cast<QPointF>(*d2)));

                    if (m_dartP3Id <= NULL_ID)
                    {
                        DrawLine(m_d2p2, QLineF(static_cast<QPointF>(*d2), ScenePos()));
                    }
                    else
                    {
                        const QSharedPointer<VPointF> d3 = GetData()->GeometricObject<VPointF>(m_dartP3Id);
                        DrawPoint(m_dartP3, static_cast<QPointF>(*d3));
                        DrawLine(m_d2p2, QLineF(static_cast<QPointF>(*d2), static_cast<QPointF>(*d3)));

                        QPointF p1;
                        QPointF p2;
                        VToolTrueDarts::FindPoint(static_cast<QPointF>(*blP1), static_cast<QPointF>(*blP2),
                                                  static_cast<QPointF>(*d1), static_cast<QPointF>(*d2),
                                                  static_cast<QPointF>(*d3), p1, p2);

                        DrawLine(m_lineblP1P1, QLineF(static_cast<QPointF>(*blP1), p1));
                        DrawLine(m_lineblP2P2, QLineF(static_cast<QPointF>(*blP2), p2));
                        DrawLine(m_p1d2, QLineF(p1, static_cast<QPointF>(*d2)));
                        DrawLine(m_d2p2, QLineF(static_cast<QPointF>(*d2), p2));

                        DrawPoint(m_point1, p1);
                        DrawPoint(m_point2, p2);
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolTrueDarts::VisualMode(quint32 id)
{
    m_baseLineP1Id = id;
    StartVisualMode();
}
