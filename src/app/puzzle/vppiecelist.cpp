/************************************************************************
 **
 **  @file   vppiecelist.cpp
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
#include "vppiecelist.h"

#include "vplayout.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pPieceList, "p.pieceList")

//---------------------------------------------------------------------------------------------------------------------
VPPieceList::VPPieceList(VPLayout *layout, VPSheet *sheet):
    m_sheet(sheet),
    m_layout(layout)
{

}

//---------------------------------------------------------------------------------------------------------------------
VPPieceList::~VPPieceList()
{
    qDeleteAll(m_pieces);
}

//---------------------------------------------------------------------------------------------------------------------
VPLayout* VPPieceList::GetLayout()
{
    return m_layout;
}

//---------------------------------------------------------------------------------------------------------------------
VPSheet* VPPieceList::GetSheet()
{
    return m_sheet;
}

//---------------------------------------------------------------------------------------------------------------------
QList<VPPiece *> VPPieceList::GetPieces()
{
    return m_pieces;
}


//---------------------------------------------------------------------------------------------------------------------
void VPPieceList::ClearSelection()
{
    for (auto *piece: m_pieces)
    {
        piece->SetIsSelected(false);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPPieceList::AddPiece(VPPiece *piece)
{
    qCDebug(pPieceList(), "piece -- %s -- added to %s", qUtf8Printable(piece->GetName()), qUtf8Printable(this->GetName()));

    m_pieces.append(piece);
    piece->SetPieceList(this);

    emit PieceAdded(piece);
}

//---------------------------------------------------------------------------------------------------------------------
void VPPieceList::RemovePiece(VPPiece *piece)
{
    m_pieces.removeAll(piece);
    piece->SetPieceList(nullptr);

    emit PieceRemoved(piece);
}

//---------------------------------------------------------------------------------------------------------------------
QString VPPieceList::GetName() const
{
    return m_name;
}

//---------------------------------------------------------------------------------------------------------------------
void VPPieceList::SetName(const QString &name)
{
    m_name = name;
}

//---------------------------------------------------------------------------------------------------------------------
void VPPieceList::SetIsVisible(bool value)
{
    m_isVisible = value;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPPieceList::GetIsVisible() const
{
    return m_isVisible;
}
