/************************************************************************
 **
 **  @file   vcurvelength.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 8, 2014
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

#include "vcurvevariable.h"

#include "../vmisc/def.h"
#include "vcurvevariable_p.h"
#include "vinternalvariable.h"

//---------------------------------------------------------------------------------------------------------------------
VCurveVariable::VCurveVariable()
  : d(new VCurveVariableData)
{
    SetType(VarType::Unknown);
}

//---------------------------------------------------------------------------------------------------------------------
VCurveVariable::VCurveVariable(const quint32 &id, const quint32 &parentId)
  : d(new VCurveVariableData(id, parentId))
{
    SetType(VarType::Unknown);
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL(VCurveVariable)

//---------------------------------------------------------------------------------------------------------------------
auto VCurveVariable::operator=(const VCurveVariable &var) -> VCurveVariable &
{
    if (&var == this)
    {
        return *this;
    }
    VInternalVariable::operator=(var);
    d = var.d;
    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VCurveVariable::VCurveVariable(VCurveVariable &&var) noexcept
  : VInternalVariable(std::move(var)),
    d(std::move(var.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VCurveVariable::operator=(VCurveVariable &&var) noexcept -> VCurveVariable &
{
    VInternalVariable::operator=(var);
    std::swap(d, var.d);
    return *this;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
VCurveVariable::~VCurveVariable() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VCurveVariable::Filter(quint32 id) -> bool
{
    if (id == NULL_ID)
    {
        return false;
    }

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnull-dereference")

    if (d->parentId != NULL_ID) // Do not check if value zero
    { // Not all curves have parents. Only those who was created after cutting the parent curve.
        return d->id == id || d->parentId == id;
    }

    // cppcheck-suppress unknownMacro
    QT_WARNING_POP

    return d->id == id;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
auto VCurveVariable::GetId() const -> quint32
{
    return d->id;
}

//---------------------------------------------------------------------------------------------------------------------
void VCurveVariable::SetId(const quint32 &id)
{
    d->id = id;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
auto VCurveVariable::GetParentId() const -> quint32
{
    return d->parentId;
}

//---------------------------------------------------------------------------------------------------------------------
void VCurveVariable::SetParentId(const quint32 &value)
{
    d->parentId = value;
}
