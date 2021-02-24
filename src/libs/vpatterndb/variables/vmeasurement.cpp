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
#include "vvariable.h"
#include "vmeasurement_p.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VMeasurement create measurement for multisize table
 * @param name measurement's name
 * @param base measurement's base value
 */
VMeasurement::VMeasurement(quint32 index, const QString &name, qreal baseA, qreal baseB, qreal baseC, qreal base)
    :VVariable(name),
      d(new VMeasurementData(index, baseA, baseB, baseC, base))
{
    SetType(VarType::Measurement);
    VInternalVariable::SetValue(d->shiftBase);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VMeasurement create measurement for individual table
 * @param name measurement's base value
 * @param base value in base size and height
 */
VMeasurement::VMeasurement(VContainer *data, quint32 index, const QString &name, const qreal &base,
                           const QString &formula, bool ok)
    :VVariable(name), d(new VMeasurementData(data, index, formula, ok, base))
{
    SetType(VarType::Measurement);
    VInternalVariable::SetValue(base);
}

//---------------------------------------------------------------------------------------------------------------------
VMeasurement::VMeasurement(const VMeasurement &m)
    :VVariable(m), d(m.d)
{}

//---------------------------------------------------------------------------------------------------------------------
VMeasurement &VMeasurement::operator=(const VMeasurement &m)
{
    if ( &m == this )
    {
        return *this;
    }
    VVariable::operator=(m);
    d = m.d;
    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VMeasurement::VMeasurement(const VMeasurement &&m) Q_DECL_NOTHROW
    :VVariable(m), d(m.d)
{}

//---------------------------------------------------------------------------------------------------------------------
VMeasurement &VMeasurement::operator=(VMeasurement &&m) Q_DECL_NOTHROW
{
    VVariable::operator=(m);
    std::swap(d, m.d);
    return *this;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
VMeasurement::~VMeasurement()
{}

//---------------------------------------------------------------------------------------------------------------------
QString VMeasurement::CorrectionHash(qreal baseA, qreal baseB, qreal baseC)
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
    return hashBlocks.join(';');
}

//---------------------------------------------------------------------------------------------------------------------
qreal VMeasurement::CalcValue() const
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
qreal VMeasurement::Correction() const
{
    const QString hash = CorrectionHash(d->currentBaseA, d->currentBaseB, d->currentBaseC);
    if (d->corrections.contains(hash))
    {
        return d->corrections.value(hash);
    }

    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetGuiText measurement name for tooltip
 * @return measurement name
 */
QString VMeasurement::GetGuiText() const
{
    return d->gui_text;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetGuiText(const QString &guiText)
{
    d->gui_text = guiText;
}

//---------------------------------------------------------------------------------------------------------------------
QString VMeasurement::GetFormula() const
{
    return d->formula;
}

//---------------------------------------------------------------------------------------------------------------------
bool VMeasurement::IsCustom() const
{
    return GetName().indexOf(CustomMSign) == 0;
}

//---------------------------------------------------------------------------------------------------------------------
int VMeasurement::Index() const
{
    return static_cast<int>(d->index);
}

//---------------------------------------------------------------------------------------------------------------------
bool VMeasurement::IsFormulaOk() const
{
    return d->formulaOk;
}

//---------------------------------------------------------------------------------------------------------------------
bool VMeasurement::IsNotUsed() const
{
    return qFuzzyIsNull(d->shiftBase) && qFuzzyIsNull(d->shiftB) && qFuzzyIsNull(d->shiftA);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VMeasurement::GetValue() const
{
    return CalcValue();
}

//---------------------------------------------------------------------------------------------------------------------
qreal *VMeasurement::GetValue()
{
    VInternalVariable::SetValue(CalcValue());
    return VInternalVariable::GetValue();
}

//---------------------------------------------------------------------------------------------------------------------
VContainer *VMeasurement::GetData()
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
qreal VMeasurement::GetBase() const
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
qreal VMeasurement::GetShiftA() const
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
qreal VMeasurement::GetShiftB() const
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
qreal VMeasurement::GetShiftC() const
{
    return d->shiftC;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetShiftC(qreal value)
{
    d->shiftC = value;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VMeasurement::GetStepA() const
{
    return d->shiftA;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetStepA(qreal value)
{
    d->stepA = value;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VMeasurement::GetStepB() const
{
    return d->stepB;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetStepB(qreal value)
{
    d->stepB = value;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VMeasurement::GetStepC() const
{
    return d->stepC;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetStepC(qreal value)
{
    d->stepC = value;
}

//---------------------------------------------------------------------------------------------------------------------
bool VMeasurement::IsSpecialUnits() const
{
    return d->specialUnits;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetSpecialUnits(bool special)
{
    d->specialUnits = special;
}

//---------------------------------------------------------------------------------------------------------------------
IMD VMeasurement::GetDimension() const
{
    return d->dimension;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetDimension(IMD type)
{
    d->dimension = type;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VMeasurement::GetCorrection(qreal baseA, qreal baseB, qreal baseC) const
{
    return d->corrections.value(VMeasurement::CorrectionHash(baseA, baseB, baseC), 0);
}

//---------------------------------------------------------------------------------------------------------------------
QMap<QString, qreal> VMeasurement::GetCorrections() const
{
    return d->corrections;
}

//---------------------------------------------------------------------------------------------------------------------
void VMeasurement::SetCorrections(const QMap<QString, qreal> &corrections)
{
    d->corrections = corrections;
}
