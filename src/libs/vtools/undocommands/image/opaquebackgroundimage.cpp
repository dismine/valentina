/************************************************************************
 **
 **  @file   opaquebackgroundimage.cpp
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
#include "opaquebackgroundimage.h"
#include "../ifc/xml/vbackgroundpatternimage.h"

//---------------------------------------------------------------------------------------------------------------------
OpaqueBackgroundImage::OpaqueBackgroundImage(QUuid id, qreal opacity, VAbstractPattern *doc, QUndoCommand *parent)
    : VUndoCommand(QDomElement(), doc, parent),
      m_id(id),
      m_opacity(opacity)
{
    setText(tr("change a background image opacity"));

    VBackgroundPatternImage image = doc->GetBackgroundImage(m_id);

    m_oldOpacity = image.Opacity();
}

//---------------------------------------------------------------------------------------------------------------------
void OpaqueBackgroundImage::undo()
{
    VBackgroundPatternImage image = doc->GetBackgroundImage(m_id);

    if (not image.IsNull())
    {
        image.SetOpacity(m_oldOpacity);
        doc->SaveBackgroundImage(image);
        emit doc->BackgroundImageOpacityChanged(m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void OpaqueBackgroundImage::redo()
{
    VBackgroundPatternImage image = doc->GetBackgroundImage(m_id);

    if (not image.IsNull())
    {
        image.SetOpacity(m_opacity);
        doc->SaveBackgroundImage(image);
        emit doc->BackgroundImageOpacityChanged(m_id);
    }
}
