/************************************************************************
 **
 **  @file   vincrement_p.h
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

#ifndef VINCREMENT_P_H
#define VINCREMENT_P_H

#include <QSharedData>

#include "../vcontainer.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VIncrementData final : public QSharedData
{
public:
    VIncrementData() = default;
    VIncrementData(VContainer *data, IncrementType incrType);
    VIncrementData(const VIncrementData &incr) = default;
    ~VIncrementData() = default;

    /** @brief id each increment have unique identificator */
    quint32 index{0};                                 // NOLINT(misc-non-private-member-variables-in-classes)
    QString formula{};                                // NOLINT(misc-non-private-member-variables-in-classes)
    bool formulaOk{false};                            // NOLINT(misc-non-private-member-variables-in-classes)
    bool previewCalculation{false};                   // NOLINT(misc-non-private-member-variables-in-classes)
    QSharedPointer<VContainer> data;                  // NOLINT(misc-non-private-member-variables-in-classes)
    IncrementType incrType{IncrementType::Increment}; // NOLINT(misc-non-private-member-variables-in-classes)
    bool specialUnits{false};                         // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VIncrementData) // NOLINT
};

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
inline VIncrementData::VIncrementData(VContainer *data, IncrementType incrType)
  : data(QSharedPointer<VContainer>(new VContainer(*data))),
    incrType(incrType)
{
    // When we create an increment in the dialog it will get neccesary data. Such data must be removed because will
    // confuse a user. Increment should not know nothing about internal variables.
    Q_STATIC_ASSERT_X(static_cast<int>(VarType::Unknown) == 12, "Check that you used all types");
    this->data->ClearVariables(QVector<VarType>{
        VarType::LineAngle, VarType::LineLength, VarType::CurveLength, VarType::CurveCLength, VarType::ArcRadius,
        VarType::CurveAngle, VarType::IncrementSeparator, VarType::PieceExternalArea, VarType::PieceSeamLineArea});
}

#endif // VINCREMENT_P_H
