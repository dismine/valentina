/************************************************************************
 **
 **  @file   vmanuallayoutstyle.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 7, 2023
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
#ifndef VMANUALLAYOUTSTYLE_H
#define VMANUALLAYOUTSTYLE_H

#include "vstylesheetstyle.h"

#include <QColor>

class VManualLayoutStyle : public VStylesheetStyle
{
public:
    VManualLayoutStyle();

    ~VManualLayoutStyle() override = default;

    VManualLayoutStyle(const VManualLayoutStyle &) = default;
    auto operator=(const VManualLayoutStyle &) -> VManualLayoutStyle & = default;

#ifdef Q_COMPILER_RVALUE_REFS
    VManualLayoutStyle(VManualLayoutStyle &&) = default;
    auto operator=(VManualLayoutStyle &&) -> VManualLayoutStyle & = default;
#endif

    void LoadJson(QJsonObject const &json) override;

    auto ToJson() const -> QJsonObject override;

    auto SheetBorderColor() const -> QColor;
    auto SheetFoldShadowColor() const -> QColor;
    auto SheetMarginColor() const -> QColor;
    auto SheetGridColor() const -> QColor;
    auto SheetTileGridColor() const -> QColor;
    auto SheetTileNumberColor() const -> QColor;
    auto PieceOkColor() const -> QColor;
    auto PieceErrorColor() const -> QColor;
    auto PieceHoverColor() const -> QColor;
    auto PieceSelectionBrushColor() const -> QColor;
    auto PieceHandleColor() const -> QColor;
    auto PieceHandleHoverColor() const -> QColor;
    auto CarrouselPieceColor() const -> QColor;
    auto CarrouselPieceSelectedColor() const -> QColor;
    auto CarrouselPieceBackgroundColor() const -> QColor;
    auto CarrouselPieceForegroundColor() const -> QColor;

private:
    QColor m_sheetBorderColor{};
    QColor m_sheetFoldShadowColor{};
    QColor m_sheetMarginColor{};
    QColor m_sheetGridColor{};
    QColor m_sheetTileGridColor{};
    QColor m_sheetTileNumberColor{};
    QColor m_pieceOkColor{};
    QColor m_pieceErrorColor{};
    QColor m_pieceHoverColor{};
    QColor m_pieceSelectionBrushColor{};
    QColor m_pieceHandleColor{};
    QColor m_pieceHandleHoverColor{};
    QColor m_carrouselPieceColor{};
    QColor m_carrouselPieceSelectedColor{};
    QColor m_carrouselPieceBackgroundColor{};
    QColor m_carrouselPieceForegroundColor{};
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VManualLayoutStyle::SheetBorderColor() const -> QColor
{
    return m_sheetBorderColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VManualLayoutStyle::SheetFoldShadowColor() const -> QColor
{
    return m_sheetFoldShadowColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VManualLayoutStyle::SheetMarginColor() const -> QColor
{
    return m_sheetMarginColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VManualLayoutStyle::SheetGridColor() const -> QColor
{
    return m_sheetGridColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VManualLayoutStyle::SheetTileGridColor() const -> QColor
{
    return m_sheetTileGridColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VManualLayoutStyle::SheetTileNumberColor() const -> QColor
{
    return m_sheetTileNumberColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VManualLayoutStyle::PieceOkColor() const -> QColor
{
    return m_pieceOkColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VManualLayoutStyle::PieceErrorColor() const -> QColor
{
    return m_pieceErrorColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VManualLayoutStyle::PieceHoverColor() const -> QColor
{
    return m_pieceHoverColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VManualLayoutStyle::PieceSelectionBrushColor() const -> QColor
{
    return m_pieceSelectionBrushColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VManualLayoutStyle::PieceHandleColor() const -> QColor
{
    return m_pieceHandleColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VManualLayoutStyle::PieceHandleHoverColor() const -> QColor
{
    return m_pieceHandleHoverColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VManualLayoutStyle::CarrouselPieceColor() const -> QColor
{
    return m_carrouselPieceColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VManualLayoutStyle::CarrouselPieceSelectedColor() const -> QColor
{
    return m_carrouselPieceSelectedColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VManualLayoutStyle::CarrouselPieceBackgroundColor() const -> QColor
{
    return m_carrouselPieceBackgroundColor;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VManualLayoutStyle::CarrouselPieceForegroundColor() const -> QColor
{
    return m_carrouselPieceForegroundColor;
}

#endif // VMANUALLAYOUTSTYLE_H
