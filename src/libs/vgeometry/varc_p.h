/************************************************************************
 **
 **  @file   varc_p.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 8, 2014
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

#ifndef VARC_P_H
#define VARC_P_H

#include <QSharedData>
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)

#include "../vmisc/defglobal.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VArcData final : public QSharedData
{
public:
    VArcData() = default;
    VArcData(qreal radius, const QString &formulaRadius);
    explicit VArcData(qreal radius);
    VArcData(const VArcData &arc) = default;
    ~VArcData() = default;

    /** @brief radius arc radius. */
    qreal radius{0}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief formulaRadius formula for arc radius. */
    QString formulaRadius{}; // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VArcData) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
inline VArcData::VArcData(qreal radius, const QString &formulaRadius)
  : radius(radius),
    formulaRadius(formulaRadius)
{
}

//---------------------------------------------------------------------------------------------------------------------
inline VArcData::VArcData(qreal radius)
  : radius(radius),
    formulaRadius(QString::number(radius))
{
}

QT_WARNING_POP

#endif // VARC_P_H
