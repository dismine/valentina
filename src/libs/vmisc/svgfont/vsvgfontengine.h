/************************************************************************
 **
 **  @file   vsvgfontengine.h
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
#ifndef VSVGFONTENGINE_H
#define VSVGFONTENGINE_H

#include "vsvgfont.h"
#include <QSharedDataPointer>

class VSvgFontEngineData;
class QPainterPath;
class QPainter;
class QPointF;

class VSvgFontEngine
{
public:
    VSvgFontEngine();
    explicit VSvgFontEngine(const VSvgFont &font);
    VSvgFontEngine(const VSvgFontEngine &engine);
    ~VSvgFontEngine();

    auto operator=(const VSvgFontEngine &engine) -> VSvgFontEngine &;

    VSvgFontEngine(VSvgFontEngine &&engine) noexcept;
    auto operator=(VSvgFontEngine &&engine) noexcept -> VSvgFontEngine &;

    auto Font() const -> VSvgFont;
    void SetFont(const VSvgFont &fnt);

    void SetFontPixelSize(int pixelSize);
    void SetFontPointSizeF(qreal pointSize);
    void SetFontPointSize(int pointSize);

    auto FontPixelSize() const -> int;
    auto FontHeight() const -> qreal;

    auto FromFontUnits(qreal val, int pixelSize = -1) const -> qreal;
    auto ToFontUnits(qreal val, int pixelSize = -1) const -> qreal;

    void AddGlyph(QChar unicode, const QPainterPath &path, qreal horizAdvX = -1);

    auto DrawPath(const QPointF &point, const QString &str, qreal penWidth = 0) const -> QPainterPath;

    void Draw(QPainter *p, const QPointF &point, const QString &str) const;
    void Draw(QPainter *p, const QRectF &rect, const QString &str, Qt::Alignment alignment = Qt::AlignLeft) const;

    auto TextWidth(const QString &str, qreal penWidth = 0) const -> qreal;

    auto BoundingRect(const QString &str, qreal penWidth = 0) const -> QRectF;

    auto InFont(QChar ch) const -> bool;
    auto CanRender(const QString &str) const -> bool;

    auto FontSample() const -> QString;

    auto ElidedText(const QString &text, SVGTextElideMode mode, int width, qreal penWidth = 0) const -> QString;

    void RecalculateFontSize();
    void RecognizeWritingSystems();

private:
    QSharedDataPointer<VSvgFontEngineData> d;

    auto TextHorizAdvX(const QString &str) const -> int;
};

Q_DECLARE_TYPEINFO(VSvgFontEngine, Q_MOVABLE_TYPE); // NOLINT

#endif // VSVGFONTENGINE_H
