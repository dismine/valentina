/************************************************************************
 **
 **  @file   literals.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 4, 2020
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
#ifndef VPLAYOUTLITERALS_H
#define VPLAYOUTLITERALS_H

#include <QString>

// All tags and attributes present in manual layout file format

namespace ML
{
extern const QString TagLayout;
extern const QString TagProperties;
extern const QString TagLayers;
extern const QString TagUnit;
extern const QString TagDescription;
extern const QString TagSize;
extern const QString TagMargin;
extern const QString TagControl;
extern const QString TagTiles;
extern const QString TagUnplacedPiecesLayer;
extern const QString TagLayer;
extern const QString TagPiece;

extern const QString AttrVersion;
extern const QString AttrWarningSuperposition;
extern const QString AttrWarningOutOfBound;
extern const QString AttrStickyEdges;
extern const QString AttrPiecesGap;
extern const QString AttrVisible;
extern const QString AttrMatchingMarks;
extern const QString AttrName;
extern const QString AttrLeft;
extern const QString AttrTop;
extern const QString AttrRight;
extern const QString AttrBottom;
extern const QString AttrWidth;
extern const QString AttrLength;
extern const QString AttrFollowGrainLine;
extern const QString AttrID;
extern const QString AttrMirrored;
extern const QString AttrTransform;
extern const QString AttrShowSeamline;
}


#endif // VPLAYOUTLITERALS_H
