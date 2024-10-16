/************************************************************************
 **
 **  @file   addbackgroundimage.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 1, 2022
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
#ifndef ADDBACKGROUNDIMAGE_H
#define ADDBACKGROUNDIMAGE_H

#include "../ifc/xml/vbackgroundpatternimage.h"
#include "../vundocommand.h"

class AddBackgroundImage : public VUndoCommand
{
    Q_OBJECT // NOLINT

public:
    AddBackgroundImage(const VBackgroundPatternImage &image, VAbstractPattern *doc, QUndoCommand *parent = nullptr);
    ~AddBackgroundImage() override = default;
    void undo() override;
    void redo() override;
signals:
    void AddItem(const QUuid &id);
    void DeleteItem(const QUuid &id);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(AddBackgroundImage) // NOLINT
    VBackgroundPatternImage m_image;
};

#endif // ADDBACKGROUNDIMAGE_H
