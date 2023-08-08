/************************************************************************
 **
 **  @file   vscenestylesheet.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   24 7, 2023
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
#include "vscenestylesheet.h"

#include "../def.h"
#include "vmanuallayoutstyle.h"
#include "vtheme.h"

#include <QColor>
#include <QGraphicsItem>

//---------------------------------------------------------------------------------------------------------------------
auto VSceneStylesheet::Instance() -> VSceneStylesheet &
{
    static VSceneStylesheet stylesheet;

    return stylesheet;
}

//---------------------------------------------------------------------------------------------------------------------
void VSceneStylesheet::ResetStyles()
{
    SetToolStyle(VToolStyle());
    SetPatternPieceStyle(VPatternPieceStyle());
    SetManualLayoutStyle(VManualLayoutStyle());
}

//---------------------------------------------------------------------------------------------------------------------
auto VSceneStylesheet::ToolStyle() -> const VToolStyle &
{
    return Instance().m_toolStyle;
}

//---------------------------------------------------------------------------------------------------------------------
void VSceneStylesheet::SetToolStyle(const VToolStyle &style)
{
    Instance().m_toolStyle = style;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSceneStylesheet::PatternPieceStyle() -> const VPatternPieceStyle &
{
    return Instance().m_patternPieceStyle;
}

//---------------------------------------------------------------------------------------------------------------------
void VSceneStylesheet::SetPatternPieceStyle(const VPatternPieceStyle &style)
{
    Instance().m_patternPieceStyle = style;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSceneStylesheet::ManualLayoutStyle() -> const VManualLayoutStyle &
{
    return Instance().m_manualLayoutStyle;
}

//---------------------------------------------------------------------------------------------------------------------
void VSceneStylesheet::SetManualLayoutStyle(const VManualLayoutStyle &style)
{
    Instance().m_manualLayoutStyle = style;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSceneStylesheet::Color(VColorRole role) -> QColor
{
    switch (role)
    {
        case (VColorRole::PatternColor):
            return ToolStyle().PatternColor();
        case (VColorRole::DraftPointColor):
            return ToolStyle().PointColor();
        case (VColorRole::DraftLabelColor):
            return ToolStyle().LabelColor();
        case (VColorRole::DraftLabelHoverColor):
            return ToolStyle().LabelHoverColor();
        case (VColorRole::DraftLabelLineColor):
            return ToolStyle().LabelLineColor();
        case (VColorRole::ControlLineColor):
            return ToolStyle().ControlLineColor();
        case (VColorRole::ControlPointColor):
            return ToolStyle().ControlPointColor();
        case (VColorRole::AccuracyRadiusColor):
            return ToolStyle().AccuracyRadiusColor();
        case (VColorRole::VisMainColor):
            return ToolStyle().VisMainColor();
        case (VColorRole::VisSupportColor):
            return ToolStyle().VisSupportColor();
        case (VColorRole::VisSupportColor2):
            return ToolStyle().VisSupportColor2();
        case (VColorRole::VisSupportColor3):
            return ToolStyle().VisSupportColor3();
        case (VColorRole::VisSupportColor4):
            return ToolStyle().VisSupportColor4();
        case (VColorRole::BasePointColor):
            return ToolStyle().BasePointColor();
        case (VColorRole::PieceColor):
            return PatternPieceStyle().PieceColor();
        case (VColorRole::PiecePointColor):
            return PatternPieceStyle().PointColor();
        case (VColorRole::PieceNodeLabelColor):
            return PatternPieceStyle().NodeLabelColor();
        case (VColorRole::PieceNodeLabelHoverColor):
            return PatternPieceStyle().NodeLabelHoverColor();
        case (VColorRole::PieceNodeLabelLineColor):
            return PatternPieceStyle().NodeLabelLineColor();
        case (VColorRole::DefaultColor):
        default:
            return Qt::black;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VSceneStylesheet::CorrectToolColor(const QGraphicsItem *item, const QColor &color) -> QColor
{
    SCASSERT(item != nullptr)

    if (item->isEnabled())
    {
        return color;
    }

    QColor disabled = ToolStyle().DisabledColor();
    disabled.setAlpha(color.alpha());
    return disabled;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSceneStylesheet::CorrectToolColorForDarkTheme(const QColor &color) -> QColor
{
    if (VTheme::ColorSheme() != VColorSheme::Dark)
    {
        return color;
    }

    QMap<QString, QString> adjustments = ToolStyle().ColorAdjustments();

    QString colorName = color.name(QColor::HexArgb);
    if (adjustments.contains(colorName))
    {
        QColor c = adjustments.value(color.name(QColor::HexArgb));
        colorName = c.name(QColor::HexArgb);
        return c;
    }

    return color.lighter(150);
}
