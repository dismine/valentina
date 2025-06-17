/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 9, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#include "vistoolflippingbyaxis.h"
#include "../vgeometry/vpointf.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolFlippingByAxis::VisToolFlippingByAxis(const VContainer *data, QGraphicsItem *parent)
  : VisOperation(data, parent)
{
    SetColorRole(VColorRole::VisSupportColor2);

    m_point1 = InitPoint(VColorRole::VisSupportColor2, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolFlippingByAxis::RefreshGeometry()
{
    if (Objects().isEmpty())
    {
        return;
    }

    QPointF firstPoint;
    QPointF secondPoint;

    if (m_originPointId != NULL_ID)
    {
        firstPoint = static_cast<QPointF>(*GetData()->GeometricObject<VPointF>(m_originPointId));
        DrawPoint(m_point1, firstPoint);

        if (m_axisType == AxisType::VerticalAxis)
        {
            secondPoint = QPointF(firstPoint.x(), firstPoint.y() + 100);
        }
        else
        {
            secondPoint = QPointF(firstPoint.x() + 100, firstPoint.y());
        }

        DrawLine(this, Axis(firstPoint, secondPoint), Qt::DashLine);
    }

    RefreshFlippedObjects(m_originPointId, firstPoint, secondPoint);
}
