/************************************************************************
 **
 **  @file   vpuzzlegraphicslayout.h
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

#ifndef VPUZZLEGRAPHICSLAYOUT_H
#define VPUZZLEGRAPHICSLAYOUT_H

#include <QGraphicsItem>
#include <QPainter>

#include "vpuzzlelayout.h"

class VPuzzleGraphicsLayout : public QGraphicsItem
{
public:
    explicit VPuzzleGraphicsLayout(VPuzzleLayout *layout, QGraphicsItem *parent = nullptr);
    ~VPuzzleGraphicsLayout();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;


    QRectF GetLayoutRect() const;
    QRectF GetMarginsRect() const;


private:
    Q_DISABLE_COPY(VPuzzleGraphicsLayout)

    VPuzzleLayout *m_layout{nullptr};
    QRectF m_boundingRect;
};

#endif // VPUZZLEGRAPHICSLAYOUT_H
