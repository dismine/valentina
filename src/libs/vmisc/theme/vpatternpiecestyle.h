/************************************************************************
 **
 **  @file   vpatternpiecestyle.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 7, 2023
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
#ifndef VPATTERNPIECESTYLE_H
#define VPATTERNPIECESTYLE_H

#include "vstylesheetstyle.h"

#include <QColor>

class VPatternPieceStyle : public VStylesheetStyle
{
public:
    VPatternPieceStyle();

    ~VPatternPieceStyle() override = default;

    VPatternPieceStyle(const VPatternPieceStyle &) = default;
    auto operator=(const VPatternPieceStyle &) -> VPatternPieceStyle & = default;

#ifdef Q_COMPILER_RVALUE_REFS
    VPatternPieceStyle(VPatternPieceStyle &&) = default;
    auto operator=(VPatternPieceStyle &&) -> VPatternPieceStyle & = default;
#endif

    void LoadJson(QJsonObject const &json) override;

    auto ToJson() const -> QJsonObject override;

    auto PieceColor() const -> QColor;
    auto PointColor() const -> QColor;
    auto NodeLabelColor() const -> QColor;
    auto NodeLabelHoverColor() const -> QColor;
    auto NodeLabelLineColor() const -> QColor;
    auto LabelBackgroundColor() const -> QColor;
    auto LabelTextColor() const -> QColor;
    auto LabelModeColor() const -> QColor;

private:
    QColor m_pieceColor{};
    QColor m_pointColor{};
    QColor m_nodeLabelColor{};
    QColor m_nodeLabelHoverColor{};
    QColor m_nodeLabelLineColor{};
    QColor m_labelBackgroundColor{};
    QColor m_labelTextColor{};
    QColor m_labelModeColor{};
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VPatternPieceStyle::PieceColor() const -> QColor
{
    return m_pieceColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPatternPieceStyle::PointColor() const -> QColor
{
    return m_pointColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPatternPieceStyle::NodeLabelColor() const -> QColor
{
    return m_nodeLabelColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPatternPieceStyle::NodeLabelHoverColor() const -> QColor
{
    return m_nodeLabelHoverColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPatternPieceStyle::NodeLabelLineColor() const -> QColor
{
    return m_nodeLabelLineColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPatternPieceStyle::LabelBackgroundColor() const -> QColor
{
    return m_labelBackgroundColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPatternPieceStyle::LabelTextColor() const -> QColor
{
    return m_labelTextColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VPatternPieceStyle::LabelModeColor() const -> QColor
{
    return m_labelModeColor;
}

#endif // VPATTERNPIECESTYLE_H
