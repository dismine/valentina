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

//---------------------------------------------------------------------------------------------------------------------
VAbstartMeasurementDimension::VAbstartMeasurementDimension(Unit units)
    : m_units(units)
{}

//---------------------------------------------------------------------------------------------------------------------
VAbstartMeasurementDimension::VAbstartMeasurementDimension(Unit units, int min, int max, int step)
    : m_units(units),
      m_minValue(min),
      m_maxValue(max),
      m_step(step)
{}

//---------------------------------------------------------------------------------------------------------------------
bool VAbstartMeasurementDimension::IsValid()
{
    m_error.clear();
    return IsUnitsValid() && IsRangeValid() && IsStepValid() && IsBaseValid();
}

//---------------------------------------------------------------------------------------------------------------------
QVector<int> VAbstartMeasurementDimension::ValidSteps() const
{
    QVector<int> steps;

    const int diff = m_maxValue - m_minValue;
    if (diff == 0)
    {
        steps.append(0); // only one possible value
    }
    else if (diff > 0)
    {
        for (int i=1; i < 9; ++i)
        {
            const int step = (m_units == Unit::Mm ? i * 10 : i);
            if (diff % step == 0)
            {
                steps.append(step);
            }
        }
    }

    return steps;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<int> VAbstartMeasurementDimension::ValidBases() const
{
    QVector<int> validBases;
    int step = Step();

    if (step < 0)
    {
        return validBases;
    }
    else if (step == 0)
    {
        step = 1;
    }

    for (int value = MinValue(); value <= MaxValue(); value += step)
    {
        validBases.append(value);
    }

    return validBases;
}

//---------------------------------------------------------------------------------------------------------------------
QStringList VAbstartMeasurementDimension::ValidBasesList() const
{
    QVector<int> bases = ValidBases();
    QStringList list;
    for(auto &base : bases)
    {
        list.append(QString::number(base));
    }
    return list;

}

//---------------------------------------------------------------------------------------------------------------------
bool VAbstartMeasurementDimension::IsRangeValid()
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
bool VAbstartMeasurementDimension::IsStepValid()
{
    bool valid = ValidSteps().indexOf(m_step) != -1;
    if (not valid)
    {
        m_error = tr("Invalid step");
    }

    return valid;
}

//---------------------------------------------------------------------------------------------------------------------
bool VAbstartMeasurementDimension::IsBaseValid()
{
    bool valid = ValidBases().indexOf(m_baseValue) != -1;
    if (not valid)
    {
        m_error = tr("Base value invalid");
    }

    return valid;
}

//---------------------------------------------------------------------------------------------------------------------
bool VAbstartMeasurementDimension::IsUnitsValid()
{
    return m_units == Unit::Cm || m_units == Unit::Mm || m_units == Unit::Inch;
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstartMeasurementDimension::DimensionName(MeasurementDimension type)
{
    switch(type)
    {
    case MeasurementDimension::X:
        return tr("Height");
    case MeasurementDimension::Y:
        return tr("Size");
    case MeasurementDimension::W:
        return tr("Hip");
    case MeasurementDimension::Z:
        return tr("Waist");
    default:
        return QString();
    }
}

//---------------------------------------------------------------------------------------------------------------------
QString VAbstartMeasurementDimension::DimensionToolTip(MeasurementDimension type, bool circumference, bool fc)
{
    switch(type)
    {
    case MeasurementDimension::X:
        return tr("Height");
    case MeasurementDimension::Y:
        if (circumference)
        {
            return fc ? tr("Chest full circumference") : tr("Chest half circumference");
        }
        else
        {
            return tr("Size");
        }
        return circumference ? tr("Chest circumference") : tr("Size");
    case MeasurementDimension::W:
        return fc ? tr("Hip full circumference") : tr("Hip half circumference");
    case MeasurementDimension::Z:
        return fc ? tr("Waist full circumference") : tr("Waist half circumference");
    default:
        return QString();
    }
}

// VXMeasurementDimension
//---------------------------------------------------------------------------------------------------------------------
VXMeasurementDimension::VXMeasurementDimension(Unit units)
    : VAbstartMeasurementDimension(units)
{}

//---------------------------------------------------------------------------------------------------------------------
VXMeasurementDimension::VXMeasurementDimension(Unit units, int min, int max, int step)
    : VAbstartMeasurementDimension(units, min, max, step)
{}

//---------------------------------------------------------------------------------------------------------------------
MeasurementDimension VXMeasurementDimension::Type() const
{
    return MeasurementDimension::X;
}

//---------------------------------------------------------------------------------------------------------------------
int VXMeasurementDimension::RangeMin() const
{
    switch(m_units)
    {
        case Unit::Cm:
            return 50;
        case Unit::Mm:
            return 500;
        case Unit::Inch:
            return 19;
        default:
            return 0;
    }
}

//---------------------------------------------------------------------------------------------------------------------
int VXMeasurementDimension::RangeMax() const
{
    switch(m_units)
    {
        case Unit::Cm:
            return 272;
        case Unit::Mm:
            return 2720;
        case Unit::Inch:
            return 107;
        default:
            return 0;
    }
}

// VYMeasurementDimension
//---------------------------------------------------------------------------------------------------------------------
VYMeasurementDimension::VYMeasurementDimension(Unit units)
    : VAbstartMeasurementDimension(units)
{}

//---------------------------------------------------------------------------------------------------------------------
VYMeasurementDimension::VYMeasurementDimension(Unit units, int min, int max, int step)
    : VAbstartMeasurementDimension(units, min, max, step)
{}

//---------------------------------------------------------------------------------------------------------------------
MeasurementDimension VYMeasurementDimension::Type() const
{
    return MeasurementDimension::Y;
}

//---------------------------------------------------------------------------------------------------------------------
int VYMeasurementDimension::RangeMin() const
{
    if (m_circumference)
    {
        switch(m_units)
        {
            case Unit::Cm:
                return 22;
            case Unit::Mm:
                return 220;
            case Unit::Inch:
                return 8;
            default:
                return 0;
        }
    }
    else
    {
        return 6;
    }
}

//---------------------------------------------------------------------------------------------------------------------
int VYMeasurementDimension::RangeMax() const
{
    if (m_circumference)
    {
        switch(m_units)
        {
            case Unit::Cm:
                return 72;
            case Unit::Mm:
                return 720;
            case Unit::Inch:
                return 29;
            default:
                return 0;
        }
    }
    else
    {
        return 58;
    }
}

// VWMeasurementDimension
//---------------------------------------------------------------------------------------------------------------------
VWMeasurementDimension::VWMeasurementDimension(Unit units)
    : VAbstartMeasurementDimension(units)
{}

//---------------------------------------------------------------------------------------------------------------------
VWMeasurementDimension::VWMeasurementDimension(Unit units, int min, int max, int step)
    : VAbstartMeasurementDimension(units, min, max, step)
{}

//---------------------------------------------------------------------------------------------------------------------
MeasurementDimension VWMeasurementDimension::Type() const
{
    return MeasurementDimension::W;
}

//---------------------------------------------------------------------------------------------------------------------
int VWMeasurementDimension::RangeMin() const
{
    switch(m_units)
    {
        case Unit::Cm:
            return 33;
        case Unit::Mm:
            return 330;
        case Unit::Inch:
            return 12;
        default:
            return 0;
    }
}

//---------------------------------------------------------------------------------------------------------------------
int VWMeasurementDimension::RangeMax() const
{
    switch(m_units)
    {
        case Unit::Cm:
            return 63;
        case Unit::Mm:
            return 630;
        case Unit::Inch:
            return 25;
        default:
            return 0;
    }
}

// VZMeasurementDimension
//---------------------------------------------------------------------------------------------------------------------
VZMeasurementDimension::VZMeasurementDimension(Unit units)
    : VAbstartMeasurementDimension(units)
{}

//---------------------------------------------------------------------------------------------------------------------
VZMeasurementDimension::VZMeasurementDimension(Unit units, int min, int max, int step)
    : VAbstartMeasurementDimension(units, min, max, step)
{}

//---------------------------------------------------------------------------------------------------------------------
MeasurementDimension VZMeasurementDimension::Type() const
{
    return MeasurementDimension::Z;
}

//---------------------------------------------------------------------------------------------------------------------
int VZMeasurementDimension::RangeMin() const
{
    switch(m_units)
    {
        case Unit::Cm:
            return 33;
        case Unit::Mm:
            return 330;
        case Unit::Inch:
            return 12;
        default:
            return 0;
    }
}

//---------------------------------------------------------------------------------------------------------------------
int VZMeasurementDimension::RangeMax() const
{
    switch(m_units)
    {
        case Unit::Cm:
            return 63;
        case Unit::Mm:
            return 630;
        case Unit::Inch:
            return 25;
        default:
            return 0;
    }
}
