/************************************************************************
 **
 **  @file   vplayoutsettings.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 7, 2021
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2021 Valentina project
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
#include "vplayoutsettings.h"

#include <QComboBox>
#include <QIcon>

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetUnit(Unit unit)
{
    m_unit = unit;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetUnit() const -> Unit
{
    return m_unit;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetWarningSuperpositionOfPieces(bool state)
{
    m_warningSuperpositionOfPieces = state;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetWarningSuperpositionOfPieces() const -> bool
{
    return m_warningSuperpositionOfPieces;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetWarningPiecesOutOfBound(bool state)
{
    m_warningPiecesOutOfBound = state;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetWarningPiecesOutOfBound() const -> bool
{
    return m_warningPiecesOutOfBound;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetTitle(const QString &title)
{
    m_title = title;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetTitle() const -> QString
{
    return m_title;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetDescription(const QString &description)
{
    m_description = description;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetDescription() const -> QString
{
    return m_description;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetTilesSize(qreal width, qreal height)
{
    m_tilesSize.setWidth(width);
    m_tilesSize.setHeight(height);
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetTilesSizeConverted(qreal width, qreal height)
{
    m_tilesSize.setWidth(UnitConvertor(width, m_unit, Unit::Px));
    m_tilesSize.setHeight(UnitConvertor(height, m_unit, Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetTilesSize(const QSizeF &size)
{
    m_tilesSize = size;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetTilesSizeConverted(const QSizeF &size)
{
    m_tilesSize =
        QSizeF(UnitConvertor(size.width(), GetUnit(), Unit::Px), UnitConvertor(size.height(), GetUnit(), Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetTilesOrientation() const -> QPageLayout::Orientation
{
    return m_tilesSize.height() >= m_tilesSize.width() ? QPageLayout::Portrait : QPageLayout::Landscape;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetTilesSize() const -> QSizeF
{
    return m_tilesSize;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetTilesSize(Unit unit) const -> QSizeF
{
    return {UnitConvertor(m_tilesSize.width(), Unit::Px, unit), UnitConvertor(m_tilesSize.height(), Unit::Px, unit)};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetTilesSizeConverted() const -> QSizeF
{
    return GetTilesSize(GetUnit());
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetTilesMargins(qreal left, qreal top, qreal right, qreal bottom)
{
    m_tilesMargins.setLeft(left);
    m_tilesMargins.setTop(top);
    m_tilesMargins.setRight(right);
    m_tilesMargins.setBottom(bottom);
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetTilesMarginsConverted(qreal left, qreal top, qreal right, qreal bottom)
{
    m_tilesMargins.setLeft(UnitConvertor(left, GetUnit(), Unit::Px));
    m_tilesMargins.setTop(UnitConvertor(top, GetUnit(), Unit::Px));
    m_tilesMargins.setRight(UnitConvertor(right, GetUnit(), Unit::Px));
    m_tilesMargins.setBottom(UnitConvertor(bottom, GetUnit(), Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetTilesMargins(const QMarginsF &margins)
{
    m_tilesMargins = margins;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetTilesMarginsConverted(const QMarginsF &margins)
{
    m_tilesMargins = UnitConvertor(margins, GetUnit(), Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetTilesMargins() const -> QMarginsF
{
    return m_tilesMargins;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetTilesMargins(Unit unit) const -> QMarginsF
{
    return UnitConvertor(m_tilesMargins, Unit::Px, unit);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetTilesMarginsConverted() const -> QMarginsF
{
    return UnitConvertor(m_tilesMargins, Unit::Px, GetUnit());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetShowTiles() const -> bool
{
    return m_showTiles;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetShowTiles(bool value)
{
    m_showTiles = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetFollowGrainline(bool state)
{
    m_followGrainLine = state;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetFollowGrainline() const -> bool
{
    return m_followGrainLine;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetPiecesGap(qreal value)
{
    m_piecesGap = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetPiecesGapConverted(qreal value)
{
    m_piecesGap = UnitConvertor(value, m_unit, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetPiecesGap() const -> qreal
{
    return m_piecesGap;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetPiecesGapConverted() const -> qreal
{
    return UnitConvertor(m_piecesGap, Unit::Px, m_unit);
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetStickyEdges(bool state)
{
    m_stickyEdges = state;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::IsStickyEdges() const -> bool
{
    return m_stickyEdges;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetShowGrid() const -> bool
{
    return m_showGrid;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetShowGrid(bool value)
{
    m_showGrid = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetGridColWidth() const -> qreal
{
    return m_gridColWidth;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetGridColWidthConverted() const -> qreal
{
    return UnitConvertor(m_gridColWidth, Unit::Px, m_unit);
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetGridColWidth(qreal value)
{
    m_gridColWidth = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetGridColWidthConverted(qreal value)
{
    m_gridColWidth = UnitConvertor(value, m_unit, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetGridRowHeight() const -> qreal
{
    return m_gridRowHeight;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetGridRowHeightConverted() const -> qreal
{
    return UnitConvertor(m_gridRowHeight, Unit::Px, m_unit);
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetGridRowHeight(qreal value)
{
    m_gridRowHeight = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetGridRowHeightConverted(qreal value)
{
    m_gridRowHeight = UnitConvertor(value, m_unit, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::IgnoreTilesMargins() const -> bool
{
    return m_ignoreTilesMargins;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetIgnoreTilesMargins(bool newIgnoreTilesMargins)
{
    m_ignoreTilesMargins = newIgnoreTilesMargins;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::HorizontalScale() const -> qreal
{
    return m_horizontalScale;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetHorizontalScale(qreal newHorizontalScale)
{
    m_horizontalScale = newHorizontalScale;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::VerticalScale() const -> qreal
{
    return m_verticalScale;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetVerticalScale(qreal newVerticalScale)
{
    m_verticalScale = newVerticalScale;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::WatermarkPath() const -> const QString &
{
    return m_watermarkPath;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetWatermarkPath(const QString &newWatermarkPath)
{
    m_watermarkPath = newWatermarkPath;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetShowWatermark() const -> bool
{
    return m_showWatermark;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetShowWatermark(bool newShowWatermark)
{
    m_showWatermark = newShowWatermark;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetPrintTilesScheme() const -> bool
{
    return m_printTilesScheme;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetPrintTilesScheme(bool newPrintTilesScheme)
{
    m_printTilesScheme = newPrintTilesScheme;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::GetShowTileNumber() const -> bool
{
    return m_showTileNumbers;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetShowTileNumber(bool newTileNumbers)
{
    m_showTileNumbers = newTileNumbers;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetBoundaryTogetherWithNotches(bool value)
{
    m_togetherWithNotches = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::IsBoundaryTogetherWithNotches() const -> bool
{
    return m_togetherWithNotches;
}

//---------------------------------------------------------------------------------------------------------------------
void VPLayoutSettings::SetCutOnFold(bool value)
{
    m_cutOnFold = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPLayoutSettings::IsCutOnFold() const -> bool
{
    return m_cutOnFold;
}
