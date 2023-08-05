/************************************************************************
 **
 **  @file   vgraphicslineitem.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 7, 2014
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

#include "vistoolline.h"

#include <QGraphicsLineItem>
#include <QPen>
#include <QPointF>
#include <QSharedPointer>
#include <new>

#include "../vgeometry/vpointf.h"
#include "../visualization.h"
#include "../vpatterndb/vcontainer.h"
#include "visline.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolLine::VisToolLine(const VContainer *data, QGraphicsItem *parent)
  : VisLine(data, parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolLine::RefreshGeometry()
{
    QLineF line;
    const QSharedPointer<VPointF> first = GetData()->GeometricObject<VPointF>(m_point1Id);
    if (m_point2Id == NULL_ID)
    {
        line = QLineF(static_cast<QPointF>(*first), ScenePos());
    }
    else
    {
        const QSharedPointer<VPointF> second = GetData()->GeometricObject<VPointF>(m_point2Id);
        line = QLineF(static_cast<QPointF>(*first), static_cast<QPointF>(*second));
    }
    DrawLine(this, line, LineStyle());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolLine::VisualMode(quint32 id)
{
    m_point1Id = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolLine::DrawLine(VScaledLine *lineItem, const QLineF &line, Qt::PenStyle style)
{
    SCASSERT(lineItem != nullptr)

    QPen visPen = lineItem->pen();
    visPen.setStyle(style);

    lineItem->setPen(visPen);
    lineItem->setLine(line);
}
