/************************************************************************
 **
 **  @file   vscenestylesheet.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   24 7, 2023
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
#ifndef VSCENESTYLESHEET_H
#define VSCENESTYLESHEET_H

#include <QtGlobal>

#include "themeDef.h"
#include "vmanuallayoutstyle.h"
#include "vpatternpiecestyle.h"
#include "vtoolstyle.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../defglobal.h"
#endif

class QGraphicsItem;

class VSceneStylesheet
{
public:
    ~VSceneStylesheet() = default;

    static auto Instance() -> VSceneStylesheet &;

    static void ResetStyles();

    static auto ToolStyle() -> VToolStyle const &;
    static void SetToolStyle(const VToolStyle &style);

    static auto PatternPieceStyle() -> VPatternPieceStyle const &;
    static void SetPatternPieceStyle(const VPatternPieceStyle &style);

    static auto ManualLayoutStyle() -> VManualLayoutStyle const &;
    static void SetManualLayoutStyle(const VManualLayoutStyle &style);

    static auto Color(VColorRole role) -> QColor;

    static auto CorrectToolColor(const QGraphicsItem *item, const QColor &color) -> QColor;
    static auto CorrectToolColorForDarkTheme(const QColor &color) -> QColor;

private:
    Q_DISABLE_COPY_MOVE(VSceneStylesheet) // NOLINT

    VToolStyle m_toolStyle{};
    VPatternPieceStyle m_patternPieceStyle{};
    VManualLayoutStyle m_manualLayoutStyle{};

    VSceneStylesheet() = default;
};

#endif // VSCENESTYLESHEET_H
