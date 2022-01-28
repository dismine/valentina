/************************************************************************
 **
 **  @file   scalebackgroundimage.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 1, 2022
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
#include "scalebackgroundimage.h"
#include "../ifc/xml/vbackgroundpatternimage.h"

//---------------------------------------------------------------------------------------------------------------------
ScaleBackgroundImage::ScaleBackgroundImage(QUuid id, const QTransform &matrix, VAbstractPattern *doc, bool allowMerge,
                                           QUndoCommand *parent)
    : VUndoCommand(QDomElement(), doc, parent),
      m_id(id),
      m_matrix(matrix),
      m_allowMerge(allowMerge)
{
    setText(tr("scale background image"));

    VBackgroundPatternImage image = doc->GetBackgroundImage(m_id);

    m_oldMatrix = image.Matrix();
}

//---------------------------------------------------------------------------------------------------------------------
void ScaleBackgroundImage::undo()
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
void ScaleBackgroundImage::redo()
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
bool ScaleBackgroundImage::mergeWith(const QUndoCommand *command)
{
    if (command->id() != id())
    {
        return false;
    }

    const auto *moveCommand = dynamic_cast<const ScaleBackgroundImage *>(command);
    SCASSERT(moveCommand != nullptr)

    if (moveCommand->ImageId() != m_id || not moveCommand->AllowMerge())
    {
        return false;
    }

    m_matrix = moveCommand->Matrix();
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
int ScaleBackgroundImage::id() const
{
    return static_cast<int>(UndoCommand::ScaleBackGroundImage);
}

//---------------------------------------------------------------------------------------------------------------------
QUuid ScaleBackgroundImage::ImageId() const
{
    return m_id;
}

//---------------------------------------------------------------------------------------------------------------------
QTransform ScaleBackgroundImage::Matrix() const
{
    return m_matrix;
}

//---------------------------------------------------------------------------------------------------------------------
bool ScaleBackgroundImage::AllowMerge() const
{
    return m_allowMerge;
}
