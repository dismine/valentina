/************************************************************************
 **
 **  @file   vsvgfontengine.cpp
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
#include "vsvgfontengine.h"
#include "../compatibility.h"
#include "../def.h"
#include "qpainterpath.h"
#include "svgdef.h"
#include "vsvgfontengine_p.h"
#include "vsvgglyph.h"

#include <QChar>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QRegion>
#include <QSet>

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
VSvgFontEngine::VSvgFontEngine()
  : d(new VSvgFontEngineData())
{
}

//---------------------------------------------------------------------------------------------------------------------
VSvgFontEngine::VSvgFontEngine(const VSvgFont &font)
  : d(new VSvgFontEngineData(font))
{
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL(VSvgFontEngine)

//---------------------------------------------------------------------------------------------------------------------
VSvgFontEngine::~VSvgFontEngine() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontEngine::operator=(const VSvgFontEngine &engine) -> VSvgFontEngine &
{
    if (&engine == this)
    {
        return *this;
    }
    d = engine.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontEngine::Font() const -> VSvgFont
{
    return d->m_font;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFontEngine::SetFont(const VSvgFont &fnt)
{
    d->m_font = fnt;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFontEngine::SetFontPixelSize(int pixelSize)
{
    d->m_font.SetPixelSize(pixelSize);
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFontEngine::SetFontPointSizeF(qreal pointSize)
{
    d->m_font.SetPointSizeF(pointSize);
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFontEngine::SetFontPointSize(int pointSize)
{
    d->m_font.SetPointSize(pointSize);
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontEngine::FontPixelSize() const -> int
{
    if (d->m_font.PixelSize() > 0)
    {
        return d->m_font.PixelSize();
    }

    qreal pointSize = d->m_font.PointSizeF();
    if (pointSize <= 0)
    {
        qWarning("VSvgFontEngine::FontPixelSize: Point size <= 0 (%f), must be greater than 0", pointSize);
        return -1;
    }

    return qRound(pointSize * PrintDPI / 72);
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontEngine::FontHeight() const -> qreal
{
    return FromFontUnits(d->m_font.Ascent() + qAbs(d->m_font.Descent()));
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontEngine::FromFontUnits(qreal val, int pixelSize) const -> qreal
{
    int pxSize = pixelSize > 0 ? pixelSize : FontPixelSize();
    return pxSize * val / d->m_font.UnitsPerEm();
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontEngine::ToFontUnits(qreal val, int pixelSize) const -> qreal
{
    int pxSize = pixelSize > 0 ? pixelSize : FontPixelSize();
    return val * d->m_font.UnitsPerEm() / pxSize;
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VSvgFontEngine::VSvgFontEngine(VSvgFontEngine &&engine) noexcept
  : d(std::move(engine.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontEngine::operator=(VSvgFontEngine &&engine) noexcept -> VSvgFontEngine &
{
    std::swap(d, engine.d);
    return *this;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
void VSvgFontEngine::AddGlyph(QChar unicode, const QPainterPath &path, qreal horizAdvX)
{
    if (unicode == QChar(0x042B) || unicode == QChar(0x044B) || unicode == QChar(0x042A) || unicode == QChar(0x044A) ||
        unicode == QChar(0x0401) || unicode == QChar(0x0451) || unicode == QChar(0x042D) || unicode == QChar(0x044D))
    {
        return;
    }

    d->m_glyphs.insert(
        unicode,
        VSvgGlyph(unicode, path, (VFuzzyComparePossibleNulls(horizAdvX, -1)) ? d->m_font.HorizAdvX() : horizAdvX));
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontEngine::DrawPath(const QPointF &point, const QString &str) const -> QPainterPath
{
    qreal pixelSize = FontPixelSize();

    if (d->m_font.UnitsPerEm() <= 0 || pixelSize <= 0)
    {
        return {};
    }

    const qreal baseLine = FromFontUnits(d->m_font.Ascent());

    QTransform matrix;
    matrix.translate(point.x(), point.y() + baseLine);
    matrix.scale(pixelSize / d->m_font.UnitsPerEm(), -pixelSize / d->m_font.UnitsPerEm());

    QPainterPath path;
    for (auto unicode : str)
    {
        if (!d->m_glyphs.contains(unicode))
        {
            unicode = u'\0';
            if (!d->m_glyphs.contains(unicode))
            {
                continue;
            }
        }
        path.addPath(matrix.map(d->m_glyphs[unicode].Path()));
        matrix.translate(d->m_glyphs[unicode].HorizAdvX(), 0);
    }

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFontEngine::Draw(QPainter *p, const QPointF &point, const QString &str) const
{
    int pixelSize = FontPixelSize();

    if (d->m_font.UnitsPerEm() <= 0 || pixelSize <= 0)
    {
        return;
    }

    const qreal baseLine = FromFontUnits(d->m_font.Ascent());

    p->save();
    p->translate(point.x(), point.y() + baseLine);
    p->scale(pixelSize / d->m_font.UnitsPerEm(), -pixelSize / d->m_font.UnitsPerEm());

    // since in SVG the embedded font ain't really a path
    // the outline has got to stay untransformed...
    qreal penWidth = p->pen().widthF();
    penWidth /= (pixelSize / d->m_font.UnitsPerEm());
    QPen pen = p->pen();
    pen.setWidthF(penWidth);
    p->setPen(pen);
    for (auto unicode : str)
    {
        if (!d->m_glyphs.contains(unicode))
        {
            unicode = u'\0';
            if (!d->m_glyphs.contains(unicode))
            {
                continue;
            }
        }
        p->drawPath(d->m_glyphs[unicode].Path());
        p->translate(d->m_glyphs[unicode].HorizAdvX(), 0);
    }
    p->restore();
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFontEngine::Draw(QPainter *p, const QRectF &rect, const QString &str, Qt::Alignment alignment) const
{
    SCASSERT(p != nullptr)

    int pixelSize = FontPixelSize();

    if (d->m_font.UnitsPerEm() <= 0 || pixelSize <= 0)
    {
        return;
    }

    const qreal baseLine = FromFontUnits(d->m_font.Ascent());

    p->save();
    p->setClipRect(rect);
    p->translate(rect.x(), rect.y() + baseLine + p->pen().widthF());
    p->scale(pixelSize / d->m_font.UnitsPerEm(), -pixelSize / d->m_font.UnitsPerEm());

    // Calculate the text width to be used for alignment
    QPoint alignmentOffset(0, 0);
    if (alignment == Qt::AlignHCenter)
    {
        qreal stringWidth = ToFontUnits(rect.width());
        int textWidth = TextHorizAdvX(str);
        alignmentOffset.setX(qRound(stringWidth / 2. - textWidth / 2.));
    }
    else if (alignment == Qt::AlignRight)
    {
        qreal stringWidth = ToFontUnits(rect.width());
        int textWidth = TextHorizAdvX(str);
        alignmentOffset.setX(qRound(stringWidth - textWidth));
    }

    p->translate(alignmentOffset);

    // since in SVG the embedded font ain't really a path
    // the outline has got to stay untransformed...
    qreal penWidth = p->pen().widthF();
    penWidth /= (pixelSize / d->m_font.UnitsPerEm());
    QPen pen = p->pen();
    pen.setWidthF(penWidth);
    p->setPen(pen);
    for (auto unicode : str)
    {
        if (!d->m_glyphs.contains(unicode))
        {
            unicode = u'\0';
            if (!d->m_glyphs.contains(unicode))
            {
                continue;
            }
        }
        p->drawPath(d->m_glyphs[unicode].Path());
        p->translate(d->m_glyphs[unicode].HorizAdvX(), 0);
    }
    p->restore();
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontEngine::TextHorizAdvX(const QString &str) const -> int
{
    int textWidth = 0;
    for (auto unicode : str)
    {
        if (!d->m_glyphs.contains(unicode))
        {
            unicode = u'\0';
            if (!d->m_glyphs.contains(unicode))
            {
                continue;
            }
        }
        textWidth += static_cast<int>(d->m_glyphs[unicode].HorizAdvX());
    }

    return textWidth;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontEngine::TextWidth(const QString &str, qreal penWidth) const -> qreal
{
    return FromFontUnits(TextHorizAdvX(str) + static_cast<int>(str.length()) * penWidth);
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontEngine::InFont(QChar ch) const -> bool
{
    return d->m_glyphs.contains(ch);
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontEngine::CanRender(const QString &str) const -> bool
{
    return std::all_of(str.cbegin(), str.cend(), [this](auto c) { return this->InFont(c); });
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontEngine::FontSample() const -> QString
{
    QString sample;

    if (CanRender(d->m_font.FamilyName()))
    {
        sample = d->m_font.FamilyName();
    }

    QList<SVGFontWritingSystem> writingSystems = d->m_font.WritingSystems();

    for (auto system : writingSystems)
    {
        QString systemSample = WritingSystemSample(system);
        if (CanRender(systemSample))
        {
            if (sample.isEmpty())
            {
                sample = systemSample;
            }
            else
            {
                sample += QStringLiteral(" %1").arg(WritingSystemSample(system));
            }
        }
    }

    return sample;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFontEngine::ElidedText(const QString &text, SVGTextElideMode mode, int width, qreal penWidth) const -> QString
{
    // Check if the text within the width
    if (TextWidth(text) <= width || mode == SVGTextElideMode::ElideNone)
    {
        return text;
    }

    QString elidedText = text;

    QString ellipsis;
    {
        // We only want to use the ellipsis character if it supported by font.
        QChar ellipsisChar = u'\x2026';
        if (InFont(ellipsisChar))
        {
            ellipsis = ellipsisChar;
        }
        else
        {
            ellipsis = "..."_L1;
        }
    }

    // Calculate the available width for the elided text
    const qreal availableWidth = width - TextWidth(ellipsis, penWidth);

    if (mode == SVGTextElideMode::ElideLeft)
    {
        // Iterate through the text from left to right, removing characters until it fits within the available width
        while (TextWidth(elidedText, penWidth) > availableWidth)
        {
            elidedText = elidedText.mid(1);
        }
        elidedText.prepend(ellipsis);
    }
    else if (mode == SVGTextElideMode::ElideRight)
    {
        // Iterate through the text from right to left, removing characters until it fits within the available width
        while (TextWidth(elidedText, penWidth) > availableWidth)
        {
            elidedText = elidedText.left(elidedText.length() - 1);
        }
        elidedText.append(ellipsis);
    }
    else if (mode == SVGTextElideMode::ElideMiddle)
    {
        // Perform middle ellipsis by splitting the text and keeping the first and last characters
        qreal remainingWidth = availableWidth;
        int leftIndex = 1;
        int rightIndex = static_cast<int>(elidedText.length() - 2);

        while (leftIndex < rightIndex && remainingWidth > 0)
        {
            qreal leftAdvance = TextWidth(elidedText.at(leftIndex), penWidth);
            qreal rightAdvance = TextWidth(elidedText.at(rightIndex), penWidth);

            if (leftAdvance <= rightAdvance && remainingWidth >= leftAdvance)
            {
                remainingWidth -= leftAdvance;
                ++leftIndex;
            }
            else if (rightAdvance <= leftAdvance && remainingWidth >= rightAdvance)
            {
                remainingWidth -= rightAdvance;
                --rightIndex;
            }
            else
            {
                break;
            }
        }

        if (leftIndex < rightIndex)
        {
            elidedText = elidedText.left(leftIndex) + ellipsis + elidedText.right(elidedText.length() - rightIndex - 1);
        }
    }

    return elidedText;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFontEngine::RecalculateFontSize()
{
    int pixelSize = 100;
    QTransform matrix;
    matrix.translate(0, FromFontUnits(d->m_font.Ascent(), pixelSize));
    matrix.scale(pixelSize / d->m_font.UnitsPerEm(), -pixelSize / d->m_font.UnitsPerEm());

    QRectF standard(0, 0, pixelSize, pixelSize);
    qreal top = standard.top();
    qreal bottom = standard.bottom();

    QHash<QChar, VSvgGlyph>::const_iterator i = d->m_glyphs.constBegin();
    while (i != d->m_glyphs.constEnd())
    {
        QPainterPath path = i.value().Path();
        QChar c = i.key();
        if (path.isEmpty() || c == u'\0')
        {
            ++i;
            continue;
        }

        QRectF glyphBoundingRect = matrix.map(i.value().Path()).boundingRect();
        glyphBoundingRect = standard.united(glyphBoundingRect);

        if (top - glyphBoundingRect.top() > 0)
        {
            top = glyphBoundingRect.top();
        }

        if (glyphBoundingRect.bottom() - bottom > 0)
        {
            bottom = glyphBoundingRect.bottom();
        }

        ++i;
    }

    if (!VFuzzyComparePossibleNulls(top, standard.top()) || !VFuzzyComparePossibleNulls(bottom, standard.bottom()))
    {
        qreal ascent = d->m_font.Ascent();
        if (!VFuzzyComparePossibleNulls(top, standard.top()))
        {
            ascent += qAbs(ToFontUnits(top, pixelSize));
            d->m_font.SetAscent(ascent);
        }

        qreal descent = d->m_font.Descent();
        if (!VFuzzyComparePossibleNulls(bottom, standard.bottom()))
        {
            descent -= ToFontUnits(bottom - standard.bottom(), pixelSize);
            d->m_font.SetDescent(descent);
        }

        qreal units = ascent + qAbs(descent);
        d->m_font.SetUnitsPerEm(units);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFontEngine::RecognizeWritingSystems()
{
    QSet<SVGFontWritingSystem> systems;
    const QList<QChar> glyphs = d->m_glyphs.keys();
    for (auto c : glyphs)
    {
        if (c.unicode() >= 0x0030 && c.unicode() <= 0x0039)
        {
            // Numerals are common to multiple writing systems
            continue;
        }

        if (CyrillicWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Cyrillic);
            continue;
        }

        if (LatinWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Latin);
            continue;
        }

        if (GreekWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Greek);
            continue;
        }

        if (ArmenianWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Armenian);
            continue;
        }

        if (HebrewWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Hebrew);
            continue;
        }

        if (ArabicWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Arabic);
            continue;
        }

        if (SyriacWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Syriac);
            continue;
        }

        if (ThaanaWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Thaana);
            continue;
        }

        if (DevanagariWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Devanagari);
            continue;
        }

        if (BengaliWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Bengali);
            continue;
        }

        if (GurmukhiWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Gurmukhi);
            continue;
        }

        if (GujaratiWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Gujarati);
            continue;
        }

        if (OriyaWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Oriya);
            continue;
        }

        if (TamilWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Tamil);
            continue;
        }

        if (TeluguWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Telugu);
            continue;
        }

        if (KannadaWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Kannada);
            continue;
        }

        if (MalayalamWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Malayalam);
            continue;
        }

        if (SinhalaWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Sinhala);
            continue;
        }

        if (ThaiWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Thai);
            continue;
        }

        if (LaoWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Lao);
            continue;
        }

        if (TibetanWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Tibetan);
            continue;
        }

        if (MyanmarWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Myanmar);
            continue;
        }

        if (GeorgianWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Georgian);
            continue;
        }

        if (KhmerWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Khmer);
            continue;
        }

        if (SimplifiedChineseWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::SimplifiedChinese);
            continue;
        }

        if (TraditionalChineseWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::TraditionalChinese);
            continue;
        }

        if (JapaneseWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Japanese);
            continue;
        }

        if (KoreanWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Korean);
            continue;
        }

        if (VietnameseWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Vietnamese);
            continue;
        }

        if (SymbolWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Symbol);
            continue;
        }

        if (OghamWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Ogham);
            continue;
        }

        if (RunicWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Runic);
            continue;
        }

        if (NkoWritingSystem(c))
        {
            systems.insert(SVGFontWritingSystem::Nko);
            continue;
        }
    }

    if (systems.isEmpty())
    {
        systems.insert(SVGFontWritingSystem::Any);
    }

    d->m_font.SetWritingSystems(ConvertToList(systems));
}
