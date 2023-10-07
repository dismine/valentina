/************************************************************************
 **
 **  @file   layoutdef.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   31 8, 2021
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2021 Valentina project
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

#include "layoutdef.h"

#include <QString>
#include <QStringList>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
auto GrainlineTypeToStr(GrainlineType type) -> QString
{
    QString result;
    switch (type)
    {
        case GrainlineType::Horizontal:
            result = "horizontal"_L1;
            break;
        case GrainlineType::Vertical:
            result = "vertical"_L1;
            break;
        case GrainlineType::NotFixed:
        default:
            result = "notFixed"_L1;
            break;
    }
    return result;
}

//---------------------------------------------------------------------------------------------------------------------
auto StrToGrainlineType(const QString &string) -> GrainlineType
{
    const QStringList types{
        QStringLiteral("horizontal"), // 0
        QStringLiteral("vertical"),   // 1
        QStringLiteral("notFixed")    // 2
    };

    GrainlineType type = GrainlineType::NotFixed;
    switch (types.indexOf(string))
    {
        case 0: // horizontal
            type = GrainlineType::Horizontal;
            break;
        case 2: // vertical
            type = GrainlineType::Vertical;
            break;
        case 3: // notFixed
        default:
            type = GrainlineType::NotFixed;
            break;
    }
    return type;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPTransformationOrigon::operator==(const VPTransformationOrigon &origin) const -> bool
{
    return this->origin == origin.origin && custom == origin.custom;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPTransformationOrigon::operator!=(const VPTransformationOrigon &origin) const -> bool
{
    return !VPTransformationOrigon::operator==(origin);
}
