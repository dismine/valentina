/************************************************************************
 **
 **  @file   vpundooriginmove.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   19 8, 2021
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
#ifndef VPUNDOORIGINMOVE_H
#define VPUNDOORIGINMOVE_H

#include "vpundocommand.h"

#include "../layout/layoutdef.h"

class VPUndoOriginMove : public VPUndoCommand
{
    Q_OBJECT // NOLINT
public:
    VPUndoOriginMove(const VPSheetPtr &sheet, const VPTransformationOrigon &origin, bool allowMerge = false,
                     QUndoCommand *parent = nullptr);
    ~VPUndoOriginMove() override =default;

    void undo() override;
    void redo() override;
    // cppcheck-suppress unusedFunction
    auto mergeWith(const QUndoCommand *command) -> bool override;
    auto id() const -> int override;

    auto Sheet() const -> VPSheetWeakPtr;
    auto Origin() const -> const VPTransformationOrigon &;

private:
    Q_DISABLE_COPY_MOVE(VPUndoOriginMove) // NOLINT

    VPSheetWeakPtr         m_sheet;
    VPTransformationOrigon m_oldOrigin{};
    VPTransformationOrigon m_origin{};

};

//---------------------------------------------------------------------------------------------------------------------
inline auto VPUndoOriginMove::Sheet() const -> VPSheetWeakPtr
{
    return m_sheet;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPUndoOriginMove::Origin() const ->const VPTransformationOrigon &
{
    return m_origin;
}

#endif // VPUNDOORIGINMOVE_H
