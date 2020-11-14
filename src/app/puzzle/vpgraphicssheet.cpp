/************************************************************************
 **
 **  @file   vpgraphicssheet.cpp
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

#include "vpgraphicssheet.h"
#include "vplayout.h"
#include <QtMath>

//---------------------------------------------------------------------------------------------------------------------
VPGraphicsSheet::VPGraphicsSheet(VPSheet *sheet, QGraphicsItem *parent):
    QGraphicsItem(parent),
    m_sheet(sheet),
    m_boundingRect(GetSheetRect())
{

}

//---------------------------------------------------------------------------------------------------------------------
VPGraphicsSheet::~VPGraphicsSheet()
{

}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsSheet::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    QPen pen(QColor(0,179,255), 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    pen.setCosmetic(true);
    QBrush noBrush(Qt::NoBrush);
    painter->setPen(pen);
    painter->setBrush(noBrush);

    if(m_showMargin)
    {
        painter->drawRect(GetMarginsRect());
    }

    if(m_showBorder)
    {
        pen.setColor(Qt::black);

        painter->setPen(pen);
        painter->drawRect(GetSheetRect());
    }

    // show the tiles grid. Maybe it shouldn't be in the graphics sheet, maybe better in maingraphicsview
    VPLayout* layout = m_sheet->GetLayout();
    if(layout->GetShowTiles())
    {
       pen.setColor(QColor(255,0,0,127));
       pen.setStyle(Qt::DashLine);
       painter->setPen(pen);

       QSizeF tilesSize = layout->GetTilesSize();
       QSizeF sheetSize = m_sheet->GetSheetSize();

       QMarginsF tilesMargins = layout->GetTilesMargins();

       PageOrientation tilesOrientation = layout->GetTilesOrientation();
       PageOrientation sheetOrientation = m_sheet->GetOrientation();

       qreal colWidth = 0;
       qreal rowHeight = 0;
       if(tilesOrientation == PageOrientation::Portrait)
       {
           colWidth = tilesSize.width() - (tilesMargins.left()+ tilesMargins.right() + UnitConvertor(1, Unit::Cm, Unit::Px));
           rowHeight = tilesSize.height() - (tilesMargins.top()+ tilesMargins.bottom() + UnitConvertor(1, Unit::Cm, Unit::Px));
       }
       else
       {
           colWidth = tilesSize.height() - (tilesMargins.left()+ tilesMargins.right() + UnitConvertor(1, Unit::Cm, Unit::Px));
           rowHeight = tilesSize.width() - (tilesMargins.top()+ tilesMargins.bottom() + UnitConvertor(1, Unit::Cm, Unit::Px));
       }
       // the "+ UnitConvertor(1, Unit::Cm, Unit::Px)" is because of the part for gluing and where we
       // have infos of the single tile. Maybe it's not the right value, to be corrected.


       qreal drawingWidth = 0;
       qreal drawingHeight = 0;

       if(sheetOrientation == PageOrientation::Portrait)
       {
            drawingWidth = sheetSize.width();
            drawingHeight = sheetSize.height();
       }
       else
       {
           drawingWidth = sheetSize.height();
           drawingHeight = sheetSize.width();
       }

       int nbCol = qCeil(drawingWidth/colWidth);
       int nbRow = qCeil(drawingHeight/rowHeight);

       for(int i=0;i<=nbCol;i++)
       {
           painter->drawLine(QPointF(i*colWidth, 0), QPointF(i*colWidth,nbRow*rowHeight));
       }

       for(int j=0;j<=nbRow;j++)
       {
            painter->drawLine(QPointF(0, j*rowHeight), QPointF(nbCol*colWidth, j*rowHeight));
       }
    }

    m_boundingRect = GetSheetRect();
}

//---------------------------------------------------------------------------------------------------------------------
QRectF VPGraphicsSheet::GetSheetRect() const
{
    QPoint topLeft = QPoint(0,0);
    QSizeF size = m_sheet->GetSheetSize();
    if(m_sheet->GetOrientation() == PageOrientation::Landscape)
    {
        size.transpose();
    }
    QRectF rect = QRectF(topLeft, size);
    return rect;
}

//---------------------------------------------------------------------------------------------------------------------
QRectF VPGraphicsSheet::GetMarginsRect() const
{
    QMarginsF margins = m_sheet->GetSheetMargins();
    QSizeF size = m_sheet->GetSheetSize();

    if(m_sheet->GetOrientation() == PageOrientation::Landscape)
    {
        size.transpose();
    }

    QRectF rect = QRectF(
                    QPointF(margins.left(),margins.top()),
                    QPointF(size.width()-margins.right(), size.height()-margins.bottom())
                );
    return rect;
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsSheet::SetShowMargin(bool value)
{
    m_showMargin = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsSheet::SetShowBorder(bool value)
{
   m_showBorder = value;
}


//---------------------------------------------------------------------------------------------------------------------
QRectF VPGraphicsSheet::boundingRect() const
{
    return m_boundingRect;
}
