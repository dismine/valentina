/************************************************************************
 **
 **  @file   def.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   31 5, 2023
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
#ifndef SVGDEF_H
#define SVGDEF_H

#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <QtCore/QHashFunctions>
#endif

class QChar;
class QString;

enum class SVGFontStyle
{
    Normal,
    Italic,
    Oblique
};

enum class SVGFontWeight
{
    Thin = 100,
    ExtraLight = 200,
    Light = 300,
    Normal = 400,
    Medium = 500,
    DemiBold = 600,
    Bold = 700,
    ExtraBold = 800,
    Black = 900
};

enum class SVGFontWritingSystem
{
    Any = 0,
    Latin = 1,
    Greek = 2,
    Cyrillic = 3,
    Armenian = 4,
    Hebrew = 5,
    Arabic = 6,
    Syriac = 7,
    Thaana = 8,
    Devanagari = 9,
    Bengali = 10,
    Gurmukhi = 11,
    Gujarati = 12,
    Oriya = 13,
    Tamil = 14,
    Telugu = 15,
    Kannada = 16,
    Malayalam = 17,
    Sinhala = 18,
    Thai = 19,
    Lao = 20,
    Tibetan = 21,
    Myanmar = 22,
    Georgian = 23,
    Khmer = 24,
    SimplifiedChinese = 25,
    TraditionalChinese = 26,
    Japanese = 27,
    Korean = 28,
    Vietnamese = 29,
    Symbol = 30,
    Other = Symbol, //	(the same as Symbol)
    Ogham = 31,
    Runic = 32,
    Nko = 33,
};

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
Q_DECL_CONST_FUNCTION inline auto qHash(SVGFontWritingSystem key, uint seed = 0) noexcept -> uint
{
    auto underlyingValue = static_cast<typename std::underlying_type<SVGFontWritingSystem>::type>(key);
    return ::qHash(underlyingValue, seed);
}
#endif

enum class SVGTextElideMode
{
    ElideLeft,
    ElideRight,
    ElideMiddle,
    ElideNone
};

auto LatinWritingSystem(QChar c) -> bool;
auto GreekWritingSystem(QChar c) -> bool;
auto CyrillicWritingSystem(QChar c) -> bool;
auto ArmenianWritingSystem(QChar c) -> bool;
auto HebrewWritingSystem(QChar c) -> bool;
auto ArabicWritingSystem(QChar c) -> bool;
auto SyriacWritingSystem(QChar c) -> bool;
auto ThaanaWritingSystem(QChar c) -> bool;
auto DevanagariWritingSystem(QChar c) -> bool;
auto BengaliWritingSystem(QChar c) -> bool;
auto GurmukhiWritingSystem(QChar c) -> bool;
auto GujaratiWritingSystem(QChar c) -> bool;
auto OriyaWritingSystem(QChar c) -> bool;
auto TamilWritingSystem(QChar c) -> bool;
auto TeluguWritingSystem(QChar c) -> bool;
auto KannadaWritingSystem(QChar c) -> bool;
auto MalayalamWritingSystem(QChar c) -> bool;
auto SinhalaWritingSystem(QChar c) -> bool;
auto ThaiWritingSystem(QChar c) -> bool;
auto LaoWritingSystem(QChar c) -> bool;
auto TibetanWritingSystem(QChar c) -> bool;
auto MyanmarWritingSystem(QChar c) -> bool;
auto GeorgianWritingSystem(QChar c) -> bool;
auto KhmerWritingSystem(QChar c) -> bool;
auto SimplifiedChineseWritingSystem(QChar c) -> bool;
auto TraditionalChineseWritingSystem(QChar c) -> bool;
auto JapaneseWritingSystem(QChar c) -> bool;
auto KoreanWritingSystem(QChar c) -> bool;
auto VietnameseWritingSystem(QChar c) -> bool;
auto SymbolWritingSystem(QChar c) -> bool;
auto OghamWritingSystem(QChar c) -> bool;
auto RunicWritingSystem(QChar c) -> bool;
auto NkoWritingSystem(QChar c) -> bool;

auto WritingSystemSample(SVGFontWritingSystem writingSystem) -> QString;

#endif // SVGDEF_H
