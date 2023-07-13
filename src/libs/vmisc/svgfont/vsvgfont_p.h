/************************************************************************
 **
 **  @file   vsvgfont_p.h
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
#ifndef VSVGFONT_P_H
#define VSVGFONT_P_H

#include <QList>
#include <QSharedData>
#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../defglobal.h"
#include "svgdef.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VSvgFontData final : public QSharedData
{
public:
    VSvgFontData() = default;
    explicit VSvgFontData(qreal horizAdvX);
    VSvgFontData(const VSvgFontData &font) = default;
    ~VSvgFontData() = default;

    QString m_id{};                                // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_familyName{};                        // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_name{};                              // NOLINT(misc-non-private-member-variables-in-classes)
    qreal m_unitsPerEm{0};                         // NOLINT(misc-non-private-member-variables-in-classes)
    qreal m_ascent{0};                             // NOLINT(misc-non-private-member-variables-in-classes)
    qreal m_descent{0};                            // NOLINT(misc-non-private-member-variables-in-classes)
    qreal m_horizAdvX{0};                          // NOLINT(misc-non-private-member-variables-in-classes)
    SVGFontStyle m_style{SVGFontStyle::Normal};    // NOLINT(misc-non-private-member-variables-in-classes)
    SVGFontWeight m_weight{SVGFontWeight::Normal}; // NOLINT(misc-non-private-member-variables-in-classes)
    int m_priority{-1};                            // NOLINT(misc-non-private-member-variables-in-classes)
    QString m_path{};                              // NOLINT(misc-non-private-member-variables-in-classes)
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes)
    QList<SVGFontWritingSystem> m_writingSystems{SVGFontWritingSystem::Any};
    qreal m_pointSize{-1}; // NOLINT(misc-non-private-member-variables-in-classes)
    qreal m_pixelSize{-1}; // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VSvgFontData) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
inline VSvgFontData::VSvgFontData(qreal horizAdvX)
  : m_horizAdvX(horizAdvX)
{
}

QT_WARNING_POP
#endif // VSVGFONT_P_H
