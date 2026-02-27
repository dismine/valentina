/************************************************************************
 **
 **  @file   vstandardtablecell.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#include "vmeasurement.h"

#include <QMap>
#include <QMessageLogger>
#include <QtDebug>

#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatternimage.h"
#include "../vformat/knownmeasurements/vknownmeasurement.h"
#include "../vformat/knownmeasurements/vknownmeasurements.h"
#include "../vformat/knownmeasurements/vknownmeasurementsdatabase.h"
#include "vmeasurement_p.h"
#include "vvariable.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
VMeasurement::VMeasurement(quint32 index, const QString &name)
  : VVariable(name),
    d(new VMeasurementData(index, MeasurementType::Separator))
{
    SetType(VarType::MeasurementSeparator);
    StoreValue(0);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VMeasurement create measurement for multisize table
 * @param name measurement's name
 * @param base measurement's base value
 */
VMeasurement::VMeasurement(quint32 index, const QString &name, qreal baseA, qreal baseB, qreal baseC, qreal base)
  : VVariable(name),
    d(new VMeasurementData(index, baseA, baseB, baseC, base))
{
    SetType(VarType::Measurement);
    StoreValue(d->shiftBase);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VMeasurement create measurement for individual table
 * @param name measurement's base value
 * @param base value in base size and height
 */
VMeasurement::VMeasurement(VContainer *data, quint32 index, const QString &name, const qreal &base,
                           const QString &formula, bool ok)
  : VVariable(name),
    d(new VMeasurementData(data, index, formula, ok, base))
{
    SetType(VarType::Measurement);
    StoreValue(base);
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL_2(VMeasurement, VVariable)

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::operator=(const VMeasurement &m) -> VMeasurement &
{
    if (&m == this)
    {
        return *this;
    }
    VVariable::operator=(m);
    d = m.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VMeasurement::VMeasurement(VMeasurement &&m) noexcept
  : VVariable(std::move(m)),
    d(std::move(m.d)) // NOLINT(bugprone-use-after-move)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::operator=(VMeasurement &&m) noexcept -> VMeasurement &
{
    VVariable::operator=(m);
    std::swap(d, m.d);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VMeasurement::~VMeasurement() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::GetKnownMeasurementsId() const -> QUuid
{
    return d->dbId;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetKnownMeasurementsId(const QUuid &id)
{
    d->dbId = id;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::CorrectionHash(qreal baseA, qreal baseB, qreal baseC) -> QString
{
    QStringList hashBlocks{QString::number(baseA)};

    if (baseB > 0)
    {
        hashBlocks.append(QString::number(baseB));
    }

    if (baseC > 0)
    {
        hashBlocks.append(QString::number(baseC));
    }
    return hashBlocks.join(';'_L1);
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::CalcValue() const -> qreal
{
    if (qFuzzyIsNull(d->currentBaseA))
    {
        return VInternalVariable::GetValue();
    }

    // Formula for calculation gradation
    const qreal kA = d->stepA > 0 ? (d->currentBaseA - d->baseA) / d->stepA : 0;
    const qreal kB = d->stepB > 0 ? (d->currentBaseB - d->baseB) / d->stepB : 0;
    const qreal kC = d->stepC > 0 ? (d->currentBaseC - d->baseC) / d->stepC : 0;

    return d->shiftBase + kA * d->shiftA + kB * d->shiftB + kC * d->shiftC + Correction();
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::Correction() const -> qreal
{
    if (const QString hash = CorrectionHash(d->currentBaseA, d->currentBaseB, d->currentBaseC);
        d->corrections.contains(hash))
    {
        return d->corrections.value(hash).correction;
    }

    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetGuiText measurement name for tooltip
 * @return measurement name
 */
auto VMeasurement::GetGuiText() const -> QString
{
    return d->gui_text;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetGuiText(const QString &guiText)
{
    d->gui_text = guiText;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::GetFormula() const -> QString
{
    return d->formula;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::IsCustom() const -> bool
{
    return GetName().indexOf(CustomMSign) == 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::Index() const -> int
{
    return static_cast<int>(d->index);
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::IsFormulaOk() const -> bool
{
    return d->formulaOk;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::GetMeasurementType() const -> MeasurementType
{
    return d->varType;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::IsNotUsed() const -> bool
{
    return qFuzzyIsNull(d->shiftBase) && qFuzzyIsNull(d->shiftB) && qFuzzyIsNull(d->shiftA);
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::GetValue() const -> qreal
{
    return CalcValue();
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::GetValue() -> qreal *
{
    StoreValue(CalcValue());
    return VInternalVariable::GetValue();
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::GetData() -> VContainer *
{
    return d->data.data();
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetBaseA(qreal base)
{
    d->currentBaseA = base;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetBaseB(qreal base)
{
    d->currentBaseB = base;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetBaseC(qreal base)
{
    d->currentBaseC = base;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetBase return value in base size and height
 * @return value
 */
auto VMeasurement::GetBase() const -> qreal
{
    return d->shiftBase;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetBase(qreal value)
{
    d->shiftBase = value;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetKheight return increment in heights
 * @return increment
 */
auto VMeasurement::GetShiftA() const -> qreal
{
    return d->shiftA;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void VMeasurement::SetShiftA(qreal value)
{
    d->shiftA = value;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetKsize return increment in sizes
 * @return increment
 */
auto VMeasurement::GetShiftB() const -> qreal
{
    return d->shiftB;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void VMeasurement::SetShiftB(qreal value)
{
    d->shiftB = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::GetShiftC() const -> qreal
{
    return d->shiftC;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetShiftC(qreal value)
{
    d->shiftC = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::GetStepA() const -> qreal
{
    return d->shiftA;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetStepA(qreal value)
{
    d->stepA = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::GetStepB() const -> qreal
{
    return d->stepB;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetStepB(qreal value)
{
    d->stepB = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::GetStepC() const -> qreal
{
    return d->stepC;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetStepC(qreal value)
{
    d->stepC = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::IsSpecialUnits() const -> bool
{
    return d->specialUnits;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetSpecialUnits(bool special)
{
    d->specialUnits = special;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::GetDimension() const -> IMD
{
    return d->dimension;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetDimension(IMD type)
{
    d->dimension = type;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::GetCorrection(qreal baseA, qreal baseB, qreal baseC) const -> qreal
{
    if (const QString hash = VMeasurement::CorrectionHash(baseA, baseB, baseC); d->corrections.contains(hash))
    {
        return d->corrections[hash].correction;
    }

    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::GetCorrections() const -> QMap<QString, VMeasurementCorrection>
{
    return d->corrections;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetCorrections(const QMap<QString, VMeasurementCorrection> &corrections)
{
    d->corrections = corrections;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::GetCustomImage() const -> VPatternImage
{
    return d->image;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetCustomImage(const VPatternImage &image)
{
    d->image = image;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::GetImage() const -> VPatternImage
{
    if (IsCustom())
    {
        return GetCustomImage();
    }

    VKnownMeasurementsDatabase const *db = VAbstractApplication::VApp()->KnownMeasurementsDatabase();
    VKnownMeasurements const knownDB = db->KnownMeasurements(GetKnownMeasurementsId());
    VKnownMeasurement const known = knownDB.Measurement(GetName());
    return knownDB.Image(known.diagram);
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::GetValueAlias(qreal baseA, qreal baseB, qreal baseC) const -> QString
{
    if (const QString hash = VMeasurement::CorrectionHash(baseA, baseB, baseC); d->corrections.contains(hash))
    {
        return d->corrections[hash].alias;
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VMeasurement::GetValueAlias() const -> QString
{
    if (qFuzzyIsNull(d->currentBaseA))
    {
        return d->valueAlias;
    }

    return GetValueAlias(d->currentBaseA, d->currentBaseB, d->currentBaseC);
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetValueAlias(const QString &alias)
{
    d->valueAlias = alias;
}
