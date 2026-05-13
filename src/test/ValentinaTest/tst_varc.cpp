/************************************************************************
 **
 **  @file   tst_varc.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "tst_varc.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/varc.h"
#include "../vlayout/vabstractpiece.h"
#include "../vmisc/def.h"

#include <QtTest>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
void PrepareTestCase(const QPointF &center, qreal startAngle, qreal endAngle)
{
    qreal radius = UnitConvertor(1, Unit::Cm, Unit::Px);
    const qreal threshold = UnitConvertor(2000, Unit::Cm, Unit::Px);
    while (radius <= threshold)
    {
        VArc const arc(VPointF(center), radius, startAngle, endAngle);
        const QVector<QPointF> points = arc.GetPoints();

        const QString testStartAngle = u"Test start angel. Arc radius %1, start angle %2, end angle %3"_s.arg(radius)
                                           .arg(startAngle)
                                           .arg(endAngle);
        QTest::newRow(qUtf8Printable(testStartAngle)) << center << startAngle << points << points.constFirst() << true;

        const QString testEndAngle =
            u"Test end angel. Arc radius %1, start angle %2, end angle %3"_s.arg(radius).arg(startAngle).arg(endAngle);
        QTest::newRow(qUtf8Printable(testEndAngle)) << center << endAngle << points << points.constLast() << true;

        radius += UnitConvertor(5, Unit::Cm, Unit::Px);
    }
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
TST_VArc::TST_VArc(QObject *parent)
  : AbstractTest(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VArc::CompareTwoWays()
{
    const VPointF center;
    const qreal radius = 100;
    const qreal f1 = 1;
    const qreal f2 = 46;
    const qreal length = M_PI * radius / 180 * (f2 - f1);

    VArc const arc1(center, radius, f1, f2);
    VArc const arc2(length, center, radius, f1);

    QCOMPARE(arc1.GetLength(), length);
    QCOMPARE(arc2.GetLength(), length);

    QCOMPARE(arc1.GetLength(), arc2.GetLength());
    QCOMPARE(arc1.GetEndAngle(), arc2.GetEndAngle());

    QCOMPARE(arc1.GetEndAngle(), f2);
    QCOMPARE(arc2.GetEndAngle(), f2);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VArc::ArcByLength_data()
{
    QTest::addColumn<qreal>("radius");
    QTest::addColumn<qreal>("startAngle");
    QTest::addColumn<qreal>("endAngle");
    QTest::addColumn<qreal>("arcAngle");
    QTest::addColumn<bool>("reversed");
    QTest::addColumn<int>("direction");

    QTest::newRow("Positive radius, positive length") << 100. << 1. << 316. << 315. << false << 1;
    QTest::newRow("Positive radius, negative length") << 100. << 1. << 316. << 45. << true << -1;
    QTest::newRow("Negative radius, negative length") << -100. << 1. << 316. << 45. << true << -1;
    QTest::newRow("Negative radius, positive length") << -100. << 1. << 316. << 45. << true << 1;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VArc::ArcByLength()
{
    QFETCH(qreal, radius);
    QFETCH(qreal, startAngle);
    QFETCH(qreal, endAngle);
    QFETCH(qreal, arcAngle);
    QFETCH(bool, reversed);
    QFETCH(int, direction);

    const qreal length = (M_PI * qAbs(radius) / 180 * arcAngle) * direction;
    VArc const arc(length, VPointF(), radius, startAngle);

    QCOMPARE(qAbs(arc.GetLength()), qAbs(length));
    QCOMPARE(arc.GetEndAngle(), endAngle);
    QCOMPARE(arc.IsFlipped(), false);
    QCOMPARE(arc.IsReversed(), reversed);
    QCOMPARE(arc.GetRadius(), radius);
    QCOMPARE(arc.AngleArc(), arcAngle);
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VArc::TestGetPoints_data()
{
    QTest::addColumn<qreal>("radius");
    QTest::addColumn<qreal>("startAngle");
    QTest::addColumn<qreal>("endAngle");

    QTest::newRow("Full circle: radius 10") << 10.0 << 0.0 << 360.0;
    QTest::newRow("Full circle: radius -10") << -10.0 << 0.0 << 360.0;
    QTest::newRow("Full circle: radius 150") << 150.0 << 0.0 << 360.0;
    QTest::newRow("Full circle: radius -150") << -150.0 << 0.0 << 360.0;
    QTest::newRow("Full circle: radius 1500") << 1500.0 << 0.0 << 360.0;
    QTest::newRow("Full circle: radius -1500") << -1500.0 << 0.0 << 360.0;
    QTest::newRow("Full circle: radius 50000") << 50000.0 << 0.0 << 360.0;
    QTest::newRow("Full circle: radius -50000") << -50000.0 << 0.0 << 360.0;
    QTest::newRow("Full circle: radius 90000") << 90000.0 << 0.0 << 360.0;
    QTest::newRow("Full circle: radius -90000") << -90000.0 << 0.0 << 360.0;

    QTest::newRow("Arc less than 45 degree, radius 100") << 100.0 << 0.0 << 10.5;
    QTest::newRow("Arc less than 45 degree, radius -100") << -100.0 << 0.0 << 10.5;
    QTest::newRow("Arc less than 45 degree, radius 150") << 150.0 << 0.0 << 10.5;
    QTest::newRow("Arc less than 45 degree, radius -150") << -150.0 << 0.0 << 10.5;
    QTest::newRow("Arc less than 45 degree, radius 1500") << 1500.0 << 0.0 << 10.5;
    QTest::newRow("Arc less than 45 degree, radius -1500") << -1500.0 << 0.0 << 10.5;
    QTest::newRow("Arc less than 45 degree, radius 50000") << 50000.0 << 0.0 << 10.5;
    QTest::newRow("Arc less than 45 degree, radius -50000") << -50000.0 << 0.0 << 10.5;
    QTest::newRow("Arc less than 45 degree, radius 90000") << 90000.0 << 0.0 << 10.5;
    QTest::newRow("Arc less than 45 degree, radius -90000") << -90000.0 << 0.0 << 10.5;

    QTest::newRow("Arc 45 degree, radius 100") << 100.0 << 0.0 << 45.0;
    QTest::newRow("Arc 45 degree, radius -100") << -100.0 << 0.0 << 45.0;
    QTest::newRow("Arc 45 degree, radius 150") << 150.0 << 0.0 << 45.0;
    QTest::newRow("Arc 45 degree, radius -150") << -150.0 << 0.0 << 45.0;
    QTest::newRow("Arc 45 degree, radius 1500") << 1500.0 << 0.0 << 45.0;
    QTest::newRow("Arc 45 degree, radius -1500") << -1500.0 << 0.0 << 45.0;
    QTest::newRow("Arc 45 degree, radius 50000") << 50000.0 << 0.0 << 45.0;
    QTest::newRow("Arc 45 degree, radius -50000") << -50000.0 << 0.0 << 45.0;
    QTest::newRow("Arc 45 degree, radius 90000") << 90000.0 << 0.0 << 45.0;
    QTest::newRow("Arc 45 degree, radius -90000") << -90000.0 << 0.0 << 45.0;

    QTest::newRow("Arc less than 90 degree, radius 100") << 100.0 << 0.0 << 75.0;
    QTest::newRow("Arc less than 90 degree, radius -100") << -100.0 << 0.0 << 75.0;
    QTest::newRow("Arc less than 90 degree, radius 150") << 150.0 << 0.0 << 75.0;
    QTest::newRow("Arc less than 90 degree, radius -150") << -150.0 << 0.0 << 75.0;
    QTest::newRow("Arc less than 90 degree, radius 1500") << 1500.0 << 0.0 << 75.0;
    QTest::newRow("Arc less than 90 degree, radius -1500") << -1500.0 << 0.0 << 75.0;
    QTest::newRow("Arc less than 90 degree, radius 50000") << 50000.0 << 0.0 << 75.0;
    QTest::newRow("Arc less than 90 degree, radius -50000") << -50000.0 << 0.0 << 75.0;
    QTest::newRow("Arc less than 90 degree, radius 90000") << 90000.0 << 0.0 << 75.0;
    QTest::newRow("Arc less than 90 degree, radius -90000") << -90000.0 << 0.0 << 75.0;

    QTest::newRow("Arc 90 degree, radius 100") << 100.0 << 0.0 << 90.0;
    QTest::newRow("Arc 90 degree, radius -100") << -100.0 << 0.0 << 90.0;
    QTest::newRow("Arc 90 degree, radius 150") << 150.0 << 0.0 << 90.0;
    QTest::newRow("Arc 90 degree, radius -150") << -150.0 << 0.0 << 90.0;
    QTest::newRow("Arc 90 degree, radius 1500") << 1500.0 << 0.0 << 90.0;
    QTest::newRow("Arc 90 degree, radius -1500") << -1500.0 << 0.0 << 90.0;
    QTest::newRow("Arc 90 degree, radius 50000") << 50000.0 << 0.0 << 90.0;
    QTest::newRow("Arc 90 degree, radius -50000") << -50000.0 << 0.0 << 90.0;
    QTest::newRow("Arc 90 degree, radius 90000") << 90000.0 << 0.0 << 90.0;
    QTest::newRow("Arc 90 degree, radius -90000") << -90000.0 << 0.0 << 90.0;

    QTest::newRow("Arc less than 135 degree, radius 100") << 100.0 << 0.0 << 110.6;
    QTest::newRow("Arc less than 135 degree, radius -100") << -100.0 << 0.0 << 110.6;
    QTest::newRow("Arc less than 135 degree, radius 150") << 150.0 << 0.0 << 110.6;
    QTest::newRow("Arc less than 135 degree, radius -150") << -150.0 << 0.0 << 110.6;
    QTest::newRow("Arc less than 135 degree, radius 1500") << 1500.0 << 0.0 << 110.6;
    QTest::newRow("Arc less than 135 degree, radius -1500") << -1500.0 << 0.0 << 110.6;
    QTest::newRow("Arc less than 135 degree, radius 50000") << 50000.0 << 0.0 << 110.6;
    QTest::newRow("Arc less than 135 degree, radius -50000") << -50000.0 << 0.0 << 110.6;
    QTest::newRow("Arc less than 135 degree, radius 90000") << 90000.0 << 0.0 << 110.6;
    QTest::newRow("Arc less than 135 degree, radius -90000") << -90000.0 << 0.0 << 110.6;

    QTest::newRow("Arc 135 degree, radius 100") << 100.0 << 0.0 << 135.0;
    QTest::newRow("Arc 135 degree, radius -100") << -100.0 << 0.0 << 135.0;
    QTest::newRow("Arc 135 degree, radius 150") << 150.0 << 0.0 << 135.0;
    QTest::newRow("Arc 135 degree, radius -150") << -150.0 << 0.0 << 135.0;
    QTest::newRow("Arc 135 degree, radius 1500") << 1500.0 << 0.0 << 135.0;
    QTest::newRow("Arc 135 degree, radius -1500") << -1500.0 << 0.0 << 135.0;
    QTest::newRow("Arc 135 degree, radius 50000") << 50000.0 << 0.0 << 135.0;
    QTest::newRow("Arc 135 degree, radius -50000") << -50000.0 << 0.0 << 135.0;
    QTest::newRow("Arc 135 degree, radius 90000") << 90000.0 << 0.0 << 135.0;
    QTest::newRow("Arc 135 degree, radius -90000") << -90000.0 << 0.0 << 135.0;

    QTest::newRow("Arc less than 180 degree, radius 100") << 100.0 << 0.0 << 160.7;
    QTest::newRow("Arc less than 180 degree, radius -100") << -100.0 << 0.0 << 160.7;
    QTest::newRow("Arc less than 180 degree, radius 150") << 150.0 << 0.0 << 160.7;
    QTest::newRow("Arc less than 180 degree, radius -150") << -150.0 << 0.0 << 160.7;
    QTest::newRow("Arc less than 180 degree, radius 1500") << 1500.0 << 0.0 << 160.7;
    QTest::newRow("Arc less than 180 degree, radius -1500") << -1500.0 << 0.0 << 160.7;
    QTest::newRow("Arc less than 180 degree, radius 50000") << 50000.0 << 0.0 << 160.7;
    QTest::newRow("Arc less than 180 degree, radius -50000") << -50000.0 << 0.0 << 160.7;
    QTest::newRow("Arc less than 180 degree, radius 90000") << 90000.0 << 0.0 << 160.7;
    QTest::newRow("Arc less than 180 degree, radius -90000") << -90000.0 << 0.0 << 160.7;

    QTest::newRow("Arc 180 degree, radius 100") << 100.0 << 0.0 << 180.0;
    QTest::newRow("Arc 180 degree, radius -100") << -100.0 << 0.0 << 180.0;
    QTest::newRow("Arc 180 degree, radius 150") << 150.0 << 0.0 << 180.0;
    QTest::newRow("Arc 180 degree, radius -150") << -150.0 << 0.0 << 180.0;
    QTest::newRow("Arc 180 degree, radius 1500") << 1500.0 << 0.0 << 180.0;
    QTest::newRow("Arc 180 degree, radius -1500") << -1500.0 << 0.0 << 180.0;
    QTest::newRow("Arc 180 degree, radius 50000") << 50000.0 << 0.0 << 180.0;
    QTest::newRow("Arc 180 degree, radius -50000") << -50000.0 << 0.0 << 180.0;
    QTest::newRow("Arc 180 degree, radius 90000") << 90000.0 << 0.0 << 180.0;
    QTest::newRow("Arc 180 degree, radius -90000") << -90000.0 << 0.0 << 180.0;

    QTest::newRow("Arc less than 270 degree, radius 100") << 100.0 << 0.0 << 150.3;
    QTest::newRow("Arc less than 270 degree, radius -100") << -100.0 << 0.0 << 150.3;
    QTest::newRow("Arc less than 270 degree, radius 150") << 150.0 << 0.0 << 150.3;
    QTest::newRow("Arc less than 270 degree, radius -150") << -150.0 << 0.0 << 150.3;
    QTest::newRow("Arc less than 270 degree, radius 1500") << 1500.0 << 0.0 << 150.3;
    QTest::newRow("Arc less than 270 degree, radius -1500") << -1500.0 << 0.0 << 150.3;
    QTest::newRow("Arc less than 270 degree, radius 50000") << 50000.0 << 0.0 << 150.3;
    QTest::newRow("Arc less than 270 degree, radius -50000") << -50000.0 << 0.0 << 150.3;
    QTest::newRow("Arc less than 270 degree, radius 90000") << 90000.0 << 0.0 << 150.3;
    QTest::newRow("Arc less than 270 degree, radius -90000") << -90000.0 << 0.0 << 150.3;

    QTest::newRow("Arc 270 degree, radius 100") << 100.0 << 0.0 << 270.0;
    QTest::newRow("Arc 270 degree, radius -100") << -100.0 << 0.0 << 270.0;
    QTest::newRow("Arc 270 degree, radius 150") << 150.0 << 0.0 << 270.0;
    QTest::newRow("Arc 270 degree, radius -150") << -150.0 << 0.0 << 270.0;
    QTest::newRow("Arc 270 degree, radius 1500") << 1500.0 << 0.0 << 270.0;
    QTest::newRow("Arc 270 degree, radius -1500") << -1500.0 << 0.0 << 270.0;
    QTest::newRow("Arc 270 degree, radius 50000") << 50000.0 << 0.0 << 270.0;
    QTest::newRow("Arc 270 degree, radius -50000") << -50000.0 << 0.0 << 270.0;
    QTest::newRow("Arc 270 degree, radius 90000") << 90000.0 << 0.0 << 270.0;
    QTest::newRow("Arc 270 degree, radius -90000") << -90000.0 << 0.0 << 270.0;

    QTest::newRow("Arc less than 360 degree, radius 100") << 100.0 << 0.0 << 340.0;
    QTest::newRow("Arc less than 360 degree, radius -100") << -100.0 << 0.0 << 340.0;
    QTest::newRow("Arc less than 360 degree, radius 150") << 150.0 << 0.0 << 340.0;
    QTest::newRow("Arc less than 360 degree, radius -150") << -150.0 << 0.0 << 340.0;
    QTest::newRow("Arc less than 360 degree, radius 1500") << 1500.0 << 0.0 << 340.0;
    QTest::newRow("Arc less than 360 degree, radius -1500") << -1500.0 << 0.0 << 340.0;
    QTest::newRow("Arc less than 360 degree, radius 50000") << 50000.0 << 0.0 << 340.0;
    QTest::newRow("Arc less than 360 degree, radius -50000") << -50000.0 << 0.0 << 340.0;
    QTest::newRow("Arc less than 360 degree, radius 90000") << 90000.0 << 0.0 << 340.0;
    QTest::newRow("Arc less than 360 degree, radius -90000") << -90000.0 << 0.0 << 340.0;

    QTest::newRow("Arc start 90 degree, angle 45 degree, radius 100") << 100.0 << 90.0 << 135.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radius -100") << -100.0 << 90.0 << 135.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radius 150") << 150.0 << 90.0 << 135.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radius -150") << -150.0 << 90.0 << 135.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radius 1500") << 1500.0 << 90.0 << 135.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radius -1500") << -1500.0 << 90.0 << 135.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radius 50000") << 50000.0 << 90.0 << 135.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radius -50000") << -50000.0 << 90.0 << 135.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radius 90000") << 90000.0 << 90.0 << 135.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radius -90000") << -90000.0 << 90.0 << 135.0;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VArc::TestGetPoints()
{
    QFETCH(qreal, radius);
    QFETCH(qreal, startAngle);
    QFETCH(qreal, endAngle);

    const VPointF center;
    VArc arc(center, radius, startAngle, endAngle);
    arc.SetApproximationScale(maxCurveApproximationScale);

    QVector<QPointF> points = arc.GetPoints();

    {
        constexpr qreal epsRadius = MmToPixel(0.5); // computing error

        for (auto point : std::as_const(points))
        {
            QLineF const rLine(static_cast<QPointF>(center), point);
            const qreal value = qAbs(rLine.length() - qAbs(radius));
            // cppcheck-suppress unreadVariable
            const QString errorMsg = u"Broken the first rule. All points should be on the same distance from "
                                     u"the center. Error ='%1'mm."_s.arg(PixelToMm(value));
            QVERIFY2(value <= epsRadius, qUtf8Printable(errorMsg));
        }
    }

    {
        qreal gSquare = 0.0; // geometry square

        if (VFuzzyComparePossibleNulls(arc.AngleArc(), 360.0))
        { // circle square
            gSquare = M_PI * qAbs(radius) * qAbs(radius);
        }
        else
        { // sector square
            gSquare = (M_PI * qAbs(radius) * qAbs(radius)) / 360.0 * arc.AngleArc();
            points.append(static_cast<QPointF>(center));
        }

        // calculated square
        const qreal cSquare = qAbs(VAbstractPiece::SumTrapezoids(points) / 2.0);
        const qreal value = qAbs(gSquare - cSquare);
        // cppcheck-suppress unreadVariable
        const QString errorMsg =
            u"Broken the second rule. Interpolation has too big computing error. Error ='%1'."_s.arg(value);
        const qreal epsSquare = gSquare * 0.24 / 100; // computing error 0.24 % from origin square
        QVERIFY2(value <= epsSquare, qUtf8Printable(errorMsg));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VArc::TestRotation_data()
{
    QTest::addColumn<QPointF>("center");
    QTest::addColumn<qreal>("radius");
    QTest::addColumn<qreal>("startAngle");
    QTest::addColumn<qreal>("endAngle");
    QTest::addColumn<QPointF>("rotatePoint");
    QTest::addColumn<qreal>("degrees");
    QTest::addColumn<QString>("prefix");

    QTest::newRow("Test arc 1") << QPointF(10, 10) << 10. << 0. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test arc 2") << QPointF(10, 10) << -10. << 0. << 90. << QPointF() << 90. << "_r";
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VArc::TestRotation()
{
    QFETCH(QPointF, center);
    QFETCH(qreal, radius);
    QFETCH(qreal, startAngle);
    QFETCH(qreal, endAngle);
    QFETCH(QPointF, rotatePoint);
    QFETCH(qreal, degrees);
    QFETCH(QString, prefix);

    // Initialize original arc and store baseline metrics
    const VArc arcOrigin(VPointF(center), radius, startAngle, endAngle);
    const qreal originalLength = qAbs(arcOrigin.GetLength());

    // Get original arc endpoints for manual calculation
    const QPointF originalP1 = arcOrigin.GetP1();
    const QPointF originalP2 = arcOrigin.GetP2();

    // Manually calculate where the endpoints SHOULD be after rotation
    const QPointF expectedRotatedP1 = VPointF::RotatePF(rotatePoint, originalP1, degrees);
    const QPointF expectedRotatedP2 = VPointF::RotatePF(rotatePoint, originalP2, degrees);

    // Perform the actual rotation via the class method
    const VArc rotatedArc = arcOrigin.Rotate(rotatePoint, degrees, prefix);

    // Verify name prefixing logic
    const QString errorMsg = u"The name doesn't contain the prefix '%1'."_s.arg(prefix);
    QVERIFY2(rotatedArc.name().endsWith(prefix), qUtf8Printable(errorMsg));

    // Sweep angle (AngleArc) should remain constant during rotation
    QVERIFY2(qAbs(arcOrigin.AngleArc() - rotatedArc.AngleArc()) <= 0.001,
             qUtf8Printable(u"Sweep angle mismatch: original %1, rotated %2"_s.arg(arcOrigin.AngleArc())
                                .arg(rotatedArc.AngleArc())));

    // Length should be invariant (within 1mm tolerance)
    const qreal rotatedLength = qAbs(rotatedArc.GetLength());
    QVERIFY2(qAbs(originalLength - rotatedLength) <= ToPixel(1, Unit::Mm),
             qUtf8Printable(u"Length mismatch. Original: %1, Rotated: %2"_s.arg(originalLength).arg(rotatedLength)));

    // Basic properties should remain identical
    QCOMPARE(arcOrigin.GetRadius(), rotatedArc.GetRadius());
    QCOMPARE(arcOrigin.IsFlipped(), rotatedArc.IsFlipped());
    QCOMPARE(arcOrigin.IsReversed(), rotatedArc.IsReversed());

    // Verify point path consistency for the original arc
    const QVector<QPointF> originalPoints = arcOrigin.GetPoints();
    QVERIFY2(!originalPoints.isEmpty(), "Original arc generated an empty point path");
    QVERIFY2(VFuzzyComparePoints(originalP1, originalPoints.constFirst()), "Original P1 != path start");
    QVERIFY2(VFuzzyComparePoints(originalP2, originalPoints.constLast()), "Original P2 != path end");

    // Verify point path consistency for the rotated arc
    const QVector<QPointF> rotatedPoints = rotatedArc.GetPoints();
    QVERIFY2(!rotatedPoints.isEmpty(), "Rotated arc generated an empty point path");

    const QPointF actualRotatedP1 = rotatedArc.GetP1();
    const QPointF actualRotatedP2 = rotatedArc.GetP2();

    // Check that GetP1/P2 match the internal path points
    QVERIFY2(VFuzzyComparePoints(actualRotatedP1, rotatedPoints.constFirst()), "Actual P1 != path start");
    QVERIFY2(VFuzzyComparePoints(actualRotatedP2, rotatedPoints.constLast()), "Actual P2 != path end");

    // Final verification: Compare actual endpoints against manual geometric rotation
    QVERIFY2(VFuzzyComparePoints(expectedRotatedP1, actualRotatedP1),
             qPrintable(QStringLiteral("Rotation failed: actual P1 does not match expected manual rotation")));
    QVERIFY2(VFuzzyComparePoints(expectedRotatedP2, actualRotatedP2),
             qPrintable(QStringLiteral("Rotation failed: actual P2 does not match expected manual rotation")));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VArc::TestFlip_data()
{
    QTest::addColumn<QPointF>("center");
    QTest::addColumn<qreal>("radius");
    QTest::addColumn<qreal>("startAngle");
    QTest::addColumn<qreal>("endAngle");
    QTest::addColumn<QLineF>("axis");
    QTest::addColumn<QString>("prefix");

    const qreal radius = 5;
    QPointF const center(10, 5);

    QPointF p1(10, 0);
    QPointF p2(5, 5);

    QLineF axis(QPointF(4, 6), QPointF(12, 6));

    QTest::newRow("Vertical axis, positive radius")
        << center << radius << QLineF(center, p1).angle() << QLineF(center, p2).angle() << axis << "a2";
    QTest::newRow("Vertical axis, negative radius")
        << center << -radius << QLineF(center, p1).angle() << QLineF(center, p2).angle() << axis << "a2";

    p1 = QPointF(15, 5);
    p2 = QPointF(10, 0);

    axis = QLineF(QPointF(9, -1), QPointF(9, 6));

    QTest::newRow("Horizontal axis, positive radius")
        << center << radius << QLineF(center, p1).angle() << QLineF(center, p2).angle() << axis << "a2";
    QTest::newRow("Horizontal axis, negative radius")
        << center << -radius << QLineF(center, p1).angle() << QLineF(center, p2).angle() << axis << "a2";

    QLineF l(center.x(), center.y(), center.x() + radius, center.y());

    l.setAngle(45);
    p2 = l.p2();

    l.setAngle(225);
    p1 = l.p2();

    l.setAngle(45 + 90);
    l.setLength(5);

    const QPointF p1Axis = l.p2();
    axis = QLineF(p1Axis.x(), p1Axis.y(), p1Axis.x() + radius, p1Axis.y());
    axis.setAngle(45);

    QTest::newRow("Diagonal axis, positive radius")
        << center << radius << QLineF(center, p1).angle() << QLineF(center, p2).angle() << axis << "a2";
    QTest::newRow("Diagonal axis, negative radius")
        << center << -radius << QLineF(center, p1).angle() << QLineF(center, p2).angle() << axis << "a2";

    // Arc crossing the axis
    axis = QLineF(QPointF(4, 5), QPointF(16, 5)); // horizontal through center
    QTest::newRow("Arc crosses axis, positive radius") << center << radius << 45.0 << 315.0 << axis << "a2";
    QTest::newRow("Arc crosses axis, negative radius") << center << -radius << 45.0 << 315.0 << axis << "a2";

    // Full circle
    QTest::newRow("Full circle, positive radius")
        << center << radius << 0.0 << 360.0 << QLineF(QPointF(4, 6), QPointF(12, 6)) << "a2";
    QTest::newRow("Full circle, negative radius")
        << center << -radius << 0.0 << 360.0 << QLineF(QPointF(4, 6), QPointF(12, 6)) << "a2";

    // Very small arc
    QTest::newRow("Small arc 1 degree, positive radius")
        << center << radius << 0.0 << 1.0 << QLineF(QPointF(4, 6), QPointF(12, 6)) << "a2";

    // Arc on the axis itself (start point lies on axis)
    axis = QLineF(QPointF(10, 0), QPointF(10, 10)); // vertical through center
    QTest::newRow("Start point on axis, positive radius") << center << radius << 270.0 << 180.0 << axis << "a2";
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VArc::TestFlip()
{
    QFETCH(QPointF, center);
    QFETCH(qreal, radius);
    QFETCH(qreal, startAngle);
    QFETCH(qreal, endAngle);
    QFETCH(QLineF, axis);
    QFETCH(QString, prefix);

    const VArc arcOrigin(VPointF(center), radius, startAngle, endAngle);
    const qreal originalLength = qAbs(arcOrigin.GetLength());
    const QPointF originalP1 = arcOrigin.GetP1();
    const QPointF originalP2 = arcOrigin.GetP2();

    const QPointF expectedCenter = VPointF::FlipPF(axis, center);
    const QPointF expectedP1 = VPointF::FlipPF(axis, originalP1);
    const QPointF expectedP2 = VPointF::FlipPF(axis, originalP2);

    const VArc flippedArc = arcOrigin.Flip(axis, prefix);

    // Verify metadata and basic state toggle
    const QString nameError = u"The name doesn't contain the prefix '%1'."_s.arg(prefix);
    QVERIFY2(flippedArc.name().endsWith(prefix), qUtf8Printable(nameError));
    QVERIFY2(flippedArc.IsFlipped() != arcOrigin.IsFlipped(), "IsFlipped property was not toggled");

    const qreal flippedLength = qAbs(flippedArc.GetLength());
    QVERIFY2(qAbs(originalLength - flippedLength) <= ToPixel(1, Unit::Mm),
             qUtf8Printable(
                 u"Length mismatch after flip. Original: %1, Flipped: %2"_s.arg(originalLength).arg(flippedLength)));

    QCOMPARE(flippedArc.GetRadius(), arcOrigin.GetRadius());
    QVERIFY2(qAbs(flippedArc.AngleArc() - arcOrigin.AngleArc()) <= 0.001, "Sweep angle mismatch");

    const QPointF actualCenter = static_cast<QPointF>(flippedArc.GetCenter());
    QVERIFY2(VFuzzyComparePoints(actualCenter, expectedCenter), "Center mismatch after flip");

    const QPointF actualP1 = flippedArc.GetP1();
    const QPointF actualP2 = flippedArc.GetP2();

    QVERIFY2(VFuzzyComparePoints(actualP1, expectedP1), "P1 mismatch after flip");
    QVERIFY2(VFuzzyComparePoints(actualP2, expectedP2), "P2 mismatch after flip");

    const QVector<QPointF> flippedPoints = flippedArc.GetPoints();
    QVERIFY2(!flippedPoints.isEmpty(), "Flipped arc generated an empty point path");
    QVERIFY2(VFuzzyComparePoints(actualP1, flippedPoints.constFirst()), "P1 != path start");
    QVERIFY2(VFuzzyComparePoints(actualP2, flippedPoints.constLast()), "P2 != path end");

    const VArc restoredArc = flippedArc.Flip(axis, prefix);

    QVERIFY2(VFuzzyComparePoints(static_cast<QPointF>(restoredArc.GetCenter()), center),
             "Center mismatch after double flip");
    QVERIFY2(VFuzzyComparePoints(restoredArc.GetP1(), originalP1), "P1 mismatch after double flip");
    QVERIFY2(VFuzzyComparePoints(restoredArc.GetP2(), originalP2), "P2 mismatch after double flip");
    QCOMPARE(restoredArc.IsFlipped(), arcOrigin.IsFlipped());
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VArc::TestRotateAndFlip_data()
{
    QTest::addColumn<QPointF>("center");
    QTest::addColumn<qreal>("radius");
    QTest::addColumn<qreal>("startAngle");
    QTest::addColumn<qreal>("endAngle");
    QTest::addColumn<QPointF>("rotatePoint");
    QTest::addColumn<qreal>("rotateDegrees");
    QTest::addColumn<QLineF>("flipAxis");
    QTest::addColumn<QString>("prefixR");
    QTest::addColumn<QString>("prefixF");

    const QLineF vertAxis(QPointF(100, 0), QPointF(100, 500));
    const QLineF horizAxis(QPointF(0, 100), QPointF(500, 100));
    QLineF diagAxis(QPointF(0, 0), QPointF(100, 100));

    // Case 1: Simple 90-degree rotation around origin then flip over vertical axis
    QTest::newRow("Rotate 90 @ Origin -> Flip Vertical")
        << QPointF(50, 0) << 50.0 << 0.0 << 180.0 << QPointF(0, 0) << 90.0 << vertAxis << "_rot" << "_flip";

    // Case 2: Rotation around arc center then flip over horizontal axis
    QTest::newRow("Rotate 45 @ Center -> Flip Horizontal")
        << QPointF(100, 100) << 30.0 << 45.0 << 135.0 << QPointF(100, 100) << 45.0 << horizAxis << "_r" << "_f";

    // Case 3: Negative rotation followed by diagonal flip
    QTest::newRow("Negative Rotate -> Diagonal Flip")
        << QPointF(200, 200) << 100.0 << 0.0 << 90.0 << QPointF(0, 0) << -90.0 << diagAxis << "_r" << "_f";

    // Case 4: Full circle (360 degrees) handling
    QTest::newRow("Full Circle sequence")
        << QPointF(0, 0) << 10.0 << 0.0 << 360.0 << QPointF(10, 10) << 180.0 << vertAxis << "_rot" << "_mirr";
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VArc::TestRotateAndFlip()
{
    QFETCH(QPointF, center);
    QFETCH(qreal, radius);
    QFETCH(qreal, startAngle);
    QFETCH(qreal, endAngle);
    QFETCH(QPointF, rotatePoint);
    QFETCH(qreal, rotateDegrees);
    QFETCH(QLineF, flipAxis);
    QFETCH(QString, prefixR);
    QFETCH(QString, prefixF);

    const VArc arcOrigin(VPointF(center), radius, startAngle, endAngle);
    const qreal originalLength = qAbs(arcOrigin.GetLength());
    const QPointF p1_0 = arcOrigin.GetP1();
    const QPointF p2_0 = arcOrigin.GetP2();

    const VArc rotatedArc = arcOrigin.Rotate(rotatePoint, rotateDegrees, prefixR);
    const VArc finalArc = rotatedArc.Flip(flipAxis, prefixF);

    const QPointF p1_rotated = VPointF::RotatePF(rotatePoint, p1_0, rotateDegrees);
    const QPointF p2_rotated = VPointF::RotatePF(rotatePoint, p2_0, rotateDegrees);

    const QPointF expectedP1 = VPointF::FlipPF(flipAxis, p1_rotated);
    const QPointF expectedP2 = VPointF::FlipPF(flipAxis, p2_rotated);

    const auto nameError = QStringLiteral("Final name '%1' does not contain expected prefixes.").arg(finalArc.name());
    QVERIFY2(finalArc.name().endsWith(prefixR + prefixF) || finalArc.name().endsWith(prefixF),
             qUtf8Printable(nameError));

    const qreal finalLength = qAbs(finalArc.GetLength());
    const qreal epsilon = ToPixel(1, Unit::Mm);

    QVERIFY2(qAbs(originalLength - finalLength) <= epsilon,
             qUtf8Printable(u"Length mismatch. Expected: %1, Actual: %2"_s.arg(originalLength).arg(finalLength)));

    QCOMPARE(finalArc.GetRadius(), arcOrigin.GetRadius());

    const QPointF actualP1 = finalArc.GetP1();
    const QPointF actualP2 = finalArc.GetP2();

    QVERIFY2(VFuzzyComparePoints(expectedP1, actualP1), "P1 mismatch after Rotate+Flip sequence");
    QVERIFY2(VFuzzyComparePoints(expectedP2, actualP2), "P2 mismatch after Rotate+Flip sequence");

    const QVector<QPointF> pathPoints = finalArc.GetPoints();
    QVERIFY2(!pathPoints.isEmpty(), "Resulting arc path is empty");

    QVERIFY2(VFuzzyComparePoints(actualP1, pathPoints.constFirst()), "P1 != first path point");
    QVERIFY2(VFuzzyComparePoints(actualP2, pathPoints.constLast()), "P2 != last path point");
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VArc::TestCutArcByLength_data()
{
    QTest::addColumn<QPointF>("center");
    QTest::addColumn<qreal>("radius");
    QTest::addColumn<qreal>("startAngle");
    QTest::addColumn<qreal>("length");
    QTest::addColumn<qreal>("cutLength");
    QTest::addColumn<QPointF>("cutPoint");

    const QPointF center(189.13625196850393, 344.1267401574803);
    Q_RELAXED_CONSTEXPR qreal radius = ToPixel(10, Unit::Cm);
    QPointF cutPoint(539.3657292513009, 202.04366960088566);
    Q_RELAXED_CONSTEXPR qreal length = ToPixel(10, Unit::Cm);

    // See file <root>/src/app/share/collection/bugs/Issue_#957.val
    QTest::newRow("Arc -10 cm length, cut length 6 cm")
        << center << radius << 45.0 << -length << ToPixel(6, Unit::Cm) << cutPoint;

    // Opposite case
    QTest::newRow("Arc -10 cm length, cut length -4 cm")
        << center << radius << 45.0 << -length << ToPixel(-4, Unit::Cm) << cutPoint;

    cutPoint = QPointF(-145.1588983496871, 167.78888781060192);

    // See file <root>/src/app/share/collection/bugs/Issue_#957.val
    QTest::newRow("Arc 10 cm length, cut length -7 cm")
        << center << radius << 135. << length << ToPixel(-7, Unit::Cm) << cutPoint;

    // Opposite case
    QTest::newRow("Arc 10 cm length, cut length 3 cm")
        << center << radius << 135. << length << ToPixel(3, Unit::Cm) << cutPoint;

    auto l = QLineF(center, QPointF(center.x() + radius, center.y()));
    l.setAngle(135);

    QTest::newRow("Arc 10 cm length, cut length 0 cm") << center << radius << 135. << length << 0. << l.p2();

    QTest::newRow("Arc 10 cm length (-10 cm radius), cut length 0 cm")
        << center << -radius << 135. << length << 0. << l.p2();

    QTest::newRow("Arc -10 cm length (-10 cm radius), cut length 0 cm")
        << center << -radius << 135. << -length << 0. << l.p2();

    QTest::newRow("Arc -10 cm length (10 cm radius), cut length 10 cm")
        << center << radius << 135. << -length << length << l.p2();

    QTest::newRow("Arc -10 cm length (-10 cm radius), cut length 10 cm")
        << center << -radius << 135. << -length << length << l.p2();

    const qreal arcAngle = qAbs(qRadiansToDegrees(ToPixel(10, Unit::Cm) / qAbs(radius)));
    l = QLineF(center, QPointF(center.x() + radius, center.y()));
    l.setAngle(135 + arcAngle);

    QTest::newRow("Arc 10 cm length, cut length 10 cm")
        << center << radius << 135. << length << ToPixel(10, Unit::Cm) << l.p2();

    l = QLineF(center, QPointF(center.x() + radius, center.y()));
    l.setAngle(135 - arcAngle);

    QTest::newRow("Arc -10 cm length (10 cm radius), cut length -10 cm")
        << center << radius << 135. << -length << -radius << l.p2();

    QTest::newRow("Arc -10 cm length (-10 cm radius), cut length -10 cm")
        << center << -radius << 135. << -length << -radius << l.p2();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VArc::TestCutArcByLength()
{
    QFETCH(QPointF, center);
    QFETCH(qreal, radius);
    QFETCH(qreal, startAngle);
    QFETCH(qreal, length);
    QFETCH(qreal, cutLength);
    QFETCH(QPointF, cutPoint);

    VArc arc(length, VPointF(center), radius, startAngle);
    arc.SetApproximationScale(0.5);

    VArc arc1;
    VArc arc2;
    QPointF const point = arc.CutArc(cutLength, &arc1, &arc2, QString());

    QCOMPARE(point, cutPoint);

    QCOMPARE(arc1.IsFlipped(), arc.IsFlipped());
    QCOMPARE(arc2.IsFlipped(), arc.IsFlipped());

    QCOMPARE(arc1.IsReversed(), arc.IsReversed());
    QCOMPARE(arc2.IsReversed(), arc.IsReversed());

    QCOMPARE(arc1.GetApproximationScale(), arc.GetApproximationScale());
    QCOMPARE(arc2.GetApproximationScale(), arc.GetApproximationScale());

    QCOMPARE(arc1.GetStartAngle(), arc.GetStartAngle());
    QCOMPARE(arc2.GetEndAngle(), arc.GetEndAngle());

    QVERIFY(VFuzzyComparePossibleNulls(arc1.GetEndAngle(), arc2.GetStartAngle()));

    const qreal lengthArc = arc.GetLength();
    const qreal lengthArc1 = arc1.GetLength();
    const qreal lengthArc2 = arc2.GetLength();

    QCOMPARE(lengthArc1 + lengthArc2, lengthArc);

    qreal const segmentsLength =
        VAbstractCurve::PathLength(arc1.GetPoints()) + VAbstractCurve::PathLength(arc2.GetPoints());
    QVERIFY(qAbs(segmentsLength - VAbstractCurve::PathLength(arc.GetPoints())) <= accuracyPointOnLine);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VArc::TestCurveIntersectAxis_data()
{
    QTest::addColumn<QPointF>("basePoint");
    QTest::addColumn<qreal>("angle");
    QTest::addColumn<QVector<QPointF>>("curvePoints");
    QTest::addColumn<QPointF>("crosPoint");
    QTest::addColumn<bool>("result");

    QPointF const basePoint(10, 10);

    PrepareTestCase(basePoint, 0, 15);
    PrepareTestCase(basePoint, 0, 25);
    PrepareTestCase(basePoint, 0, 45);
    PrepareTestCase(basePoint, 0, 90);
    PrepareTestCase(basePoint, 0, 180);
    PrepareTestCase(basePoint, 0, 270);
    PrepareTestCase(basePoint, 180, 315);
    PrepareTestCase(basePoint, 183, 312);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VArc::TestCurveIntersectAxis()
{
    QFETCH(QPointF, basePoint);
    QFETCH(qreal, angle);
    QFETCH(QVector<QPointF>, curvePoints);
    QFETCH(QPointF, crosPoint);
    QFETCH(bool, result);

    QPointF intersectionPoint;
    const bool found = VAbstractCurve::CurveIntersectAxis(basePoint, angle, curvePoints, &intersectionPoint);
    QCOMPARE(found, result);

    ComparePointsDistance(intersectionPoint, crosPoint, accuracyPointOnLine);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VArc::EmptyArc()
{
    VArc const empty;

    ComparePathsDistance(empty.GetPoints(), {QPointF()});
    QCOMPARE(empty.GetLength(), 0.);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VArc::TestCurvature_data()
{
    QTest::addColumn<qreal>("radius");
    QTest::addColumn<qreal>("startAngle");
    QTest::addColumn<qreal>("endAngle");

    QTest::newRow("Full circle: radius 10") << 10.0 << 0.0 << 360.0;
    QTest::newRow("Full circle: radius -10") << -10.0 << 0.0 << 360.0;
    QTest::newRow("Full circle: radius 150") << 150.0 << 0.0 << 360.0;
    QTest::newRow("Full circle: radius -150") << -150.0 << 0.0 << 360.0;
    QTest::newRow("Full circle: radius 1500") << 1500.0 << 0.0 << 360.0;
    QTest::newRow("Full circle: radius -1500") << -1500.0 << 0.0 << 360.0;
    QTest::newRow("Full circle: radius 50000") << 50000.0 << 0.0 << 360.0;
    QTest::newRow("Full circle: radius -50000") << -50000.0 << 0.0 << 360.0;
    QTest::newRow("Full circle: radius 90000") << 90000.0 << 0.0 << 360.0;
    QTest::newRow("Full circle: radius -90000") << -90000.0 << 0.0 << 360.0;

    QTest::newRow("Arc less than 45 degree, radius 100") << 100.0 << 0.0 << 10.5;
    QTest::newRow("Arc less than 45 degree, radius -100") << -100.0 << 0.0 << 10.5;
    QTest::newRow("Arc less than 45 degree, radius 150") << 150.0 << 0.0 << 10.5;
    QTest::newRow("Arc less than 45 degree, radius -150") << -150.0 << 0.0 << 10.5;
    QTest::newRow("Arc less than 45 degree, radius 1500") << 1500.0 << 0.0 << 10.5;
    QTest::newRow("Arc less than 45 degree, radius -1500") << -1500.0 << 0.0 << 10.5;
    QTest::newRow("Arc less than 45 degree, radius 50000") << 50000.0 << 0.0 << 10.5;
    QTest::newRow("Arc less than 45 degree, radius -50000") << -50000.0 << 0.0 << 10.5;
    QTest::newRow("Arc less than 45 degree, radius 90000") << 90000.0 << 0.0 << 10.5;
    QTest::newRow("Arc less than 45 degree, radius -90000") << -90000.0 << 0.0 << 10.5;

    QTest::newRow("Arc 45 degree, radius 100") << 100.0 << 0.0 << 45.0;
    QTest::newRow("Arc 45 degree, radius -100") << -100.0 << 0.0 << 45.0;
    QTest::newRow("Arc 45 degree, radius 150") << 150.0 << 0.0 << 45.0;
    QTest::newRow("Arc 45 degree, radius -150") << -150.0 << 0.0 << 45.0;
    QTest::newRow("Arc 45 degree, radius 1500") << 1500.0 << 0.0 << 45.0;
    QTest::newRow("Arc 45 degree, radius -1500") << -1500.0 << 0.0 << 45.0;
    QTest::newRow("Arc 45 degree, radius 50000") << 50000.0 << 0.0 << 45.0;
    QTest::newRow("Arc 45 degree, radius -50000") << -50000.0 << 0.0 << 45.0;
    QTest::newRow("Arc 45 degree, radius 90000") << 90000.0 << 0.0 << 45.0;
    QTest::newRow("Arc 45 degree, radius -90000") << -90000.0 << 0.0 << 45.0;

    QTest::newRow("Arc less than 90 degree, radius 100") << 100.0 << 0.0 << 75.0;
    QTest::newRow("Arc less than 90 degree, radius -100") << -100.0 << 0.0 << 75.0;
    QTest::newRow("Arc less than 90 degree, radius 150") << 150.0 << 0.0 << 75.0;
    QTest::newRow("Arc less than 90 degree, radius -150") << -150.0 << 0.0 << 75.0;
    QTest::newRow("Arc less than 90 degree, radius 1500") << 1500.0 << 0.0 << 75.0;
    QTest::newRow("Arc less than 90 degree, radius -1500") << -1500.0 << 0.0 << 75.0;
    QTest::newRow("Arc less than 90 degree, radius 50000") << 50000.0 << 0.0 << 75.0;
    QTest::newRow("Arc less than 90 degree, radius -50000") << -50000.0 << 0.0 << 75.0;
    QTest::newRow("Arc less than 90 degree, radius 90000") << 90000.0 << 0.0 << 75.0;
    QTest::newRow("Arc less than 90 degree, radius -90000") << -90000.0 << 0.0 << 75.0;

    QTest::newRow("Arc 90 degree, radius 100") << 100.0 << 0.0 << 90.0;
    QTest::newRow("Arc 90 degree, radius -100") << -100.0 << 0.0 << 90.0;
    QTest::newRow("Arc 90 degree, radius 150") << 150.0 << 0.0 << 90.0;
    QTest::newRow("Arc 90 degree, radius -150") << -150.0 << 0.0 << 90.0;
    QTest::newRow("Arc 90 degree, radius 1500") << 1500.0 << 0.0 << 90.0;
    QTest::newRow("Arc 90 degree, radius -1500") << -1500.0 << 0.0 << 90.0;
    QTest::newRow("Arc 90 degree, radius 50000") << 50000.0 << 0.0 << 90.0;
    QTest::newRow("Arc 90 degree, radius -50000") << -50000.0 << 0.0 << 90.0;
    QTest::newRow("Arc 90 degree, radius 90000") << 90000.0 << 0.0 << 90.0;
    QTest::newRow("Arc 90 degree, radius -90000") << -90000.0 << 0.0 << 90.0;

    QTest::newRow("Arc less than 135 degree, radius 100") << 100.0 << 0.0 << 110.6;
    QTest::newRow("Arc less than 135 degree, radius -100") << -100.0 << 0.0 << 110.6;
    QTest::newRow("Arc less than 135 degree, radius 150") << 150.0 << 0.0 << 110.6;
    QTest::newRow("Arc less than 135 degree, radius -150") << -150.0 << 0.0 << 110.6;
    QTest::newRow("Arc less than 135 degree, radius 1500") << 1500.0 << 0.0 << 110.6;
    QTest::newRow("Arc less than 135 degree, radius -1500") << -1500.0 << 0.0 << 110.6;
    QTest::newRow("Arc less than 135 degree, radius 50000") << 50000.0 << 0.0 << 110.6;
    QTest::newRow("Arc less than 135 degree, radius -50000") << -50000.0 << 0.0 << 110.6;
    QTest::newRow("Arc less than 135 degree, radius 90000") << 90000.0 << 0.0 << 110.6;
    QTest::newRow("Arc less than 135 degree, radius -90000") << -90000.0 << 0.0 << 110.6;

    QTest::newRow("Arc 135 degree, radius 100") << 100.0 << 0.0 << 135.0;
    QTest::newRow("Arc 135 degree, radius -100") << -100.0 << 0.0 << 135.0;
    QTest::newRow("Arc 135 degree, radius 150") << 150.0 << 0.0 << 135.0;
    QTest::newRow("Arc 135 degree, radius -150") << -150.0 << 0.0 << 135.0;
    QTest::newRow("Arc 135 degree, radius 1500") << 1500.0 << 0.0 << 135.0;
    QTest::newRow("Arc 135 degree, radius -1500") << -1500.0 << 0.0 << 135.0;
    QTest::newRow("Arc 135 degree, radius 50000") << 50000.0 << 0.0 << 135.0;
    QTest::newRow("Arc 135 degree, radius -50000") << -50000.0 << 0.0 << 135.0;
    QTest::newRow("Arc 135 degree, radius 90000") << 90000.0 << 0.0 << 135.0;
    QTest::newRow("Arc 135 degree, radius -90000") << -90000.0 << 0.0 << 135.0;

    QTest::newRow("Arc less than 180 degree, radius 100") << 100.0 << 0.0 << 160.7;
    QTest::newRow("Arc less than 180 degree, radius -100") << -100.0 << 0.0 << 160.7;
    QTest::newRow("Arc less than 180 degree, radius 150") << 150.0 << 0.0 << 160.7;
    QTest::newRow("Arc less than 180 degree, radius -150") << -150.0 << 0.0 << 160.7;
    QTest::newRow("Arc less than 180 degree, radius 1500") << 1500.0 << 0.0 << 160.7;
    QTest::newRow("Arc less than 180 degree, radius -1500") << -1500.0 << 0.0 << 160.7;
    QTest::newRow("Arc less than 180 degree, radius 50000") << 50000.0 << 0.0 << 160.7;
    QTest::newRow("Arc less than 180 degree, radius -50000") << -50000.0 << 0.0 << 160.7;
    QTest::newRow("Arc less than 180 degree, radius 90000") << 90000.0 << 0.0 << 160.7;
    QTest::newRow("Arc less than 180 degree, radius -90000") << -90000.0 << 0.0 << 160.7;

    QTest::newRow("Arc 180 degree, radius 100") << 100.0 << 0.0 << 180.0;
    QTest::newRow("Arc 180 degree, radius -100") << -100.0 << 0.0 << 180.0;
    QTest::newRow("Arc 180 degree, radius 150") << 150.0 << 0.0 << 180.0;
    QTest::newRow("Arc 180 degree, radius -150") << -150.0 << 0.0 << 180.0;
    QTest::newRow("Arc 180 degree, radius 1500") << 1500.0 << 0.0 << 180.0;
    QTest::newRow("Arc 180 degree, radius -1500") << -1500.0 << 0.0 << 180.0;
    QTest::newRow("Arc 180 degree, radius 50000") << 50000.0 << 0.0 << 180.0;
    QTest::newRow("Arc 180 degree, radius -50000") << -50000.0 << 0.0 << 180.0;
    QTest::newRow("Arc 180 degree, radius 90000") << 90000.0 << 0.0 << 180.0;
    QTest::newRow("Arc 180 degree, radius -90000") << -90000.0 << 0.0 << 180.0;

    QTest::newRow("Arc less than 270 degree, radius 100") << 100.0 << 0.0 << 150.3;
    QTest::newRow("Arc less than 270 degree, radius -100") << -100.0 << 0.0 << 150.3;
    QTest::newRow("Arc less than 270 degree, radius 150") << 150.0 << 0.0 << 150.3;
    QTest::newRow("Arc less than 270 degree, radius -150") << -150.0 << 0.0 << 150.3;
    QTest::newRow("Arc less than 270 degree, radius 1500") << 1500.0 << 0.0 << 150.3;
    QTest::newRow("Arc less than 270 degree, radius -1500") << -1500.0 << 0.0 << 150.3;
    QTest::newRow("Arc less than 270 degree, radius 50000") << 50000.0 << 0.0 << 150.3;
    QTest::newRow("Arc less than 270 degree, radius -50000") << -50000.0 << 0.0 << 150.3;
    QTest::newRow("Arc less than 270 degree, radius 90000") << 90000.0 << 0.0 << 150.3;
    QTest::newRow("Arc less than 270 degree, radius -90000") << -90000.0 << 0.0 << 150.3;

    QTest::newRow("Arc 270 degree, radius 100") << 100.0 << 0.0 << 270.0;
    QTest::newRow("Arc 270 degree, radius -100") << -100.0 << 0.0 << 270.0;
    QTest::newRow("Arc 270 degree, radius 150") << 150.0 << 0.0 << 270.0;
    QTest::newRow("Arc 270 degree, radius -150") << -150.0 << 0.0 << 270.0;
    QTest::newRow("Arc 270 degree, radius 1500") << 1500.0 << 0.0 << 270.0;
    QTest::newRow("Arc 270 degree, radius -1500") << -1500.0 << 0.0 << 270.0;
    QTest::newRow("Arc 270 degree, radius 50000") << 50000.0 << 0.0 << 270.0;
    QTest::newRow("Arc 270 degree, radius -50000") << -50000.0 << 0.0 << 270.0;
    QTest::newRow("Arc 270 degree, radius 90000") << 90000.0 << 0.0 << 270.0;
    QTest::newRow("Arc 270 degree, radius -90000") << -90000.0 << 0.0 << 270.0;

    QTest::newRow("Arc less than 360 degree, radius 100") << 100.0 << 0.0 << 340.0;
    QTest::newRow("Arc less than 360 degree, radius -100") << -100.0 << 0.0 << 340.0;
    QTest::newRow("Arc less than 360 degree, radius 150") << 150.0 << 0.0 << 340.0;
    QTest::newRow("Arc less than 360 degree, radius -150") << -150.0 << 0.0 << 340.0;
    QTest::newRow("Arc less than 360 degree, radius 1500") << 1500.0 << 0.0 << 340.0;
    QTest::newRow("Arc less than 360 degree, radius -1500") << -1500.0 << 0.0 << 340.0;
    QTest::newRow("Arc less than 360 degree, radius 50000") << 50000.0 << 0.0 << 340.0;
    QTest::newRow("Arc less than 360 degree, radius -50000") << -50000.0 << 0.0 << 340.0;
    QTest::newRow("Arc less than 360 degree, radius 90000") << 90000.0 << 0.0 << 340.0;
    QTest::newRow("Arc less than 360 degree, radius -90000") << -90000.0 << 0.0 << 340.0;

    QTest::newRow("Arc start 90 degree, angle 45 degree, radius 100") << 100.0 << 90.0 << 135.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radius -100") << -100.0 << 90.0 << 135.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radius 150") << 150.0 << 90.0 << 135.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radius -150") << -150.0 << 90.0 << 135.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radius 1500") << 1500.0 << 90.0 << 135.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radius -1500") << -1500.0 << 90.0 << 135.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radius 50000") << 50000.0 << 90.0 << 135.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radius -50000") << -50000.0 << 90.0 << 135.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radius 90000") << 90000.0 << 90.0 << 135.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radius -90000") << -90000.0 << 90.0 << 135.0;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VArc::TestCurvature()
{
    QFETCH(qreal, radius);
    QFETCH(qreal, startAngle);
    QFETCH(qreal, endAngle);

    const qreal tolerance = 0.1;
    qreal const expectedCurvature = 1. / qAbs(radius);
    qreal const scale = VArc::OptimalApproximationScale(radius, startAngle, endAngle, 0.1);

    const VPointF center;
    VArc arc(center, radius, startAngle, endAngle);
    arc.SetApproximationScale(scale);

    qreal const curvature = VAbstractCurve::Curvature(arc.GetPoints());

    if (scale < 10)
    {
        QVERIFY(expectedCurvature - curvature <= expectedCurvature * tolerance);

        QVector<QPointF> const p1 = arc.GetPoints();
        arc.SetApproximationScale(10);
        QVector<QPointF> const p2 = arc.GetPoints();
        QVERIFY(p1.size() <= p2.size());
    }
}
