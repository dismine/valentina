/************************************************************************
 **
 **  @file   vpuzzlegraphicslayout.cpp
 **  @author Ronan Le Tiec
 **  @date   3 5, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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

#include "vpuzzlegraphicslayout.h"

//---------------------------------------------------------------------------------------------------------------------
VPuzzleGraphicsLayout::VPuzzleGraphicsLayout(VPuzzleLayout *layout, QGraphicsItem *parent):
    QGraphicsItem(parent),
    m_layout(layout),
    m_boundingRect(GetLayoutRect())
{

}

//---------------------------------------------------------------------------------------------------------------------
VPuzzleGraphicsLayout::~VPuzzleGraphicsLayout()
{

}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleGraphicsLayout::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    QPen pen(QColor(0,179,255), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    pen.setCosmetic(true);
    QBrush noBrush(Qt::NoBrush);
    painter->setPen(pen);
    painter->setBrush(noBrush);

    painter->drawRect(GetMarginsRect());

    pen.setColor(Qt::black);

    painter->setPen(pen);
    painter->drawRect(GetLayoutRect());

    m_boundingRect = GetLayoutRect();
}

//---------------------------------------------------------------------------------------------------------------------
QRectF VPuzzleGraphicsLayout::GetLayoutRect() const
{
    QRectF rect = QRectF(QPointF(0,0), m_layout->GetLayoutSize());
    return rect;
}

//---------------------------------------------------------------------------------------------------------------------
QRectF VPuzzleGraphicsLayout::GetMarginsRect() const
{
    QMarginsF margins = m_layout->GetLayoutMargins();
    QSizeF size = m_layout->GetLayoutSize();
    QRectF rect = QRectF(
                    QPointF(margins.left(),margins.top()),
                    QPointF(size.width()-margins.right(), size.height()-margins.bottom())
                );
    return rect;
}


//---------------------------------------------------------------------------------------------------------------------
QRectF VPuzzleGraphicsLayout::boundingRect() const
{
    return m_boundingRect;
}
