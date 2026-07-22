/************************************************************************
 **
 **  @file   tst_vfoldline.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 7, 2026
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

#include "tst_vfoldline.h"
#include "../vlayout/vfoldline.h"
#include "../vmisc/svgfont/vsvgfontdatabase.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"

#include <cstdio>

#include <QGraphicsSimpleTextItem>
#include <QLineF>
#include <QScopeGuard>
#include <QtTest>

// ponytail: marker to localize a Windows-CI-only crash inside this test class; revert once found
namespace
{
void TraceRow(const char *function)
{
    const char *tag = QTest::currentDataTag();
    std::fprintf(stdout, "[tst_vfoldline] running %s [%s]\n", function, tag ? tag : "");
    std::fflush(stdout);
}

void TraceRowDone(const char *function)
{
    const char *tag = QTest::currentDataTag();
    std::fprintf(stdout, "[tst_vfoldline] done %s [%s]\n", function, tag ? tag : "");
    std::fflush(stdout);
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
TST_VFoldLine::TST_VFoldLine(QObject *parent)
  : AbstractTest(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
// Regression test for a bug where a flipped piece's fold-line label swung away from its intended
// anchor as the fold line's own angle changed, instead of pivoting rigidly on it (the local
// mirror-correction transform was sandwiched around the rotation instead of composed before it).
// This check does not depend on exact font metrics or pixel positions -- only on the label's
// anchor staying at a *constant* distance from labelPos.pos regardless of the fold line's angle.
void TST_VFoldLine::LabelStaysAnchoredAcrossAngles_data() const
{
    QTest::addColumn<bool>("verticallyFlipped");
    QTest::addColumn<bool>("horizontallyFlipped");

    QTest::newRow("vertically flipped") << true << false;
    QTest::newRow("horizontally flipped") << false << true;
    QTest::newRow("both flipped") << true << true;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VFoldLine::LabelStaysAnchoredAcrossAngles() const
{
    TraceRow("LabelStaysAnchoredAcrossAngles");
    QFETCH(bool, verticallyFlipped);
    QFETCH(bool, horizontallyFlipped);

    const QPointF center(500, 500);
    const qreal foldLineLength = 800;

    QVector<qreal> distances;

    // Sweep a fixed-length fold line through several angles, all pivoting around the same
    // center point, so the ONLY thing that changes between iterations is the angle -- position,
    // width and label text stay identical, isolating the angle as the variable under test.
    const QVector<qreal> angles{0., 30., 90., 150., 200., 260., 310.};
    for (qreal angleDeg : angles)
    {
        // Build a half-line from center, rotate it to angleDeg via QLineF::setAngle(), then
        // mirror it through center to get the full-length line. This keeps the fold line
        // centered on `center` for every angle instead of swinging its midpoint around.
        QLineF half(center, center + QPointF(foldLineLength / 2., 0));
        half.setAngle(angleDeg);
        QLineF const foldLine(2 * center - half.p2(), half.p2());

        VFoldLine foldLineObj(foldLine, FoldLineType::TwoArrowsTextAbove);
        foldLineObj.SetLabel(QStringLiteral("Test label"));
        foldLineObj.SetOutlineFont(QFont());
        foldLineObj.SetVerticallyFlipped(verticallyFlipped);
        foldLineObj.SetHorizontallyFlipped(horizontallyFlipped);

        // LabelPosition() computes labelPos.pos/angle straight from the geometry above (no
        // rendering involved yet) -- this is the anchor the label transform is supposed to
        // pivot on.
        bool ok = false;
        FoldLabelPosData const posData = foldLineObj.LabelPosition(ok);
        QVERIFY(ok);

        // UpdateFoldLineLabel() is the actual production code path under test: it builds the
        // QGraphicsSimpleTextItem's transform (position + rotation + flip correction) exactly
        // as the real piece-rendering code does.
        QGraphicsSimpleTextItem item;
        foldLineObj.UpdateFoldLineLabel(&item);

        // The item's own bounding-rect center is the point the flip correction anchors on (see
        // UpdateFoldLineLabel): mapping it through the item's transform should land at a
        // constant distance from labelPos.pos regardless of angle, the same way it would for the
        // non-flipped case (rotation alone preserves distance from the rotation's own center).
        QPointF const anchor = item.transform().map(item.boundingRect().center());
        distances.append(QLineF(anchor, posData.pos).length());
    }

    // A correct transform pivots this anchor rigidly around labelPos.pos: rotating the fold
    // line changes WHERE the anchor sits, but not HOW FAR it sits from pos. So every recorded
    // distance should equal the first one. The buggy version instead let the flip correction's
    // shift get swept up by the rotation, making the distance balloon or shrink with angle.
    for (int i = 1; i < distances.size(); ++i)
    {
        QVERIFY2(qAbs(distances.at(i) - distances.constFirst()) < 1.,
                 qUtf8Printable(QStringLiteral("Label anchor distance from labelPos.pos must stay constant as the "
                                                "fold line's angle changes (angle %1: got %2, expected %3)")
                                    .arg(angles.at(i))
                                    .arg(distances.at(i))
                                    .arg(distances.constFirst())));
    }

    TraceRowDone("LabelStaysAnchoredAcrossAngles");
}

//---------------------------------------------------------------------------------------------------------------------
// Regression test for a bug where a flipped piece's fold-line label was mirror-corrected for
// readability but still ended up rotated by an uncontrolled, angle-dependent amount relative to
// how the equivalent non-flipped label reads -- readable only by coincidence at a couple of
// special angles (e.g. 90/270), visibly upside down or skewed everywhere else. Renders the SAME
// fold line both normally and flipped (with m_matrix carrying the same mirror
// VLayoutPiece::Mirror()/FlipVertically()/FlipHorizontally() bakes in), then checks the flipped
// item's linear (rotation) transform part against an exactly-computed expected one.
//
// The expected rotation offset is NOT symmetric between the two flip axes -- this is the crux of
// a real regression, not a simplifying assumption, so it deserves emphasis: an earlier version of
// this test (and the code it was checking) assumed vertical-only and horizontal-only flips should
// behave the same way and unified them onto one shared offset. That assumption was verified
// against exactly one case (vertical flip, via a real two-piece "cut on fold, mirrored" render)
// and silently generalized to horizontal flip without checking it independently -- which was
// wrong. Only vertical flip (and "both flipped", which needs no correction at all: two orthogonal
// mirrors compose to a pure 180-degree rotation, scale(-1,-1) is exactly Rotate(180), and the
// label just follows the piece's own rotation) actually wants the 180-degree offset. Horizontal
// flip wants 0 -- matching the non-flipped case exactly, the same way "both flipped" and vertical
// flip happen to match each other. This traces back to TwoArrowsTextAbove's label offset being an
// inherently Y-directional ("above" the fold line) concept: a Y-mirror (horizontal flip)
// interacts with that offset differently than an X-mirror (vertical flip) does. Each of the three
// rows below was independently confirmed against a real VPGraphicsPiece render (not derived from
// one and generalized to the others) before being encoded here -- see vfoldline.cpp's
// UpdateFoldLineLabel() for where this is implemented.
//
// Why compare raw 2x2 matrices instead of an "up vector" dot product: for a single flip, the
// local mirror correction is built so that, algebraically, m_matrix's flip and the correction's
// own scale cancel completely, leaving the flipped item's linear part EXACTLY equal to itemA's
// own linear part rotated by the expected offset (proven by construction: the scale is applied
// after the rotation, not before it, so it composes with m_matrix's own mirror into the identity
// -- verified against an asymmetric test shape at every 45-degree step in a standalone check, not
// just this test's angle sweep). An earlier version of this test instead mirrored one label's
// "up" vector and dot-producted it against the other's; that looked plausible but was
// mathematically unsound -- for a vertical "up" vector rotated by Rotate(-angle) and then
// mirrored, the dot product against the unmirrored vector works out to sin^2-cos^2 of the angle,
// which flips sign every 45 degrees regardless of whether the rendering is actually correct, so
// it could not reliably tell a correct transform from a broken one.
void TST_VFoldLine::LabelOrientationMatchesNonFlipped_data() const
{
    QTest::addColumn<bool>("verticallyFlipped");
    QTest::addColumn<bool>("horizontallyFlipped");
    QTest::addColumn<qreal>("mirrorScaleX");
    QTest::addColumn<qreal>("mirrorScaleY");
    QTest::addColumn<qreal>("expectedExtraRotationDeg");

    // mirrorScaleX/Y is the m_matrix mirror VLayoutPiece::Mirror()/FlipVertically() (scale
    // (-1,1)) or FlipHorizontally() (scale(1,-1)) would actually bake in for that flip state.
    QTest::newRow("vertically flipped") << true << false << -1. << 1. << 180.;
    QTest::newRow("horizontally flipped") << false << true << 1. << -1. << 0.;
    QTest::newRow("both flipped") << true << true << -1. << -1. << 180.;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VFoldLine::LabelOrientationMatchesNonFlipped() const
{
    TraceRow("LabelOrientationMatchesNonFlipped");
    QFETCH(bool, verticallyFlipped);
    QFETCH(bool, horizontallyFlipped);
    QFETCH(qreal, mirrorScaleX);
    QFETCH(qreal, mirrorScaleY);
    QFETCH(qreal, expectedExtraRotationDeg);

    const QPointF center(500, 500);
    const qreal foldLineLength = 800;

    QTransform mirror;
    mirror.scale(mirrorScaleX, mirrorScaleY);

    // The fold line's own local angle (independent of any piece rotation applied later via
    // m_matrix) determines which quadrant labelPos.angle falls into -- a correction that only
    // works for one specific angle would let this bug back in for other fold-line orientations,
    // which is exactly how the real regression slipped past a single-angle check.
    const QVector<qreal> angles{0., 45., 90., 135., 180., 225., 270., 315.};
    for (qreal angleDeg : angles)
    {
        QLineF half(center, center + QPointF(foldLineLength / 2., 0));
        half.setAngle(angleDeg);
        QLineF const foldLine(2 * center - half.p2(), half.p2());

        // Item A: the trusted-correct reference. Same fold line, m_matrix left at identity (no
        // flip), so its rendering is never subject to the bug under test.
        VFoldLine nonFlipped(foldLine, FoldLineType::TwoArrowsTextAbove);
        nonFlipped.SetLabel(QStringLiteral("Test label"));
        nonFlipped.SetOutlineFont(QFont());

        QGraphicsSimpleTextItem itemA;
        nonFlipped.UpdateFoldLineLabel(&itemA);

        // Item B: the SAME fold line, but flipped, with m_matrix set to the mirror this flip
        // state implies. This mimics how VLayoutPiece actually wires up a flipped piece: same
        // local geometry, only the flip flags and m_matrix differ.
        VFoldLine flipped(foldLine, FoldLineType::TwoArrowsTextAbove);
        flipped.SetLabel(QStringLiteral("Test label"));
        flipped.SetOutlineFont(QFont());
        flipped.SetVerticallyFlipped(verticallyFlipped);
        flipped.SetHorizontallyFlipped(horizontallyFlipped);
        flipped.SetMatrix(mirror);

        QGraphicsSimpleTextItem itemB;
        flipped.UpdateFoldLineLabel(&itemB);

        // itemA's own linear part is whatever LabelPosition() decided for this fold line (its
        // angle need not equal the raw sweep angle -- LabelPosition() derives it from the fold
        // line's geometry, not the angle passed to QLineF::setAngle() above). Build "expected" by
        // taking itemA's actual transform and appending the extra rotation the flip is allowed to
        // introduce, rather than recomputing -angleDeg from scratch and risking a mismatch with
        // whatever convention LabelPosition() actually uses.
        QTransform extraRotation;
        extraRotation.rotate(expectedExtraRotationDeg);
        QTransform const expected = itemA.transform() * extraRotation;

        const QTransform &actual = itemB.transform();
        QVERIFY2(qAbs(actual.m11() - expected.m11()) < 1e-6 && qAbs(actual.m12() - expected.m12()) < 1e-6 &&
                     qAbs(actual.m21() - expected.m21()) < 1e-6 && qAbs(actual.m22() - expected.m22()) < 1e-6,
                 qUtf8Printable(QStringLiteral("Flipped label's linear transform must equal the non-flipped "
                                                "label's rotated by %1 degrees at fold-line angle %2 (got [%3 %4 / "
                                                "%5 %6], expected [%7 %8 / %9 %10]) -- otherwise the label renders "
                                                "upside down or off-angle.")
                                    .arg(expectedExtraRotationDeg)
                                    .arg(angleDeg)
                                    .arg(actual.m11())
                                    .arg(actual.m12())
                                    .arg(actual.m21())
                                    .arg(actual.m22())
                                    .arg(expected.m11())
                                    .arg(expected.m12())
                                    .arg(expected.m21())
                                    .arg(expected.m22())));

        // The glyph itself must not be mirror-imaged (unreadable backwards letters): a positive
        // determinant means the transform preserves handedness (no net reflection). This is
        // implied by the exact-match check above, but kept as an independent, simpler sanity
        // check in case the tolerance above is ever loosened.
        auto Determinant = [](const QTransform &t) { return t.m11() * t.m22() - t.m12() * t.m21(); };
        QVERIFY2(Determinant(itemB.transform()) > 0,
                 qUtf8Printable(QStringLiteral("Flipped label's glyphs must not be mirror-imaged at fold-line angle "
                                                "%1 -- the local mirror correction must cancel m_matrix's mirror.")
                                    .arg(angleDeg)));
    }

    TraceRowDone("LabelOrientationMatchesNonFlipped");
}

//---------------------------------------------------------------------------------------------------------------------
// Regression test for a bug where a flipped label sat an extra height's worth of distance away
// from the fold line itself -- constant across angles (so LabelStaysAnchoredAcrossAngles alone
// would not catch it), and even reproducible by a specific corner of the item's bounding rect
// still landing exactly on labelPos.pos (so an earlier, weaker version of this test that only
// checked that did not catch it either: touching labelPos.pos does not by itself guarantee the
// label is as close to the *line* as the non-flipped case is, since pos->line distance can
// itself differ once the wrong corner is the one touching pos). This measures the label's
// closest point to the actual fold line (mapped through m_matrix) directly, and requires it to
// match the non-flipped label's closest distance to its own fold line.
void TST_VFoldLine::LabelStaysCloseToFoldLine_data() const
{
    QTest::addColumn<bool>("verticallyFlipped");
    QTest::addColumn<bool>("horizontallyFlipped");
    QTest::addColumn<qreal>("mirrorScaleX");
    QTest::addColumn<qreal>("mirrorScaleY");

    QTest::newRow("vertically flipped") << true << false << -1. << 1.;
    QTest::newRow("horizontally flipped") << false << true << 1. << -1.;
    QTest::newRow("both flipped") << true << true << -1. << -1.;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VFoldLine::LabelStaysCloseToFoldLine() const
{
    TraceRow("LabelStaysCloseToFoldLine");
    QFETCH(bool, verticallyFlipped);
    QFETCH(bool, horizontallyFlipped);
    QFETCH(qreal, mirrorScaleX);
    QFETCH(qreal, mirrorScaleY);

    const QPointF center(500, 500);
    const qreal foldLineLength = 800;

    QTransform mirror;
    mirror.scale(mirrorScaleX, mirrorScaleY);

    const QVector<qreal> angles{0., 45., 90., 135., 180., 225., 270., 315.};
    for (qreal angleDeg : angles)
    {
        QLineF half(center, center + QPointF(foldLineLength / 2., 0));
        half.setAngle(angleDeg);
        QLineF const foldLine(2 * center - half.p2(), half.p2());

        // Reference: same fold line, not flipped, m_matrix left at identity.
        VFoldLine nonFlipped(foldLine, FoldLineType::TwoArrowsTextAbove);
        nonFlipped.SetLabel(QStringLiteral("Test label"));
        nonFlipped.SetOutlineFont(QFont());

        QGraphicsSimpleTextItem itemA;
        nonFlipped.UpdateFoldLineLabel(&itemA);

        // Same fold line again, but flipped with m_matrix set to the corresponding mirror.
        VFoldLine flipped(foldLine, FoldLineType::TwoArrowsTextAbove);
        flipped.SetLabel(QStringLiteral("Test label"));
        flipped.SetOutlineFont(QFont());
        flipped.SetVerticallyFlipped(verticallyFlipped);
        flipped.SetHorizontallyFlipped(horizontallyFlipped);
        flipped.SetMatrix(mirror);

        QGraphicsSimpleTextItem itemB;
        flipped.UpdateFoldLineLabel(&itemB);

        // Comparing only perpendicular distance to the infinite fold line -- as an earlier
        // version of this test did -- cannot catch a label that has drifted sideways *along* the
        // line's own direction: a footprint translated by a whole label-width still projects to
        // a small perpendicular distance from that infinite line, so that kind of bug sailed
        // right through. Comparing the whole footprint's center against where the non-flipped
        // footprint's center lands once mapped through the SAME mirror catches both the
        // perpendicular (distance-to-line) and along-the-line (width) axes at once.
        QPointF const centerA = itemA.transform().map(itemA.boundingRect().center());
        QPointF const centerB = itemB.transform().map(itemB.boundingRect().center());
        QPointF const expectedCenterB = mirror.map(centerA);

        QVERIFY2(QLineF(centerB, expectedCenterB).length() < 1.,
                 qUtf8Printable(QStringLiteral("Flipped label's footprint center must land where the non-flipped "
                                                "label's footprint center does once mapped through the same "
                                                "mirror, at fold-line angle %1 (expected (%2, %3), got (%4, %5)) "
                                                "-- a mismatch means the label sits an extra label-width/height "
                                                "away from the fold line, or drifted sideways along it, instead "
                                                "of hugging it like the non-flipped case.")
                                    .arg(angleDeg)
                                    .arg(expectedCenterB.x())
                                    .arg(expectedCenterB.y())
                                    .arg(centerB.x())
                                    .arg(centerB.y())));
    }

    TraceRowDone("LabelStaysCloseToFoldLine");
}

//---------------------------------------------------------------------------------------------------------------------
// Regression test for a bug where, with single-stroke outline fonts enabled (the setting that
// makes CreateFoldLineItem() draw the label as a QPainterPath via VFoldLine::OutlineFontLabel()
// instead of a QGraphicsSimpleTextItem via UpdateFoldLineLabel()), a flipped label's path landed
// far from the fold line. This went through two rounds: the perpendicular distance to the fold
// line was wrong first (missing footprint-realigning shift for the horizontally flipped case);
// once that was fixed, the label was still offset sideways *along* the fold line's own direction
// by roughly a whole label width, because the horizontally flipped branch carried a spurious
// "+180" rotation offset together with a shift built for the wrong axis (copied from the
// vertically flipped branch's width-based shift instead of an analogous height-based one for its
// own, differently oriented mirror). This exercises the real, path-producing entry point
// (FoldLinePath(), the same one CreateFoldLineItem() calls) rather than UpdateFoldLineLabel(),
// toggling the single-stroke-outline-font setting on for the duration since that's what selects
// the affected code path.
void TST_VFoldLine::LabelPathStaysCloseToFoldLine_data() const
{
    QTest::addColumn<bool>("verticallyFlipped");
    QTest::addColumn<bool>("horizontallyFlipped");
    QTest::addColumn<qreal>("mirrorScaleX");
    QTest::addColumn<qreal>("mirrorScaleY");

    QTest::newRow("vertically flipped") << true << false << -1. << 1.;
    QTest::newRow("horizontally flipped") << false << true << 1. << -1.;
    QTest::newRow("both flipped") << true << true << -1. << -1.;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VFoldLine::LabelPathStaysCloseToFoldLine() const
{
    TraceRow("LabelPathStaysCloseToFoldLine");
    QFETCH(bool, verticallyFlipped);
    QFETCH(bool, horizontallyFlipped);
    QFETCH(qreal, mirrorScaleX);
    QFETCH(qreal, mirrorScaleY);

    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    bool const wasSingleStroke = settings->GetSingleStrokeOutlineFont();
    settings->SetSingleStrokeOutlineFont(true);
    auto restoreSettings = qScopeGuard([settings, wasSingleStroke]()
                                        { settings->SetSingleStrokeOutlineFont(wasSingleStroke); });

    const QPointF center(500, 500);
    const qreal foldLineLength = 800;

    QTransform mirror;
    mirror.scale(mirrorScaleX, mirrorScaleY);

    const QVector<qreal> angles{0., 45., 90., 135., 180., 225., 270., 315.};
    for (qreal angleDeg : angles)
    {
        QLineF half(center, center + QPointF(foldLineLength / 2., 0));
        half.setAngle(angleDeg);
        QLineF const foldLine(2 * center - half.p2(), half.p2());

        // Reference: same fold line, not flipped, m_matrix left at identity.
        VFoldLine nonFlipped(foldLine, FoldLineType::TwoArrowsTextAbove);
        nonFlipped.SetLabel(QStringLiteral("Test label"));
        nonFlipped.SetOutlineFont(QFont());

        // FoldLinePath() returns [0]=arrows mark, [1]=label path for this fold-line type; the
        // label path is always present regardless of settings (only *which* internal function
        // builds it depends on GetSingleLineFonts()).
        QVector<QPainterPath> const shapeA = nonFlipped.FoldLinePath();
        QVERIFY(shapeA.size() > 1);

        // Same fold line again, but flipped with m_matrix set to the corresponding mirror.
        VFoldLine flipped(foldLine, FoldLineType::TwoArrowsTextAbove);
        flipped.SetLabel(QStringLiteral("Test label"));
        flipped.SetOutlineFont(QFont());
        flipped.SetVerticallyFlipped(verticallyFlipped);
        flipped.SetHorizontallyFlipped(horizontallyFlipped);
        flipped.SetMatrix(mirror);

        QVector<QPainterPath> const shapeB = flipped.FoldLinePath();
        QVERIFY(shapeB.size() > 1);

        // Checking only perpendicular distance to the (unclamped, infinite) line -- as an
        // earlier version of this test did -- cannot catch a label that has drifted sideways
        // *along* the line's own direction: a footprint translated by a whole label-width still
        // projects to a small perpendicular distance from that infinite line, so that kind of
        // bug sailed right through. Comparing the whole footprint's center against where the
        // non-flipped footprint's center lands once mapped through the SAME mirror catches both
        // the perpendicular (distance-to-line) and along-the-line (width) axes at once.
        //
        // Tolerance is a few pixels, not the sub-pixel one used elsewhere: QPainterPath::
        // boundingRect() is a fast bound over a curve's/stroke's control points, not the curve's
        // exact geometric extent, and that control-point bound isn't perfectly rotation-
        // equivariant -- rotating the path and then taking its bounding rect doesn't always land
        // in exactly the same place as taking the bounding rect and then rotating that. That
        // shows up as a fixed few-pixel gap only at non-axis-aligned angles (confirmed by
        // sweeping every angle here: exactly 0 at 0/90/180/270, a small constant at 45-degree
        // multiples) -- real position bugs (like the ones this test was written to catch) are
        // two orders of magnitude larger and show up at every angle.
        QPointF const centerA = shapeA.constLast().boundingRect().center();
        QPointF const centerB = shapeB.constLast().boundingRect().center();
        QPointF const expectedCenterB = mirror.map(centerA);

        QVERIFY2(QLineF(centerB, expectedCenterB).length() < 5.,
                 qUtf8Printable(QStringLiteral("Flipped label path's footprint center must land where the non-"
                                                "flipped label path's footprint center does once mapped through "
                                                "the same mirror, at fold-line angle %1 (expected (%2, %3), got "
                                                "(%4, %5)) -- a mismatch along the fold line's own direction means "
                                                "the label drifted sideways (e.g. by a whole label width) even "
                                                "though its distance to the line looks fine.")
                                    .arg(angleDeg)
                                    .arg(expectedCenterB.x())
                                    .arg(expectedCenterB.y())
                                    .arg(centerB.x())
                                    .arg(centerB.y())));
    }

    TraceRowDone("LabelPathStaysCloseToFoldLine");
}

//---------------------------------------------------------------------------------------------------------------------
// Regression test for a bug where, with SVG single-line fonts enabled (GetSingleLineFonts(),
// which routes CreateFoldLineItem()/FoldLinePath() through VFoldLine::SVGFontLabel() instead of
// OutlineFontLabel()), a horizontally flipped label's path landed far from the fold line --
// SVGFontLabel() carries the same "rotate about a pivot, mirror correction innermost" structure
// as OutlineFontLabel()'s single-stroke branch, and had the same two-round bug in its
// horizontally flipped case (see LabelPathStaysCloseToFoldLine): first a missing footprint shift,
// then, once that was fixed, a spurious "+180" rotation plus a width-based shift left over from
// the vertically flipped branch instead of an analogous height-based one, which pushed the label
// sideways along the fold line by roughly a whole label width even though its distance to the
// line measured correctly. LabelPathStaysCloseToFoldLine never exercised this function at all
// (it only toggles SetSingleStrokeOutlineFont(), never SetSingleLineFonts()).
void TST_VFoldLine::SVGLabelPathStaysCloseToFoldLine_data() const
{
    QTest::addColumn<bool>("verticallyFlipped");
    QTest::addColumn<bool>("horizontallyFlipped");
    QTest::addColumn<qreal>("mirrorScaleX");
    QTest::addColumn<qreal>("mirrorScaleY");

    QTest::newRow("vertically flipped") << true << false << -1. << 1.;
    QTest::newRow("horizontally flipped") << false << true << 1. << -1.;
    QTest::newRow("both flipped") << true << true << -1. << -1.;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VFoldLine::SVGLabelPathStaysCloseToFoldLine() const
{
    TraceRow("SVGLabelPathStaysCloseToFoldLine");
    QFETCH(bool, verticallyFlipped);
    QFETCH(bool, horizontallyFlipped);
    QFETCH(qreal, mirrorScaleX);
    QFETCH(qreal, mirrorScaleY);

    // SVGFontLabel() silently falls back to OutlineFontLabel() when no SVG font is available
    // (LabelSVGFontEngine() returns false), so this test needs a real, loadable SVG font to
    // actually exercise the code path under test. Use the repo's own share/svgfonts (shipped
    // with the app, so always present and portable to any checkout/CI machine) via
    // QFINDTESTDATA, which resolves relative to this source file in both in-tree and shadow
    // builds.
    QString const fontDir = QFINDTESTDATA("../../app/share/svgfonts");
    QVERIFY2(!fontDir.isEmpty(), "Could not locate share/svgfonts test data.");

    VSvgFontDatabase *db = VAbstractApplication::VApp()->SVGFontDatabase();
    db->PopulateFontDatabase(fontDir);
    QStringList const families = db->Families();
    if (families.isEmpty())
    {
        QSKIP("No SVG font available to exercise SVGFontLabel() with.");
    }
    QString const fontFamily = families.constFirst();

    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    bool const wasSingleLine = settings->GetSingleLineFonts();
    settings->SetSingleLineFonts(true);
    auto restoreSettings = qScopeGuard([settings, wasSingleLine]()
                                        { settings->SetSingleLineFonts(wasSingleLine); });

    const QPointF center(500, 500);
    const qreal foldLineLength = 800;

    QTransform mirror;
    mirror.scale(mirrorScaleX, mirrorScaleY);

    const QVector<qreal> angles{0., 45., 90., 135., 180., 225., 270., 315.};
    for (qreal angleDeg : angles)
    {
        QLineF half(center, center + QPointF(foldLineLength / 2., 0));
        half.setAngle(angleDeg);
        QLineF const foldLine(2 * center - half.p2(), half.p2());

        // Reference: same fold line, not flipped, m_matrix left at identity.
        VFoldLine nonFlipped(foldLine, FoldLineType::TwoArrowsTextAbove);
        nonFlipped.SetLabel(QStringLiteral("Test label"));
        nonFlipped.SetSvgFont(fontFamily);

        QVector<QPainterPath> const shapeA = nonFlipped.FoldLinePath();
        QVERIFY(shapeA.size() > 1);

        // Same fold line again, but flipped with m_matrix set to the corresponding mirror.
        VFoldLine flipped(foldLine, FoldLineType::TwoArrowsTextAbove);
        flipped.SetLabel(QStringLiteral("Test label"));
        flipped.SetSvgFont(fontFamily);
        flipped.SetVerticallyFlipped(verticallyFlipped);
        flipped.SetHorizontallyFlipped(horizontallyFlipped);
        flipped.SetMatrix(mirror);

        QVector<QPainterPath> const shapeB = flipped.FoldLinePath();
        QVERIFY(shapeB.size() > 1);

        // See LabelPathStaysCloseToFoldLine: comparing only perpendicular distance to the
        // infinite line cannot catch a footprint that has drifted sideways along the line's own
        // direction (e.g. by a whole label width), which is exactly the class of bug this test
        // exists to catch. Comparing the footprint centers (flipped vs. non-flipped mapped
        // through the same mirror) catches both axes. Tolerance is a few pixels rather than
        // sub-pixel for the same reason as LabelPathStaysCloseToFoldLine: QPainterPath::
        // boundingRect() isn't perfectly rotation-equivariant for curved paths, which leaves a
        // fixed few-pixel gap only at non-axis-aligned angles -- two orders of magnitude smaller
        // than the sideways-drift bug this test exists to catch.
        QPointF const centerA = shapeA.constLast().boundingRect().center();
        QPointF const centerB = shapeB.constLast().boundingRect().center();
        QPointF const expectedCenterB = mirror.map(centerA);

        QVERIFY2(QLineF(centerB, expectedCenterB).length() < 5.,
                 qUtf8Printable(QStringLiteral("Flipped SVG label path's footprint center must land where the "
                                                "non-flipped label path's footprint center does once mapped "
                                                "through the same mirror, at fold-line angle %1 (expected (%2, "
                                                "%3), got (%4, %5)) -- a mismatch along the fold line's own "
                                                "direction means the label drifted sideways (e.g. by a whole "
                                                "label width) even though its distance to the line looks fine.")
                                    .arg(angleDeg)
                                    .arg(expectedCenterB.x())
                                    .arg(expectedCenterB.y())
                                    .arg(centerB.x())
                                    .arg(centerB.y())));
    }

    TraceRowDone("SVGLabelPathStaysCloseToFoldLine");
}



//---------------------------------------------------------------------------------------------------------------------
// Regression test closing a gap the earlier LabelPathStaysCloseToFoldLine/SVGLabelPathStaysCloseToFoldLine
// tests left open: those check the label PATH's footprint *position* but never its *orientation*
// (unlike LabelOrientationMatchesNonFlipped, which checks orientation but only for
// UpdateFoldLineLabel()'s QGraphicsSimpleTextItem path). A flipped path could pass both of those
// position checks while still reading upside down or at some other wrong rotation, exactly the
// class of bug that would slip through undetected.
//
// A QPainterPath has no associated QTransform to read back (unlike QGraphicsSimpleTextItem), so
// orientation is measured directly from the rendered geometry instead: pointAtPercent(0.0) and
// pointAtPercent(0.5) give two points spread across the label path's actual (asymmetric, for real
// text) outline. Because every transform involved here (rotation, axis-aligned scale) is an
// isometry, arc-length parametrization is preserved under it, so the vector between those two
// points on the flipped path must equal the SAME vector on the non-flipped path, rotated by
// whatever extra rotation this flip combination is expected to introduce -- 180 degrees for
// vertical-only and "both flipped", but 0 for horizontal-only; see LabelOrientationMatchesNonFlipped
// for why this is NOT symmetric between the two axes (a real regression, not a simplifying
// assumption -- an earlier version of this test got that wrong by generalizing from vertical flip
// alone). Using two arbitrary path points (rather than a symmetric bounding-rect corner pair)
// avoids the blind spot that let a real extra-rotation bug through undetected before: a rectangle
// has 180-degree symmetry, so a rectangle-based check cannot distinguish a correctly-oriented
// footprint from one rotated by exactly 180 degrees extra composed with a swapped corner --
// verified by reproducing that exact bug against a hand-built asymmetric shape before this fix,
// where the rectangle-only check still reported zero error.
void TST_VFoldLine::LabelPathOrientationMatchesNonFlipped_data() const
{
    QTest::addColumn<QString>("fontMode");
    QTest::addColumn<bool>("verticallyFlipped");
    QTest::addColumn<bool>("horizontallyFlipped");
    QTest::addColumn<qreal>("mirrorScaleX");
    QTest::addColumn<qreal>("mirrorScaleY");
    QTest::addColumn<qreal>("expectedExtraRotationDeg");

    for (const QString &mode : {QStringLiteral("singleStroke"), QStringLiteral("svg"), QStringLiteral("addText")})
    {
        QTest::addRow("%s, vertically flipped", qUtf8Printable(mode)) << mode << true << false << -1. << 1. << 180.;
        QTest::addRow("%s, horizontally flipped", qUtf8Printable(mode))
            << mode << false << true << 1. << -1. << 0.;
        QTest::addRow("%s, both flipped", qUtf8Printable(mode)) << mode << true << true << -1. << -1. << 180.;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VFoldLine::LabelPathOrientationMatchesNonFlipped() const
{
    TraceRow("LabelPathOrientationMatchesNonFlipped");
    QFETCH(QString, fontMode);
    QFETCH(bool, verticallyFlipped);
    QFETCH(bool, horizontallyFlipped);
    QFETCH(qreal, mirrorScaleX);
    QFETCH(qreal, mirrorScaleY);
    QFETCH(qreal, expectedExtraRotationDeg);

    QString fontFamily;
    if (fontMode == QStringLiteral("svg"))
    {
        QString const fontDir = QFINDTESTDATA("../../app/share/svgfonts");
        VSvgFontDatabase *db = VAbstractApplication::VApp()->SVGFontDatabase();
        db->PopulateFontDatabase(fontDir);
        QStringList const families = db->Families();
        if (families.isEmpty())
        {
            QSKIP("No SVG font available to exercise SVGFontLabel() with.");
        }
        fontFamily = families.constFirst();
    }

    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    bool const wasSingleStroke = settings->GetSingleStrokeOutlineFont();
    bool const wasSingleLine = settings->GetSingleLineFonts();
    settings->SetSingleStrokeOutlineFont(fontMode == QStringLiteral("singleStroke"));
    settings->SetSingleLineFonts(fontMode == QStringLiteral("svg"));
    auto restoreSettings = qScopeGuard(
        [settings, wasSingleStroke, wasSingleLine]()
        {
            settings->SetSingleStrokeOutlineFont(wasSingleStroke);
            settings->SetSingleLineFonts(wasSingleLine);
        });

    const QPointF center(500, 500);
    const qreal foldLineLength = 800;

    QTransform mirror;
    mirror.scale(mirrorScaleX, mirrorScaleY);

    // Two points spread across the label path's own (asymmetric, for real text) outline; see the
    // comment above LabelPathOrientationMatchesNonFlipped_data() for why arc-length percent is a
    // reliable, transform-preserved landmark here.
    auto LandmarkVector = [](const QPainterPath &path) -> QPointF
    { return path.pointAtPercent(0.5) - path.pointAtPercent(0.0); };

    const QVector<qreal> angles{0., 45., 90., 135., 180., 225., 270., 315.};
    for (qreal angleDeg : angles)
    {
        std::fprintf(stdout, "[tst_vfoldline]   angle=%g begin\n", angleDeg);
        std::fflush(stdout);

        QLineF half(center, center + QPointF(foldLineLength / 2., 0));
        half.setAngle(angleDeg);
        QLineF const foldLine(2 * center - half.p2(), half.p2());

        VFoldLine nonFlipped(foldLine, FoldLineType::TwoArrowsTextAbove);
        nonFlipped.SetLabel(QStringLiteral("Test label"));
        nonFlipped.SetOutlineFont(QFont());
        nonFlipped.SetSvgFont(fontFamily);
        QVector<QPainterPath> const shapeA = nonFlipped.FoldLinePath();
        QVERIFY(shapeA.size() > 1);

        std::fprintf(stdout, "[tst_vfoldline]   angle=%g nonFlipped done, calling flipped.FoldLinePath()\n",
                     angleDeg);
        std::fflush(stdout);

        VFoldLine flipped(foldLine, FoldLineType::TwoArrowsTextAbove);
        flipped.SetLabel(QStringLiteral("Test label"));
        flipped.SetOutlineFont(QFont());
        flipped.SetSvgFont(fontFamily);
        flipped.SetVerticallyFlipped(verticallyFlipped);
        flipped.SetHorizontallyFlipped(horizontallyFlipped);
        flipped.SetMatrix(mirror);
        QVector<QPainterPath> const shapeB = flipped.FoldLinePath();
        QVERIFY(shapeB.size() > 1);

        std::fprintf(stdout, "[tst_vfoldline]   angle=%g flipped.FoldLinePath() returned\n", angleDeg);
        std::fflush(stdout);

        QPointF const vecA = LandmarkVector(shapeA.constLast());
        QPointF const vecB = LandmarkVector(shapeB.constLast());

        QTransform extraRot;
        extraRot.rotate(expectedExtraRotationDeg);
        QPointF const expectedVecB = extraRot.map(vecA);

        // Signed angle between vecB and expectedVecB via atan2 of the cross/dot products -- robust
        // to the vectors' differing lengths (elided text width can differ slightly by construction
        // details) since only their relative angle is being checked.
        qreal const angleErrDeg =
            qRadiansToDegrees(qAtan2(vecB.x() * expectedVecB.y() - vecB.y() * expectedVecB.x(),
                                      vecB.x() * expectedVecB.x() + vecB.y() * expectedVecB.y()));

        QVERIFY2(qAbs(angleErrDeg) < 1.,
                 qUtf8Printable(QStringLiteral("Flipped label path must be rotated by %1 degrees relative to the "
                                                "non-flipped label path at fold-line angle %2 (got a %3 degree "
                                                "mismatch instead) -- otherwise the label renders upside down or "
                                                "at the wrong angle.")
                                    .arg(expectedExtraRotationDeg)
                                    .arg(angleDeg)
                                    .arg(angleErrDeg)));
    }

    TraceRowDone("LabelPathOrientationMatchesNonFlipped");
}
