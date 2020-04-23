/************************************************************************
 **
 **  @file   vpuzzlelayer.h
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
#ifndef VPUZZLELAYER_H
#define VPUZZLELAYER_H

#include <QList>
#include "vpuzzlepiece.h"

class VPuzzleLayer
{
public:
    VPuzzleLayer();
    ~VPuzzleLayer();

    QList<VPuzzlePiece *> GetPieces();
    void AddPiece(VPuzzlePiece *piece);
    void RemovePiece(VPuzzlePiece *piece);

    // here add some more function if we want to add/move a piece at a
    // certain position in the list

    QString GetName();
    void SetName(QString name);

    void SetIsVisible(bool value);
    bool GetIsVisible();

private:
    QString m_name;
    QList<VPuzzlePiece *> m_pieces;

    // control
    bool m_isVisible;

};

#endif // VPUZZLELAYER_H
