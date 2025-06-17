/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 6, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#include "scalesceneitems.h"
#include "../vmisc/theme/vscenestylesheet.h"
#include "../vmisc/vabstractapplication.h"
#include "global.h"

#include <QPen>

//---------------------------------------------------------------------------------------------------------------------
VScaledLine::VScaledLine(VColorRole role, QGraphicsItem *parent)
  : QGraphicsLineItem(parent),
    m_role(role)
{
}

//---------------------------------------------------------------------------------------------------------------------
VScaledLine::VScaledLine(const QLineF &line, VColorRole role, QGraphicsItem *parent)
  : QGraphicsLineItem(line, parent),
    m_role(role)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VScaledLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen lPen = pen();

    if (m_role != VColorRole::CustomColor)
    {
        lPen.setColor(VSceneStylesheet::CorrectToolColor(this, VSceneStylesheet::Color(m_role)));
    }

    const qreal width = ScaleWidth(m_isBoldLine ? VAbstractApplication::VApp()->Settings()->WidthMainLine()
                                                : VAbstractApplication::VApp()->Settings()->WidthHairLine(),
                                   SceneScale(scene()));
    lPen.setWidthF(qRound(width * 100.) / 100.);
    setPen(lPen);

    PaintWithFixItemHighlightSelected<QGraphicsLineItem>(this, painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
VScaledEllipse::VScaledEllipse(VColorRole role, QGraphicsItem *parent)
  : QGraphicsEllipseItem(parent),
    m_role(role)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VScaledEllipse::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    const qreal scale = SceneScale(scene());
    const qreal width = ScaleWidth(VAbstractApplication::VApp()->Settings()->WidthMainLine(), scale);

    QPen visPen = pen();

    if (m_role != VColorRole::CustomColor)
    {
        visPen.setColor(VSceneStylesheet::Color(m_role));
    }

    visPen.setWidthF(width);

    setPen(visPen);
    if (m_pointMode)
    {
        ScaleCircleSize(this, scale);
    }

    PaintWithFixItemHighlightSelected<QGraphicsEllipseItem>(this, painter, option, widget);
}
