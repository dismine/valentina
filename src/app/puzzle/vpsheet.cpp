/************************************************************************
 **
 **  @file   vpsheet.cpp
 **  @author Ronan Le Tiec
 **  @date   23 5, 2020
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
#include "vpsheet.h"

#include "vppiecelist.h"
#include "vplayout.h"

//---------------------------------------------------------------------------------------------------------------------
VPSheet::VPSheet(VPLayout* layout) :
    m_layout(layout)
{
    m_pieceList = new VPPieceList(layout);
}

//---------------------------------------------------------------------------------------------------------------------
VPSheet::~VPSheet()
{
    delete m_pieceList;
}

//---------------------------------------------------------------------------------------------------------------------
VPPieceList* VPSheet::GetPieceList()
{
    return m_pieceList;
}

//---------------------------------------------------------------------------------------------------------------------
QString VPSheet::GetName() const
{
    return m_name;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetName(const QString &name)
{
    m_name = name;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetSize(qreal width, qreal height)
{
    m_size.setWidth(width);
    m_size.setHeight(height);
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetSizeConverted(qreal width, qreal height)
{
    m_size.setWidth(UnitConvertor(width, m_layout->GetUnit(), Unit::Px));
    m_size.setHeight(UnitConvertor(height, m_layout->GetUnit(), Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetSize(const QSizeF &size)
{
    m_size = size;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetSizeConverted(const QSizeF &size)
{
    m_size = QSizeF(
                UnitConvertor(size.width(), m_layout->GetUnit(), Unit::Px),
                UnitConvertor(size.height(), m_layout->GetUnit(), Unit::Px)
                );
}

//---------------------------------------------------------------------------------------------------------------------
QSizeF VPSheet::GetSheetSize() const
{
    return m_size;
}

//---------------------------------------------------------------------------------------------------------------------
QSizeF VPSheet::GetSheetSizeConverted() const
{
    QSizeF convertedSize = QSizeF(
                UnitConvertor(m_size.width(), Unit::Px, m_layout->GetUnit()),
                UnitConvertor(m_size.height(), Unit::Px, m_layout->GetUnit())
                );

    return convertedSize;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetMargins(qreal left, qreal top, qreal right, qreal bottom)
{
    m_margins.setLeft(left);
    m_margins.setTop(top);
    m_margins.setRight(right);
    m_margins.setBottom(bottom);
}
//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetMarginsConverted(qreal left, qreal top, qreal right, qreal bottom)
{
    m_margins.setLeft(UnitConvertor(left, m_layout->GetUnit(), Unit::Px));
    m_margins.setTop(UnitConvertor(top, m_layout->GetUnit(), Unit::Px));
    m_margins.setRight(UnitConvertor(right, m_layout->GetUnit(), Unit::Px));
    m_margins.setBottom(UnitConvertor(bottom, m_layout->GetUnit(), Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetMargins(const QMarginsF &margins)
{
    m_margins = margins;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetSheetMarginsConverted(const QMarginsF &margins)
{
    m_margins = UnitConvertor(margins, m_layout->GetUnit(), Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF VPSheet::GetSheetMargins() const
{
    return m_margins;
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF VPSheet::GetSheetMarginsConverted() const
{
    return UnitConvertor(m_margins, Unit::Px, m_layout->GetUnit());
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetFollowGrainline(FollowGrainline state)
{
    m_followGrainLine = state;
}

//---------------------------------------------------------------------------------------------------------------------
FollowGrainline VPSheet::GetFollowGrainline() const
{
    return m_followGrainLine;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetPiecesGap(qreal value)
{
    m_piecesGap = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetPiecesGapConverted(qreal value)
{
    m_piecesGap = UnitConvertor(value, m_layout->GetUnit(), Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPSheet::GetPiecesGap() const
{
    return m_piecesGap;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPSheet::GetPiecesGapConverted() const
{
    return UnitConvertor(m_piecesGap, Unit::Px, m_layout->GetUnit());
}


//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetStickyEdges(bool state)
{
    m_stickyEdges = state;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPSheet::GetStickyEdges() const
{
    return m_stickyEdges;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::ClearSelection()
{
    m_pieceList->ClearSelection();
}
