/************************************************************************
 **
 **  @file   holdallbackgroundimages.cpp
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
#include "holdallbackgroundimages.h"
#include "../ifc/xml/vbackgroundpatternimage.h"

//---------------------------------------------------------------------------------------------------------------------
HoldAllBackgroundImages::HoldAllBackgroundImages(bool hold, VAbstractPattern *doc, QUndoCommand *parent)
  : VUndoCommand(doc, parent),
    m_hold(hold)
{
    if (hold)
    {
        setText(tr("hold all background images"));
    }
    else
    {
        setText(tr("unhold background images"));
    }

    QVector<VBackgroundPatternImage> const images = doc->GetBackgroundImages();

    for (const auto& image : images)
    {
        m_oldHold.insert(image.Id(), image.Hold());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void HoldAllBackgroundImages::undo()
{
    QVector<VBackgroundPatternImage> images = Doc()->GetBackgroundImages();

    for (auto &image : images)
    {
        image.SetHold(m_oldHold.value(image.Id(), image.Hold()));
    }

    Doc()->SaveBackgroundImages(images);
    emit Doc()->BackgroundImagesHoldChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void HoldAllBackgroundImages::redo()
{
    QVector<VBackgroundPatternImage> images = Doc()->GetBackgroundImages();

    for (auto &image : images)
    {
        image.SetHold(m_hold);
    }

    Doc()->SaveBackgroundImages(images);
    emit Doc()->BackgroundImagesHoldChanged();
}
