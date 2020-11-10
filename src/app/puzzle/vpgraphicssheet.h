/************************************************************************
 **
 **  @file   vpgraphicssheet.h
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

#ifndef VPGRAPHICSSHEET_H
#define VPGRAPHICSSHEET_H

#include <QGraphicsItem>
#include <QPainter>

#include "vpsheet.h"

class VPGraphicsSheet : public QGraphicsItem
{
public:
    explicit VPGraphicsSheet(VPSheet *sheet, QGraphicsItem *parent = nullptr);
    ~VPGraphicsSheet();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;


    QRectF GetSheetRect() const;
    QRectF GetMarginsRect() const;


private:
    Q_DISABLE_COPY(VPGraphicsSheet)

    VPSheet *m_sheet{nullptr};
    QRectF m_boundingRect;
};

#endif // VPGRAPHICSSHEET_H
