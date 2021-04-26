/************************************************************************
 **
 **  @file   vplayout.h
 **  @author Ronan Le Tiec
 **  @date   13 4, 2020
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
#ifndef VPLAYOUT_H
#define VPLAYOUT_H


#include <QList>

#include "def.h"

class VPPieceList;
class VPPiece;
class VPSheet;

class VPLayout : public QObject
{
    Q_OBJECT
public:
    VPLayout();
    virtual ~VPLayout();

    /**
     * @brief GetUnplacedPieceList Returns the piece list of unplaced pieces
     * @return the unplaced pieces list
     */
    VPPieceList* GetUnplacedPieceList();

    /**
     * @brief GetTrashPieceList Returns the piece list of the trash
     * @return the pieces list of trashed pieces.
     */
    VPPieceList* GetTrashPieceList();

    VPSheet* AddSheet();
    VPSheet* AddSheet(VPSheet *sheet);
    QList<VPSheet *> GetSheets();

    /**
     * @brief GetSelectedPieces Returns the list of the selected pieces
     * @return the selected pieces
     */
    QList<VPPiece *> GetSelectedPieces();

    /**
     * @brief SetUnit Sets the unit of the layout to the given unit
     * @param unit the new unit
     */
    void SetUnit(Unit unit);

    /**
     * @brief GetUnit Returns the current unit of the layout
     * @return the unit
     */
    Unit GetUnit() const;

    void SetWarningSuperpositionOfPieces(bool state);
    bool GetWarningSuperpositionOfPieces() const;

    void SetWarningPiecesOutOfBound(bool state);
    bool GetWarningPiecesOutOfBound() const;

    /**
     * @brief SetTitle Sets the title of the layout to the given value
     * @param title the title of the layout
     */
    void SetTitle(QString title);

    /**
     * @brief GetTitle Returns the title of the layout
     * @return title of the layout
     */
    QString GetTitle() const;

    /**
     * @brief SetDescription Sets the description of the layout to the given value
     * @param description the description of the layout
     */
    void SetDescription(QString description);

    /**
     * @brief GetDescription Returns the description of the layout.
     * @return description of the layout
     */
    QString GetDescription() const;

    /**
     * @brief ClearSelection goes through the unplaced pieces and through the sheets and calls
     * SetIsSelected(false) for the pieces that were selected.
     */
    void ClearSelection();

    /**
     * @brief ClearSelectionExceptForPieceList same as clearSelection but it leaves the selection
     * for the given piece list like it ist.
     *
     * @param pieceList the piece list to let be the way it is.
     */
    void ClearSelectionExceptForGivenPieceList(VPPieceList* pieceList);

    /**
     * @brief MovePieceToPieceList Moves the given piece to the given piece list
     * @param piece the piece to move
     * @param pieceList the piece list to move the piece to
     */
    void MovePieceToPieceList(VPPiece* piece, VPPieceList* pieceList);

    /**
     * @brief SetFocusedSheet Sets the focused sheet, to which pieces are added from the carrousel via drag
     * and drop
     * @param focusedSheet the new active sheet. If nullptr, then it sets automaticaly the first sheet from m_sheets
     */
    void SetFocusedSheet(VPSheet *focusedSheet = nullptr);

    /**
     * @brief GetFocusedSheet Returns the focused sheet, to which pieces are added from the carrousel via drag
     * and drop
     * @return the focused sheet
     */
    VPSheet* GetFocusedSheet();


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
    QSizeF GetTilesSize() const;

    /**
     * @brief GetTilesSize Returns the size of the tiles in given Unit
     * @return tiles size
     */
    QSizeF GetTilesSize(Unit unit) const;

    /**
     * @brief GetTilesSizeConverted Returns the size of the tiles in the layout's unit
     * @return the size in the layout's unit
     */
    QSizeF GetTilesSizeConverted() const;

    /**
     * @brief GetOrientation Returns the orientation of the tiles
     * @return orientation of the tiles
     */
    PageOrientation GetTilesOrientation();

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
    QMarginsF GetTilesMargins() const;

    /**
     * @brief GetTilesMargins Returns margins of the tiles in the given unit
     * @param unit the unit in which we want the margins
     * @return the margins in the given unit
     */
    QMarginsF GetTilesMargins(Unit unit) const;

    /**
     * @brief GetTilesMarginsConverted Returns the margins of the tiles in the layout's unit
     * @return the margins in the tiles's unit
     */
    QMarginsF GetTilesMarginsConverted() const;

    /**
     * @brief GetShowTiles Returns true if the tiles has to be shown on the current sheet
     * @return true if the tiles has to be shown on the current sheet
     */
    bool GetShowTiles();

    /**
     * @brief SetShowTiles Sets wether to show the tiles on the current sheet or not
     * @param value true to show the tiles
     */
    void SetShowTiles(bool value);



signals:

    void PieceMovedToPieceList(VPPiece *piece, VPPieceList *pieceListBefore, VPPieceList *pieceListAfter);

private:
    Q_DISABLE_COPY(VPLayout)

    VPPieceList *m_unplacedPieceList;

    /**
     * @brief m_trashPieceList Holds the pieces that were deleted
     */
    VPPieceList *m_trashPieceList;

    QList<VPSheet*> m_sheets;
    VPSheet *m_focusedSheet{nullptr};

    // format
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
};

#endif // VPLAYOUT_H
