/************************************************************************
 **
 **  @file   vpundopiecerotate.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 8, 2021
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
#include "vpundopiecerotate.h"
#include "../layout/vplayout.h"
#include "../layout/vppiece.h"

namespace
{
auto RoundAngle(qreal angle) -> qreal
{
    QLineF l(10, 10, 100, 10);
    l.setAngle(angle);
    return l.angle();
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VPUndoPieceRotate::VPUndoPieceRotate(const VPPiecePtr &piece, const VPTransformationOrigon &origin, qreal angle,
                                     qreal angleSum, bool allowMerge, QUndoCommand *parent)
  : VPUndoCommand(allowMerge, parent),
    m_piece(piece),
    m_origin(origin),
    m_angle(angle),
    m_angleSum(angleSum)
{
    SCASSERT(not piece.isNull())

    m_oldTransform = piece->GetMatrix();

    VPLayoutPtr layout = piece->Layout();
    if (not layout.isNull())
    {
        m_followGrainline = layout->LayoutSettings().GetFollowGrainline();
    }

    setText(tr("rotate piece"));
}

//---------------------------------------------------------------------------------------------------------------------
void VPUndoPieceRotate::undo()
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
void VPUndoPieceRotate::redo()
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

    if (m_firstCall)
    {
        if ((m_followGrainline || piece->IsFollowGrainline()) && piece->IsGrainlineEnabled())
        {
            piece->Rotate(m_origin.origin, m_angleSum);
        }
        else
        {
            piece->Rotate(m_origin.origin, m_angle);
        }
    }
    else
    {
        piece->Rotate(m_origin.origin, m_angle);
    }

    if (m_followGrainline || piece->IsFollowGrainline())
    {
        piece->RotateToGrainline(m_origin);
    }

    emit layout->PieceTransformationChanged(piece);

    if (m_firstCall)
    {
        m_firstCall = false;
    }

    emit layout->LayoutChanged();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPieceRotate::mergeWith(const QUndoCommand *command) -> bool
{
    if (command->id() != id()) // make sure other is also an VPUndoPieceMove command
    {
        return false;
    }

    const auto *moveCommand = dynamic_cast<const VPUndoPieceRotate *>(command);
    SCASSERT(moveCommand != nullptr)

    VPPiecePtr piece = Piece();
    if (not moveCommand->AllowMerge() || (moveCommand->Piece().isNull() || piece.isNull()) ||
        moveCommand->Piece() != piece || moveCommand->Origin() != m_origin ||
        moveCommand->FollowGrainline() != m_followGrainline)
    {
        return false;
    }

    m_angle += moveCommand->Angle();
    m_angle = RoundAngle(m_angle);
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPieceRotate::id() const -> int
{
    return static_cast<int>(ML::UndoCommand::RotatePiece);
}

// rotate pieces
//---------------------------------------------------------------------------------------------------------------------
VPUndoPiecesRotate::VPUndoPiecesRotate(const QList<VPPiecePtr> &pieces, const VPTransformationOrigon &origin,
                                       qreal angle, qreal angleSum, bool allowMerge, QUndoCommand *parent)
  : VPUndoCommand(allowMerge, parent),
    m_origin(origin),
    m_angle(angle),
    m_angleSum(angleSum)
{
    setText(QObject::tr("rotate pieces"));

    for (const auto &piece : pieces)
    {
        if (not piece.isNull())
        {
            m_pieces.append(piece);
            m_oldTransforms.insert(piece->GetUniqueID(), piece->GetMatrix());
        }
    }

    VPLayoutPtr layout = Layout();
    if (not layout.isNull())
    {
        m_followGrainline = layout->LayoutSettings().GetFollowGrainline();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPUndoPiecesRotate::undo()
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

    for (const auto &piece : qAsConst(m_pieces))
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
void VPUndoPiecesRotate::redo()
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

    for (const auto &piece : qAsConst(m_pieces))
    {
        VPPiecePtr p = piece.toStrongRef();
        if (not p.isNull())
        {
            if (m_firstCall)
            {
                if ((m_followGrainline || p->IsFollowGrainline()) && p->IsGrainlineEnabled())
                {
                    p->Rotate(m_origin.origin, m_angleSum);
                }
                else
                {
                    p->Rotate(m_origin.origin, m_angle);
                }
            }
            else
            {
                p->Rotate(m_origin.origin, m_angle);
            }

            if (m_followGrainline || p->IsFollowGrainline())
            {
                p->RotateToGrainline(m_origin);
            }

            emit layout->PieceTransformationChanged(p);
        }
    }

    if (m_firstCall)
    {
        m_firstCall = false;
    }

    emit layout->LayoutChanged();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPiecesRotate::mergeWith(const QUndoCommand *command) -> bool
{
    if (command->id() != id()) // make sure other is also an VPUndoPieceMove command
    {
        return false;
    }

    const auto *moveCommand = dynamic_cast<const VPUndoPiecesRotate *>(command);
    SCASSERT(moveCommand != nullptr)

    if (not moveCommand->AllowMerge() || moveCommand->PieceIds() != PieceIds() || moveCommand->Origin() != m_origin ||
        moveCommand->FollowGrainline() != m_followGrainline)
    {
        return false;
    }

    m_angle += moveCommand->Angle();
    m_angle = RoundAngle(m_angle);
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPiecesRotate::id() const -> int
{
    return static_cast<int>(ML::UndoCommand::RotatePieces);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPiecesRotate::PieceIds() const -> QSet<QString>
{
    QSet<QString> ids;
    for (const auto &piece : m_pieces)
    {
        VPPiecePtr p = piece.toStrongRef();
        if (not p.isNull())
        {
            ids.insert(p->GetUniqueID());
        }
    };

    return ids;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPiecesRotate::Layout() const -> VPLayoutPtr
{
    for (const auto &piece : m_pieces)
    {
        VPPiecePtr p = piece.toStrongRef();
        if (not p.isNull())
        {
            return p->Layout();
        }
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPiecesRotate::Sheet() const -> VPSheetPtr
{
    for (const auto &piece : m_pieces)
    {
        VPPiecePtr p = piece.toStrongRef();
        if (not p.isNull())
        {
            return p->Sheet();
        }
    }

    return {};
}
