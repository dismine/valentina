/************************************************************************
 **
 **  @file   resetbackgroundimage.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 1, 2022
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
#include "resetbackgroundimage.h"
#include "../ifc/xml/vbackgroundpatternimage.h"

//---------------------------------------------------------------------------------------------------------------------
ResetBackgroundImage::ResetBackgroundImage(QUuid id, VAbstractPattern *doc, QUndoCommand *parent)
  : VUndoCommand(doc, parent),
    m_id(id)
{
    setText(tr("reset background image transformation"));

    VBackgroundPatternImage const image = doc->GetBackgroundImage(m_id);
    m_oldMatrix = image.Matrix();
}

//---------------------------------------------------------------------------------------------------------------------
void ResetBackgroundImage::undo()
{
    VBackgroundPatternImage image = Doc()->GetBackgroundImage(m_id);

    if (not image.IsNull())
    {
        image.SetMatrix(m_oldMatrix);
        Doc()->SaveBackgroundImage(image);
        emit Doc()->BackgroundImageTransformationChanged(m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ResetBackgroundImage::redo()
{
    VBackgroundPatternImage image = Doc()->GetBackgroundImage(m_id);

    if (not image.IsNull())
    {
        image.SetMatrix(QTransform());
        Doc()->SaveBackgroundImage(image);
        emit Doc()->BackgroundImageTransformationChanged(m_id);
    }
}
