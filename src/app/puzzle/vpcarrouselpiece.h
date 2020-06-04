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

#include "vppiece.h"


class VPCarrouselPiece : public QListWidgetItem
{
public:
    explicit VPCarrouselPiece(VPPiece *piece, QListWidget* parent);
    ~VPCarrouselPiece();

    /**
     * @brief GetPiece Returns the corresponding layout piece
     * @return  the corresponding layout piece
     */
    VPPiece * GetPiece();

    /**
     * @brief RefreshSelection refreshes the selection of the piece according to the selection information of m_piece
     */
    void RefreshSelection();

    /**
     * @brief CreatePieceIcon Creates an icon of the piece of given size
     * @param size of the icon
     * @return the created icon
     */
    QIcon CreatePieceIcon(const QSize &size) const;


private:
    Q_DISABLE_COPY(VPCarrouselPiece)
    VPPiece *m_piece;

private slots:

};

#endif // VPCARROUSELPIECE_H
