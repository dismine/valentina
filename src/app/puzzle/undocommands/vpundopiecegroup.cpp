/************************************************************************
 **
 **  @file   vpundopiecegroup.cpp
 **  @author Yann Lossouarn <yann(at)lossouarn.net>
 **  @date   13 October, 2026
 **
 **  @brief  
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making 
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project 
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
#include "vpundopiecegroup.h"

#include "../layout/vplayout.h"
#include "../layout/vppiece.h"

//---------------------------------------------------------------------------------------------------------------------
VPUndoPieceGroup::VPUndoPieceGroup(const QList<VPPiecePtr> &pieces, const QString &group, QUndoCommand *parent)
  : VPUndoCommand(parent),
    m_group(group)
{
    setText(QObject::tr("change piece group"));
    for (const auto &piece : pieces)
    {
        if (not piece.isNull())
        {
            m_pieces.append(piece);
            m_oldGroups.append(piece->Group());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPUndoPieceGroup::undo()
{
    Apply(true);
}

//---------------------------------------------------------------------------------------------------------------------
void VPUndoPieceGroup::redo()
{
    Apply(false);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPieceGroup::id() const -> int
{
    return static_cast<int>(ML::UndoCommand::PieceGroup);
}

//---------------------------------------------------------------------------------------------------------------------
void VPUndoPieceGroup::Apply(bool back)
{
    VPPiecePtr last;
    for (int i = 0; i < m_pieces.size(); ++i)
    {
        VPPiecePtr const piece = m_pieces.at(i).toStrongRef();
        if (piece.isNull())
        {
            continue;
        }
        piece->SetGroup(back ? m_oldGroups.at(i) : m_group);
        last = piece;
    }
    // one refresh for the whole command: the handlers rebuild the carousel and the scene
    if (not last.isNull())
    {
        if (VPLayoutPtr const layout = last->Layout(); not layout.isNull())
        {
            emit layout->PieceGroupChanged(last);
        }
    }
}
