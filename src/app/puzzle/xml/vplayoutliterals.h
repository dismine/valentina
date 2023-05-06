/************************************************************************
 **
 **  @file   vplayoutliterals.h
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
extern const QString TagUnit;
extern const QString TagTitle;
extern const QString TagDescription;
extern const QString TagSize;
extern const QString TagMargin;
extern const QString TagControl;
extern const QString TagTiles;
extern const QString TagUnplacedPieces;
extern const QString TagPieces;
extern const QString TagPiece;
extern const QString TagSheets;
extern const QString TagSheet;
extern const QString TagName;
extern const QString TagSeamLine;
extern const QString TagSeamAllowance;
extern const QString TagGrainline;
extern const QString TagNotches;
extern const QString TagNotch;
extern const QString TagInternalPaths;
extern const QString TagInternalPath;
extern const QString TagMarkers;
extern const QString TagMarker;
extern const QString TagLabels;
extern const QString TagPieceLabel;
extern const QString TagPatternLabel;
extern const QString TagLines;
extern const QString TagLine;
extern const QString TagScale;
extern const QString TagWatermark;
extern const QString TagPoint;

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
extern const QString AttrFollowGrainline;
extern const QString AttrUID;
extern const QString AttrMirrored;
extern const QString AttrForbidFlipping;
extern const QString AttrForceFlipping;
extern const QString AttrSewLineOnDrawing;
extern const QString AttrTransform;
extern const QString AttrShowSeamline;
extern const QString AttrEnabled;
extern const QString AttrBuiltIn;
extern const QString AttrArrowDirection;
extern const QString AttrType;
extern const QString AttrBaseLine;
extern const QString AttrPath;
extern const QString AttrCut;
extern const QString AttrPenStyle;
extern const QString AttrCenter;
extern const QString AttrBox;
extern const QString AttrShape;
extern const QString AttrFont;
extern const QString AttrFontSize;
extern const QString AttrBold;
extern const QString AttrItalic;
extern const QString AttrAlignment;
extern const QString AttrGradationLabel;
extern const QString AttrCopyNumber;
extern const QString AttrGrainlineType;
extern const QString AttrXScale;
extern const QString AttrYScale;
extern const QString AttrIgnoreMargins;
extern const QString AttrShowPreview;
extern const QString AttrPrintScheme;
extern const QString AttrTileNumber;
extern const QString AttrZValue;
extern const QString AttrX;
extern const QString AttrY;
extern const QString AttrTurnPoint;
extern const QString AttrCurvePoint;
extern const QString AttrClockwiseOpening;

extern const QString oneWayUpStr;
extern const QString oneWayDownStr;
extern const QString fourWaysStr;
extern const QString twoWaysUpDownStr;
extern const QString twoWaysUpLeftStr;
extern const QString twoWaysUpRightStr;
extern const QString twoWaysDownLeftStr;
extern const QString twoWaysDownRightStr;
extern const QString threeWaysUpDownLeftStr;
extern const QString threeWaysUpDownRightStr;
extern const QString threeWaysUpLeftRightStr;
extern const QString threeWaysDownLeftRightStr;

extern const QChar groupSep;
extern const QChar coordintatesSep;
extern const QChar pointsSep;
extern const QChar itemsSep;
}  // namespace ML


#endif // VPLAYOUTLITERALS_H
