/************************************************************************
 **
 **  @file   vpundooptimizeonline.cpp
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

#include "vpundooptimizeonline.h"
#include "../layout/vplayout.h"
#include "../layout/vppiece.h"
#include "../layout/vpsheet.h"

//---------------------------------------------------------------------------------------------------------------------
VPUndoOptimizeOnline::VPUndoOptimizeOnline(const VPSheetPtr &sheet, const QList<VPNestedPiece> &pieces,
                                           const QSizeF &oldSheetSizePx, const QSizeF &newSheetSizePx,
                                           QUndoCommand *parent)
  : VPUndoCommand(false, parent),
    m_sheet(sheet),
    m_pieces(pieces),
    m_oldSize(oldSheetSizePx),
    m_newSize(newSheetSizePx)
{
    setText(tr("optimise placement online"));
}

//---------------------------------------------------------------------------------------------------------------------
void VPUndoOptimizeOnline::undo()
{
    Apply(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VPUndoOptimizeOnline::redo()
{
    Apply(true);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoOptimizeOnline::id() const -> int
{
    return static_cast<int>(ML::UndoCommand::OptimizeOnline);
}

//---------------------------------------------------------------------------------------------------------------------
void VPUndoOptimizeOnline::Apply(bool forward)
{
    VPSheetPtr const sheet = m_sheet.toStrongRef();
    if (sheet.isNull())
    {
        return;
    }
    VPLayoutPtr const layout = sheet->GetLayout();
    if (layout.isNull())
    {
        return;
    }
    if (layout->GetFocusedSheet() != sheet)
    {
        layout->SetFocusedSheet(sheet);
    }
    sheet->SetSheetSize(forward ? m_newSize : m_oldSize);

    for (const auto &entry : std::as_const(m_pieces))
    {
        VPPiecePtr const piece = entry.piece.toStrongRef();
        if (piece.isNull())
        {
            continue;
        }
        VPSheetPtr const target = forward ? sheet : entry.oldSheet.toStrongRef();
        const bool sheetChanged = piece->Sheet() != target;
        piece->SetSheet(target);
        piece->SetMatrix(forward ? entry.newMatrix : entry.oldMatrix);
        piece->SetVerticallyFlipped(forward ? entry.newFlipped : entry.oldFlipped);
        piece->SetHorizontallyFlipped(forward ? entry.newHFlipped : entry.oldHFlipped);
        piece->SetSelected(false);
        if (sheetChanged)
        {
            emit layout->PieceSheetChanged(piece);
        }
        emit layout->PieceTransformationChanged(piece);
    }
    // the main window refreshes the scene, the tiles and the sheet property tab on this signal
    emit layout->ActiveSheetChanged(sheet);
    emit layout->LayoutChanged();
}
