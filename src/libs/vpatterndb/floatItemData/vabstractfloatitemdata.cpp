/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 2, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#include "vabstractfloatitemdata.h"
#include "../vmisc/def.h"
#include "vabstractfloatitemdata_p.h"

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wnoexcept")

VAbstractFloatItemData::VAbstractFloatItemData()
  : d(new VAbstractFloatItemDataPrivate())
{
}

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL(VAbstractFloatItemData)

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractFloatItemData::operator=(const VAbstractFloatItemData &data) -> VAbstractFloatItemData &
{
    if (&data == this)
    {
        return *this;
    }
    d = data.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VAbstractFloatItemData::VAbstractFloatItemData(VAbstractFloatItemData &&data) noexcept
  : d(std::move(data.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractFloatItemData::operator=(VAbstractFloatItemData &&data) noexcept -> VAbstractFloatItemData &
{
    std::swap(d, data.d);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VAbstractFloatItemData::~VAbstractFloatItemData() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractFloatItemData::GetPos() const -> QPointF
{
    return d->m_ptPos;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractFloatItemData::SetPos(const QPointF &ptPos)
{
    d->m_ptPos = ptPos;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractFloatItemData::IsEnabled() const -> bool
{
    return d->m_bEnabled;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractFloatItemData::SetEnabled(bool bEnabled)
{
    d->m_bEnabled = bEnabled;
}
