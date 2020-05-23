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
     * @brief ClearSelection goes through the unplaced pieces and through the sheets and calls
     * SetIsSelected(false) for the pieces that were selected.
     */
    void ClearSelection();

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


signals:

    void PieceMovedToPieceList(VPPiece *piece, VPPieceList *pieceListBefore, VPPieceList *pieceListAfter);

private:
    Q_DISABLE_COPY(VPLayout)

    VPPieceList *m_unplacedPieceList;

    QList<VPSheet*> m_sheets;

    /**
     TODO : To be replaced by m_focusedSheet
     * @brief m_focusedPieceList pointer the the focused piece list, to which pieces will be
     * added via drag and drop, or if no piece list is defined.
     */
    VPPieceList *m_focusedPieceList{nullptr};


    VPSheet *m_focusedSheet{nullptr};

    // format
    Unit m_unit{Unit::Cm};

    bool m_warningSuperpositionOfPieces{false};
    bool m_warningPiecesOutOfBound{false};


};

#endif // VPLAYOUT_H
