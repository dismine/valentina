/************************************************************************
 **
 **  @file   vsvgfontdatabase.h
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
#ifndef VSVGFONTDATABASE_H
#define VSVGFONTDATABASE_H

#include "vsvgfont.h"
#include "vsvgfontengine.h"
#include <QCache>
#include <QHash>
#include <QString>

class VSvgFontDatabase
{
public:
    VSvgFontDatabase() = default;

    void PopulateFontDatabase(const QString &path = QString());

    auto IsPopulated() const -> bool;

    void InvalidatePath(const QString &path);

    auto Families() const -> QStringList;

    auto Font(const QString &family, SVGFontStyle style = SVGFontStyle::Normal,
              SVGFontWeight weight = SVGFontWeight::Normal) const -> VSvgFont;

    auto FontEngine(const VSvgFont &font) const -> VSvgFontEngine;
    auto FontEngine(const QString &family, SVGFontStyle style = SVGFontStyle::Normal,
                    SVGFontWeight weight = SVGFontWeight::Normal, int pointSize = -1) const -> VSvgFontEngine;

    auto Bold(const QString &family, SVGFontStyle style) const -> bool;
    auto Italic(const QString &family, SVGFontStyle style) const -> bool;

    static auto SystemSVGFontPath() -> QString;

private:
    bool m_populated{false};
    QHash<QString, VSvgFont> m_fontDB{};
    QHash<QString, QString> m_indexFontPath{};
    QHash<QString, QList<QString>> m_indexFontFamily{};
    mutable QCache<QByteArray, VSvgFont> m_fontCache{15};
    mutable QCache<QByteArray, VSvgFontEngine> m_fontEngineCache{5};

    void UpdateIndexes();

    void ParseDirectory(const QString &path, int priority);

    void Invalidate(const QString &path);

    auto QueryFont(const QString &family, SVGFontStyle style = SVGFontStyle::Normal,
                   SVGFontWeight weight = SVGFontWeight::Normal) const -> VSvgFont;

    auto QueryFontEngine(const QString &family, SVGFontStyle style = SVGFontStyle::Normal,
                         SVGFontWeight weight = SVGFontWeight::Normal, int pointSize = -1) const -> VSvgFontEngine;
};

#endif // VSVGFONTDATABASE_H
