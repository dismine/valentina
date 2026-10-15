/************************************************************************
 **
 **  @file   vpundooptimizeonline.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 October, 2026
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

#ifndef VPUNDOOPTIMIZEONLINE_H
#define VPUNDOOPTIMIZEONLINE_H

#include "../layout/layoutdef.h"
#include "vpundocommand.h"

#include <QList>
#include <QSizeF>
#include <QTransform>

struct VPNestedPiece
{
    VPPieceWeakPtr piece{};
    VPSheetWeakPtr oldSheet{};
    QTransform oldMatrix{};
    bool oldFlipped{false};
    bool oldHFlipped{false};
    QTransform newMatrix{};
    bool newFlipped{false};
    bool newHFlipped{false};
};

/** @brief One step for a whole online nesting result: every piece's sheet, matrix and flip, plus the sheet size. */
class VPUndoOptimizeOnline : public VPUndoCommand
{
    Q_OBJECT // NOLINT

public:
    VPUndoOptimizeOnline(const VPSheetPtr &sheet, const QList<VPNestedPiece> &pieces, const QSizeF &oldSheetSizePx,
                         const QSizeF &newSheetSizePx, QUndoCommand *parent = nullptr);
    ~VPUndoOptimizeOnline() override = default;

    void undo() override;
    void redo() override;
    auto id() const -> int override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VPUndoOptimizeOnline) // NOLINT

    VPSheetWeakPtr m_sheet;
    QList<VPNestedPiece> m_pieces;
    QSizeF m_oldSize;
    QSizeF m_newSize;

    void Apply(bool forward);
};

#endif // VPUNDOOPTIMIZEONLINE_H
