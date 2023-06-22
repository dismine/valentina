/************************************************************************
 **
 **  @file   vsvgglyph.h
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
#ifndef VSVGGLYPH_H
#define VSVGGLYPH_H

#include <QSharedDataPointer>

class VSvgGlyphData;
class QPainterPath;

class VSvgGlyph
{
public:
    VSvgGlyph();
    VSvgGlyph(QChar unicode, const QPainterPath &path, qreal horizAdvX);
    VSvgGlyph(const VSvgGlyph &glyph);
    ~VSvgGlyph();

    auto operator=(const VSvgGlyph &glyph) -> VSvgGlyph &;
#ifdef Q_COMPILER_RVALUE_REFS
    VSvgGlyph(VSvgGlyph &&glyph) noexcept;
    auto operator=(VSvgGlyph &&glyph) noexcept -> VSvgGlyph &;
#endif

    void SetUnicode(const QChar &unicode);
    auto Unicode() const -> QChar;

    void SetPath(const QPainterPath &path);
    auto Path() const -> QPainterPath;

    void SetHorizAdvX(qreal horizAdvX);
    auto HorizAdvX() const -> qreal;

private:
    QSharedDataPointer<VSvgGlyphData> d;
};

Q_DECLARE_TYPEINFO(VSvgGlyph, Q_MOVABLE_TYPE); // NOLINT

#endif // VSVGGLYPH_H
