/************************************************************************
 **
 **  @file   movetool.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 1, 2026
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
#ifndef MOVETOOL_H
#define MOVETOOL_H

#include "../vmisc/typedef.h"
#include "vundocommand.h"

class MoveToolUp : public VUndoCommand
{
    Q_OBJECT // NOLINT

public:
    MoveToolUp(VAbstractPattern *doc, vidtype currentId, vidtype topId, QUndoCommand *parent = nullptr);
    ~MoveToolUp() override = default;

    void undo() override;
    void redo() override;

    auto mergeWith(const QUndoCommand *command) -> bool override;
    auto id() const -> int override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(MoveToolUp) // NOLINT

    vidtype m_currentId;
    vidtype m_topId;
    vidtype m_originalPreviousId{NULL_ID}; // ID of element that was before m_currentId
};

class MoveToolDown : public VUndoCommand
{
    Q_OBJECT // NOLINT

public:
    MoveToolDown(VAbstractPattern *doc, vidtype currentId, vidtype bottomId, QUndoCommand *parent = nullptr);
    ~MoveToolDown() override = default;

    void undo() override;
    void redo() override;

    auto mergeWith(const QUndoCommand *command) -> bool override;
    auto id() const -> int override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(MoveToolDown) // NOLINT
    vidtype m_currentId;
    vidtype m_bottomId;
    vidtype m_originalNextId{NULL_ID}; // ID of element that was after m_currentId
};

#endif // MOVETOOL_H
