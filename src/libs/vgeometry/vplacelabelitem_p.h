/************************************************************************
 **
 **  @file   vplacelabelitem_p.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 10, 2017
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
#ifndef VPLACELABELITEM_P_H
#define VPLACELABELITEM_P_H

#include "vgeometrydef.h"
#include <QSharedData>

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VPlaceLabelItemData final : public QSharedData
{
public:
    VPlaceLabelItemData() = default;
    VPlaceLabelItemData(const VPlaceLabelItemData &item) = default;
    ~VPlaceLabelItemData() = default;

    QString width{};                             // NOLINT(misc-non-private-member-variables-in-classes)
    QString height{};                            // NOLINT(misc-non-private-member-variables-in-classes)
    QString angle{'0'};                          // NOLINT(misc-non-private-member-variables-in-classes)
    QString visibilityTrigger{'1'};              // NOLINT(misc-non-private-member-variables-in-classes)
    PlaceLabelType type{PlaceLabelType::Button}; // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 centerPoint{0};                      // NOLINT(misc-non-private-member-variables-in-classes)

    qreal wValue{0};          // NOLINT(misc-non-private-member-variables-in-classes)
    qreal hValue{0};          // NOLINT(misc-non-private-member-variables-in-classes)
    qreal aValue{0};          // NOLINT(misc-non-private-member-variables-in-classes)
    qreal correctionAngle{0}; // NOLINT(misc-non-private-member-variables-in-classes)
    qreal isVisible{1};       // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VPlaceLabelItemData) // NOLINT
};

QT_WARNING_POP

#endif // VPLACELABELITEM_P_H
