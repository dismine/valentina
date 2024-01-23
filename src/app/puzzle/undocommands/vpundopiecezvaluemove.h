/************************************************************************
 **
 **  @file   vpundopiecezvaluemove.h
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
#ifndef VPUNDOPIECEZVALUEMOVE_H
#define VPUNDOPIECEZVALUEMOVE_H

#include "../layout/layoutdef.h"
#include "vpundocommand.h"

namespace ML
{
enum class ZValueMove
{
    Top,
    Up,
    Down,
    Bottom
};
}

class VPUndoPieceZValueMove : public VPUndoCommand
{
    Q_OBJECT // NOLINT

public:
    VPUndoPieceZValueMove(const VPPiecePtr &piece, ML::ZValueMove move, QUndoCommand *parent = nullptr);
    ~VPUndoPieceZValueMove() override = default;

    void undo() override;
    void redo() override;
    auto id() const -> int override;

private:
    Q_DISABLE_COPY_MOVE(VPUndoPieceZValueMove) // NOLINT

    VPPieceWeakPtr m_piece;
    ML::ZValueMove m_move;
    QHash<QString, qreal> m_oldValues{};

    auto Piece() const -> VPPiecePtr;
    auto Sheet() const -> VPSheetPtr;

    auto Levels(const QList<VPPiecePtr> &pieces, bool skip) const -> QList<QVector<QString>>;
    auto LevelStep(const QList<VPPiecePtr> &pieces) const -> qreal;
};

class VPUndoPiecesZValueMove : public VPUndoCommand
{
    Q_OBJECT // NOLINT

public:
    VPUndoPiecesZValueMove(const QList<VPPiecePtr> &pieces, ML::ZValueMove move, QUndoCommand *parent = nullptr);
    ~VPUndoPiecesZValueMove() override = default;

    void undo() override;
    void redo() override;
    auto id() const -> int override;

private:
    Q_DISABLE_COPY_MOVE(VPUndoPiecesZValueMove) // NOLINT

    QList<VPPieceWeakPtr> m_pieces{};
    ML::ZValueMove m_move;
    QHash<QString, qreal> m_oldValues{};

    auto Layout() const -> VPLayoutPtr;
    auto Sheet() const -> VPSheetPtr;

    auto PieceIds() const -> QVector<QString>;
    static auto Levels(const QList<VPPiecePtr> &allPieces, const QVector<QString> &skipPieces, bool skip)
        -> QList<QVector<QString>>;
    static auto LevelStep(const QList<VPPiecePtr> &pieces) -> qreal;
};

#endif // VPUNDOPIECEZVALUEMOVE_H
