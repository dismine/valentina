/************************************************************************
 **
 **  @file   vdecorationaligningdelegate.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 10, 2020
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
#include "vdecorationaligningdelegate.h"

#include <QPainter>

//---------------------------------------------------------------------------------------------------------------------
VDecorationAligningDelegate::VDecorationAligningDelegate(Qt::Alignment alignment, QObject *parent)
  : QStyledItemDelegate(parent),
    m_alignment(alignment)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VDecorationAligningDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                        const QModelIndex &index) const
{
    auto icon(qvariant_cast<QIcon>(index.data(Qt::DecorationRole)));

    if ((option.state & QStyle::State_Selected) != 0U)
    {
        painter->fillRect(option.rect, option.palette.highlight());
    }

    if ((index.flags() & Qt::ItemIsEnabled) != 0U)
    {
        icon.paint(painter, option.rect, m_alignment);
    }
    else
    {
        icon.paint(painter, option.rect, m_alignment, QIcon::Disabled);
    }
}
