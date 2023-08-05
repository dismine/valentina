/************************************************************************
 **
 **  @file   themeDef.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   26 7, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#ifndef THEMEDEF_H
#define THEMEDEF_H

#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtCore/QHashFunctions>
#endif

enum class VColorRole
{
    DefaultColor,
    PatternColor,
    CustomColor, // Controled by pen color outside of an item
    DraftPointColor,
    DraftLabelColor,
    DraftLabelHoverColor,
    DraftLabelLineColor,
    ControlLineColor,
    ControlPointColor,
    AccuracyRadiusColor,
    VisMainColor,
    VisSupportColor,
    VisSupportColor2,
    VisSupportColor3,
    VisSupportColor4,
    BasePointColor,
    PieceColor,
    PiecePointColor,
    PieceNodeLabelColor,
    PieceNodeLabelHoverColor,
    PieceNodeLabelLineColor
};

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
Q_DECL_CONST_FUNCTION inline auto qHash(VColorRole key, uint seed = 0) noexcept -> uint
{
    auto underlyingValue = static_cast<typename std::underlying_type<VColorRole>::type>(key);
    return ::qHash(underlyingValue, seed);
}
#endif

#endif // THEMEDEF_H
