/************************************************************************
 **
 **  @file   vsvgglyph.cpp
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
#include "vsvgglyph.h"
#include "../def.h"
#include "vsvgglyph_p.h"

#include <QChar>
#include <QPainterPath>

//---------------------------------------------------------------------------------------------------------------------
VSvgGlyph::VSvgGlyph()
  : d(new VSvgGlyphData())
{
}

//---------------------------------------------------------------------------------------------------------------------
VSvgGlyph::VSvgGlyph(QChar unicode, const QPainterPath &path, qreal horizAdvX)
  : d(new VSvgGlyphData(unicode, path, horizAdvX))
{
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL(VSvgGlyph)

//---------------------------------------------------------------------------------------------------------------------
VSvgGlyph::~VSvgGlyph() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VSvgGlyph::operator=(const VSvgGlyph &glyph) -> VSvgGlyph &
{
    if (&glyph == this)
    {
        return *this;
    }
    d = glyph.d;
    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VSvgGlyph::VSvgGlyph(VSvgGlyph &&glyph) noexcept
  : d(std::move(glyph.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgGlyph::operator=(VSvgGlyph &&glyph) noexcept -> VSvgGlyph &
{
    std::swap(d, glyph.d);
    return *this;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
void VSvgGlyph::SetUnicode(const QChar &unicode)
{
    d->m_unicode = unicode;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgGlyph::Unicode() const -> QChar
{
    return d->m_unicode;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgGlyph::SetPath(const QPainterPath &path)
{
    d->m_path = path;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgGlyph::Path() const -> QPainterPath
{
    return d->m_path;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgGlyph::SetHorizAdvX(qreal horizAdvX)
{
    d->m_horizAdvX = horizAdvX;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgGlyph::HorizAdvX() const -> qreal
{
    return d->m_horizAdvX;
}
