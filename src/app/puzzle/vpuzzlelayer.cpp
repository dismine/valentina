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

#include "vpuzzlelayout.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pLayer, "p.layer")

//---------------------------------------------------------------------------------------------------------------------
VPuzzleLayer::VPuzzleLayer(VPuzzleLayout *layout):
    m_layout(layout)
{

}

//---------------------------------------------------------------------------------------------------------------------
VPuzzleLayer::~VPuzzleLayer()
{

}

//---------------------------------------------------------------------------------------------------------------------
VPuzzleLayout* VPuzzleLayer::GetLayout()
{
    return m_layout;
}

//---------------------------------------------------------------------------------------------------------------------
QList<VPuzzlePiece *> VPuzzleLayer::GetPieces()
{
    return m_pieces;
}


//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayer::ClearSelection()
{
    for (auto piece: m_pieces)
    {
        piece->SetIsSelected(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayer::AddPiece(VPuzzlePiece *piece)
{
    qCDebug(pLayer(), "piece -- %s -- added to %s", qUtf8Printable(piece->GetName()), qUtf8Printable(this->GetName()));

    m_pieces.append(piece);
    piece->SetLayer(this);

    emit PieceAdded(piece);
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayer::RemovePiece(VPuzzlePiece *piece)
{
    m_pieces.removeAll(piece);
    piece->SetLayer(nullptr);

    emit PieceRemoved(piece);
}

//---------------------------------------------------------------------------------------------------------------------
QString VPuzzleLayer::GetName() const
{
    return m_name;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayer::SetName(const QString &name)
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
