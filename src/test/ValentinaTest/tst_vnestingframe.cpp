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
    return qAbs(a.left() - b.left()) < 1e-6 && qAbs(a.top() - b.top()) < 1e-6 && qAbs(a.width() - b.width()) < 1e-6
           && qAbs(a.height() - b.height()) < 1e-6;
}

// VLayoutPiece::SetGrainline is protected; the test needs to set one directly.
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

// the 50 x 100 mm rectangle with a grainline drawn from (10, 90) mm at `angleDeg` (QLineF convention)
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
    // the strip spans y in [0, fabric width]; the raw piece sits at y in [-50, 0], so a real placement shifts it
    // by +50 along y. Sent and placed that way, it lands with its grainline along the sheet's roll axis, unflipped.
    const QTransform m = NestingLocalToStrip() * QTransform::fromTranslate(0, 50)
                         * NestingStripToSheet(margins, vertical);
    QVERIFY(m.determinant() > 0);
    const QRectF r = Bounds(RectPx(), m);
    if (vertical)
    {
        QVERIFY(Near(r, QRectF(margins.right() - 50 * k, margins.top(), 50 * k, 100 * k)));
    }
    else
    {
        QVERIFY(Near(r, QRectF(margins.left(), margins.top(), 100 * k, 50 * k)));
    }
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::IdentityPlacementSitsAtStripOrigin()
{
    const QRectF margins(0, 0, 1500 * k, 3000 * k);
    const QTransform m = NestingPieceMatrix(QTransform(), {0, 50, 0, false}, margins, true);
    QVERIFY(Near(Bounds(RectPx(), m), QRectF(margins.right() - 50 * k, 0, 50 * k, 100 * k)));
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::Rotation180KeepsBoundingBox()
{
    const QRectF margins(0, 0, 1500 * k, 3000 * k);
    // in strip mm the rectangle spans x 0..100, y -50..0; rotated 180 it spans x -100..0, y 0..50;
    // translating by (100, 0) then gives the same box as the unrotated piece translated by (0, 50)
    const QTransform a = NestingPieceMatrix(QTransform(), {0, 50, 0, false}, margins, true);
    const QTransform b = NestingPieceMatrix(QTransform(), {100, 0, 180, false}, margins, true);
    QVERIFY(Near(Bounds(RectPx(), a), Bounds(RectPx(), b)));
    QVERIFY(b.determinant() > 0);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::MirrorFlipsHandedness()
{
    const QRectF margins(0, 0, 1500 * k, 3000 * k);
    QVERIFY(NestingPieceMatrix(QTransform(), {0, 0, 0, true}, margins, true).determinant() < 0);
    QVERIFY(NestingPieceMatrix(QTransform(), {0, 0, 0, true}, margins, false).determinant() < 0);
    QVERIFY(NestingPlacementMatrix({0, 0, 0, true}).determinant() < 0);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::FlippedFlag()
{
    QVERIFY(not NestingFlipped(false, false));
    QVERIFY(NestingFlipped(false, true));
    QVERIFY(NestingFlipped(true, false));
    QVERIFY(not NestingFlipped(true, true));
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::RequestShape()
{
    QVector<int> skipped;
    const QJsonObject req = NestingRequest({Input("A"), Input("B")},
                                           1500,
                                           2,
                                           {NestingRotations::ZeroAnd180, 60},
                                           skipped);
    QVERIFY(skipped.isEmpty());
    QCOMPARE(req.value("fabric_width_mm").toDouble(), 1500.0);
    QCOMPARE(req.value("gap_mm").toDouble(), 2.0);
    QCOMPARE(req.value("time_s").toInt(), 0); // spec 20: no wall cap
    QCOMPARE(req.value("budget_tokens").toInt(), 60);
    const QJsonArray pieces = req.value("pieces").toArray();
    QCOMPARE(pieces.size(), 2);
    const QJsonObject p0 = pieces.at(0).toObject();
    QCOMPARE(p0.value("id").toInt(), 0);
    QCOMPARE(p0.value("name").toString(), QStringLiteral("A"));
    QCOMPARE(p0.value("quantity").toInt(), 1);
    QCOMPARE(p0.value("rotations").toArray(), QJsonArray({0, 180}));
    QVERIFY(p0.value("mirror").toBool());
    const QJsonArray poly = p0.value("polygon").toArray();
    QCOMPARE(poly.size(), 4);
    QCOMPARE(poly.at(2).toArray().at(0).toDouble(), 100.0); // (50k,100k) px -> (100, -50) mm
    QCOMPARE(poly.at(2).toArray().at(1).toDouble(), -50.0);
    QCOMPARE(NestingRequest({Input("A")}, 1500, 2, {NestingRotations::Any, 60}, skipped)
                 .value("pieces")
                 .toArray()
                 .at(0)
                 .toObject()
                 .value("rotations")
                 .toString(),
             QStringLiteral("continuous"));
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
    const QJsonObject req = NestingRequest({Input("ok"), two, flat}, 1500, 2, {NestingRotations::ZeroOnly, 60}, skipped);
    QCOMPARE(skipped, QVector<int>({1, 2}));
    const QJsonArray pieces = req.value("pieces").toArray();
    QCOMPARE(pieces.size(), 1);
    QCOMPARE(pieces.at(0).toObject().value("id").toInt(), 0); // ids index the *sent* list
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::RequestFlippingRules()
{
    QVector<int> skipped;
    const QJsonArray pieces = NestingRequest({Input("free"), Input("forbid", true), Input("force", false, true)},
                                             1500,
                                             2,
                                             {NestingRotations::ZeroAnd180, 60},
                                             skipped)
                                  .value("pieces")
                                  .toArray();
    QVERIFY(pieces.at(0).toObject().value("mirror").toBool());
    QVERIFY(not pieces.at(1).toObject().value("mirror").toBool());
    QVERIFY(not pieces.at(2).toObject().value("mirror").toBool());
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VNestingFrame::PlanComplete()
{
    auto placement = [](int id)
    { return QJsonObject{{"piece_id", id}, {"x", 0}, {"y", 0}, {"rotation_deg", 0}, {"mirrored", false}}; };
    const QJsonObject ok{{"placements", QJsonArray{placement(0), placement(1)}}};
    QVERIFY(NestingPlanComplete(ok, {1, 1}));
    QVERIFY(not NestingPlanComplete(ok, {1, 1, 1}));
    QVERIFY(not NestingPlanComplete(QJsonObject{{"placements", QJsonArray{placement(0), placement(0)}}}, {1, 1}));
    QVERIFY(not NestingPlanComplete(QJsonObject{{"placements", QJsonArray{placement(0), placement(5)}}}, {1, 1}));
    const auto map = NestingPlacements(QJsonObject{
        {"placements",
         QJsonArray{QJsonObject{{"piece_id", 1}, {"x", 12.5}, {"y", -3}, {"rotation_deg", 180}, {"mirrored", true}}}}});
    QCOMPARE(map.size(), 1);
    QCOMPARE(map.value(1).x, 12.5);
    QCOMPARE(map.value(1).rotationDeg, 180.0);
    QVERIFY(map.value(1).mirrored);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::RequestRotationsAndQuantity()
{
    QCOMPARE(NestingRotationsString(NestingRotations::ZeroAnd180), QStringLiteral("0,180"));
    QCOMPARE(NestingRotationsString(NestingRotations::ZeroOnly), QStringLiteral("0"));
    QCOMPARE(NestingRotationsString(NestingRotations::Any), QStringLiteral("continuous"));

    QVector<int> skipped;
    VNestingPieceInput three = Input("A");
    three.quantity = 3;
    const QJsonObject req = NestingRequest({three}, 1500, 2, {NestingRotations::ZeroOnly, 60}, skipped);
    const QJsonObject p0 = req.value("pieces").toArray().at(0).toObject();
    QCOMPARE(p0.value("quantity").toInt(), 3);
    QCOMPARE(p0.value("rotations").toArray(), QJsonArray{QJsonValue(0)});
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::PlanCompleteWithQuantities()
{
    auto placement = [](int id)
    { return QJsonObject{{"piece_id", id}, {"x", 0}, {"y", 0}, {"rotation_deg", 0}, {"mirrored", false}}; };
    const QJsonObject result{{"placements", QJsonArray{placement(0), placement(0), placement(1)}}};
    QVERIFY(NestingPlanComplete(result, {2, 1}));
    QVERIFY(not NestingPlanComplete(result, {1, 1})); // one copy too many for piece 0
    QVERIFY(not NestingPlanComplete(result, {2, 2})); // one copy missing for piece 1
    const QMultiMap<int, VNestingPlacement> placements = NestingPlacements(result);
    QCOMPARE(placements.count(0), 2);
    QCOMPARE(placements.count(1), 1);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::GrainlineAlignment_data()
{
    QTest::addColumn<int>("arrows");
    QTest::addColumn<qreal>("expectedAngle");
    // line drawn at 30°: up arrow -> line points up (90°), down arrow -> line points down (270°),
    // both -> the smaller turn (+60° to 90°), four ways -> the left arrow (line+90°) points up -> line at 0°
    QTest::newRow("up") << static_cast<int>(GrainlineArrowDirection::oneWayUp) << 90.0;
    QTest::newRow("down") << static_cast<int>(GrainlineArrowDirection::oneWayDown) << 270.0;
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
    QVERIFY(qAbs(m.determinant() - 1) < 1e-9); // a rotation, no mirror
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
    QVERIFY(qAbs(base.determinant() + 1) < 1e-9); // mirrored once
    QVERIFY2(qAbs(MappedAngle(piece, base) - 90.0) < 1e-6,
             qPrintable(QString::number(MappedAngle(piece, base)))); // and still aligned
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::ResultPaperPlacesPieces()
{
    // two 50 x 100 mm rectangles one after the other along the 1500 mm wide strip: strip x is the marker length
    const VLayoutPiece a = RectPiece();
    const VLayoutPiece b = RectPiece();
    QMultiMap<int, VNestingPlacement> placements;
    placements.insert(0, {0, 50, 0, false}); // local (x,y) -> strip (y,-x): y in [-50,0] + 50 = [0,50]
    placements.insert(1, {100, 50, 0, false});
    const qreal W = 1500 * k;
    const VLayoutPaper paper = NestingResultPaper({a, b}, placements, W, 200 * k, 2 * k, false);
    QCOMPARE(paper.GetWidth(), qCeil(W));
    QCOMPARE(paper.GetHeight(), qCeil(200 * k));
    const QVector<VLayoutPiece> details = paper.GetDetails();
    QCOMPARE(details.size(), 2);
    // vertical sheet frame: strip x runs down the sheet, strip y runs leftwards from the right edge
    const QRectF r0 = details.at(0).MappedDetailBoundingRect();
    QVERIFY2(qAbs(r0.left() - (W - 50 * k)) < 1e-6 && qAbs(r0.top()) < 1e-6,
             qPrintable(QStringLiteral("%1 %2").arg(r0.left()).arg(r0.top())));
    QVERIFY(qAbs(r0.width() - 50 * k) < 1e-6 && qAbs(r0.height() - 100 * k) < 1e-6);
    const QRectF r1 = details.at(1).MappedDetailBoundingRect();
    QVERIFY2(qAbs(r1.top() - 100 * k) < 1e-6, qPrintable(QString::number(r1.top())));
    QVERIFY(not details.at(0).IsVerticallyFlipped());
    QVERIFY(qAbs(details.at(0).GetLayoutWidth() - 2 * k) < 1e-9);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::ResultPaperCopies()
{
    const VLayoutPiece a = RectPiece();
    QMultiMap<int, VNestingPlacement> placements;
    placements.insert(0, {0, 50, 0, false});
    placements.insert(0, {100, 50, 0, true}); // second copy mirrored
    const VLayoutPaper paper = NestingResultPaper({a}, placements, 1500 * k, 200 * k, 0, false);
    const QVector<VLayoutPiece> details = paper.GetDetails();
    QCOMPARE(details.size(), 2);
    QCOMPARE(static_cast<int>(details.at(0).IsVerticallyFlipped())
                 + static_cast<int>(details.at(1).IsVerticallyFlipped()),
             1);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::ExpandQuantityFlipsSymmetricalCopies()
{
    // the same expansion the local nester does: every odd copy of a symmetrical, force-flipped piece is unflipped
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
void TST_VNestingFrame::RequestSkipsByIndex()
{
    // two pieces share a name; only the degenerate one is skipped, by position
    QVector<int> skipped;
    VNestingPieceInput twin = Input("Detail");
    twin.contourPx = {{0, 0}, {10, 10}};
    const QJsonObject req = NestingRequest({Input("Detail"), twin}, 1500, 2, {NestingRotations::ZeroOnly, 60}, skipped);
    QCOMPARE(skipped, QVector<int>({1}));
    QCOMPARE(req.value("pieces").toArray().size(), 1);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::ContourUsesSeamAllowance()
{
    // the outline sent is the one the local nester collides on: the seam allowance when there is one
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
void TST_VNestingFrame::RequestWritesOptions()
{
    QCOMPARE(NestingSpeedString(NestingSpeed::Normal), QStringLiteral("normal"));
    QCOMPARE(NestingSpeedString(NestingSpeed::Fast), QStringLiteral("fast"));
    QCOMPARE(NestingSpeedString(NestingSpeed::Ultra), QStringLiteral("ultra"));

    QVector<int> skipped;
    VNestingOptions options;
    options.rotations = NestingRotations::ZeroOnly;
    options.budgetTokens = 120;
    options.speed = NestingSpeed::Fast;
    options.targetLengthMm = 1200.5;
    options.untilConverged = true;
    options.preview = true;
    const QJsonObject req = NestingRequest({Input("A")}, 1500, 2, options, skipped);
    QCOMPARE(req.value("budget_tokens").toInt(), 120); // spec 20: the budget is in tokens, time is no cap
    QCOMPARE(req.value("time_s").toInt(), 0);
    QCOMPARE(req.value("speed").toString(), QStringLiteral("fast"));
    QCOMPARE(req.value("target_length_mm").toDouble(), 1200.5);
    QCOMPARE(req.value("until_converged").toBool(), true);
    QCOMPARE(req.value("preview").toBool(), true);

    // defaults: no target key at all (the service rejects 0), normal speed, nothing early, not a preview
    const QJsonObject plain = NestingRequest({Input("A")}, 1500, 2, VNestingOptions{}, skipped);
    QVERIFY(not plain.contains("target_length_mm"));
    QVERIFY(not plain.contains("budget_tokens")); // no budget: unlimited, the service bills the work done
    QCOMPARE(plain.value("time_s").toInt(), 0);
    QCOMPARE(plain.value("speed").toString(), QStringLiteral("normal"));
    QCOMPARE(plain.value("until_converged").toBool(), false);
    QCOMPARE(plain.value("preview").toBool(), false);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::RequestArea()
{
    // Input("A") is a 100 x 50 mm rectangle (see Input): two copies plus a skipped degenerate piece
    QVector<int> skipped;
    VNestingPieceInput two = Input("A");
    two.quantity = 2;
    VNestingPieceInput flat = Input("flat");
    flat.contourPx = {{0, 0}, {10, 0}, {20, 0}};
    const QJsonObject req = NestingRequest({two, Input("B"), flat}, 1500, 2, VNestingOptions{}, skipped);
    QCOMPARE(NestingRequestArea(req), 3 * 100.0 * 50.0);
    QCOMPARE(NestingRequestArea(QJsonObject{}), 0.0);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::RequestWritesRolls()
{
    QVector<int> skipped;
    VNestingOptions options;
    options.rollsMm = {3000, 2500};
    options.rollJunctionMm = 80;
    const QJsonObject req = NestingRequest({Input("A")}, 1500, 2, options, skipped);
    QCOMPARE(req.value("rolls_mm").toArray(), (QJsonArray{3000.0, 2500.0}));
    QCOMPARE(req.value("roll_junction_mm").toDouble(), 80.0);
    // no rolls: neither key, the service would reject an empty list
    const QJsonObject plain = NestingRequest({Input("A")}, 1500, 2, VNestingOptions{}, skipped);
    QVERIFY(not plain.contains("rolls_mm") && not plain.contains("roll_junction_mm"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::RequestMergesIdenticalPieces()
{
    // identical pieces travel as one item with a quantity (the service's distinct-piece limit, and the service is
    // faster with quantities); a different group keeps its own item. itemOf maps every input to its item, -1 when skipped.
    QVector<int> skipped, itemOf;
    VNestingPieceInput other = Input("A");
    other.group = QStringLiteral("size 40");
    VNestingPieceInput flat = Input("C");
    flat.contourPx = {{0, 0}, {10, 10}};
    const QJsonObject req = NestingRequest({Input("A"), Input("B"), flat, Input("A"), other},
                                           1500,
                                           2,
                                           {NestingRotations::ZeroOnly, 60},
                                           skipped,
                                           &itemOf);
    const QJsonArray pieces = req.value("pieces").toArray();
    QCOMPARE(pieces.size(), 3);
    QCOMPARE(pieces.at(0).toObject().value("quantity").toInt(), 2);
    QCOMPARE(pieces.at(2).toObject().value("group").toString(), QStringLiteral("size 40"));
    QCOMPARE(skipped, QVector<int>({2}));
    QCOMPARE(itemOf, QVector<int>({0, 1, -1, 0, 2}));

    // coordinates are sent rounded to a thousandth of a millimetre: the body of a large layout counts
    VNestingPieceInput tiny = Input("T");
    tiny.contourPx = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    const QJsonArray poly = NestingRequest({tiny}, 1500, 2, {NestingRotations::ZeroOnly, 60}, skipped)
                                .value("pieces")
                                .toArray()
                                .at(0)
                                .toObject()
                                .value("polygon")
                                .toArray();
    QCOMPARE(qAbs(poly.at(2).toArray().at(0).toDouble()), 0.265); // 1 px = 0.264583 mm
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::ResultPapersSplitByRoll()
{
    // the service answers with the roll of every copy and the rolls' start and used length (spec 13.2)
    const QJsonObject result{
        {"marker_length_mm", 500},
        {"placements",
         QJsonArray{
             QJsonObject{{"piece_id", 0}, {"x", 0}, {"y", 50}, {"rotation_deg", 0}, {"mirrored", false}, {"roll", 0}},
             QJsonObject{{"piece_id", 0}, {"x", 350}, {"y", 50}, {"rotation_deg", 0}, {"mirrored", false}, {"roll", 1}}}},
        {"rolls",
         QJsonArray{QJsonObject{{"index", 0}, {"start_mm", 0}, {"length_mm", 100}},
                    QJsonObject{{"index", 1}, {"start_mm", 300}, {"length_mm", 150}}}}};
    const QMultiMap<int, VNestingPlacement> placements = NestingPlacements(result);
    QCOMPARE(placements.size(), 2);
    QCOMPARE(placements.values(0).at(0).roll + placements.values(0).at(1).roll, 1);

    const QVector<VLayoutPaper> papers = NestingResultPapers({RectPiece()}, placements, result, 1500 * k, 0, false);
    QCOMPARE(papers.size(), 2);
    QCOMPARE(papers.at(0).GetHeight(), qCeil(100 * k));
    QCOMPARE(papers.at(1).GetHeight(), qCeil(150 * k));
    QCOMPARE(papers.at(1).GetDetails().size(), 1);
    // the copy on roll 1 is placed relative to that roll's start: strip x 350 - 300 = 50 mm from the paper top
    const QRectF r = papers.at(1).GetDetails().at(0).MappedDetailBoundingRect();
    QVERIFY2(qAbs(r.top() - 50 * k) < 1e-6, qPrintable(QString::number(r.top())));

    // without rolls: one paper of the marker length, as before
    QJsonObject single = result;
    single.remove(QStringLiteral("rolls"));
    QCOMPARE(NestingResultPapers({RectPiece()}, placements, single, 1500 * k, 0, false).size(), 1);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::RequestGroupsAndFixed()
{
    VNestingPieceInput a = Input("A");
    a.group = QStringLiteral("jacket");
    VNestingPieceInput f = Input("F");
    f.fixedMm = {QPointF(10, 5), QPointF(110, 5), QPointF(110, 53), QPointF(10, 53)};
    VNestingOptions options{NestingRotations::ZeroAnd180, 60};
    options.groupByGroup = true;
    QVector<int> skipped;
    const QJsonObject req = NestingRequest({a, f}, 1500, 2, options, skipped);
    QVERIFY(skipped.isEmpty());
    QVERIFY(req.value("group_by_group").toBool());
    const QJsonArray pieces = req.value("pieces").toArray();
    QCOMPARE(pieces.at(0).toObject().value("group").toString(), QStringLiteral("jacket"));
    QVERIFY(not pieces.at(0).toObject().contains("fixed"));
    const QJsonObject fixed = pieces.at(1).toObject();
    QVERIFY(fixed.value("fixed").toBool());
    QVERIFY(not fixed.contains("rotations"));
    QCOMPARE(fixed.value("polygon").toArray().at(2).toArray().at(0).toDouble(), 110.0);
    QCOMPARE(fixed.value("polygon").toArray().at(2).toArray().at(1).toDouble(), 53.0);
    QVERIFY(not NestingRequest({a}, 1500, 2, {NestingRotations::ZeroAnd180, 60}, skipped).contains("group_by_group"));
    QVERIFY(not req.contains("no_backfill"));
    options.noBackfill = true;
    QVERIFY(NestingRequest({a, f}, 1500, 2, options, skipped).value("no_backfill").toBool());
    options.groupByGroup = false; // meaningless without the sequencing: not sent
    QVERIFY(not NestingRequest({a, f}, 1500, 2, options, skipped).contains("no_backfill"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VNestingFrame::FixedPolygonRoundTrip()
{
    // a piece placed by a result, read back as a fixed polygon, is the request polygon moved by the placement
    const QRectF margins(100, 200, 4000, 6000);
    for (const bool vertical : {true, false})
    {
        const VNestingPlacement p{300, 40, 90, true};
        const QTransform base;
        const QVector<QPointF> local = RectPx();
        const QVector<QPointF> onSheet = NestingPieceMatrix(base, p, margins, vertical).map(QPolygonF(local));
        const QVector<QPointF> back = NestingFixedPolygon(onSheet, margins, vertical);
        const QVector<QPointF> expected = NestingPlacementMatrix(p).map(
            QPolygonF((base * NestingLocalToStrip()).map(QPolygonF(local))));
        QCOMPARE(back.size(), expected.size());
        for (int i = 0; i < back.size(); ++i)
        {
            QVERIFY2(qAbs(back.at(i).x() - expected.at(i).x()) < 1e-6, qPrintable(QString::number(i)));
            QVERIFY2(qAbs(back.at(i).y() - expected.at(i).y()) < 1e-6, qPrintable(QString::number(i)));
        }
    }
}
