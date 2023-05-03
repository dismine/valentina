/************************************************************************
 **
 **  @file   rotatebackgroundimage.cpp
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
#include "rotatebackgroundimage.h"
#include "../ifc/xml/vbackgroundpatternimage.h"

//---------------------------------------------------------------------------------------------------------------------
RotateBackgroundImage::RotateBackgroundImage(QUuid id, const QTransform &matrix, VAbstractPattern *doc, bool allowMerge,
                                             QUndoCommand *parent)
    : VUndoCommand(QDomElement(), doc, parent),
      m_id(id),
      m_matrix(matrix),
      m_allowMerge(allowMerge)
{
    setText(tr("rotate background image"));

    VBackgroundPatternImage image = doc->GetBackgroundImage(m_id);

    m_oldMatrix = image.Matrix();
}

//---------------------------------------------------------------------------------------------------------------------
void RotateBackgroundImage::undo()
{
    VBackgroundPatternImage image = doc->GetBackgroundImage(m_id);

    if (not image.IsNull())
    {
        image.SetMatrix(m_oldMatrix);
        doc->SaveBackgroundImage(image);
        emit doc->BackgroundImageTransformationChanged(m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void RotateBackgroundImage::redo()
{
    VBackgroundPatternImage image = doc->GetBackgroundImage(m_id);

    if (not image.IsNull())
    {
        image.SetMatrix(m_matrix);
        doc->SaveBackgroundImage(image);
        emit doc->BackgroundImageTransformationChanged(m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto RotateBackgroundImage::mergeWith(const QUndoCommand *command) -> bool
{
    if (command->id() != id())
    {
        return false;
    }

    const auto *moveCommand = dynamic_cast<const RotateBackgroundImage *>(command);
    SCASSERT(moveCommand != nullptr)

    if (moveCommand->ImageId() != m_id || not moveCommand->AllowMerge())
    {
        return false;
    }

    m_matrix = moveCommand->Matrix();
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto RotateBackgroundImage::id() const -> int
{
    return static_cast<int>(UndoCommand::RotateBackGroundImage);
}

//---------------------------------------------------------------------------------------------------------------------
auto RotateBackgroundImage::ImageId() const -> QUuid
{
    return m_id;
}

//---------------------------------------------------------------------------------------------------------------------
auto RotateBackgroundImage::Matrix() const -> QTransform
{
    return m_matrix;
}

//---------------------------------------------------------------------------------------------------------------------
auto RotateBackgroundImage::AllowMerge() const -> bool
{
    return m_allowMerge;
}
