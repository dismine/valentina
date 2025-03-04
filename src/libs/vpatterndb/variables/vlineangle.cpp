/************************************************************************
 **
 **  @file   vlineangle.cpp
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

#include "vlineangle.h"

#include <QLineF>
#include <QMessageLogger>
#include <QPointF>
#include <QString>
#include <QtMath>

#include "../ifc/ifcdef.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/def.h"
#include "vinternalvariable.h"
#include "vlineangle_p.h"

//---------------------------------------------------------------------------------------------------------------------
VLineAngle::VLineAngle()
  : VInternalVariable(),
    d(new VLineAngleData)
{
    SetType(VarType::LineAngle);
}

//---------------------------------------------------------------------------------------------------------------------
VLineAngle::VLineAngle(const VPointF *p1, const quint32 &p1Id, const VPointF *p2, const quint32 &p2Id)
  : VInternalVariable(),
    d(new VLineAngleData(p1Id, p2Id))
{
    SetType(VarType::LineAngle);

    // cppcheck-suppress unknownMacro
    SCASSERT(p1 != nullptr)
    SCASSERT(p2 != nullptr)

    SetName(QString(angleLine_ + "%1_%2").arg(p1->name(), p2->name()));
    SetValue(p1, p2);
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL_2(VLineAngle, VInternalVariable)

//---------------------------------------------------------------------------------------------------------------------
auto VLineAngle::operator=(const VLineAngle &var) -> VLineAngle &
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
VLineAngle::VLineAngle(VLineAngle &&var) noexcept
  : VInternalVariable(std::move(var)),
    d(std::move(var.d)) // NOLINT(bugprone-use-after-move)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VLineAngle::operator=(VLineAngle &&var) noexcept -> VLineAngle &
{
    VInternalVariable::operator=(var);
    std::swap(d, var.d);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VLineAngle::~VLineAngle() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VLineAngle::Filter(quint32 id) -> bool
{
    return id == d->p1Id || id == d->p2Id;
}

//---------------------------------------------------------------------------------------------------------------------
void VLineAngle::SetValue(const VPointF *p1, const VPointF *p2)
{
    SCASSERT(p1 != nullptr)
    SCASSERT(p2 != nullptr)
    // Correct angle. Try avoid results like 6,7563e-15.
    const qreal angle =
        qFloor(QLineF(static_cast<QPointF>(*p1), static_cast<QPointF>(*p2)).angle() * 100000.) / 100000.;
    StoreValue(angle);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
auto VLineAngle::GetP1Id() const -> quint32
{
    return d->p1Id;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
auto VLineAngle::GetP2Id() const -> quint32
{
    return d->p2Id;
}
