/************************************************************************
 **
 **  @file   vpcarrouselpiecelist.h
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

#ifndef VPCARROUSELPIECELIST_H
#define VPCARROUSELPIECELIST_H

#include <QWidget>
#include "vpuzzlelayer.h"
#include "vpiececarrouselpiece.h"

class VPCarrousel;

class VPCarrouselPieceList : public QWidget
{
    Q_OBJECT
public:
    VPCarrouselPieceList(VPuzzleLayer *layer, VPCarrousel *carrousel);
    ~VPCarrouselPieceList();

    void Init();
    void Refresh();

    /**
     * @brief Clear it clears the carrousel layer from its pieces
     */
    void Clear();

    QList<VPieceCarrouselPiece*> GetCarrouselPieces();

    VPCarrousel* GetCarrousel();

    /**
     * @brief GetPuzzleLayer Returns the corresponding VPuzzleLayer
     * @return the VPuzzleLayer
     */
    VPuzzleLayer* GetLayer();

private:
    Q_DISABLE_COPY(VPCarrouselPieceList)

    VPuzzleLayer *m_layer;
    VPCarrousel *m_carrousel;
    QList<VPieceCarrouselPiece*> m_carrouselPieces;

private slots:

    /**
     * @brief on_PieceUpdated This slot is called when a piece was added
     */
    void on_PieceAdded(VPuzzlePiece* piece);

    /**
     * @brief on_PieceUpdated This slot is called when a piece was removed
     */
    void on_PieceRemoved(VPuzzlePiece* piece);

};

#endif // VPCARROUSELPIECELIST_H
