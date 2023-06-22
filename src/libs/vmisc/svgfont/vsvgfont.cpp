/************************************************************************
 **
 **  @file   vsvgfont.cpp
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
#include "vsvgfont.h"
#include "svgdef.h"
#include "vsvgfont_p.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QDataStream>
#include <QIODevice>

//---------------------------------------------------------------------------------------------------------------------
VSvgFont::VSvgFont()
  : d(new VSvgFontData())
{
}

//---------------------------------------------------------------------------------------------------------------------
VSvgFont::VSvgFont(qreal horizAdvX)
  : d(new VSvgFontData(horizAdvX))
{
}

//---------------------------------------------------------------------------------------------------------------------
VSvgFont::VSvgFont(const VSvgFont &font) // NOLINT(modernize-use-equals-default)
  : d(font.d)
{
}

//---------------------------------------------------------------------------------------------------------------------
VSvgFont::~VSvgFont() // NOLINT(modernize-use-equals-default)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFont::operator=(const VSvgFont &font) -> VSvgFont &
{
    if (&font == this)
    {
        return *this;
    }
    d = font.d;
    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VSvgFont::VSvgFont(VSvgFont &&font) noexcept
  : d(std::move(font.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFont::operator=(VSvgFont &&font) noexcept -> VSvgFont &
{
    std::swap(d, font.d);
    return *this;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
void VSvgFont::SetId(const QString &id)
{
    d->m_id = id;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFont::Id() const -> QString
{
    return d->m_id;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFont::SetFamilyName(const QString &name)
{
    d->m_familyName = name;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFont::FamilyName() const -> QString
{
    return d->m_familyName;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFont::SetName(const QString &name)
{
    d->m_name = name;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFont::Name() const -> QString
{
    if (!d->m_name.isEmpty())
    {
        return d->m_name;
    }

    return d->m_familyName;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFont::SetUnitsPerEm(qreal upem)
{
    d->m_unitsPerEm = upem;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFont::UnitsPerEm() const -> qreal
{
    return d->m_unitsPerEm;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFont::SetAscent(qreal ascent)
{
    d->m_ascent = ascent;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VSvgFont::Ascent() const
{
    return d->m_ascent;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFont::SetDescent(qreal descent)
{
    d->m_descent = descent;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VSvgFont::Descent() const
{
    return d->m_descent;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFont::SetHorizAdvX(qreal horizAdvX)
{
    d->m_horizAdvX = horizAdvX;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFont::HorizAdvX() const -> qreal
{
    return d->m_horizAdvX;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFont::SetStyle(SVGFontStyle style)
{
    d->m_style = style;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFont::Style() const -> SVGFontStyle
{
    return d->m_style;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFont::SetItalic(bool enable)
{
    d->m_style = enable ? SVGFontStyle::Italic : SVGFontStyle::Normal;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFont::Iatic() const -> bool
{
    return d->m_style == SVGFontStyle::Italic;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFont::SetBold(bool enable)
{
    d->m_weight = enable ? SVGFontWeight::Bold : SVGFontWeight::Normal;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFont::Bold() const -> bool
{
    return d->m_weight == SVGFontWeight::Bold;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFont::SetWeight(SVGFontWeight weight)
{
    d->m_weight = weight;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFont::Weight() const -> SVGFontWeight
{
    return d->m_weight;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFont::IsValid() const -> bool
{
    return d->m_horizAdvX > 0 && !d->m_id.isEmpty() && d->m_unitsPerEm > 0 && !d->m_familyName.isEmpty();
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFont::SetPriority(int priority)
{
    d->m_priority = priority;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFont::Priority() const -> int
{
    return d->m_priority;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFont::SetPath(const QString &path)
{
    d->m_path = path;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFont::Path() const -> QString
{
    return d->m_path;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFont::SetWritingSystems(const QList<SVGFontWritingSystem> &writingSystems)
{
    d->m_writingSystems = writingSystems;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFont::WritingSystems() const -> QList<SVGFontWritingSystem>
{
    return d->m_writingSystems;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFont::Hash() const -> QByteArray
{
    return VSvgFont::Hash(d->m_familyName, d->m_style, d->m_weight);
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFont::Hash(const QString &family, SVGFontStyle style, SVGFontWeight weight) -> QByteArray
{
    QString combinedString =
        family + QString::number(static_cast<int>(style)) + QString::number(static_cast<int>(weight));

    QByteArray data;

    QDataStream in(&data, QIODevice::WriteOnly);
    in << combinedString;

    QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Sha256);

    return hash;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFont::SetPixelSize(int pixelSize)
{
    if (pixelSize <= 0)
    {
        qWarning("VSvgFont::SetPixelSize: Pixel size <= 0 (%d)", pixelSize);
        return;
    }

    d->m_pixelSize = pixelSize;
    d->m_pointSize = -1;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFont::PixelSize() const -> int
{
    return static_cast<int>(d->m_pixelSize);
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFont::SetPointSizeF(qreal pointSize)
{
    if (pointSize <= 0)
    {
        qWarning("VSvgFont::SetPointSizeF: Point size <= 0 (%f), must be greater than 0", pointSize);
        return;
    }

    d->m_pointSize = pointSize;
    d->m_pixelSize = -1;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFont::PointSizeF() const -> qreal
{
    return d->m_pointSize;
}

//---------------------------------------------------------------------------------------------------------------------
void VSvgFont::SetPointSize(int pointSize)
{
    if (pointSize <= 0)
    {
        qWarning("VSvgFont::SetPointSize: Point size <= 0 (%d), must be greater than 0", pointSize);
        return;
    }

    d->m_pointSize = static_cast<qreal>(pointSize);
    d->m_pixelSize = -1;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSvgFont::PointSize() const -> int
{
    return static_cast<int>(d->m_pointSize);
}
