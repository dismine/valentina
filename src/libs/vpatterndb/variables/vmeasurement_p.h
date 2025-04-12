/************************************************************************
 **
 **  @file   vmeasurement_p.h
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

#ifndef VMEASUREMENT_P_H
#define VMEASUREMENT_P_H

#include <QSharedData>

#include "../ifc/xml/vpatternimage.h"
#include "../vcontainer.h"
#include "../vmisc/def.h"
#include <utility>
#include <QUuid>

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VMeasurementData final : public QSharedData
{
public:
    VMeasurementData(quint32 index, MeasurementType varType);
    VMeasurementData(quint32 index, qreal baseA, qreal baseB, qreal baseC, qreal base);
    VMeasurementData(VContainer *data, quint32 index, QString formula, bool ok, qreal base);
    VMeasurementData(const VMeasurementData &m) = default;
    ~VMeasurementData() = default;

    QSharedPointer<VContainer> data{}; // NOLINT(misc-non-private-member-variables-in-classes)
    quint32 index;                     // NOLINT(misc-non-private-member-variables-in-classes)
    QString formula{};                 // NOLINT(misc-non-private-member-variables-in-classes)
    QString gui_text{};                // NOLINT(misc-non-private-member-variables-in-classes)
    QString valueAlias{};              // NOLINT(misc-non-private-member-variables-in-classes)
    bool formulaOk{true};              // NOLINT(misc-non-private-member-variables-in-classes)

    qreal currentBaseA{0}; // NOLINT(misc-non-private-member-variables-in-classes)
    qreal currentBaseB{0}; // NOLINT(misc-non-private-member-variables-in-classes)
    qreal currentBaseC{0}; // NOLINT(misc-non-private-member-variables-in-classes)

    qreal shiftBase{0}; // NOLINT(misc-non-private-member-variables-in-classes)

    qreal shiftA{0}; // NOLINT(misc-non-private-member-variables-in-classes)
    qreal shiftB{0}; // NOLINT(misc-non-private-member-variables-in-classes)
    qreal shiftC{0}; // NOLINT(misc-non-private-member-variables-in-classes)

    qreal stepA{0}; // NOLINT(misc-non-private-member-variables-in-classes)
    qreal stepB{0}; // NOLINT(misc-non-private-member-variables-in-classes)
    qreal stepC{0}; // NOLINT(misc-non-private-member-variables-in-classes)

    qreal baseA{0}; // NOLINT(misc-non-private-member-variables-in-classes)
    qreal baseB{0}; // NOLINT(misc-non-private-member-variables-in-classes)
    qreal baseC{0}; // NOLINT(misc-non-private-member-variables-in-classes)

    QMap<QString, VMeasurementCorrection> corrections{}; // NOLINT(misc-non-private-member-variables-in-classes)

    bool specialUnits{false}; // NOLINT(misc-non-private-member-variables-in-classes)

    IMD dimension{IMD::N}; // NOLINT(misc-non-private-member-variables-in-classes)

    MeasurementType varType{MeasurementType::Measurement}; // NOLINT(misc-non-private-member-variables-in-classes)

    VPatternImage image{}; // NOLINT(misc-non-private-member-variables-in-classes)

    QUuid dbId{}; // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VMeasurementData) // NOLINT
};

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
inline VMeasurementData::VMeasurementData(quint32 index, MeasurementType varType)
  : index(index),
    varType(varType)
{
}

//---------------------------------------------------------------------------------------------------------------------
inline VMeasurementData::VMeasurementData(quint32 index, qreal baseA, qreal baseB, qreal baseC, qreal base)
  : index(index),
    shiftBase(base),
    baseA(baseA),
    baseB(baseB),
    baseC(baseC)
{
}

//---------------------------------------------------------------------------------------------------------------------
inline VMeasurementData::VMeasurementData(VContainer *data, quint32 index, QString formula, bool ok, qreal base)
  : data(QSharedPointer<VContainer>(new VContainer(*data))),
    index(index),
    formula(std::move(formula)),
    formulaOk(ok),
    shiftBase(base)
{
}

#endif // VMEASUREMENT_P_H
