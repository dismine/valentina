/************************************************************************
 **
 **  @file   vcurveangle.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   1 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "vcurveangle.h"

#include <QLatin1String>
#include <QMessageLogger>

#include "../ifc/ifcdef.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vspline.h"
#include "../vmisc/def.h"
#include "vcurvevariable.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
VCurveAngle::VCurveAngle()
  : VCurveVariable()
{
    SetType(VarType::CurveAngle);
}

//---------------------------------------------------------------------------------------------------------------------
VCurveAngle::VCurveAngle(const quint32 &id, const quint32 &parentId)
  : VCurveVariable(id, parentId)
{
    SetType(VarType::CurveAngle);
}

//---------------------------------------------------------------------------------------------------------------------
VCurveAngle::VCurveAngle(const quint32 &id, const quint32 &parentId, const VAbstractCurve *curve, CurveAngle angle)
  : VCurveVariable(id, parentId)
{
    SetType(VarType::CurveAngle);
    SCASSERT(curve != nullptr)
    if (angle == CurveAngle::StartAngle)
    {
        SetValue(curve->GetStartAngle());
        SetName(angle1_V + curve->name());

        if (not curve->GetAlias().isEmpty())
        {
            SetAlias(angle1_V + curve->GetAlias());
        }
    }
    else
    {
        SetValue(curve->GetEndAngle());
        SetName(angle2_V + curve->name());

        if (not curve->GetAlias().isEmpty())
        {
            SetAlias(angle2_V + curve->GetAlias());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
VCurveAngle::VCurveAngle(const quint32 &id, const quint32 &parentId, const VAbstractCurve *baseCurve,
                         const VSpline &spl, CurveAngle angle, qint32 segment)
  : VCurveVariable(id, parentId)
{
    // cppcheck-suppress unknownMacro
    SCASSERT(baseCurve != nullptr)

    SetType(VarType::CurveAngle);
    if (angle == CurveAngle::StartAngle)
    {
        SetValue(spl.GetStartAngle());
        SetName(angle1_V + baseCurve->name() + '_'_L1 + seg_ + QString().setNum(segment));

        if (not baseCurve->GetAlias().isEmpty())
        {
            SetAlias(angle1_V + baseCurve->GetAlias() + '_'_L1 + seg_ + QString().setNum(segment));
        }
    }
    else
    {
        SetValue(spl.GetEndAngle());
        SetName(angle2_V + baseCurve->name() + '_'_L1 + seg_ + QString().setNum(segment));

        if (not baseCurve->GetAlias().isEmpty())
        {
            SetAlias(angle2_V + baseCurve->GetAlias() + '_'_L1 + seg_ + QString().setNum(segment));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
VEllipticalArcRotation::VEllipticalArcRotation()
  : VCurveAngle()
{
    SetType(VarType::CurveAngle);
}

//---------------------------------------------------------------------------------------------------------------------
VEllipticalArcRotation::VEllipticalArcRotation(const quint32 &id, const quint32 &parentId, const VEllipticalArc *elArc)
  : VCurveAngle(id, parentId)
{
    SetType(VarType::CurveAngle);
    SCASSERT(elArc != nullptr)
    SetValue(elArc->GetRotationAngle());
    SetName(rotation_V + elArc->name());

    if (not elArc->GetAlias().isEmpty())
    {
        SetAlias(rotation_V + elArc->GetAlias());
    }
}
