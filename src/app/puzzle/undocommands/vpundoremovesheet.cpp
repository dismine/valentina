/************************************************************************
 **
 **  @file   vpundoremovesheet.cpp
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
#include "vpundoremovesheet.h"
#include "../vmisc/def.h"
#include "../layout/vpsheet.h"
#include "../layout/vppiece.h"
#include "../layout/vplayout.h"

//---------------------------------------------------------------------------------------------------------------------
VPUndoRemoveSheet::VPUndoRemoveSheet(const VPSheetPtr &sheet, QUndoCommand *parent)
    : VPUndoCommand(false, parent),
      m_sheet(sheet)
{
    SCASSERT(not sheet.isNull())

    QList<VPPiecePtr> pieces = sheet->GetPieces();
    for (auto piece : pieces)
    {
        m_pieces.append(piece);
    }

    setText(tr("add sheet"));
}

//---------------------------------------------------------------------------------------------------------------------
void VPUndoRemoveSheet::undo()
{
    VPSheetPtr sheet = m_sheet.toStrongRef();
    if (sheet.isNull())
    {
        return;
    }

    if (not sheet.isNull())
    {
        sheet->SetVisible(true);

        for (const auto &piece : m_pieces)
        {
            VPPiecePtr p = piece.toStrongRef();
            if (not p.isNull())
            {
                p->SetSheet(sheet);
            }
        }

        VPLayoutPtr layout = sheet->GetLayout();
        if (layout.isNull())
        {
            return;
        }

        emit layout->SheetListChanged();
        layout->SetFocusedSheet(sheet);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPUndoRemoveSheet::redo()
{
    VPSheetPtr sheet = m_sheet.toStrongRef();
    if (sheet.isNull())
    {
        return;
    }

    if (not sheet.isNull())
    {
        sheet->SetVisible(false);

        for (const auto &piece : m_pieces)
        {
            VPPiecePtr p = piece.toStrongRef();
            if (not p.isNull())
            {
                p->SetSheet(VPSheetPtr());
            }
        }
    }

    VPLayoutPtr layout = sheet->GetLayout();
    if (layout.isNull())
    {
        return;
    }

    emit layout->SheetListChanged();
    layout->SetFocusedSheet(VPSheetPtr());
    emit layout->LayoutChanged();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoRemoveSheet::id() const -> int
{
    return static_cast<int>(ML::UndoCommand::RemoveSheet);
}
