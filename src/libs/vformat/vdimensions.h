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
#include "qglobal.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 13, 0)

enum class MeasurementDimension : qint8
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

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline auto VFuzzyContains(const QMap<qreal, T> &c, qreal value) -> bool
{
    auto i = c.constBegin();
    while (i != c.constEnd())
    {
        if (VFuzzyComparePossibleNulls(i.key(), value))
        {
            return true;
        }
        ++i;
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
template <template <typename> class Cont> inline auto VFuzzyContains(const Cont<qreal> &c, qreal value) -> bool
{
    for (auto val : c)
    {
        if (VFuzzyComparePossibleNulls(val, value))
        {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
template <template <typename> class Cont> inline auto VFuzzyIndexOf(const Cont<qreal> &c, qreal value) -> vsizetype
{
    for (int i = 0; i < c.size(); ++i)
    {
        if (VFuzzyComparePossibleNulls(c.at(i), value))
        {
            return i;
        }
    }
    return -1;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline auto VFuzzyValue(const QMap<qreal, T> &c, qreal value, const T &defaultValue = T()) -> T
{
    auto i = c.constBegin();
    while (i != c.constEnd())
    {
        if (VFuzzyComparePossibleNulls(i.key(), value))
        {
            return i.value();
        }
        ++i;
    }
    return defaultValue;
}

class VAbstartMeasurementDimension
{
    Q_DECLARE_TR_FUNCTIONS(VAbstartMeasurementDimension) // NOLINT

public:
    VAbstartMeasurementDimension() = default;
    explicit VAbstartMeasurementDimension(Unit units);
    VAbstartMeasurementDimension(Unit units, qreal min, qreal max, qreal step);
    virtual ~VAbstartMeasurementDimension() = default;

    virtual auto Type() const -> MeasurementDimension = 0;

    auto IsValid() -> bool;

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

    auto IsBodyMeasurement() const -> bool;
    void SetBodyMeasurement(bool measurement);

    auto RangeMin() const -> int;
    auto RangeMax() const -> int;

    virtual auto Axis() const -> QChar = 0;

    auto Name() const -> QString;

    auto ValidSteps() const -> QVector<qreal>;
    auto ValidBases() const -> QVector<qreal>;
    auto ValidBasesList() const -> QStringList;

    static auto ValidBases(qreal min, qreal max, qreal step, const QSet<qreal> &exclude) -> QVector<qreal>;
    static auto DimensionName(MeasurementDimension type) -> QString;
    static auto DimensionToolTip(const MeasurementDimension_p &dimension, bool fc) -> QString;

    auto Labels() const -> DimesionLabels;
    void SetLabels(const DimesionLabels &labels);

    auto CustomName() const -> const QString &;
    void SetCustomName(const QString &newCustomName);

protected:
    auto IsRangeValid() const -> bool;
    auto IsStepValid() const -> bool;
    auto IsBaseValid() const -> bool;
    auto IsUnitsValid() const -> bool;

private:
    Q_DISABLE_COPY_MOVE(VAbstartMeasurementDimension) // NOLINT

    Unit m_units{Unit::Cm};
    qreal m_minValue{0};
    qreal m_maxValue{0};
    qreal m_step{-1};
    qreal m_baseValue{0};
    mutable QString m_error{};
    DimesionLabels m_labels{};
    bool m_measurement{true};
    QString m_customName{};
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
inline auto VAbstartMeasurementDimension::IsBodyMeasurement() const -> bool
{
    return m_measurement;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstartMeasurementDimension::SetBodyMeasurement(bool measurement)
{
    m_measurement = measurement;
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

//---------------------------------------------------------------------------------------------------------------------
inline auto VAbstartMeasurementDimension::CustomName() const -> const QString &
{
    return m_customName;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VAbstartMeasurementDimension::SetCustomName(const QString &newCustomName)
{
    m_customName = newCustomName;
}

// VXMeasurementDimension
//---------------------------------------------------------------------------------------------------------------------
class VXMeasurementDimension : public VAbstartMeasurementDimension
{
public:
    VXMeasurementDimension() = default;
    explicit VXMeasurementDimension(Unit units);
    VXMeasurementDimension(Unit units, qreal min, qreal max, qreal step);
    ~VXMeasurementDimension() override = default;

    auto Type() const -> MeasurementDimension override;

    auto Axis() const -> QChar override;

private:
    Q_DISABLE_COPY_MOVE(VXMeasurementDimension) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VXMeasurementDimension::Type() const -> MeasurementDimension
{
    return MeasurementDimension::X;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VXMeasurementDimension::Axis() const -> QChar
{
    return QLatin1Char('X');
}

// VYMeasurementDimension
//---------------------------------------------------------------------------------------------------------------------
class VYMeasurementDimension : public VAbstartMeasurementDimension
{
public:
    VYMeasurementDimension() = default;
    explicit VYMeasurementDimension(Unit units);
    VYMeasurementDimension(Unit units, qreal min, qreal max, qreal step);
    ~VYMeasurementDimension() override = default;

    auto Type() const -> MeasurementDimension override;

    auto Axis() const -> QChar override;

private:
    Q_DISABLE_COPY_MOVE(VYMeasurementDimension) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VYMeasurementDimension::Type() const -> MeasurementDimension
{
    return MeasurementDimension::Y;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VYMeasurementDimension::Axis() const -> QChar
{
    return QLatin1Char('Y');
}

// VWMeasurementDimension
//---------------------------------------------------------------------------------------------------------------------
class VWMeasurementDimension : public VAbstartMeasurementDimension
{
public:
    VWMeasurementDimension() = default;
    explicit VWMeasurementDimension(Unit units);
    VWMeasurementDimension(Unit units, qreal min, qreal max, qreal step);
    ~VWMeasurementDimension() override = default;

    auto Type() const -> MeasurementDimension override;

    auto Axis() const -> QChar override;

private:
    Q_DISABLE_COPY_MOVE(VWMeasurementDimension) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VWMeasurementDimension::Type() const -> MeasurementDimension
{
    return MeasurementDimension::W;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VWMeasurementDimension::Axis() const -> QChar
{
    return QLatin1Char('W');
}

// VZMeasurementDimension
//---------------------------------------------------------------------------------------------------------------------
class VZMeasurementDimension : public VAbstartMeasurementDimension
{
public:
    VZMeasurementDimension() = default;
    explicit VZMeasurementDimension(Unit units);
    VZMeasurementDimension(Unit units, qreal min, qreal max, qreal step);
    ~VZMeasurementDimension() override = default;

    auto Type() const -> MeasurementDimension override;

    auto Axis() const -> QChar override;

private:
    Q_DISABLE_COPY_MOVE(VZMeasurementDimension) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VZMeasurementDimension::Type() const -> MeasurementDimension
{
    return MeasurementDimension::Z;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VZMeasurementDimension::Axis() const -> QChar
{
    return QLatin1Char('Z');
}

class VDimensionRestriction
{
public:
    VDimensionRestriction() = default;
    VDimensionRestriction(qreal min, qreal max, const QString &exclude = QString());

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
