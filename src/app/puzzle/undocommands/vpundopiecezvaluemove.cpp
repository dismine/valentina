/************************************************************************
 **
 **  @file   vpundopiecezvaluemove.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 2, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Valentina project
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
#include "vpundopiecezvaluemove.h"
#include "../layout/vplayout.h"
#include "../layout/vppiece.h"
#include "../layout/vpsheet.h"
#include "undocommands/vpundocommand.h"

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto CorrectedZValues(const QList<QVector<QString>> &order) -> QHash<QString, qreal>
{
    QHash<QString, qreal> correctedZValues;

    qreal step = 0;
    if (not order.isEmpty())
    {
        step = 1.0 / static_cast<int>(order.size());
    }

    for (int i = 0; i < order.size(); ++i)
    {
        const QVector<QString> &level = order.at(i);
        for (const auto &pieceId : level)
        {
            correctedZValues.insert(pieceId, i * step);
        }
    }

    return correctedZValues;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VPUndoPieceZValueMove::VPUndoPieceZValueMove(const VPPiecePtr &piece, ML::ZValueMove move, QUndoCommand *parent)
  : VPUndoCommand(parent),
    m_piece(piece),
    m_move(move)
{
    setText(QObject::tr("z value move piece"));

    VPSheetPtr const sheet = Sheet();
    if (not sheet.isNull())
    {
        const QList<VPPiecePtr> pieces = sheet->GetPieces();
        for (const auto &p : pieces)
        {
            if (not p.isNull())
            {
                m_oldValues.insert(p->GetUniqueID(), p->ZValue());
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPUndoPieceZValueMove::undo()
{
    VPPiecePtr const piece = Piece();
    if (piece.isNull())
    {
        return;
    }

    VPLayoutPtr const layout = piece->Layout();
    if (layout.isNull())
    {
        return;
    }

    VPSheetPtr const sheet = Sheet();
    if (sheet.isNull())
    {
        return;
    }

    if (layout->GetFocusedSheet() != sheet)
    {
        layout->SetFocusedSheet(sheet);
    }

    const QList<VPPiecePtr> pieces = sheet->GetPieces();

    for (const auto &p : pieces)
    {
        if (not p.isNull() && m_oldValues.contains(p->GetUniqueID()))
        {
            p->SetZValue(m_oldValues.value(p->GetUniqueID()));
            emit layout->PieceZValueChanged(p);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPUndoPieceZValueMove::redo()
{
    VPPiecePtr const piece = Piece();
    if (piece.isNull())
    {
        return;
    }

    VPLayoutPtr const layout = piece->Layout();
    if (layout.isNull())
    {
        return;
    }

    VPSheetPtr const sheet = Sheet();
    if (sheet.isNull())
    {
        return;
    }

    if (layout->GetFocusedSheet() != sheet)
    {
        layout->SetFocusedSheet(sheet);
    }

    const QList<VPPiecePtr> pieces = sheet->GetPieces();

    QList<QVector<QString>> order;

    if (m_move == ML::ZValueMove::Top)
    {
        order = Levels(pieces, true);
        order.append({piece->GetUniqueID()});
    }
    else if (m_move == ML::ZValueMove::Up)
    {
        const qreal step = LevelStep(pieces);
        for (const auto &p : pieces)
        {
            if (p->GetUniqueID() != piece->GetUniqueID())
            {
                p->SetZValue(p->ZValue() - step);
            }
        }

        order = Levels(pieces, false);
    }
    else if (m_move == ML::ZValueMove::Down)
    {
        const qreal step = LevelStep(pieces);
        for (const auto &p : pieces)
        {
            if (p->GetUniqueID() != piece->GetUniqueID())
            {
                p->SetZValue(p->ZValue() + step);
            }
        }

        order = Levels(pieces, false);
    }
    else if (m_move == ML::ZValueMove::Bottom)
    {
        order = Levels(pieces, true);
        order.prepend({piece->GetUniqueID()});
    }

    QHash<QString, qreal> const correctedZValues = CorrectedZValues(order);
    for (const auto &p : pieces)
    {
        if (not p.isNull())
        {
            p->SetZValue(correctedZValues.value(p->GetUniqueID(), p->ZValue()));
            emit layout->PieceZValueChanged(p);
        }
    }

    emit layout->LayoutChanged();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPieceZValueMove::id() const -> int
{
    return static_cast<int>(ML::UndoCommand::ZValueMovePiece);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPieceZValueMove::Piece() const -> VPPiecePtr
{
    return m_piece;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPieceZValueMove::Sheet() const -> VPSheetPtr
{
    if (VPPiecePtr const p = Piece(); not p.isNull())
    {
        return p->Sheet();
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPieceZValueMove::Levels(const QList<VPPiecePtr> &pieces, bool skip) const -> QList<QVector<QString>>
{
    VPPiecePtr const piece = Piece();
    if (piece.isNull())
    {
        return {};
    }

    QMap<qreal, QVector<QString>> levels;

    for (const auto &p : pieces)
    {
        if (p.isNull() || (skip && p->GetUniqueID() == piece->GetUniqueID()))
        {
            continue;
        }

        if (levels.contains(p->ZValue()))
        {
            QVector<QString> level_images = levels.value(p->ZValue());
            level_images.append(p->GetUniqueID());
            levels[p->ZValue()] = level_images;
        }
        else
        {
            levels[p->ZValue()] = {p->GetUniqueID()};
        }
    }

    return levels.values(); // clazy:exclude=clazy-qt6-deprecated-api-fixes
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPieceZValueMove::LevelStep(const QList<VPPiecePtr> &pieces) const -> qreal
{
    QList<QVector<QString>> const levels = Levels(pieces, false);
    if (levels.isEmpty())
    {
        return 0;
    }

    return 1.0 / static_cast<int>(levels.size());
}

//---------------------------------------------------------------------------------------------------------------------
// Z value pieces
VPUndoPiecesZValueMove::VPUndoPiecesZValueMove(const QList<VPPiecePtr> &pieces, ML::ZValueMove move,
                                               QUndoCommand *parent)
  : VPUndoCommand(parent),
    m_move(move)
{
    setText(QObject::tr("z value move pieces"));

    m_pieces.reserve(pieces.size());
    for (const auto &p : pieces)
    {
        m_pieces.append(p);
    }

    VPSheetPtr const sheet = Sheet();
    if (not sheet.isNull())
    {
        const QList<VPPiecePtr> pieces = sheet->GetPieces();
        for (const auto &p : pieces)
        {
            if (not p.isNull())
            {
                m_oldValues.insert(p->GetUniqueID(), p->ZValue());
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPUndoPiecesZValueMove::undo()
{
    if (m_pieces.isEmpty())
    {
        return;
    }

    VPLayoutPtr const layout = Layout();
    if (layout.isNull())
    {
        return;
    }

    VPSheetPtr const sheet = Sheet();
    if (sheet.isNull())
    {
        return;
    }

    if (layout->GetFocusedSheet() != sheet)
    {
        layout->SetFocusedSheet(sheet);
    }

    const QList<VPPiecePtr> pieces = sheet->GetPieces();

    for (const auto &p : pieces)
    {
        if (not p.isNull() && m_oldValues.contains(p->GetUniqueID()))
        {
            p->SetZValue(m_oldValues.value(p->GetUniqueID()));
            emit layout->PieceZValueChanged(p);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPUndoPiecesZValueMove::redo()
{
    if (m_pieces.isEmpty())
    {
        return;
    }

    VPLayoutPtr const layout = Layout();
    if (layout.isNull())
    {
        return;
    }

    VPSheetPtr const sheet = Sheet();
    if (sheet.isNull())
    {
        return;
    }

    if (layout->GetFocusedSheet() != sheet)
    {
        layout->SetFocusedSheet(sheet);
    }

    const QList<VPPiecePtr> allPieces = sheet->GetPieces();
    QVector<QString> const ids = PieceIds();

    QList<QVector<QString>> order;

    if (m_move == ML::ZValueMove::Top)
    {
        order = Levels(allPieces, ids, true);
        order.append(ids);
    }
    else if (m_move == ML::ZValueMove::Up)
    {
        const qreal step = LevelStep(allPieces);
        for (const auto &p : allPieces)
        {
            if (not ids.contains(p->GetUniqueID()))
            {
                p->SetZValue(p->ZValue() - step);
            }
        }

        order = Levels(allPieces, ids, false);
    }
    else if (m_move == ML::ZValueMove::Down)
    {
        const qreal step = LevelStep(allPieces);
        for (const auto &p : allPieces)
        {
            if (not ids.contains(p->GetUniqueID()))
            {
                p->SetZValue(p->ZValue() + step);
            }
        }

        order = Levels(allPieces, ids, false);
    }
    else if (m_move == ML::ZValueMove::Bottom)
    {
        order = Levels(allPieces, ids, true);
        order.prepend(ids);
    }

    QHash<QString, qreal> const correctedZValues = CorrectedZValues(order);
    for (const auto &p : allPieces)
    {
        if (not p.isNull())
        {
            p->SetZValue(correctedZValues.value(p->GetUniqueID(), p->ZValue()));
            emit layout->PieceZValueChanged(p);
        }
    }

    emit layout->LayoutChanged();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPiecesZValueMove::id() const -> int
{
    return static_cast<int>(ML::UndoCommand::ZValueMovePieces);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPiecesZValueMove::Layout() const -> VPLayoutPtr
{
    for (const auto &piece : m_pieces)
    {
        VPPiecePtr const p = piece.toStrongRef();
        if (not p.isNull())
        {
            return p->Layout();
        }
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPiecesZValueMove::Sheet() const -> VPSheetPtr
{
    for (const auto &piece : m_pieces)
    {
        VPPiecePtr const p = piece.toStrongRef();
        if (not p.isNull())
        {
            return p->Sheet();
        }
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPiecesZValueMove::PieceIds() const -> QVector<QString>
{
    QVector<QString> ids;
    ids.reserve(m_pieces.size());

    for (const auto &piece : m_pieces)
    {
        VPPiecePtr const p = piece.toStrongRef();
        if (not p.isNull())
        {
            ids.append(p->GetUniqueID());
        }
    }

    return ids;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPiecesZValueMove::Levels(const QList<VPPiecePtr> &allPieces, const QVector<QString> &skipPieces, bool skip)
    -> QList<QVector<QString>>
{
    QMap<qreal, QVector<QString>> levels;

    for (const auto &p : allPieces)
    {
        if (p.isNull() || (skip && skipPieces.contains(p->GetUniqueID())))
        {
            continue;
        }

        if (levels.contains(p->ZValue()))
        {
            QVector<QString> level_images = levels.value(p->ZValue());
            level_images.append(p->GetUniqueID());
            levels[p->ZValue()] = level_images;
        }
        else
        {
            levels[p->ZValue()] = {p->GetUniqueID()};
        }
    }

    return levels.values(); // clazy:exclude=clazy-qt6-deprecated-api-fixes
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoPiecesZValueMove::LevelStep(const QList<VPPiecePtr> &pieces) -> qreal
{
    QList<QVector<QString>> const levels = Levels(pieces, QVector<QString>(), false);
    if (levels.isEmpty())
    {
        return 0;
    }

    return 1.0 / static_cast<int>(levels.size());
}
