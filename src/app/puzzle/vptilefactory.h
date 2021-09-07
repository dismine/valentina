/************************************************************************
 **
 **  @file   vptilefactory.h
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

#ifndef VPTILEFACTORY_H
#define VPTILEFACTORY_H

#include <QtMath>
#include <QObject>

#include "layout/vplayout.h"

class QGraphicsScene;
class VCommonSettings;
class QPainter;

class VPTileFactory : QObject
{
    Q_OBJECT

public:
    VPTileFactory(const VPLayoutPtr &layout, VCommonSettings *commonSettings);

    virtual ~VPTileFactory() = default;

    /**
     * @brief drawTile draws the tile of given coordinate (row, col) from the
     * current sheet of the layout with the given painter
     * @param painter pointer to painter
     * @param row current row
     * @param col current column
     */
    void drawTile(QPainter *painter, QPrinter *printer, const VPSheetPtr &sheet, int row, int col);

    /**
     * @brief refreshTileInfos Resfreshes the tile infos (m_drawingAreaHeight, m_drawingAreaWidth)
     */
    void refreshTileInfos();

    /**
     * @brief RowNb Returns the number of row pages
     * @return number of row pages
     */
    auto RowNb(const VPSheetPtr &sheet) const -> int;

    /**
     * @brief ColNb Returns the number of col pages
     * @return number of col pages
     */
    auto ColNb(const VPSheetPtr &sheet) const -> int;

    /**
     * @brief DrawingAreaHeight Returns the usable height of the tile in Px
     * @return usable height of the tile in Px
     */
    auto DrawingAreaHeight() const -> qreal;

    /**
     * @brief DrawingAreaWidth Returns the usable width of the tile in Px
     * @return usable width of the tile in Px
     */
    auto DrawingAreaWidth() const -> qreal;

    /**
     * @brief tileStripeWidth the width of the info / glueing stripe in Px
     */
    static constexpr qreal tileStripeWidth = UnitConvertor(1, Unit::Cm, Unit::Px);

private:
    Q_DISABLE_COPY(VPTileFactory)

    VPLayoutWeakPtr  m_layout;
    VCommonSettings *m_commonSettings{nullptr};

    /**
     * @brief m_drawingAreaHeight the height of the drawing area
     */
    qreal m_drawingAreaHeight{0};

    /**
     * @brief m_drawingAreaWidth the width of the drawing area
     */
    qreal m_drawingAreaWidth{0};

};

#endif // VPTILEFACTORY_H
