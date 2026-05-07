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

    const VArc arcOrigin(VPointF(center), radius, startAngle, endAngle);
    const VArc rotatedArc = arcOrigin.Rotate(rotatePoint, degrees, prefix);

    QCOMPARE(arcOrigin.GetLength(), rotatedArc.GetLength());
    QCOMPARE(arcOrigin.AngleArc(), rotatedArc.AngleArc());
    QCOMPARE(arcOrigin.GetRadius(), rotatedArc.GetRadius());
    QCOMPARE(arcOrigin.IsFlipped(), rotatedArc.IsFlipped());
    QCOMPARE(arcOrigin.IsReversed(), rotatedArc.IsReversed());
    // cppcheck-suppress unreadVariable
    const QString errorMsg = u"The name doesn't contain the prefix '%1'."_s.arg(prefix);
    QVERIFY2(rotatedArc.name().endsWith(prefix), qUtf8Printable(errorMsg));
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

    VArc const originArc(VPointF(center), radius, startAngle, endAngle);
    const qreal length1 = qAbs(originArc.GetLength());

    const VArc flipped = originArc.Flip(axis, prefix);
    const qreal length2 = qAbs(flipped.GetLength());

    // Name contains prefix
    const QString errorMsg = u"The name doesn't contain the prefix '%1'."_s.arg(prefix);
    QVERIFY2(flipped.name().endsWith(prefix), qUtf8Printable(errorMsg));

    // IsFlipped is toggled
    QCOMPARE(flipped.IsFlipped(), true);

    QString const errorLengthMsg
        = u"Difference between original and flipped lengthes bigger than eps = %1. l1 = %2; l2 = %3"_s;
    QVERIFY2(qAbs(length1 - length2) <= ToPixel(1, Unit::Mm),
             qUtf8Printable(errorLengthMsg.arg(ToPixel(1, Unit::Mm)).arg(length1).arg(length2)));

    // Radius and sweep angle are preserved
    QCOMPARE(originArc.GetRadius(), flipped.GetRadius());
    QCOMPARE(originArc.AngleArc(), flipped.AngleArc());

    // Geometric correctness: center, p1, p2 are mirrored onto the axis
    const QPointF expectedCenter = VPointF::FlipPF(axis, center);
    const QPointF expectedP1 = VPointF::FlipPF(axis, originArc.GetP1());
    const QPointF expectedP2 = VPointF::FlipPF(axis, originArc.GetP2());

    const auto flippedCenter = static_cast<QPointF>(flipped.GetCenter());
    QVERIFY2(VFuzzyComparePoints(flippedCenter, expectedCenter), qUtf8Printable(u"Center mismatch after flip"_s));
    QVERIFY2(VFuzzyComparePoints(flipped.GetP1(), expectedP1), qUtf8Printable(u"P1 mismatch after flip"_s));
    QVERIFY2(VFuzzyComparePoints(flipped.GetP2(), expectedP2), qUtf8Printable(u"P2 mismatch after flip"_s));

    // Double flip restores original arc
    const VArc doubleFlipped = flipped.Flip(axis, prefix);
    QVERIFY2(VFuzzyComparePoints(static_cast<QPointF>(doubleFlipped.GetCenter()), center),
             qUtf8Printable(u"Center mismatch after double flip"_s));
    QVERIFY2(VFuzzyComparePoints(doubleFlipped.GetP1(), originArc.GetP1()),
             qUtf8Printable(u"P1 mismatch after double flip"_s));
    QVERIFY2(VFuzzyComparePoints(doubleFlipped.GetP2(), originArc.GetP2()),
             qUtf8Printable(u"P2 mismatch after double flip"_s));
    QCOMPARE(doubleFlipped.GetLength(), originArc.GetLength());
    QCOMPARE(doubleFlipped.AngleArc(), originArc.AngleArc());
    QVERIFY2(doubleFlipped.IsFlipped() == originArc.IsFlipped(),
             qUtf8Printable(u"IsFlipped mismatch after double flip"_s));
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
