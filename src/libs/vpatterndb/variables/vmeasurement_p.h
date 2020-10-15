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

#include "../vcontainer.h"
#include "../vmisc/diagnostic.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VMeasurementData : public QSharedData
{
public:

    VMeasurementData(quint32 index, qreal baseA, qreal baseB, qreal baseC, qreal base)
        : index(index),
          shiftBase(base),
          baseA(baseA),
          baseB(baseB),
          baseC(baseC)
    {}

    VMeasurementData(VContainer *data, quint32 index, const QString &formula, bool ok, qreal base)
        : data(QSharedPointer<VContainer>(new VContainer(*data))),
          index(index),
          formula(formula),
          formulaOk(ok),
          shiftBase(base)
    {}

    VMeasurementData(const VMeasurementData &m)
        : QSharedData(m),
          data(m.data),
          index(m.index),
          formula(m.formula),
          gui_text(m.gui_text),
          formulaOk(m.formulaOk),
          currentBaseA(m.currentBaseA),
          currentBaseB(m.currentBaseB),
          currentBaseC(m.currentBaseC),
          shiftBase(m.shiftBase),
          shiftA(m.shiftA),
          shiftB(m.shiftB),
          shiftC(m.shiftC),
          stepA(m.stepA),
          stepB(m.stepB),
          stepC(m.stepC),
          baseA(m.baseA),
          baseB(m.baseB),
          baseC(m.baseC),
          corrections(m.corrections),
          specialUnits(m.specialUnits),
          dimension(m.dimension)
    {}

    virtual  ~VMeasurementData();

    QSharedPointer<VContainer> data{};
    quint32 index;
    QString formula{};
    QString gui_text{};
    bool formulaOk{true};

    qreal currentBaseA{0};
    qreal currentBaseB{0};
    qreal currentBaseC{0};

    qreal shiftBase{0};

    qreal shiftA{0};
    qreal shiftB{0};
    qreal shiftC{0};

    qreal stepA{0};
    qreal stepB{0};
    qreal stepC{0};

    qreal baseA{0};
    qreal baseB{0};
    qreal baseC{0};

    QMap<QString, qreal> corrections{};

    bool specialUnits{false};

    IMD dimension{IMD::N};

private:
    Q_DISABLE_ASSIGN(VMeasurementData)
};

VMeasurementData::~VMeasurementData()
{}

QT_WARNING_POP

#endif // VMEASUREMENT_P_H
