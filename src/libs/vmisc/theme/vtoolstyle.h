/************************************************************************
 **
 **  @file   vtoolstyle.h
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
#ifndef VTOOLSTYLE_H
#define VTOOLSTYLE_H

#include "vstylesheetstyle.h"

#include <QColor>
#include <QMap>

class VToolStyle : public VStylesheetStyle
{
public:
    VToolStyle();

    ~VToolStyle() override = default;

    VToolStyle(const VToolStyle &) = default;
    auto operator=(const VToolStyle &) -> VToolStyle & = default;

#ifdef Q_COMPILER_RVALUE_REFS
    VToolStyle(VToolStyle &&) = default;
    auto operator=(VToolStyle &&) -> VToolStyle & = default;
#endif

    void LoadJson(QJsonObject const &json) override;

    auto ToJson() const -> QJsonObject override;

    auto ColorAdjustments() const -> QMap<QString, QString>;

    auto PatternColor() const -> QColor;
    auto DisabledColor() const -> QColor;
    auto PointColor() const -> QColor;
    auto LabelColor() const -> QColor;
    auto LabelHoverColor() const -> QColor;
    auto LabelLineColor() const -> QColor;
    auto AccuracyRadiusColor() const -> QColor;
    auto ControlLineColor() const -> QColor;
    auto ControlPointColor() const -> QColor;
    auto VisMainColor() const -> QColor;
    auto VisSupportColor() const -> QColor;
    auto VisSupportColor2() const -> QColor;
    auto VisSupportColor3() const -> QColor;
    auto VisSupportColor4() const -> QColor;
    auto BasePointColor() const -> QColor;

private:
    QColor m_patternColor{};
    QColor m_disabledColor{};
    QColor m_pointColor{};
    QColor m_labelColor{};
    QColor m_labelHoverColor{};
    QColor m_labelLineColor{};
    QColor m_controlLineColor{};
    QColor m_controlPointColor{};
    QColor m_accuracyRadiusColor{};
    QColor m_visMainColor{};
    QColor m_visSupportColor{};
    QColor m_visSupportColor2{};
    QColor m_visSupportColor3{};
    QColor m_visSupportColor4{};
    QColor m_basePointColor{};
    QMap<QString, QString> m_colorAdjustments{};
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolStyle::PatternColor() const -> QColor
{
    return m_patternColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolStyle::DisabledColor() const -> QColor
{
    return m_disabledColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolStyle::ColorAdjustments() const -> QMap<QString, QString>
{
    return m_colorAdjustments;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolStyle::PointColor() const -> QColor
{
    return m_pointColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolStyle::LabelColor() const -> QColor
{
    return m_labelColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolStyle::LabelHoverColor() const -> QColor
{
    return m_labelHoverColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolStyle::LabelLineColor() const -> QColor
{
    return m_labelLineColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolStyle::AccuracyRadiusColor() const -> QColor
{
    return m_accuracyRadiusColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolStyle::ControlLineColor() const -> QColor
{
    return m_controlLineColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolStyle::ControlPointColor() const -> QColor
{
    return m_controlPointColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolStyle::VisMainColor() const -> QColor
{
    return m_visMainColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolStyle::VisSupportColor() const -> QColor
{
    return m_visSupportColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolStyle::VisSupportColor2() const -> QColor
{
    return m_visSupportColor2;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolStyle::VisSupportColor3() const -> QColor
{
    return m_visSupportColor3;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolStyle::VisSupportColor4() const -> QColor
{
    return m_visSupportColor4;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VToolStyle::BasePointColor() const -> QColor
{
    return m_basePointColor;
}

#endif // VTOOLSTYLE_H
