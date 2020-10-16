/************************************************************************
 **
 **  @file   vdimensions.h
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
#ifndef VDIMENSIONS_H
#define VDIMENSIONS_H

#include <QCoreApplication>
#include <QMap>

#include "../vmisc/def.h"

enum class MeasurementDimension: qint8
{
    X = 0, // height
    Y = 1, // size (chest half circumference)
    W = 2, // hip half circumference
    Z = 3  // waist half circumference
};

class VAbstartMeasurementDimension;
template <class T> class QSharedPointer;

using MeasurementDimension_p = QSharedPointer<VAbstartMeasurementDimension>;
using DimesionLabels = QMap<int, QString>;

class VAbstartMeasurementDimension
{
    Q_DECLARE_TR_FUNCTIONS(VAbstartMeasurementDimension)
public:
    VAbstartMeasurementDimension() =default;
    explicit VAbstartMeasurementDimension(Unit units);
    VAbstartMeasurementDimension(Unit units, int min, int max, int step);
    virtual ~VAbstartMeasurementDimension() =default;

    virtual MeasurementDimension Type() const =0;

    virtual bool IsValid();

    int  MinValue() const;
    void SetMinValue(int minValue);

    int  MaxValue() const;
    void SetMaxValue(int maxValue);

    int  Step() const;
    void SetStep(int step);

    int  BaseValue() const;
    void SetBaseValue(int baseValue);

    QString Error() const;

    Unit Units() const;

    virtual bool IsCircumference() const;

    virtual int RangeMin() const =0;
    virtual int RangeMax() const =0;

    QVector<int> ValidSteps() const;
    QVector<int> ValidBases() const;
    QStringList  ValidBasesList() const;

    static QVector<int> ValidBases(int min, int max, int step);
    static QString DimensionName(MeasurementDimension type);
    static QString DimensionToolTip(MeasurementDimension type, bool circumference, bool fc);

    DimesionLabels Labels() const;
    void           SetLabels(const DimesionLabels &labels);

protected:
    Unit           m_units{Unit::Cm};
    int            m_minValue{0};
    int            m_maxValue{0};
    int            m_step{-1};
    int            m_baseValue{0};
    QString        m_error{};
    DimesionLabels m_labels{};

    bool IsRangeValid();
    bool IsStepValid();
    bool IsBaseValid();
    bool IsUnitsValid();
};

//---------------------------------------------------------------------------------------------------------------------
inline int VAbstartMeasurementDimension::MinValue() const
{
    return m_minValue;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstartMeasurementDimension::SetMinValue(int minValue)
{
    m_minValue = minValue;
}

//---------------------------------------------------------------------------------------------------------------------
inline int VAbstartMeasurementDimension::MaxValue() const
{
    return m_maxValue;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstartMeasurementDimension::SetMaxValue(int maxValue)
{
    m_maxValue = maxValue;
}

//---------------------------------------------------------------------------------------------------------------------
inline int VAbstartMeasurementDimension::Step() const
{
    return m_step;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstartMeasurementDimension::SetStep(int step)
{
    m_step = step;
}

//---------------------------------------------------------------------------------------------------------------------
inline int VAbstartMeasurementDimension::BaseValue() const
{
    return m_baseValue;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstartMeasurementDimension::SetBaseValue(int baseValue)
{
    m_baseValue = baseValue;
}

//---------------------------------------------------------------------------------------------------------------------
inline QString VAbstartMeasurementDimension::Error() const
{
    return m_error;
}

//---------------------------------------------------------------------------------------------------------------------
inline Unit VAbstartMeasurementDimension::Units() const
{
    return m_units;
}

//---------------------------------------------------------------------------------------------------------------------
inline bool VAbstartMeasurementDimension::IsCircumference() const
{
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
inline DimesionLabels VAbstartMeasurementDimension::Labels() const
{
    return m_labels;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstartMeasurementDimension::SetLabels(const DimesionLabels &labels)
{
    m_labels = labels;
}

// VXMeasurementDimension
//---------------------------------------------------------------------------------------------------------------------
class VXMeasurementDimension : public VAbstartMeasurementDimension
{
public:
    VXMeasurementDimension() =default;
    explicit VXMeasurementDimension(Unit units);
    VXMeasurementDimension(Unit units, int min, int max, int step);

    virtual MeasurementDimension Type() const override;

    virtual int RangeMin() const override;
    virtual int RangeMax() const override;

    virtual bool IsCircumference() const override;
};

//---------------------------------------------------------------------------------------------------------------------
inline bool VXMeasurementDimension::IsCircumference() const
{
    return false;
}

// VYMeasurementDimension
//---------------------------------------------------------------------------------------------------------------------
class VYMeasurementDimension : public VAbstartMeasurementDimension
{
public:
    VYMeasurementDimension() =default;
    explicit VYMeasurementDimension(Unit units);
    VYMeasurementDimension(Unit units, int min, int max, int step);

    virtual MeasurementDimension Type() const override;

    virtual int RangeMin() const override;
    virtual int RangeMax() const override;

    virtual bool IsCircumference() const override;
    void SetCircumference(bool circumference);

private:
    bool m_circumference{true};
};

//---------------------------------------------------------------------------------------------------------------------
inline bool VYMeasurementDimension::IsCircumference() const
{
    return m_circumference;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VYMeasurementDimension::SetCircumference(bool circumference)
{
    m_circumference = circumference;
}

// VWMeasurementDimension
//---------------------------------------------------------------------------------------------------------------------
class VWMeasurementDimension : public VAbstartMeasurementDimension
{
public:
    VWMeasurementDimension() =default;
    explicit VWMeasurementDimension(Unit units);
    VWMeasurementDimension(Unit units, int min, int max, int step);

    virtual MeasurementDimension Type() const override;

    virtual int RangeMin() const override;
    virtual int RangeMax() const override;
};

// VZMeasurementDimension
//---------------------------------------------------------------------------------------------------------------------
class VZMeasurementDimension : public VAbstartMeasurementDimension
{
public:
    VZMeasurementDimension() =default;
    explicit VZMeasurementDimension(Unit units);
    VZMeasurementDimension(Unit units, int min, int max, int step);

    virtual MeasurementDimension Type() const override;

    virtual int RangeMin() const override;
    virtual int RangeMax() const override;
};

#endif // VDIMENSIONS_H
