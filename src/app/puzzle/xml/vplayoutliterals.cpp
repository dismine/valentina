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

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace ML // Manual layout
{
const QString TagLayout = QStringLiteral("layout");                 // NOLINT(cert-err58-cpp)
const QString TagProperties = QStringLiteral("properties");         // NOLINT(cert-err58-cpp)
const QString TagUnit = QStringLiteral("unit");                     // NOLINT(cert-err58-cpp)
const QString TagTitle = QStringLiteral("title");                   // NOLINT(cert-err58-cpp)
const QString TagDescription = QStringLiteral("description");       // NOLINT(cert-err58-cpp)
const QString TagSize = QStringLiteral("size");                     // NOLINT(cert-err58-cpp)
const QString TagMargin = QStringLiteral("margin");                 // NOLINT(cert-err58-cpp)
const QString TagControl = QStringLiteral("control");               // NOLINT(cert-err58-cpp)
const QString TagTiles = QStringLiteral("tiles");                   // NOLINT(cert-err58-cpp)
const QString TagUnplacedPieces = QStringLiteral("unplacedPieces"); // NOLINT(cert-err58-cpp)
const QString TagPieces = QStringLiteral("pieces");                 // NOLINT(cert-err58-cpp)
const QString TagPiece = QStringLiteral("piece");                   // NOLINT(cert-err58-cpp)
const QString TagSheets = QStringLiteral("sheets");                 // NOLINT(cert-err58-cpp)
const QString TagSheet = QStringLiteral("sheet");                   // NOLINT(cert-err58-cpp)
const QString TagName = QStringLiteral("name");                     // NOLINT(cert-err58-cpp)
const QString TagSeamLine = QStringLiteral("seamLine");             // NOLINT(cert-err58-cpp)
const QString TagSeamAllowance = QStringLiteral("seamAllowance");   // NOLINT(cert-err58-cpp)
const QString TagGrainline = QStringLiteral("grainline");           // NOLINT(cert-err58-cpp)
const QString TagNotches = QStringLiteral("notches");               // NOLINT(cert-err58-cpp)
const QString TagNotch = QStringLiteral("notch");                   // NOLINT(cert-err58-cpp)
const QString TagInternalPaths = QStringLiteral("internalPaths");   // NOLINT(cert-err58-cpp)
const QString TagInternalPath = QStringLiteral("internalPath");     // NOLINT(cert-err58-cpp)
const QString TagMarkers = QStringLiteral("markers");               // NOLINT(cert-err58-cpp)
const QString TagMarker = QStringLiteral("marker");                 // NOLINT(cert-err58-cpp)
const QString TagLabels = QStringLiteral("labels");                 // NOLINT(cert-err58-cpp)
const QString TagPieceLabel = QStringLiteral("pieceLabel");         // NOLINT(cert-err58-cpp)
const QString TagPatternLabel = QStringLiteral("patternLabel");     // NOLINT(cert-err58-cpp)
const QString TagLines = QStringLiteral("lines");                   // NOLINT(cert-err58-cpp)
const QString TagLine = QStringLiteral("line");                     // NOLINT(cert-err58-cpp)
const QString TagScale = QStringLiteral("scale");                   // NOLINT(cert-err58-cpp)
const QString TagWatermark = QStringLiteral("watermark");           // NOLINT(cert-err58-cpp)
const QString TagPoint = QStringLiteral("point");                   // NOLINT(cert-err58-cpp)

const QString AttrWarningSuperposition = QStringLiteral("warningSuperposition"); // NOLINT(cert-err58-cpp)
const QString AttrWarningOutOfBound = QStringLiteral("warningOutOfBound");       // NOLINT(cert-err58-cpp)
const QString AttrStickyEdges = QStringLiteral("stickyEdges");                   // NOLINT(cert-err58-cpp)
const QString AttrPiecesGap = QStringLiteral("piecesGap");                       // NOLINT(cert-err58-cpp)
const QString AttrVisible = QStringLiteral("visible");                           // NOLINT(cert-err58-cpp)
const QString AttrMatchingMarks = QStringLiteral("matchingMarks");               // NOLINT(cert-err58-cpp)
const QString AttrName = QStringLiteral("name");                                 // NOLINT(cert-err58-cpp)
const QString AttrLeft = QStringLiteral("left");                                 // NOLINT(cert-err58-cpp)
const QString AttrTop = QStringLiteral("top");                                   // NOLINT(cert-err58-cpp)
const QString AttrRight = QStringLiteral("right");                               // NOLINT(cert-err58-cpp)
const QString AttrBottom = QStringLiteral("bottom");                             // NOLINT(cert-err58-cpp)
const QString AttrWidth = QStringLiteral("width");                               // NOLINT(cert-err58-cpp)
const QString AttrLength = QStringLiteral("length");                             // NOLINT(cert-err58-cpp)
const QString AttrFollowGrainline = QStringLiteral("followGrainline");           // NOLINT(cert-err58-cpp)
const QString AttrUID = QStringLiteral("uid");                                   // NOLINT(cert-err58-cpp)
const QString AttrMirrored = QStringLiteral("mirrored");                         // NOLINT(cert-err58-cpp)
const QString AttrForbidFlipping = QStringLiteral("forbidFlipping");             // NOLINT(cert-err58-cpp)
const QString AttrForceFlipping = QStringLiteral("forceFlipping");               // NOLINT(cert-err58-cpp)
const QString AttrSewLineOnDrawing = QStringLiteral("sewLineOnDrawing");         // NOLINT(cert-err58-cpp)
const QString AttrTransform = QStringLiteral("transform");                       // NOLINT(cert-err58-cpp)
const QString AttrShowSeamline = QStringLiteral("showSeamline");                 // NOLINT(cert-err58-cpp)
const QString AttrEnabled = QStringLiteral("enabled");                           // NOLINT(cert-err58-cpp)
const QString AttrBuiltIn = QStringLiteral("builtIn");                           // NOLINT(cert-err58-cpp)
const QString AttrArrowDirection = QStringLiteral("arrowDirection");             // NOLINT(cert-err58-cpp)
const QString AttrType = QStringLiteral("type");                                 // NOLINT(cert-err58-cpp)
const QString AttrBaseLine = QStringLiteral("baseLine");                         // NOLINT(cert-err58-cpp)
const QString AttrPath = QStringLiteral("path");                                 // NOLINT(cert-err58-cpp)
const QString AttrCut = QStringLiteral("cut");                                   // NOLINT(cert-err58-cpp)
const QString AttrPenStyle = QStringLiteral("penStyle");                         // NOLINT(cert-err58-cpp)
const QString AttrCenter = QStringLiteral("center");                             // NOLINT(cert-err58-cpp)
const QString AttrBox = QStringLiteral("box");                                   // NOLINT(cert-err58-cpp)
const QString AttrShape = QStringLiteral("shape");                               // NOLINT(cert-err58-cpp)
const QString AttrFont = QStringLiteral("font");                                 // NOLINT(cert-err58-cpp)
const QString AttrSVGFont = QStringLiteral("svgFont");                           // NOLINT(cert-err58-cpp)
const QString AttrFontSize = QStringLiteral("fontSize");                         // NOLINT(cert-err58-cpp)
const QString AttrBold = QStringLiteral("bold");                                 // NOLINT(cert-err58-cpp)
const QString AttrItalic = QStringLiteral("italic");                             // NOLINT(cert-err58-cpp)
const QString AttrAlignment = QStringLiteral("alignment");                       // NOLINT(cert-err58-cpp)
const QString AttrGradationLabel = QStringLiteral("gradationLabel");             // NOLINT(cert-err58-cpp)
const QString AttrCopyNumber = QStringLiteral("copyNumber");                     // NOLINT(cert-err58-cpp)
const QString AttrGrainlineType = QStringLiteral("grainlineType");               // NOLINT(cert-err58-cpp)
const QString AttrXScale = QStringLiteral("xScale");                             // NOLINT(cert-err58-cpp)
const QString AttrYScale = QStringLiteral("yScale");                             // NOLINT(cert-err58-cpp)
const QString AttrIgnoreMargins = QStringLiteral("ignoreMargins");               // NOLINT(cert-err58-cpp)
const QString AttrShowPreview = QStringLiteral("showPreview");                   // NOLINT(cert-err58-cpp)
const QString AttrPrintScheme = QStringLiteral("printScheme");                   // NOLINT(cert-err58-cpp)
const QString AttrTileNumber = QStringLiteral("tileNumber");                     // NOLINT(cert-err58-cpp)
const QString AttrZValue = QStringLiteral("zValue");                             // NOLINT(cert-err58-cpp)
const QString AttrX = QStringLiteral("x");                                       // NOLINT(cert-err58-cpp)
const QString AttrY = QStringLiteral("y");                                       // NOLINT(cert-err58-cpp)
const QString AttrTurnPoint = QStringLiteral("turnPoint");                       // NOLINT(cert-err58-cpp)
const QString AttrCurvePoint = QStringLiteral("curvePoint");                     // NOLINT(cert-err58-cpp)
const QString AttrClockwiseOpening = QStringLiteral("clockwiseOpening");         // NOLINT(cert-err58-cpp)

const QString oneWayUpStr = QStringLiteral("oneWayUp");                             // NOLINT(cert-err58-cpp)
const QString oneWayDownStr = QStringLiteral("oneWayDown");                         // NOLINT(cert-err58-cpp)
const QString fourWaysStr = QStringLiteral("fourWays");                             // NOLINT(cert-err58-cpp)
const QString twoWaysUpDownStr = QStringLiteral("twoWaysUpDown");                   // NOLINT(cert-err58-cpp)
const QString twoWaysUpLeftStr = QStringLiteral("twoWaysUpLeft");                   // NOLINT(cert-err58-cpp)
const QString twoWaysUpRightStr = QStringLiteral("twoWaysUpRight");                 // NOLINT(cert-err58-cpp)
const QString twoWaysDownLeftStr = QStringLiteral("twoWaysDownLeft");               // NOLINT(cert-err58-cpp)
const QString twoWaysDownRightStr = QStringLiteral("twoWaysDownRight");             // NOLINT(cert-err58-cpp)
const QString threeWaysUpDownLeftStr = QStringLiteral("threeWaysUpDownLeft");       // NOLINT(cert-err58-cpp)
const QString threeWaysUpDownRightStr = QStringLiteral("threeWaysUpDownRight");     // NOLINT(cert-err58-cpp)
const QString threeWaysUpLeftRightStr = QStringLiteral("threeWaysUpLeftRight");     // NOLINT(cert-err58-cpp)
const QString threeWaysDownLeftRightStr = QStringLiteral("threeWaysDownLeftRight"); // NOLINT(cert-err58-cpp)

const QChar groupSep = ';'_L1;
const QChar coordintatesSep = ','_L1;
const QChar pointsSep = ' '_L1;
const QChar itemsSep = '*'_L1;
} // namespace ML
