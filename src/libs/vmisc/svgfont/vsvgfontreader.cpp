/************************************************************************
 **
 **  @file   vsvgfontreader.cpp
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
#include "vsvgfontreader.h"
#include "../../ifc/exception/vexception.h"
#include "../def.h"
#include "qpainterpath.h"
#include "svgdef.h"
#include "vsvgfont.h"
#include "vsvgfontengine.h"
#include "vsvgpathtokenizer.h"

#include <QFile>
#include <QPainterPath>
#include <QtDebug>

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto ParseFontStyle(const QString &fontStyle) -> SVGFontStyle
{
    if (fontStyle == QLatin1String("normal"))
    {
        return SVGFontStyle::Normal;
    }

    if (fontStyle == QLatin1String("italic"))
    {
        return SVGFontStyle::Italic;
    }

    if (fontStyle == QLatin1String("oblique"))
    {
        return SVGFontStyle::Oblique;
    }

    return SVGFontStyle::Normal;
}
//---------------------------------------------------------------------------------------------------------------------
auto ParseFontWeight(const QString &fontWeight) -> SVGFontWeight
{
    if (fontWeight.isEmpty())
    {
        return SVGFontWeight::Normal;
    }

    bool ok;
    int parsedWeight = fontWeight.toInt(&ok);

    if (ok)
    {
        switch (parsedWeight)
        {
            case static_cast<int>(SVGFontWeight::Thin):
                return SVGFontWeight::Thin;
            case static_cast<int>(SVGFontWeight::ExtraLight):
                return SVGFontWeight::ExtraLight;
            case static_cast<int>(SVGFontWeight::Light):
                return SVGFontWeight::Light;
            case static_cast<int>(SVGFontWeight::Medium):
                return SVGFontWeight::Medium;
            case static_cast<int>(SVGFontWeight::DemiBold):
                return SVGFontWeight::DemiBold;
            case static_cast<int>(SVGFontWeight::Bold):
                return SVGFontWeight::Bold;
            case static_cast<int>(SVGFontWeight::ExtraBold):
                return SVGFontWeight::ExtraBold;
            case static_cast<int>(SVGFontWeight::Black):
                return SVGFontWeight::Black;
            case static_cast<int>(SVGFontWeight::Normal):
            default:
                return SVGFontWeight::Normal;
        }
    }
    else
    {
        QString fontWeightLower = fontWeight.toLower();
        if (fontWeightLower == QLatin1String("normal"))
        {
            return SVGFontWeight::Normal;
        }

        if (fontWeightLower == QLatin1String("bold"))
        {
            return SVGFontWeight::Bold;
        }
    }
    return SVGFontWeight::Normal;
}

//---------------------------------------------------------------------------------------------------------------------
auto InitFont(const QXmlStreamAttributes &fontAttr) -> VSvgFont
{
    const auto hax = fontAttr.value(QLatin1String("horiz-adv-x"));
    qreal horizAdvX = hax.toDouble();

    QString id = fontAttr.value(QLatin1String("id")).toString();
    if (id.isEmpty())
    {
        id = fontAttr.value(QLatin1String("xml:id")).toString();
    }

    VSvgFont font(horizAdvX);
    font.SetId(id);
    return font;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontReader::ReadSvgFontHeader(QFile *file) -> VSvgFont
{
    SCASSERT(file != nullptr)

    setDevice(file);

    try
    {
        if (readNextStartElement())
        {
            if (name() == QLatin1String("svg"))
            {
                while (readNextStartElement())
                {
                    if (name() == QLatin1String("defs"))
                    {
                        return ReadFontHeader();
                    }

                    skipCurrentElement();
                }
            }
            else
            {
                raiseError(tr("Incorrect file"));
            }
        }
    }
    catch (const VException &e)
    {
        raiseError(e.ErrorMessage());
    }

    if (hasError())
    {
        qDebug() << "Error parsing SVG font file:" << errorString();
    }

    clear();

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontReader::ReadSvgFont(QFile *file) -> VSvgFontEngine
{
    SCASSERT(file != nullptr)

    setDevice(file);

    try
    {
        if (readNextStartElement())
        {
            if (name() == QLatin1String("svg"))
            {
                while (readNextStartElement())
                {
                    if (name() == QLatin1String("defs"))
                    {
                        return ReadFontData();
                    }

                    skipCurrentElement();
                }
            }
            else
            {
                raiseError(tr("Incorrect file"));
            }
        }
    }
    catch (const VException &e)
    {
        raiseError(e.ErrorMessage());
    }

    if (hasError())
    {
        qDebug() << "Error parsing SVG font file:" << errorString();
    }

    clear();

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontReader::ReadFontHeader() -> VSvgFont
{
    AssertRootTag(QLatin1String("defs"));

    while (readNextStartElement())
    {
        if (name() == QLatin1String("font"))
        {
            return ReadFontFace();
        }

        skipCurrentElement();
    }
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontReader::ReadFontFace() -> VSvgFont
{
    AssertRootTag(QLatin1String("font"));

    VSvgFont font = InitFont(attributes());

    while (readNextStartElement())
    {
        if (name() == QLatin1String("font-face"))
        {
            SetFontFace(&font);
            return font;
        }

        skipCurrentElement();
    }

    return font;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontReader::ReadFontData() -> VSvgFontEngine
{
    AssertRootTag(QLatin1String("defs"));

    while (readNextStartElement())
    {
        if (name() == QLatin1String("font"))
        {
            return ReadFont();
        }

        skipCurrentElement();
    }

    readElementText();

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontReader::ReadFont() -> VSvgFontEngine
{
    AssertRootTag(QLatin1String("font"));

    VSvgFont font = InitFont(attributes());

    VSvgFontEngine engine;

    while (readNextStartElement())
    {
        if (name() == QLatin1String("font-face"))
        {
            SetFontFace(&font);
            engine = VSvgFontEngine(font);
        }
        else
        {
            skipCurrentElement();
        }
    }

    while (readNextStartElement())
    {
        if (name() == QLatin1String("missing-glyph") || name() == QLatin1String("glyph"))
        {
            ParseSvgGlyph(&engine, attributes());
        }
        else
        {
            skipCurrentElement();
        }
    }

    readElementText();

    engine.RecalculateFontSize();
    engine.RecognizeWritingSystems();

    return engine;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFontReader::AssertRootTag(const QString &tag) const
{
    if (not(isStartElement() && name() == tag))
    {
        throw VException(tr("Unexpected tag %1 in line %2").arg(name().toString()).arg(lineNumber()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFontReader::SetFontFace(VSvgFont *font)
{
    QXmlStreamAttributes fontFaceAttr = attributes();
    QString fontFamily = fontFaceAttr.value(QLatin1String("font-family")).toString();
    const auto unitsPerEmStr = fontFaceAttr.value(QLatin1String("units-per-em"));

    qreal unitsPerEm = unitsPerEmStr.toDouble();
    if (qFuzzyIsNull(unitsPerEm))
    {
        unitsPerEm = 1000;
    }

    const auto ascentStr = fontFaceAttr.value(QLatin1String("ascent"));
    qreal ascent = ascentStr.toDouble();
    if (qFuzzyIsNull(ascent))
    {
        ascent = 800;
    }

    const auto descentStr = fontFaceAttr.value(QLatin1String("descent"));
    qreal descent = descentStr.toDouble();
    if (qFuzzyIsNull(descent))
    {
        descent = -200;
    }

    QString fontStyle = fontFaceAttr.value(QLatin1String("font-style")).toString();
    QString fontWeight = fontFaceAttr.value(QLatin1String("font-weight")).toString();
    QString fontName;

    while (readNextStartElement())
    {
        if (name() == QLatin1String("font-face-src"))
        {
            while (readNextStartElement())
            {
                if (name() == QLatin1String("font-face-name"))
                {
                    fontName = attributes().value(QLatin1String("name")).toString();
                }
                else
                {
                    skipCurrentElement();
                }
            }
        }
        else
        {
            skipCurrentElement();
        }
    }

    font->SetFamilyName(fontFamily);
    font->SetName(fontName);
    font->SetUnitsPerEm(unitsPerEm);
    font->SetAscent(ascent);
    font->SetDescent(descent);
    font->SetStyle(ParseFontStyle(fontStyle));
    font->SetWeight(ParseFontWeight(fontWeight));
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFontReader::ParseSvgGlyph(VSvgFontEngine *engine, const QXmlStreamAttributes &glyphAttr)
{
    auto uncStr = glyphAttr.value(QLatin1String("unicode"));
    auto havStr = glyphAttr.value(QLatin1String("horiz-adv-x"));
    auto pathStr = glyphAttr.value(QLatin1String("d"));

    QChar unicode = (uncStr.isEmpty()) ? u'\0' : uncStr.at(0);
    qreal havx = (havStr.isEmpty()) ? -1 : havStr.toDouble();
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    VSVGPathTokenizer tokenizer(pathStr.toString());
    tokenizer.SetSinglePath(true); // Do not close subpaths
    tokenizer.ToPainterPath(path);

    engine->AddGlyph(unicode, path, havx);

    readElementText();
}
