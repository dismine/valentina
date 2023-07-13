/************************************************************************
 **
 **  @file   vpiecelabeldata.cpp
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

#include "vpiecelabeldata.h"
#include "vpiecelabeldata_p.h"

#include <QList>

//---------------------------------------------------------------------------------------------------------------------
VPieceLabelData::VPieceLabelData()
    : VPatternLabelData(),
      d(new VPieceLabelDataPrivate())
{}

//---------------------------------------------------------------------------------------------------------------------
VPieceLabelData::VPieceLabelData(const VPieceLabelData &data)
    : VPatternLabelData(data),
      d (data.d)
{}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceLabelData::operator=(const VPieceLabelData &data) -> VPieceLabelData &
{
    if ( &data == this )
    {
        return *this;
    }
    VPatternLabelData::operator=(data);
    d = data.d;
    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VPieceLabelData::VPieceLabelData(VPieceLabelData &&data) noexcept
    : VPatternLabelData(std::move(data)),
      d (std::move(data.d))
{}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceLabelData::operator=(VPieceLabelData &&data) noexcept->VPieceLabelData &
{
    VPatternLabelData::operator=(data);
    std::swap(d, data.d);
    return *this;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
VPieceLabelData::~VPieceLabelData()
{}

//---------------------------------------------------------------------------------------------------------------------
void VPieceLabelData::Clear()
{
    d->m_qsLetter.clear();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceLabelData::GetLetter() const -> QString
{
    return d->m_qsLetter;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceLabelData::SetLetter(const QString &qsLetter)
{
    d->m_qsLetter = qsLetter.left(5);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceLabelData::GetAnnotation() const -> QString
{
    return d->m_annotation;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceLabelData::SetAnnotation(const QString &val)
{
    d->m_annotation = val;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceLabelData::GetOrientation() const -> QString
{
    return d->m_orientation;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceLabelData::SetOrientation(const QString &val)
{
    d->m_orientation = val;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceLabelData::GetRotationWay() const -> QString
{
    return d->m_rotationWay;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceLabelData::SetRotationWay(const QString &val)
{
    d->m_rotationWay = val;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceLabelData::GetTilt() const -> QString
{
    return d->m_tilt;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceLabelData::SetTilt(const QString &val)
{
    d->m_tilt = val;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceLabelData::GetFoldPosition() const -> QString
{
    return d->m_foldPosition;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceLabelData::SetFoldPosition(const QString &val)
{
    d->m_foldPosition = val;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceLabelData::GetQuantity() const -> quint16
{
    return d->m_quantity;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceLabelData::SetQuantity(quint16 val)
{
    d->m_quantity = qMax(static_cast<quint16>(1), val);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceLabelData::IsOnFold() const -> bool
{
    return d->m_onFold;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceLabelData::SetOnFold(bool onFold)
{
    d->m_onFold = onFold;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceLabelData::GetAreaShortName() const -> QString
{
    return d->m_areaShortName;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceLabelData::SetAreaShortName(const QString &val)
{
    d->m_areaShortName = val;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceLabelData::GetLabelTemplate() const -> QVector<VLabelTemplateLine>
{
    return d->m_lines;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceLabelData::SetLabelTemplate(const QVector<VLabelTemplateLine> &lines)
{
    d->m_lines = lines;
}
