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
#include "vppiece.h"
#include "vpsheet.h"


#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pLayout, "p.layout")

//---------------------------------------------------------------------------------------------------------------------
VPLayout::VPLayout(QObject *parent) :
    QObject(parent),
    m_trashSheet(new VPSheet(this))
{}

//---------------------------------------------------------------------------------------------------------------------
VPLayout::~VPLayout()
{
    qDeleteAll(m_pieces);
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::AddPiece(VPPiece *piece)
{
    if ((piece != nullptr) && not m_pieces.contains(piece))
    {
        piece->SetLayout(this);
        m_pieces.append(piece);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::GetPieces() const -> QList<VPPiece *>
{
    return m_pieces;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::GetUnplacedPieces() const -> QList<VPPiece *>
{
    return PiecesForSheet(nullptr);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::GetTrashedPieces() const -> QList<VPPiece *>
{
    return PiecesForSheet(m_trashSheet);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::AddSheet() -> VPSheet*
{
    auto *newSheet = new VPSheet(this);
    m_sheets.append(newSheet);
    return newSheet;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::AddSheet(VPSheet *sheet) -> VPSheet*
{
    if ((sheet != nullptr) && not m_sheets.contains(sheet))
    {
        sheet->setParent(this);
        m_sheets.append(sheet);
    }
    return sheet;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::GetSheets() -> QList<VPSheet *>
{
    return m_sheets;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayout::SetFocusedSheet(VPSheet *focusedSheet)
{
    if (m_sheets.isEmpty())
    {
        m_focusedSheet = nullptr;
    }
    else
    {
        m_focusedSheet = focusedSheet == nullptr ? m_sheets.first() : focusedSheet;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::GetFocusedSheet() -> VPSheet*
{
    return m_focusedSheet;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::GetTrashSheet() -> VPSheet*
{
    return m_trashSheet;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::LayoutSettings() -> VPLayoutSettings &
{
    return m_layoutSettings;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayout::PiecesForSheet(const VPSheet *sheet) const -> QList<VPPiece *>
{
    QList<VPPiece *> list;
    list.reserve(m_pieces.size());

    for (auto *piece : m_pieces)
    {
        if ((piece != nullptr) && piece->Sheet() == sheet)
        {
            list.append(piece);
        }
    }

    return list;
}
