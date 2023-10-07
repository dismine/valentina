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

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto ParseFontStyle(const QString &fontStyle) -> SVGFontStyle
{
    if (fontStyle == "normal"_L1)
    {
        return SVGFontStyle::Normal;
    }

    if (fontStyle == "italic"_L1)
    {
        return SVGFontStyle::Italic;
    }

    if (fontStyle == "oblique"_L1)
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
        if (fontWeightLower == "normal"_L1)
        {
            return SVGFontWeight::Normal;
        }

        if (fontWeightLower == "bold"_L1)
        {
            return SVGFontWeight::Bold;
        }
    }
    return SVGFontWeight::Normal;
}

//---------------------------------------------------------------------------------------------------------------------
auto InitFont(const QXmlStreamAttributes &fontAttr) -> VSvgFont
{
    const auto hax = fontAttr.value("horiz-adv-x"_L1);
    qreal horizAdvX = hax.toDouble();

    QString id = fontAttr.value("id"_L1).toString();
    if (id.isEmpty())
    {
        id = fontAttr.value("xml:id"_L1).toString();
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
            if (name() == "svg"_L1)
            {
                while (readNextStartElement())
                {
                    if (name() == "defs"_L1)
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
            if (name() == "svg"_L1)
            {
                while (readNextStartElement())
                {
                    if (name() == "defs"_L1)
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
    AssertRootTag("defs"_L1);

    while (readNextStartElement())
    {
        if (name() == "font"_L1)
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
    AssertRootTag("font"_L1);

    VSvgFont font = InitFont(attributes());

    while (readNextStartElement())
    {
        if (name() == "font-face"_L1)
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
    AssertRootTag("defs"_L1);

    while (readNextStartElement())
    {
        if (name() == "font"_L1)
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
    AssertRootTag("font"_L1);

    VSvgFont font = InitFont(attributes());

    VSvgFontEngine engine;

    while (readNextStartElement())
    {
        if (name() == "font-face"_L1)
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
        if (name() == "missing-glyph"_L1 || name() == "glyph"_L1)
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
    QString fontFamily = fontFaceAttr.value("font-family"_L1).toString();
    const auto unitsPerEmStr = fontFaceAttr.value("units-per-em"_L1);

    qreal unitsPerEm = unitsPerEmStr.toDouble();
    if (qFuzzyIsNull(unitsPerEm))
    {
        unitsPerEm = 1000;
    }

    const auto ascentStr = fontFaceAttr.value("ascent"_L1);
    qreal ascent = ascentStr.toDouble();
    if (qFuzzyIsNull(ascent))
    {
        ascent = 800;
    }

    const auto descentStr = fontFaceAttr.value("descent"_L1);
    qreal descent = descentStr.toDouble();
    if (qFuzzyIsNull(descent))
    {
        descent = -200;
    }

    QString fontStyle = fontFaceAttr.value("font-style"_L1).toString();
    QString fontWeight = fontFaceAttr.value("font-weight"_L1).toString();
    QString fontName;

    while (readNextStartElement())
    {
        if (name() == "font-face-src"_L1)
        {
            while (readNextStartElement())
            {
                if (name() == "font-face-name"_L1)
                {
                    fontName = attributes().value("name"_L1).toString();
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
    auto uncStr = glyphAttr.value("unicode"_L1);
    auto havStr = glyphAttr.value("horiz-adv-x"_L1);
    auto pathStr = glyphAttr.value("d"_L1);

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
