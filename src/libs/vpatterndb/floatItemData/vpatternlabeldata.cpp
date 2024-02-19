/************************************************************************
 **
 **  @file   vpatternlabeldata.cpp
 **  @author Bojan Kverh
 **  @date   June 16, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#include "vpatternlabeldata.h"
#include "../vmisc/def.h"
#include "vpatternlabeldata_p.h"

//---------------------------------------------------------------------------------------------------------------------
VPatternLabelData::VPatternLabelData()
  : VAbstractFloatItemData(),
    d(new VPatternLabelDataPrivate())
{
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL_2(VPatternLabelData, VAbstractFloatItemData)

//---------------------------------------------------------------------------------------------------------------------
auto VPatternLabelData::operator=(const VPatternLabelData &data) -> VPatternLabelData &
{
    if (&data == this)
    {
        return *this;
    }
    VAbstractFloatItemData::operator=(data);
    d = data.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VPatternLabelData::VPatternLabelData(VPatternLabelData &&data) noexcept
  : VAbstractFloatItemData(std::move(data)),
    d(std::move(data.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternLabelData::operator=(VPatternLabelData &&data) noexcept -> VPatternLabelData &
{
    VAbstractFloatItemData::operator=(data);
    std::swap(d, data.d);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VPatternLabelData::~VPatternLabelData() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VPatternLabelData::GetLabelWidth() const -> QString
{
    return d->m_dLabelWidth;
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternLabelData::SetLabelWidth(const QString &dLabelW)
{
    d->m_dLabelWidth = dLabelW;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternLabelData::GetLabelHeight() const -> QString
{
    return d->m_dLabelHeight;
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternLabelData::SetLabelHeight(const QString &dLabelH)
{
    d->m_dLabelHeight = dLabelH;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternLabelData::GetFontSize() const -> int
{
    return d->m_iFontSize;
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternLabelData::SetFontSize(int iSize)
{
    d->m_iFontSize = iSize;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternLabelData::GetRotation() const -> QString
{
    return d->m_dLabelAngle;
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternLabelData::SetRotation(const QString &dRot)
{
    d->m_dLabelAngle = dRot;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternLabelData::CenterPin() const -> quint32
{
    return d->m_centerPin;
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternLabelData::SetCenterPin(const quint32 &centerPin)
{
    d->m_centerPin = centerPin;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternLabelData::TopLeftPin() const -> quint32
{
    return d->m_topLeftPin;
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternLabelData::SetTopLeftPin(const quint32 &topLeftPin)
{
    d->m_topLeftPin = topLeftPin;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternLabelData::BottomRightPin() const -> quint32
{
    return d->m_bottomRightPin;
}

//---------------------------------------------------------------------------------------------------------------------
void VPatternLabelData::SetBottomRightPin(const quint32 &bottomRightPin)
{
    d->m_bottomRightPin = bottomRightPin;
}
