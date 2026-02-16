/************************************************************************
 **
 **  @file   holdbackgroundimage.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 1, 2022
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
#include "holdbackgroundimage.h"
#include "../ifc/xml/vbackgroundpatternimage.h"

//---------------------------------------------------------------------------------------------------------------------
HoldBackgroundImage::HoldBackgroundImage(QUuid id, bool hold, VAbstractPattern *doc, QUndoCommand *parent)
  : VUndoCommand(doc, parent),
    m_id(id),
    m_hold(hold)
{
    if (hold)
    {
        setText(tr("hold background image"));
    }
    else
    {
        setText(tr("unhold background image"));
    }

    VBackgroundPatternImage const image = doc->GetBackgroundImage(m_id);

    m_oldHold = image.Hold();
}

//---------------------------------------------------------------------------------------------------------------------
void HoldBackgroundImage::undo()
{
    VBackgroundPatternImage image = Doc()->GetBackgroundImage(m_id);

    if (not image.IsNull())
    {
        image.SetHold(m_oldHold);
        Doc()->SaveBackgroundImage(image);
        emit Doc()->BackgroundImageHoldChanged(m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void HoldBackgroundImage::redo()
{
    VBackgroundPatternImage image = Doc()->GetBackgroundImage(m_id);

    if (not image.IsNull())
    {
        image.SetHold(m_hold);
        Doc()->SaveBackgroundImage(image);
        emit Doc()->BackgroundImageHoldChanged(m_id);
    }
}
