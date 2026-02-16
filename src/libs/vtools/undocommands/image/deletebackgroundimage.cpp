/************************************************************************
 **
 **  @file   deletebackgroundimage.cpp
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
#include "deletebackgroundimage.h"

//---------------------------------------------------------------------------------------------------------------------
DeleteBackgroundImage::DeleteBackgroundImage(const VBackgroundPatternImage &image,
                                             VAbstractPattern *doc,
                                             QUndoCommand *parent)
  : VUndoCommand(doc, parent),
    m_image(image)
{
    setText(tr("delete background image"));

    QVector<VBackgroundPatternImage> const allImages = doc->GetBackgroundImages();

    for (int i = 0; i < allImages.size(); ++i)
    {
        if (allImages.at(i).Id() == image.Id())
        {
            m_index = i;
            break;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void DeleteBackgroundImage::undo()
{
    qCDebug(vUndo, "Undo.");

    QVector<VBackgroundPatternImage> allImages = Doc()->GetBackgroundImages();

    if (m_index == -1)
    {
        Doc()->SaveBackgroundImage(m_image);
    }
    else
    {
        allImages.insert(m_index, m_image);
        Doc()->SaveBackgroundImages(allImages);
    }

    emit AddItem(m_image.Id());
}

//---------------------------------------------------------------------------------------------------------------------
void DeleteBackgroundImage::redo()
{
    qCDebug(vUndo, "Redo.");

    Doc()->DeleteBackgroundImage(m_image.Id());

    emit DeleteItem(m_image.Id());
}
