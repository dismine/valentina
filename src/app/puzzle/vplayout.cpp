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


#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pLayout, "p.layout")

//---------------------------------------------------------------------------------------------------------------------
VPLayout::VPLayout() :
    m_unplacedPieceList(new VPPieceList(this)),
    m_sheets(QList<VPSheet*>())
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
void VPLayout::SetTitle(QString title)
{
    m_title = title;
}

//---------------------------------------------------------------------------------------------------------------------
QString VPLayout::GetTitle() const
{
    return m_title;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::SetDescription(QString description)
{
    m_description = description;
}

//---------------------------------------------------------------------------------------------------------------------
QString VPLayout::GetDescription() const
{
    return m_description;
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
void VPLayout::ClearSelectionExceptForGivenPieceList(VPPieceList* pieceList)
{
    if(m_unplacedPieceList != pieceList)
    {
        m_unplacedPieceList->ClearSelection();
    }

    for (auto sheet : m_sheets)
    {
        if(sheet->GetPieceList() != pieceList)
        {
            sheet->ClearSelection();
        }
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


//---------------------------------------------------------------------------------------------------------------------
void VPLayout::SetTilesSize(qreal width, qreal height)
{
    m_tilesSize.setWidth(width);
    m_tilesSize.setHeight(height);
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::SetTilesSizeConverted(qreal width, qreal height)
{
    m_tilesSize.setWidth(UnitConvertor(width, GetUnit(), Unit::Px));
    m_tilesSize.setHeight(UnitConvertor(height, GetUnit(), Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::SetTilesSize(const QSizeF &size)
{
    m_tilesSize = size;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::SetTilesSizeConverted(const QSizeF &size)
{
    m_tilesSize = QSizeF(
                UnitConvertor(size.width(), GetUnit(), Unit::Px),
                UnitConvertor(size.height(), GetUnit(), Unit::Px)
                );
}

//---------------------------------------------------------------------------------------------------------------------
QSizeF VPLayout::GetTilesSize() const
{
    return m_tilesSize;
}

//---------------------------------------------------------------------------------------------------------------------
QSizeF VPLayout::GetTilesSize(Unit unit) const
{
    QSizeF convertedSize = QSizeF(
                UnitConvertor(m_tilesSize.width(), Unit::Px, unit),
                UnitConvertor(m_tilesSize.height(), Unit::Px, unit)
                );

    return convertedSize;
}

//---------------------------------------------------------------------------------------------------------------------
QSizeF VPLayout::GetTilesSizeConverted() const
{
    return GetTilesSize(GetUnit());
}

//---------------------------------------------------------------------------------------------------------------------
PageOrientation VPLayout::GetTilesOrientation()
{
    return m_tilesOrientation;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::SetTilesOrientation(PageOrientation orientation)
{
    if(orientation != m_tilesOrientation)
    {
        m_tilesOrientation = orientation;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::SetTilesMargins(qreal left, qreal top, qreal right, qreal bottom)
{
    m_tilesMargins.setLeft(left);
    m_tilesMargins.setTop(top);
    m_tilesMargins.setRight(right);
    m_tilesMargins.setBottom(bottom);
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::SetTilesMarginsConverted(qreal left, qreal top, qreal right, qreal bottom)
{
    m_tilesMargins.setLeft(UnitConvertor(left, GetUnit(), Unit::Px));
    m_tilesMargins.setTop(UnitConvertor(top, GetUnit(), Unit::Px));
    m_tilesMargins.setRight(UnitConvertor(right, GetUnit(), Unit::Px));
    m_tilesMargins.setBottom(UnitConvertor(bottom, GetUnit(), Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::SetTilesMargins(const QMarginsF &margins)
{
    m_tilesMargins = margins;
}


//---------------------------------------------------------------------------------------------------------------------
void VPLayout::SetTilesMarginsConverted(const QMarginsF &margins)
{
    m_tilesMargins = UnitConvertor(margins, GetUnit(), Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF VPLayout::GetTilesMargins() const
{
    return m_tilesMargins;
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF VPLayout::GetTilesMargins(Unit unit) const
{
    return UnitConvertor(m_tilesMargins, Unit::Px, unit);
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF VPLayout::GetTilesMarginsConverted() const
{
    return UnitConvertor(m_tilesMargins, Unit::Px, GetUnit());
}

//---------------------------------------------------------------------------------------------------------------------
bool VPLayout::GetShowTiles()
{
    return m_showTiles;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::SetShowTiles(bool value)
{
    m_showTiles = value;
}
