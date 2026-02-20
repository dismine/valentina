/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 11, 2016
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

#ifndef SAVEPIECEPATHOPTIONS_H
#define SAVEPIECEPATHOPTIONS_H

#include <QtGlobal>

#include "../vpatterndb/vpiecepath.h"
#include "vundocommand.h"

class SavePiecePathOptions : public VUndoCommand
{
    Q_OBJECT // NOLINT
public:
    SavePiecePathOptions(quint32 pieceId,
                         VPiecePath oldPath,
                         VPiecePath newPath,
                         VAbstractPattern *doc,
                         VContainer *data,
                         quint32 id,
                         QUndoCommand *parent = nullptr);
    ~SavePiecePathOptions() override = default;

    void undo() override;
    void redo() override;
    auto mergeWith(const QUndoCommand *command) -> bool override;
    auto id() const -> int override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(SavePiecePathOptions) // NOLINT

    const VPiecePath m_oldPath;
    VPiecePath       m_newPath;

    VContainer *m_data;
    quint32 m_pieceId;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto SavePiecePathOptions::id() const -> int
{
    return static_cast<int>(UndoCommand::SavePiecePathOptions);
}

#endif // SAVEPIECEPATHOPTIONS_H
