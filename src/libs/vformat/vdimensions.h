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
#include <QSet>

#include "../vmisc/def.h"

enum class MeasurementDimension: qint8
{
    X = 0, // height
    Y = 1, // size (chest half circumference)
    W = 2, // waist half circumference
    Z = 3  // hip half circumference
};

class VAbstartMeasurementDimension;
template <class T> class QSharedPointer;

using MeasurementDimension_p = QSharedPointer<VAbstartMeasurementDimension>;
using DimesionLabels = QMap<qreal, QString>;

class VAbstartMeasurementDimension
{
    Q_DECLARE_TR_FUNCTIONS(VAbstartMeasurementDimension)
public:
    VAbstartMeasurementDimension() =default;
    explicit VAbstartMeasurementDimension(Unit units);
    VAbstartMeasurementDimension(Unit units, qreal min, qreal max, qreal step);
    virtual ~VAbstartMeasurementDimension() =default;

    virtual MeasurementDimension Type() const =0;

    virtual bool IsValid();

    auto MinValue() const -> qreal;
    void SetMinValue(qreal minValue);

    auto MaxValue() const -> qreal;
    void SetMaxValue(qreal maxValue);

    auto Step() const -> qreal;
    void SetStep(qreal step);

    auto BaseValue() const -> qreal;
    void SetBaseValue(qreal baseValue);

    auto Error() const -> QString;

    auto Units() const -> Unit;

    virtual auto IsCircumference() const -> bool;

    virtual auto RangeMin() const -> int =0;
    virtual auto RangeMax() const -> int =0;

    auto ValidSteps() const -> QVector<qreal>;
    auto ValidBases() const -> QVector<qreal>;
    auto ValidBasesList() const -> QStringList;

    static auto ValidBases(qreal min, qreal max, qreal step, const QSet<qreal> &exclude) -> QVector<qreal>;
    static auto DimensionName(MeasurementDimension type) -> QString;
    static auto DimensionToolTip(MeasurementDimension type, bool circumference, bool fc) -> QString;

    auto Labels() const -> DimesionLabels;
    void SetLabels(const DimesionLabels &labels);

protected:
    Unit           m_units{Unit::Cm};
    qreal          m_minValue{0};
    qreal          m_maxValue{0};
    qreal          m_step{-1};
    qreal          m_baseValue{0};
    QString        m_error{};
    DimesionLabels m_labels{};

    auto IsRangeValid() -> bool;
    auto IsStepValid() -> bool;
    auto IsBaseValid() -> bool;
    auto IsUnitsValid() const -> bool;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstartMeasurementDimension::MinValue() const -> qreal
{
    return m_minValue;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstartMeasurementDimension::SetMinValue(qreal minValue)
{
    m_minValue = minValue;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstartMeasurementDimension::MaxValue() const -> qreal
{
    return m_maxValue;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstartMeasurementDimension::SetMaxValue(qreal maxValue)
{
    m_maxValue = maxValue;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstartMeasurementDimension::Step() const -> qreal
{
    return m_step;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstartMeasurementDimension::SetStep(qreal step)
{
    m_step = step;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstartMeasurementDimension::BaseValue() const -> qreal
{
    return m_baseValue;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstartMeasurementDimension::SetBaseValue(qreal baseValue)
{
    m_baseValue = baseValue;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstartMeasurementDimension::Error() const -> QString
{
    return m_error;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstartMeasurementDimension::Units() const -> Unit
{
    return m_units;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstartMeasurementDimension::IsCircumference() const -> bool
{
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstartMeasurementDimension::Labels() const -> DimesionLabels
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
    VXMeasurementDimension(Unit units, qreal min, qreal max, qreal step);

    virtual auto Type() const -> MeasurementDimension override;

    virtual auto RangeMin() const -> int override;
    virtual auto RangeMax() const -> int override;

    virtual bool IsCircumference() const override;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VXMeasurementDimension::IsCircumference() const -> bool
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
    VYMeasurementDimension(Unit units, qreal min, qreal max, qreal step);

    virtual auto Type() const -> MeasurementDimension override;

    virtual auto RangeMin() const -> int override;
    virtual auto RangeMax() const -> int override;

    virtual auto IsCircumference() const -> bool override;
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
    VWMeasurementDimension(Unit units, qreal min, qreal max, qreal step);

    virtual auto Type() const -> MeasurementDimension override;

    virtual auto RangeMin() const -> int override;
    virtual auto RangeMax() const -> int override;
};

// VZMeasurementDimension
//---------------------------------------------------------------------------------------------------------------------
class VZMeasurementDimension : public VAbstartMeasurementDimension
{
public:
    VZMeasurementDimension() =default;
    explicit VZMeasurementDimension(Unit units);
    VZMeasurementDimension(Unit units, qreal min, qreal max, qreal step);

    virtual auto Type() const -> MeasurementDimension override;

    virtual auto RangeMin() const -> int override;
    virtual auto RangeMax() const -> int override;
};

class VDimensionRestriction
{
public:
    VDimensionRestriction()
    {}

    VDimensionRestriction(qreal min, qreal max, const QString &exclude = QString()) :
        m_min(min),
        m_max(max)
    {
        SetExcludeString(exclude);
    }

    void SetMin(qreal min);
    auto GetMin() const -> qreal;

    void SetMax(qreal max);
    auto GetMax() const -> qreal;

    void SetExcludeString(const QString &exclude);
    auto GetExcludeString() const -> QString;

    void SetExcludeValues(const QSet<qreal> &exclude);
    auto GetExcludeValues() const -> QSet<qreal>;
private:
    qreal m_min{0};
    qreal m_max{0};
    QSet<qreal> m_exclude{};
};

//---------------------------------------------------------------------------------------------------------------------
inline void VDimensionRestriction::SetMin(qreal min)
{
    m_min = min;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VDimensionRestriction::GetMin() const -> qreal
{
    return m_min;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VDimensionRestriction::SetMax(qreal max)
{
    m_max = max;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VDimensionRestriction::GetMax() const -> qreal
{
    return m_max;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VDimensionRestriction::SetExcludeValues(const QSet<qreal> &exclude)
{
    m_exclude = exclude;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VDimensionRestriction::GetExcludeValues() const -> QSet<qreal>
{
    return m_exclude;
}

#endif // VDIMENSIONS_H
