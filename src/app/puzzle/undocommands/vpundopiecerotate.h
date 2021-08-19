/************************************************************************
 **
 **  @file   vpundopiecerotate.h
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
#ifndef VPUNDOPIECEROTATE_H
#define VPUNDOPIECEROTATE_H

#include "vpundocommand.h"

#include <QTransform>

#include "../layout/layoutdef.h"

class VPUndoPieceRotate : public VPUndoCommand
{
    Q_OBJECT
public:
    VPUndoPieceRotate(const VPPiecePtr &piece, const QPointF &origin, qreal angle, bool allowMerge,
                      QUndoCommand *parent = nullptr);

    virtual ~VPUndoPieceRotate()=default;

    virtual void undo() override;
    virtual void redo() override;
    // cppcheck-suppress unusedFunction
    virtual auto mergeWith(const QUndoCommand *command) -> bool override;
    virtual auto id() const -> int override ;

    auto Piece() const -> VPPiecePtr;
    auto Origin() const -> QPointF;
    auto Angle() const -> qreal;

private:
    Q_DISABLE_COPY(VPUndoPieceRotate)

    VPPieceWeakPtr m_piece;
    QTransform     m_oldTransform{};
    QPointF        m_origin;
    qreal          m_angle;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VPUndoPieceRotate::Piece() const -> VPPiecePtr
{
    return m_piece;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPUndoPieceRotate::Origin() const -> QPointF
{
    return m_origin;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPUndoPieceRotate::Angle() const -> qreal
{
    return m_angle;
}

// Rotate pieces
class VPUndoPiecesRotate : public VPUndoCommand
{
    Q_OBJECT
public:
    explicit VPUndoPiecesRotate(const QVector<VPPiecePtr> &pieces, const QPointF &origin, qreal angle, bool allowMerge,
                                QUndoCommand *parent = nullptr);
    virtual ~VPUndoPiecesRotate()=default;

    virtual void undo() override;
    virtual void redo() override;
    // cppcheck-suppress unusedFunction
    virtual auto mergeWith(const QUndoCommand *command) -> bool override;
    virtual auto id() const -> int override ;

    auto PieceIds() const -> QSet<QString>;
    auto Origin() const -> QPointF;
    auto Angle() const -> qreal;

private:
    Q_DISABLE_COPY(VPUndoPiecesRotate)

    QVector<VPPieceWeakPtr>   m_pieces{};
    QMap<QString, QTransform> m_oldTransforms{};
    QPointF                   m_origin;
    qreal                     m_angle;

    auto Layout() const -> VPLayoutPtr;
    auto Sheet() const -> VPSheetPtr;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VPUndoPiecesRotate::Origin() const -> QPointF
{
    return m_origin;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPUndoPiecesRotate::Angle() const -> qreal
{
    return m_angle;
}

#endif // VPUNDOPIECEROTATE_H
