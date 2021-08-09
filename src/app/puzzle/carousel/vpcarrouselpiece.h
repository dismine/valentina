/************************************************************************
 **
 **  @file   vpcarrouselpiece.h
 **  @author Ronan Le Tiec
 **  @date   25 4, 2020
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
#ifndef VPCARROUSELPIECE_H
#define VPCARROUSELPIECE_H

#include <QMouseEvent>
#include <QListWidgetItem>

class VPPiece;

class VPCarrouselPiece : public QListWidgetItem
{
public:
    enum { Type = UserType + 1};

    explicit VPCarrouselPiece(VPPiece *piece, QListWidget* parent);
    virtual ~VPCarrouselPiece() = default;

    /**
     * @brief GetPiece Returns the corresponding layout piece
     * @return  the corresponding layout piece
     */
    auto GetPiece() -> VPPiece *;

    /**
     * @brief RefreshSelection refreshes the selection of the piece according to the selection information of m_piece
     */
    void RefreshSelection();

    /**
     * @brief CreatePieceIcon Creates an icon of the piece of given size
     * @param size of the icon
     * @return the created icon
     */
    auto CreatePieceIcon(const QSize &size, bool isDragIcon = false) const -> QIcon;


private:
    Q_DISABLE_COPY(VPCarrouselPiece)
    VPPiece *m_piece;
};

#endif // VPCARROUSELPIECE_H
