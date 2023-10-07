/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   24 9, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#include "vcurveclength.h"

#include <QLatin1String>
#include <QMessageLogger>

#include "../ifc/ifcdef.h"
#include "../vgeometry/vspline.h"
#include "../vmisc/def.h"
#include "vcurvevariable.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
VCurveCLength::VCurveCLength()
  : VCurveVariable()
{
    SetType(VarType::CurveCLength);
}

//---------------------------------------------------------------------------------------------------------------------
VCurveCLength::VCurveCLength(const quint32 &id, const quint32 &parentId, const VAbstractBezier *curve,
                             CurveCLength cType, Unit patternUnit)
  : VCurveVariable(id, parentId)
{
    SetType(VarType::CurveCLength);
    SCASSERT(curve != nullptr)
    if (cType == CurveCLength::C1)
    {
        SetValue(FromPixel(curve->GetC1Length(), patternUnit));
        SetName(c1Length_V + curve->name());

        if (not curve->GetAlias().isEmpty())
        {
            SetAlias(c1Length_V + curve->GetAlias());
        }
    }
    else
    {
        SetValue(FromPixel(curve->GetC2Length(), patternUnit));
        SetName(c2Length_V + curve->name());

        if (not curve->GetAlias().isEmpty())
        {
            SetAlias(c2Length_V + curve->GetAlias());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
VCurveCLength::VCurveCLength(const quint32 &id, const quint32 &parentId, const VAbstractBezier *baseCurve,
                             const VSpline &spl, CurveCLength cType, Unit patternUnit, qint32 segment)
  : VCurveVariable(id, parentId)
{
    // cppcheck-suppress unknownMacro
    SCASSERT(baseCurve != nullptr)

    SetType(VarType::CurveCLength);
    if (cType == CurveCLength::C1)
    {
        SetValue(FromPixel(spl.GetC1Length(), patternUnit));
        SetName(c1Length_V + baseCurve->name() + '_'_L1 + seg_ + QString().setNum(segment));

        if (not baseCurve->GetAlias().isEmpty())
        {
            SetAlias(c1Length_V + baseCurve->GetAlias() + '_'_L1 + seg_ + QString().setNum(segment));
        }
    }
    else
    {
        SetValue(FromPixel(spl.GetC2Length(), patternUnit));
        SetName(c2Length_V + baseCurve->name() + '_'_L1 + seg_ + QString().setNum(segment));

        if (not baseCurve->GetAlias().isEmpty())
        {
            SetAlias(c2Length_V + baseCurve->GetAlias() + '_'_L1 + seg_ + QString().setNum(segment));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
VCurveCLength::VCurveCLength(const VCurveCLength &var)
  : VCurveVariable(var)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VCurveCLength::operator=(const VCurveCLength &var) -> VCurveCLength &
{
    if (&var == this)
    {
        return *this;
    }
    VCurveVariable::operator=(var);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VCurveCLength::~VCurveCLength()
{
}
