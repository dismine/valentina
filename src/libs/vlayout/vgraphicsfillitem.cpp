/************************************************************************
 **
 **  @file   vgraphicsfillitem.cpp
 **  @author Bojan Kverh
 **  @date   October 16, 2016
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

#include "vgraphicsfillitem.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)
#include "../vmisc/backport/qpainterstateguard.h"
#else
#include <QPainterStateGuard>
#endif

//---------------------------------------------------------------------------------------------------------------------
VGraphicsFillItem::VGraphicsFillItem(QGraphicsItem *parent)
  : QGraphicsPathItem(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
VGraphicsFillItem::VGraphicsFillItem(const QPainterPath &path, QGraphicsItem *parent)
  : QGraphicsPathItem(path, parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VGraphicsFillItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    QPainterStateGuard const guard(painter);

    QPen pen;

    if (m_customPen)
    {
        pen = this->pen();
    }
    else
    {
        pen = painter->pen();
        pen.setWidthF(width);
        pen.setCapStyle(Qt::RoundCap);
        pen.setJoinStyle(Qt::RoundJoin);
    }
    painter->setPen(pen);

    painter->setBrush(painter->pen().color());
    painter->drawPath(path());
}

//---------------------------------------------------------------------------------------------------------------------
VGraphicsFoldLineItem::VGraphicsFoldLineItem(QGraphicsItem *parent)
  : VGraphicsFillItem(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
VGraphicsFoldLineItem::VGraphicsFoldLineItem(const QPainterPath &path, QGraphicsItem *parent)
  : VGraphicsFillItem(path, parent)
{
}
