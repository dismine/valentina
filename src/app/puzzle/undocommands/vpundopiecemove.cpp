/************************************************************************
 **
 **  @file   vpundopiecemove.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 8, 2021
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2021 Valentina project
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
#include "vpundopiecemove.h"
#include "../vmisc/def.h"
#include "../layout/vppiece.h"
#include "../layout/vplayout.h"

//---------------------------------------------------------------------------------------------------------------------
VPUndoPieceMove::VPUndoPieceMove(const VPPiecePtr &piece, qreal dx, qreal dy, bool allowMerge, QUndoCommand *parent)
    : VPUndoCommand(allowMerge, parent),
      m_piece(piece),
      m_dx(dx),
      m_dy(dy)
{
    SCASSERT(not piece.isNull())

    m_oldTransform = piece->GetMatrix();

    setText(tr("move piece"));
}

//---------------------------------------------------------------------------------------------------------------------
void VPUndoPieceMove::undo()
{
    VPPiecePtr piece = Piece();
    if (piece.isNull())
    {
        return;
    }

    VPLayoutPtr layout = piece->Layout();
    if (layout.isNull())
    {
        return;
    }

    if (layout->GetFocusedSheet() != piece->Sheet())
    {
        layout->SetFocusedSheet(piece->Sheet());
    }

    piece->SetMatrix(m_oldTransform);
    emit layout->PieceTransformationChanged(piece);
}

//---------------------------------------------------------------------------------------------------------------------
void VPUndoPieceMove::redo()
{
    VPPiecePtr piece = Piece();
    if (piece.isNull())
    {
        return;
    }

    VPLayoutPtr layout = piece->Layout();
    if (layout.isNull())
    {
        return;
    }

    if (layout->GetFocusedSheet() != piece->Sheet())
    {
        layout->SetFocusedSheet(piece->Sheet());
    }

    piece->Translate(m_dx, m_dy);
    emit layout->PieceTransformationChanged(piece);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPieceMove::mergeWith(const QUndoCommand *command) -> bool
{
    if (command->id() != id()) // make sure other is also an VPUndoPieceMove command
    {
        return false;
    }

    const auto *moveCommand = dynamic_cast<const VPUndoPieceMove *>(command);
    SCASSERT(moveCommand != nullptr)

    VPPiecePtr piece = Piece();
    if (moveCommand->Piece().isNull() || piece.isNull() || moveCommand->Piece() != piece ||
            not moveCommand->AllowMerge())
    {
        return false;
    }

    m_dx += moveCommand->Dx();
    m_dy += moveCommand->Dy();
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPieceMove::id() const -> int
{
    return static_cast<int>(ML::UndoCommand::MovePiece);
}

// move pieces
//---------------------------------------------------------------------------------------------------------------------
VPUndoPiecesMove::VPUndoPiecesMove(const QList<VPPiecePtr> &pieces, qreal dx, qreal dy, bool allowMerge,
                                   QUndoCommand *parent)
    : VPUndoCommand(allowMerge, parent),
      m_dx(dx),
      m_dy(dy)
{
    setText(QObject::tr("move pieces"));

    for (const auto& piece : pieces)
    {
        if (not piece.isNull())
        {
            m_pieces.append(piece);
            m_oldTransforms.insert(piece->GetUniqueID(), piece->GetMatrix());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPUndoPiecesMove::undo()
{
    if (m_pieces.isEmpty())
    {
        return;
    }

    VPLayoutPtr layout = Layout();
    if (layout.isNull())
    {
        return;
    }

    VPSheetPtr sheet = Sheet();
    if (layout->GetFocusedSheet() != sheet)
    {
        layout->SetFocusedSheet(sheet);
    }

    for (const auto& piece : m_pieces)
    {
        VPPiecePtr p = piece.toStrongRef();
        if (not p.isNull())
        {
            if (m_oldTransforms.contains(p->GetUniqueID()))
            {
                p->SetMatrix(m_oldTransforms.value(p->GetUniqueID()));
                emit layout->PieceTransformationChanged(p);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPUndoPiecesMove::redo()
{
    if (m_pieces.isEmpty())
    {
        return;
    }

    VPLayoutPtr layout = Layout();
    if (layout.isNull())
    {
        return;
    }

    VPSheetPtr sheet = Sheet();
    if (layout->GetFocusedSheet() != sheet)
    {
        layout->SetFocusedSheet(sheet);
    }

    for (const auto& piece : m_pieces)
    {
        VPPiecePtr p = piece.toStrongRef();
        if (not p.isNull())
        {
            p->Translate(m_dx, m_dy);
            emit layout->PieceTransformationChanged(p);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPiecesMove::mergeWith(const QUndoCommand *command) -> bool
{
    if (command->id() != id()) // make sure other is also an VPUndoPieceMove command
    {
        return false;
    }

    const auto *moveCommand = dynamic_cast<const VPUndoPiecesMove *>(command);
    SCASSERT(moveCommand != nullptr)

    if (moveCommand->PieceIds() != PieceIds())
    {
        return false;
    }

    if (not moveCommand->AllowMerge())
    {
        return false;
    }

    m_dx += moveCommand->Dx();
    m_dy += moveCommand->Dy();
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPiecesMove::id() const -> int
{
    return static_cast<int>(ML::UndoCommand::MovePieces);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPiecesMove::Layout() const -> VPLayoutPtr
{
    for (const auto& piece : m_pieces)
    {
        VPPiecePtr p = piece.toStrongRef();
        if (not p.isNull())
        {
            return p->Layout();
        }
    }

    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPiecesMove::Sheet() const  -> VPSheetPtr
{
    for (const auto& piece : m_pieces)
    {
        VPPiecePtr p = piece.toStrongRef();
        if (not p.isNull())
        {
            return p->Sheet();
        }
    }

    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPUndoPiecesMove::PieceIds() const -> QSet<QString>
{
    QSet<QString> ids;
    for (const auto& piece : m_pieces)
    {
        VPPiecePtr p = piece.toStrongRef();
        if (not p.isNull())
        {
            ids.insert(p->GetUniqueID());
        }
    };

    return ids;
}
