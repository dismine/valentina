/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 9, 2016
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

#include "vistoolflippingbyline.h"
#include "../vgeometry/vpointf.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolFlippingByLine::VisToolFlippingByLine(const VContainer *data, QGraphicsItem *parent)
    : VisOperation(data, parent)
{
    m_point1 = InitPoint(Color(VColor::SupportColor2), this);
    m_point2 = InitPoint(Color(VColor::SupportColor2), this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolFlippingByLine::RefreshGeometry()
{
    if (Objects().isEmpty())
    {
        return;
    }

    QPointF firstPoint;
    QPointF secondPoint;

    if (m_lineP1Id != NULL_ID)
    {
        firstPoint = static_cast<QPointF>(*GetData()->GeometricObject<VPointF>(m_lineP1Id));
        DrawPoint(m_point1, firstPoint, Color(VColor::SupportColor2));

        if (m_lineP2Id == NULL_ID)
        {
            secondPoint = ScenePos();
        }
        else
        {
            secondPoint = static_cast<QPointF>(*GetData()->GeometricObject<VPointF>(m_lineP2Id));
            DrawPoint(m_point2, secondPoint, Color(VColor::SupportColor2));
        }

        DrawLine(this, QLineF(firstPoint, secondPoint), Color(VColor::SupportColor2), Qt::DashLine);
    }

    RefreshFlippedObjects(m_lineP1Id, firstPoint, secondPoint);
}
