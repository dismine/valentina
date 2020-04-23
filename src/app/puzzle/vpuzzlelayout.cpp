/************************************************************************
 **
 **  @file   vpuzzlelayout.cpp
 **  @author Ronan Le Tiec
 **  @date   13 4, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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
#include "vpuzzlelayout.h"

//---------------------------------------------------------------------------------------------------------------------
VPuzzleLayout::VPuzzleLayout() :
    m_unplacedPiecesLayer(new VPuzzleLayer()),
    m_layers(QList<VPuzzleLayer *>()),
    m_unit(Unit::Cm),
    m_size(QSizeF()),
    m_margins(QMarginsF()),
    m_followGrainLine(FollowGrainline::No),
    m_piecesGap(0),
    m_warningSuperpositionOfPieces(false),
    m_warningPiecesOutOfBound(false),
    m_stickyEdges(false)
{

}

//---------------------------------------------------------------------------------------------------------------------
VPuzzleLayout::~VPuzzleLayout()
{
    // TODO
}

//---------------------------------------------------------------------------------------------------------------------
VPuzzleLayer* VPuzzleLayout::GetUnplacedPiecesLayer()
{
    return m_unplacedPiecesLayer;
}

//---------------------------------------------------------------------------------------------------------------------
VPuzzleLayer* VPuzzleLayout::AddLayer()
{
    VPuzzleLayer *newLayer = new VPuzzleLayer();
    m_layers.append(newLayer);
    return newLayer;
}

//---------------------------------------------------------------------------------------------------------------------
VPuzzleLayer* VPuzzleLayout::AddLayer(VPuzzleLayer *layer)
{
    m_layers.append(layer);
    return layer;
}

//---------------------------------------------------------------------------------------------------------------------
QList<VPuzzleLayer *> VPuzzleLayout::GetLayers()
{
    return m_layers;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetUnit(Unit unit)
{
    m_unit = unit;
}

//---------------------------------------------------------------------------------------------------------------------
Unit VPuzzleLayout::getUnit()
{
    return m_unit;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetLayoutSize(qreal width, qreal height)
{
    m_size.setWidth(width);
    m_size.setHeight(height);
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetLayoutSizeConverted(qreal width, qreal height)
{
    m_size.setWidth(UnitConvertor(width, m_unit,Unit::Px));
    m_size.setHeight(UnitConvertor(height, m_unit,Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetLayoutSize(QSizeF size)
{
    m_size = size;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetLayoutSizeConverted(QSizeF size)
{
    m_size = QSizeF(
                UnitConvertor(size.width(), m_unit,Unit::Px),
                UnitConvertor(size.height(), m_unit,Unit::Px)
                );
}

//---------------------------------------------------------------------------------------------------------------------
QSizeF VPuzzleLayout::GetLayoutSize()
{
    return m_size;
}

//---------------------------------------------------------------------------------------------------------------------
QSizeF VPuzzleLayout::GetLayoutSizeConverted()
{
    QSizeF convertedSize = QSizeF(
                UnitConvertor(m_size.width(), Unit::Px, m_unit),
                UnitConvertor(m_size.height(), Unit::Px, m_unit)
                );

    return convertedSize;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetLayoutMargins(qreal left, qreal top, qreal right, qreal bottom)
{
    m_margins.setLeft(left);
    m_margins.setTop(top);
    m_margins.setRight(right);
    m_margins.setBottom(bottom);
}
//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetLayoutMarginsConverted(qreal left, qreal top, qreal right, qreal bottom)
{
    m_margins.setLeft(UnitConvertor(left, m_unit, Unit::Px));
    m_margins.setTop(UnitConvertor(top, m_unit, Unit::Px));
    m_margins.setRight(UnitConvertor(right, m_unit, Unit::Px));
    m_margins.setBottom(UnitConvertor(bottom, m_unit, Unit::Px));
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetLayoutMargins(QMarginsF margins)
{
    m_margins = margins;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetLayoutMarginsConverted(QMarginsF margins)
{
    m_margins = UnitConvertor(margins, m_unit, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF VPuzzleLayout::GetLayoutMargins()
{
    return m_margins;
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF VPuzzleLayout::GetLayoutMarginsConverted()
{
    return UnitConvertor(m_margins, Unit::Px, m_unit);
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetFollowGrainline(FollowGrainline state)
{
    m_followGrainLine = state;
}

//---------------------------------------------------------------------------------------------------------------------
FollowGrainline VPuzzleLayout::SetFollowGrainline()
{
    return m_followGrainLine;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetPiecesGap(qreal value)
{
    m_piecesGap = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetPiecesGapConverted(qreal value)
{
    m_piecesGap = UnitConvertor(value, m_unit, Unit::Px);
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPuzzleLayout::GetPiecesGap()
{
    return m_piecesGap;
}

//---------------------------------------------------------------------------------------------------------------------
qreal VPuzzleLayout::GetPiecesGapConverted()
{
    return UnitConvertor(m_piecesGap, Unit::Px, m_unit);
}


//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetWarningSuperpositionOfPieces(bool state)
{
    m_warningSuperpositionOfPieces = state;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPuzzleLayout::GetWarningSuperpositionOfPieces()
{
    return m_warningSuperpositionOfPieces;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetWarningPiecesOutOfBound(bool state)
{
    m_warningPiecesOutOfBound = state;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPuzzleLayout::GetWarningPiecesOutOfBound()
{
    return m_warningPiecesOutOfBound;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetStickyEdges(bool state)
{
    m_stickyEdges = state;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPuzzleLayout::GetStickyEdges()
{
    return m_stickyEdges;
}
