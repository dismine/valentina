/************************************************************************
 **
 **  @file   vpundopiecegroup.h
 **  @author Yann Lossouarn <yann(at)lossouarn.net>
 **  @date   13 October, 2026
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
#ifndef VPUNDOPIECEGROUP_H
#define VPUNDOPIECEGROUP_H

#include "../layout/layoutdef.h"
#include "vpundocommand.h"

#include <QList>
#include <QStringList>

// Spec 21.3: moves pieces to a group (or renames one: every member moved to the new name), undoable.
class VPUndoPieceGroup : public VPUndoCommand
{
    Q_OBJECT // NOLINT

public:
    VPUndoPieceGroup(const QList<VPPiecePtr> &pieces, const QString &group, QUndoCommand *parent = nullptr);
    ~VPUndoPieceGroup() override = default;

    void undo() override;
    void redo() override;
    auto id() const -> int override;

private:
    Q_DISABLE_COPY_MOVE(VPUndoPieceGroup) // NOLINT

    QList<VPPieceWeakPtr> m_pieces{};
    QStringList m_oldGroups{};
    QString m_group;

    void Apply(bool back);
};

#endif // VPUNDOPIECEGROUP_H
