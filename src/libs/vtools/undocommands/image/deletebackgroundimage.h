/************************************************************************
 **
 **  @file   deletebackgroundimage.h
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
#ifndef DELETEBACKGROUNDIMAGE_H
#define DELETEBACKGROUNDIMAGE_H

#include "../vundocommand.h"
#include "../ifc/xml/vbackgroundpatternimage.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 13, 0)


class DeleteBackgroundImage : public VUndoCommand
{
    Q_OBJECT
public:
    DeleteBackgroundImage(const VBackgroundPatternImage& image, VAbstractPattern *doc, QUndoCommand *parent = nullptr);
    ~DeleteBackgroundImage() override =default;
    void undo() override;
    void redo() override;
signals:
    void AddItem(const QUuid &id);
    void DeleteItem(const QUuid &id);
private:
    Q_DISABLE_COPY_MOVE(DeleteBackgroundImage)
    VBackgroundPatternImage m_image;
    int m_index{-1};
};

#endif // DELETEBACKGROUNDIMAGE_H
