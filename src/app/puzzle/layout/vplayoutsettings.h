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

enum class FollowGrainline : qint8 {
    No = 0,
    Follow90 = 1,
    Follow180 = 2
};

enum class PaperSizeTemplate : qint8 {
    A0 = 0,
    A1,
    A2,
    A3,
    A4,
    Letter,
    Legal,
    Tabloid,
    Roll24in,
    Roll30in,
    Roll36in,
    Roll42in,
    Roll44in,
    Roll48in,
    Roll62in,
    Roll72in,
    Custom
};

class VPLayoutSettings
{
    Q_DECLARE_TR_FUNCTIONS(VPLayoutSettings)
public:
    VPLayoutSettings() = default;

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

    void SetWarningSuperpositionOfPieces(bool state);
    auto GetWarningSuperpositionOfPieces() const -> bool;

    void SetWarningPiecesOutOfBound(bool state);
    auto GetWarningPiecesOutOfBound() const -> bool;

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
     * @brief GetOrientation Returns the orientation of the tiles
     * @return orientation of the tiles
     */
    auto GetTilesOrientation() -> PageOrientation;

    /**
     * @brief SetOrientation Sets the orientation of the tiles to the given value
     * @param orientation the new tiles orientation
     */
    void SetTilesOrientation(PageOrientation orientation);

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

    /**
     * @brief GetTemplateSize Returns the size in Px of the given template
     * @param tmpl paper size template
     * @return the size in Px
     */
    static auto GetTemplateSize(PaperSizeTemplate tmpl) -> QSizeF;

    /**
     * @brief GetTemplateName Returns the name of the given template
     * @param tmpl paper size template
     * @return name of the given template
     */
    static auto GetTemplateName(PaperSizeTemplate tmpl) -> QString;

    /**
     * @brief GetTemplate GetTemplate Returns the template that corresponds to the given size
     * @param size the Size in Px
     * @return template that corresponds to the given size
     */
    static auto GetTemplate(QSizeF size) -> PaperSizeTemplate;

    /**
     * @brief PopulateComboBox Populates the given combo with the given templates
     * @param tmpls list of paper size templates
     * @param comboBox pointer to the combobox
     */
    static void PopulateComboBox(QVector<PaperSizeTemplate> *tmpls, QComboBox* comboBox);

    /**
     * @brief isRollTemplate Returns wether the given template is a roll or not.
     * @param tmpl paper size template
     * @return true if the given template is a roll
     */
    static auto isRollTemplate(PaperSizeTemplate tmpl) -> bool;

    /**
     * @brief SetSheetSize sets the size of the sheet, the values have to be in Unit::Px
     * @param width sheet width
     * @param height sheet height
     */
    void SetSheetSize(qreal width, qreal height);

    /**
     * @brief SetSheetSize sets the size of the sheet, the values have to be in the layout's unit
     * @param width sheet width
     * @param height sheet height
     */
    void SetSheetSizeConverted(qreal width, qreal height);

    /**
     * @brief SetSheetSize sets the size of the sheet, the values have to be in Unit::Px
     * @param size sheet size
     */
    void SetSheetSize(const QSizeF &size);
    /**
     * @brief SetSheetSizeConverted sets the size of the sheet, the values have to be in the layout's unit
     * @param size sheet size
     */
    void SetSheetSizeConverted(const QSizeF &size);

    /**
     * @brief GetSheetSize Returns the size in Unit::Px
     * @return sheet size in Unit::Px
     */
    auto GetSheetSize() const -> QSizeF;

    /**
     * @brief GetSheetSizeConverted Returns the size in the layout's unit
     * @return the size in the layout's unit
     */
    auto GetSheetSizeConverted() const -> QSizeF;

    /**
     * @brief GetOrientation Returns the orientation of the sheet
     * @return orientation of the sheet
     */
    auto GetOrientation() -> PageOrientation;

    /**
     * @brief SetOrientation Sets the orientation of the sheet to the given value
     * @param orientation the new page orientation
     */
    void SetOrientation(PageOrientation orientation);

    /**
     * @brief SetSheetMargins, set the margins of the sheet, the values have to be in Unit::Px
     * @param left in Unit::Px
     * @param top in Unit::Px
     * @param right in Unit::Px
     * @param bottom in Unit::Px
     */
    void SetSheetMargins(qreal left, qreal top, qreal right, qreal bottom);

    /**
     * @brief SetSheetMargins, set the margins of the sheet, the values have to be in the unit of the layout
     * @param left in Unit::Px
     * @param top in Unit::Px
     * @param right in Unit::Px
     * @param bottom in Unit::Px
     */
    void SetSheetMarginsConverted(qreal left, qreal top, qreal right, qreal bottom);

    /**
     * @brief SetSheetMargins set the margins of the sheet, the values have to be in Unit::Px
     * @param margins sheet margins
     */
    void SetSheetMargins(const QMarginsF &margins);

    /**
     * @brief SetSheetMargins set the margins of the sheet, the values have to be in the unit of the layout
     * @param margins sheet margins
     */
    void SetSheetMarginsConverted(const QMarginsF &margins);

    /**
     * @brief GetSheetMargins Returns the size in Unit::Px
     * @return the size in Unit::Px
     */
    auto GetSheetMargins() const -> QMarginsF;

    /**
     * @brief GetSheetMarginsConverted Returns the margins in the layout's unit
     * @return the margins in the sheet's unit
     */
    auto GetSheetMarginsConverted() const -> QMarginsF;

    /**
     * @brief SetFollowGrainline Sets the type of grainline for the pieces to follow
     * @param state the type of grainline
     */
    void SetFollowGrainline(FollowGrainline state);

    /**
     * @brief GetFollowGrainline Returns if the sheet's pieces follow a grainline or not
     * @return wether the pieces follow a grainline and if so, which grainline
     */
    auto GetFollowGrainline() const -> FollowGrainline;

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

    void SetStickyEdges(bool state);
    auto GetStickyEdges() const -> bool;

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

    /**
     * @brief holds the orientation of the tiles
     */
    PageOrientation m_tilesOrientation {PageOrientation::Portrait};

    // margins
    /**
     * @brief m_margins the margins of the tiles in Unit::Px
     */
    QMarginsF m_tilesMargins{};

    bool m_showTiles{false};

    /**
     * @brief m_size the Size in Unit::Px
     */
    QSizeF m_size{};

    /**
     * @brief holds the orientation of the sheet
     */
    PageOrientation m_orientation {PageOrientation::Portrait};

    // margins
    /**
     * @brief m_margins the margins in Unit::Px
     */
    QMarginsF m_margins{};

    // control
    FollowGrainline m_followGrainLine{FollowGrainline::No};

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
};

#endif // VPLAYOUTSETTINGS_H
