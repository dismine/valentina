/************************************************************************
 **
 **  @file   vknownmeasurementsdatabase.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 10, 2023
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
#ifndef VKNOWNMEASUREMENTSDATABASE_H
#define VKNOWNMEASUREMENTSDATABASE_H

#include <QCache>
#include <QHash>
#include <QString>
#include <QUuid>

class VKnownMeasurements;

struct VKnownMeasurementsHeader
{
    QUuid uid{};
    QString name{};
    QString description{};
    QString path{};
};

class VKnownMeasurementsDatabase
{
public:
    VKnownMeasurementsDatabase() = default;

    void PopulateMeasurementsDatabase();

    auto IsPopulated() const -> bool;

    auto AllKnownMeasurements() const -> QHash<QUuid, VKnownMeasurementsHeader>;

    auto KnownMeasurements(const QUuid &id) const -> VKnownMeasurements;

private:
    bool m_populated{false};
    QHash<QUuid, VKnownMeasurementsHeader> m_measurementsDB{};
    QHash<QUuid, QString> m_indexMeasurementsPath{};
    mutable QCache<QUuid, VKnownMeasurements> m_measurementsCache{15};

    void UpdateIndexes();

    void ParseDirectory(const QString &path);
};

#endif // VKNOWNMEASUREMENTSDATABASE_H
