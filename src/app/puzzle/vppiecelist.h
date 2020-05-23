/************************************************************************
 **
 **  @file   vppiecelist.h
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
#ifndef VPPIECELIST_H
#define VPPIECELIST_H

#include <QList>
#include "vpuzzlepiece.h"

class VPuzzleLayout;

class VPPieceList : public QObject
{
    Q_OBJECT
public:
    VPPieceList(VPuzzleLayout *layout);
    ~VPPieceList();

    QList<VPuzzlePiece *> GetPieces();
    void AddPiece(VPuzzlePiece *piece);
    void RemovePiece(VPuzzlePiece *piece);

    // here add some more function if we want to add/move a piece at a
    // certain position in the list

    QString GetName() const;
    void SetName(const QString &name);

    void SetIsVisible(bool value);
    bool GetIsVisible() const;

    /**
     * @brief GetLayout Returns the layout in which this piece list is
     * @return the layout of this piece list
     */
    VPuzzleLayout* GetLayout();

    /**
     * @brief ClearSelection Clears the selection of the pieces in this piece list
     */
    void ClearSelection();

signals:
    /**
     * @brief PieceAdded The signal is emited when a piece was added
     */
    void PieceAdded(VPuzzlePiece *piece);

    /**
     * @brief PieceRemoved The signal is emited when a piece was removed
     */
    void PieceRemoved(VPuzzlePiece *piece);

private:
    Q_DISABLE_COPY(VPPieceList)

    QString m_name{};
    QList<VPuzzlePiece *> m_pieces{};

    VPuzzleLayout *m_layout{nullptr};

    // control
    bool m_isVisible{true};

};

#endif // VPPIECELIST_H
