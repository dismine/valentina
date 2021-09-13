/************************************************************************
 **
 **  @file   vpundocommand.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 8, 2021
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
#ifndef VPUNDOCOMMAND_H
#define VPUNDOCOMMAND_H

#include <QObject>
#include <QUndoCommand>
#include <QLoggingCategory>

namespace ML
{
enum class UndoCommand: qint8
{
    MovePiece = 0,
    MovePieces = 1,
    RotatePiece = 2,
    RotatePieces = 3,
    MoveOrigin = 4,
    MoveOnSheet = 5,
    AddSheet = 6,
    RemoveSheet = 7,
};
}

Q_DECLARE_LOGGING_CATEGORY(vpUndo)

class VPUndoCommand : public QObject, public QUndoCommand
{
    Q_OBJECT
public:
    explicit VPUndoCommand(bool allowMerge = false, QUndoCommand *parent = nullptr);
    virtual ~VPUndoCommand() =default;

    auto AllowMerge() const -> bool;

protected:
    bool m_allowMerge;

private:
    Q_DISABLE_COPY(VPUndoCommand)
};


//---------------------------------------------------------------------------------------------------------------------
inline auto VPUndoCommand::AllowMerge() const -> bool
{
    return m_allowMerge;
}

#endif // VPUNDOCOMMAND_H
