/************************************************************************
 **
 **  @file   varcradius.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   30 5, 2015
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

#ifndef VARCRADIUS_H
#define VARCRADIUS_H

#include <QtGlobal>

#include "../vmisc/def.h"
#include "vcurvevariable.h"

class VArc;
class VEllipticalArc;

class VArcRadius final : public VCurveVariable
{
public:
    VArcRadius();
    VArcRadius(const quint32 &id, const quint32 &parentId, const VArc *arc, Unit patternUnit);
    VArcRadius(const quint32 &id, const quint32 &parentId, const VEllipticalArc *elArc, int numberRadius,
               Unit patternUnit);
    VArcRadius(const VArcRadius &var) = default;
    auto operator=(const VArcRadius &var) -> VArcRadius &;
    ~VArcRadius() override = default;

    VArcRadius(VArcRadius &&var) noexcept = default;
    auto operator=(VArcRadius &&var) noexcept -> VArcRadius & = default;
};

#endif // VARCRADIUS_H
