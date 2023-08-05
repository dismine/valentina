/*******************************************************************
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
#include "../layout/vplayout.h"
#include "../layout/vpsheet.h"
#include "qnamespace.h"
#include "theme/vscenestylesheet.h"

#include <QtMath>

//---------------------------------------------------------------------------------------------------------------------
VPGraphicsSheet::VPGraphicsSheet(const VPLayoutPtr &layout, QGraphicsItem *parent)
  : QGraphicsItem(parent),
    m_layout(layout)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsSheet::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    bool ignoreMargins = true;

    VPLayoutPtr layout = m_layout.toStrongRef();

    if (!layout.isNull())
    {
        VPSheetPtr sheet = layout->GetFocusedSheet();
        if (!sheet.isNull())
        {
            ignoreMargins = sheet->IgnoreMargins();
        }
    }

    if (m_showMargin && !ignoreMargins)
    {
        QPen pen(VSceneStylesheet::ManualLayoutStyle().SheetMarginColor(), 1.5, Qt::SolidLine, Qt::RoundCap,
                 Qt::RoundJoin);
        pen.setCosmetic(true);

        painter->save();
        painter->setPen(pen);
        painter->drawRect(GetMarginsRect());
        painter->restore();
    }

    QRectF sheetRect = GetSheetRect();

    if (m_showBorder)
    {
        QPen pen(VSceneStylesheet::ManualLayoutStyle().SheetBorderColor(), 1.5, Qt::SolidLine, Qt::RoundCap,
                 Qt::RoundJoin);
        pen.setCosmetic(true);

        painter->save();
        painter->setPen(pen);
        painter->drawRect(sheetRect);
        painter->restore();
    }

    if (not layout.isNull() && layout->LayoutSettings().GetShowGrid())
    {
        QPen pen(VSceneStylesheet::ManualLayoutStyle().SheetGridColor(), 1.5, Qt::SolidLine, Qt::RoundCap,
                 Qt::RoundJoin);
        pen.setCosmetic(true);

        painter->save();
        painter->setPen(pen);

        qreal colWidth = layout->LayoutSettings().GetGridColWidth();
        if (colWidth > 0)
        {
            qreal colX = colWidth;
            while (colX < sheetRect.right())
            {
                QLineF line = QLineF(colX, 0, colX, sheetRect.bottom());
                painter->drawLine(line);
                colX += colWidth;
            }
        }

        qreal rowHeight = layout->LayoutSettings().GetGridRowHeight();
        if (rowHeight > 0)
        {
            qreal rowY = rowHeight;

            while (rowY < sheetRect.bottom())
            {
                QLineF line = QLineF(0, rowY, sheetRect.right(), rowY);
                painter->drawLine(line);
                rowY += rowHeight;
            }
        }
        painter->restore();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsSheet::GetSheetRect() const -> QRectF
{
    VPLayoutPtr layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return {};
    }

    VPSheetPtr sheet = layout->GetFocusedSheet();
    if (sheet.isNull())
    {
        return {};
    }
    return sheet->GetSheetRect();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsSheet::GetMarginsRect() const -> QRectF
{
    VPLayoutPtr layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return {};
    }

    VPSheetPtr sheet = layout->GetFocusedSheet();
    if (sheet.isNull())
    {
        return {};
    }
    return sheet->GetMarginsRect();
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
void VPGraphicsSheet::RefreshBoundingRect()
{
    prepareGeometryChange();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsSheet::boundingRect() const -> QRectF
{
    return GetSheetRect();
}
