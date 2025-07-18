/************************************************************************
 **
 **  @file   vincrementtablerow.cpp
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

#include "vincrement.h"

#include "../vmisc/def.h"
#include "vincrement_p.h"
#include "vvariable.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VIncrement create enpty increment
 */
VIncrement::VIncrement()
  : VVariable(),
    d(new VIncrementData)
{
    SetType(VarType::Increment);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VIncrementTableRow create increment
 * @param name increment's name
 */
VIncrement::VIncrement(VContainer *data, const QString &name, IncrementType incrType)
  : VVariable(name, QString()),
    d(new VIncrementData(data, incrType))
{
    incrType == IncrementType::Separator ? SetType(VarType::IncrementSeparator) : SetType(VarType::Increment);
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL_2(VIncrement, VVariable)

//---------------------------------------------------------------------------------------------------------------------
auto VIncrement::operator=(const VIncrement &incr) -> VIncrement &
{
    if (&incr == this)
    {
        return *this;
    }
    VVariable::operator=(incr);
    d = incr.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VIncrement::VIncrement(VIncrement &&incr) noexcept
  : VVariable(std::move(incr)),
    d(std::move(incr.d)) // NOLINT(bugprone-use-after-move)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VIncrement::operator=(VIncrement &&incr) noexcept -> VIncrement &
{
    VVariable::operator=(incr);
    std::swap(d, incr.d);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VIncrement::~VIncrement() = default;

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief getIndex return index of row. Row index for increments and preview calucalations is unique. Check type before
 * using.
 * @return index
 */
auto VIncrement::GetIndex() const -> quint32
{
    return d->index;
}

//---------------------------------------------------------------------------------------------------------------------
void VIncrement::SetFormula(qreal base, const QString &formula, bool ok)
{
    StoreValue(base);
    d->formula = formula;
    d->formulaOk = ok;
}

//---------------------------------------------------------------------------------------------------------------------
auto VIncrement::GetFormula() const -> QString
{
    return d->formula;
}

//---------------------------------------------------------------------------------------------------------------------
auto VIncrement::IsFormulaOk() const -> bool
{
    return d->formulaOk;
}

//---------------------------------------------------------------------------------------------------------------------
void VIncrement::SetIndex(quint32 index)
{
    d->index = index;
}

//---------------------------------------------------------------------------------------------------------------------
auto VIncrement::GetData() -> VContainer *
{
    return d->data.data();
}

//---------------------------------------------------------------------------------------------------------------------
auto VIncrement::GetIncrementType() const -> IncrementType
{
    return d->incrType;
}

//---------------------------------------------------------------------------------------------------------------------
auto VIncrement::IsPreviewCalculation() const -> bool
{
    return d->previewCalculation;
}

//---------------------------------------------------------------------------------------------------------------------
void VIncrement::SetPreviewCalculation(bool value)
{
    d->previewCalculation = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VIncrement::IsSpecialUnits() const -> bool
{
    return d->specialUnits;
}

//---------------------------------------------------------------------------------------------------------------------
void VIncrement::SetSpecialUnits(bool special)
{
    d->specialUnits = special;
}
