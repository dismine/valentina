/************************************************************************
 **
 **  @file   vsvgglyph_p.h
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
#ifndef VSVGGLYPH_P_H
#define VSVGGLYPH_P_H

#include <QChar>
#include <QPainterPath>
#include <QSharedData>
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../defglobal.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VSvgGlyphData final : public QSharedData
{
public:
    VSvgGlyphData() = default;
    VSvgGlyphData(QChar unicode, const QPainterPath &path, qreal horizAdvX);
    VSvgGlyphData(const VSvgGlyphData &glyph) = default;
    ~VSvgGlyphData() = default;

    QChar m_unicode{0};    // NOLINT(misc-non-private-member-variables-in-classes)
    QPainterPath m_path{}; // NOLINT(misc-non-private-member-variables-in-classes)
    qreal m_horizAdvX{0};  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VSvgGlyphData) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
inline VSvgGlyphData::VSvgGlyphData(QChar unicode, const QPainterPath &path, qreal horizAdvX)
  : m_unicode(unicode),
    m_path(path),
    m_horizAdvX(horizAdvX)
{
}

QT_WARNING_POP

#endif // VSVGGLYPH_P_H
