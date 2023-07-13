/************************************************************************
 **
 **  @file   VAbstractArcData.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 4, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#ifndef VABSTRACTARC_P_H
#define VABSTRACTARC_P_H

#include <QSharedData>
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "vpointf.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VAbstractArcData final : public QSharedData
{
public:
    VAbstractArcData() = default;
    VAbstractArcData(const VPointF &center, qreal f1, const QString &formulaF1, qreal f2, const QString &formulaF2);
    VAbstractArcData(const QString &formulaLength, const VPointF &center, qreal f1, const QString &formulaF1);
    VAbstractArcData(const VPointF &center, qreal f1);
    VAbstractArcData(const VPointF &center, qreal f1, qreal f2);
    VAbstractArcData(const VAbstractArcData &arc) = default;
    ~VAbstractArcData() = default;

    /** @brief f1 start angle in degree. */
    qreal f1{0}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief formulaF1 formula for start angle. */
    QString formulaF1{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief f2 end angle in degree. */
    qreal f2{0}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief formulaF2 formula for end angle. */
    QString formulaF2{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief center center point of arc. */
    VPointF center{}; // NOLINT(misc-non-private-member-variables-in-classes)

    bool isFlipped{false}; // NOLINT(misc-non-private-member-variables-in-classes)

    QString formulaLength{}; // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VAbstractArcData) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
inline VAbstractArcData::VAbstractArcData(const VPointF &center, qreal f1, const QString &formulaF1, qreal f2,
                                          const QString &formulaF2)
  : f1(f1),
    formulaF1(formulaF1),
    f2(f2),
    formulaF2(formulaF2),
    center(center)
{
}

//---------------------------------------------------------------------------------------------------------------------
inline VAbstractArcData::VAbstractArcData(const QString &formulaLength, const VPointF &center, qreal f1,
                                          const QString &formulaF1)
  : f1(f1),
    formulaF1(formulaF1),
    formulaF2('0'),
    center(center),
    formulaLength(formulaLength)
{
}

//---------------------------------------------------------------------------------------------------------------------
inline VAbstractArcData::VAbstractArcData(const VPointF &center, qreal f1)
  : f1(f1),
    formulaF1(QString::number(f1)),
    formulaF2('0'),
    center(center)
{
}

//---------------------------------------------------------------------------------------------------------------------
inline VAbstractArcData::VAbstractArcData(const VPointF &center, qreal f1, qreal f2)
  : f1(f1),
    formulaF1(QString::number(f1)),
    f2(f2),
    formulaF2(QString::number(f2)),
    center(center)
{
}

QT_WARNING_POP

#endif // VABSTRACTARCDATA_H
