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

#include <QHash>
#include <QJsonArray>
#include <QJsonObject>
#include <QPointF>
#include <QRectF>
#include <QString>
#include <QStringList>
#include <QTransform>
#include <QVector>

/**
 * Version 1 nesting job. Profile millimetres have the grain along +X and the origin at the unbuffered cut's
 * minimum corner. The marker is Y-up: +X along the roll, +Y across the cloth. The sheet is Puzzle's scene.
 */
#include "vlayoutpaper.h"

class VLayoutPiece;

enum class NestingRotations : qint8
{
    ZeroAnd180 = 0,
    ZeroOnly = 1,
    Any = 2
};

// The project's point tolerance, in millimetres. One number for the job.
constexpr qreal NestingGeometryToleranceMm = 0.1555;
constexpr qreal NestingPxPerMm = PrintDPI / 25.4;

struct VNestingOptions
{
    NestingRotations rotations{NestingRotations::ZeroOnly};
    qreal timeBudgetS{30};     // positive seconds typed by the user; never derived from the piece count
    bool fixedLength{false};   // the layout dialog's sheet length is fixed
    qreal usableLengthMm{0};   // that length, when fixedLength is set
};

// A placement is flip, then a counterclockwise rotation about the profile origin, then a translation.
struct VNestingPlacement
{
    QString instanceId{};
    QString flip{QStringLiteral("none")}; // none, x, y, both
    qreal rotationDeg{0};
    qreal xMm{0};
    qreal yMm{0};
};

struct VNestingPieceInput
{
    QString name{};
    QVector<QPointF> contourPx{};
    QTransform base{};
    bool forbidFlipping{false};
    bool forceFlipping{false};
    int quantity{1};
    QString id{};                    // opaque piece id; empty lets the shim mint one. Copies are not merged by name.
    QVector<QPointF> placedSheetPx{}; // non-empty: this copy is already on the sheet and travels as a lock
};

struct VNestingRead
{
    QString status{};
    bool complete{false};
    qreal lengthUsedMm{0};
    QHash<QString, VNestingPlacement> placements{};
    QStringList unplaced{};
    bool hasReport{false};
    QJsonObject report{};
};

struct NestingRotationFlip
{
    QString rotation{};      // oneWay, twoWay, any
    QString flipAllowance{}; // none, x, y, both
};

// Loss: the layout dialog offers one rotation for every piece (follow the grainline, or that grainline
// and 180°) and one mirror switch. fourWay, eightWay, any, and a tilt window cannot be named. Today's mirror is a single reflection
// across the grain, which is profile +X, so an allowed mirror is flipAllowance "y" and not "x" or "both". A
// forced flip is baked into the profile before the job; the job then forbids a further flip. forbidFlipping
// is not a job field: it is flipAllowance "none".
auto NestingMapRotationAndFlip(NestingRotations rotations, bool forbidFlipping, bool forceFlipping)
    -> NestingRotationFlip;

auto NestingLocalToStrip() -> QTransform;
auto NestingFlipMatrix(const QString &flip) -> QTransform;
auto NestingPlacementMatrix(const VNestingPlacement &p) -> QTransform;
auto NestingMarkerToSheet(const QRectF &marginsRectPx, bool grainlineVertical) -> QTransform;
auto NestingSheetToMarker(const QRectF &marginsRectPx, bool grainlineVertical) -> QTransform;
auto NestingPieceMatrix(const QVector<QPointF> &contourPx,
                        const QTransform &base,
                        const VNestingPlacement &p,
                        const QRectF &marginsRectPx,
                        bool grainlineVertical) -> QTransform;
auto NestingFlipped(bool baseFlipped, bool mirrored) -> bool;
auto NestingFlipMirrors(const QString &flip) -> bool;
// Outward around-offset of a profile ring, corner arcs included. The origin is not moved.
auto NestingAroundBuffer(const QVector<QPointF> &cutMm, qreal distanceMm) -> QVector<QPointF>;
// One version 1 job. instanceOf, when given, is parallel to pieces: empty when that input was skipped.
auto NestingRequest(const QVector<VNestingPieceInput> &pieces,
                    qreal usableWidthMm,
                    qreal gapMm,
                    const VNestingOptions &options,
                    const QRectF &marginsRectPx,
                    bool grainlineVertical,
                    QVector<int> &skipped,
                    QStringList *instanceOf = nullptr) -> QJsonObject;
// Sum of cut-contour areas (mm²), one per instance. Buffered area is not piece area.
auto NestingRequestArea(const QJsonObject &job) -> qreal;
auto NestingContour(const VLayoutPiece &piece, bool togetherWithNotches) -> QVector<QPointF>;
auto NestingGrainlineAlignment(const VLayoutPiece &piece, const QTransform &base) -> QTransform;
auto NestingLayoutBase(const VLayoutPiece &piece) -> QTransform;
auto NestingReadResult(const QJsonObject &result) -> VNestingRead;
auto NestingPlanComplete(const QJsonObject &result, const QStringList &instanceIds) -> bool;
auto NestingPlacements(const QJsonObject &result) -> QHash<QString, VNestingPlacement>;
// Host summary. A missing report adds nothing and is not a failure. An incomplete status is said so.
auto NestingSummaryText(const QJsonObject &result, const QJsonObject &job = QJsonObject()) -> QString;
auto NestingResultPaper(const QVector<VLayoutPiece> &sent,
                        const QStringList &instanceIds,
                        const QHash<QString, VNestingPlacement> &placements,
                        qreal fabricWidthPx,
                        qreal markerLengthPx,
                        qreal layoutWidthPx,
                        bool togetherWithNotches) -> VLayoutPaper;
// One paper per bin. Pieces the result does not place are left out of the paper.
auto NestingResultPapers(const QVector<VLayoutPiece> &sent,
                         const QStringList &instanceIds,
                         const QJsonObject &result,
                         qreal fabricWidthPx,
                         qreal layoutWidthPx,
                         bool togetherWithNotches) -> QVector<VLayoutPaper>;

#endif // VNESTINGFRAME_H
