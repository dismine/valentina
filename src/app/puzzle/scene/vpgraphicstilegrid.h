/************************************************************************
 **
 **  @file   vpgraphicstilegrid.h
 **  @author Ronan Le Tiec
 **  @date   19 11, 2020
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

#ifndef VPGRAPHICSTILEGRID_H
#define VPGRAPHICSTILEGRID_H

#include <QGraphicsItem>
#include <QPainter>
#include <QUuid>

#include "../vmisc/def.h"
#include "../layout/layoutdef.h"

class VPTileFactory;
class VPLayout;

class VPGraphicsTileGrid : public QGraphicsItem
{
public:
    explicit VPGraphicsTileGrid(const VPLayoutPtr &layout, const QUuid &sheetUuid, QGraphicsItem *parent = nullptr);
    ~VPGraphicsTileGrid()=default;

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private:
    Q_DISABLE_COPY(VPGraphicsTileGrid)

    VPLayoutWeakPtr m_layout;
    QUuid m_sheetUuid;
};

#endif // VPGRAPHICSTILEGRID_H
