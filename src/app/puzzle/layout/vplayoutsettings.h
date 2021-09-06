/************************************************************************
 **
 **  @file   vplayoutsettings.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 7, 2021
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2021 Valentina project
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
#ifndef VPLAYOUTSETTINGS_H
#define VPLAYOUTSETTINGS_H

#include <QMarginsF>
#include <QSizeF>
#include <QString>
#include <QCoreApplication>

#include "def.h"

class VPLayoutSettings
{
    Q_DECLARE_TR_FUNCTIONS(VPLayoutSettings)
public:
    VPLayoutSettings() = default;

    // Layout

    /**
     * @brief SetUnit Sets the unit of the layout to the given unit
     * @param unit the new unit
     */
    void SetUnit(Unit unit);

    /**
     * @brief GetUnit Returns the current unit of the layout
     * @return the unit
     */
    auto GetUnit() const -> Unit;

    /**
     * @brief SetTitle Sets the title of the layout to the given value
     * @param title the title of the layout
     */
    void SetTitle(const QString &title);

    /**
     * @brief GetTitle Returns the title of the layout
     * @return title of the layout
     */
    auto GetTitle() const -> QString;

    /**
     * @brief SetDescription Sets the description of the layout to the given value
     * @param description the description of the layout
     */
    void SetDescription(const QString &description);

    /**
     * @brief GetDescription Returns the description of the layout.
     * @return description of the layout
     */
    auto GetDescription() const -> QString;

    void SetStickyEdges(bool state);
    auto GetStickyEdges() const -> bool;

    // Piece

    void SetWarningSuperpositionOfPieces(bool state);
    auto GetWarningSuperpositionOfPieces() const -> bool;

    void SetWarningPiecesOutOfBound(bool state);
    auto GetWarningPiecesOutOfBound() const -> bool;

    /**
     * @brief SetFollowGrainline Sets the type of grainline for the pieces to follow
     * @param state the type of grainline
     */
    void SetFollowGrainline(bool state);

    /**
     * @brief GetFollowGrainline Returns if the sheet's pieces follow a grainline or not
     * @return wether the pieces follow a grainline and if so, which grainline
     */
    auto GetFollowGrainline() const -> bool;

    /**
     * @brief SetPiecesGap sets the pieces gap to the given value, the unit has to be in Unit::Px
     * @param value pieces gap
     */
    void SetPiecesGap(qreal value);

    /**
     * @brief SetPiecesGapConverted sets the pieces gap to the given value, the unit has to be in the layout's unit
     * @param value pieces gap
     */
    void SetPiecesGapConverted(qreal value);

    /**
     * @brief GetPiecesGap returns the pieces gap in Unit::Px
     * @return the pieces gap in Unit::Px
     */
    auto GetPiecesGap() const -> qreal;

    /**
     * @brief GetPiecesGapConverted returns the pieces gap in the layout's unit
     * @return the pieces gap in the layout's unit
     */
    auto GetPiecesGapConverted() const -> qreal;

    // Tiles

    /**
     * @brief SetTilesSize sets the size of the tiles, the values have to be in Unit::Px
     * @param width tiles width
     * @param height tiles height
     */
    void SetTilesSize(qreal width, qreal height);

    /**
     * @brief SetTilesSizeConverted sets the size of the sheet, the values have to be in the layout's unit
     * @param width tiles width
     * @param height tiles height
     */
    void SetTilesSizeConverted(qreal width, qreal height);

    /**
     * @brief SetTilesSize sets the size of the tiles, the values have to be in Unit::Px
     * @param size tiles size
     */
    void SetTilesSize(const QSizeF &size);
    /**
     * @brief SetTilesSizeConverted sets the size of the tiles, the values have to be in the layout's unit
     * @param size tiles size
     */
    void SetTilesSizeConverted(const QSizeF &size);

    auto GetTilesOrientation() const -> QPageLayout::Orientation;

    /**
     * @brief GetTilesSize Returns the size of the tiles in Unit::Px
     * @return tiles size in Unit::Px
     */
    auto GetTilesSize() const -> QSizeF;

    /**
     * @brief GetTilesSize Returns the size of the tiles in given Unit
     * @return tiles size
     */
    auto GetTilesSize(Unit unit) const -> QSizeF;

    /**
     * @brief GetTilesSizeConverted Returns the size of the tiles in the layout's unit
     * @return the size in the layout's unit
     */
    auto GetTilesSizeConverted() const -> QSizeF;

    /**
     * @brief SetTilesMargins, set the margins of the tiles, the values have to be in Unit::Px
     * @param left in Unit::Px
     * @param top in Unit::Px
     * @param right in Unit::Px
     * @param bottom in Unit::Px
     */
    void SetTilesMargins(qreal left, qreal top, qreal right, qreal bottom);

    /**
     * @brief SetSheetMargins, set the margins of the tiles, the values have to be in the unit of the layout
     * @param left in Unit::Px
     * @param top in Unit::Px
     * @param right in Unit::Px
     * @param bottom in Unit::Px
     */
    void SetTilesMarginsConverted(qreal left, qreal top, qreal right, qreal bottom);

    /**
     * @brief SetTilesMargins set the margins of the tiles, the values have to be in Unit::Px
     * @param margins tiles margins
     */
    void SetTilesMargins(const QMarginsF &margins);

    /**
     * @brief SetTilesMarginsConverted set the margins of the tiles, the values have to be in the unit of the layout
     * @param margins tiles margins
     */
    void SetTilesMarginsConverted(const QMarginsF &margins);

    /**
     * @brief GetTilesMargins Returns margins of the tiles in Unit::Px
     * @return the margins in Unit::Px
     */
    auto GetTilesMargins() const -> QMarginsF;

    /**
     * @brief GetTilesMargins Returns margins of the tiles in the given unit
     * @param unit the unit in which we want the margins
     * @return the margins in the given unit
     */
    auto GetTilesMargins(Unit unit) const -> QMarginsF;

    /**
     * @brief GetTilesMarginsConverted Returns the margins of the tiles in the layout's unit
     * @return the margins in the tiles's unit
     */
    auto GetTilesMarginsConverted() const -> QMarginsF;

    /**
     * @brief GetShowTiles Returns true if the tiles has to be shown on the current sheet
     * @return true if the tiles has to be shown on the current sheet
     */
    auto GetShowTiles() const -> bool;

    /**
     * @brief SetShowTiles Sets wether to show the tiles on the current sheet or not
     * @param value true to show the tiles
     */
    void SetShowTiles(bool value);

    // Sheet
    /**
     * @brief GetShowGrid Returns true if the placement grid has to be shown on the current sheet
     * @return true if the placement grid has to be shown on the current sheet
     */
    auto GetShowGrid() const -> bool;

    /**
     * @brief SetShowGrid Returns true if the placement grid has to be shown on the current sheet
     * @param value whether to show the grid or not
     */
    void SetShowGrid(bool value);

    /**
    * @brief GetGridColWidth returns the placement grid column width in Unit::Px
    * @return the placement grid column width in Unit::Px
    */
    auto GetGridColWidth() const -> qreal;

    /**
    * @brief GetGridColWidth returns the placement grid column width in the layout's unit
    * @return the placement grid column width in the layout's unit
    */
    auto GetGridColWidthConverted() const -> qreal;

    /**
     * @brief SetGridColWidth sets the placement grid column width to the given value, the unit has to be Unit::Px
     * @param value the placement grid column width in Unit::Px
     */
    void SetGridColWidth(qreal value);

    /**
     * @brief SetGridColWidthConverted sets the placement grid column width to the given value, the unit has to be in
     * the layout's unit
     * @param value the placement grid column width in the layout's unit
     */
    void SetGridColWidthConverted(qreal value);

    /**
    * @brief GetGridRowHeight returns the placement grid row height in Unit::Px
    * @return the placement grid row height in Unit::Px
    */
    auto GetGridRowHeight() const -> qreal;

    /**
    * @brief GetGridRowHeightConverted returns the placement grid row height in the layout's unit
    * @return the placement grid row height in the layout's unit
    */
    auto GetGridRowHeightConverted() const -> qreal;

    /**
     * @brief SetGridRowHeight sets the placement grid row height to the given value, the unit has to be Unit::Px
     * @param value the placement grid row height in Unit::Px
     */
    void SetGridRowHeight(qreal value);

    /**
     * @brief SetGridRowHeightConverted sets the placement grid row height to the given value, the unit has to be in
     * the layout's unit
     * @param value the placement grid row height in the layout's unit
     */
    void SetGridRowHeightConverted(qreal value);

    auto IgnoreTilesMargins() const -> bool;
    void SetIgnoreTilesMargins(bool newIgnoreTilesMargins);

    auto HorizontalScale() const -> qreal;
    void SetHorizontalScale(qreal newHorizontalScale);

    auto VerticalScale() const -> qreal;
    void SetVerticalScale(qreal newVerticalScale);

private:
    Unit m_unit{Unit::Cm};

    bool m_warningSuperpositionOfPieces{false};
    bool m_warningPiecesOutOfBound{false};

    QString m_title{};
    QString m_description{};

    /**
     * @brief m_size the Size of the tiles in Unit::Px
     */
    QSizeF m_tilesSize{};

    // margins
    /**
     * @brief m_margins the margins of the tiles in Unit::Px
     */
    QMarginsF m_tilesMargins{};

    bool m_ignoreTilesMargins{false};

    bool m_showTiles{false};

    // control
    bool m_followGrainLine{false};

    /**
     * @brief m_piecesGap the pieces gap in Unit::Px
     */
    qreal m_piecesGap{0};

    // placement grid
    /**
     * @brief GetShowGrid Returns true if the placement grid has to be shown on the current sheet
     */
    bool m_showGrid{false};
    /**
     * @brief m_gridColWidth the column width of the placement grid in Unit::Px
     */
    qreal m_gridColWidth{0};

    /**
     * @brief m_gridRowHeight the row height of the placement grid in Unit::Px
     */
    qreal m_gridRowHeight{0};

    bool m_stickyEdges{false};

    qreal m_horizontalScale{1.0};
    qreal m_verticalScale{1.0};
};

#endif // VPLAYOUTSETTINGS_H
