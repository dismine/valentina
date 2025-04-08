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

#include "../layout/layoutdef.h"

class VPTileFactory;
class VPLayout;
struct VWatermarkData;
class VSvgFontEngine;

class VPGraphicsTileGrid : public QGraphicsItem
{
public:
    explicit VPGraphicsTileGrid(const VPLayoutPtr &layout, const QUuid &sheetUuid, QGraphicsItem *parent = nullptr);
    ~VPGraphicsTileGrid() override = default;

    auto boundingRect() const -> QRectF override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    auto PrintMode() const -> bool;
    void SetPrintMode(bool newPrintMode);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VPGraphicsTileGrid) // NOLINT

    VPLayoutWeakPtr m_layout;
    QUuid m_sheetUuid;
    bool m_printMode{false};

    void PaintTileNumberOutlineFont(QPainter *painter,
                                    const QRectF &img,
                                    int i,
                                    int j,
                                    const VPLayoutPtr &layout,
                                    const QFont &font,
                                    int nbCol) const;
    void PaintTileNumberSVGFont(QPainter *painter,
                                const QRectF &img,
                                int i,
                                int j,
                                const VPLayoutPtr &layout,
                                const QFont &font,
                                int nbCol,
                                const VSvgFontEngine &engine) const;
    void DrawGrid(QPainter *painter, int nbRow, int nbCol, const QMarginsF &margins, qreal width, qreal height) const;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VPGraphicsTileGrid::PrintMode() const -> bool
{
    return m_printMode;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPGraphicsTileGrid::SetPrintMode(bool newPrintMode)
{
    m_printMode = newPrintMode;
}

#endif // VPGRAPHICSTILEGRID_H
