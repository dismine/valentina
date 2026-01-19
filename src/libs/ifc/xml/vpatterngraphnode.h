/************************************************************************
 **
 **  @file   vpatterngraphnode.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   17 1, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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
#ifndef VPATTERNGRAPHNODE_H
#define VPATTERNGRAPHNODE_H

#include "../vmisc/typedef.h"

// Node type enumeration
enum class VNodeType : quint8
{
    TOOL,
    MODELING_TOOL,
    OBJECT,
    MODELING_OBJECT,
    PIECE
};

// Node structure
struct VNode
{
    vidtype id{NULL_ID};
    VNodeType type{VNodeType::TOOL};
    int indexPatternBlock{-1};

    // Each node uniquely represented by id. No need for additional checks
    auto operator==(const VNode &other) const -> bool;
};

#endif // VPATTERNGRAPHNODE_H
