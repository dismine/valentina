/************************************************************************
 **
 **  @file   renamebackgroundimage.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 1, 2022
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
#include "renamebackgroundimage.h"
#include "../ifc/xml/vbackgroundpatternimage.h"

//---------------------------------------------------------------------------------------------------------------------
RenameBackgroundImage::RenameBackgroundImage(QUuid id, const QString &name, VAbstractPattern *doc, QUndoCommand *parent)
  : VUndoCommand(doc, parent),
    m_id(id),
    m_name(name)
{
    setText(tr("rename background image"));

    VBackgroundPatternImage const image = doc->GetBackgroundImage(m_id);

    m_oldName = image.Name();
}

//---------------------------------------------------------------------------------------------------------------------
void RenameBackgroundImage::undo()
{
    VBackgroundPatternImage image = Doc()->GetBackgroundImage(m_id);

    if (not image.IsNull())
    {
        image.SetName(m_oldName);
        Doc()->SaveBackgroundImage(image);
        emit Doc()->BackgroundImageNameChanged(m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void RenameBackgroundImage::redo()
{
    VBackgroundPatternImage image = Doc()->GetBackgroundImage(m_id);

    if (not image.IsNull())
    {
        image.SetName(m_name);
        Doc()->SaveBackgroundImage(image);
        emit Doc()->BackgroundImageNameChanged(m_id);
    }
}
