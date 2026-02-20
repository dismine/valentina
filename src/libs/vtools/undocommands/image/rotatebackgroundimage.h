/************************************************************************
 **
 **  @file   rotatebackgroundimage.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 1, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Valentina project
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
#ifndef ROTATEBACKGROUNDIMAGE_H
#define ROTATEBACKGROUNDIMAGE_H

#include "../vundocommand.h"

#include <QTransform>
#include <QUuid>

class RotateBackgroundImage : public VUndoCommand
{
    Q_OBJECT // NOLINT

public:
    RotateBackgroundImage(QUuid id, const QTransform &matrix, VAbstractPattern *doc, bool allowMerge = false,
                          QUndoCommand *parent = nullptr);
    ~RotateBackgroundImage() override = default;

    void undo() override;
    void redo() override;
    // cppcheck-suppress unusedFunction
    auto mergeWith(const QUndoCommand *command) -> bool override;
    auto id() const -> int override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(RotateBackgroundImage) // NOLINT

    QUuid m_id;
    QTransform m_matrix;
    QTransform m_oldMatrix{};
    bool m_allowMerge;
};

#endif // ROTATEBACKGROUNDIMAGE_H
