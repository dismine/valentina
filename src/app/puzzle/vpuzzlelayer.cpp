/************************************************************************
 **
 **  @file   vpuzzlelayer.cpp
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
#include "vpuzzlelayer.h"

//---------------------------------------------------------------------------------------------------------------------
VPuzzleLayer::VPuzzleLayer() :
    m_name(QString("")),
    m_pieces(QList<VPuzzlePiece*>()),
    m_isVisible(true)
{

}

//---------------------------------------------------------------------------------------------------------------------
VPuzzleLayer::~VPuzzleLayer()
{

}

//---------------------------------------------------------------------------------------------------------------------
QList<VPuzzlePiece *> VPuzzleLayer::GetPieces()
{
    return m_pieces;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayer::AddPiece(VPuzzlePiece *piece)
{
    m_pieces.append(piece);
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayer::RemovePiece(VPuzzlePiece *piece)
{
    m_pieces.removeAll(piece);
}

//---------------------------------------------------------------------------------------------------------------------
QString VPuzzleLayer::GetName() const
{
    return m_name;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayer::SetName(QString name)
{
    m_name = name;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayer::SetIsVisible(bool value)
{
    m_isVisible = value;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPuzzleLayer::GetIsVisible() const
{
    return m_isVisible;
}
