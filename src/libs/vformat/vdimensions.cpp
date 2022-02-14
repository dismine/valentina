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
{}

//---------------------------------------------------------------------------------------------------------------------
VAbstartMeasurementDimension::VAbstartMeasurementDimension(Unit units, qreal min, qreal max, qreal step)
    : m_units(units),
      m_minValue(min),
      m_maxValue(max),
      m_step(step)
{}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::IsValid() -> bool
{
    m_error.clear();
    return IsUnitsValid() && IsRangeValid() && IsStepValid() && IsBaseValid();
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::RangeMin() const -> int
{
    if (m_circumference)
    {
        const int rangeMinCm = 20;
        const int rangeMinMm = 200;
        const int rangeMinInch = 8;

        switch(Units())
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
auto VAbstartMeasurementDimension::RangeMax() const -> int
{
    if (m_circumference)
    {
        const int rangeMaxCm = 272;
        const int rangeMaxMm = 2720;
        const int rangeMaxInch = 107;

        switch(Units())
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

    return 100;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::Name() const -> QString
{
    return m_customName.isEmpty() ? DimensionName(Type()) : m_customName;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::ValidSteps() const -> QVector<qreal>
{
    const qreal stepBarrier = 8.5;
    const qreal s = 0.5;

    QVector<qreal> steps;
    steps.reserve(qRound((stepBarrier - s) * 2 - 1));

    const qreal diff = m_maxValue - m_minValue;
    if (qFuzzyIsNull(diff))
    {
        steps.append(0); // only one possible value
    }
    else if (diff > 0)
    {
        qreal candidate = 1;
        do
        {
            const qreal step = (m_units == Unit::Mm ? candidate * 10 : candidate);
            qreal intpart;
            if (qFuzzyIsNull(std::modf(diff / step, &intpart)))
            {
                steps.append(step);
            }
            candidate += s;
        }
        while(candidate < stepBarrier);
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
    QVector<qreal> bases = ValidBases();
    QStringList list;
    list.reserve(bases.size());
    for(auto &base : bases)
    {
        list.append(QString::number(base));
    }
    return list;

}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::ValidBases(qreal min, qreal max, qreal step,
                                              const QSet<qreal> &exclude) -> QVector<qreal>
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
    do
    {
        if (not exclude.contains(value))
        {
            validBases.append(value);
        }
        value += step;
    }
    while(value < max + step);

    if (validBases.isEmpty())
    {
        value = min;
        do
        {
            validBases.append(value);
            value += step;
        }
        while(value < max + step);
    }

    return validBases;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::IsRangeValid() -> bool
{
    bool valid = m_minValue > 0 && m_maxValue > 0 && m_minValue >= RangeMin() && m_minValue <= RangeMax()
                 && m_minValue <= m_maxValue;

    if (not valid)
    {
        m_error = tr("Invalid min/max range");
    }

    return valid;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::IsStepValid() -> bool
{
    bool valid = ValidSteps().indexOf(m_step) != -1;
    if (not valid)
    {
        m_error = tr("Invalid step");
    }

    return valid;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::IsBaseValid() -> bool
{
    bool valid = ValidBases().indexOf(m_baseValue) != -1;
    if (not valid)
    {
        m_error = tr("Base value invalid");
    }

    return valid;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::IsUnitsValid() const -> bool
{
    return m_units == Unit::Cm || m_units == Unit::Mm || m_units == Unit::Inch;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstartMeasurementDimension::DimensionName(MeasurementDimension type) -> QString
{
    switch(type)
    {
        case MeasurementDimension::X:
            return tr("Height", "dimension");
        case MeasurementDimension::Y:
            return tr("Size", "dimension");
        case MeasurementDimension::W:
            return tr("Waist", "dimension");
        case MeasurementDimension::Z:
            return tr("Hip", "dimension");
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

    switch(dimension->Type())
    {
        case MeasurementDimension::Y:
            if (dimension->CustomName().isEmpty() && dimension->IsCircumference())
            {
                return fc ? tr("Chest full circumference", "dimension") : tr("Chest half circumference", "dimension");
            }
            return {};
        case MeasurementDimension::W:
            if (dimension->CustomName().isEmpty() && dimension->IsCircumference())
            {
                return fc ? tr("Waist full circumference", "dimension") : tr("Waist half circumference", "dimension");
            }
            return {};
        case MeasurementDimension::Z:
            if (dimension->CustomName().isEmpty() && dimension->IsCircumference())
            {
                return fc ? tr("Hip full circumference", "dimension") : tr("Hip half circumference", "dimension");
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
{}

//---------------------------------------------------------------------------------------------------------------------
VXMeasurementDimension::VXMeasurementDimension(Unit units, qreal min, qreal max, qreal step)
    : VAbstartMeasurementDimension(units, min, max, step)
{}

// VYMeasurementDimension
//---------------------------------------------------------------------------------------------------------------------
VYMeasurementDimension::VYMeasurementDimension(Unit units)
    : VAbstartMeasurementDimension(units)
{}

//---------------------------------------------------------------------------------------------------------------------
VYMeasurementDimension::VYMeasurementDimension(Unit units, qreal min, qreal max, qreal step)
    : VAbstartMeasurementDimension(units, min, max, step)
{}

// VWMeasurementDimension
//---------------------------------------------------------------------------------------------------------------------
VWMeasurementDimension::VWMeasurementDimension(Unit units)
    : VAbstartMeasurementDimension(units)
{}

//---------------------------------------------------------------------------------------------------------------------
VWMeasurementDimension::VWMeasurementDimension(Unit units, qreal min, qreal max, qreal step)
    : VAbstartMeasurementDimension(units, min, max, step)
{}

// VZMeasurementDimension
//---------------------------------------------------------------------------------------------------------------------
VZMeasurementDimension::VZMeasurementDimension(Unit units)
    : VAbstartMeasurementDimension(units)
{}

//---------------------------------------------------------------------------------------------------------------------
VZMeasurementDimension::VZMeasurementDimension(Unit units, qreal min, qreal max, qreal step)
    : VAbstartMeasurementDimension(units, min, max, step)
{}

// VDimensionRestriction
//---------------------------------------------------------------------------------------------------------------------
VDimensionRestriction::VDimensionRestriction(qreal min, qreal max, const QString &exclude) :
    m_min(min),
    m_max(max)
{
    SetExcludeString(exclude);
}

//---------------------------------------------------------------------------------------------------------------------
void VDimensionRestriction::SetExcludeString(const QString &exclude)
{
    m_exclude.clear();

    QStringList values = exclude.split(';');
    for(auto &value : values)
    {
        bool ok = false;
        qreal val = value.toDouble(&ok);

        if (ok)
        {
            m_exclude.insert(val);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VDimensionRestriction::GetExcludeString() const -> QString
{
    QList<qreal> list = m_exclude.values();
    QStringList excludeList;

    for(auto &value : list)
    {
        excludeList.append(QString::number(value));
    }

    return excludeList.join(';');
}
