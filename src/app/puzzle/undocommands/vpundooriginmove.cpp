/************************************************************************
 **
 **  @file   vpundooriginmove.cpp
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
#include "vpundooriginmove.h"
#include "../vmisc/def.h"
#include "../layout/vpsheet.h"
#include "../layout/vplayout.h"

//---------------------------------------------------------------------------------------------------------------------
VPUndoOriginMove::VPUndoOriginMove(const VPSheetPtr &sheet, const VPTransformationOrigon &origin, bool allowMerge,
                                   QUndoCommand *parent)
    : VPUndoCommand(allowMerge, parent),
      m_sheet(sheet),
      m_origin(origin)
{
    SCASSERT(not sheet.isNull())

    m_oldOrigin = sheet->TransformationOrigin();

    setText(tr("move transformation origin"));
}

//---------------------------------------------------------------------------------------------------------------------
void VPUndoOriginMove::undo()
{
    VPSheetPtr sheet = m_sheet.toStrongRef();
    if (sheet.isNull())
    {
        return;
    }

    VPLayoutPtr layout = sheet->GetLayout();
    if (layout.isNull())
    {
        return;
    }

    if (layout->GetFocusedSheet() != sheet)
    {
        layout->SetFocusedSheet(sheet);
    }

    sheet->SetTransformationOrigin(m_oldOrigin);
    layout->TransformationOriginChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void VPUndoOriginMove::redo()
{
    VPSheetPtr sheet = m_sheet.toStrongRef();
    if (sheet.isNull())
    {
        return;
    }

    VPLayoutPtr layout = sheet->GetLayout();
    if (layout.isNull())
    {
        return;
    }

    if (layout->GetFocusedSheet() != sheet)
    {
        layout->SetFocusedSheet(sheet);
    }

    sheet->SetTransformationOrigin(m_origin);
    emit layout->TransformationOriginChanged();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoOriginMove::mergeWith(const QUndoCommand *command) -> bool
{
    if (command->id() != id()) // make sure other is also an VPUndoPieceMove command
    {
        return false;
    }

    const auto *moveCommand = dynamic_cast<const VPUndoOriginMove *>(command);
    SCASSERT(moveCommand != nullptr)

    VPSheetPtr sheet = Sheet();
    if (moveCommand->Sheet().isNull() || sheet.isNull() || not moveCommand->AllowMerge())
    {
        return false;
    }

    VPTransformationOrigon origin = moveCommand->Origin();

    if (origin.custom != m_origin.custom)
    {
        return false;
    }

    m_origin.origin += origin.origin;
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPUndoOriginMove::id() const -> int
{
    return static_cast<int>(ML::UndoCommand::MoveOrigin);
}
