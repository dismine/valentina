/************************************************************************
 **
 **  @file   tst_vlabelarrangeengine.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 5, 2026
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

#include "tst_vlabelarrangeengine.h"

#include <QtTest>

#include "../../libs/vwidgets/labelarrange/labelarrangetypes.h"
#include "../../libs/vwidgets/labelarrange/vlabelarrangeengine.h"

//---------------------------------------------------------------------------------------------------------------------
TST_VLabelArrangeEngine::TST_VLabelArrangeEngine(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VLabelArrangeEngine::TestArrange_Empty()
{
    const QVector<QPointF> result = VLabelArrangeEngine::Arrange({});
    QVERIFY(result.isEmpty());
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VLabelArrangeEngine::TestArrange_SingleLabel_NoCoverAnchor()
{
    LabelArrangeData d;
    d.anchor = QPointF(100, 100);
    d.labelSize = QSizeF(40, 15);
    d.currentOffset = QPointF(5, 5); // well away from anchor

    const QVector<QPointF> result = VLabelArrangeEngine::Arrange({d});
    QCOMPARE(result.size(), 1);

    // The single label has no competition; check it doesn't cover its anchor
    const QRectF placed{d.anchor + result.at(0), d.labelSize};
    const QRectF anchorZone{d.anchor - QPointF(4, 4), QSizeF(8, 8)};
    QVERIFY(!placed.intersects(anchorZone));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VLabelArrangeEngine::TestArrange_TwoNonOverlapping_Unchanged()
{
    // Place two labels far apart — engine should not bother moving them
    LabelArrangeData a;
    a.anchor = QPointF(0, 0);
    a.labelSize = QSizeF(40, 15);
    a.currentOffset = QPointF(50, 0); // label at (50,0)

    LabelArrangeData b;
    b.anchor = QPointF(200, 0);
    b.labelSize = QSizeF(40, 15);
    b.currentOffset = QPointF(50, 0); // label at (250,0)

    const QVector<QPointF> result = VLabelArrangeEngine::Arrange({a, b});
    QCOMPARE(result.size(), 2);

    // The two placed rectangles must not overlap
    const QRectF ra{a.anchor + result.at(0), a.labelSize};
    const QRectF rb{b.anchor + result.at(1), b.labelSize};
    QVERIFY(!ra.intersects(rb));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VLabelArrangeEngine::TestArrange_TwoOverlapping_Separated()
{
    // Two labels at nearly the same anchor, both starting at offset (0,0)
    LabelArrangeData a;
    a.anchor = QPointF(0, 0);
    a.labelSize = QSizeF(40, 15);
    a.currentOffset = QPointF(0, 0);

    LabelArrangeData b;
    b.anchor = QPointF(1, 0); // anchors 1 px apart — labels definitely overlap
    b.labelSize = QSizeF(40, 15);
    b.currentOffset = QPointF(0, 0);

    const QVector<QPointF> result = VLabelArrangeEngine::Arrange({a, b});
    QCOMPARE(result.size(), 2);

    // After arrangement, total overlap area must be less than before
    const QRectF ra{a.anchor + result.at(0), a.labelSize};
    const QRectF rb{b.anchor + result.at(1), b.labelSize};
    const QRectF inter = ra.intersected(rb);
    const qreal overlapAfter = inter.isEmpty() ? 0.0 : inter.width() * inter.height();
    const qreal overlapBefore = a.labelSize.width() * a.labelSize.height(); // full overlap
    QVERIFY(overlapAfter < overlapBefore);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VLabelArrangeEngine::TestArrange_LabelCoveringAnchor_MovedAway()
{
    // Label placed exactly on top of its anchor
    LabelArrangeData d;
    d.anchor = QPointF(50, 50);
    d.labelSize = QSizeF(40, 15);
    d.currentOffset = QPointF(0, 0); // top-left of label == anchor

    const QVector<QPointF> result = VLabelArrangeEngine::Arrange({d});
    QCOMPARE(result.size(), 1);

    const QRectF placed{d.anchor + result.at(0), d.labelSize};
    const QRectF anchorZone{d.anchor - QPointF(4, 4), QSizeF(8, 8)};
    QVERIFY(!placed.intersects(anchorZone));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VLabelArrangeEngine::TestArrange_Idempotent()
{
    // Run arrange twice; the second pass should not change positions
    LabelArrangeData a;
    a.anchor = QPointF(0, 0);
    a.labelSize = QSizeF(40, 15);
    a.currentOffset = QPointF(0, 0);

    LabelArrangeData b;
    b.anchor = QPointF(1, 0);
    b.labelSize = QSizeF(40, 15);
    b.currentOffset = QPointF(0, 0);

    const QVector<QPointF> pass1 = VLabelArrangeEngine::Arrange({a, b});
    QCOMPARE(pass1.size(), 2);

    a.currentOffset = pass1.at(0);
    b.currentOffset = pass1.at(1);
    const QVector<QPointF> pass2 = VLabelArrangeEngine::Arrange({a, b});

    for (int i = 0; i < pass1.size(); ++i)
    {
        QVERIFY(qAbs(pass2.at(i).x() - pass1.at(i).x()) < 0.5);
        QVERIFY(qAbs(pass2.at(i).y() - pass1.at(i).y()) < 0.5);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VLabelArrangeEngine::TestArrange_TooCloseAnchors_GracefulDegradation()
{
    // Identical anchors — impossible to separate; must not crash and must return valid offsets
    LabelArrangeData a;
    a.anchor = QPointF(10, 10);
    a.labelSize = QSizeF(40, 15);
    a.currentOffset = QPointF(0, 0);

    LabelArrangeData b;
    b.anchor = QPointF(10, 10); // same anchor
    b.labelSize = QSizeF(40, 15);
    b.currentOffset = QPointF(0, 0);

    const QVector<QPointF> result = VLabelArrangeEngine::Arrange({a, b});
    QCOMPARE(result.size(), 2);

    // Offsets must be finite numbers
    QVERIFY(qIsFinite(result.at(0).x()) && qIsFinite(result.at(0).y()));
    QVERIFY(qIsFinite(result.at(1).x()) && qIsFinite(result.at(1).y()));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VLabelArrangeEngine::TestArrange_LabelCoveringOtherAnchor_MovedAway()
{
    // Label A starts at an offset that puts it on top of label B's anchor, but NOT its own.
    // This is the "other anchor" violation the engine must detect and fix.
    LabelArrangeData a;
    a.anchor = QPointF(0, 0);
    a.labelSize = QSizeF(40, 15);
    a.anchorRadius = 4.0;
    a.currentOffset = QPointF(10, 0); // rect (10,0,40,15) — clears own anchor zone (-4,-4,8,8)

    LabelArrangeData b;
    b.anchor = QPointF(20, 5); // B's anchor falls inside A's placed rect
    b.labelSize = QSizeF(40, 15);
    b.anchorRadius = 4.0;
    b.currentOffset = QPointF(150, 0); // B is placed far away — no conflict from B's side

    const QRectF initialA{a.anchor + a.currentOffset, a.labelSize};
    const QRectF aAnchorZone{a.anchor - QPointF(a.anchorRadius, a.anchorRadius),
                              QSizeF(2.0 * a.anchorRadius, 2.0 * a.anchorRadius)};
    const QRectF bAnchorZone{b.anchor - QPointF(b.anchorRadius, b.anchorRadius),
                              QSizeF(2.0 * b.anchorRadius, 2.0 * b.anchorRadius)};

    // Pre-conditions: A covers B's anchor, but NOT A's own anchor
    QVERIFY(initialA.intersects(bAnchorZone));
    QVERIFY(!initialA.intersects(aAnchorZone));

    const QVector<QPointF> result = VLabelArrangeEngine::Arrange({a, b});
    QCOMPARE(result.size(), 2);

    const QRectF placedA{a.anchor + result.at(0), a.labelSize};
    QVERIFY(!placedA.intersects(bAnchorZone));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VLabelArrangeEngine::TestArrange_NoLabelCoversAnyAnchor()
{
    // Three labels, each starting at offset (0,0) covering its own anchor.
    // Anchors are 80 px apart so there is enough room to separate without covering each other.
    // After arrangement every placed rect must be clear of every anchor zone.
    const QSizeF labelSize(40, 15);
    const qreal radius = 4.0;

    LabelArrangeData a;
    a.anchor = QPointF(0, 0);
    a.labelSize = labelSize;
    a.anchorRadius = radius;
    a.currentOffset = QPointF(0, 0);

    LabelArrangeData b;
    b.anchor = QPointF(80, 0);
    b.labelSize = labelSize;
    b.anchorRadius = radius;
    b.currentOffset = QPointF(0, 0);

    LabelArrangeData c;
    c.anchor = QPointF(0, 80);
    c.labelSize = labelSize;
    c.anchorRadius = radius;
    c.currentOffset = QPointF(0, 0);

    const QVector<LabelArrangeData> labels = {a, b, c};
    const QVector<QPointF> result = VLabelArrangeEngine::Arrange(labels);
    QCOMPARE(result.size(), 3);

    for (int i = 0; i < labels.size(); ++i)
    {
        const QRectF placed{labels.at(i).anchor + result.at(i), labelSize};
        for (int j = 0; j < labels.size(); ++j)
        {
            const QRectF anchorZone{labels.at(j).anchor - QPointF(radius, radius),
                                    QSizeF(2.0 * radius, 2.0 * radius)};
            QVERIFY2(!placed.intersects(anchorZone),
                     qPrintable(QStringLiteral("Label %1 at offset (%2,%3) covers anchor %4 at (%5,%6)")
                                    .arg(i)
                                    .arg(result.at(i).x())
                                    .arg(result.at(i).y())
                                    .arg(j)
                                    .arg(labels.at(j).anchor.x())
                                    .arg(labels.at(j).anchor.y())));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VLabelArrangeEngine::TestArrange_AnchorRadiusRespected()
{
    // Verify that anchorRadius is actually used, not hardcoded.
    // offset (5, 5) with anchor (100, 100) clears a 4 px radius zone but falls inside a 30 px radius zone.
    LabelArrangeData d;
    d.anchor = QPointF(100, 100);
    d.labelSize = QSizeF(40, 15);
    d.currentOffset = QPointF(5, 5);

    // With radius 4: placed rect (105,105,40,15) does NOT cover zone (96,96,8,8) — x starts at 105 > 104
    d.anchorRadius = 4.0;
    {
        const QRectF placed{d.anchor + d.currentOffset, d.labelSize};
        const QRectF zone{d.anchor - QPointF(d.anchorRadius, d.anchorRadius),
                          QSizeF(2.0 * d.anchorRadius, 2.0 * d.anchorRadius)};
        QVERIFY(!placed.intersects(zone)); // pre-condition: label is already outside the small zone
    }

    // With radius 30: same position IS inside zone (70,70,60,60) — x [105,145] ∩ [70,130] is non-empty
    d.anchorRadius = 30.0;
    {
        const QRectF placed{d.anchor + d.currentOffset, d.labelSize};
        const QRectF zone{d.anchor - QPointF(d.anchorRadius, d.anchorRadius),
                          QSizeF(2.0 * d.anchorRadius, 2.0 * d.anchorRadius)};
        QVERIFY(placed.intersects(zone)); // pre-condition: label IS inside the large zone
    }

    const QVector<QPointF> result = VLabelArrangeEngine::Arrange({d}); // anchorRadius = 30
    QCOMPARE(result.size(), 1);

    const QRectF placed{d.anchor + result.at(0), d.labelSize};
    const QRectF zone{d.anchor - QPointF(d.anchorRadius, d.anchorRadius),
                      QSizeF(2.0 * d.anchorRadius, 2.0 * d.anchorRadius)};
    QVERIFY(!placed.intersects(zone));
}
