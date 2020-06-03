/************************************************************************
 **
 **  @file   vpmimedatapiece.h
 **  @author Ronan Le Tiec
 **  @date   4 5, 2020
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

#ifndef VPMIMEDATAPIECE_H
#define VPMIMEDATAPIECE_H

#include <QMimeData>

#include "vppiece.h"

class VPMimeDataPiece : public QMimeData
{
    Q_OBJECT

public:

    VPMimeDataPiece();
    ~VPMimeDataPiece();

    /**
     * @brief GetPiecePtr Returns the piece pointer of the mime data
     * @return  piece pointer
     */
    VPPiece* GetPiecePtr() const;

    /**
     * @brief SetPiecePtr sets the piece pointer to the given value
     * @param piece the piece pointer
     */
    void SetPiecePtr(VPPiece* piece);

private:
    Q_DISABLE_COPY(VPMimeDataPiece)

    VPPiece *m_piece{nullptr};

};


#endif // VPMIMEDATAPIECE_H
