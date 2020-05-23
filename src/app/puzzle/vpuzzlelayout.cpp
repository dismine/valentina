/************************************************************************
 **
 **  @file   vpuzzlelayout.cpp
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
#include "vpuzzlelayout.h"
#include "vppiecelist.h"
#include "vpuzzlepiece.h"

//---------------------------------------------------------------------------------------------------------------------
VPuzzleLayout::VPuzzleLayout() :
    m_unplacedPieceList(new VPPieceList(this))
{
    m_unplacedPieceList->SetName(QObject::tr("Unplaced pieces"));

    // create a standard default piecelist:
    VPPieceList *pieceList = new VPPieceList(this);
    pieceList->SetName(QObject::tr("Layout"));
    AddPieceList(pieceList);

    // sets the default active piece list
    SetFocusedPieceList();
}

//---------------------------------------------------------------------------------------------------------------------
VPuzzleLayout::~VPuzzleLayout()
{
    qDeleteAll(m_pieceLists);
    delete m_unplacedPieceList;
}

//---------------------------------------------------------------------------------------------------------------------
VPPieceList* VPuzzleLayout::GetUnplacedPieceList()
{
    return m_unplacedPieceList;
}

//---------------------------------------------------------------------------------------------------------------------
VPPieceList* VPuzzleLayout::AddPieceList()
{
    VPPieceList *newPieceList = new VPPieceList(this);
    m_pieceLists.append(newPieceList);
    return newPieceList;
}

//---------------------------------------------------------------------------------------------------------------------
VPPieceList* VPuzzleLayout::AddPieceList(VPPieceList *pieceList)
{
    m_pieceLists.append(pieceList);
    return pieceList;
}

//---------------------------------------------------------------------------------------------------------------------
QList<VPPieceList *> VPuzzleLayout::GetPiecesLists()
{
    return m_pieceLists;
}

//---------------------------------------------------------------------------------------------------------------------
QList<VPuzzlePiece *> VPuzzleLayout::GetSelectedPieces()
{
    QList<VPuzzlePiece *> result = QList<VPuzzlePiece *>();

    QList<VPPieceList *> pieceLists = m_pieceLists;
    pieceLists.prepend(m_unplacedPieceList);

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
void VPuzzleLayout::SetUnit(Unit unit)
{
    m_unit = unit;
}

//---------------------------------------------------------------------------------------------------------------------
Unit VPuzzleLayout::GetUnit() const
{
    return m_unit;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetLayoutSize(qreal width, qreal height)
{
    m_size.setWidth(width);
    m_size.setHeight(height);
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetLayoutSizeConverted(qreal width, qreal height)
{
    m_size.setWidth(UnitConvertor(width, m_unit, Unit::Px));
    m_size.setHeight(UnitConvertor(height, m_unit, Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetLayoutSize(const QSizeF &size)
{
    m_size = size;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetLayoutSizeConverted(const QSizeF &size)
{
    m_size = QSizeF(
                UnitConvertor(size.width(), m_unit, Unit::Px),
                UnitConvertor(size.height(), m_unit, Unit::Px)
                );
}

//---------------------------------------------------------------------------------------------------------------------
QSizeF VPuzzleLayout::GetLayoutSize() const
{
    return m_size;
}

//---------------------------------------------------------------------------------------------------------------------
QSizeF VPuzzleLayout::GetLayoutSizeConverted() const
{
    QSizeF convertedSize = QSizeF(
                UnitConvertor(m_size.width(), Unit::Px, m_unit),
                UnitConvertor(m_size.height(), Unit::Px, m_unit)
                );

    return convertedSize;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetLayoutMargins(qreal left, qreal top, qreal right, qreal bottom)
{
    m_margins.setLeft(left);
    m_margins.setTop(top);
    m_margins.setRight(right);
    m_margins.setBottom(bottom);
}
//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetLayoutMarginsConverted(qreal left, qreal top, qreal right, qreal bottom)
{
    m_margins.setLeft(UnitConvertor(left, m_unit, Unit::Px));
    m_margins.setTop(UnitConvertor(top, m_unit, Unit::Px));
    m_margins.setRight(UnitConvertor(right, m_unit, Unit::Px));
    m_margins.setBottom(UnitConvertor(bottom, m_unit, Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetLayoutMargins(const QMarginsF &margins)
{
    m_margins = margins;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetLayoutMarginsConverted(const QMarginsF &margins)
{
    m_margins = UnitConvertor(margins, m_unit, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF VPuzzleLayout::GetLayoutMargins() const
{
    return m_margins;
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF VPuzzleLayout::GetLayoutMarginsConverted() const
{
    return UnitConvertor(m_margins, Unit::Px, m_unit);
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetFollowGrainline(FollowGrainline state)
{
    m_followGrainLine = state;
}

//---------------------------------------------------------------------------------------------------------------------
FollowGrainline VPuzzleLayout::GetFollowGrainline() const
{
    return m_followGrainLine;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetPiecesGap(qreal value)
{
    m_piecesGap = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetPiecesGapConverted(qreal value)
{
    m_piecesGap = UnitConvertor(value, m_unit, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPuzzleLayout::GetPiecesGap() const
{
    return m_piecesGap;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPuzzleLayout::GetPiecesGapConverted() const
{
    return UnitConvertor(m_piecesGap, Unit::Px, m_unit);
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetWarningSuperpositionOfPieces(bool state)
{
    m_warningSuperpositionOfPieces = state;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPuzzleLayout::GetWarningSuperpositionOfPieces() const
{
    return m_warningSuperpositionOfPieces;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetWarningPiecesOutOfBound(bool state)
{
    m_warningPiecesOutOfBound = state;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPuzzleLayout::GetWarningPiecesOutOfBound() const
{
    return m_warningPiecesOutOfBound;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetStickyEdges(bool state)
{
    m_stickyEdges = state;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPuzzleLayout::GetStickyEdges() const
{
    return m_stickyEdges;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::ClearSelection()
{
    m_unplacedPieceList->ClearSelection();

    for (auto pieceList : m_pieceLists)
    {
        pieceList->ClearSelection();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetFocusedPieceList(VPPieceList* focusedPieceList)
{
    if(focusedPieceList == nullptr)
    {
        m_focusedPieceList = m_pieceLists.first();
    }
    else
    {
        m_focusedPieceList = focusedPieceList;
    }
}

//---------------------------------------------------------------------------------------------------------------------
VPPieceList* VPuzzleLayout::GetFocusedPieceList()
{
    return m_focusedPieceList;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::MovePieceToPieceList(VPuzzlePiece* piece, VPPieceList* pieceList)
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
