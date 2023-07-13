/************************************************************************
 **
 **  @file   vinternalvariable.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 7, 2014
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

#include "vinternalvariable.h"
#include "vinternalvariable_p.h"

//---------------------------------------------------------------------------------------------------------------------
VInternalVariable::VInternalVariable()
  : d(new VInternalVariableData)
{
}

//---------------------------------------------------------------------------------------------------------------------
VInternalVariable::VInternalVariable(const VInternalVariable &var) = default;

//---------------------------------------------------------------------------------------------------------------------
auto VInternalVariable::operator=(const VInternalVariable &var) -> VInternalVariable &
{
    if (&var == this)
    {
        return *this;
    }
    d = var.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VInternalVariable::VInternalVariable(VInternalVariable &&var) noexcept
  : d(std::move(var.d))
{
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
auto VInternalVariable::operator=(VInternalVariable &&var) noexcept -> VInternalVariable &
{
    std::swap(d, var.d);
    return *this;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
VInternalVariable::~VInternalVariable() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VInternalVariable::Filter(quint32 id) -> bool
{
    Q_UNUSED(id)
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VInternalVariable::IsNotUsed() const -> bool
{
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VInternalVariable::GetValue() const -> qreal
{
    return d->value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VInternalVariable::GetValue() -> qreal *
{
    return &d->value;
}

//---------------------------------------------------------------------------------------------------------------------
void VInternalVariable::SetValue(const qreal &value)
{
    d->value = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VInternalVariable::GetName() const -> QString
{
    return d->name;
}

//---------------------------------------------------------------------------------------------------------------------
void VInternalVariable::SetName(const QString &name)
{
    d->name = name.simplified();
}

//---------------------------------------------------------------------------------------------------------------------
auto VInternalVariable::GetType() const -> VarType
{
    return d->type;
}

//---------------------------------------------------------------------------------------------------------------------
void VInternalVariable::SetType(const VarType &type)
{
    d->type = type;
}

//---------------------------------------------------------------------------------------------------------------------
void VInternalVariable::SetAlias(const QString &alias)
{
    d->alias = alias;
}

//---------------------------------------------------------------------------------------------------------------------
auto VInternalVariable::GetAlias() const -> QString
{
    return d->alias;
}
