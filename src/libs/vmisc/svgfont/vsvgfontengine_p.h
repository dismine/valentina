/************************************************************************
 **
 **  @file   vsvgfontengine_p.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   30 5, 2023
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
#ifndef VSVGFONTENGINE_P_H
#define VSVGFONTENGINE_P_H

#include "vsvgfont.h"
#include <QChar>
#include <QHash>
#include <QSharedData>
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../defglobal.h"
#include "vsvgglyph.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VSvgFontEngineData final : public QSharedData
{
public:
    VSvgFontEngineData() = default;
    explicit VSvgFontEngineData(const VSvgFont &font);
    VSvgFontEngineData(const VSvgFontEngineData &engine) = default;
    ~VSvgFontEngineData() = default;

    VSvgFont m_font{};                  // NOLINT(misc-non-private-member-variables-in-classes)
    QHash<QChar, VSvgGlyph> m_glyphs{}; // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VSvgFontEngineData) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
inline VSvgFontEngineData::VSvgFontEngineData(const VSvgFont &font)
  : m_font(font)
{
}

QT_WARNING_POP

#endif // VSVGFONTENGINE_P_H
