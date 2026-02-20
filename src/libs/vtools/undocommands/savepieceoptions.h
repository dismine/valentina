/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 11, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#ifndef SAVEPIECEOPTIONS_H
#define SAVEPIECEOPTIONS_H

#include <QtGlobal>

#include "../vpatterndb/vpiece.h"
#include "vundocommand.h"

class SavePieceOptions : public VUndoCommand
{
    Q_OBJECT // NOLINT
public:
    SavePieceOptions(VPiece oldDet, VPiece newDet, VAbstractPattern *doc, quint32 id, QUndoCommand *parent = nullptr);
    ~SavePieceOptions() override = default;

    void undo() override;
    void redo() override;
    auto mergeWith(const QUndoCommand *command) -> bool override;
    auto id() const -> int override;
signals:
    void UpdateGroups();
private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(SavePieceOptions) // NOLINT

    const VPiece m_oldDet;
    VPiece m_newDet;
};

#endif // SAVEPIECEOPTIONS_H
