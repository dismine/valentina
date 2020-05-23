/************************************************************************
 **
 **  @file   vplayout.cpp
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
#include "vplayout.h"
#include "vppiecelist.h"
#include "vppiece.h"
#include "vpsheet.h"

//---------------------------------------------------------------------------------------------------------------------
VPLayout::VPLayout() :
    m_unplacedPieceList(new VPPieceList(this))
{
    m_unplacedPieceList->SetName(QObject::tr("Unplaced pieces"));
}

//---------------------------------------------------------------------------------------------------------------------
VPLayout::~VPLayout()
{
    qDeleteAll(m_sheets);
    delete m_unplacedPieceList;
}

//---------------------------------------------------------------------------------------------------------------------
VPPieceList* VPLayout::GetUnplacedPieceList()
{
    return m_unplacedPieceList;
}

//---------------------------------------------------------------------------------------------------------------------
VPSheet* VPLayout::AddSheet()
{
    VPSheet *newSheet = new VPSheet(this);
    m_sheets.append(newSheet);
    return newSheet;
}

//---------------------------------------------------------------------------------------------------------------------
VPSheet* VPLayout::AddSheet(VPSheet *sheet)
{
    m_sheets.append(sheet);
    return sheet;
}

//---------------------------------------------------------------------------------------------------------------------
QList<VPSheet *> VPLayout::GetSheets()
{
    return m_sheets;
}

//---------------------------------------------------------------------------------------------------------------------
QList<VPPiece *> VPLayout::GetSelectedPieces()
{
    QList<VPPiece *> result = QList<VPPiece *>();

    QList<VPPieceList *> pieceLists = QList<VPPieceList *>();
    pieceLists.append(m_unplacedPieceList);
    for (auto sheet : m_sheets)
    {
        pieceLists.append(sheet->GetPieceList());
    }

    for (auto pieceList : pieceLists)
    {
        for (auto piece : pieceList->GetPieces())
        {
            if(piece->GetIsSelected())
            {
                result.append(piece);
            }
        }
    }

    return result;
}


//---------------------------------------------------------------------------------------------------------------------
void VPLayout::SetUnit(Unit unit)
{
    m_unit = unit;
}

//---------------------------------------------------------------------------------------------------------------------
Unit VPLayout::GetUnit() const
{
    return m_unit;
}


//---------------------------------------------------------------------------------------------------------------------
void VPLayout::SetWarningSuperpositionOfPieces(bool state)
{
    m_warningSuperpositionOfPieces = state;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPLayout::GetWarningSuperpositionOfPieces() const
{
    return m_warningSuperpositionOfPieces;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::SetWarningPiecesOutOfBound(bool state)
{
    m_warningPiecesOutOfBound = state;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPLayout::GetWarningPiecesOutOfBound() const
{
    return m_warningPiecesOutOfBound;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::ClearSelection()
{
    m_unplacedPieceList->ClearSelection();

    for (auto sheet : m_sheets)
    {
        sheet->ClearSelection();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::MovePieceToPieceList(VPPiece* piece, VPPieceList* pieceList)
{
    VPPieceList* pieceListBefore = piece->GetPieceList();

    if(pieceListBefore != nullptr)
    {
        piece->GetPieceList()->RemovePiece(piece);
    }
    pieceList->AddPiece(piece);

    // signal, that a piece was moved
    emit PieceMovedToPieceList(piece, pieceListBefore,pieceList);
}


//---------------------------------------------------------------------------------------------------------------------
void VPLayout::SetFocusedSheet(VPSheet *focusedSheet)
{
    if(focusedSheet == nullptr)
    {
        m_focusedSheet = m_sheets.first();
    }
    else
    {
        m_focusedSheet = focusedSheet;
    }
}

//---------------------------------------------------------------------------------------------------------------------
VPSheet* VPLayout::GetFocusedSheet()
{
    return m_focusedSheet;
}
