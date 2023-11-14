/************************************************************************
 **
 **  @file   vformula_p.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   17 4, 2019
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2019 Valentina project
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
#ifndef VFORMULA_P_H
#define VFORMULA_P_H

#include <QCoreApplication>
#include <QSharedData>
#include <limits>

#include "../vmisc/defglobal.h"
#include "../vmisc/typedef.h"

class VContainer;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VFormulaData : public QSharedData
{
    Q_DECLARE_TR_FUNCTIONS(VFormulaData) // NOLINT

public:
    VFormulaData() = default;
    VFormulaData(const QString &formula, const VContainer *container);
    VFormulaData(const VFormulaData &formula) = default;
    ~VFormulaData() = default;

    QString formula{};                                     // NOLINT(misc-non-private-member-variables-in-classes)
    QString strValue{tr("Error")};                         // NOLINT(misc-non-private-member-variables-in-classes)
    bool checkZero{false};                                 // NOLINT(misc-non-private-member-variables-in-classes)
    bool checkLessThanZero{false};                         // NOLINT(misc-non-private-member-variables-in-classes)
    const VContainer *data{nullptr};                       // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 toolId{NULL_ID};                               // NOLINT(misc-non-private-member-variables-in-classes)
    QString postfix{};                                     // NOLINT(misc-non-private-member-variables-in-classes)
    bool error{true};                                      // NOLINT(misc-non-private-member-variables-in-classes)
    qreal dValue{std::numeric_limits<qreal>::quiet_NaN()}; // NOLINT(misc-non-private-member-variables-in-classes)
    QString reason{tr("Formula is empty")};                // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VFormulaData) // NOLINT
};

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
inline VFormulaData::VFormulaData(const QString &formula, const VContainer *container)
  : formula(formula),
    data(container),
    reason(tr("Not evaluated"))
{
}

#endif // VFORMULA_P_H
