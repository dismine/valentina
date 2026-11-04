/************************************************************************
 **
 **  @file   tst_vnestingframe.cpp
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

#include "tst_vnestingframe.h"
#include "../vlayout/vbank.h"
#include "../vlayout/vlayoutpaper.h"
#include "../vlayout/vlayoutpiece.h"
#include "../vlayout/vlayoutpoint.h"
#include "../vlayout/vnestingframe.h"
#include "../vmisc/def.h"
#include "../vwidgets/vpiecegrainline.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineF>
#include <QtMath>
#include <QtTest>

namespace
{
const qreal k = NestingPxPerMm;

// 100 mm along the grainline (local y), 50 mm across (local x), in px
auto RectPx() -> QVector<QPointF>
{
    return {{0, 0}, {50 * k, 0}, {50 * k, 100 * k}, {0, 100 * k}};
}

auto Bounds(const QVector<QPointF> &pts, const QTransform &m) -> QRectF
{
    QPolygonF poly;
    for (const auto &p : pts)
    {
        poly << m.map(p);
    }
    return poly.boundingRect();
}

auto Near(const QRectF &a, const QRectF &b) -> bool
{
    return qAbs(a.left() - b.left()) < 1e-4 && qAbs(a.top() - b.top()) < 1e-4 && qAbs(a.width() - b.width()) < 1e-4
           && qAbs(a.height() - b.height()) < 1e-4;
}

class GrainlinePiece : public VLayoutPiece
{
public:
    using VLayoutPiece::SetGrainline;
};

auto RectPiece() -> GrainlinePiece
{
    GrainlinePiece piece;
    QVector<VLayoutPoint> contour;
    for (const auto &p : RectPx())
    {
        contour.append(VLayoutPoint(p));
    }
    piece.SetContourPoints(contour);
    return piece;
}

auto PieceWithGrainline(qreal angleDeg, GrainlineArrowDirection arrows) -> GrainlinePiece
{
    GrainlinePiece piece = RectPiece();
    QLineF line(QPointF(10 * k, 90 * k), QPointF(10 * k + 80 * k, 90 * k));
    line.setAngle(angleDeg);
    piece.SetGrainline(VPieceGrainline(line, arrows));
    return piece;
}

auto MappedAngle(const VLayoutPiece &piece, const QTransform &m) -> qreal
{
    return m.map(piece.GetGrainlineMainLine()).angle();
}

auto Input(const QString &name, bool forbid = false, bool force = false) -> VNestingPieceInput
{
    return {name, RectPx(), QTransform(), forbid, force};
}

auto Place(qreal xMm, qreal yMm, qreal rotationDeg = 0, const QString &flip = QStringLiteral("none"))
    -> VNestingPlacement
{
    VNestingPlacement placement;
    placement.xMm = xMm;
    placement.yMm = yMm;
    placement.rotationDeg = rotationDeg;
    placement.flip = flip;
    return placement;
}

auto Job(const QVector<VNestingPieceInput> &pieces, const VNestingOptions &options, QVector<int> &skipped,
         qreal gapMm = 2, const QRectF &margins = QRectF(), bool vertical = true, QStringList *instanceOf = nullptr)
    -> QJsonObject
{
    return NestingRequest(pieces, 1500, gapMm, options, margins, vertical, skipped, instanceOf);
}

auto PlacementJson(const QString &instanceId, qreal xMm, qreal yMm, qreal rotationDeg, const QString &flip)
    -> QJsonObject
{
    return {{QStringLiteral("instanceId"), instanceId},
            {QStringLiteral("flip"), flip},
            {QStringLiteral("rotationDeg"), rotationDeg},
            {QStringLiteral("translation"), QJsonObject{{QStringLiteral("xMm"), xMm}, {QStringLiteral("yMm"), yMm}}}};
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
TST_VNestingFrame::TST_VNestingFrame(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::LocalToStripTurnsGrainlineToX()
{
    const QRectF r = Bounds(RectPx(), NestingLocalToStrip());
    QVERIFY(Near(r, QRectF(0, -50, 100, 50))); // 100 mm along x, 50 mm across, a proper rotation (y goes negative)
    QVERIFY(NestingLocalToStrip().determinant() > 0);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::RoundTripIsTranslationOnly_data()
{
    QTest::addColumn<bool>("vertical");
    QTest::newRow("vertical") << true;
    QTest::newRow("horizontal") << false;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::RoundTripIsTranslationOnly()
{
    QFETCH(bool, vertical);
    const QRectF margins(10, 20, 1500 * k, 3000 * k);
    // The profile origin is the unbuffered cut's minimum corner, so an identity placement is translation (0, 0).
    const QTransform m = NestingPieceMatrix(RectPx(), QTransform(), Place(0, 0), margins, vertical);
    // Qt Y points down and marker Y points up the screen, so the sheet map's determinant is negative.
    QVERIFY(m.determinant() < 0);
    const QRectF r = Bounds(RectPx(), m);
    if (vertical)
    {
        // Portrait. Length runs up from the bottom. Width runs left from the right edge.
        QVERIFY(Near(r, QRectF(margins.right() - 50 * k, margins.bottom() - 100 * k, 50 * k, 100 * k)));
    }
    else
    {
        // Landscape. Length runs right. Width runs up from the bottom.
        QVERIFY(Near(r, QRectF(margins.left(), margins.bottom() - 50 * k, 100 * k, 50 * k)));
    }
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::IdentityPlacementSitsAtSheetOrigin()
{
    const QRectF margins(0, 0, 1500 * k, 3000 * k);
    const QTransform m = NestingPieceMatrix(RectPx(), QTransform(), Place(0, 0), margins, true);
    QVERIFY(Near(Bounds(RectPx(), m), QRectF(margins.right() - 50 * k, margins.bottom() - 100 * k, 50 * k, 100 * k)));
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::Rotation180KeepsBoundingBox()
{
    const QRectF margins(0, 0, 1500 * k, 3000 * k);
    // Profile spans x 0..100, y 0..50. A half turn about the origin needs translation (100, 50) to land on that box.
    const QTransform a = NestingPieceMatrix(RectPx(), QTransform(), Place(0, 0), margins, true);
    const QTransform b = NestingPieceMatrix(RectPx(), QTransform(), Place(100, 50, 180), margins, true);
    QVERIFY(Near(Bounds(RectPx(), a), Bounds(RectPx(), b)));
    QVERIFY(b.determinant() < 0);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::MirrorFlipsHandedness()
{
    const QRectF margins(0, 0, 1500 * k, 3000 * k);
    const qreal plainVertical = NestingPieceMatrix(RectPx(), QTransform(), Place(0, 0), margins, true).determinant();
    const qreal flippedVertical
        = NestingPieceMatrix(RectPx(), QTransform(), Place(0, 0, 0, QStringLiteral("y")), margins, true).determinant();
    const qreal plainHorizontal = NestingPieceMatrix(RectPx(), QTransform(), Place(0, 0), margins, false).determinant();
    const qreal flippedHorizontal
        = NestingPieceMatrix(RectPx(), QTransform(), Place(0, 0, 0, QStringLiteral("y")), margins, false).determinant();
    QVERIFY(plainVertical * flippedVertical < 0);
    QVERIFY(plainHorizontal * flippedHorizontal < 0);
    QVERIFY(NestingPlacementMatrix(Place(0, 0, 0, QStringLiteral("y"))).determinant() < 0);
    QVERIFY(NestingPlacementMatrix(Place(0, 0, 0, QStringLiteral("both"))).determinant() > 0);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::FlippedFlag()
{
    QVERIFY(not NestingFlipped(false, false));
    QVERIFY(NestingFlipped(false, true));
    QVERIFY(NestingFlipped(true, false));
    QVERIFY(not NestingFlipped(true, true));
    QVERIFY(NestingFlipMirrors(QStringLiteral("y")));
    QVERIFY(NestingFlipMirrors(QStringLiteral("x")));
    QVERIFY(not NestingFlipMirrors(QStringLiteral("both")));
    QVERIFY(not NestingFlipMirrors(QStringLiteral("none")));
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::MapRotationAndFlip()
{
    const NestingRotationFlip zero = NestingMapRotationAndFlip(NestingRotations::ZeroOnly, false, false);
    QCOMPARE(zero.rotation, QStringLiteral("oneWay"));
    QCOMPARE(zero.flipAllowance, QStringLiteral("y"));
    QCOMPARE(NestingMapRotationAndFlip(NestingRotations::ZeroAnd180, false, false).rotation, QStringLiteral("twoWay"));
    QCOMPARE(NestingMapRotationAndFlip(NestingRotations::Any, false, false).rotation, QStringLiteral("any"));
    QCOMPARE(NestingMapRotationAndFlip(NestingRotations::Any, true, false).flipAllowance, QStringLiteral("none"));
    QCOMPARE(NestingMapRotationAndFlip(NestingRotations::Any, false, true).flipAllowance, QStringLiteral("none"));
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::RequestIsVersion1()
{
    QVector<int> skipped;
    const QJsonObject req = Job({Input("A"), Input("B")}, {NestingRotations::ZeroAnd180, 60}, skipped);
    QVERIFY(skipped.isEmpty());
    QCOMPARE(req.value(QStringLiteral("version")).toInt(), 1);
    QCOMPARE(req.value(QStringLiteral("timeBudgetS")).toDouble(), 60.0);
    QCOMPARE(req.value(QStringLiteral("matchMode")).toString(), QStringLiteral("off"));
    QCOMPARE(req.value(QStringLiteral("honorPlacement")).toBool(), false);
    QCOMPARE(req.value(QStringLiteral("geometryToleranceMm")).toDouble(), NestingGeometryToleranceMm);
    const QJsonObject bin = req.value(QStringLiteral("bin")).toObject();
    QCOMPARE(bin.value(QStringLiteral("usableWidthMm")).toDouble(), 1500.0);
    QCOMPARE(bin.value(QStringLiteral("lengthMode")).toString(), QStringLiteral("open"));
    QCOMPARE(bin.value(QStringLiteral("layoutMode")).toString(), QStringLiteral("single"));
    QVERIFY(bin.value(QStringLiteral("obstacles")).toArray().isEmpty());
    const QStringList absent{QStringLiteral("gap_mm"),
                             QStringLiteral("budget_tokens"),
                             QStringLiteral("speed"),
                             QStringLiteral("group"),
                             QStringLiteral("group_by_group"),
                             QStringLiteral("no_backfill"),
                             QStringLiteral("preview"),
                             QStringLiteral("until_converged"),
                             QStringLiteral("time_s"),
                             QStringLiteral("fabric_width_mm"),
                             QStringLiteral("rolls"),
                             QStringLiteral("forbidFlipping"),
                             QStringLiteral("app")};
    for (const QString &key : absent)
    {
        QVERIFY2(not req.contains(key), qPrintable(key));
    }
    const QJsonObject piece = req.value(QStringLiteral("pieces")).toArray().at(0).toObject();
    QCOMPARE(piece.value(QStringLiteral("rotation")).toString(), QStringLiteral("twoWay"));
    QCOMPARE(piece.value(QStringLiteral("flipAllowance")).toString(), QStringLiteral("y"));
    QCOMPARE(piece.value(QStringLiteral("fold")).toString(), QStringLiteral("none"));
    QVERIFY(piece.value(QStringLiteral("holes")).toArray().isEmpty());
    QVERIFY(piece.value(QStringLiteral("match")).toArray().isEmpty());
    QVERIFY(not piece.contains(QStringLiteral("priority")));
    QVERIFY(not piece.contains(QStringLiteral("name")));
    QVERIFY(not piece.contains(QStringLiteral("quantity")));
    const QJsonArray cut = piece.value(QStringLiteral("cut")).toArray();
    QCOMPARE(cut.size(), 4);
    QCOMPARE(cut.at(0).toArray().at(0).toDouble(), 0.0);
    QCOMPARE(cut.at(0).toArray().at(1).toDouble(), 50.0);
    QCOMPARE(cut.at(2).toArray().at(0).toDouble(), 100.0);
    QCOMPARE(cut.at(2).toArray().at(1).toDouble(), 0.0);

    VNestingPieceInput many = Input("A");
    many.quantity = 8;
    QCOMPARE(Job({many}, {NestingRotations::Any, 45}, skipped).value(QStringLiteral("timeBudgetS")).toDouble(), 45.0);
    QCOMPARE(Job({Input("A")}, {NestingRotations::Any, 45}, skipped).value(QStringLiteral("timeBudgetS")).toDouble(),
             45.0);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::RequestSkipsDegeneratePieces()
{
    QVector<int> skipped;
    VNestingPieceInput two = Input("two");
    two.contourPx = {{0, 0}, {10, 10}};
    VNestingPieceInput flat = Input("flat");
    flat.contourPx = {{0, 0}, {10, 0}, {20, 0}};
    const QJsonObject req = Job({Input("ok"), two, flat}, {NestingRotations::ZeroOnly, 60}, skipped);
    QCOMPARE(skipped, QVector<int>({1, 2}));
    const QJsonArray pieces = req.value(QStringLiteral("pieces")).toArray();
    QCOMPARE(pieces.size(), 1);
    QCOMPARE(pieces.at(0).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("p0"));
    QCOMPARE(pieces.at(0).toObject().value(QStringLiteral("rotation")).toString(), QStringLiteral("oneWay"));
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::RequestOneInstancePerCopy()
{
    QVector<int> skipped;
    QStringList instanceOf;
    VNestingPieceInput named = Input("A");
    named.id = QStringLiteral("sleeve");
    VNestingPieceInput twin = Input("A");
    twin.id = QStringLiteral("sleeve");
    VNestingPieceInput three = Input("B");
    three.quantity = 3;
    const QJsonObject req = Job({named, twin, three}, {NestingRotations::ZeroOnly, 30}, skipped, 2, QRectF(), true,
                                &instanceOf);
    const QJsonArray pieces = req.value(QStringLiteral("pieces")).toArray();
    const QJsonArray instances = req.value(QStringLiteral("instances")).toArray();
    QCOMPARE(pieces.size(), 3); // same name is not merged
    QCOMPARE(instances.size(), 5);
    QCOMPARE(pieces.at(0).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("sleeve#0"));
    QCOMPARE(pieces.at(1).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("sleeve#1"));
    QCOMPARE(pieces.at(0).toObject().value(QStringLiteral("flipAllowance")).toString(), QStringLiteral("y"));
    QCOMPARE(instances.at(0).toObject().value(QStringLiteral("id")).toString(), QStringLiteral("sleeve#0//authored/0"));
    QCOMPARE(instances.at(2).toObject().value(QStringLiteral("index")).toInt(), 0);
    QCOMPARE(instances.at(4).toObject().value(QStringLiteral("index")).toInt(), 2);
    QCOMPARE(instanceOf, QStringList({QStringLiteral("sleeve#0//authored/0"), QStringLiteral("sleeve#1//authored/0"),
                                      QStringLiteral("p2//authored/0")}));

    QVector<int> flipSkipped;
    const QJsonArray rules = Job({Input("free"), Input("forbid", true), Input("force", false, true)},
                                 {NestingRotations::ZeroAnd180, 30}, flipSkipped)
                                 .value(QStringLiteral("pieces"))
                                 .toArray();
    QCOMPARE(rules.at(0).toObject().value(QStringLiteral("flipAllowance")).toString(), QStringLiteral("y"));
    QCOMPARE(rules.at(1).toObject().value(QStringLiteral("flipAllowance")).toString(), QStringLiteral("none"));
    QCOMPARE(rules.at(2).toObject().value(QStringLiteral("flipAllowance")).toString(), QStringLiteral("none"));
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::RequestBakesHalfGap()
{
    QVector<int> skipped;
    const QJsonObject piece = Job({Input("A")}, {NestingRotations::ZeroOnly, 30}, skipped, 2)
                                  .value(QStringLiteral("pieces"))
                                  .toArray()
                                  .at(0)
                                  .toObject();
    QVERIFY(not piece.contains(QStringLiteral("gap_mm")));
    qreal minX = 1e9;
    qreal maxX = -1e9;
    qreal minY = 1e9;
    qreal maxY = -1e9;
    for (const QJsonValue value : piece.value(QStringLiteral("collision")).toArray())
    {
        const QJsonArray pair = value.toArray();
        minX = qMin(minX, pair.at(0).toDouble());
        maxX = qMax(maxX, pair.at(0).toDouble());
        minY = qMin(minY, pair.at(1).toDouble());
        maxY = qMax(maxY, pair.at(1).toDouble());
    }
    // Half of the 2 mm gap on every side. The cut itself stays on 0..100 by 0..50.
    QVERIFY(qAbs(minX + 1.0) < 0.02);
    QVERIFY(qAbs(maxX - 101.0) < 0.02);
    QVERIFY(qAbs(minY + 1.0) < 0.02);
    QVERIFY(qAbs(maxY - 51.0) < 0.02);
    QCOMPARE(piece.value(QStringLiteral("collision")).toArray().size() > 4, true);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::RequestLengthMode()
{
    QVector<int> skipped;
    const QJsonObject openBin = Job({Input("A")}, {NestingRotations::Any, 30}, skipped).value(QStringLiteral("bin")).toObject();
    QCOMPARE(openBin.value(QStringLiteral("lengthMode")).toString(), QStringLiteral("open"));
    QVERIFY(not openBin.contains(QStringLiteral("usableLengthMm")));

    VNestingOptions fixed{NestingRotations::Any, 30, true, 2000};
    const QJsonObject fixedBin = Job({Input("A")}, fixed, skipped).value(QStringLiteral("bin")).toObject();
    QCOMPARE(fixedBin.value(QStringLiteral("lengthMode")).toString(), QStringLiteral("fixedFail"));
    QCOMPARE(fixedBin.value(QStringLiteral("usableLengthMm")).toDouble(), 2000.0);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::RequestLockRoundTrip_data()
{
    QTest::addColumn<bool>("vertical");
    QTest::addColumn<QString>("flip");
    QTest::addColumn<qreal>("rotation");
    QTest::addColumn<qreal>("xMm");
    QTest::addColumn<qreal>("yMm");
    QTest::newRow("vertical identity") << true << QStringLiteral("none") << 0.0 << 0.0 << 0.0;
    QTest::newRow("vertical mirror") << true << QStringLiteral("y") << 90.0 << 300.0 << 40.0;
    QTest::newRow("horizontal") << false << QStringLiteral("none") << 15.0 << 80.0 << 25.0;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::RequestLockRoundTrip()
{
    QFETCH(bool, vertical);
    QFETCH(QString, flip);
    QFETCH(qreal, rotation);
    QFETCH(qreal, xMm);
    QFETCH(qreal, yMm);
    const QRectF margins(100, 200, 4000, 6000);
    const VNestingPlacement pose = Place(xMm, yMm, rotation, flip);
    const QTransform matrix = NestingPieceMatrix(RectPx(), QTransform(), pose, margins, vertical);
    VNestingPieceInput locked = Input("locked");
    locked.placedSheetPx = matrix.map(QPolygonF(RectPx()));
    QVector<int> skipped;
    const QJsonObject instance = Job({locked}, {NestingRotations::Any, 30}, skipped, 0, margins, vertical)
                                     .value(QStringLiteral("instances"))
                                     .toArray()
                                     .at(0)
                                     .toObject();
    const QJsonObject lock = instance.value(QStringLiteral("lock")).toObject();
    QVERIFY2(not instance.value(QStringLiteral("lock")).isNull(), qPrintable(QJsonDocument(instance).toJson()));
    const VNestingPlacement back = Place(lock.value(QStringLiteral("translation")).toObject().value(QStringLiteral("xMm")).toDouble(),
                                         lock.value(QStringLiteral("translation")).toObject().value(QStringLiteral("yMm")).toDouble(),
                                         lock.value(QStringLiteral("rotationDeg")).toDouble(),
                                         lock.value(QStringLiteral("flip")).toString());
    const QRectF expected = Bounds(RectPx(), matrix);
    const QRectF again = Bounds(RectPx(), NestingPieceMatrix(RectPx(), QTransform(), back, margins, vertical));
    // The lock is stored to a thousandth of a millimetre.
    QVERIFY(qAbs(again.left() - expected.left()) < 0.05 && qAbs(again.top() - expected.top()) < 0.05
            && qAbs(again.width() - expected.width()) < 0.05 && qAbs(again.height() - expected.height()) < 0.05);

    VNestingPieceInput broken = Input("broken");
    broken.placedSheetPx = {{0, 0}, {1, 0}, {0, 1}};
    const QJsonValue nullLock = Job({broken}, {NestingRotations::Any, 30}, skipped, 0, margins, vertical)
                                    .value(QStringLiteral("instances"))
                                    .toArray()
                                    .at(0)
                                    .toObject()
                                    .value(QStringLiteral("lock"));
    QVERIFY(nullLock.isNull());
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::PlanComplete()
{
    const QString a = QStringLiteral("a//authored/0");
    const QString b = QStringLiteral("b//authored/0");
    const QJsonObject ok{{QStringLiteral("status"), QStringLiteral("complete")},
                         {QStringLiteral("bins"),
                          QJsonArray{QJsonObject{{QStringLiteral("lengthUsedMm"), 10},
                                                 {QStringLiteral("placements"),
                                                  QJsonArray{PlacementJson(a, 0, 0, 0, QStringLiteral("none")),
                                                             PlacementJson(b, 12.5, -3, 180, QStringLiteral("y"))}}}}},
                         {QStringLiteral("unplaced"), QJsonArray{}}};
    QVERIFY(NestingPlanComplete(ok, {a, b}));
    QVERIFY(not NestingPlanComplete(ok, {a, b, QStringLiteral("c")}));
    QJsonObject incomplete = ok;
    incomplete.insert(QStringLiteral("status"), QStringLiteral("incomplete"));
    QVERIFY(not NestingPlanComplete(incomplete, {a, b}));
    const QHash<QString, VNestingPlacement> map = NestingPlacements(ok);
    QCOMPARE(map.size(), 2);
    QCOMPARE(map.value(b).xMm, 12.5);
    QCOMPARE(map.value(b).yMm, -3.0);
    QCOMPARE(map.value(b).rotationDeg, 180.0);
    QCOMPARE(map.value(b).flip, QStringLiteral("y"));
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::SummaryShowsIncompleteAndReport()
{
    QVector<int> skipped;
    const QJsonObject job = Job({Input("A")}, {NestingRotations::Any, 30}, skipped);
    const QString id = job.value(QStringLiteral("instances")).toArray().at(0).toObject().value(QStringLiteral("id")).toString();
    const QJsonObject incomplete{{QStringLiteral("status"), QStringLiteral("incomplete")},
                                 {QStringLiteral("bins"),
                                  QJsonArray{QJsonObject{{QStringLiteral("lengthUsedMm"), 1000},
                                                         {QStringLiteral("placements"), QJsonArray{}}}}},
                                 {QStringLiteral("unplaced"), QJsonArray{id}}};
    const QString text = NestingSummaryText(incomplete, job);
    QVERIFY2(text.contains(QStringLiteral("not finished")), qPrintable(text));
    QVERIFY2(text.contains(id), qPrintable(text));

    const QJsonObject bare{{QStringLiteral("status"), QStringLiteral("complete")},
                           {QStringLiteral("bins"),
                            QJsonArray{QJsonObject{{QStringLiteral("lengthUsedMm"), 1000},
                                                   {QStringLiteral("placements"),
                                                    QJsonArray{PlacementJson(id, 0, 0, 0, QStringLiteral("none"))}}}}},
                           {QStringLiteral("unplaced"), QJsonArray{}}};
    QVERIFY2(not NestingSummaryText(bare, job).contains(QStringLiteral("not finished")),
             qPrintable(NestingSummaryText(bare, job)));
    QVERIFY2(NestingSummaryText(bare).contains(QStringLiteral("placed")) || not NestingSummaryText(bare).isEmpty(),
             "a missing report is not a failure");

    QJsonObject reported = bare;
    reported.insert(QStringLiteral("report"), QJsonObject{{QStringLiteral("stopReason"), QStringLiteral("timeBudget")}});
    QVERIFY2(NestingSummaryText(reported, job).contains(QStringLiteral("timeBudget")), qPrintable(NestingSummaryText(reported, job)));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::GrainlineAlignment_data()
{
    QTest::addColumn<int>("arrows");
    QTest::addColumn<qreal>("expectedAngle");
    QTest::newRow("up") << static_cast<int>(GrainlineArrowDirection::oneWayUp) << 270.0;
    QTest::newRow("down") << static_cast<int>(GrainlineArrowDirection::oneWayDown) << 90.0;
    QTest::newRow("up and down") << static_cast<int>(GrainlineArrowDirection::twoWaysUpDown) << 90.0;
    QTest::newRow("four ways") << static_cast<int>(GrainlineArrowDirection::fourWays) << 0.0;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::GrainlineAlignment()
{
    QFETCH(int, arrows);
    QFETCH(qreal, expectedAngle);
    const GrainlinePiece piece = PieceWithGrainline(30, static_cast<GrainlineArrowDirection>(arrows));
    const QTransform m = NestingGrainlineAlignment(piece, QTransform());
    QVERIFY2(qAbs(MappedAngle(piece, m) - expectedAngle) < 1e-6, qPrintable(QString::number(MappedAngle(piece, m))));
    QVERIFY(qAbs(m.determinant() - 1) < 1e-9);
    QVERIFY(qAbs(m.dx()) < 1e-9 && qAbs(m.dy()) < 1e-9);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::GrainlineAlignmentIdentityWithoutGrainline()
{
    const GrainlinePiece piece = RectPiece();
    QVERIFY(not piece.IsGrainlineEnabled());
    QVERIFY(NestingGrainlineAlignment(piece, QTransform()).isIdentity());
    QVERIFY(NestingLayoutBase(piece).isIdentity());
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::LayoutBaseForceFlipped()
{
    GrainlinePiece piece = PieceWithGrainline(30, GrainlineArrowDirection::oneWayUp);
    piece.SetForceFlipping(true);
    const QTransform base = NestingLayoutBase(piece);
    QVERIFY(qAbs(base.determinant() + 1) < 1e-9);
    QVERIFY2(qAbs(MappedAngle(piece, base) - 270.0) < 1e-6, qPrintable(QString::number(MappedAngle(piece, base))));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::ResultPaperPlacesPieces()
{
    const QString a = QStringLiteral("a//authored/0");
    const QString b = QStringLiteral("b//authored/0");
    QHash<QString, VNestingPlacement> placements;
    placements.insert(a, Place(0, 0));
    placements.insert(b, Place(100, 0));
    const qreal W = 1500 * k;
    const VLayoutPaper paper = NestingResultPaper({RectPiece(), RectPiece()}, {a, b}, placements, W, 200 * k, 2 * k, false);
    QCOMPARE(paper.GetWidth(), qCeil(W));
    QCOMPARE(paper.GetHeight(), qCeil(200 * k));
    const QVector<VLayoutPiece> details = paper.GetDetails();
    QCOMPARE(details.size(), 2);
    const QRectF r0 = details.at(0).MappedDetailBoundingRect();
    QVERIFY2(qAbs(r0.left() - (W - 50 * k)) < 1e-4 && qAbs(r0.top() - 100 * k) < 1e-4,
             qPrintable(QStringLiteral("%1 %2").arg(r0.left()).arg(r0.top())));
    QVERIFY(qAbs(r0.width() - 50 * k) < 1e-4 && qAbs(r0.height() - 100 * k) < 1e-4);
    const QRectF r1 = details.at(1).MappedDetailBoundingRect();
    QVERIFY2(qAbs(r1.top()) < 1e-4, qPrintable(QString::number(r1.top())));
    QVERIFY(not details.at(0).IsVerticallyFlipped());
    QVERIFY(qAbs(details.at(0).GetLayoutWidth() - 2 * k) < 1e-9);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::ResultPaperCopies()
{
    const QString plain = QStringLiteral("a//authored/0");
    const QString mirrored = QStringLiteral("a//authored/1");
    QHash<QString, VNestingPlacement> placements;
    placements.insert(plain, Place(0, 0));
    placements.insert(mirrored, Place(100, 0, 0, QStringLiteral("y")));
    const VLayoutPaper paper = NestingResultPaper({RectPiece(), RectPiece()}, {plain, mirrored}, placements, 1500 * k,
                                                  200 * k, 0, false);
    const QVector<VLayoutPiece> details = paper.GetDetails();
    QCOMPARE(details.size(), 2);
    QCOMPARE(static_cast<int>(details.at(0).IsVerticallyFlipped()) + static_cast<int>(details.at(1).IsVerticallyFlipped()),
             1);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::ResultPapersOnePerBin()
{
    const QString id = QStringLiteral("p0//authored/0");
    const QJsonObject result{
        {QStringLiteral("status"), QStringLiteral("complete")},
        {QStringLiteral("bins"),
         QJsonArray{QJsonObject{{QStringLiteral("lengthUsedMm"), 100},
                                {QStringLiteral("placements"), QJsonArray{PlacementJson(id, 0, 0, 0, QStringLiteral("none"))}}},
                    QJsonObject{{QStringLiteral("lengthUsedMm"), 150},
                                {QStringLiteral("placements"),
                                 QJsonArray{PlacementJson(id, 50, 0, 0, QStringLiteral("none"))}}}}},
        {QStringLiteral("unplaced"), QJsonArray{}}};
    const QVector<VLayoutPaper> papers = NestingResultPapers({RectPiece()}, {id}, result, 1500 * k, 0, false);
    QCOMPARE(papers.size(), 2);
    QCOMPARE(papers.at(0).GetHeight(), qCeil(100 * k));
    QCOMPARE(papers.at(1).GetHeight(), qCeil(150 * k));
    QCOMPARE(papers.at(1).GetDetails().size(), 1);
    const QRectF r = papers.at(1).GetDetails().at(0).MappedDetailBoundingRect();
    QVERIFY2(qAbs(r.top()) < 1e-4, qPrintable(QString::number(r.top())));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::ExpandQuantityFlipsSymmetricalCopies()
{
    GrainlinePiece sleeve = RectPiece();
    sleeve.SetQuantity(2);
    sleeve.SetSymmetricalCopy(true);
    sleeve.SetForceFlipping(true);
    GrainlinePiece plain = RectPiece();
    plain.SetQuantity(3);
    const QVector<VLayoutPiece> expanded = VBank::PrepareQuantity({sleeve, plain});
    QCOMPARE(expanded.size(), 5);
    QVERIFY(expanded.at(0).IsForceFlipping() && not expanded.at(0).IsForbidFlipping());
    QVERIFY(expanded.at(1).IsForbidFlipping() && not expanded.at(1).IsForceFlipping());
    QVERIFY(not expanded.at(4).IsForceFlipping() && not expanded.at(4).IsForbidFlipping());
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::ContourUsesSeamAllowance()
{
    GrainlinePiece piece = RectPiece();
    QVERIFY(NestingContour(piece, false).size() == 4);
    QVector<VLayoutPoint> sa;
    for (const auto &p : QVector<QPointF>{{-k, -k}, {51 * k, -k}, {51 * k, 101 * k}, {-k, 101 * k}, {-k, 50 * k}})
    {
        sa.append(VLayoutPoint(p));
    }
    piece.SetSeamAllowance(true);
    piece.SetSeamAllowancePoints(sa, true, false);
    const QVector<QPointF> contour = NestingContour(piece, false);
    QCOMPARE(contour.size(), 5);
    QVERIFY(qAbs(contour.at(0).x() + k) < 1e-9);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::RequestArea()
{
    QVector<int> skipped;
    VNestingPieceInput two = Input("A");
    two.quantity = 2;
    VNestingPieceInput flat = Input("flat");
    flat.contourPx = {{0, 0}, {10, 0}, {20, 0}};
    const QJsonObject req = Job({two, Input("B"), flat}, VNestingOptions{}, skipped);
    QCOMPARE(NestingRequestArea(req), 3 * 100.0 * 50.0);
    QCOMPARE(NestingRequestArea(QJsonObject{}), 0.0);
}
