/************************************************************************
 **
 **  @file   vknownmeasurements.h
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
#ifndef VKNOWNMEASUREMENTS_H
#define VKNOWNMEASUREMENTS_H

#include "../ifc/xml/vpatternimage.h"
#include <QSharedDataPointer>
#include <QTypeInfo>

class VKnownMeasurementsData;
class QUuid;
struct VKnownMeasurement;

class VKnownMeasurements
{
public:
    VKnownMeasurements();
    ~VKnownMeasurements();

    VKnownMeasurements(const VKnownMeasurements &measurements);

    auto operator=(const VKnownMeasurements &paper) -> VKnownMeasurements &;

    VKnownMeasurements(VKnownMeasurements &&paper) noexcept;
    auto operator=(VKnownMeasurements &&paper) noexcept -> VKnownMeasurements &;

    auto IsValid() const -> bool;

    auto GetUId() const -> QUuid;
    void SetUId(const QUuid &id);

    auto Name() const -> QString;
    void SetName(const QString &name);

    auto Description() const -> QString;
    void SetDescription(const QString &desc);

    auto IsReadOnly() const -> bool;
    void SetReadOnly(bool ro);

    auto Measurements() const -> QHash<QString, VKnownMeasurement>;
    auto OrderedMeasurements() const -> QMap<int, VKnownMeasurement>;
    auto OrderedGroupMeasurements(const QString &group) const -> QMap<int, VKnownMeasurement>;
    auto Images() const -> QMap<QUuid, VPatternImage>;
    auto Groups() const -> QStringList;

    auto Measurement(const QString &name) const -> VKnownMeasurement;
    auto Image(const QUuid &id) const -> VPatternImage;

    void AddMeasurement(const VKnownMeasurement &m);
    void AddImage(const QUuid &id, const VPatternImage &image);

private:
    QSharedDataPointer<VKnownMeasurementsData> d;
};

Q_DECLARE_TYPEINFO(VKnownMeasurements, Q_MOVABLE_TYPE); // NOLINT

#endif // VKNOWNMEASUREMENTS_H
