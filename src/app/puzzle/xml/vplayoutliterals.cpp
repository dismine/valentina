/************************************************************************
 **
 **  @file   vplayoutliterals.cpp
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
#include "vplayoutliterals.h"

namespace ML // Manual layout
{
const QString TagLayout              = QStringLiteral("layout");
const QString TagProperties          = QStringLiteral("properties");
const QString TagUnit                = QStringLiteral("unit");
const QString TagTitle               = QStringLiteral("title");
const QString TagDescription         = QStringLiteral("description");
const QString TagSize                = QStringLiteral("size");
const QString TagMargin              = QStringLiteral("margin");
const QString TagControl             = QStringLiteral("control");
const QString TagTiles               = QStringLiteral("tiles");
const QString TagUnplacedPieces      = QStringLiteral("unplacedPieces");
const QString TagPieces              = QStringLiteral("pieces");
const QString TagPiece               = QStringLiteral("piece");
const QString TagSheets              = QStringLiteral("sheets");
const QString TagSheet               = QStringLiteral("sheet");
const QString TagName                = QStringLiteral("name");
const QString TagSeamLine            = QStringLiteral("seamLine");
const QString TagSeamAllowance       = QStringLiteral("seamAllowance");
const QString TagGrainline           = QStringLiteral("grainline");
const QString TagNotches             = QStringLiteral("notches");
const QString TagNotch               = QStringLiteral("notch");
const QString TagInternalPaths       = QStringLiteral("internalPaths");
const QString TagInternalPath        = QStringLiteral("internalPath");
const QString TagMarkers             = QStringLiteral("markers");
const QString TagMarker              = QStringLiteral("marker");
const QString TagLabels              = QStringLiteral("labels");
const QString TagPieceLabel          = QStringLiteral("pieceLabel");
const QString TagPatternLabel        = QStringLiteral("patternLabel");
const QString TagLines               = QStringLiteral("lines");
const QString TagLine                = QStringLiteral("line");
const QString TagScale               = QStringLiteral("scale");
const QString TagWatermark           = QStringLiteral("watermark");

const QString AttrWarningSuperposition = QStringLiteral("warningSuperposition");
const QString AttrWarningOutOfBound    = QStringLiteral("warningOutOfBound");
const QString AttrStickyEdges          = QStringLiteral("stickyEdges");
const QString AttrPiecesGap            = QStringLiteral("piecesGap");
const QString AttrVisible              = QStringLiteral("visible");
const QString AttrMatchingMarks        = QStringLiteral("matchingMarks");
const QString AttrName                 = QStringLiteral("name");
const QString AttrLeft                 = QStringLiteral("left");
const QString AttrTop                  = QStringLiteral("top");
const QString AttrRight                = QStringLiteral("right");
const QString AttrBottom               = QStringLiteral("bottom");
const QString AttrWidth                = QStringLiteral("width");
const QString AttrLength               = QStringLiteral("length");
const QString AttrFollowGrainline      = QStringLiteral("followGrainline");
const QString AttrID                   = QStringLiteral("id");
const QString AttrMirrored             = QStringLiteral("mirrored");
const QString AttrTransform            = QStringLiteral("transform");
const QString AttrShowSeamline         = QStringLiteral("showSeamline");
const QString AttrEnabled              = QStringLiteral("enabled");
const QString AttrBuiltIn              = QStringLiteral("builtIn");
const QString AttrAngle                = QStringLiteral("angle");
const QString AttrArrowDirection       = QStringLiteral("arrowDirection");
const QString AttrType                 = QStringLiteral("type");
const QString AttrBaseLine             = QStringLiteral("baseLine");
const QString AttrPath                 = QStringLiteral("path");
const QString AttrCut                  = QStringLiteral("cut");
const QString AttrPenStyle             = QStringLiteral("penStyle");
const QString AttrCenter               = QStringLiteral("center");
const QString AttrBox                  = QStringLiteral("box");
const QString AttrShape                = QStringLiteral("shape");
const QString AttrFont                 = QStringLiteral("font");
const QString AttrFontSize             = QStringLiteral("fontSize");
const QString AttrBold                 = QStringLiteral("bold");
const QString AttrItalic               = QStringLiteral("italic");
const QString AttrAlignment            = QStringLiteral("alignment");
const QString AttrGradationLabel       = QStringLiteral("gradationLabel");
const QString AttrCopyNumber           = QStringLiteral("copyNumber");
const QString AttrGrainlineType        = QStringLiteral("grainlineType");
const QString AttrXScale               = QStringLiteral("xScale");
const QString AttrYScale               = QStringLiteral("yScale");
const QString AttrIgnoreMargins        = QStringLiteral("ignoreMargins");
const QString AttrShowPreview          = QStringLiteral("showPreview");
const QString AttrPrintScheme          = QStringLiteral("printScheme");
const QString AttrTileNumber           = QStringLiteral("tileNumber");

const QString atFrontStr               = QStringLiteral("atFront");
const QString atRearStr                = QStringLiteral("atRear");
const QString atBothStr                = QStringLiteral("atBoth");

const QChar groupSep                   = QLatin1Char(';');
const QChar coordintatesSep            = QLatin1Char(',');
const QChar pointsSep                  = QLatin1Char(' ');
const QChar itemsSep                   = QLatin1Char('*');
}  // namespace ML
