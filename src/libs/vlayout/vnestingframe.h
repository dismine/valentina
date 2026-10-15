/************************************************************************
 **
 **  @file   vnestingframe.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 October, 2026
 **
 **  @brief  
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making 
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project 
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

#ifndef VNESTINGFRAME_H
#define VNESTINGFRAME_H

#include "../vmisc/def.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QMap>
#include <QMultiMap>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QStringList>
#include <QTransform>
#include <QVector>

/**
 * Frames for the online nesting service (nestor). Local piece pixels have the grainline along +y; the service's
 * strip runs along +x with the fabric width along y, in millimetres; the sheet is Puzzle's scene in pixels.
 * Every change of frame here is a proper rotation, so a piece is only ever mirrored when the service says so.
 */
#include "vlayoutpaper.h"

class VLayoutPiece;

enum class NestingRotations : qint8
{
    ZeroAnd180 = 0,
    ZeroOnly = 1,
    Any = 2
};

auto NestingRotationsString(NestingRotations rotations) -> QString;

enum class NestingSpeed : qint8
{
    Normal = 0,
    Fast = 1,
    Ultra = 2
};

auto NestingSpeedString(NestingSpeed speed) -> QString;

struct VNestingOptions
{
    NestingRotations rotations{NestingRotations::ZeroAnd180};
    int budgetTokens{0}; // spec 20: the most the job may cost; 0: no budget, until stopped
    NestingSpeed speed{NestingSpeed::Normal};
    qreal targetLengthMm{0}; // 0: no target
    bool untilConverged{false};
    bool preview{false};
    QVector<qreal> rollsMm{}; // physical roll lengths, the last one repeating; empty = one endless roll
    qreal rollJunctionMm{50}; // unusable band at each junction (tail of a roll plus head of the next)
    bool groupByGroup{false}; // spec 21.5: nest the pieces' groups one after another, earlier ones fixed
    bool noBackfill{false};   // spec 21.5: a group never fills a hole left by the earlier ones
};

struct VNestingPlacement
{
    qreal x{0};
    qreal y{0};
    qreal rotationDeg{0};
    bool mirrored{false};
    int roll{0};
};

struct VNestingPieceInput
{
    QString name{};
    QVector<QPointF> contourPx{};
    QTransform base{};
    bool forbidFlipping{false};
    bool forceFlipping{false};
    int quantity{1};
    QString group{};            // spec 21: pieces of one import; empty = none
    QVector<QPointF> fixedMm{}; // spec 21.4: already placed, in strip mm; non-empty = fixed, contourPx unused
};

constexpr qreal NestingPxPerMm = PrintDPI / 25.4;

auto NestingLocalToStrip() -> QTransform;
auto NestingPlacementMatrix(const VNestingPlacement &p) -> QTransform;
auto NestingStripToSheet(const QRectF &marginsRectPx, bool grainlineVertical) -> QTransform;
auto NestingPieceMatrix(const QTransform &base,
                        const VNestingPlacement &p,
                        const QRectF &marginsRectPx,
                        bool grainlineVertical) -> QTransform;
auto NestingFlipped(bool baseFlipped, bool mirrored) -> bool;
auto NestingPolygon(const QVector<QPointF> &localPx, const QTransform &base) -> QJsonArray;
// Sheet px -> strip mm, the inverse of NestingStripToSheet: a placed piece's outline as the service's fixed polygon.
auto NestingFixedPolygon(const QVector<QPointF> &sheetPx, const QRectF &marginsRectPx, bool grainlineVertical)
    -> QVector<QPointF>;
// Identical free pieces (same outline, name, group and flip rules) travel as one item with a quantity; itemOf, when
// given, maps every input to its item id (-1 when skipped) so that a result's copies can be handed back.
auto NestingRequest(const QVector<VNestingPieceInput> &pieces,
                    qreal fabricWidthMm,
                    qreal gapMm,
                    const VNestingOptions &options,
                    QVector<int> &skipped,
                    QVector<int> *itemOf = nullptr) -> QJsonObject;
// Area (mm²) of every copy in a request, for the savings table of the preview.
auto NestingRequestArea(const QJsonObject &request) -> qreal;
// The outline the local nester collides on: seam allowance when there is one, main contour otherwise.
auto NestingContour(const VLayoutPiece &piece, bool togetherWithNotches) -> QVector<QPointF>;
// Rotation to apply after `base` so that the piece's grainline main line lies along local y, choosing the
// direction by the arrow rule of VPPiece::RotateToGrainline. Identity without grainline.
auto NestingGrainlineAlignment(const VLayoutPiece &piece, const QTransform &base) -> QTransform;
// Base matrix of a Valentina layout piece: forced-flip mirror, then grainline alignment.
auto NestingLayoutBase(const VLayoutPiece &piece) -> QTransform;
auto NestingPlanComplete(const QJsonObject &result, const QVector<int> &quantities) -> bool;
auto NestingPlacements(const QJsonObject &result) -> QMultiMap<int, VNestingPlacement>;
// One portrait paper of the fabric width and the marker length (px), every placed copy positioned on it.
auto NestingResultPaper(const QVector<VLayoutPiece> &sent,
                        const QMultiMap<int, VNestingPlacement> &placements,
                        qreal fabricWidthPx,
                        qreal markerLengthPx,
                        qreal layoutWidthPx,
                        bool togetherWithNotches) -> VLayoutPaper;
// One paper per roll that received pieces (result "rolls", spec 13.2), or one paper of the marker length.
auto NestingResultPapers(const QVector<VLayoutPiece> &sent,
                         const QMultiMap<int, VNestingPlacement> &placements,
                         const QJsonObject &result,
                         qreal fabricWidthPx,
                         qreal layoutWidthPx,
                         bool togetherWithNotches) -> QVector<VLayoutPaper>;

#endif // VNESTINGFRAME_H
