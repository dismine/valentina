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
    m_layoutSize(QSizeF()),
    m_layoutMargins(QMarginsF()),
    m_followGrainLine(FollowGrainline::No),
    m_piecesGap(0),
    m_warningSuperpositionOfPieces(false),
    m_warningPiecesOutOfBound(false),
    m_stickyEdges(false)
{
    m_piecesGap = 0;
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
    m_layoutUnit = unit;
}

//---------------------------------------------------------------------------------------------------------------------
Unit VPuzzleLayout::getUnit()
{
    return m_layoutUnit;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetLayoutSize(qreal width, qreal height)
{
    m_layoutSize.setWidth(width);
    m_layoutSize.setHeight(height);
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetLayoutSize(QSizeF size)
{
    m_layoutSize = size;
}

//---------------------------------------------------------------------------------------------------------------------
QSizeF VPuzzleLayout::GetLayoutSize()
{
    return m_layoutSize;
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetLayoutMargins(qreal left, qreal top, qreal right, qreal bottom)
{
    m_layoutMargins.setLeft(left);
    m_layoutMargins.setTop(top);
    m_layoutMargins.setRight(right);
    m_layoutMargins.setRight(bottom);
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleLayout::SetLayoutMargins(QMarginsF margins)
{
    m_layoutMargins = margins;
}

//---------------------------------------------------------------------------------------------------------------------
QMarginsF VPuzzleLayout::GetLayoutMargins()
{
    return m_layoutMargins;
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
qreal VPuzzleLayout::GetPiecesGap()
{
    return m_piecesGap;
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
