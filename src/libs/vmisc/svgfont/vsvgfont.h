/************************************************************************
 **
 **  @file   vsvgfont.h
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
#ifndef VSVGFONT_H
#define VSVGFONT_H

#include "svgdef.h"
#include <QMetaType>
#include <QSharedDataPointer>
#include <QString>

class VSvgFontData;

class VSvgFont
{
public:
    VSvgFont();
    explicit VSvgFont(qreal horizAdvX);
    VSvgFont(const VSvgFont &font);
    ~VSvgFont();

    auto operator=(const VSvgFont &font) -> VSvgFont &;
#ifdef Q_COMPILER_RVALUE_REFS
    VSvgFont(VSvgFont &&font) noexcept;
    auto operator=(VSvgFont &&font) noexcept -> VSvgFont &;
#endif

    void SetId(const QString &id);
    auto Id() const -> QString;

    void SetFamilyName(const QString &name);
    auto FamilyName() const -> QString;

    void SetName(const QString &name);
    auto Name() const -> QString;

    void SetUnitsPerEm(qreal upem);
    auto UnitsPerEm() const -> qreal;

    void SetAscent(qreal ascent);
    auto Ascent() const -> qreal;

    void SetDescent(qreal descent);
    auto Descent() const -> qreal;

    void SetHorizAdvX(qreal horizAdvX);
    auto HorizAdvX() const -> qreal;

    void SetStyle(SVGFontStyle style);
    auto Style() const -> SVGFontStyle;

    void SetItalic(bool enable);
    auto Iatic() const -> bool;

    void SetBold(bool enable);
    auto Bold() const -> bool;

    void SetWeight(SVGFontWeight weight);
    auto Weight() const -> SVGFontWeight;

    auto IsValid() const -> bool;

    void SetPriority(int priority);
    auto Priority() const -> int;

    void SetPath(const QString &path);
    auto Path() const -> QString;

    void SetWritingSystems(const QList<SVGFontWritingSystem> &writingSystems);
    auto WritingSystems() const -> QList<SVGFontWritingSystem>;

    auto Hash() const -> QByteArray;

    static auto Hash(const QString &family, SVGFontStyle style, SVGFontWeight weight) -> QByteArray;

    void SetPixelSize(int pixelSize);
    auto PixelSize() const -> int;

    void SetPointSizeF(qreal pointSize);
    auto PointSizeF() const -> qreal;

    void SetPointSize(int pointSize);
    auto PointSize() const -> int;

private:
    QSharedDataPointer<VSvgFontData> d;
};

Q_DECLARE_METATYPE(VSvgFont)                  // NOLINT
Q_DECLARE_TYPEINFO(VSvgFont, Q_MOVABLE_TYPE); // NOLINT

#endif // VSVGFONT_H
