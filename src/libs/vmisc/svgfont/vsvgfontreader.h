/************************************************************************
 **
 **  @file   vsvgfontreader.h
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
#ifndef VSVGFONTREADER_H
#define VSVGFONTREADER_H

#include <QCoreApplication>
#include <QString>
#include <QXmlStreamReader>

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../defglobal.h"
#endif

class QFile;
class VSvgFont;
class VSvgFontEngine;

class VSvgFontReader : public QXmlStreamReader
{
    Q_DECLARE_TR_FUNCTIONS(VSvgFontReader) // NOLINT

public:
    VSvgFontReader() = default;
    ~VSvgFontReader() = default;

    auto ReadSvgFontHeader(QFile *file) -> VSvgFont;
    auto ReadSvgFont(QFile *file) -> VSvgFontEngine;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VSvgFontReader) // NOLINT

    auto ReadFontHeader() -> VSvgFont;
    auto ReadFontFace() -> VSvgFont;
    auto ReadFontData() -> VSvgFontEngine;
    auto ReadFont() -> VSvgFontEngine;

    void AssertRootTag(const QString &tag) const;

    void SetFontFace(VSvgFont *font);

    void ParseSvgGlyph(VSvgFontEngine *engine, const QXmlStreamAttributes &glyphAttr);
};

#endif // VSVGFONTREADER_H
