/************************************************************************
 **
 **  @file   movebackgroundimage.cpp
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
#include "movebackgroundimage.h"
#include "../ifc/xml/vbackgroundpatternimage.h"

//---------------------------------------------------------------------------------------------------------------------
MoveBackgroundImage::MoveBackgroundImage(QUuid id, qreal dx, qreal dy, VAbstractPattern *doc, bool allowMerge,
                                         QUndoCommand *parent)
    : VUndoCommand(QDomElement(), doc, parent),
      m_id(id),
      m_dx(dx),
      m_dy(dy),
      m_allowMerge(allowMerge)
{
    setText(tr("move background image"));

    VBackgroundPatternImage const image = doc->GetBackgroundImage(m_id);

    m_oldPos = image.Matrix();
}

//---------------------------------------------------------------------------------------------------------------------
void MoveBackgroundImage::undo()
{
    VBackgroundPatternImage image = doc->GetBackgroundImage(m_id);

    if (not image.IsNull())
    {
        image.SetMatrix(m_oldPos);
        doc->SaveBackgroundImage(image);
        emit doc->BackgroundImagePositionChanged(m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void MoveBackgroundImage::redo()
{
    VBackgroundPatternImage image = doc->GetBackgroundImage(m_id);

    if (not image.IsNull())
    {
        QTransform matrix = image.Matrix();
        QTransform m;
        m.translate(m_dx, m_dy);
        matrix *= m;
        image.SetMatrix(matrix);
        doc->SaveBackgroundImage(image);
        emit doc->BackgroundImagePositionChanged(m_id);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveBackgroundImage::mergeWith(const QUndoCommand *command) -> bool
{
    if (command->id() != id())
    {
        return false;
    }

    const auto *moveCommand = dynamic_cast<const MoveBackgroundImage *>(command);
    SCASSERT(moveCommand != nullptr)

    if (moveCommand->ImageId() != m_id || not moveCommand->AllowMerge())
    {
        return false;
    }

    m_dx = moveCommand->Dx();
    m_dy = moveCommand->Dy();
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveBackgroundImage::id() const -> int
{
    return static_cast<int>(UndoCommand::MoveBackGroundImage);
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveBackgroundImage::ImageId() const -> QUuid
{
    return m_id;
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveBackgroundImage::Dx() const -> qreal
{
    return m_dx;
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveBackgroundImage::Dy() const -> qreal
{
    return m_dy;
}

//---------------------------------------------------------------------------------------------------------------------
auto MoveBackgroundImage::AllowMerge() const -> bool
{
    return m_allowMerge;
}
