/************************************************************************
 **
 **  @file   vknownmeasurements.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 10, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#include "vknownmeasurements.h"
#include "../vmisc/def.h"
#include "vknownmeasurements_p.h"

#include <QMap>
#include <QSet>
#include <QString>

//---------------------------------------------------------------------------------------------------------------------
VKnownMeasurements::VKnownMeasurements()
  : d(new VKnownMeasurementsData)
{
}

//---------------------------------------------------------------------------------------------------------------------
VKnownMeasurements::~VKnownMeasurements() // NOLINT(modernize-use-equals-default)
{
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL(VKnownMeasurements)

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurements::operator=(const VKnownMeasurements &measurements) -> VKnownMeasurements &
{
    if (&measurements == this)
    {
        return *this;
    }
    d = measurements.d;
    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VKnownMeasurements::VKnownMeasurements(VKnownMeasurements &&paper) noexcept
  : d(std::move(paper.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurements::operator=(VKnownMeasurements &&paper) noexcept -> VKnownMeasurements &
{
    std::swap(d, paper.d);
    return *this;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurements::IsValid() const -> bool
{
    return !d->m_uid.isNull();
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurements::GetUId() const -> QUuid
{
    return d->m_uid;
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurements::SetUId(const QUuid &id)
{
    d->m_uid = id;
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurements::Name() const -> QString
{
    return d->m_name;
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurements::SetName(const QString &name)
{
    d->m_name = name;
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurements::Description() const -> QString
{
    return d->m_description;
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurements::SetDescription(const QString &desc)
{
    d->m_description = desc;
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurements::IsReadOnly() const -> bool
{
    return d->m_readOnly;
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurements::SetReadOnly(bool ro)
{
    d->m_readOnly = ro;
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurements::Measurements() const -> QHash<QString, VKnownMeasurement>
{
    return d->m_measurements;
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurements::OrderedMeasurements() const -> QMap<int, VKnownMeasurement>
{
    QMap<int, VKnownMeasurement> ordered;
    auto i = d->m_measurements.constBegin();
    while (i != d->m_measurements.constEnd())
    {
        ordered.insert(i.value().index, i.value());
        ++i;
    }

    return ordered;
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurements::OrderedGroupMeasurements(const QString &group) const -> QMap<int, VKnownMeasurement>
{
    QMap<int, VKnownMeasurement> ordered;
    auto i = d->m_measurements.constBegin();
    while (i != d->m_measurements.constEnd())
    {
        if (group == i.value().group)
        {
            ordered.insert(i.value().index, i.value());
        }
        ++i;
    }

    return ordered;
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurements::Images() const -> QMap<QUuid, VPatternImage>
{
    return d->m_images;
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurements::Groups() const -> QStringList
{
    QSet<QString> groups;

    auto i = d->m_measurements.constBegin();
    while (i != d->m_measurements.constEnd())
    {
        if (!i.value().group.isEmpty())
        {
            groups.insert(i.value().group);
        }
        ++i;
    }

    return groups.values();
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurements::Measurement(const QString &name) const -> VKnownMeasurement
{
    return d->m_measurements.value(name);
}

//---------------------------------------------------------------------------------------------------------------------
auto VKnownMeasurements::Image(const QUuid &id) const -> VPatternImage
{
    return d->m_images.value(id);
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurements::AddMeasurement(const VKnownMeasurement &m)
{
    d->m_measurements.insert(m.name, m);
}

//---------------------------------------------------------------------------------------------------------------------
void VKnownMeasurements::AddImage(const QUuid &id, const VPatternImage &image)
{
    d->m_images.insert(id, image);
}
