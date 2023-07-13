/************************************************************************
 **
 **  @file   vgrainlinedata.cpp
 **  @author Bojan Kverh
 **  @date   September 06, 2016
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

#include <QPointF>

#include "vgrainlinedata.h"
#include "vgrainlinedata_p.h"

//---------------------------------------------------------------------------------------------------------------------
VGrainlineData::VGrainlineData()
    : VAbstractFloatItemData(),
      d(new VGrainlineDataPrivate())
{}

//---------------------------------------------------------------------------------------------------------------------
VGrainlineData::VGrainlineData(const VGrainlineData &data)
    : VAbstractFloatItemData(data),
      d (data.d)
{}

//---------------------------------------------------------------------------------------------------------------------
auto VGrainlineData::operator=(const VGrainlineData &data) -> VGrainlineData &
{
    if ( &data == this )
    {
        return *this;
    }
    VAbstractFloatItemData::operator=(data);
    d = data.d;
    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VGrainlineData::VGrainlineData(VGrainlineData &&data) noexcept
    : VAbstractFloatItemData(std::move(data)),
      d (std::move(data.d))
{}

//---------------------------------------------------------------------------------------------------------------------
auto VGrainlineData::operator=(VGrainlineData &&data) noexcept->VGrainlineData &
{
    VAbstractFloatItemData::operator=(data);
    std::swap(d, data.d);
    return *this;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
VGrainlineData::~VGrainlineData()
{}

//---------------------------------------------------------------------------------------------------------------------
auto VGrainlineData::GetLength() const -> QString
{
    return d->m_qsLength;
}

//---------------------------------------------------------------------------------------------------------------------
void VGrainlineData::SetLength(const QString& qsLen)
{
    d->m_qsLength = qsLen;
}

//---------------------------------------------------------------------------------------------------------------------
auto VGrainlineData::GetRotation() const -> QString
{
    return d->m_dRotation;
}

//---------------------------------------------------------------------------------------------------------------------
void VGrainlineData::SetRotation(const QString& qsRot)
{
    d->m_dRotation = qsRot;
}

//---------------------------------------------------------------------------------------------------------------------
auto VGrainlineData::GetArrowType() const -> GrainlineArrowDirection
{
    return d->m_eArrowType;
}

//---------------------------------------------------------------------------------------------------------------------
void VGrainlineData::SetArrowType(GrainlineArrowDirection eAT)
{
    d->m_eArrowType = eAT;
}

//---------------------------------------------------------------------------------------------------------------------
auto VGrainlineData::CenterPin() const -> quint32
{
    return d->m_centerPin;
}

//---------------------------------------------------------------------------------------------------------------------
void VGrainlineData::SetCenterPin(quint32 centerPin)
{
    d->m_centerPin = centerPin;
}

//---------------------------------------------------------------------------------------------------------------------
auto VGrainlineData::TopPin() const -> quint32
{
    return d->m_topPin;
}

//---------------------------------------------------------------------------------------------------------------------
void VGrainlineData::SetTopPin(quint32 topPin)
{
    d->m_topPin = topPin;
}

//---------------------------------------------------------------------------------------------------------------------
auto VGrainlineData::BottomPin() const -> quint32
{
    return d->m_bottomPin;
}

//---------------------------------------------------------------------------------------------------------------------
void VGrainlineData::SetBottomPin(quint32 bottomPin)
{
    d->m_bottomPin = bottomPin;
}
