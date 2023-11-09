/************************************************************************
 **
 **  @file   vknownmeasurements_p.h
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
#ifndef VKNOWNMEASUREMENTS_P_H
#define VKNOWNMEASUREMENTS_P_H

#include "../ifc/xml/vpatternimage.h"
#include "../vmisc/defglobal.h"
#include "vknownmeasurement.h"

#include <QHash>
#include <QMap>
#include <QSharedData>
#include <QString>
#include <QUuid>

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VKnownMeasurementsData : public QSharedData
{
public:
    VKnownMeasurementsData() = default;
    VKnownMeasurementsData(const VKnownMeasurementsData &measurements) = default;
    ~VKnownMeasurementsData() = default;

    QUuid m_uid{};           // NOLINT (misc-non-private-member-variables-in-classes)
    QString m_name{};        // NOLINT (misc-non-private-member-variables-in-classes)
    QString m_description{}; // NOLINT (misc-non-private-member-variables-in-classes)
    bool m_readOnly{false};  // NOLINT (misc-non-private-member-variables-in-classes)

    QMap<QUuid, VPatternImage> m_images{};              // NOLINT (misc-non-private-member-variables-in-classes)
    QHash<QString, VKnownMeasurement> m_measurements{}; // NOLINT (misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VKnownMeasurementsData) // NOLINT
};

QT_WARNING_POP

#endif // VKNOWNMEASUREMENTS_P_H
