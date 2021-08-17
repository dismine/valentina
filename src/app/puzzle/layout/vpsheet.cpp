/************************************************************************
 **
 **  @file   vpsheet.cpp
 **  @author Ronan Le Tiec
 **  @date   23 5, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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
#include "vpsheet.h"

#include "vplayout.h"

//---------------------------------------------------------------------------------------------------------------------
VPSheet::VPSheet(const VPLayoutPtr &layout) :
    m_layout(layout)
{
    SCASSERT(layout != nullptr)
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetLayout() const -> VPLayoutPtr
{
    return m_layout;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetPieces() const -> QList<VPPiecePtr>
{
    QList<VPPiecePtr> list;

    VPLayoutPtr layout = GetLayout();
    if (not layout.isNull())
    {
        return layout->PiecesForSheet(m_uuid);
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GetName() const -> QString
{
    return m_name;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetName(const QString &name)
{
    m_name = name;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::Uuid() const -> const QUuid &
{
    return m_uuid;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::IsVisible() const -> bool
{
    return m_visible;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetVisible(bool visible)
{
    m_visible = visible;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::GrainlineType() const -> enum GrainlineType
{
    VPLayoutPtr layout = GetLayout();
    if (not layout.isNull())
    {
        QSizeF size =  layout->LayoutSettings().GetSheetSize();
        if (size.height() < size.width())
        {
            return GrainlineType::Horizontal;
        }
    }

    return GrainlineType::Vertical;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPSheet::TransformationOrigin() const -> const VPTransformationOrigon &
{
    return m_transformationOrigin;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::SetTransformationOrigin(const VPTransformationOrigon &newTransformationOrigin)
{
    m_transformationOrigin = newTransformationOrigin;
}

//---------------------------------------------------------------------------------------------------------------------
void VPSheet::Clear()
{
    m_name.clear();
    m_visible = true;
    m_transformationOrigin = VPTransformationOrigon();
}
