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

#include <QObject>
#include <QtMath>

#include "../ifc/ifcdef.h"
#include "layout/layoutdef.h"

class QGraphicsScene;
class VCommonSettings;
class QPainter;

class VPTileFactory : public QObject
{
    Q_OBJECT // NOLINT

public:
    VPTileFactory(const VPLayoutPtr &layout, VCommonSettings *commonSettings, QObject *parent = nullptr);

    ~VPTileFactory() override = default;

    /**
     * @brief drawTile draws the tile of given coordinate (row, col) from the
     * current sheet of the layout with the given painter
     * @param painter pointer to painter
     * @param row current row
     * @param col current column
     * @param showRuler show ruler at the bottom of page
     */
    void drawTile(QPainter *painter, QPrinter *printer, const VPSheetPtr &sheet, int row, int col, bool showRuler = true);

    /**
     * @brief refreshTileInfos Resfreshes the tile infos (m_drawingAreaHeight, m_drawingAreaWidth)
     */
    void RefreshTileInfos();

    void RefreshWatermarkData();

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
    static constexpr qreal tileStripeWidth = CmToPixel(1.0);

    auto WatermarkData() const -> const VWatermarkData &;

    static void PaintWatermarkText(QPainter *painter, const QRectF &img, const VWatermarkData &watermarkData,
                                   qreal xScale = 1.0, qreal yScale = 1.0);
    static void PaintWatermarkImage(QPainter *painter, const QRectF &img, const VWatermarkData &watermarkData,
                                    const QString &watermarkPath, bool folowColorScheme, qreal xScale = 1.0,
                                    qreal yScale = 1.0);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VPTileFactory) // NOLINT

    VPLayoutWeakPtr m_layout;
    VCommonSettings *m_commonSettings{nullptr};

    /**
     * @brief m_drawingAreaHeight the height of the drawing area
     */
    qreal m_drawingAreaHeight{0};

    /**
     * @brief m_drawingAreaWidth the width of the drawing area
     */
    qreal m_drawingAreaWidth{0};

    VWatermarkData m_watermarkData{};

    void DrawRuler(QPainter *painter, qreal scale) const;
    void DrawWatermark(QPainter *painter) const;

    auto PenTileInfos() const -> QPen;

    void DrawTilePageContent(QPainter *painter, const VPSheetPtr &sheet, int row, int col, QPrinter *printer) const;

    void DrawTopTriangle(QPainter *painter) const;
    void DrawLeftTriangle(QPainter *painter) const;
    void DrawBottomTriangle(QPainter *painter) const;
    void DrawRightTriangle(QPainter *painter) const;

    void DrawTopLineScissors(QPainter *painter) const;
    void DrawLeftLineScissors(QPainter *painter) const;

    void DrawTopCuttingLine(QPainter *painter) const;
    void DrawLeftCuttingLine(QPainter *painter) const;
    void DrawBottomCuttingLine(QPainter *painter) const;
    void DrawRightCuttingLine(QPainter *painter) const;

    void DrawSolidTopLine(QPainter *painter, int col, int nbCol) const;
    void DrawSolidLeftLine(QPainter *painter, int row, int nbRow) const;
    void DrawSolidBottomLine(QPainter *painter, int col, int nbCol) const;
    void DrawSolidRightLine(QPainter *painter, int row, int nbRow) const;

    void DrawTextInformation(QPainter *painter, int row, int col, int nbRow, int nbCol, const QString &sheetName) const;
};

#endif // VPTILEFACTORY_H
