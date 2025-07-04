/************************************************************************
 **
 **  @file   vdimensions.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 9, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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
#include "vdimensions.h"

#include <QSet>
#include <QVector>
#include <cmath>

//---------------------------------------------------------------------------------------------------------------------
VAbstartMeasurementDimension::VAbstartMeasurementDimension(Unit units)
  : m_units(units)
{
}

//---------------------------------------------------------------------------------------------------------------------
VAbstartMeasurementDimension::VAbstartMeasurementDimension(Unit units, qreal min, qreal max, qreal step)
  : m_units(units),
    m_minValue(min),
    m_maxValue(max),
    m_step(step)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::IsValid() -> bool
{
    m_error.clear();
    return IsUnitsValid() && IsRangeValid() && IsStepValid() && IsBaseValid();
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::Decimals() const -> int
{
    if (m_units == Unit::Mm)
    {
        return 2;
    }
    else if (m_units == Unit::Inch)
    {
        return 5;
    }

    return 3;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::RangeMin() const -> qreal
{
    if (m_measurement)
    {
        const qreal rangeMinCm = 1;
        const qreal rangeMinMm = 10;
        const qreal rangeMinInch = 2.54;

        switch (Units())
        {
            case Unit::Cm:
                return rangeMinCm;
            case Unit::Mm:
                return rangeMinMm;
            case Unit::Inch:
                return rangeMinInch;
            default:
                return 0;
        }
    }

    return 1;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::RangeMax() const -> qreal
{
    if (m_measurement)
    {
        const qreal rangeMaxCm = 272;
        const qreal rangeMaxMm = 2720;
        const qreal rangeMaxInch = 107.09;

        switch (Units())
        {
            case Unit::Cm:
                return rangeMaxCm;
            case Unit::Mm:
                return rangeMaxMm;
            case Unit::Inch:
                return rangeMaxInch;
            default:
                return 0;
        }
    }

    return 272;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::Name() const -> QString
{
    return m_customName.isEmpty() ? DimensionName(Type()) : m_customName;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::ValidSteps() const -> QVector<qreal>
{
    QVector<qreal> steps;

    if (const qreal diff = m_maxValue - m_minValue; qFuzzyIsNull(diff))
    {
        steps.append(0); // only one possible value
    }
    else if (diff > 0)
    {
        qreal step = 0.01;
        if (m_units == Unit::Mm)
        {
            step = 0.1;
        }
        else if (m_units == Unit::Inch)
        {
            step = 0.03937;
        }

        const int stepsCount = qRound(diff / step);
        steps.reserve(stepsCount);

        for (int i = 1; i <= stepsCount; ++i)
        {
            const qreal currentStep = i * step;

            qreal intpart = NAN;
            if (qFuzzyIsNull(std::modf(diff / currentStep, &intpart)))
            {
                steps.append(currentStep);
            }
        }
    }

    return steps;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::ValidBases() const -> QVector<qreal>
{
    return VAbstartMeasurementDimension::ValidBases(m_minValue, m_maxValue, m_step, QSet<qreal>());
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::ValidBasesList() const -> QStringList
{
    QVector<qreal> const bases = ValidBases();
    QStringList list;
    list.reserve(bases.size());
    for (const auto &base : bases)
    {
        list.append(QString::number(base));
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::ValidBases(qreal min, qreal max, qreal step, const QSet<qreal> &exclude)
    -> QVector<qreal>
{
    QVector<qreal> validBases;

    if (step < 0 || min > max)
    {
        return validBases;
    }

    if (qFuzzyIsNull(step))
    {
        step = 1;
    }

    validBases.reserve(qRound((max - min) / step));

    qreal value = min;
    int i = 1;
    do
    {
        if (not VFuzzyContains(exclude, value))
        {
            validBases.append(value);
        }
        value = min + step * i;
        ++i;
    } while (value < max + step);

    if (validBases.isEmpty())
    {
        value = min;
        int i = 1;
        do
        {
            validBases.append(value);
            value = min + step * i;
            ++i;
        } while (value < max + step);
    }

    return validBases;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::IsRangeValid() const -> bool
{
    bool const valid = m_minValue > 0 && m_maxValue > 0 &&
                       (m_minValue > RangeMin() || VFuzzyComparePossibleNulls(m_minValue, RangeMin())) &&
                       (m_maxValue < RangeMax() || VFuzzyComparePossibleNulls(m_maxValue, RangeMax())) &&
                       (m_minValue < m_maxValue || VFuzzyComparePossibleNulls(m_minValue, m_maxValue));

    if (not valid)
    {
        m_error = QCoreApplication::translate("VAbstartMeasurementDimension", "Invalid min/max range");
    }

    return valid;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::IsStepValid() const -> bool
{
    bool const valid = VFuzzyIndexOf(ValidSteps(), m_step) != -1;
    if (not valid)
    {
        m_error = QCoreApplication::translate("VAbstartMeasurementDimension", "Invalid step");
    }

    return valid;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::IsBaseValid() const -> bool
{
    bool const valid = VFuzzyIndexOf(ValidBases(), m_baseValue) != -1;
    if (not valid)
    {
        m_error = QCoreApplication::translate("VAbstartMeasurementDimension", "Base value invalid");
    }

    return valid;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::IsUnitsValid() const -> bool
{
    bool const valid = (m_units == Unit::Cm || m_units == Unit::Mm || m_units == Unit::Inch);

    if (not valid)
    {
        m_error = QCoreApplication::translate("VAbstartMeasurementDimension", "Units are invalid");
    }

    return valid;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::DimensionName(MeasurementDimension type) -> QString
{
    switch (type)
    {
        case MeasurementDimension::X:
            return QCoreApplication::translate("VAbstartMeasurementDimension", "Height", "dimension");
        case MeasurementDimension::Y:
            return QCoreApplication::translate("VAbstartMeasurementDimension", "Size", "dimension");
        case MeasurementDimension::W:
            return QCoreApplication::translate("VAbstartMeasurementDimension", "Waist", "dimension");
        case MeasurementDimension::Z:
            return QCoreApplication::translate("VAbstartMeasurementDimension", "Hip", "dimension");
        default:
            return {};
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::DimensionToolTip(const MeasurementDimension_p &dimension, bool fc) -> QString
{
    if (dimension.isNull())
    {
        return {};
    }

    switch (dimension->Type())
    {
        case MeasurementDimension::Y:
            if (dimension->CustomName().isEmpty() && dimension->IsBodyMeasurement())
            {
                return fc ? QCoreApplication::translate("VAbstartMeasurementDimension", "Chest full circumference",
                                                        "dimension")
                          : QCoreApplication::translate("VAbstartMeasurementDimension", "Chest half circumference",
                                                        "dimension");
            }
            return {};
        case MeasurementDimension::W:
            if (dimension->CustomName().isEmpty() && dimension->IsBodyMeasurement())
            {
                return fc ? QCoreApplication::translate("VAbstartMeasurementDimension", "Waist full circumference",
                                                        "dimension")
                          : QCoreApplication::translate("VAbstartMeasurementDimension", "Waist half circumference",
                                                        "dimension");
            }
            return {};
        case MeasurementDimension::Z:
            if (dimension->CustomName().isEmpty() && dimension->IsBodyMeasurement())
            {
                return fc ? QCoreApplication::translate("VAbstartMeasurementDimension", "Hip full circumference",
                                                        "dimension")
                          : QCoreApplication::translate("VAbstartMeasurementDimension", "Hip half circumference",
                                                        "dimension");
            }
            return {};
        case MeasurementDimension::X:
        default:
            return {};
    }
}

// VXMeasurementDimension
//---------------------------------------------------------------------------------------------------------------------
VXMeasurementDimension::VXMeasurementDimension(Unit units)
  : VAbstartMeasurementDimension(units)
{
}

//---------------------------------------------------------------------------------------------------------------------
VXMeasurementDimension::VXMeasurementDimension(Unit units, qreal min, qreal max, qreal step)
  : VAbstartMeasurementDimension(units, min, max, step)
{
}

// VYMeasurementDimension
//---------------------------------------------------------------------------------------------------------------------
VYMeasurementDimension::VYMeasurementDimension(Unit units)
  : VAbstartMeasurementDimension(units)
{
}

//---------------------------------------------------------------------------------------------------------------------
VYMeasurementDimension::VYMeasurementDimension(Unit units, qreal min, qreal max, qreal step)
  : VAbstartMeasurementDimension(units, min, max, step)
{
}

// VWMeasurementDimension
//---------------------------------------------------------------------------------------------------------------------
VWMeasurementDimension::VWMeasurementDimension(Unit units)
  : VAbstartMeasurementDimension(units)
{
}

//---------------------------------------------------------------------------------------------------------------------
VWMeasurementDimension::VWMeasurementDimension(Unit units, qreal min, qreal max, qreal step)
  : VAbstartMeasurementDimension(units, min, max, step)
{
}

// VZMeasurementDimension
//---------------------------------------------------------------------------------------------------------------------
VZMeasurementDimension::VZMeasurementDimension(Unit units)
  : VAbstartMeasurementDimension(units)
{
}

//---------------------------------------------------------------------------------------------------------------------
VZMeasurementDimension::VZMeasurementDimension(Unit units, qreal min, qreal max, qreal step)
  : VAbstartMeasurementDimension(units, min, max, step)
{
}

// VDimensionRestriction
//---------------------------------------------------------------------------------------------------------------------
VDimensionRestriction::VDimensionRestriction(qreal min, qreal max, const QString &exclude)
  : m_min(min),
    m_max(max)
{
    SetExcludeString(exclude);
}

//---------------------------------------------------------------------------------------------------------------------
void VDimensionRestriction::SetExcludeString(const QString &exclude)
{
    m_exclude.clear();

    QStringList const values = exclude.split(';');
    for (const auto &value : values)
    {
        bool ok = false;
        qreal const val = value.toDouble(&ok);

        if (ok)
        {
            m_exclude.insert(val);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VDimensionRestriction::GetExcludeString() const -> QString
{
    QList<qreal> const list = m_exclude.values();
    QStringList excludeList;
    excludeList.reserve(list.size());

    for (const auto &value : list)
    {
        excludeList.append(QString::number(value));
    }

    return excludeList.join(';');
}
