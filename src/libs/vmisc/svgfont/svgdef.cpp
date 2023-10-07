/************************************************************************
 **
 **  @file   svgdef.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 6, 2023
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

#include "svgdef.h"

#include <QChar>
#include <QString>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
auto LatinWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return (codePoint > 0x0000 && codePoint <= 0x007F)      // Basic Latin
           || (codePoint >= 0x0080 && codePoint <= 0x00FF)  // Latin-1 Supplement
           || (codePoint >= 0x0100 && codePoint <= 0x017F)  // Latin Extended-A
           || (codePoint >= 0x0180 && codePoint <= 0x024F)  // Latin Extended-B
           || (codePoint >= 0x0250 && codePoint <= 0x02AF)  // IPA Extensions
           || (codePoint >= 0x02B0 && codePoint <= 0x02FF)  // Spacing Modifier Letters
           || (codePoint >= 0x1D00 && codePoint <= 0x1D7F)  // Phonetic Extensions
           || (codePoint >= 0x1D80 && codePoint <= 0x1DBF)  // Phonetic Extensions Supplement
           || (codePoint >= 0x1E00 && codePoint <= 0x1EFF)  // Latin Extended Additional
           || (codePoint >= 0x2070 && codePoint <= 0x209F)  // Superscripts and Subscripts
           || (codePoint >= 0x2100 && codePoint <= 0x214F)  // Letterlike Symbols
           || (codePoint >= 0x2150 && codePoint <= 0x218F)  // Number Forms
           || (codePoint >= 0x2C60 && codePoint <= 0x2C7F)  // Latin Extended-C
           || (codePoint >= 0xA720 && codePoint <= 0xA7FF)  // Latin Extended-D
           || (codePoint >= 0xAB30 && codePoint <= 0xAB6F)  // Latin Extended-E
           || (codePoint >= 0xFB00 && codePoint <= 0xFB4F)  // Alphabetic Presentation Forms (Latin ligatures)
           || (codePoint >= 0xFF00 && codePoint <= 0xFFEF); // Halfwidth and Fullwidth Forms
}

//---------------------------------------------------------------------------------------------------------------------
auto GreekWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return (codePoint >= 0x0370 && codePoint <= 0x03FF)     // Greek and Coptic
           || (codePoint >= 0x1F00 && codePoint <= 0x1FFF); // Greek Extended
}

//---------------------------------------------------------------------------------------------------------------------
auto CyrillicWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return (codePoint >= 0x0400 && codePoint <= 0x04FF)     // Cyrillic
           || (codePoint >= 0x0500 && codePoint <= 0x052F)  // Cyrillic Supplement
           || (codePoint >= 0x2DE0 && codePoint <= 0x2DFF)  // Cyrillic Extended-A
           || (codePoint >= 0xA640 && codePoint <= 0xA69F)  // Cyrillic Extended-B
           || (codePoint >= 0x1C80 && codePoint <= 0x1C8F); // Cyrillic Extended-C
}

//---------------------------------------------------------------------------------------------------------------------
auto ArmenianWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return (codePoint >= 0x0530 && codePoint <= 0x058F)     // Armenian
           || (codePoint >= 0xFB00 && codePoint <= 0xFB17); // Alphabetic Pres. Forms
}

//---------------------------------------------------------------------------------------------------------------------
auto HebrewWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return (codePoint >= 0x0590 && codePoint <= 0x05FF)     // Hebrew,
           || (codePoint >= 0xFB1D && codePoint <= 0xFB4F); // Alphabetic Presentation Forms
}

//---------------------------------------------------------------------------------------------------------------------
auto ArabicWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return (codePoint >= 0x0600 && codePoint <= 0x06FF)     // Arabic
           || (codePoint >= 0x0750 && codePoint <= 0x077F)  // Arabic Supplement
           || (codePoint >= 0x08A0 && codePoint <= 0x08FF)  // Arabic Extended-A
           || (codePoint >= 0x0870 && codePoint <= 0x089F)  // Arabic Extended-B
           || (codePoint >= 0xFB50 && codePoint <= 0xFDFF)  // Arabic Pres. Forms-A
           || (codePoint >= 0xFE70 && codePoint <= 0xFEFF); // Arabic Pres. Forms-B
}

//---------------------------------------------------------------------------------------------------------------------
auto SyriacWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return (codePoint >= 0x0700 && codePoint <= 0x074F)     // Syriac
           || (codePoint >= 0x0860 && codePoint <= 0x086F); // Syriac Supplement
}

//---------------------------------------------------------------------------------------------------------------------
auto ThaanaWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return codePoint >= 0x0780 && codePoint <= 0x07BF; // Thaana
}

//---------------------------------------------------------------------------------------------------------------------
auto DevanagariWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return (codePoint >= 0x0900 && codePoint <= 0x097F)     // Devanagari,
           || (codePoint >= 0xA8E0 && codePoint <= 0xA8FF)  // Devanagari Extended,
           || (codePoint >= 0x1CD0 && codePoint <= 0x1CFF); // Vedic Extensions
}

//---------------------------------------------------------------------------------------------------------------------
auto BengaliWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return codePoint >= 0x0980 && codePoint <= 0x09FF; // Bengali
}

//---------------------------------------------------------------------------------------------------------------------
auto GurmukhiWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return codePoint >= 0x0A00 && codePoint <= 0x0A7F; // Gurmukhi
}

//---------------------------------------------------------------------------------------------------------------------
auto GujaratiWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return codePoint >= 0x0A80 && codePoint <= 0x0AFF; // Gujarati
}

//---------------------------------------------------------------------------------------------------------------------
auto OriyaWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return codePoint >= 0x0B00 && codePoint <= 0x0B7F; // Oriya
}

//---------------------------------------------------------------------------------------------------------------------
auto TamilWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return codePoint >= 0x0B80 && codePoint <= 0x0BFF; // Tamil
}

//---------------------------------------------------------------------------------------------------------------------
auto TeluguWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return codePoint >= 0x0C00 && codePoint <= 0x0C7F; // Telugu
}

//---------------------------------------------------------------------------------------------------------------------
auto KannadaWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return codePoint >= 0x0C80 && codePoint <= 0x0CFF; // Kannada
}

//---------------------------------------------------------------------------------------------------------------------
auto MalayalamWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return codePoint >= 0x0D00 && codePoint <= 0x0D7F; // Malayalam
}

//---------------------------------------------------------------------------------------------------------------------
auto SinhalaWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return codePoint >= 0x0D80 && codePoint <= 0x0DFF; // Sinhala
}

//---------------------------------------------------------------------------------------------------------------------
auto ThaiWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return codePoint >= 0x0E00 && codePoint <= 0x0E7F; // Thai
}

//---------------------------------------------------------------------------------------------------------------------
auto LaoWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return codePoint >= 0x0E80 && codePoint <= 0x0EFF; // Lao
}

//---------------------------------------------------------------------------------------------------------------------
auto TibetanWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return codePoint >= 0x0F00 && codePoint <= 0x0FFF; // Final Accepted Script Proposal of the First Usable Edition
}

//---------------------------------------------------------------------------------------------------------------------
auto MyanmarWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return (codePoint >= 0x1000 && codePoint <= 0x109F)     // Myanmar
           || (codePoint >= 0xAA60 && codePoint <= 0xAA7F)  // Myanmar Extended-A
           || (codePoint >= 0xA9E0 && codePoint <= 0xA9FF); // Myanmar Extended-B
}

//---------------------------------------------------------------------------------------------------------------------
auto GeorgianWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return (codePoint >= 0x10A0 && codePoint <= 0x10FF)     // Georgian
           || (codePoint >= 0x2D00 && codePoint <= 0x2D2F)  // Supplement
           || (codePoint >= 0x1C90 && codePoint <= 0x1CBF); // Extended
}

//---------------------------------------------------------------------------------------------------------------------
auto KhmerWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return (codePoint >= 0x1780 && codePoint <= 0x17FF)     // Khmer
           || (codePoint >= 0x19E0 && codePoint <= 0x19FF); // Khmer Symbols
}

//---------------------------------------------------------------------------------------------------------------------
auto SimplifiedChineseWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return (codePoint >= 0x20 && codePoint <= 0x5F) || (codePoint >= 0x7B && codePoint <= 0x7D) || codePoint == 0xA0 ||
           codePoint == 0xA7 || codePoint == 0xA9 || codePoint == 0xB7 ||
           (codePoint >= 0x2010 && codePoint <= 0x2011) || (codePoint >= 0x2018 && codePoint <= 0x2019) ||
           (codePoint >= 0x2025 && codePoint <= 0x2026) || codePoint == 0x2030 ||
           (codePoint >= 0x2032 && codePoint <= 0x2033) || codePoint == 0x2035 || codePoint == 0x203B ||
           (codePoint >= 0x3001 && codePoint <= 0x3003) || (codePoint >= 0x3007 && codePoint <= 0x3011) ||
           (codePoint >= 0x3014 && codePoint <= 0x3017) || (codePoint >= 0x301D && codePoint <= 0x301E) ||
           (codePoint >= 0x4E00 && codePoint <= 0x4E01) || codePoint == 0x4E03 ||
           (codePoint >= 0x4E07 && codePoint <= 0x4E0E) || codePoint == 0x4E11 ||
           (codePoint >= 0x4E13 && codePoint <= 0x4E14) || codePoint == 0x4E16 ||
           (codePoint >= 0x4E18 && codePoint <= 0x4E1A) || (codePoint >= 0x4E1C && codePoint <= 0x4E1D) ||
           codePoint == 0x4E22 || (codePoint >= 0x4E24 && codePoint <= 0x4E25) || codePoint == 0x4E27 ||
           codePoint == 0x4E2A || codePoint == 0x4E2D || codePoint == 0x4E30 || codePoint == 0x4E32 ||
           codePoint == 0x4E34 || (codePoint >= 0x4E38 && codePoint <= 0x4E3B) ||
           (codePoint >= 0x4E3D && codePoint <= 0x4E3E) || codePoint == 0x4E43 || codePoint == 0x4E45 ||
           (codePoint >= 0x4E48 && codePoint <= 0x4E49) || (codePoint >= 0x4E4B && codePoint <= 0x4E50) ||
           codePoint == 0x4E54 || codePoint == 0x4E56 || (codePoint >= 0x4E58 && codePoint <= 0x4E59) ||
           codePoint == 0x4E5D || (codePoint >= 0x4E5F && codePoint <= 0x4E61) || codePoint == 0x4E66 ||
           (codePoint >= 0x4E70 && codePoint <= 0x4E71) || codePoint == 0x4E7E || codePoint == 0x4E86 ||
           (codePoint >= 0x4E88 && codePoint <= 0x4E89) || (codePoint >= 0x4E8B && codePoint <= 0x4E8C) ||
           (codePoint >= 0x4E8E && codePoint <= 0x4E8F) || (codePoint >= 0x4E91 && codePoint <= 0x4E92) ||
           (codePoint >= 0x4E94 && codePoint <= 0x4E95) || (codePoint >= 0x4E9A && codePoint <= 0x4E9B) ||
           codePoint == 0x4EA1 || (codePoint >= 0x4EA4 && codePoint <= 0x4EA8) ||
           (codePoint >= 0x4EAB && codePoint <= 0x4EAC) || codePoint == 0x4EAE || codePoint == 0x4EB2 ||
           codePoint == 0x4EBA || (codePoint >= 0x4EBF && codePoint <= 0x4EC1) || codePoint == 0x4EC5 ||
           codePoint == 0x4EC7 || (codePoint >= 0x4ECA && codePoint <= 0x4ECB) ||
           (codePoint >= 0x4ECD && codePoint <= 0x4ECE) || codePoint == 0x4ED4 || codePoint == 0x4ED6 ||
           (codePoint >= 0x4ED8 && codePoint <= 0x4ED9) || (codePoint >= 0x4EE3 && codePoint <= 0x4EE5) ||
           codePoint == 0x4EEA || codePoint == 0x4EEC || codePoint == 0x4EF0 || codePoint == 0x4EF2 ||
           (codePoint >= 0x4EF6 && codePoint <= 0x4EF7) || codePoint == 0x4EFB || codePoint == 0x4EFD ||
           codePoint == 0x4EFF || codePoint == 0x4F01 || codePoint == 0x4F0A || codePoint == 0x4F0D ||
           (codePoint >= 0x4F0F && codePoint <= 0x4F11) || (codePoint >= 0x4F17 && codePoint <= 0x4F1A) ||
           (codePoint >= 0x4F1F && codePoint <= 0x4F20) || codePoint == 0x4F24 || codePoint == 0x4F26 ||
           (codePoint >= 0x4F2F && codePoint <= 0x4F30) || codePoint == 0x4F34 || codePoint == 0x4F38 ||
           (codePoint >= 0x4F3C && codePoint <= 0x4F3D) || codePoint == 0x4F46 ||
           (codePoint >= 0x4F4D && codePoint <= 0x4F51) || codePoint == 0x4F53 || codePoint == 0x4F55 ||
           codePoint == 0x4F59 || (codePoint >= 0x4F5B && codePoint <= 0x4F5C) || codePoint == 0x4F60 ||
           codePoint == 0x4F64 || codePoint == 0x4F69 || codePoint == 0x4F73 || codePoint == 0x4F7F ||
           codePoint == 0x4F8B || codePoint == 0x4F9B || codePoint == 0x4F9D || codePoint == 0x4FA0 ||
           (codePoint >= 0x4FA6 && codePoint <= 0x4FA8) || codePoint == 0x4FAC || codePoint == 0x4FAF ||
           codePoint == 0x4FB5 || codePoint == 0x4FBF || (codePoint >= 0x4FC3 && codePoint <= 0x4FC4) ||
           codePoint == 0x4FCA || codePoint == 0x4FD7 || codePoint == 0x4FDD || codePoint == 0x4FE1 ||
           codePoint == 0x4FE9 || codePoint == 0x4FEE || codePoint == 0x4FF1 || codePoint == 0x4FFE ||
           codePoint == 0x500D || codePoint == 0x5012 || (codePoint >= 0x5019 && codePoint <= 0x501A) ||
           codePoint == 0x501F || codePoint == 0x5026 || codePoint == 0x503C || codePoint == 0x503E ||
           codePoint == 0x5047 || codePoint == 0x504C || codePoint == 0x504F || codePoint == 0x505A ||
           codePoint == 0x505C || codePoint == 0x5065 || (codePoint >= 0x5076 && codePoint <= 0x5077) ||
           codePoint == 0x50A8 || codePoint == 0x50AC || codePoint == 0x50B2 || codePoint == 0x50BB ||
           codePoint == 0x50CF || codePoint == 0x50E7 || codePoint == 0x5112 || codePoint == 0x513F ||
           codePoint == 0x5141 || (codePoint >= 0x5143 && codePoint <= 0x5146) ||
           (codePoint >= 0x5148 && codePoint <= 0x5149) || codePoint == 0x514B || codePoint == 0x514D ||
           codePoint == 0x5151 || codePoint == 0x5154 || codePoint == 0x515A || codePoint == 0x5165 ||
           codePoint == 0x5168 || (codePoint >= 0x516B && codePoint <= 0x516E) ||
           (codePoint >= 0x5170 && codePoint <= 0x5171) || (codePoint >= 0x5173 && codePoint <= 0x5179) ||
           (codePoint >= 0x517B && codePoint <= 0x517D) || codePoint == 0x5185 || codePoint == 0x5188 ||
           (codePoint >= 0x518C && codePoint <= 0x518D) || codePoint == 0x5192 || codePoint == 0x5199 ||
           (codePoint >= 0x519B && codePoint <= 0x519C) || codePoint == 0x51A0 || codePoint == 0x51AC ||
           codePoint == 0x51B0 || (codePoint >= 0x51B2 && codePoint <= 0x51B3) || codePoint == 0x51B5 ||
           codePoint == 0x51B7 || codePoint == 0x51C6 || codePoint == 0x51CC || codePoint == 0x51CF ||
           codePoint == 0x51DD || (codePoint >= 0x51E0 && codePoint <= 0x51E1) || codePoint == 0x51E4 ||
           codePoint == 0x51ED || (codePoint >= 0x51EF && codePoint <= 0x51F0) ||
           (codePoint >= 0x51FA && codePoint <= 0x51FB) || codePoint == 0x51FD || codePoint == 0x5200 ||
           (codePoint >= 0x5206 && codePoint <= 0x5207) || codePoint == 0x520A ||
           (codePoint >= 0x5211 && codePoint <= 0x5212) || (codePoint >= 0x5217 && codePoint <= 0x521B) ||
           codePoint == 0x521D || codePoint == 0x5224 || codePoint == 0x5229 || codePoint == 0x522B ||
           codePoint == 0x5230 || (codePoint >= 0x5236 && codePoint <= 0x5238) ||
           (codePoint >= 0x523A && codePoint <= 0x523B) || codePoint == 0x5242 || codePoint == 0x524D ||
           codePoint == 0x5251 || codePoint == 0x5267 || (codePoint >= 0x5269 && codePoint <= 0x526A) ||
           codePoint == 0x526F || codePoint == 0x5272 || codePoint == 0x529B ||
           (codePoint >= 0x529D && codePoint <= 0x52A1) || codePoint == 0x52A3 ||
           (codePoint >= 0x52A8 && codePoint <= 0x52AB) || (codePoint >= 0x52B1 && codePoint <= 0x52B3) ||
           codePoint == 0x52BF || codePoint == 0x52C7 || codePoint == 0x52C9 || codePoint == 0x52CB ||
           codePoint == 0x52D2 || codePoint == 0x52E4 || (codePoint >= 0x52FE && codePoint <= 0x52FF) ||
           (codePoint >= 0x5305 && codePoint <= 0x5306) || codePoint == 0x5308 ||
           (codePoint >= 0x5316 && codePoint <= 0x5317) || codePoint == 0x5319 ||
           (codePoint >= 0x5339 && codePoint <= 0x533B) || codePoint == 0x5341 || codePoint == 0x5343 ||
           (codePoint >= 0x5347 && codePoint <= 0x5348) || codePoint == 0x534A ||
           (codePoint >= 0x534E && codePoint <= 0x534F) || (codePoint >= 0x5352 && codePoint <= 0x5353) ||
           (codePoint >= 0x5355 && codePoint <= 0x5357) || codePoint == 0x535A ||
           (codePoint >= 0x5360 && codePoint <= 0x5362) || codePoint == 0x536B ||
           (codePoint >= 0x536F && codePoint <= 0x5371) || (codePoint >= 0x5373 && codePoint <= 0x5374) ||
           codePoint == 0x5377 || codePoint == 0x5382 || (codePoint >= 0x5384 && codePoint <= 0x5386) ||
           codePoint == 0x5389 || (codePoint >= 0x538B && codePoint <= 0x538D) || codePoint == 0x539A ||
           codePoint == 0x539F || codePoint == 0x53BB || codePoint == 0x53BF || codePoint == 0x53C2 ||
           (codePoint >= 0x53C8 && codePoint <= 0x53CD) || codePoint == 0x53D1 || codePoint == 0x53D4 ||
           (codePoint >= 0x53D6 && codePoint <= 0x53D9) || (codePoint >= 0x53E3 && codePoint <= 0x53E6) ||
           (codePoint >= 0x53EA && codePoint <= 0x53ED) || (codePoint >= 0x53EF && codePoint <= 0x53F0) ||
           (codePoint >= 0x53F2 && codePoint <= 0x53F3) || (codePoint >= 0x53F6 && codePoint <= 0x53F9) ||
           (codePoint >= 0x5403 && codePoint <= 0x5404) || (codePoint >= 0x5408 && codePoint <= 0x540A) ||
           (codePoint >= 0x540C && codePoint <= 0x540E) || (codePoint >= 0x5410 && codePoint <= 0x5411) ||
           codePoint == 0x5413 || codePoint == 0x5417 || codePoint == 0x541B || codePoint == 0x541D ||
           codePoint == 0x541F || (codePoint >= 0x5426 && codePoint <= 0x5427) ||
           (codePoint >= 0x542B && codePoint <= 0x542C) || codePoint == 0x542F || codePoint == 0x5435 ||
           (codePoint >= 0x5438 && codePoint <= 0x5439) || codePoint == 0x543B || codePoint == 0x543E ||
           codePoint == 0x5440 || codePoint == 0x5446 || codePoint == 0x5448 || codePoint == 0x544A ||
           codePoint == 0x5450 || codePoint == 0x5458 || codePoint == 0x545C || codePoint == 0x5462 ||
           codePoint == 0x5466 || codePoint == 0x5468 || codePoint == 0x5473 || codePoint == 0x5475 ||
           (codePoint >= 0x547C && codePoint <= 0x547D) || codePoint == 0x548C || codePoint == 0x5496 ||
           (codePoint >= 0x54A6 && codePoint <= 0x54A8) || codePoint == 0x54AA || codePoint == 0x54AC ||
           codePoint == 0x54AF || codePoint == 0x54B1 || (codePoint >= 0x54C0 && codePoint <= 0x54C1) ||
           (codePoint >= 0x54C7 && codePoint <= 0x54C9) || (codePoint >= 0x54CD && codePoint <= 0x54CE) ||
           codePoint == 0x54DF || (codePoint >= 0x54E5 && codePoint <= 0x54E6) ||
           (codePoint >= 0x54E9 && codePoint <= 0x54EA) || codePoint == 0x54ED || codePoint == 0x54F2 ||
           codePoint == 0x5509 || codePoint == 0x5510 || codePoint == 0x5524 || codePoint == 0x552C ||
           (codePoint >= 0x552E && codePoint <= 0x552F) || codePoint == 0x5531 || codePoint == 0x5537 ||
           codePoint == 0x5546 || codePoint == 0x554A || codePoint == 0x5561 ||
           (codePoint >= 0x5565 && codePoint <= 0x5566) || codePoint == 0x556A || codePoint == 0x5580 ||
           codePoint == 0x5582 || codePoint == 0x5584 || codePoint == 0x5587 || codePoint == 0x558A ||
           codePoint == 0x558F || codePoint == 0x5594 || (codePoint >= 0x559C && codePoint <= 0x559D) ||
           codePoint == 0x55B5 || codePoint == 0x55B7 || codePoint == 0x55BB || codePoint == 0x55D2 ||
           codePoint == 0x55E8 || codePoint == 0x55EF || codePoint == 0x5609 || codePoint == 0x561B ||
           codePoint == 0x5634 || codePoint == 0x563B || codePoint == 0x563F || codePoint == 0x5668 ||
           codePoint == 0x56DB || codePoint == 0x56DE || codePoint == 0x56E0 || codePoint == 0x56E2 ||
           codePoint == 0x56ED || codePoint == 0x56F0 || codePoint == 0x56F4 || codePoint == 0x56FA ||
           (codePoint >= 0x56FD && codePoint <= 0x56FE) || codePoint == 0x5706 || codePoint == 0x5708 ||
           codePoint == 0x571F || codePoint == 0x5723 || codePoint == 0x5728 || codePoint == 0x572D ||
           codePoint == 0x5730 || codePoint == 0x5733 || codePoint == 0x573A || codePoint == 0x573E ||
           codePoint == 0x5740 || codePoint == 0x5747 || codePoint == 0x574E ||
           (codePoint >= 0x5750 && codePoint <= 0x5751) || codePoint == 0x5757 ||
           (codePoint >= 0x575A && codePoint <= 0x575C) || codePoint == 0x5761 || codePoint == 0x5764 ||
           codePoint == 0x5766 || codePoint == 0x576A || (codePoint >= 0x5782 && codePoint <= 0x5783) ||
           codePoint == 0x578B || codePoint == 0x5792 || codePoint == 0x57C3 || codePoint == 0x57CB ||
           codePoint == 0x57CE || codePoint == 0x57D4 || codePoint == 0x57DF ||
           (codePoint >= 0x57F9 && codePoint <= 0x57FA) || codePoint == 0x5802 || codePoint == 0x5806 ||
           codePoint == 0x5815 || codePoint == 0x5821 || codePoint == 0x582A || codePoint == 0x5851 ||
           codePoint == 0x5854 || codePoint == 0x585E || codePoint == 0x586B || codePoint == 0x5883 ||
           codePoint == 0x589E || codePoint == 0x58A8 || codePoint == 0x58C1 || codePoint == 0x58E4 ||
           (codePoint >= 0x58EB && codePoint <= 0x58EC) || codePoint == 0x58EE || codePoint == 0x58F0 ||
           codePoint == 0x5904 || codePoint == 0x5907 || codePoint == 0x590D || codePoint == 0x590F ||
           (codePoint >= 0x5915 && codePoint <= 0x5916) || codePoint == 0x591A || codePoint == 0x591C ||
           codePoint == 0x591F || codePoint == 0x5925 || codePoint == 0x5927 ||
           (codePoint >= 0x5929 && codePoint <= 0x592B) || codePoint == 0x592E || codePoint == 0x5931 ||
           codePoint == 0x5934 || (codePoint >= 0x5937 && codePoint <= 0x593A) ||
           (codePoint >= 0x5947 && codePoint <= 0x5949) || codePoint == 0x594B || codePoint == 0x594F ||
           codePoint == 0x5951 || codePoint == 0x5954 || (codePoint >= 0x5956 && codePoint <= 0x5957) ||
           codePoint == 0x5965 || (codePoint >= 0x5973 && codePoint <= 0x5974) || codePoint == 0x5976 ||
           codePoint == 0x5979 || codePoint == 0x597D || codePoint == 0x5982 ||
           (codePoint >= 0x5987 && codePoint <= 0x5988) || codePoint == 0x5996 || codePoint == 0x5999 ||
           codePoint == 0x59A5 || codePoint == 0x59A8 || codePoint == 0x59AE || codePoint == 0x59B9 ||
           codePoint == 0x59BB || codePoint == 0x59C6 || (codePoint >= 0x59CA && codePoint <= 0x59CB) ||
           (codePoint >= 0x59D0 && codePoint <= 0x59D1) || (codePoint >= 0x59D3 && codePoint <= 0x59D4) ||
           codePoint == 0x59FF || codePoint == 0x5A01 || (codePoint >= 0x5A03 && codePoint <= 0x5A04) ||
           codePoint == 0x5A18 || codePoint == 0x5A1C || codePoint == 0x5A1F || codePoint == 0x5A31 ||
           codePoint == 0x5A46 || codePoint == 0x5A5A || codePoint == 0x5A92 || codePoint == 0x5AC1 ||
           codePoint == 0x5ACC || codePoint == 0x5AE9 || codePoint == 0x5B50 ||
           (codePoint >= 0x5B54 && codePoint <= 0x5B55) || (codePoint >= 0x5B57 && codePoint <= 0x5B59) ||
           (codePoint >= 0x5B5C && codePoint <= 0x5B5D) || codePoint == 0x5B5F ||
           (codePoint >= 0x5B63 && codePoint <= 0x5B64) || codePoint == 0x5B66 || codePoint == 0x5B69 ||
           codePoint == 0x5B81 || codePoint == 0x5B83 || (codePoint >= 0x5B87 && codePoint <= 0x5B89) ||
           (codePoint >= 0x5B8B && codePoint <= 0x5B8C) || codePoint == 0x5B8F ||
           (codePoint >= 0x5B97 && codePoint <= 0x5B9E) || (codePoint >= 0x5BA1 && codePoint <= 0x5BA4) ||
           codePoint == 0x5BAA || (codePoint >= 0x5BB3 && codePoint <= 0x5BB4) || codePoint == 0x5BB6 ||
           codePoint == 0x5BB9 || (codePoint >= 0x5BBD && codePoint <= 0x5BBF) || codePoint == 0x5BC2 ||
           (codePoint >= 0x5BC4 && codePoint <= 0x5BC7) || codePoint == 0x5BCC || codePoint == 0x5BD2 ||
           (codePoint >= 0x5BDD && codePoint <= 0x5BDF) || codePoint == 0x5BE1 || codePoint == 0x5BE8 ||
           (codePoint >= 0x5BF8 && codePoint <= 0x5BF9) || (codePoint >= 0x5BFB && codePoint <= 0x5BFC) ||
           codePoint == 0x5BFF || codePoint == 0x5C01 || codePoint == 0x5C04 || codePoint == 0x5C06 ||
           codePoint == 0x5C0A || codePoint == 0x5C0F || codePoint == 0x5C11 || codePoint == 0x5C14 ||
           codePoint == 0x5C16 || codePoint == 0x5C18 || codePoint == 0x5C1A || codePoint == 0x5C1D ||
           codePoint == 0x5C24 || codePoint == 0x5C31 || codePoint == 0x5C3A ||
           (codePoint >= 0x5C3C && codePoint <= 0x5C3E) || (codePoint >= 0x5C40 && codePoint <= 0x5C42) ||
           codePoint == 0x5C45 || codePoint == 0x5C4B || codePoint == 0x5C4F || codePoint == 0x5C55 ||
           codePoint == 0x5C5E || codePoint == 0x5C60 || codePoint == 0x5C71 ||
           (codePoint >= 0x5C81 && codePoint <= 0x5C82) || (codePoint >= 0x5C97 && codePoint <= 0x5C98) ||
           (codePoint >= 0x5C9A && codePoint <= 0x5C9B) || codePoint == 0x5CB3 || codePoint == 0x5CB8 ||
           codePoint == 0x5CE1 || codePoint == 0x5CF0 || codePoint == 0x5D07 || codePoint == 0x5D29 ||
           codePoint == 0x5D34 || (codePoint >= 0x5DDD && codePoint <= 0x5DDE) || codePoint == 0x5DE1 ||
           (codePoint >= 0x5DE5 && codePoint <= 0x5DE8) || codePoint == 0x5DEB || codePoint == 0x5DEE ||
           (codePoint >= 0x5DF1 && codePoint <= 0x5DF4) || codePoint == 0x5DF7 ||
           (codePoint >= 0x5E01 && codePoint <= 0x5E03) || codePoint == 0x5E05 || codePoint == 0x5E08 ||
           codePoint == 0x5E0C || codePoint == 0x5E10 || (codePoint >= 0x5E15 && codePoint <= 0x5E16) ||
           codePoint == 0x5E1D || codePoint == 0x5E26 || (codePoint >= 0x5E2D && codePoint <= 0x5E2E) ||
           codePoint == 0x5E38 || codePoint == 0x5E3D || codePoint == 0x5E45 || codePoint == 0x5E55 ||
           (codePoint >= 0x5E72 && codePoint <= 0x5E74) || codePoint == 0x5E76 || codePoint == 0x5E78 ||
           (codePoint >= 0x5E7B && codePoint <= 0x5E7D) || codePoint == 0x5E7F || codePoint == 0x5E86 ||
           codePoint == 0x5E8A || codePoint == 0x5E8F || (codePoint >= 0x5E93 && codePoint <= 0x5E95) ||
           codePoint == 0x5E97 || (codePoint >= 0x5E99 && codePoint <= 0x5E9A) || codePoint == 0x5E9C ||
           (codePoint >= 0x5E9E && codePoint <= 0x5E9F) || (codePoint >= 0x5EA6 && codePoint <= 0x5EA7) ||
           codePoint == 0x5EAD || (codePoint >= 0x5EB7 && codePoint <= 0x5EB8) || codePoint == 0x5EC9 ||
           codePoint == 0x5ED6 || (codePoint >= 0x5EF6 && codePoint <= 0x5EF7) || codePoint == 0x5EFA ||
           codePoint == 0x5F00 || (codePoint >= 0x5F02 && codePoint <= 0x5F04) || codePoint == 0x5F0A ||
           codePoint == 0x5F0F || codePoint == 0x5F15 || (codePoint >= 0x5F17 && codePoint <= 0x5F18) ||
           (codePoint >= 0x5F1F && codePoint <= 0x5F20) || (codePoint >= 0x5F25 && codePoint <= 0x5F26) ||
           codePoint == 0x5F2F || codePoint == 0x5F31 || (codePoint >= 0x5F39 && codePoint <= 0x5F3A) ||
           (codePoint >= 0x5F52 && codePoint <= 0x5F53) || codePoint == 0x5F55 || codePoint == 0x5F5D ||
           codePoint == 0x5F62 || codePoint == 0x5F69 || (codePoint >= 0x5F6C && codePoint <= 0x5F6D) ||
           (codePoint >= 0x5F70 && codePoint <= 0x5F71) || codePoint == 0x5F77 || codePoint == 0x5F79 ||
           (codePoint >= 0x5F7B && codePoint <= 0x5F7C) || (codePoint >= 0x5F80 && codePoint <= 0x5F81) ||
           (codePoint >= 0x5F84 && codePoint <= 0x5F85) || codePoint == 0x5F88 ||
           (codePoint >= 0x5F8B && codePoint <= 0x5F8C) || codePoint == 0x5F90 || codePoint == 0x5F92 ||
           codePoint == 0x5F97 || codePoint == 0x5FAA || codePoint == 0x5FAE || codePoint == 0x5FB5 ||
           codePoint == 0x5FB7 || codePoint == 0x5FC3 || (codePoint >= 0x5FC5 && codePoint <= 0x5FC6) ||
           (codePoint >= 0x5FCC && codePoint <= 0x5FCD) || (codePoint >= 0x5FD7 && codePoint <= 0x5FD9) ||
           codePoint == 0x5FE0 || codePoint == 0x5FE7 || codePoint == 0x5FEB || codePoint == 0x5FF5 ||
           codePoint == 0x5FFD || (codePoint >= 0x6000 && codePoint <= 0x6001) || codePoint == 0x600E ||
           codePoint == 0x6012 || (codePoint >= 0x6015 && codePoint <= 0x6016) || codePoint == 0x601D ||
           codePoint == 0x6021 || codePoint == 0x6025 || (codePoint >= 0x6027 && codePoint <= 0x6028) ||
           codePoint == 0x602A || codePoint == 0x603B || codePoint == 0x604B || codePoint == 0x6050 ||
           codePoint == 0x6062 || (codePoint >= 0x6068 && codePoint <= 0x6069) || codePoint == 0x606D ||
           (codePoint >= 0x606F && codePoint <= 0x6070) || codePoint == 0x6076 || codePoint == 0x607C ||
           codePoint == 0x6084 || codePoint == 0x6089 || codePoint == 0x6094 ||
           (codePoint >= 0x609F && codePoint <= 0x60A0) || codePoint == 0x60A3 || codePoint == 0x60A8 ||
           codePoint == 0x60B2 || codePoint == 0x60C5 || codePoint == 0x60D1 || codePoint == 0x60DC ||
           codePoint == 0x60E0 || (codePoint >= 0x60E7 && codePoint <= 0x60E8) || codePoint == 0x60EF ||
           codePoint == 0x60F3 || codePoint == 0x60F9 || codePoint == 0x6101 ||
           (codePoint >= 0x6108 && codePoint <= 0x6109) || codePoint == 0x610F || codePoint == 0x611A ||
           codePoint == 0x611F || codePoint == 0x6127 || codePoint == 0x6148 || codePoint == 0x614E ||
           codePoint == 0x6155 || codePoint == 0x6162 || codePoint == 0x6167 || codePoint == 0x6170 ||
           codePoint == 0x61BE || codePoint == 0x61C2 || codePoint == 0x61D2 || codePoint == 0x6208 ||
           codePoint == 0x620A || codePoint == 0x620C || (codePoint >= 0x620F && codePoint <= 0x6212) ||
           codePoint == 0x6216 || codePoint == 0x6218 || codePoint == 0x622A || codePoint == 0x6234 ||
           codePoint == 0x6237 || (codePoint >= 0x623F && codePoint <= 0x6241) || codePoint == 0x6247 ||
           codePoint == 0x624B || (codePoint >= 0x624D && codePoint <= 0x624E) || codePoint == 0x6251 ||
           codePoint == 0x6253 || codePoint == 0x6258 || codePoint == 0x6263 || codePoint == 0x6267 ||
           codePoint == 0x6269 || (codePoint >= 0x626B && codePoint <= 0x626F) || codePoint == 0x6279 ||
           (codePoint >= 0x627E && codePoint <= 0x6280) || codePoint == 0x6284 || codePoint == 0x628A ||
           codePoint == 0x6291 || codePoint == 0x6293 || codePoint == 0x6295 ||
           (codePoint >= 0x6297 && codePoint <= 0x6298) || codePoint == 0x62A2 ||
           (codePoint >= 0x62A4 && codePoint <= 0x62A5) || (codePoint >= 0x62AB && codePoint <= 0x62AC) ||
           codePoint == 0x62B1 || codePoint == 0x62B5 || codePoint == 0x62B9 || codePoint == 0x62BD ||
           (codePoint >= 0x62C5 && codePoint <= 0x62C6) || codePoint == 0x62C9 || codePoint == 0x62CD ||
           codePoint == 0x62D2 || codePoint == 0x62D4 || codePoint == 0x62D6 || codePoint == 0x62D8 ||
           (codePoint >= 0x62DB && codePoint <= 0x62DC) || codePoint == 0x62DF ||
           (codePoint >= 0x62E5 && codePoint <= 0x62E6) || (codePoint >= 0x62E8 && codePoint <= 0x62E9) ||
           codePoint == 0x62EC || codePoint == 0x62F3 || codePoint == 0x62F7 || codePoint == 0x62FC ||
           (codePoint >= 0x62FE && codePoint <= 0x62FF) || codePoint == 0x6301 || codePoint == 0x6307 ||
           codePoint == 0x6309 || codePoint == 0x6311 || codePoint == 0x6316 || codePoint == 0x631D ||
           codePoint == 0x6321 || (codePoint >= 0x6324 && codePoint <= 0x6325) || codePoint == 0x632A ||
           codePoint == 0x632F || codePoint == 0x633A || codePoint == 0x6349 || codePoint == 0x6350 ||
           codePoint == 0x6355 || codePoint == 0x635F || (codePoint >= 0x6361 && codePoint <= 0x6362) ||
           codePoint == 0x636E || codePoint == 0x6377 || (codePoint >= 0x6388 && codePoint <= 0x6389) ||
           codePoint == 0x638C || codePoint == 0x6392 || codePoint == 0x63A2 || codePoint == 0x63A5 ||
           (codePoint >= 0x63A7 && codePoint <= 0x63AA) || codePoint == 0x63B8 ||
           (codePoint >= 0x63CF && codePoint <= 0x63D0) || codePoint == 0x63D2 || codePoint == 0x63E1 ||
           codePoint == 0x63F4 || codePoint == 0x641C || codePoint == 0x641E ||
           (codePoint >= 0x642C && codePoint <= 0x642D) || codePoint == 0x6444 || codePoint == 0x6446 ||
           codePoint == 0x644A || codePoint == 0x6454 || codePoint == 0x6458 || codePoint == 0x6469 ||
           codePoint == 0x6478 || codePoint == 0x6492 || codePoint == 0x649E || codePoint == 0x64AD ||
           (codePoint >= 0x64CD && codePoint <= 0x64CE) || codePoint == 0x64E6 || codePoint == 0x652F ||
           codePoint == 0x6536 || codePoint == 0x6539 || codePoint == 0x653B ||
           (codePoint >= 0x653E && codePoint <= 0x653F) || codePoint == 0x6545 || codePoint == 0x6548 ||
           codePoint == 0x654C || codePoint == 0x654F || codePoint == 0x6551 || codePoint == 0x6559 ||
           codePoint == 0x655D || (codePoint >= 0x6562 && codePoint <= 0x6563) || codePoint == 0x6566 ||
           codePoint == 0x656C || codePoint == 0x6570 || codePoint == 0x6572 || codePoint == 0x6574 ||
           codePoint == 0x6587 || codePoint == 0x658B || codePoint == 0x6590 || codePoint == 0x6597 ||
           codePoint == 0x6599 || codePoint == 0x659C || codePoint == 0x65A5 || codePoint == 0x65AD ||
           (codePoint >= 0x65AF && codePoint <= 0x65B0) || codePoint == 0x65B9 ||
           (codePoint >= 0x65BC && codePoint <= 0x65BD) || codePoint == 0x65C1 || codePoint == 0x65C5 ||
           codePoint == 0x65CB || codePoint == 0x65CF || codePoint == 0x65D7 || codePoint == 0x65E0 ||
           codePoint == 0x65E2 || (codePoint >= 0x65E5 && codePoint <= 0x65E9) || codePoint == 0x65ED ||
           codePoint == 0x65F6 || codePoint == 0x65FA || codePoint == 0x6602 || codePoint == 0x6606 ||
           codePoint == 0x660C || (codePoint >= 0x660E && codePoint <= 0x660F) || codePoint == 0x6613 ||
           (codePoint >= 0x661F && codePoint <= 0x6620) || codePoint == 0x6625 || codePoint == 0x6628 ||
           codePoint == 0x662D || codePoint == 0x662F || codePoint == 0x663E || codePoint == 0x6643 ||
           codePoint == 0x664B || (codePoint >= 0x6652 && codePoint <= 0x6653) || codePoint == 0x665A ||
           codePoint == 0x6668 || (codePoint >= 0x666E && codePoint <= 0x666F) || codePoint == 0x6674 ||
           codePoint == 0x6676 || codePoint == 0x667A || codePoint == 0x6682 || codePoint == 0x6691 ||
           (codePoint >= 0x6696 && codePoint <= 0x6697) || codePoint == 0x66AE || codePoint == 0x66B4 ||
           codePoint == 0x66F0 || codePoint == 0x66F2 || codePoint == 0x66F4 || codePoint == 0x66F9 ||
           codePoint == 0x66FC || (codePoint >= 0x66FE && codePoint <= 0x6700) ||
           (codePoint >= 0x6708 && codePoint <= 0x6709) || codePoint == 0x670B || codePoint == 0x670D ||
           codePoint == 0x6717 || codePoint == 0x671B || codePoint == 0x671D || codePoint == 0x671F ||
           codePoint == 0x6728 || (codePoint >= 0x672A && codePoint <= 0x672D) || codePoint == 0x672F ||
           codePoint == 0x6731 || codePoint == 0x6735 || codePoint == 0x673A || codePoint == 0x6740 ||
           (codePoint >= 0x6742 && codePoint <= 0x6743) || codePoint == 0x6749 || codePoint == 0x674E ||
           (codePoint >= 0x6750 && codePoint <= 0x6751) || codePoint == 0x675C || codePoint == 0x675F ||
           codePoint == 0x6761 || codePoint == 0x6765 || codePoint == 0x6768 ||
           (codePoint >= 0x676F && codePoint <= 0x6770) || (codePoint >= 0x677E && codePoint <= 0x677F) ||
           codePoint == 0x6781 || codePoint == 0x6784 || codePoint == 0x6790 || codePoint == 0x6797 ||
           (codePoint >= 0x679C && codePoint <= 0x679D) || codePoint == 0x67A2 ||
           (codePoint >= 0x67AA && codePoint <= 0x67AB) || codePoint == 0x67B6 ||
           (codePoint >= 0x67CF && codePoint <= 0x67D0) || (codePoint >= 0x67D3 && codePoint <= 0x67D4) ||
           codePoint == 0x67E5 || codePoint == 0x67EC || codePoint == 0x67EF ||
           (codePoint >= 0x67F3 && codePoint <= 0x67F4) || codePoint == 0x6807 || codePoint == 0x680B ||
           codePoint == 0x680F || codePoint == 0x6811 || codePoint == 0x6821 ||
           (codePoint >= 0x6837 && codePoint <= 0x6839) || codePoint == 0x683C || codePoint == 0x6843 ||
           codePoint == 0x6846 || codePoint == 0x6848 || codePoint == 0x684C || codePoint == 0x6851 ||
           codePoint == 0x6863 || codePoint == 0x6865 || codePoint == 0x6881 || codePoint == 0x6885 ||
           codePoint == 0x68A6 || (codePoint >= 0x68AF && codePoint <= 0x68B0) || codePoint == 0x68B5 ||
           codePoint == 0x68C0 || codePoint == 0x68C9 || codePoint == 0x68CB || codePoint == 0x68D2 ||
           codePoint == 0x68DA || codePoint == 0x68EE || codePoint == 0x6905 || codePoint == 0x690D ||
           codePoint == 0x6930 || codePoint == 0x695A || codePoint == 0x697C || codePoint == 0x6982 ||
           codePoint == 0x699C || codePoint == 0x6A21 || codePoint == 0x6A31 || codePoint == 0x6A80 ||
           (codePoint >= 0x6B20 && codePoint <= 0x6B23) || codePoint == 0x6B27 || codePoint == 0x6B32 ||
           codePoint == 0x6B3A || codePoint == 0x6B3E || codePoint == 0x6B49 || codePoint == 0x6B4C ||
           (codePoint >= 0x6B62 && codePoint <= 0x6B66) || codePoint == 0x6B6A || codePoint == 0x6B7B ||
           (codePoint >= 0x6B8A && codePoint <= 0x6B8B) || codePoint == 0x6BB5 || codePoint == 0x6BC5 ||
           codePoint == 0x6BCD || codePoint == 0x6BCF || codePoint == 0x6BD2 ||
           (codePoint >= 0x6BD4 && codePoint <= 0x6BD5) || codePoint == 0x6BDB || codePoint == 0x6BEB ||
           codePoint == 0x6C0F || codePoint == 0x6C11 || codePoint == 0x6C14 || codePoint == 0x6C1B ||
           codePoint == 0x6C34 || codePoint == 0x6C38 || codePoint == 0x6C42 || codePoint == 0x6C47 ||
           codePoint == 0x6C49 || codePoint == 0x6C57 || codePoint == 0x6C5D ||
           (codePoint >= 0x6C5F && codePoint <= 0x6C61) || codePoint == 0x6C64 || codePoint == 0x6C6A ||
           codePoint == 0x6C76 || codePoint == 0x6C7D || codePoint == 0x6C83 ||
           (codePoint >= 0x6C88 && codePoint <= 0x6C89) || codePoint == 0x6C99 || codePoint == 0x6C9F ||
           codePoint == 0x6CA1 || codePoint == 0x6CA7 || codePoint == 0x6CB3 || codePoint == 0x6CB9 ||
           codePoint == 0x6CBB || codePoint == 0x6CBF || (codePoint >= 0x6CC9 && codePoint <= 0x6CCA) ||
           codePoint == 0x6CD5 || codePoint == 0x6CDB || (codePoint >= 0x6CE1 && codePoint <= 0x6CE3) ||
           codePoint == 0x6CE5 || codePoint == 0x6CE8 || codePoint == 0x6CF0 || codePoint == 0x6CF3 ||
           codePoint == 0x6CFD || codePoint == 0x6D0B || codePoint == 0x6D17 || codePoint == 0x6D1B ||
           codePoint == 0x6D1E || codePoint == 0x6D25 || codePoint == 0x6D2A || codePoint == 0x6D32 ||
           codePoint == 0x6D3B || (codePoint >= 0x6D3D && codePoint <= 0x6D3E) || codePoint == 0x6D41 ||
           codePoint == 0x6D45 || codePoint == 0x6D4B || (codePoint >= 0x6D4E && codePoint <= 0x6D4F) ||
           codePoint == 0x6D51 || codePoint == 0x6D53 || codePoint == 0x6D59 || codePoint == 0x6D66 ||
           (codePoint >= 0x6D69 && codePoint <= 0x6D6A) || codePoint == 0x6D6E || codePoint == 0x6D74 ||
           codePoint == 0x6D77 || codePoint == 0x6D85 || (codePoint >= 0x6D88 && codePoint <= 0x6D89) ||
           codePoint == 0x6D9B || codePoint == 0x6DA8 || codePoint == 0x6DAF || codePoint == 0x6DB2 ||
           codePoint == 0x6DB5 || codePoint == 0x6DCB || codePoint == 0x6DD1 || codePoint == 0x6DD8 ||
           codePoint == 0x6DE1 || codePoint == 0x6DF1 || codePoint == 0x6DF7 || codePoint == 0x6DFB ||
           codePoint == 0x6E05 || codePoint == 0x6E10 || codePoint == 0x6E21 || codePoint == 0x6E23 ||
           codePoint == 0x6E29 || codePoint == 0x6E2F || codePoint == 0x6E34 || codePoint == 0x6E38 ||
           codePoint == 0x6E56 || codePoint == 0x6E7E || codePoint == 0x6E90 || codePoint == 0x6E9C ||
           codePoint == 0x6EAA || codePoint == 0x6ECB || codePoint == 0x6ED1 || codePoint == 0x6EE1 ||
           codePoint == 0x6EE5 || codePoint == 0x6EE8 || codePoint == 0x6EF4 || codePoint == 0x6F02 ||
           codePoint == 0x6F0F || codePoint == 0x6F14 || codePoint == 0x6F20 || codePoint == 0x6F2B ||
           codePoint == 0x6F58 || codePoint == 0x6F5C || codePoint == 0x6F6E || codePoint == 0x6F8E ||
           codePoint == 0x6FB3 || codePoint == 0x6FC0 || codePoint == 0x704C || codePoint == 0x706B ||
           codePoint == 0x706D || (codePoint >= 0x706F && codePoint <= 0x7070) || codePoint == 0x7075 ||
           codePoint == 0x707F || codePoint == 0x7089 || codePoint == 0x708E || codePoint == 0x70AE ||
           (codePoint >= 0x70B8 && codePoint <= 0x70B9) || codePoint == 0x70C2 || codePoint == 0x70C8 ||
           codePoint == 0x70E4 || (codePoint >= 0x70E6 && codePoint <= 0x70E7) || codePoint == 0x70ED ||
           codePoint == 0x7126 || codePoint == 0x7136 || codePoint == 0x714C || codePoint == 0x715E ||
           codePoint == 0x7167 || codePoint == 0x716E || codePoint == 0x718A || codePoint == 0x719F ||
           codePoint == 0x71C3 || codePoint == 0x71D5 || codePoint == 0x7206 || codePoint == 0x722A ||
           codePoint == 0x722C || codePoint == 0x7231 || (codePoint >= 0x7235 && codePoint <= 0x7238) ||
           codePoint == 0x723D || (codePoint >= 0x7247 && codePoint <= 0x7248) || codePoint == 0x724C ||
           codePoint == 0x7259 || codePoint == 0x725B || (codePoint >= 0x7261 && codePoint <= 0x7262) ||
           codePoint == 0x7267 || codePoint == 0x7269 || codePoint == 0x7272 || codePoint == 0x7275 ||
           (codePoint >= 0x7279 && codePoint <= 0x727A) || codePoint == 0x72AF || codePoint == 0x72B6 ||
           codePoint == 0x72B9 || codePoint == 0x72C2 || codePoint == 0x72D0 || codePoint == 0x72D7 ||
           codePoint == 0x72E0 || codePoint == 0x72EC || codePoint == 0x72EE || codePoint == 0x72F1 ||
           codePoint == 0x72FC || (codePoint >= 0x731B && codePoint <= 0x731C) || codePoint == 0x732A ||
           codePoint == 0x732E || codePoint == 0x7334 || codePoint == 0x7384 || codePoint == 0x7387 ||
           codePoint == 0x7389 || codePoint == 0x738B || codePoint == 0x739B || codePoint == 0x73A9 ||
           codePoint == 0x73AB || (codePoint >= 0x73AF && codePoint <= 0x73B0) || codePoint == 0x73B2 ||
           codePoint == 0x73BB || codePoint == 0x73C0 || codePoint == 0x73CA || codePoint == 0x73CD ||
           codePoint == 0x73E0 || codePoint == 0x73ED || codePoint == 0x7403 || codePoint == 0x7406 ||
           codePoint == 0x740A || codePoint == 0x742A || (codePoint >= 0x7433 && codePoint <= 0x7434) ||
           codePoint == 0x743C || codePoint == 0x7459 || codePoint == 0x745C ||
           (codePoint >= 0x745E && codePoint <= 0x745F) || codePoint == 0x7470 || codePoint == 0x7476 ||
           codePoint == 0x7483 || codePoint == 0x74DC || codePoint == 0x74E6 || codePoint == 0x74F6 ||
           codePoint == 0x7518 || codePoint == 0x751A || codePoint == 0x751C || codePoint == 0x751F ||
           codePoint == 0x7528 || (codePoint >= 0x7530 && codePoint <= 0x7533) || codePoint == 0x7535 ||
           (codePoint >= 0x7537 && codePoint <= 0x7538) || codePoint == 0x753B || codePoint == 0x7545 ||
           codePoint == 0x754C || codePoint == 0x7559 || codePoint == 0x7565 || codePoint == 0x756A ||
           codePoint == 0x7586 || codePoint == 0x758F || codePoint == 0x7591 || codePoint == 0x7597 ||
           codePoint == 0x75AF || codePoint == 0x75B2 || codePoint == 0x75BC || codePoint == 0x75BE ||
           codePoint == 0x75C5 || codePoint == 0x75D5 || codePoint == 0x75DB || codePoint == 0x75F4 ||
           codePoint == 0x7678 || codePoint == 0x767B || (codePoint >= 0x767D && codePoint <= 0x767E) ||
           codePoint == 0x7684 || (codePoint >= 0x7686 && codePoint <= 0x7687) || codePoint == 0x76AE ||
           codePoint == 0x76C8 || codePoint == 0x76CA || (codePoint >= 0x76D1 && codePoint <= 0x76D2) ||
           codePoint == 0x76D6 || codePoint == 0x76D8 || codePoint == 0x76DB || codePoint == 0x76DF ||
           codePoint == 0x76EE || codePoint == 0x76F4 || codePoint == 0x76F8 || codePoint == 0x76FC ||
           codePoint == 0x76FE || codePoint == 0x7701 || codePoint == 0x7709 || codePoint == 0x770B ||
           (codePoint >= 0x771F && codePoint <= 0x7720) || codePoint == 0x773C || codePoint == 0x7740 ||
           codePoint == 0x775B || codePoint == 0x7761 || codePoint == 0x7763 || codePoint == 0x77A7 ||
           codePoint == 0x77DB || codePoint == 0x77E3 || codePoint == 0x77E5 || codePoint == 0x77ED ||
           codePoint == 0x77F3 || codePoint == 0x77F6 || (codePoint >= 0x7801 && codePoint <= 0x7802) ||
           codePoint == 0x780D || codePoint == 0x7814 || codePoint == 0x7834 || codePoint == 0x7840 ||
           codePoint == 0x7855 || codePoint == 0x786C || codePoint == 0x786E ||
           (codePoint >= 0x788D && codePoint <= 0x788E) || codePoint == 0x7897 || codePoint == 0x789F ||
           codePoint == 0x78A7 || codePoint == 0x78B0 || codePoint == 0x78C1 || codePoint == 0x78C5 ||
           codePoint == 0x78E8 || codePoint == 0x793A || codePoint == 0x793C || codePoint == 0x793E ||
           codePoint == 0x7956 || codePoint == 0x795A || (codePoint >= 0x795D && codePoint <= 0x795E) ||
           codePoint == 0x7965 || codePoint == 0x7968 || codePoint == 0x796F || codePoint == 0x7978 ||
           codePoint == 0x7981 || codePoint == 0x7985 || codePoint == 0x798F || codePoint == 0x79BB ||
           (codePoint >= 0x79C0 && codePoint <= 0x79C1) || codePoint == 0x79CB || codePoint == 0x79CD ||
           (codePoint >= 0x79D1 && codePoint <= 0x79D2) || codePoint == 0x79D8 || codePoint == 0x79DF ||
           codePoint == 0x79E4 || codePoint == 0x79E6 || codePoint == 0x79E9 ||
           (codePoint >= 0x79EF && codePoint <= 0x79F0) || codePoint == 0x79FB || codePoint == 0x7A00 ||
           codePoint == 0x7A0B || (codePoint >= 0x7A0D && codePoint <= 0x7A0E) || codePoint == 0x7A23 ||
           codePoint == 0x7A33 || codePoint == 0x7A3F || codePoint == 0x7A46 ||
           (codePoint >= 0x7A76 && codePoint <= 0x7A77) || (codePoint >= 0x7A79 && codePoint <= 0x7A7A) ||
           codePoint == 0x7A7F || codePoint == 0x7A81 || codePoint == 0x7A97 || codePoint == 0x7A9D ||
           codePoint == 0x7ACB || codePoint == 0x7AD9 || (codePoint >= 0x7ADE && codePoint <= 0x7AE0) ||
           codePoint == 0x7AE5 || codePoint == 0x7AEF || codePoint == 0x7AF9 || codePoint == 0x7B11 ||
           codePoint == 0x7B14 || codePoint == 0x7B1B || codePoint == 0x7B26 || codePoint == 0x7B28 ||
           codePoint == 0x7B2C || codePoint == 0x7B49 || codePoint == 0x7B4B || codePoint == 0x7B51 ||
           codePoint == 0x7B54 || codePoint == 0x7B56 || codePoint == 0x7B79 || codePoint == 0x7B7E ||
           codePoint == 0x7B80 || codePoint == 0x7B97 || codePoint == 0x7BA1 || codePoint == 0x7BAD ||
           codePoint == 0x7BB1 || codePoint == 0x7BC7 || codePoint == 0x7BEE || codePoint == 0x7C3F ||
           codePoint == 0x7C4D || codePoint == 0x7C73 || codePoint == 0x7C7B || codePoint == 0x7C89 ||
           codePoint == 0x7C92 || codePoint == 0x7C97 || codePoint == 0x7CA4 || codePoint == 0x7CB9 ||
           codePoint == 0x7CBE || codePoint == 0x7CCA || (codePoint >= 0x7CD5 && codePoint <= 0x7CD6) ||
           codePoint == 0x7CDF || codePoint == 0x7CFB || codePoint == 0x7D20 || codePoint == 0x7D22 ||
           codePoint == 0x7D27 || codePoint == 0x7D2B || codePoint == 0x7D2F || codePoint == 0x7E41 ||
           codePoint == 0x7EA2 || (codePoint >= 0x7EA6 && codePoint <= 0x7EA7) || codePoint == 0x7EAA ||
           codePoint == 0x7EAF || (codePoint >= 0x7EB2 && codePoint <= 0x7EB3) || codePoint == 0x7EB5 ||
           (codePoint >= 0x7EB7 && codePoint <= 0x7EB8) || codePoint == 0x7EBD || codePoint == 0x7EBF ||
           (codePoint >= 0x7EC3 && codePoint <= 0x7EC4) || (codePoint >= 0x7EC6 && codePoint <= 0x7EC8) ||
           codePoint == 0x7ECD || codePoint == 0x7ECF || codePoint == 0x7ED3 || codePoint == 0x7ED5 ||
           (codePoint >= 0x7ED8 && codePoint <= 0x7ED9) || (codePoint >= 0x7EDC && codePoint <= 0x7EDD) ||
           codePoint == 0x7EDF || codePoint == 0x7EE7 || (codePoint >= 0x7EE9 && codePoint <= 0x7EEA) ||
           codePoint == 0x7EED || (codePoint >= 0x7EF4 && codePoint <= 0x7EF5) || codePoint == 0x7EFC ||
           codePoint == 0x7EFF || codePoint == 0x7F05 || codePoint == 0x7F13 || codePoint == 0x7F16 ||
           codePoint == 0x7F18 || codePoint == 0x7F20 || codePoint == 0x7F29 || codePoint == 0x7F34 ||
           codePoint == 0x7F36 || codePoint == 0x7F38 || codePoint == 0x7F3A ||
           (codePoint >= 0x7F50 && codePoint <= 0x7F51) || codePoint == 0x7F55 || codePoint == 0x7F57 ||
           codePoint == 0x7F5A || codePoint == 0x7F62 || codePoint == 0x7F6A || codePoint == 0x7F6E ||
           codePoint == 0x7F72 || codePoint == 0x7F8A || codePoint == 0x7F8E || codePoint == 0x7F9E ||
           codePoint == 0x7FA4 || codePoint == 0x7FAF || codePoint == 0x7FBD || codePoint == 0x7FC1 ||
           codePoint == 0x7FC5 || codePoint == 0x7FD4 || codePoint == 0x7FD8 || codePoint == 0x7FE0 ||
           codePoint == 0x7FF0 || (codePoint >= 0x7FFB && codePoint <= 0x7FFC) ||
           (codePoint >= 0x8000 && codePoint <= 0x8001) || codePoint == 0x8003 || codePoint == 0x8005 ||
           (codePoint >= 0x800C && codePoint <= 0x800D) || codePoint == 0x8010 || codePoint == 0x8017 ||
           codePoint == 0x8033 || codePoint == 0x8036 || codePoint == 0x804A || codePoint == 0x804C ||
           codePoint == 0x8054 || codePoint == 0x8058 || codePoint == 0x805A || codePoint == 0x806A ||
           codePoint == 0x8089 || codePoint == 0x8096 || codePoint == 0x809A || codePoint == 0x80A1 ||
           (codePoint >= 0x80A4 && codePoint <= 0x80A5) || codePoint == 0x80A9 || codePoint == 0x80AF ||
           codePoint == 0x80B2 || codePoint == 0x80C1 || codePoint == 0x80C6 || codePoint == 0x80CC ||
           codePoint == 0x80CE || codePoint == 0x80D6 || codePoint == 0x80DC || codePoint == 0x80DE ||
           codePoint == 0x80E1 || codePoint == 0x80F6 || codePoint == 0x80F8 || codePoint == 0x80FD ||
           codePoint == 0x8106 || codePoint == 0x8111 || codePoint == 0x8131 || codePoint == 0x8138 ||
           codePoint == 0x814A || codePoint == 0x8150 || codePoint == 0x8153 || codePoint == 0x8170 ||
           codePoint == 0x8179 || (codePoint >= 0x817E && codePoint <= 0x817F) || codePoint == 0x81C2 ||
           codePoint == 0x81E3 || codePoint == 0x81EA || codePoint == 0x81ED ||
           (codePoint >= 0x81F3 && codePoint <= 0x81F4) || (codePoint >= 0x820C && codePoint <= 0x820D) ||
           codePoint == 0x8212 || (codePoint >= 0x821E && codePoint <= 0x821F) || codePoint == 0x822A ||
           codePoint == 0x822C || codePoint == 0x8230 || codePoint == 0x8239 || codePoint == 0x826F ||
           codePoint == 0x8272 || codePoint == 0x827A || codePoint == 0x827E || codePoint == 0x8282 ||
           codePoint == 0x8292 || codePoint == 0x829D || codePoint == 0x82A6 ||
           (codePoint >= 0x82AC && codePoint <= 0x82AD) || codePoint == 0x82B1 || codePoint == 0x82B3 ||
           codePoint == 0x82CD || codePoint == 0x82CF || codePoint == 0x82D7 ||
           (codePoint >= 0x82E5 && codePoint <= 0x82E6) || codePoint == 0x82F1 ||
           (codePoint >= 0x8302 && codePoint <= 0x8303) || codePoint == 0x8328 || codePoint == 0x832B ||
           codePoint == 0x8336 || codePoint == 0x8349 || codePoint == 0x8350 || codePoint == 0x8352 ||
           codePoint == 0x8363 || codePoint == 0x836F || codePoint == 0x8377 || codePoint == 0x8389 ||
           codePoint == 0x838E || (codePoint >= 0x83AA && codePoint <= 0x83AB) ||
           (codePoint >= 0x83B1 && codePoint <= 0x83B2) || codePoint == 0x83B7 || codePoint == 0x83DC ||
           codePoint == 0x83E9 || codePoint == 0x83F2 || codePoint == 0x8404 || codePoint == 0x840D ||
           (codePoint >= 0x8424 && codePoint <= 0x8425) || (codePoint >= 0x8427 && codePoint <= 0x8428) ||
           codePoint == 0x843D || codePoint == 0x8457 || codePoint == 0x845B || codePoint == 0x8461 ||
           codePoint == 0x8482 || codePoint == 0x848B || codePoint == 0x8499 || codePoint == 0x84C9 ||
           codePoint == 0x84DD || codePoint == 0x84EC || codePoint == 0x8511 || codePoint == 0x8521 ||
           codePoint == 0x8584 || codePoint == 0x85AA || codePoint == 0x85C9 || codePoint == 0x85CF ||
           codePoint == 0x85E4 || codePoint == 0x864E || codePoint == 0x8651 || codePoint == 0x866B ||
           codePoint == 0x8679 || (codePoint >= 0x867D && codePoint <= 0x867E) || codePoint == 0x8681 ||
           codePoint == 0x86C7 || codePoint == 0x86CB || codePoint == 0x86D9 || codePoint == 0x86EE ||
           codePoint == 0x8702 || codePoint == 0x871C || codePoint == 0x8776 || codePoint == 0x878D ||
           codePoint == 0x87F9 || codePoint == 0x8822 || codePoint == 0x8840 || codePoint == 0x884C ||
           codePoint == 0x8857 || codePoint == 0x8861 || codePoint == 0x8863 || codePoint == 0x8865 ||
           codePoint == 0x8868 || codePoint == 0x888B || codePoint == 0x88AB || codePoint == 0x88AD ||
           (codePoint >= 0x88C1 && codePoint <= 0x88C2) || codePoint == 0x88C5 || codePoint == 0x88D5 ||
           codePoint == 0x88E4 || codePoint == 0x897F || codePoint == 0x8981 || codePoint == 0x8986 ||
           (codePoint >= 0x89C1 && codePoint <= 0x89C2) || codePoint == 0x89C4 || codePoint == 0x89C6 ||
           (codePoint >= 0x89C8 && codePoint <= 0x89C9) || codePoint == 0x89D2 || codePoint == 0x89E3 ||
           codePoint == 0x8A00 || codePoint == 0x8A89 || codePoint == 0x8A93 || codePoint == 0x8B66 ||
           (codePoint >= 0x8BA1 && codePoint <= 0x8BA2) || codePoint == 0x8BA4 ||
           (codePoint >= 0x8BA8 && codePoint <= 0x8BA9) || (codePoint >= 0x8BAD && codePoint <= 0x8BB0) ||
           codePoint == 0x8BB2 || (codePoint >= 0x8BB7 && codePoint <= 0x8BB8) || codePoint == 0x8BBA ||
           (codePoint >= 0x8BBE && codePoint <= 0x8BBF) || codePoint == 0x8BC1 || codePoint == 0x8BC4 ||
           codePoint == 0x8BC6 || codePoint == 0x8BC9 || codePoint == 0x8BCD || codePoint == 0x8BD1 ||
           codePoint == 0x8BD5 || codePoint == 0x8BD7 || codePoint == 0x8BDA ||
           (codePoint >= 0x8BDD && codePoint <= 0x8BDE) || codePoint == 0x8BE2 ||
           (codePoint >= 0x8BE5 && codePoint <= 0x8BE6) || codePoint == 0x8BED || codePoint == 0x8BEF ||
           codePoint == 0x8BF4 || (codePoint >= 0x8BF7 && codePoint <= 0x8BF8) ||
           (codePoint >= 0x8BFA && codePoint <= 0x8BFB) || codePoint == 0x8BFE || codePoint == 0x8C01 ||
           codePoint == 0x8C03 || codePoint == 0x8C05 || codePoint == 0x8C08 ||
           (codePoint >= 0x8C0A && codePoint <= 0x8C0B) || codePoint == 0x8C13 || codePoint == 0x8C1C ||
           codePoint == 0x8C22 || codePoint == 0x8C28 || codePoint == 0x8C31 || codePoint == 0x8C37 ||
           codePoint == 0x8C46 || codePoint == 0x8C61 || codePoint == 0x8C6A || codePoint == 0x8C8C ||
           (codePoint >= 0x8D1D && codePoint <= 0x8D1F) || (codePoint >= 0x8D21 && codePoint <= 0x8D25) ||
           (codePoint >= 0x8D27 && codePoint <= 0x8D2A) || codePoint == 0x8D2D || codePoint == 0x8D2F ||
           codePoint == 0x8D31 || (codePoint >= 0x8D34 && codePoint <= 0x8D35) ||
           (codePoint >= 0x8D38 && codePoint <= 0x8D3A) || codePoint == 0x8D3C || codePoint == 0x8D3E ||
           codePoint == 0x8D44 || (codePoint >= 0x8D4B && codePoint <= 0x8D4C) ||
           (codePoint >= 0x8D4F && codePoint <= 0x8D50) || codePoint == 0x8D54 || codePoint == 0x8D56 ||
           (codePoint >= 0x8D5A && codePoint <= 0x8D5B) || codePoint == 0x8D5E || codePoint == 0x8D60 ||
           codePoint == 0x8D62 || codePoint == 0x8D64 || codePoint == 0x8D6B || codePoint == 0x8D70 ||
           codePoint == 0x8D75 || codePoint == 0x8D77 || codePoint == 0x8D81 || codePoint == 0x8D85 ||
           (codePoint >= 0x8D8A && codePoint <= 0x8D8B) || codePoint == 0x8DA3 || codePoint == 0x8DB3 ||
           codePoint == 0x8DC3 || codePoint == 0x8DCC || codePoint == 0x8DD1 || codePoint == 0x8DDD ||
           codePoint == 0x8DDF || codePoint == 0x8DEF || codePoint == 0x8DF3 || codePoint == 0x8E0F ||
           codePoint == 0x8E22 || codePoint == 0x8E29 || codePoint == 0x8EAB || codePoint == 0x8EB2 ||
           codePoint == 0x8F66 || (codePoint >= 0x8F68 && codePoint <= 0x8F69) || codePoint == 0x8F6C ||
           (codePoint >= 0x8F6E && codePoint <= 0x8F70) || codePoint == 0x8F7B || codePoint == 0x8F7D ||
           codePoint == 0x8F83 || (codePoint >= 0x8F85 && codePoint <= 0x8F86) ||
           (codePoint >= 0x8F88 && codePoint <= 0x8F89) || codePoint == 0x8F91 || codePoint == 0x8F93 ||
           codePoint == 0x8F9B || codePoint == 0x8F9E || (codePoint >= 0x8FA8 && codePoint <= 0x8FA9) ||
           (codePoint >= 0x8FB0 && codePoint <= 0x8FB1) || codePoint == 0x8FB9 || codePoint == 0x8FBE ||
           codePoint == 0x8FC1 || codePoint == 0x8FC5 || (codePoint >= 0x8FC7 && codePoint <= 0x8FC8) ||
           codePoint == 0x8FCE || (codePoint >= 0x8FD0 && codePoint <= 0x8FD1) || codePoint == 0x8FD4 ||
           (codePoint >= 0x8FD8 && codePoint <= 0x8FD9) || (codePoint >= 0x8FDB && codePoint <= 0x8FDF) ||
           codePoint == 0x8FE6 || (codePoint >= 0x8FEA && codePoint <= 0x8FEB) || codePoint == 0x8FF0 ||
           codePoint == 0x8FF7 || codePoint == 0x8FFD || (codePoint >= 0x9000 && codePoint <= 0x9003) ||
           codePoint == 0x9006 || (codePoint >= 0x9009 && codePoint <= 0x900A) ||
           (codePoint >= 0x900F && codePoint <= 0x9010) || codePoint == 0x9012 || codePoint == 0x9014 ||
           (codePoint >= 0x901A && codePoint <= 0x901B) || codePoint == 0x901D ||
           (codePoint >= 0x901F && codePoint <= 0x9020) || codePoint == 0x9022 || codePoint == 0x9038 ||
           (codePoint >= 0x903B && codePoint <= 0x903C) || codePoint == 0x9047 || codePoint == 0x904D ||
           codePoint == 0x9053 || codePoint == 0x9057 || (codePoint >= 0x906D && codePoint <= 0x906E) ||
           codePoint == 0x9075 || (codePoint >= 0x907F && codePoint <= 0x9080) || codePoint == 0x9093 ||
           codePoint == 0x90A3 || codePoint == 0x90A6 || codePoint == 0x90AA || codePoint == 0x90AE ||
           codePoint == 0x90B1 || codePoint == 0x90BB || codePoint == 0x90CE || codePoint == 0x90D1 ||
           codePoint == 0x90E8 || codePoint == 0x90ED || codePoint == 0x90FD || codePoint == 0x9102 ||
           codePoint == 0x9149 || codePoint == 0x914B || codePoint == 0x914D || codePoint == 0x9152 ||
           (codePoint >= 0x9177 && codePoint <= 0x9178) || codePoint == 0x9189 || codePoint == 0x9192 ||
           codePoint == 0x91C7 || codePoint == 0x91CA || (codePoint >= 0x91CC && codePoint <= 0x91CF) ||
           codePoint == 0x91D1 || codePoint == 0x9488 || codePoint == 0x9493 || codePoint == 0x949F ||
           codePoint == 0x94A2 || codePoint == 0x94A6 || codePoint == 0x94B1 || codePoint == 0x94BB ||
           codePoint == 0x94C1 || codePoint == 0x94C3 || codePoint == 0x94DC || codePoint == 0x94E2 ||
           codePoint == 0x94ED || codePoint == 0x94F6 || codePoint == 0x94FA || codePoint == 0x94FE ||
           (codePoint >= 0x9500 && codePoint <= 0x9501) || codePoint == 0x9505 || codePoint == 0x950B ||
           codePoint == 0x9519 || codePoint == 0x9521 || codePoint == 0x9526 || codePoint == 0x952E ||
           codePoint == 0x953A || codePoint == 0x9547 || codePoint == 0x955C || codePoint == 0x956D ||
           codePoint == 0x957F || codePoint == 0x95E8 || codePoint == 0x95EA ||
           (codePoint >= 0x95ED && codePoint <= 0x95EE) || codePoint == 0x95F0 || codePoint == 0x95F2 ||
           codePoint == 0x95F4 || codePoint == 0x95F7 || codePoint == 0x95F9 || codePoint == 0x95FB ||
           codePoint == 0x9601 || codePoint == 0x9605 || codePoint == 0x9610 || codePoint == 0x9614 ||
           codePoint == 0x961F || codePoint == 0x962E || (codePoint >= 0x9632 && codePoint <= 0x9636) ||
           codePoint == 0x963B || (codePoint >= 0x963F && codePoint <= 0x9640) ||
           (codePoint >= 0x9644 && codePoint <= 0x9646) || codePoint == 0x9648 || codePoint == 0x964D ||
           codePoint == 0x9650 || codePoint == 0x9662 || codePoint == 0x9664 ||
           (codePoint >= 0x9669 && codePoint <= 0x966A) || (codePoint >= 0x9675 && codePoint <= 0x9677) ||
           codePoint == 0x9686 || (codePoint >= 0x968F && codePoint <= 0x9690) || codePoint == 0x9694 ||
           codePoint == 0x969C || codePoint == 0x96BE || (codePoint >= 0x96C4 && codePoint <= 0x96C6) ||
           codePoint == 0x96C9 || codePoint == 0x96E8 || codePoint == 0x96EA || codePoint == 0x96EF ||
           codePoint == 0x96F3 || (codePoint >= 0x96F6 && codePoint <= 0x96F7) || codePoint == 0x96FE ||
           codePoint == 0x9700 || codePoint == 0x9707 || codePoint == 0x970D || codePoint == 0x9716 ||
           codePoint == 0x9732 || (codePoint >= 0x9738 && codePoint <= 0x9739) || codePoint == 0x9752 ||
           codePoint == 0x9756 || codePoint == 0x9759 || codePoint == 0x975E || codePoint == 0x9760 ||
           codePoint == 0x9762 || codePoint == 0x9769 || codePoint == 0x977C || codePoint == 0x978B ||
           codePoint == 0x9791 || codePoint == 0x97E6 || codePoint == 0x97E9 || codePoint == 0x97F3 ||
           (codePoint >= 0x9875 && codePoint <= 0x9876) || (codePoint >= 0x9879 && codePoint <= 0x987B) ||
           (codePoint >= 0x987D && codePoint <= 0x987F) || codePoint == 0x9884 ||
           (codePoint >= 0x9886 && codePoint <= 0x9887) || codePoint == 0x9891 ||
           (codePoint >= 0x9897 && codePoint <= 0x9898) || codePoint == 0x989D || codePoint == 0x98CE ||
           (codePoint >= 0x98D8 && codePoint <= 0x98D9) || (codePoint >= 0x98DE && codePoint <= 0x98DF) ||
           codePoint == 0x9910 || (codePoint >= 0x996D && codePoint <= 0x996E) ||
           (codePoint >= 0x9970 && codePoint <= 0x9971) || codePoint == 0x997C || codePoint == 0x9986 ||
           codePoint == 0x9996 || codePoint == 0x9999 || codePoint == 0x99A8 || codePoint == 0x9A6C ||
           codePoint == 0x9A71 || codePoint == 0x9A76 || codePoint == 0x9A7B || codePoint == 0x9A7E ||
           codePoint == 0x9A8C || codePoint == 0x9A91 || codePoint == 0x9A97 || codePoint == 0x9A9A ||
           codePoint == 0x9AA4 || codePoint == 0x9AA8 || codePoint == 0x9AD8 || codePoint == 0x9B3C ||
           codePoint == 0x9B42 || codePoint == 0x9B45 || codePoint == 0x9B54 || codePoint == 0x9C7C ||
           codePoint == 0x9C81 || codePoint == 0x9C9C || codePoint == 0x9E1F || codePoint == 0x9E21 ||
           codePoint == 0x9E23 || codePoint == 0x9E2D || codePoint == 0x9E3F || codePoint == 0x9E45 ||
           codePoint == 0x9E64 || codePoint == 0x9E70 || codePoint == 0x9E7F || codePoint == 0x9EA6 ||
           codePoint == 0x9EBB || codePoint == 0x9EC4 || codePoint == 0x9ECE || codePoint == 0x9ED1 ||
           codePoint == 0x9ED8 || codePoint == 0x9F13 || codePoint == 0x9F20 || codePoint == 0x9F3B ||
           codePoint == 0x9F50 || codePoint == 0x9F7F || codePoint == 0x9F84 || codePoint == 0x9F99 ||
           codePoint == 0x9F9F || (codePoint >= 0xFE30 && codePoint <= 0xFE31) ||
           (codePoint >= 0xFE33 && codePoint <= 0xFE44) || (codePoint >= 0xFE49 && codePoint <= 0xFE52) ||
           (codePoint >= 0xFE54 && codePoint <= 0xFE57) || (codePoint >= 0xFE59 && codePoint <= 0xFE61) ||
           codePoint == 0xFE63 || codePoint == 0xFE68 || (codePoint >= 0xFE6A && codePoint <= 0xFE6B) ||
           (codePoint >= 0xFF01 && codePoint <= 0xFF03) || (codePoint >= 0xFF05 && codePoint <= 0xFF0A) ||
           (codePoint >= 0xFF0C && codePoint <= 0xFF0F) || (codePoint >= 0xFF1A && codePoint <= 0xFF1B) ||
           (codePoint >= 0xFF1F && codePoint <= 0xFF20) || (codePoint >= 0xFF3B && codePoint <= 0xFF3D) ||
           codePoint == 0xFF3F || codePoint == 0xFF5B || codePoint == 0xFF5D;
}

//---------------------------------------------------------------------------------------------------------------------
auto TraditionalChineseWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return (codePoint >= 0x20 && codePoint <= 0x40) || (codePoint >= 0x5B && codePoint <= 0x5F) ||
           (codePoint >= 0x7B && codePoint <= 0x7D) || codePoint == 0xA0 || codePoint == 0xA7 || codePoint == 0xA9 ||
           codePoint == 0xB7 || (codePoint >= 0x2010 && codePoint <= 0x2011) ||
           (codePoint >= 0x2018 && codePoint <= 0x2019) || (codePoint >= 0x2020 && codePoint <= 0x2021) ||
           (codePoint >= 0x2025 && codePoint <= 0x2027) || codePoint == 0x2030 ||
           (codePoint >= 0x2032 && codePoint <= 0x2033) || codePoint == 0x2035 || codePoint == 0x203B ||
           codePoint == 0x203E || (codePoint >= 0x3001 && codePoint <= 0x3003) ||
           (codePoint >= 0x3007 && codePoint <= 0x3011) || (codePoint >= 0x3014 && codePoint <= 0x3015) ||
           (codePoint >= 0x301D && codePoint <= 0x301E) || (codePoint >= 0x4E00 && codePoint <= 0x4E01) ||
           codePoint == 0x4E03 || (codePoint >= 0x4E08 && codePoint <= 0x4E0D) || codePoint == 0x4E11 ||
           codePoint == 0x4E14 || codePoint == 0x4E16 || (codePoint >= 0x4E18 && codePoint <= 0x4E19) ||
           codePoint == 0x4E1F || codePoint == 0x4E26 || codePoint == 0x4E28 || codePoint == 0x4E2D ||
           codePoint == 0x4E32 || codePoint == 0x4E36 || (codePoint >= 0x4E38 && codePoint <= 0x4E39) ||
           codePoint == 0x4E3B || codePoint == 0x4E3F || codePoint == 0x4E43 || codePoint == 0x4E45 ||
           codePoint == 0x4E48 || codePoint == 0x4E4B || (codePoint >= 0x4E4E && codePoint <= 0x4E4F) ||
           codePoint == 0x4E56 || (codePoint >= 0x4E58 && codePoint <= 0x4E59) || codePoint == 0x4E5D ||
           codePoint == 0x4E5F || codePoint == 0x4E7E || codePoint == 0x4E82 ||
           (codePoint >= 0x4E85 && codePoint <= 0x4E86) || codePoint == 0x4E88 ||
           (codePoint >= 0x4E8B && codePoint <= 0x4E8C) || codePoint == 0x4E8E ||
           (codePoint >= 0x4E91 && codePoint <= 0x4E92) || (codePoint >= 0x4E94 && codePoint <= 0x4E95) ||
           codePoint == 0x4E9B || codePoint == 0x4E9E || (codePoint >= 0x4EA0 && codePoint <= 0x4EA1) ||
           (codePoint >= 0x4EA4 && codePoint <= 0x4EA6) || codePoint == 0x4EA8 ||
           (codePoint >= 0x4EAB && codePoint <= 0x4EAC) || codePoint == 0x4EAE || codePoint == 0x4EBA ||
           (codePoint >= 0x4EC0 && codePoint <= 0x4EC1) || codePoint == 0x4EC7 ||
           (codePoint >= 0x4ECA && codePoint <= 0x4ECB) || codePoint == 0x4ECD || codePoint == 0x4ED4 ||
           codePoint == 0x4ED6 || (codePoint >= 0x4ED8 && codePoint <= 0x4ED9) ||
           (codePoint >= 0x4EE3 && codePoint <= 0x4EE5) || codePoint == 0x4EF0 || codePoint == 0x4EF2 ||
           codePoint == 0x4EF6 || codePoint == 0x4EFB || codePoint == 0x4EFD || codePoint == 0x4F01 ||
           codePoint == 0x4F0A || codePoint == 0x4F0D || (codePoint >= 0x4F10 && codePoint <= 0x4F11) ||
           codePoint == 0x4F19 || (codePoint >= 0x4F2F && codePoint <= 0x4F30) || codePoint == 0x4F34 ||
           codePoint == 0x4F38 || (codePoint >= 0x4F3C && codePoint <= 0x4F3D) || codePoint == 0x4F46 ||
           (codePoint >= 0x4F48 && codePoint <= 0x4F49) || (codePoint >= 0x4F4D && codePoint <= 0x4F4F) ||
           (codePoint >= 0x4F54 && codePoint <= 0x4F55) || codePoint == 0x4F59 ||
           (codePoint >= 0x4F5B && codePoint <= 0x4F5C) || codePoint == 0x4F60 || codePoint == 0x4F69 ||
           codePoint == 0x4F73 || codePoint == 0x4F7F || codePoint == 0x4F86 || codePoint == 0x4F8B ||
           codePoint == 0x4F9B || codePoint == 0x4F9D || codePoint == 0x4FAF || codePoint == 0x4FB5 ||
           codePoint == 0x4FBF || (codePoint >= 0x4FC2 && codePoint <= 0x4FC4) || codePoint == 0x4FCA ||
           codePoint == 0x4FD7 || codePoint == 0x4FDD || (codePoint >= 0x4FE0 && codePoint <= 0x4FE1) ||
           codePoint == 0x4FEE || codePoint == 0x4FF1 || codePoint == 0x4FFE || codePoint == 0x500B ||
           codePoint == 0x500D || (codePoint >= 0x5011 && codePoint <= 0x5012) ||
           (codePoint >= 0x5019 && codePoint <= 0x501A) || codePoint == 0x501F || codePoint == 0x502B ||
           codePoint == 0x503C || codePoint == 0x5047 || codePoint == 0x5049 || codePoint == 0x504F ||
           codePoint == 0x505A || codePoint == 0x505C || codePoint == 0x5065 ||
           (codePoint >= 0x5074 && codePoint <= 0x5077) || codePoint == 0x5091 || codePoint == 0x5099 ||
           (codePoint >= 0x50A2 && codePoint <= 0x50A3) || (codePoint >= 0x50B2 && codePoint <= 0x50B3) ||
           codePoint == 0x50B7 || codePoint == 0x50BB || codePoint == 0x50BE || codePoint == 0x50C5 ||
           codePoint == 0x50CF || codePoint == 0x50D1 || codePoint == 0x50E7 || codePoint == 0x50F9 ||
           codePoint == 0x5100 || codePoint == 0x5104 || codePoint == 0x5112 || codePoint == 0x5118 ||
           codePoint == 0x512A || codePoint == 0x513F || codePoint == 0x5141 ||
           (codePoint >= 0x5143 && codePoint <= 0x5145) || (codePoint >= 0x5147 && codePoint <= 0x5149) ||
           codePoint == 0x514B || codePoint == 0x514D || codePoint == 0x5152 || codePoint == 0x5154 ||
           codePoint == 0x5165 || (codePoint >= 0x5167 && codePoint <= 0x5169) ||
           (codePoint >= 0x516B && codePoint <= 0x516E) || codePoint == 0x5171 ||
           (codePoint >= 0x5175 && codePoint <= 0x5178) || codePoint == 0x517C || codePoint == 0x5182 ||
           codePoint == 0x518A || codePoint == 0x518D || codePoint == 0x5192 || codePoint == 0x5196 ||
           codePoint == 0x51A0 || (codePoint >= 0x51AB && codePoint <= 0x51AC) || codePoint == 0x51B0 ||
           codePoint == 0x51B7 || codePoint == 0x51C6 || codePoint == 0x51CC || codePoint == 0x51DD ||
           (codePoint >= 0x51E0 && codePoint <= 0x51E1) || (codePoint >= 0x51F0 && codePoint <= 0x51F1) ||
           codePoint == 0x51F5 || codePoint == 0x51FA || codePoint == 0x51FD || codePoint == 0x5200 ||
           (codePoint >= 0x5206 && codePoint <= 0x5207) || codePoint == 0x520A || codePoint == 0x5217 ||
           codePoint == 0x521D || (codePoint >= 0x5224 && codePoint <= 0x5225) ||
           (codePoint >= 0x5229 && codePoint <= 0x522A) || codePoint == 0x5230 ||
           (codePoint >= 0x5236 && codePoint <= 0x5237) || (codePoint >= 0x523A && codePoint <= 0x523B) ||
           codePoint == 0x5247 || (codePoint >= 0x524C && codePoint <= 0x524D) || codePoint == 0x525B ||
           (codePoint >= 0x5269 && codePoint <= 0x526A) || codePoint == 0x526F || codePoint == 0x5272 ||
           codePoint == 0x5275 || codePoint == 0x5283 || codePoint == 0x5287 || codePoint == 0x5289 ||
           codePoint == 0x528D || codePoint == 0x529B || (codePoint >= 0x529F && codePoint <= 0x52A0) ||
           (codePoint >= 0x52A9 && codePoint <= 0x52AB) || codePoint == 0x52C1 || codePoint == 0x52C7 ||
           codePoint == 0x52C9 || codePoint == 0x52D2 || codePoint == 0x52D5 || codePoint == 0x52D9 ||
           (codePoint >= 0x52DD && codePoint <= 0x52DE) || codePoint == 0x52E2 || codePoint == 0x52E4 ||
           codePoint == 0x52F5 || (codePoint >= 0x52F8 && codePoint <= 0x52F9) || codePoint == 0x52FF ||
           codePoint == 0x5305 || codePoint == 0x5308 || (codePoint >= 0x5315 && codePoint <= 0x5317) ||
           codePoint == 0x531A || (codePoint >= 0x5338 && codePoint <= 0x5339) ||
           (codePoint >= 0x5340 && codePoint <= 0x5341) || codePoint == 0x5343 ||
           (codePoint >= 0x5347 && codePoint <= 0x5348) || codePoint == 0x534A ||
           (codePoint >= 0x5352 && codePoint <= 0x5354) || codePoint == 0x5357 || codePoint == 0x535A ||
           codePoint == 0x535C || codePoint == 0x5361 || codePoint == 0x5369 ||
           (codePoint >= 0x536F && codePoint <= 0x5371) || codePoint == 0x5373 || codePoint == 0x5377 ||
           codePoint == 0x537B || codePoint == 0x5382 || codePoint == 0x5384 || codePoint == 0x5398 ||
           codePoint == 0x539A || codePoint == 0x539F || codePoint == 0x53AD || codePoint == 0x53B2 ||
           codePoint == 0x53B6 || codePoint == 0x53BB || codePoint == 0x53C3 || codePoint == 0x53C8 ||
           (codePoint >= 0x53CA && codePoint <= 0x53CB) || codePoint == 0x53CD || codePoint == 0x53D4 ||
           (codePoint >= 0x53D6 && codePoint <= 0x53D7) || (codePoint >= 0x53E3 && codePoint <= 0x53E6) ||
           (codePoint >= 0x53EA && codePoint <= 0x53ED) || (codePoint >= 0x53EF && codePoint <= 0x53F0) ||
           (codePoint >= 0x53F2 && codePoint <= 0x53F3) || codePoint == 0x53F8 ||
           (codePoint >= 0x5403 && codePoint <= 0x5404) || (codePoint >= 0x5408 && codePoint <= 0x540A) ||
           (codePoint >= 0x540C && codePoint <= 0x540E) || (codePoint >= 0x5410 && codePoint <= 0x5412) ||
           codePoint == 0x541B || (codePoint >= 0x541D && codePoint <= 0x5420) ||
           (codePoint >= 0x5426 && codePoint <= 0x5427) || codePoint == 0x542B || codePoint == 0x5433 ||
           codePoint == 0x5435 || (codePoint >= 0x5438 && codePoint <= 0x5439) || codePoint == 0x543E ||
           codePoint == 0x5440 || codePoint == 0x5442 || codePoint == 0x5446 || codePoint == 0x544A ||
           codePoint == 0x5462 || codePoint == 0x5468 || codePoint == 0x5473 || codePoint == 0x5475 ||
           (codePoint >= 0x547C && codePoint <= 0x547D) || codePoint == 0x548C || codePoint == 0x5496 ||
           (codePoint >= 0x54A6 && codePoint <= 0x54A7) || codePoint == 0x54AA || codePoint == 0x54AC ||
           codePoint == 0x54B1 || (codePoint >= 0x54C0 && codePoint <= 0x54C1) ||
           (codePoint >= 0x54C7 && codePoint <= 0x54C9) || codePoint == 0x54CE || codePoint == 0x54E1 ||
           (codePoint >= 0x54E5 && codePoint <= 0x54E6) || (codePoint >= 0x54E9 && codePoint <= 0x54EA) ||
           codePoint == 0x54ED || codePoint == 0x54F2 || codePoint == 0x5509 || codePoint == 0x5510 ||
           codePoint == 0x5514 || codePoint == 0x552C || (codePoint >= 0x552E && codePoint <= 0x552F) ||
           codePoint == 0x5531 || (codePoint >= 0x5537 && codePoint <= 0x5538) || codePoint == 0x5546 ||
           codePoint == 0x554A || codePoint == 0x554F || codePoint == 0x555F || codePoint == 0x5561 ||
           (codePoint >= 0x5565 && codePoint <= 0x5566) || codePoint == 0x556A || codePoint == 0x5580 ||
           codePoint == 0x5582 || codePoint == 0x5584 || codePoint == 0x5587 || codePoint == 0x558A ||
           codePoint == 0x5594 || (codePoint >= 0x559C && codePoint <= 0x559D) || codePoint == 0x55AC ||
           codePoint == 0x55AE || codePoint == 0x55B5 || codePoint == 0x55CE || codePoint == 0x55DA ||
           codePoint == 0x55E8 || codePoint == 0x55EF || codePoint == 0x5606 || codePoint == 0x5609 ||
           codePoint == 0x5617 || codePoint == 0x561B || codePoint == 0x5634 || codePoint == 0x563B ||
           codePoint == 0x563F || codePoint == 0x5668 || codePoint == 0x5674 || codePoint == 0x5687 ||
           codePoint == 0x56B4 || codePoint == 0x56C9 || codePoint == 0x56D7 || codePoint == 0x56DB ||
           codePoint == 0x56DE || codePoint == 0x56E0 || codePoint == 0x56F0 || codePoint == 0x56FA ||
           codePoint == 0x5708 || codePoint == 0x570B || codePoint == 0x570D ||
           (codePoint >= 0x5712 && codePoint <= 0x5713) || codePoint == 0x5716 || codePoint == 0x5718 ||
           codePoint == 0x571C || codePoint == 0x571F || codePoint == 0x5728 || codePoint == 0x572D ||
           codePoint == 0x5730 || codePoint == 0x573E || codePoint == 0x5740 || codePoint == 0x5747 ||
           codePoint == 0x574E || codePoint == 0x5750 || codePoint == 0x5761 || codePoint == 0x5764 ||
           codePoint == 0x5766 || codePoint == 0x576A || (codePoint >= 0x5782 && codePoint <= 0x5783) ||
           codePoint == 0x578B || codePoint == 0x57C3 || codePoint == 0x57CE || codePoint == 0x57D4 ||
           codePoint == 0x57DF || codePoint == 0x57F7 || (codePoint >= 0x57F9 && codePoint <= 0x57FA) ||
           codePoint == 0x5802 || (codePoint >= 0x5805 && codePoint <= 0x5806) || codePoint == 0x5821 ||
           codePoint == 0x582A || codePoint == 0x5831 || codePoint == 0x5834 || codePoint == 0x584A ||
           codePoint == 0x5854 || codePoint == 0x5857 || codePoint == 0x585E || codePoint == 0x586B ||
           codePoint == 0x5875 || codePoint == 0x5883 || codePoint == 0x589E || codePoint == 0x58A8 ||
           codePoint == 0x58AE || codePoint == 0x58C1 || codePoint == 0x58C7 || codePoint == 0x58D3 ||
           codePoint == 0x58D8 || codePoint == 0x58DE || codePoint == 0x58E2 ||
           (codePoint >= 0x58EB && codePoint <= 0x58EC) || codePoint == 0x58EF || codePoint == 0x58FD ||
           codePoint == 0x5902 || codePoint == 0x590A || codePoint == 0x590F ||
           (codePoint >= 0x5915 && codePoint <= 0x5916) || codePoint == 0x591A || codePoint == 0x591C ||
           codePoint == 0x5920 || codePoint == 0x5922 || codePoint == 0x5925 || codePoint == 0x5927 ||
           (codePoint >= 0x5929 && codePoint <= 0x592B) || codePoint == 0x592E || codePoint == 0x5931 ||
           (codePoint >= 0x5937 && codePoint <= 0x5938) || codePoint == 0x593E ||
           (codePoint >= 0x5947 && codePoint <= 0x5949) || (codePoint >= 0x594E && codePoint <= 0x594F) ||
           codePoint == 0x5951 || codePoint == 0x5954 || codePoint == 0x5957 || codePoint == 0x5967 ||
           codePoint == 0x596A || codePoint == 0x596E || (codePoint >= 0x5973 && codePoint <= 0x5974) ||
           codePoint == 0x5976 || codePoint == 0x5979 || codePoint == 0x597D || codePoint == 0x5982 ||
           codePoint == 0x5999 || codePoint == 0x599D || codePoint == 0x59A5 || codePoint == 0x59A8 ||
           codePoint == 0x59AE || codePoint == 0x59B3 || codePoint == 0x59B9 || codePoint == 0x59BB ||
           codePoint == 0x59C6 || (codePoint >= 0x59CA && codePoint <= 0x59CB) ||
           (codePoint >= 0x59D0 && codePoint <= 0x59D1) || (codePoint >= 0x59D3 && codePoint <= 0x59D4) ||
           codePoint == 0x59FF || codePoint == 0x5A01 || codePoint == 0x5A03 || codePoint == 0x5A18 ||
           codePoint == 0x5A1B || codePoint == 0x5A41 || codePoint == 0x5A46 || codePoint == 0x5A5A ||
           codePoint == 0x5A66 || codePoint == 0x5A92 || codePoint == 0x5ABD || codePoint == 0x5ACC ||
           codePoint == 0x5AE9 || codePoint == 0x5B50 || codePoint == 0x5B54 ||
           (codePoint >= 0x5B57 && codePoint <= 0x5B58) || codePoint == 0x5B5D || codePoint == 0x5B5F ||
           (codePoint >= 0x5B63 && codePoint <= 0x5B64) || codePoint == 0x5B69 || codePoint == 0x5B6B ||
           codePoint == 0x5B78 || codePoint == 0x5B80 || codePoint == 0x5B83 || codePoint == 0x5B85 ||
           (codePoint >= 0x5B87 && codePoint <= 0x5B89) || (codePoint >= 0x5B8B && codePoint <= 0x5B8C) ||
           codePoint == 0x5B8F || (codePoint >= 0x5B97 && codePoint <= 0x5B9C) ||
           (codePoint >= 0x5BA2 && codePoint <= 0x5BA4) || codePoint == 0x5BAE || codePoint == 0x5BB3 ||
           codePoint == 0x5BB6 || codePoint == 0x5BB9 || codePoint == 0x5BBF || codePoint == 0x5BC2 ||
           (codePoint >= 0x5BC4 && codePoint <= 0x5BC6) || codePoint == 0x5BCC || codePoint == 0x5BD2 ||
           (codePoint >= 0x5BDE && codePoint <= 0x5BDF) || codePoint == 0x5BE2 ||
           (codePoint >= 0x5BE6 && codePoint <= 0x5BE9) || (codePoint >= 0x5BEB && codePoint <= 0x5BEC) ||
           codePoint == 0x5BEE || (codePoint >= 0x5BF5 && codePoint <= 0x5BF6) || codePoint == 0x5BF8 ||
           codePoint == 0x5C01 || codePoint == 0x5C04 || (codePoint >= 0x5C07 && codePoint <= 0x5C08) ||
           (codePoint >= 0x5C0A && codePoint <= 0x5C0B) || (codePoint >= 0x5C0D && codePoint <= 0x5C0F) ||
           codePoint == 0x5C11 || codePoint == 0x5C16 || codePoint == 0x5C1A || codePoint == 0x5C22 ||
           codePoint == 0x5C24 || codePoint == 0x5C31 || codePoint == 0x5C38 || codePoint == 0x5C3A ||
           codePoint == 0x5C3C || codePoint == 0x5C3E || (codePoint >= 0x5C40 && codePoint <= 0x5C41) ||
           (codePoint >= 0x5C45 && codePoint <= 0x5C46) || codePoint == 0x5C4B || codePoint == 0x5C4F ||
           codePoint == 0x5C55 || codePoint == 0x5C60 || codePoint == 0x5C64 || codePoint == 0x5C6C ||
           codePoint == 0x5C6E || codePoint == 0x5C71 || codePoint == 0x5CA1 || codePoint == 0x5CA9 ||
           codePoint == 0x5CB8 || codePoint == 0x5CF0 || codePoint == 0x5CF6 || codePoint == 0x5CFD ||
           codePoint == 0x5D07 || codePoint == 0x5D19 || codePoint == 0x5D34 || codePoint == 0x5D50 ||
           codePoint == 0x5DBA || codePoint == 0x5DDB || (codePoint >= 0x5DDD && codePoint <= 0x5DDE) ||
           codePoint == 0x5DE1 || (codePoint >= 0x5DE5 && codePoint <= 0x5DE8) || codePoint == 0x5DEB ||
           codePoint == 0x5DEE || (codePoint >= 0x5DF1 && codePoint <= 0x5DF4) || codePoint == 0x5DF7 ||
           codePoint == 0x5DFE || (codePoint >= 0x5E02 && codePoint <= 0x5E03) || codePoint == 0x5E0C ||
           (codePoint >= 0x5E15 && codePoint <= 0x5E16) || codePoint == 0x5E1B || codePoint == 0x5E1D ||
           codePoint == 0x5E25 || codePoint == 0x5E2B || codePoint == 0x5E2D || codePoint == 0x5E33 ||
           codePoint == 0x5E36 || codePoint == 0x5E38 || codePoint == 0x5E3D || codePoint == 0x5E45 ||
           codePoint == 0x5E55 || codePoint == 0x5E63 || codePoint == 0x5E6B ||
           (codePoint >= 0x5E72 && codePoint <= 0x5E74) || (codePoint >= 0x5E78 && codePoint <= 0x5E7F) ||
           codePoint == 0x5E87 || codePoint == 0x5E8A || codePoint == 0x5E8F || codePoint == 0x5E95 ||
           codePoint == 0x5E97 || codePoint == 0x5E9A || codePoint == 0x5E9C ||
           (codePoint >= 0x5EA6 && codePoint <= 0x5EA7) || codePoint == 0x5EAB || codePoint == 0x5EAD ||
           (codePoint >= 0x5EB7 && codePoint <= 0x5EB8) || codePoint == 0x5EC9 || codePoint == 0x5ED6 ||
           codePoint == 0x5EE0 || (codePoint >= 0x5EE2 && codePoint <= 0x5EE3) ||
           (codePoint >= 0x5EF3 && codePoint <= 0x5EF4) || (codePoint >= 0x5EF6 && codePoint <= 0x5EF7) ||
           codePoint == 0x5EFA || codePoint == 0x5EFE || codePoint == 0x5F04 || codePoint == 0x5F0B ||
           codePoint == 0x5F0F || codePoint == 0x5F13 || codePoint == 0x5F15 ||
           (codePoint >= 0x5F17 && codePoint <= 0x5F18) || codePoint == 0x5F1F || codePoint == 0x5F26 ||
           codePoint == 0x5F31 || codePoint == 0x5F35 || codePoint == 0x5F37 || codePoint == 0x5F48 ||
           codePoint == 0x5F4A || codePoint == 0x5F4C || codePoint == 0x5F4E || codePoint == 0x5F50 ||
           (codePoint >= 0x5F5D && codePoint <= 0x5F5E) || (codePoint >= 0x5F61 && codePoint <= 0x5F62) ||
           codePoint == 0x5F65 || codePoint == 0x5F69 || (codePoint >= 0x5F6C && codePoint <= 0x5F6D) ||
           (codePoint >= 0x5F70 && codePoint <= 0x5F71) || codePoint == 0x5F73 || codePoint == 0x5F79 ||
           codePoint == 0x5F7C || (codePoint >= 0x5F80 && codePoint <= 0x5F81) || codePoint == 0x5F85 ||
           codePoint == 0x5F88 || (codePoint >= 0x5F8B && codePoint <= 0x5F8C) ||
           (codePoint >= 0x5F90 && codePoint <= 0x5F92) || codePoint == 0x5F97 || codePoint == 0x5F9E ||
           codePoint == 0x5FA9 || codePoint == 0x5FAE || codePoint == 0x5FB5 || codePoint == 0x5FB7 ||
           codePoint == 0x5FB9 || codePoint == 0x5FC3 || codePoint == 0x5FC5 ||
           (codePoint >= 0x5FCC && codePoint <= 0x5FCD) || (codePoint >= 0x5FD7 && codePoint <= 0x5FD9) ||
           codePoint == 0x5FE0 || codePoint == 0x5FEB || codePoint == 0x5FF5 || codePoint == 0x5FFD ||
           codePoint == 0x600E || codePoint == 0x6012 || (codePoint >= 0x6015 && codePoint <= 0x6016) ||
           codePoint == 0x601D || codePoint == 0x6021 || codePoint == 0x6025 ||
           (codePoint >= 0x6027 && codePoint <= 0x6028) || codePoint == 0x602A || codePoint == 0x6046 ||
           codePoint == 0x6050 || codePoint == 0x6062 || codePoint == 0x6065 ||
           (codePoint >= 0x6068 && codePoint <= 0x6069) || codePoint == 0x606D ||
           (codePoint >= 0x606F && codePoint <= 0x6070) || codePoint == 0x6085 || codePoint == 0x6089 ||
           codePoint == 0x6094 || (codePoint >= 0x609F && codePoint <= 0x60A0) || codePoint == 0x60A8 ||
           codePoint == 0x60B2 || codePoint == 0x60B6 || codePoint == 0x60C5 || codePoint == 0x60D1 ||
           codePoint == 0x60DC || (codePoint >= 0x60E0 && codePoint <= 0x60E1) || codePoint == 0x60F1 ||
           codePoint == 0x60F3 || codePoint == 0x60F9 || codePoint == 0x6101 ||
           (codePoint >= 0x6108 && codePoint <= 0x6109) || codePoint == 0x610F ||
           (codePoint >= 0x611A && codePoint <= 0x611B) || codePoint == 0x611F || codePoint == 0x6148 ||
           codePoint == 0x614B || codePoint == 0x6155 || codePoint == 0x6158 ||
           (codePoint >= 0x6162 && codePoint <= 0x6163) || codePoint == 0x6167 || codePoint == 0x616E ||
           codePoint == 0x6170 || codePoint == 0x6176 || codePoint == 0x617E || codePoint == 0x6182 ||
           (codePoint >= 0x6190 && codePoint <= 0x6191) || codePoint == 0x61B2 || codePoint == 0x61B6 ||
           codePoint == 0x61BE || codePoint == 0x61C2 || codePoint == 0x61C9 ||
           (codePoint >= 0x61F6 && codePoint <= 0x61F7) || codePoint == 0x61FC || codePoint == 0x6200 ||
           codePoint == 0x6208 || codePoint == 0x620A || codePoint == 0x620C ||
           (codePoint >= 0x6210 && codePoint <= 0x6212) || codePoint == 0x6216 || codePoint == 0x622A ||
           codePoint == 0x6230 || codePoint == 0x6232 || codePoint == 0x6234 || codePoint == 0x6236 ||
           (codePoint >= 0x623F && codePoint <= 0x6241) || codePoint == 0x6247 || codePoint == 0x624B ||
           (codePoint >= 0x624D && codePoint <= 0x624E) || codePoint == 0x6253 || codePoint == 0x6258 ||
           codePoint == 0x6263 || codePoint == 0x6265 || codePoint == 0x626D || codePoint == 0x626F ||
           codePoint == 0x6279 || (codePoint >= 0x627E && codePoint <= 0x6280) || codePoint == 0x6284 ||
           codePoint == 0x628A || codePoint == 0x6293 || codePoint == 0x6295 ||
           (codePoint >= 0x6297 && codePoint <= 0x6298) || (codePoint >= 0x62AB && codePoint <= 0x62AC) ||
           codePoint == 0x62B1 || codePoint == 0x62B5 || codePoint == 0x62B9 || codePoint == 0x62BD ||
           codePoint == 0x62C6 || codePoint == 0x62C9 || codePoint == 0x62CB || codePoint == 0x62CD ||
           codePoint == 0x62CF || codePoint == 0x62D2 || codePoint == 0x62D4 || codePoint == 0x62D6 ||
           (codePoint >= 0x62DB && codePoint <= 0x62DC) || codePoint == 0x62EC || codePoint == 0x62F3 ||
           codePoint == 0x62FC || (codePoint >= 0x62FE && codePoint <= 0x62FF) || codePoint == 0x6301 ||
           codePoint == 0x6307 || codePoint == 0x6309 || codePoint == 0x6311 || codePoint == 0x6316 ||
           codePoint == 0x632A || codePoint == 0x632F || codePoint == 0x633A || codePoint == 0x6350 ||
           codePoint == 0x6355 || codePoint == 0x6368 || codePoint == 0x6372 || codePoint == 0x6377 ||
           codePoint == 0x6383 || (codePoint >= 0x6388 && codePoint <= 0x6389) || codePoint == 0x638C ||
           codePoint == 0x6392 || codePoint == 0x639B || (codePoint >= 0x63A1 && codePoint <= 0x63A2) ||
           codePoint == 0x63A5 || (codePoint >= 0x63A7 && codePoint <= 0x63A8) || codePoint == 0x63AA ||
           (codePoint >= 0x63CF && codePoint <= 0x63D0) || codePoint == 0x63D2 ||
           (codePoint >= 0x63DA && codePoint <= 0x63DB) || codePoint == 0x63E1 || codePoint == 0x63EE ||
           codePoint == 0x63F4 || codePoint == 0x640D || codePoint == 0x6416 || codePoint == 0x641C ||
           codePoint == 0x641E || (codePoint >= 0x642C && codePoint <= 0x642D) || codePoint == 0x6436 ||
           codePoint == 0x6458 || codePoint == 0x6469 || codePoint == 0x6478 || codePoint == 0x6490 ||
           codePoint == 0x6492 || codePoint == 0x649E || codePoint == 0x64A3 || codePoint == 0x64A5 ||
           codePoint == 0x64AD || (codePoint >= 0x64BE && codePoint <= 0x64BF) || codePoint == 0x64C1 ||
           codePoint == 0x64C7 || (codePoint >= 0x64CA && codePoint <= 0x64CB) ||
           (codePoint >= 0x64CD && codePoint <= 0x64CE) || codePoint == 0x64D4 || codePoint == 0x64DA ||
           codePoint == 0x64E0 || codePoint == 0x64E6 || codePoint == 0x64EC || codePoint == 0x64F4 ||
           codePoint == 0x64FA || codePoint == 0x64FE || codePoint == 0x651D || codePoint == 0x652F ||
           codePoint == 0x6534 || codePoint == 0x6536 || codePoint == 0x6539 || codePoint == 0x653B ||
           (codePoint >= 0x653E && codePoint <= 0x653F) || codePoint == 0x6545 || codePoint == 0x6548 ||
           codePoint == 0x654D || codePoint == 0x654F || codePoint == 0x6551 ||
           (codePoint >= 0x6557 && codePoint <= 0x6559) || codePoint == 0x655D ||
           (codePoint >= 0x6562 && codePoint <= 0x6563) || codePoint == 0x6566 || codePoint == 0x656C ||
           (codePoint >= 0x6574 && codePoint <= 0x6575) || codePoint == 0x6578 || codePoint == 0x6587 ||
           codePoint == 0x6590 || codePoint == 0x6597 || codePoint == 0x6599 || codePoint == 0x65A4 ||
           (codePoint >= 0x65AF && codePoint <= 0x65B0) || codePoint == 0x65B7 || codePoint == 0x65B9 ||
           (codePoint >= 0x65BC && codePoint <= 0x65BD) || codePoint == 0x65C1 || codePoint == 0x65C5 ||
           codePoint == 0x65CB || codePoint == 0x65CF || codePoint == 0x65D7 || codePoint == 0x65E0 ||
           codePoint == 0x65E2 || (codePoint >= 0x65E5 && codePoint <= 0x65E6) || codePoint == 0x65E9 ||
           codePoint == 0x65ED || codePoint == 0x65FA || codePoint == 0x6602 ||
           (codePoint >= 0x6606 && codePoint <= 0x6607) || codePoint == 0x660C ||
           (codePoint >= 0x660E && codePoint <= 0x660F) || codePoint == 0x6613 ||
           (codePoint >= 0x661F && codePoint <= 0x6620) || codePoint == 0x6625 || codePoint == 0x6628 ||
           codePoint == 0x662D || codePoint == 0x662F || codePoint == 0x6642 || codePoint == 0x6649 ||
           codePoint == 0x6652 || codePoint == 0x665A || codePoint == 0x6668 ||
           (codePoint >= 0x666E && codePoint <= 0x666F) || codePoint == 0x6674 || codePoint == 0x6676 ||
           codePoint == 0x667A || codePoint == 0x6691 || (codePoint >= 0x6696 && codePoint <= 0x6697) ||
           codePoint == 0x66AB || codePoint == 0x66B4 || codePoint == 0x66C6 || codePoint == 0x66C9 ||
           codePoint == 0x66F0 || codePoint == 0x66F2 || codePoint == 0x66F4 || codePoint == 0x66F8 ||
           codePoint == 0x66FC || (codePoint >= 0x66FE && codePoint <= 0x6700) || codePoint == 0x6703 ||
           (codePoint >= 0x6708 && codePoint <= 0x6709) || codePoint == 0x670B || codePoint == 0x670D ||
           codePoint == 0x6717 || codePoint == 0x671B || codePoint == 0x671D || codePoint == 0x671F ||
           codePoint == 0x6728 || (codePoint >= 0x672A && codePoint <= 0x672D) || codePoint == 0x6731 ||
           codePoint == 0x6735 || codePoint == 0x6749 || codePoint == 0x674E ||
           (codePoint >= 0x6750 && codePoint <= 0x6751) || codePoint == 0x675C || codePoint == 0x675F ||
           (codePoint >= 0x676F && codePoint <= 0x6771) || (codePoint >= 0x677E && codePoint <= 0x677F) ||
           codePoint == 0x6790 || codePoint == 0x6797 || (codePoint >= 0x679C && codePoint <= 0x679D) ||
           codePoint == 0x67B6 || (codePoint >= 0x67CF && codePoint <= 0x67D0) ||
           (codePoint >= 0x67D3 && codePoint <= 0x67D4) || codePoint == 0x67E5 || codePoint == 0x67EC ||
           codePoint == 0x67EF || (codePoint >= 0x67F3 && codePoint <= 0x67F4) || codePoint == 0x6821 ||
           (codePoint >= 0x6838 && codePoint <= 0x6839) || codePoint == 0x683C || codePoint == 0x6843 ||
           codePoint == 0x6848 || codePoint == 0x684C || codePoint == 0x6851 || codePoint == 0x6881 ||
           codePoint == 0x6885 || codePoint == 0x689D || codePoint == 0x68A8 ||
           (codePoint >= 0x68AF && codePoint <= 0x68B0) || codePoint == 0x68B5 || codePoint == 0x68C4 ||
           codePoint == 0x68C9 || codePoint == 0x68CB || codePoint == 0x68D2 || codePoint == 0x68DA ||
           codePoint == 0x68EE || codePoint == 0x6905 || codePoint == 0x690D || codePoint == 0x6930 ||
           codePoint == 0x694A || codePoint == 0x6953 || codePoint == 0x695A || codePoint == 0x696D ||
           codePoint == 0x6975 || codePoint == 0x6982 || codePoint == 0x699C || codePoint == 0x69AE ||
           codePoint == 0x69CB || codePoint == 0x69CD || codePoint == 0x6A02 || codePoint == 0x6A13 ||
           codePoint == 0x6A19 || codePoint == 0x6A1E || codePoint == 0x6A21 || codePoint == 0x6A23 ||
           codePoint == 0x6A39 || codePoint == 0x6A4B || codePoint == 0x6A5F || codePoint == 0x6A6B ||
           codePoint == 0x6A80 || codePoint == 0x6A94 || codePoint == 0x6AA2 || codePoint == 0x6B04 ||
           codePoint == 0x6B0A || (codePoint >= 0x6B20 && codePoint <= 0x6B21) || codePoint == 0x6B23 ||
           codePoint == 0x6B32 || codePoint == 0x6B3A || (codePoint >= 0x6B3D && codePoint <= 0x6B3E) ||
           codePoint == 0x6B49 || codePoint == 0x6B4C || codePoint == 0x6B50 ||
           (codePoint >= 0x6B61 && codePoint <= 0x6B66) || codePoint == 0x6B72 ||
           (codePoint >= 0x6B77 && codePoint <= 0x6B79) || codePoint == 0x6B7B || codePoint == 0x6B8A ||
           codePoint == 0x6B98 || codePoint == 0x6BB3 || codePoint == 0x6BB5 || codePoint == 0x6BBA ||
           codePoint == 0x6BBC || codePoint == 0x6BC0 || codePoint == 0x6BC5 || codePoint == 0x6BCB ||
           codePoint == 0x6BCD || codePoint == 0x6BCF || codePoint == 0x6BD2 || codePoint == 0x6BD4 ||
           codePoint == 0x6BDB || codePoint == 0x6BEB || codePoint == 0x6C0F || codePoint == 0x6C11 ||
           codePoint == 0x6C14 || codePoint == 0x6C23 || codePoint == 0x6C34 || codePoint == 0x6C38 ||
           codePoint == 0x6C42 || codePoint == 0x6C57 || codePoint == 0x6C5D ||
           (codePoint >= 0x6C5F && codePoint <= 0x6C61) || codePoint == 0x6C6A || codePoint == 0x6C76 ||
           codePoint == 0x6C7A || codePoint == 0x6C7D || codePoint == 0x6C83 ||
           (codePoint >= 0x6C88 && codePoint <= 0x6C89) || codePoint == 0x6C92 || codePoint == 0x6C96 ||
           codePoint == 0x6C99 || codePoint == 0x6CB3 || codePoint == 0x6CB9 || codePoint == 0x6CBB ||
           codePoint == 0x6CBF || codePoint == 0x6CC1 || (codePoint >= 0x6CC9 && codePoint <= 0x6CCA) ||
           codePoint == 0x6CD5 || (codePoint >= 0x6CE1 && codePoint <= 0x6CE2) || codePoint == 0x6CE5 ||
           codePoint == 0x6CE8 || codePoint == 0x6CF0 || codePoint == 0x6CF3 || codePoint == 0x6D0B ||
           codePoint == 0x6D17 || codePoint == 0x6D1B || codePoint == 0x6D1E ||
           (codePoint >= 0x6D29 && codePoint <= 0x6D2A) || codePoint == 0x6D32 || codePoint == 0x6D3B ||
           (codePoint >= 0x6D3D && codePoint <= 0x6D3E) || codePoint == 0x6D41 || codePoint == 0x6D66 ||
           (codePoint >= 0x6D69 && codePoint <= 0x6D6A) || codePoint == 0x6D6E || codePoint == 0x6D77 ||
           (codePoint >= 0x6D87 && codePoint <= 0x6D89) || codePoint == 0x6DAF || codePoint == 0x6DB2 ||
           codePoint == 0x6DB5 || codePoint == 0x6DBC || codePoint == 0x6DD1 || codePoint == 0x6DDA ||
           codePoint == 0x6DE1 || codePoint == 0x6DE8 || codePoint == 0x6DF1 || codePoint == 0x6DF7 ||
           codePoint == 0x6DFA || codePoint == 0x6E05 || codePoint == 0x6E1B || codePoint == 0x6E21 ||
           codePoint == 0x6E2C || codePoint == 0x6E2F || codePoint == 0x6E38 || codePoint == 0x6E56 ||
           codePoint == 0x6E6F || codePoint == 0x6E90 || codePoint == 0x6E96 || codePoint == 0x6E9D ||
           (codePoint >= 0x6EAA && codePoint <= 0x6EAB) || (codePoint >= 0x6EC4 && codePoint <= 0x6EC5) ||
           codePoint == 0x6ECB || codePoint == 0x6ED1 || codePoint == 0x6EF4 ||
           (codePoint >= 0x6EFE && codePoint <= 0x6EFF) || codePoint == 0x6F02 || codePoint == 0x6F0F ||
           codePoint == 0x6F14 || codePoint == 0x6F20 || codePoint == 0x6F22 || codePoint == 0x6F2B ||
           codePoint == 0x6F32 || codePoint == 0x6F38 || codePoint == 0x6F54 || codePoint == 0x6F58 ||
           codePoint == 0x6F5B || codePoint == 0x6F6E || codePoint == 0x6FA4 || codePoint == 0x6FB3 ||
           codePoint == 0x6FC0 || codePoint == 0x6FC3 || codePoint == 0x6FDF || codePoint == 0x6FE4 ||
           codePoint == 0x6FEB || codePoint == 0x6FF1 || codePoint == 0x700F || codePoint == 0x704C ||
           codePoint == 0x7063 || codePoint == 0x706B || codePoint == 0x7070 || codePoint == 0x707D ||
           codePoint == 0x708E || codePoint == 0x70AE || codePoint == 0x70B8 || codePoint == 0x70BA ||
           codePoint == 0x70C8 || codePoint == 0x70CF || codePoint == 0x70E4 || codePoint == 0x7121 ||
           codePoint == 0x7126 || codePoint == 0x7136 || codePoint == 0x7159 || codePoint == 0x715E ||
           codePoint == 0x7167 || codePoint == 0x7169 || codePoint == 0x718A || codePoint == 0x719F ||
           codePoint == 0x71B1 || codePoint == 0x71C3 || codePoint == 0x71C8 || codePoint == 0x71D2 ||
           codePoint == 0x71DF || codePoint == 0x7206 || codePoint == 0x7210 || codePoint == 0x721B ||
           codePoint == 0x722A || (codePoint >= 0x722C && codePoint <= 0x722D) ||
           (codePoint >= 0x7235 && codePoint <= 0x7236) || codePoint == 0x7238 ||
           (codePoint >= 0x723A && codePoint <= 0x723B) || (codePoint >= 0x723D && codePoint <= 0x723F) ||
           (codePoint >= 0x7246 && codePoint <= 0x7248) || codePoint == 0x724C || codePoint == 0x7259 ||
           codePoint == 0x725B || codePoint == 0x7260 || codePoint == 0x7267 || codePoint == 0x7269 ||
           codePoint == 0x7272 || codePoint == 0x7279 || codePoint == 0x727D || codePoint == 0x72A7 ||
           codePoint == 0x72AC || codePoint == 0x72AF || codePoint == 0x72C0 || codePoint == 0x72C2 ||
           codePoint == 0x72D0 || codePoint == 0x72D7 || codePoint == 0x72E0 || codePoint == 0x72FC ||
           (codePoint >= 0x731B && codePoint <= 0x731C) || codePoint == 0x7334 || codePoint == 0x7336 ||
           (codePoint >= 0x7344 && codePoint <= 0x7345) || codePoint == 0x734E || codePoint == 0x7368 ||
           codePoint == 0x7372 || codePoint == 0x7378 || codePoint == 0x737B || codePoint == 0x7384 ||
           codePoint == 0x7387 || codePoint == 0x7389 || codePoint == 0x738B || codePoint == 0x73A9 ||
           codePoint == 0x73AB || codePoint == 0x73B2 || codePoint == 0x73BB || codePoint == 0x73CA ||
           codePoint == 0x73CD || codePoint == 0x73E0 || codePoint == 0x73E5 || codePoint == 0x73ED ||
           codePoint == 0x73FE || codePoint == 0x7403 || codePoint == 0x7406 || codePoint == 0x7409 ||
           codePoint == 0x742A || codePoint == 0x7434 || codePoint == 0x7459 || codePoint == 0x745C ||
           (codePoint >= 0x745E && codePoint <= 0x745F) || codePoint == 0x7464 || codePoint == 0x746A ||
           codePoint == 0x7470 || codePoint == 0x74B0 || codePoint == 0x74DC || codePoint == 0x74E6 ||
           codePoint == 0x74F6 || codePoint == 0x7518 || codePoint == 0x751A || codePoint == 0x751C ||
           codePoint == 0x751F || codePoint == 0x7522 || codePoint == 0x7528 ||
           (codePoint >= 0x7530 && codePoint <= 0x7533) || (codePoint >= 0x7537 && codePoint <= 0x7538) ||
           codePoint == 0x754C || codePoint == 0x7559 || codePoint == 0x7562 || codePoint == 0x7565 ||
           (codePoint >= 0x756A && codePoint <= 0x756B) || codePoint == 0x7570 || codePoint == 0x7576 ||
           codePoint == 0x7586 || codePoint == 0x758B || codePoint == 0x758F ||
           (codePoint >= 0x7591 && codePoint <= 0x7592) || codePoint == 0x75BC || codePoint == 0x75C5 ||
           codePoint == 0x75D5 || codePoint == 0x75DB || codePoint == 0x75F4 || codePoint == 0x760B ||
           codePoint == 0x7642 || codePoint == 0x7661 || codePoint == 0x7676 || codePoint == 0x7678 ||
           (codePoint >= 0x767B && codePoint <= 0x767E) || codePoint == 0x7684 ||
           (codePoint >= 0x7686 && codePoint <= 0x7687) || codePoint == 0x76AE || codePoint == 0x76BF ||
           codePoint == 0x76C3 || codePoint == 0x76CA || (codePoint >= 0x76DB && codePoint <= 0x76DC) ||
           codePoint == 0x76DF || codePoint == 0x76E1 || (codePoint >= 0x76E3 && codePoint <= 0x76E4) ||
           codePoint == 0x76E7 || codePoint == 0x76EE || codePoint == 0x76F2 || codePoint == 0x76F4 ||
           codePoint == 0x76F8 || codePoint == 0x76FC || codePoint == 0x76FE || codePoint == 0x7701 ||
           codePoint == 0x7709 || codePoint == 0x770B || (codePoint >= 0x771F && codePoint <= 0x7720) ||
           codePoint == 0x773C || codePoint == 0x773E || codePoint == 0x775B || codePoint == 0x7761 ||
           codePoint == 0x7763 || codePoint == 0x77A7 || codePoint == 0x77AD || codePoint == 0x77DB ||
           (codePoint >= 0x77E2 && codePoint <= 0x77E3) || codePoint == 0x77E5 || codePoint == 0x77ED ||
           codePoint == 0x77F3 || codePoint == 0x7802 || codePoint == 0x780D || codePoint == 0x7814 ||
           codePoint == 0x7832 || codePoint == 0x7834 || codePoint == 0x786C || codePoint == 0x788E ||
           codePoint == 0x7897 || codePoint == 0x789F || codePoint == 0x78A7 || codePoint == 0x78A9 ||
           codePoint == 0x78B0 || codePoint == 0x78BA || codePoint == 0x78BC || codePoint == 0x78C1 ||
           codePoint == 0x78E8 || codePoint == 0x78EF || codePoint == 0x790E || codePoint == 0x7919 ||
           codePoint == 0x793A || codePoint == 0x793E || (codePoint >= 0x7955 && codePoint <= 0x7956) ||
           (codePoint >= 0x795A && codePoint <= 0x795B) || (codePoint >= 0x795D && codePoint <= 0x795E) ||
           codePoint == 0x7965 || codePoint == 0x7968 || codePoint == 0x797F || codePoint == 0x7981 ||
           (codePoint >= 0x798D && codePoint <= 0x798F) || codePoint == 0x79AA || codePoint == 0x79AE ||
           codePoint == 0x79B8 || codePoint == 0x79BE || (codePoint >= 0x79C0 && codePoint <= 0x79C1) ||
           codePoint == 0x79CB || (codePoint >= 0x79D1 && codePoint <= 0x79D2) || codePoint == 0x79D8 ||
           codePoint == 0x79DF || codePoint == 0x79E4 || codePoint == 0x79E6 || codePoint == 0x79FB ||
           codePoint == 0x7A05 || codePoint == 0x7A0B || codePoint == 0x7A0D || codePoint == 0x7A2E ||
           codePoint == 0x7A31 || codePoint == 0x7A3F || codePoint == 0x7A46 ||
           (codePoint >= 0x7A4C && codePoint <= 0x7A4D) || codePoint == 0x7A69 || codePoint == 0x7A74 ||
           codePoint == 0x7A76 || (codePoint >= 0x7A79 && codePoint <= 0x7A7A) || codePoint == 0x7A7F ||
           codePoint == 0x7A81 || codePoint == 0x7A97 || codePoint == 0x7AA9 || codePoint == 0x7AAE ||
           codePoint == 0x7AB6 || codePoint == 0x7ACB || codePoint == 0x7AD9 ||
           (codePoint >= 0x7ADF && codePoint <= 0x7AE0) || codePoint == 0x7AE5 || codePoint == 0x7AEF ||
           codePoint == 0x7AF6 || codePoint == 0x7AF9 || codePoint == 0x7B11 || codePoint == 0x7B1B ||
           codePoint == 0x7B26 || codePoint == 0x7B28 || codePoint == 0x7B2C || codePoint == 0x7B46 ||
           codePoint == 0x7B49 || codePoint == 0x7B4B || codePoint == 0x7B54 || codePoint == 0x7B56 ||
           codePoint == 0x7B97 || codePoint == 0x7BA1 || codePoint == 0x7BAD || codePoint == 0x7BB1 ||
           codePoint == 0x7BC0 || codePoint == 0x7BC4 || codePoint == 0x7BC7 || codePoint == 0x7BC9 ||
           codePoint == 0x7C21 || codePoint == 0x7C2B || codePoint == 0x7C3D || codePoint == 0x7C3F ||
           codePoint == 0x7C43 || (codePoint >= 0x7C4C && codePoint <= 0x7C4D) || codePoint == 0x7C64 ||
           codePoint == 0x7C73 || codePoint == 0x7C89 || codePoint == 0x7C97 || codePoint == 0x7CB5 ||
           codePoint == 0x7CBE || codePoint == 0x7CCA || codePoint == 0x7CD5 || codePoint == 0x7CDF ||
           codePoint == 0x7CF8 || codePoint == 0x7CFB || codePoint == 0x7CFE || codePoint == 0x7D00 ||
           (codePoint >= 0x7D04 && codePoint <= 0x7D05) || codePoint == 0x7D0D || codePoint == 0x7D10 ||
           codePoint == 0x7D14 || (codePoint >= 0x7D19 && codePoint <= 0x7D1B) || codePoint == 0x7D20 ||
           codePoint == 0x7D22 || codePoint == 0x7D2B || (codePoint >= 0x7D2F && codePoint <= 0x7D30) ||
           codePoint == 0x7D39 || codePoint == 0x7D42 || codePoint == 0x7D44 || codePoint == 0x7D50 ||
           codePoint == 0x7D55 || codePoint == 0x7D61 || codePoint == 0x7D66 ||
           (codePoint >= 0x7D71 && codePoint <= 0x7D72) || codePoint == 0x7D93 || codePoint == 0x7D9C ||
           codePoint == 0x7DA0 || codePoint == 0x7DAD || (codePoint >= 0x7DB1 && codePoint <= 0x7DB2) ||
           codePoint == 0x7DCA || codePoint == 0x7DD2 || codePoint == 0x7DDA || codePoint == 0x7DE3 ||
           (codePoint >= 0x7DE8 && codePoint <= 0x7DE9) || codePoint == 0x7DEC || codePoint == 0x7DEF ||
           codePoint == 0x7DF4 || codePoint == 0x7E1B || codePoint == 0x7E23 || codePoint == 0x7E2E ||
           codePoint == 0x7E31 || (codePoint >= 0x7E3D && codePoint <= 0x7E3E) || codePoint == 0x7E41 ||
           codePoint == 0x7E46 || codePoint == 0x7E54 || codePoint == 0x7E5E || codePoint == 0x7E6A ||
           codePoint == 0x7E73 || codePoint == 0x7E7C || codePoint == 0x7E8C || codePoint == 0x7F36 ||
           codePoint == 0x7F38 || codePoint == 0x7F3A || codePoint == 0x7F51 || codePoint == 0x7F55 ||
           codePoint == 0x7F6A || codePoint == 0x7F6E || codePoint == 0x7F70 || codePoint == 0x7F72 ||
           codePoint == 0x7F75 || codePoint == 0x7F77 || codePoint == 0x7F85 || codePoint == 0x7F8A ||
           codePoint == 0x7F8E || codePoint == 0x7F9E || codePoint == 0x7FA4 || codePoint == 0x7FA9 ||
           codePoint == 0x7FBD || codePoint == 0x7FC1 || codePoint == 0x7FD2 || codePoint == 0x7FD4 ||
           codePoint == 0x7FF0 || codePoint == 0x7FF9 || (codePoint >= 0x7FFB && codePoint <= 0x7FFC) ||
           (codePoint >= 0x8000 && codePoint <= 0x8001) || codePoint == 0x8003 || codePoint == 0x8005 ||
           (codePoint >= 0x800C && codePoint <= 0x800D) || codePoint == 0x8010 || codePoint == 0x8012 ||
           codePoint == 0x8017 || codePoint == 0x8033 || codePoint == 0x8036 || codePoint == 0x804A ||
           codePoint == 0x8056 || codePoint == 0x805A || codePoint == 0x805E ||
           (codePoint >= 0x806F && codePoint <= 0x8070) || codePoint == 0x8072 || codePoint == 0x8077 ||
           codePoint == 0x807D || codePoint == 0x807F || codePoint == 0x8089 || codePoint == 0x809A ||
           codePoint == 0x80A1 || codePoint == 0x80A5 || codePoint == 0x80A9 || codePoint == 0x80AF ||
           codePoint == 0x80B2 || codePoint == 0x80CC || codePoint == 0x80CE || codePoint == 0x80D6 ||
           codePoint == 0x80DE || codePoint == 0x80E1 || codePoint == 0x80F8 || codePoint == 0x80FD ||
           codePoint == 0x8106 || codePoint == 0x812B || (codePoint >= 0x8153 && codePoint <= 0x8154) ||
           codePoint == 0x8166 || codePoint == 0x8170 || codePoint == 0x8173 || codePoint == 0x817F ||
           codePoint == 0x81BD || codePoint == 0x81C9 || codePoint == 0x81D8 || codePoint == 0x81E3 ||
           codePoint == 0x81E5 || codePoint == 0x81E8 || codePoint == 0x81EA || codePoint == 0x81ED ||
           (codePoint >= 0x81F3 && codePoint <= 0x81F4) || codePoint == 0x81FA || codePoint == 0x81FC ||
           (codePoint >= 0x8207 && codePoint <= 0x820A) || (codePoint >= 0x820C && codePoint <= 0x820D) ||
           codePoint == 0x8212 || codePoint == 0x821B || (codePoint >= 0x821E && codePoint <= 0x821F) ||
           codePoint == 0x822A || codePoint == 0x822C || codePoint == 0x8239 || codePoint == 0x8266 ||
           (codePoint >= 0x826E && codePoint <= 0x826F) || codePoint == 0x8272 || codePoint == 0x8278 ||
           codePoint == 0x827E || codePoint == 0x829D || codePoint == 0x82AC || codePoint == 0x82B1 ||
           codePoint == 0x82B3 || (codePoint >= 0x82E5 && codePoint <= 0x82E6) || codePoint == 0x82F1 ||
           codePoint == 0x8305 || codePoint == 0x832B || codePoint == 0x8332 || codePoint == 0x8336 ||
           codePoint == 0x8349 || codePoint == 0x8352 || codePoint == 0x8377 || codePoint == 0x837C ||
           (codePoint >= 0x8389 && codePoint <= 0x838A) || codePoint == 0x838E || codePoint == 0x83AB ||
           codePoint == 0x83DC || codePoint == 0x83E9 || codePoint == 0x83EF || codePoint == 0x83F2 ||
           codePoint == 0x8404 || codePoint == 0x840A || codePoint == 0x842C || codePoint == 0x843D ||
           codePoint == 0x8449 || codePoint == 0x8457 || codePoint == 0x845B || codePoint == 0x8461 ||
           codePoint == 0x8482 || codePoint == 0x8499 || codePoint == 0x84B2 || codePoint == 0x84BC ||
           codePoint == 0x84CB || codePoint == 0x84EE || codePoint == 0x8515 || codePoint == 0x8521 ||
           codePoint == 0x8523 || codePoint == 0x856D || codePoint == 0x8584 || codePoint == 0x85A6 ||
           (codePoint >= 0x85A9 && codePoint <= 0x85AA) || codePoint == 0x85C9 || codePoint == 0x85CD ||
           codePoint == 0x85CF || codePoint == 0x85DD || (codePoint >= 0x85E4 && codePoint <= 0x85E5) ||
           (codePoint >= 0x8606 && codePoint <= 0x8607) || codePoint == 0x862D ||
           (codePoint >= 0x864D && codePoint <= 0x864E) || codePoint == 0x8655 || codePoint == 0x865B ||
           codePoint == 0x865F || codePoint == 0x8667 || codePoint == 0x866B || codePoint == 0x86C7 ||
           codePoint == 0x86CB || codePoint == 0x86D9 || codePoint == 0x8702 || codePoint == 0x871C ||
           codePoint == 0x8776 || codePoint == 0x878D || codePoint == 0x87A2 || codePoint == 0x87F2 ||
           codePoint == 0x87F9 || codePoint == 0x880D || codePoint == 0x883B || codePoint == 0x8840 ||
           codePoint == 0x884C || codePoint == 0x8853 || codePoint == 0x8857 || codePoint == 0x885B ||
           codePoint == 0x885D || codePoint == 0x8861 || codePoint == 0x8863 || codePoint == 0x8868 ||
           codePoint == 0x888B || codePoint == 0x88AB || (codePoint >= 0x88C1 && codePoint <= 0x88C2) ||
           codePoint == 0x88D5 || (codePoint >= 0x88DC && codePoint <= 0x88DD) || codePoint == 0x88E1 ||
           codePoint == 0x88FD || codePoint == 0x8907 || codePoint == 0x8932 ||
           (codePoint >= 0x897E && codePoint <= 0x897F) || codePoint == 0x8981 || codePoint == 0x8986 ||
           codePoint == 0x898B || codePoint == 0x898F || codePoint == 0x8996 || codePoint == 0x89AA ||
           codePoint == 0x89BA || codePoint == 0x89BD || codePoint == 0x89C0 || codePoint == 0x89D2 ||
           codePoint == 0x89E3 || codePoint == 0x89F8 || codePoint == 0x8A00 || codePoint == 0x8A02 ||
           codePoint == 0x8A08 || codePoint == 0x8A0A || codePoint == 0x8A0E || codePoint == 0x8A13 ||
           (codePoint >= 0x8A17 && codePoint <= 0x8A18) || codePoint == 0x8A25 || codePoint == 0x8A2A ||
           codePoint == 0x8A2D || codePoint == 0x8A31 || codePoint == 0x8A34 ||
           (codePoint >= 0x8A3B && codePoint <= 0x8A3C) || codePoint == 0x8A55 || codePoint == 0x8A5E ||
           codePoint == 0x8A62 || codePoint == 0x8A66 || codePoint == 0x8A69 ||
           (codePoint >= 0x8A71 && codePoint <= 0x8A73) || codePoint == 0x8A87 ||
           (codePoint >= 0x8A8C && codePoint <= 0x8A8D) || codePoint == 0x8A93 || codePoint == 0x8A95 ||
           codePoint == 0x8A9E || codePoint == 0x8AA0 || codePoint == 0x8AA4 || codePoint == 0x8AAA ||
           codePoint == 0x8AB0 || codePoint == 0x8AB2 || codePoint == 0x8ABC || codePoint == 0x8ABF ||
           codePoint == 0x8AC7 || codePoint == 0x8ACB || codePoint == 0x8AD2 || codePoint == 0x8AD6 ||
           codePoint == 0x8AF8 || codePoint == 0x8AFA || codePoint == 0x8AFE || codePoint == 0x8B00 ||
           codePoint == 0x8B02 || codePoint == 0x8B1B || codePoint == 0x8B1D || codePoint == 0x8B49 ||
           codePoint == 0x8B58 || codePoint == 0x8B5C || codePoint == 0x8B66 ||
           (codePoint >= 0x8B6F && codePoint <= 0x8B70) || codePoint == 0x8B77 || codePoint == 0x8B7D ||
           codePoint == 0x8B80 || codePoint == 0x8B8A || codePoint == 0x8B93 || codePoint == 0x8B9A ||
           codePoint == 0x8C37 || codePoint == 0x8C46 || codePoint == 0x8C48 || codePoint == 0x8C50 ||
           codePoint == 0x8C55 || codePoint == 0x8C61 || codePoint == 0x8C6A || codePoint == 0x8C6C ||
           codePoint == 0x8C78 || codePoint == 0x8C8C || codePoint == 0x8C93 ||
           (codePoint >= 0x8C9D && codePoint <= 0x8C9E) || (codePoint >= 0x8CA0 && codePoint <= 0x8CA2) ||
           codePoint == 0x8CA8 || (codePoint >= 0x8CAA && codePoint <= 0x8CAC) || codePoint == 0x8CB4 ||
           codePoint == 0x8CB7 || (codePoint >= 0x8CBB && codePoint <= 0x8CBC) || codePoint == 0x8CC0 ||
           (codePoint >= 0x8CC7 && codePoint <= 0x8CC8) || codePoint == 0x8CD3 || codePoint == 0x8CDC ||
           codePoint == 0x8CDE || (codePoint >= 0x8CE2 && codePoint <= 0x8CE4) || codePoint == 0x8CE6 ||
           codePoint == 0x8CEA || codePoint == 0x8CED || codePoint == 0x8CF4 || codePoint == 0x8CFA ||
           (codePoint >= 0x8CFC && codePoint <= 0x8CFD) || codePoint == 0x8D08 || codePoint == 0x8D0A ||
           codePoint == 0x8D0F || codePoint == 0x8D64 || codePoint == 0x8D6B || codePoint == 0x8D70 ||
           codePoint == 0x8D77 || codePoint == 0x8D85 || codePoint == 0x8D8A || codePoint == 0x8D95 ||
           codePoint == 0x8D99 || codePoint == 0x8DA3 || codePoint == 0x8DA8 || codePoint == 0x8DB3 ||
           codePoint == 0x8DCC || codePoint == 0x8DCE || codePoint == 0x8DD1 || codePoint == 0x8DDD ||
           codePoint == 0x8DDF || codePoint == 0x8DE1 || codePoint == 0x8DEF || codePoint == 0x8DF3 ||
           codePoint == 0x8E0F || codePoint == 0x8E22 || codePoint == 0x8E5F || codePoint == 0x8E64 ||
           codePoint == 0x8E8D || codePoint == 0x8EAB || codePoint == 0x8EB2 || codePoint == 0x8ECA ||
           (codePoint >= 0x8ECC && codePoint <= 0x8ECD) || codePoint == 0x8ED2 || codePoint == 0x8EDF ||
           codePoint == 0x8F03 || codePoint == 0x8F09 || (codePoint >= 0x8F14 && codePoint <= 0x8F15) ||
           codePoint == 0x8F1B || codePoint == 0x8F1D || (codePoint >= 0x8F29 && codePoint <= 0x8F2A) ||
           codePoint == 0x8F2F || codePoint == 0x8F38 || codePoint == 0x8F49 || codePoint == 0x8F5F ||
           codePoint == 0x8F9B || codePoint == 0x8FA6 || codePoint == 0x8FA8 || codePoint == 0x8FAD ||
           (codePoint >= 0x8FAF && codePoint <= 0x8FB2) || codePoint == 0x8FB5 || codePoint == 0x8FC5 ||
           codePoint == 0x8FCE || codePoint == 0x8FD1 || codePoint == 0x8FD4 || codePoint == 0x8FE6 ||
           (codePoint >= 0x8FEA && codePoint <= 0x8FEB) || codePoint == 0x8FF0 || codePoint == 0x8FF4 ||
           codePoint == 0x8FF7 || codePoint == 0x8FFD || (codePoint >= 0x9000 && codePoint <= 0x9001) ||
           codePoint == 0x9003 || codePoint == 0x9006 || (codePoint >= 0x900F && codePoint <= 0x9010) ||
           codePoint == 0x9014 || (codePoint >= 0x9019 && codePoint <= 0x901B) || codePoint == 0x901D ||
           (codePoint >= 0x901F && codePoint <= 0x9020) || (codePoint >= 0x9022 && codePoint <= 0x9023) ||
           (codePoint >= 0x9031 && codePoint <= 0x9032) || codePoint == 0x9038 || codePoint == 0x903C ||
           codePoint == 0x9047 || (codePoint >= 0x904A && codePoint <= 0x904B) ||
           (codePoint >= 0x904D && codePoint <= 0x904E) || (codePoint >= 0x9053 && codePoint <= 0x9055) ||
           codePoint == 0x9059 || codePoint == 0x905C || codePoint == 0x9060 || codePoint == 0x9069 ||
           (codePoint >= 0x906D && codePoint <= 0x906E) || codePoint == 0x9072 ||
           (codePoint >= 0x9077 && codePoint <= 0x9078) || codePoint == 0x907A ||
           (codePoint >= 0x907F && codePoint <= 0x9081) || codePoint == 0x9084 || codePoint == 0x908A ||
           codePoint == 0x908F || codePoint == 0x9091 || codePoint == 0x90A3 || codePoint == 0x90A6 ||
           codePoint == 0x90AA || codePoint == 0x90B1 || codePoint == 0x90CE || codePoint == 0x90E8 ||
           codePoint == 0x90ED || codePoint == 0x90F5 || codePoint == 0x90FD || codePoint == 0x9102 ||
           codePoint == 0x9109 || codePoint == 0x912D || codePoint == 0x9130 || codePoint == 0x9149 ||
           codePoint == 0x914D || codePoint == 0x9152 || (codePoint >= 0x9177 && codePoint <= 0x9178) ||
           codePoint == 0x9189 || codePoint == 0x9192 || codePoint == 0x919C || codePoint == 0x91AB ||
           (codePoint >= 0x91C6 && codePoint <= 0x91C7) || (codePoint >= 0x91CB && codePoint <= 0x91CF) ||
           codePoint == 0x91D1 || codePoint == 0x91DD || codePoint == 0x91E3 || codePoint == 0x9234 ||
           codePoint == 0x9262 || codePoint == 0x9280 || codePoint == 0x9285 || codePoint == 0x9296 ||
           codePoint == 0x9298 || codePoint == 0x92B3 || codePoint == 0x92B7 || codePoint == 0x92D2 ||
           codePoint == 0x92FC || codePoint == 0x9304 || codePoint == 0x9322 || codePoint == 0x9326 ||
           codePoint == 0x932B || codePoint == 0x932F || codePoint == 0x934B || codePoint == 0x9375 ||
           codePoint == 0x937E || codePoint == 0x938A || codePoint == 0x9396 || codePoint == 0x93AE ||
           codePoint == 0x93E1 || codePoint == 0x9418 || codePoint == 0x9435 || codePoint == 0x9451 ||
           codePoint == 0x9577 || codePoint == 0x9580 || codePoint == 0x9583 || codePoint == 0x9589 ||
           codePoint == 0x958B || codePoint == 0x958F || (codePoint >= 0x9592 && codePoint <= 0x9593) ||
           codePoint == 0x95A3 || codePoint == 0x95B1 || codePoint == 0x95C6 || codePoint == 0x95CA ||
           codePoint == 0x95CD || codePoint == 0x95D0 || codePoint == 0x95DC || codePoint == 0x95E1 ||
           codePoint == 0x961C || codePoint == 0x9632 || codePoint == 0x963B ||
           (codePoint >= 0x963F && codePoint <= 0x9640) || codePoint == 0x9644 || codePoint == 0x964D ||
           codePoint == 0x9650 || (codePoint >= 0x9662 && codePoint <= 0x9664) || codePoint == 0x966A ||
           codePoint == 0x9670 || codePoint == 0x9673 || (codePoint >= 0x9675 && codePoint <= 0x9678) ||
           codePoint == 0x967D || codePoint == 0x9686 || codePoint == 0x968A || codePoint == 0x968E ||
           codePoint == 0x9694 || (codePoint >= 0x969B && codePoint <= 0x969C) || codePoint == 0x96A8 ||
           codePoint == 0x96AA || codePoint == 0x96B1 || codePoint == 0x96B6 || codePoint == 0x96B9 ||
           codePoint == 0x96BB || (codePoint >= 0x96C4 && codePoint <= 0x96C6) || codePoint == 0x96C9 ||
           codePoint == 0x96D6 || codePoint == 0x96D9 || codePoint == 0x96DC || codePoint == 0x96DE ||
           (codePoint >= 0x96E2 && codePoint <= 0x96E3) || codePoint == 0x96E8 || codePoint == 0x96EA ||
           codePoint == 0x96F2 || (codePoint >= 0x96F6 && codePoint <= 0x96F7) || codePoint == 0x96FB ||
           codePoint == 0x9700 || codePoint == 0x9707 || codePoint == 0x970D || codePoint == 0x9727 ||
           codePoint == 0x9732 || (codePoint >= 0x9738 && codePoint <= 0x9739) || codePoint == 0x9742 ||
           codePoint == 0x9748 || (codePoint >= 0x9751 && codePoint <= 0x9752) || codePoint == 0x9756 ||
           codePoint == 0x975C || codePoint == 0x975E || codePoint == 0x9760 || codePoint == 0x9762 ||
           codePoint == 0x9769 || codePoint == 0x977C || codePoint == 0x978B || codePoint == 0x97C3 ||
           codePoint == 0x97CB || codePoint == 0x97D3 || codePoint == 0x97ED || codePoint == 0x97F3 ||
           codePoint == 0x97FB || codePoint == 0x97FF || (codePoint >= 0x9801 && codePoint <= 0x9802) ||
           (codePoint >= 0x9805 && codePoint <= 0x9806) || codePoint == 0x9808 ||
           (codePoint >= 0x9810 && codePoint <= 0x9811) || codePoint == 0x9813 ||
           (codePoint >= 0x9817 && codePoint <= 0x9818) || codePoint == 0x981E || codePoint == 0x982D ||
           codePoint == 0x983B || codePoint == 0x9846 || (codePoint >= 0x984C && codePoint <= 0x984D) ||
           codePoint == 0x984F || codePoint == 0x9858 || codePoint == 0x985E || codePoint == 0x9867 ||
           codePoint == 0x986F || codePoint == 0x98A8 || codePoint == 0x98C4 || codePoint == 0x98DB ||
           codePoint == 0x98DF || codePoint == 0x98EF || codePoint == 0x98F2 ||
           (codePoint >= 0x98FD && codePoint <= 0x98FE) || codePoint == 0x9905 || codePoint == 0x990A ||
           codePoint == 0x9910 || codePoint == 0x9918 || codePoint == 0x9928 || codePoint == 0x9996 ||
           codePoint == 0x9999 || codePoint == 0x99AC || codePoint == 0x99D0 || codePoint == 0x99D5 ||
           codePoint == 0x99DB || codePoint == 0x9A0E || codePoint == 0x9A19 || codePoint == 0x9A37 ||
           codePoint == 0x9A45 || codePoint == 0x9A57 || codePoint == 0x9A5A || codePoint == 0x9AA8 ||
           codePoint == 0x9AD4 || codePoint == 0x9AD8 || codePoint == 0x9ADF || codePoint == 0x9AEE ||
           codePoint == 0x9B06 || codePoint == 0x9B25 || codePoint == 0x9B27 || codePoint == 0x9B2F ||
           (codePoint >= 0x9B31 && codePoint <= 0x9B32) || codePoint == 0x9B3C ||
           (codePoint >= 0x9B41 && codePoint <= 0x9B42) || codePoint == 0x9B45 || codePoint == 0x9B54 ||
           codePoint == 0x9B5A || codePoint == 0x9B6F || codePoint == 0x9BAE || codePoint == 0x9CE5 ||
           (codePoint >= 0x9CF3 && codePoint <= 0x9CF4) || codePoint == 0x9D3B || codePoint == 0x9D5D ||
           codePoint == 0x9DF9 || codePoint == 0x9E75 || codePoint == 0x9E7F || codePoint == 0x9E97 ||
           codePoint == 0x9EA5 || codePoint == 0x9EB5 || (codePoint >= 0x9EBB && codePoint <= 0x9EBC) ||
           codePoint == 0x9EC3 || (codePoint >= 0x9ECD && codePoint <= 0x9ECE) || codePoint == 0x9ED1 ||
           codePoint == 0x9ED8 || codePoint == 0x9EDE || codePoint == 0x9EE8 || codePoint == 0x9EF9 ||
           codePoint == 0x9EFD || codePoint == 0x9F0E || codePoint == 0x9F13 || codePoint == 0x9F20 ||
           codePoint == 0x9F3B || (codePoint >= 0x9F4A && codePoint <= 0x9F4B) || codePoint == 0x9F52 ||
           codePoint == 0x9F61 || codePoint == 0x9F8D || codePoint == 0x9F9C || codePoint == 0x9FA0 ||
           (codePoint >= 0xFE30 && codePoint <= 0xFE44) || (codePoint >= 0xFE49 && codePoint <= 0xFE52) ||
           (codePoint >= 0xFE54 && codePoint <= 0xFE61) || codePoint == 0xFE63 || codePoint == 0xFE68 ||
           (codePoint >= 0xFE6A && codePoint <= 0xFE6B) || (codePoint >= 0xFF01 && codePoint <= 0xFF03) ||
           (codePoint >= 0xFF05 && codePoint <= 0xFF0A) || (codePoint >= 0xFF0C && codePoint <= 0xFF0F) ||
           (codePoint >= 0xFF1A && codePoint <= 0xFF1B) || (codePoint >= 0xFF1F && codePoint <= 0xFF20) ||
           (codePoint >= 0xFF3B && codePoint <= 0xFF3D) || codePoint == 0xFF3F || codePoint == 0xFF5B ||
           codePoint == 0xFF5D;
}

//---------------------------------------------------------------------------------------------------------------------
auto JapaneseWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return (codePoint >= 0x3040 && codePoint <= 0x309F)     // Hiragana
           || (codePoint >= 0x30A0 && codePoint <= 0x30FF)  // Katakana
           || (codePoint >= 0x4E00 && codePoint <= 0x9FBF); // Kanji
}

//---------------------------------------------------------------------------------------------------------------------
auto KoreanWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return (codePoint >= 0x1100 && codePoint <= 0x11FF) || (codePoint >= 0x3130 && codePoint <= 0x318F) ||
           (codePoint >= 0xA960 && codePoint <= 0xA97F) || (codePoint >= 0xD7B0 && codePoint <= 0xD7FF);
}

//---------------------------------------------------------------------------------------------------------------------
auto VietnameseWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return (codePoint >= 0x0020 && codePoint <= 0x002F) || (codePoint >= 0x0030 && codePoint <= 0x0039) ||
           (codePoint >= 0x003A && codePoint <= 0x0040) || (codePoint >= 0x0041 && codePoint <= 0x005A) ||
           (codePoint >= 0x005B && codePoint <= 0x0060) || (codePoint >= 0x0061 && codePoint <= 0x007A) ||
           (codePoint >= 0x007B && codePoint <= 0x007E) || (codePoint >= 0x00C0 && codePoint <= 0x00C3) ||
           (codePoint >= 0x00C8 && codePoint <= 0x00CA) || (codePoint >= 0x00CC && codePoint <= 0x00CD) ||
           codePoint == 0x00D0 || (codePoint >= 0x00D2 && codePoint <= 0x00D5) ||
           (codePoint >= 0x00D9 && codePoint <= 0x00DA) || codePoint == 0x00DD ||
           (codePoint >= 0x00E0 && codePoint <= 0x00E3) || (codePoint >= 0x00E8 && codePoint <= 0x00EA) ||
           (codePoint >= 0x00EC && codePoint <= 0x00ED) || (codePoint >= 0x00F2 && codePoint <= 0x00F5) ||
           (codePoint >= 0x00F9 && codePoint <= 0x00FA) || codePoint == 0x00FD ||
           (codePoint >= 0x0102 && codePoint <= 0x0103) || (codePoint >= 0x0110 && codePoint <= 0x0111) ||
           (codePoint >= 0x0128 && codePoint <= 0x0129) || (codePoint >= 0x0168 && codePoint <= 0x0169) ||
           (codePoint >= 0x01A0 && codePoint <= 0x01B0) || (codePoint >= 0x1EA0 && codePoint <= 0x1EF9) ||
           (codePoint >= 0x02C6 && codePoint <= 0x0323);
}

//---------------------------------------------------------------------------------------------------------------------
auto SymbolWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return (codePoint >= 0x2600 && codePoint <= 0x26FF)     // Miscellaneous Symbols
           || (codePoint >= 0x19E0 && codePoint <= 0x19FF)  // Khmer Symbols
           || (codePoint >= 0x20A0 && codePoint <= 0x20CF)  // Currency Symbols
           || (codePoint >= 0x20D0 && codePoint <= 0x20FF)  // Combining Diacritical Marks for Symbols
           || (codePoint >= 0x2100 && codePoint <= 0x214F)  // Letterlike Symbols
           || (codePoint >= 0x27C0 && codePoint <= 0x27EF)  // Miscellaneous Mathematical Symbols-A
           || (codePoint >= 0x2980 && codePoint <= 0x29FF)  // Miscellaneous Mathematical Symbols-B
           || (codePoint >= 0x2B00 && codePoint <= 0x2BFF)  // Miscellaneous Symbols and Arrows
           || (codePoint >= 0x3000 && codePoint <= 0x303F)  // CJK Symbols and Punctuation
           || (codePoint >= 0x4DC0 && codePoint <= 0x4DFF); // Yijing Hexagram Symbols
}

//---------------------------------------------------------------------------------------------------------------------
auto OghamWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return codePoint >= 0x1680 && codePoint <= 0x169F; // Ogham
}

//---------------------------------------------------------------------------------------------------------------------
auto RunicWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return codePoint >= 0x16A0 && codePoint <= 0x16FF; // Runic
}

//---------------------------------------------------------------------------------------------------------------------
auto NkoWritingSystem(QChar c) -> bool
{
    char16_t codePoint = c.unicode();

    return codePoint >= 0x07C0 && codePoint <= 0x07FF; // Nko
}

//---------------------------------------------------------------------------------------------------------------------
auto WritingSystemSample(SVGFontWritingSystem writingSystem) -> QString
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    return [&]() -> QStringView
    {
        switch (writingSystem)
        {
            case SVGFontWritingSystem::Any:
            case SVGFontWritingSystem::Symbol:
                // show only ascii characters
                return u"AaBbzZ";
            case SVGFontWritingSystem::Latin:
                return u"Aa\x00C3\x00E1Zz";
            case SVGFontWritingSystem::Greek:
                return u"\x0393\x03B1\x03A9\x03C9";
            case SVGFontWritingSystem::Cyrillic:
                return u"\x0414\x0434\x0436\x044f";
            case SVGFontWritingSystem::Armenian:
                return u"\x053f\x054f\x056f\x057f";
            case SVGFontWritingSystem::Hebrew:
                return u"\x05D0\x05D1\x05D2\x05D3";
            case SVGFontWritingSystem::Arabic:
                return u"\x0623\x0628\x062C\x062F\x064A\x0629\x0020\x0639\x0631\x0628\x064A\x0629";
            case SVGFontWritingSystem::Syriac:
                return u"\x0715\x0725\x0716\x0726";
            case SVGFontWritingSystem::Thaana:
                return u"\x0784\x0794\x078c\x078d";
            case SVGFontWritingSystem::Devanagari:
                return u"\x0905\x0915\x0925\x0935";
            case SVGFontWritingSystem::Bengali:
                return u"\x0986\x0996\x09a6\x09b6";
            case SVGFontWritingSystem::Gurmukhi:
                return u"\x0a05\x0a15\x0a25\x0a35";
            case SVGFontWritingSystem::Gujarati:
                return u"\x0a85\x0a95\x0aa5\x0ab5";
            case SVGFontWritingSystem::Oriya:
                return u"\x0b06\x0b16\x0b2b\x0b36";
            case SVGFontWritingSystem::Tamil:
                return u"\x0b89\x0b99\x0ba9\x0bb9";
            case SVGFontWritingSystem::Telugu:
                return u"\x0c05\x0c15\x0c25\x0c35";
            case SVGFontWritingSystem::Kannada:
                return u"\x0c85\x0c95\x0ca5\x0cb5";
            case SVGFontWritingSystem::Malayalam:
                return u"\x0d05\x0d15\x0d25\x0d35";
            case SVGFontWritingSystem::Sinhala:
                return u"\x0d90\x0da0\x0db0\x0dc0";
            case SVGFontWritingSystem::Thai:
                return u"\x0e02\x0e12\x0e22\x0e32";
            case SVGFontWritingSystem::Lao:
                return u"\x0e8d\x0e9d\x0ead\x0ebd";
            case SVGFontWritingSystem::Tibetan:
                return u"\x0f00\x0f01\x0f02\x0f03";
            case SVGFontWritingSystem::Myanmar:
                return u"\x1000\x1001\x1002\x1003";
            case SVGFontWritingSystem::Georgian:
                return u"\x10a0\x10b0\x10c0\x10d0";
            case SVGFontWritingSystem::Khmer:
                return u"\x1780\x1790\x17b0\x17c0";
            case SVGFontWritingSystem::SimplifiedChinese:
                return u"\x4e2d\x6587\x8303\x4f8b";
            case SVGFontWritingSystem::TraditionalChinese:
                return u"\x4e2d\x6587\x7bc4\x4f8b";
            case SVGFontWritingSystem::Japanese:
                return u"\x30b5\x30f3\x30d7\x30eb\x3067\x3059";
            case SVGFontWritingSystem::Korean:
                return u"\xac00\xac11\xac1a\xac2f";
            case SVGFontWritingSystem::Vietnamese:
                return u"\x1ED7\x1ED9\x1ED1\x1ED3";
            case SVGFontWritingSystem::Ogham:
                return u"\x1681\x1682\x1683\x1684";
            case SVGFontWritingSystem::Runic:
                return u"\x16a0\x16a1\x16a2\x16a3";
            case SVGFontWritingSystem::Nko:
                return u"\x7ca\x7cb\x7cc\x7cd";
            default:
                return nullptr;
        }
    }()
                        .toString();
#else
    QString sample;
    switch (writingSystem)
    {
        case SVGFontWritingSystem::Any:
        case SVGFontWritingSystem::Symbol:
            sample += "AaBbzZ"_L1;
            break;
        case SVGFontWritingSystem::Latin:
            sample = "Aa"_L1;
            sample += QChar(0x00C3);
            sample += QChar(0x00E1);
            sample += "Zz"_L1;
            break;
        case SVGFontWritingSystem::Greek:
            sample += QChar(0x0393);
            sample += QChar(0x03B1);
            sample += QChar(0x03A9);
            sample += QChar(0x03C9);
            break;
        case SVGFontWritingSystem::Cyrillic:
            sample += QChar(0x0414);
            sample += QChar(0x0434);
            sample += QChar(0x0436);
            sample += QChar(0x044f);
            break;
        case SVGFontWritingSystem::Armenian:
            sample += QChar(0x053f);
            sample += QChar(0x054f);
            sample += QChar(0x056f);
            sample += QChar(0x057f);
            break;
        case SVGFontWritingSystem::Hebrew:
            sample += QChar(0x05D0);
            sample += QChar(0x05D1);
            sample += QChar(0x05D2);
            sample += QChar(0x05D3);
            break;
        case SVGFontWritingSystem::Arabic:
            sample += QChar(0x0623);
            sample += QChar(0x0628);
            sample += QChar(0x062C);
            sample += QChar(0x062F);
            sample += QChar(0x064A);
            sample += QChar(0x0629);
            sample += QChar(0x0020);
            sample += QChar(0x0639);
            sample += QChar(0x0631);
            sample += QChar(0x0628);
            sample += QChar(0x064A);
            sample += QChar(0x0629);
            break;
        case SVGFontWritingSystem::Syriac:
            sample += QChar(0x0715);
            sample += QChar(0x0725);
            sample += QChar(0x0716);
            sample += QChar(0x0726);
            break;
        case SVGFontWritingSystem::Thaana:
            sample += QChar(0x0784);
            sample += QChar(0x0794);
            sample += QChar(0x078c);
            sample += QChar(0x078d);
            break;
        case SVGFontWritingSystem::Devanagari:
            sample += QChar(0x0905);
            sample += QChar(0x0915);
            sample += QChar(0x0925);
            sample += QChar(0x0935);
            break;
        case SVGFontWritingSystem::Bengali:
            sample += QChar(0x0986);
            sample += QChar(0x0996);
            sample += QChar(0x09a6);
            sample += QChar(0x09b6);
            break;
        case SVGFontWritingSystem::Gurmukhi:
            sample += QChar(0x0a05);
            sample += QChar(0x0a15);
            sample += QChar(0x0a25);
            sample += QChar(0x0a35);
            break;
        case SVGFontWritingSystem::Gujarati:
            sample += QChar(0x0a85);
            sample += QChar(0x0a95);
            sample += QChar(0x0aa5);
            sample += QChar(0x0ab5);
            break;
        case SVGFontWritingSystem::Oriya:
            sample += QChar(0x0b06);
            sample += QChar(0x0b16);
            sample += QChar(0x0b2b);
            sample += QChar(0x0b36);
            break;
        case SVGFontWritingSystem::Tamil:
            sample += QChar(0x0b89);
            sample += QChar(0x0b99);
            sample += QChar(0x0ba9);
            sample += QChar(0x0bb9);
            break;
        case SVGFontWritingSystem::Telugu:
            sample += QChar(0x0c05);
            sample += QChar(0x0c15);
            sample += QChar(0x0c25);
            sample += QChar(0x0c35);
            break;
        case SVGFontWritingSystem::Kannada:
            sample += QChar(0x0c85);
            sample += QChar(0x0c95);
            sample += QChar(0x0ca5);
            sample += QChar(0x0cb5);
            break;
        case SVGFontWritingSystem::Malayalam:
            sample += QChar(0x0d05);
            sample += QChar(0x0d15);
            sample += QChar(0x0d25);
            sample += QChar(0x0d35);
            break;
        case SVGFontWritingSystem::Sinhala:
            sample += QChar(0x0d90);
            sample += QChar(0x0da0);
            sample += QChar(0x0db0);
            sample += QChar(0x0dc0);
            break;
        case SVGFontWritingSystem::Thai:
            sample += QChar(0x0e02);
            sample += QChar(0x0e12);
            sample += QChar(0x0e22);
            sample += QChar(0x0e32);
            break;
        case SVGFontWritingSystem::Lao:
            sample += QChar(0x0e8d);
            sample += QChar(0x0e9d);
            sample += QChar(0x0ead);
            sample += QChar(0x0ebd);
            break;
        case SVGFontWritingSystem::Tibetan:
            sample += QChar(0x0f00);
            sample += QChar(0x0f01);
            sample += QChar(0x0f02);
            sample += QChar(0x0f03);
            break;
        case SVGFontWritingSystem::Myanmar:
            sample += QChar(0x1000);
            sample += QChar(0x1001);
            sample += QChar(0x1002);
            sample += QChar(0x1003);
            break;
        case SVGFontWritingSystem::Georgian:
            sample += QChar(0x10a0);
            sample += QChar(0x10b0);
            sample += QChar(0x10c0);
            sample += QChar(0x10d0);
            break;
        case SVGFontWritingSystem::Khmer:
            sample += QChar(0x1780);
            sample += QChar(0x1790);
            sample += QChar(0x17b0);
            sample += QChar(0x17c0);
            break;
        case SVGFontWritingSystem::SimplifiedChinese:
            sample += QChar(0x4e2d);
            sample += QChar(0x6587);
            sample += QChar(0x8303);
            sample += QChar(0x4f8b);
            break;
        case SVGFontWritingSystem::TraditionalChinese:
            sample += QChar(0x4e2d);
            sample += QChar(0x6587);
            sample += QChar(0x7bc4);
            sample += QChar(0x4f8b);
            break;
        case SVGFontWritingSystem::Japanese:
            sample += QChar(0x30b5);
            sample += QChar(0x30f3);
            sample += QChar(0x30d7);
            sample += QChar(0x30eb);
            sample += QChar(0x3067);
            sample += QChar(0x3059);
            break;
        case SVGFontWritingSystem::Korean:
            sample += QChar(0xac00);
            sample += QChar(0xac11);
            sample += QChar(0xac1a);
            sample += QChar(0xac2f);
            break;
        case SVGFontWritingSystem::Vietnamese:
        {
            static const char vietnameseUtf8[] = {
                char(0xef), char(0xbb), char(0xbf), char(0xe1), char(0xbb), char(0x97), char(0xe1), char(0xbb),
                char(0x99), char(0xe1), char(0xbb), char(0x91), char(0xe1), char(0xbb), char(0x93),
            };
            sample += QString::fromUtf8(vietnameseUtf8, sizeof(vietnameseUtf8));
            break;
        }
        case SVGFontWritingSystem::Ogham:
            sample += QChar(0x1681);
            sample += QChar(0x1682);
            sample += QChar(0x1683);
            sample += QChar(0x1684);
            break;
        case SVGFontWritingSystem::Runic:
            sample += QChar(0x16a0);
            sample += QChar(0x16a1);
            sample += QChar(0x16a2);
            sample += QChar(0x16a3);
            break;
        case SVGFontWritingSystem::Nko:
            sample += QChar(0x7ca);
            sample += QChar(0x7cb);
            sample += QChar(0x7cc);
            sample += QChar(0x7cd);
            break;
        default:
            break;
    }
    return sample;
#endif
}
