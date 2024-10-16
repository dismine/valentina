/************************************************************************
 **
 **  @file   opaquebackgroundimage.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 2, 2022
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
#ifndef OPAQUEBACKGROUNDIMAGE_H
#define OPAQUEBACKGROUNDIMAGE_H

#include "../vundocommand.h"

class OpaqueBackgroundImage : public VUndoCommand
{
    Q_OBJECT // NOLINT

public:
    OpaqueBackgroundImage(QUuid id, qreal opacity, VAbstractPattern *doc, QUndoCommand *parent = nullptr);
    ~OpaqueBackgroundImage() override = default;
    void undo() override;
    void redo() override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(OpaqueBackgroundImage) // NOLINT
    QUuid m_id;
    qreal m_opacity;
    qreal m_oldOpacity{1.0};
};

#endif // OPAQUEBACKGROUNDIMAGE_H
