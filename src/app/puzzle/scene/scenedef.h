/************************************************************************
 **
 **  @file   scenedef.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 8, 2021
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
#ifndef SCENEDEF_H
#define SCENEDEF_H

#include <QColor>
#include <QtGlobal>

enum class PGraphicsItem : int
{
    Piece = 1,
    Handles = 2,
    TransformationOrigin = 3
};

constexpr qreal foldTextMargin = 5;
constexpr int foldFontSize = 34;
constexpr QColor tileColor(180, 180, 180);

#endif // SCENEDEF_H
