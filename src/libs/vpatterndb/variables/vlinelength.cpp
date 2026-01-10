/************************************************************************
 **
 **  @file   vlinelength.cpp
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

#include "vlinelength.h"

#include <QLineF>
#include <QMessageLogger>
#include <QPointF>
#include <QString>

#include "../ifc/ifcdef.h"
#include "../vgeometry/vpointf.h"
#include "vinternalvariable.h"
#include "vlinelength_p.h"

//---------------------------------------------------------------------------------------------------------------------
VLengthLine::VLengthLine()
  : VInternalVariable(),
    d(new VLengthLineData)
{
    SetType(VarType::LineLength);
}

//---------------------------------------------------------------------------------------------------------------------
VLengthLine::VLengthLine(
    const VPointF *p1, quint32 p1Id, const VPointF *p2, quint32 p2Id, Unit patternUnit, quint32 mainReference)
  : VInternalVariable(),
    d(new VLengthLineData(p1Id, p2Id, patternUnit))
{
    // cppcheck-suppress unknownMacro
    SCASSERT(p1 != nullptr)
    SCASSERT(p2 != nullptr)

    SetType(VarType::LineLength);
    SetName(line_ + p1->name() + '_' + p2->name());
    SetValue(p1, p2);

    if (mainReference == NULL_ID)
    {
        SetReferences({p1Id, p2Id});
    }
    else
    {
        SetReferences({mainReference});
    }
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL_2(VLengthLine, VInternalVariable)

//---------------------------------------------------------------------------------------------------------------------
auto VLengthLine::operator=(const VLengthLine &var) -> VLengthLine &
{
    if (&var == this)
    {
        return *this;
    }
    VInternalVariable::operator=(var);
    d = var.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VLengthLine::VLengthLine(VLengthLine &&var) noexcept
  : VInternalVariable(std::move(var)),
    d(std::move(var.d)) // NOLINT(bugprone-use-after-move)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VLengthLine::operator=(VLengthLine &&var) noexcept -> VLengthLine &
{
    VInternalVariable::operator=(var);
    std::swap(d, var.d);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VLengthLine::~VLengthLine() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VLengthLine::Filter(quint32 id) -> bool
{
    return id == d->p1Id || id == d->p2Id;
}

//---------------------------------------------------------------------------------------------------------------------
void VLengthLine::SetValue(const VPointF *p1, const VPointF *p2)
{
    SCASSERT(p1 != nullptr)
    SCASSERT(p2 != nullptr)

    StoreValue(FromPixel(QLineF(static_cast<QPointF>(*p1), static_cast<QPointF>(*p2)).length(), d->patternUnit));
}

//---------------------------------------------------------------------------------------------------------------------
auto VLengthLine::GetP1Id() const -> quint32
{
    return d->p1Id;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLengthLine::GetP2Id() const -> quint32
{
    return d->p2Id;
}
