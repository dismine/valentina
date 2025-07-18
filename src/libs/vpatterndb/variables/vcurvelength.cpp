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

#include "vcurvelength.h"

#include <QLatin1String>
#include <QMessageLogger>

#include "../ifc/ifcdef.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vspline.h"
#include "vcurvevariable.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
VCurveLength::VCurveLength()
  : VCurveVariable()
{
    SetType(VarType::CurveLength);
}

//---------------------------------------------------------------------------------------------------------------------
VCurveLength::VCurveLength(const quint32 &id, const quint32 &parentId, const VAbstractCurve *curve, Unit patternUnit)
  : VCurveVariable(id, parentId)
{
    SetType(VarType::CurveLength);
    // cppcheck-suppress unknownMacro
    SCASSERT(curve != nullptr)
    SetName(curve->name());

    if (not curve->GetAlias().isEmpty())
    {
        SetAlias(curve->GetAlias());
    }

    StoreValue(FromPixel(curve->GetLength(), patternUnit));
}

//---------------------------------------------------------------------------------------------------------------------
VCurveLength::VCurveLength(const quint32 &id, const quint32 &parentId, const VAbstractCurve *baseCurve,
                           const VSpline &spl, Unit patternUnit, qint32 segment)
  : VCurveVariable(id, parentId)
{
    SCASSERT(baseCurve != nullptr)

    SetType(VarType::CurveLength);
    SetName(baseCurve->name() + '_'_L1 + seg_ + QString().setNum(segment));

    if (not baseCurve->GetAlias().isEmpty())
    {
        SetAlias(baseCurve->GetAlias() + '_'_L1 + seg_ + QString().setNum(segment));
    }

    StoreValue(FromPixel(spl.GetLength(), patternUnit));
}

//---------------------------------------------------------------------------------------------------------------------
VCurveLength::VCurveLength(const VCurveLength &var)
  : VCurveVariable(var)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VCurveLength::operator=(const VCurveLength &var) -> VCurveLength &
{
    if (&var == this)
    {
        return *this;
    }
    VCurveVariable::operator=(var);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VCurveLength::~VCurveLength()
{
}
