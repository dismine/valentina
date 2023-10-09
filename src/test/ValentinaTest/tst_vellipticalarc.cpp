/************************************************************************
 **
 **  @file   tst_vellipticalarc.cpp
 **  @author Valentina Zhuravska <zhuravska19(at)gmail.com>
 **  @date   12 2, 2016
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

#include "tst_vellipticalarc.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vlayout/vabstractpiece.h"

#include <QtGlobal>
#include <QtTest>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
TST_VEllipticalArc::TST_VEllipticalArc(QObject *parent)
  : AbstractTest(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::CompareTwoWays_data()
{
    QTest::addColumn<QPointF>("c");
    QTest::addColumn<qreal>("radius1");
    QTest::addColumn<qreal>("radius2");
    QTest::addColumn<qreal>("f1");
    QTest::addColumn<qreal>("f2");
    QTest::addColumn<qreal>("rotationAngle");

    QTest::newRow("Test case 1") << QPointF() << 100. << 200. << 0. << 90.0 << 0.;
    QTest::newRow("Test case 2") << QPointF() << 100. << 200. << 0. << 180.0 << 0.;
    QTest::newRow("Test case 3") << QPointF() << 100. << 200. << 0. << 270.0 << 0.;
    QTest::newRow("Test case 4") << QPointF() << 100. << 200. << 0. << 360.0 << 0.;
    QTest::newRow("Test case 5") << QPointF(10, 10) << 100. << 200. << 0. << 90.0 << 80.;
    QTest::newRow("Test case 6") << QPointF(10, 10) << 100. << 200. << 0. << 180.0 << 80.;
    QTest::newRow("Test case 7") << QPointF(10, 10) << 100. << 200. << 0. << 270.0 << 80.;
    QTest::newRow("Test case 8") << QPointF(10, 10) << 100. << 200. << 0. << 360.0 << 80.;
    QTest::newRow("Test case 9") << QPointF() << 100. << 200. << 0. << 90.0 << 80.;
    QTest::newRow("Test case 10") << QPointF() << 100. << 200. << 0. << 180.0 << 80.;
    QTest::newRow("Test case 11") << QPointF() << 100. << 200. << 0. << 270.0 << 80.;
    QTest::newRow("Test case 12") << QPointF() << 100. << 200. << 0. << 360.0 << 80.;
    QTest::newRow("Test case 13") << QPointF(10, 10) << 100. << 200. << 0. << 90.0 << 80.;
    QTest::newRow("Test case 14") << QPointF(10, 10) << 100. << 200. << 0. << 180.0 << 80.;
    QTest::newRow("Test case 15") << QPointF(10, 10) << 100. << 200. << 0. << 270.0 << 80.;
    QTest::newRow("Test case 16") << QPointF(10, 10) << 100. << 200. << 0. << 360.0 << 80.;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VEllipticalArc::CompareTwoWays()
{
    QFETCH(QPointF, c);
    QFETCH(qreal, radius1);
    QFETCH(qreal, radius2);
    QFETCH(qreal, f1);
    QFETCH(qreal, f2);
    QFETCH(qreal, rotationAngle);

    const VPointF center(c);

    VEllipticalArc arc1(center, radius1, radius2, f1, f2, rotationAngle);
    const qreal length = arc1.GetLength();

    VEllipticalArc arc2(length, center, radius1, radius2, f1, rotationAngle);

    const qreal lengthEps = ToPixel(0.45, Unit::Mm); // computing error

    // cppcheck-suppress unreadVariable
    QString errorLengthMsg = u"Difference between real and computing lengthes bigger than eps = %1. l1 = %2; l2 = %3"_s;
    QVERIFY2(qAbs(arc2.GetLength() - length) <= lengthEps,
             qUtf8Printable(errorLengthMsg.arg(lengthEps).arg(arc2.GetLength()).arg(length)));
    QVERIFY2(qAbs(arc1.GetLength() - arc2.GetLength()) <= lengthEps,
             qUtf8Printable(errorLengthMsg.arg(lengthEps).arg(arc2.GetLength()).arg(arc2.GetLength())));

    const qreal angleEps = 0.4;
    // cppcheck-suppress unreadVariable
    const QString errorAngleMsg =
        u"Difference between real and computing angles bigger than eps = %1. f1 = %2; f2 = %3"_s;
    // compare angles
    const qreal diff = qAbs(arc1.GetEndAngle() - arc2.GetEndAngle());
    QVERIFY2(qAbs(diff - 360.0 * (diff / 360.0)) <= angleEps,
             qUtf8Printable(errorAngleMsg.arg(angleEps).arg(arc1.GetEndAngle()).arg(arc2.GetEndAngle())));
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VEllipticalArc::NegativeArc()
{
    const VPointF center;
    const qreal radius1 = 100;
    const qreal radius2 = 200;
    const qreal f1 = 1;
    const qreal f2 = 181;
    const qreal rotationAngle = 0;

    // Full ellipse
    const qreal h = ((radius1 - radius2) * (radius1 - radius2)) / ((radius1 + radius2) * (radius1 + radius2));
    const qreal length = M_PI * (radius1 + radius2) * (1 + 3 * h / (10 + qSqrt(4 - 3 * h))) / 2;
    VEllipticalArc arc(-length, center, radius1, radius2, f1, rotationAngle);

    const qreal eps = ToPixel(0.45, Unit::Mm); // computing error
    // cppcheck-suppress unreadVariable
    const QString errorMsg =
        u"Difference between real and computing lengthes bigger than eps = %1.  v1 = %2; v2 = %3"_s;

    QVERIFY2(qAbs(arc.GetLength() + length) <= eps, qUtf8Printable(errorMsg.arg(eps).arg(arc.GetLength()).arg(length)));

    const qreal angleEps = 0.4;
    QVERIFY2(arc.GetEndAngle() - f2 <= angleEps, qUtf8Printable(errorMsg.arg(eps).arg(arc.GetEndAngle()).arg(f2)));
}

// cppcheck-suppress unusedFunction
//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestGetPoints1_data()
{
    TestData();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestGetPoints2_data()
{
    TestData();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestGetPoints3_data()
{
    TestData();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestGetPoints4_data()
{
    TestData();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestData()
{
    QTest::addColumn<qreal>("radius1");
    QTest::addColumn<qreal>("radius2");
    QTest::addColumn<qreal>("startAngle");
    QTest::addColumn<qreal>("endAngle");
    QTest::addColumn<qreal>("rotationAngle");

    QTest::newRow("Full circle: radiuses 10, 20; start 0") << 10.0 << 20.0 << 0.0 << 360.0 << 0.0;
    QTest::newRow("Full circle: radiuses 150, 200; start 0") << 150.0 << 200.0 << 0.0 << 360.0 << 0.0;
    QTest::newRow("Full circle: radiuses 150, 200, rotation 30; start 0") << 150.0 << 200.0 << 0.0 << 360.0 << 30.0;
    QTest::newRow("Full circle: radiuses 1500, 1000; start 0") << 1500.0 << 1000.0 << 0.0 << 360.0 << 0.0;
    QTest::newRow("Full circle: radiuses 1500, 1000, rotation 50; start 0") << 1500.0 << 1000.0 << 0.0 << 360.0 << 50.0;
    QTest::newRow("Full circle: radiuses 15000, 10000, rotation 90; start 0")
        << 15000.0 << 10000.0 << 0.0 << 360.0 << 90.0;

    QTest::newRow("Full circle: radiuses 10, 20; start 90") << 10.0 << 20.0 << 90.0 << 90.0 << 0.0;
    QTest::newRow("Full circle: radiuses 150, 200; start 90") << 150.0 << 200.0 << 90.0 << 90.0 << 0.0;
    QTest::newRow("Full circle: radiuses 150, 200, rotation 30; start 90") << 150.0 << 200.0 << 90.0 << 90.0 << 30.0;
    QTest::newRow("Full circle: radiuses 1500, 1000; start 90") << 1500.0 << 1000.0 << 90.0 << 90.0 << 0.0;
    QTest::newRow("Full circle: radiuses 1500, 1000, rotation 50; start 90")
        << 1500.0 << 1000.0 << 90.0 << 90.0 << 50.0;
    QTest::newRow("Full circle: radiuses 15000, 10000, rotation 90; start 90")
        << 15000.0 << 10000.0 << 90.0 << 90.0 << 90.0;

    QTest::newRow("Arc less than 45 degree, radiuses 100, 50") << 100.0 << 50.0 << 0.0 << 10.5 << 0.0;
    QTest::newRow("Arc less than 45 degree, radiuses 150, 50, rotation 180") << 150.0 << 50.0 << 0.0 << 10.5 << 180.0;
    QTest::newRow("Arc less than 45 degree, radiuses 1500, 800, rotation 90") << 1500.0 << 800.0 << 0.0 << 10.5 << 90.0;
    QTest::newRow("Arc less than 45 degree, radiuses 15000, 10000, rotation 40")
        << 50000.0 << 10000.0 << 0.0 << 10.5 << 40.0;
    QTest::newRow("Arc less than 45 degree, radiuses 15000, 10000") << 15000.0 << 10000.0 << 0.0 << 10.5 << 0.0;

    QTest::newRow("Arc 45 degree, radiuses 100, 50, rotation 45") << 100.0 << 50.0 << 0.0 << 45.0 << 45.0;
    QTest::newRow("Arc 45 degree, radiuses 150, 15, rotation 30") << 150.0 << 15.0 << 0.0 << 45.0 << 30.0;
    QTest::newRow("Arc 45 degree, radiuses 1500, 150, rotation 45") << 1500.0 << 150.0 << 0.0 << 45.0 << 45.0;
    QTest::newRow("Arc 45 degree, radiuses 15000, 15000") << 15000.0 << 15000.0 << 0.0 << 45.0 << 0.0;
    QTest::newRow("Arc 45 degree, radiuses 15000, 10000, rotation 270") << 15000.0 << 10000.0 << 0.0 << 45.0 << 270.0;

    QTest::newRow("Arc less than 90 degree, radiuses 100, 400, rotation 50") << 100.0 << 400.0 << 0.0 << 75.0 << 50.0;
    QTest::newRow("Arc less than 90 degree, radiuses 150, 400, rotation 90") << 150.0 << 400.0 << 0.0 << 75.0 << 90.0;
    QTest::newRow("Arc less than 90 degree, radiuses 1500, 50000, rotation 180")
        << 1500.0 << 50000.0 << 0.0 << 75.0 << 180.0;
    QTest::newRow("Arc less than 90 degree, radiuses 50000, 5000, rotation 30")
        << 50000.0 << 5000.0 << 0.0 << 75.0 << 30.0;
    QTest::newRow("Arc less than 90 degree, radiuses 90000, 50000, rotation 30")
        << 90000.0 << 50000.0 << 0.0 << 75.0 << 30.0;

    QTest::newRow("Arc 90 degree, radiuses 100, 50, rotation 30") << 100.0 << 50.0 << 0.0 << 90.0 << 30.0;
    QTest::newRow("Arc 90 degree, radiuses 150, 400") << 150.0 << 400.0 << 0.0 << 90.0 << 0.0;
    QTest::newRow("Arc 90 degree, radiuses 1500, 800, rotation 70") << 1500.0 << 800.0 << 0.0 << 90.0 << 70.0;
    QTest::newRow("Arc 90 degree, radiuses 15000, 5000, rotation 30") << 15000.0 << 1500.0 << 0.0 << 90.0 << 30.0;
    QTest::newRow("Arc 90 degree, radiuses 15000, 14000, rotation 235") << 15000.0 << 14000.0 << 0.0 << 90.0 << 235.0;

    QTest::newRow("Arc less than 135 degree, radiuses 100, 50, rotation 60") << 100.0 << 50.0 << 0.0 << 110.6 << 60.0;
    QTest::newRow("Arc less than 135 degree, radiuses 150, 400, rotation 300")
        << 150.0 << 400.0 << 0.0 << 110.6 << 300.0;
    QTest::newRow("Arc less than 135 degree, radiuses 1500, 800, rotation 360")
        << 1500.0 << 800.0 << 0.0 << 110.6 << 360.0;
    QTest::newRow("Arc less than 135 degree, radiuses 15000, 1500, rotation 290")
        << 15000.0 << 1500.0 << 0.0 << 110.6 << 290.0;
    QTest::newRow("Arc less than 135 degree, radiuses 15000, 1500") << 15000.0 << 1500.0 << 0.0 << 110.6 << 0.0;

    QTest::newRow("Arc 135 degree, radiuses 100, 50") << 100.0 << 50.0 << 0.0 << 135.0 << 0.0;
    QTest::newRow("Arc 135 degree, radiuses 150, 400") << 150.0 << 400.0 << 0.0 << 135.0 << 0.0;
    QTest::newRow("Arc 135 degree, radiuses 1500, 800") << 1500.0 << 800.0 << 0.0 << 135.0 << 0.0;
    QTest::newRow("Arc 135 degree, radiuses 15000, 1500, rotation 20") << 15000.0 << 1500.0 << 0.0 << 135.0 << 20.0;
    QTest::newRow("Arc 135 degree, radiuses 15000, 10000") << 15000.0 << 10000.0 << 0.0 << 135.0 << 0.0;

    QTest::newRow("Arc less than 180 degree, radiuses 100, 50") << 100.0 << 50.0 << 0.0 << 160.7 << 0.0;
    QTest::newRow("Arc less than 180 degree, radiuses 150, 400") << 150.0 << 400.0 << 0.0 << 160.7 << 0.0;
    QTest::newRow("Arc less than 180 degree, radiuses 1500, 800") << 1500.0 << 800.0 << 0.0 << 160.7 << 0.0;
    QTest::newRow("Arc less than 180 degree, radiuses 15000, 1500, rotation 270")
        << 15000.0 << 1500.0 << 0.0 << 160.7 << 270.0;
    QTest::newRow("Arc less than 180 degree, radiuses 15000, 10000") << 15000.0 << 10000.0 << 0.0 << 160.7 << 0.0;

    QTest::newRow("Arc 180 degree, radiuses 100, 50") << 100.0 << 50.0 << 0.0 << 180.0 << 0.0;
    QTest::newRow("Arc 180 degree, radiuses 150, 400") << 150.0 << 400.0 << 0.0 << 180.0 << 0.0;
    QTest::newRow("Arc 180 degree, radiuses 1500, 800") << 1500.0 << 800.0 << 0.0 << 180.0 << 0.0;
    QTest::newRow("Arc 180 degree, radiuses 15000, 1500, rotation 60") << 15000.0 << 1500.0 << 0.0 << 180.0 << 60.0;
    QTest::newRow("Arc 180 degree, radiuses 15000, 10000") << 15000.0 << 10000.0 << 0.0 << 180.0 << 0.0;

    QTest::newRow("Arc less than 270 degree, radiuses 100, 50") << 100.0 << 50.0 << 0.0 << 150.3 << 0.0;
    QTest::newRow("Arc less than 270 degree, radiuses 150, 400") << 150.0 << 400.0 << 0.0 << 150.3 << 0.0;
    QTest::newRow("Arc less than 270 degree, radiuses 1500, 800") << 1500.0 << 800.0 << 0.0 << 150.3 << 0.0;
    QTest::newRow("Arc less than 270 degree, radiuses 15000, 1500, rotation 20")
        << 15000.0 << 1500.0 << 0.0 << 150.3 << 20.0;
    QTest::newRow("Arc less than 270 degree, radiuses 15000, 10000") << 15000.0 << 10000.0 << 0.0 << 150.3 << 0.0;

    QTest::newRow("Arc 270 degree, radiuses 100, 50") << 100.0 << 50.0 << 0.0 << 270.0 << 0.0;
    QTest::newRow("Arc 270 degree, radiuses 150, 400") << 150.0 << 400.0 << 0.0 << 270.0 << 0.0;
    QTest::newRow("Arc 270 degree, radiuses 1500, 800") << 1500.0 << 800.0 << 0.0 << 270.0 << 0.0;
    QTest::newRow("Arc 270 degree, radiuses 15000, 1500, rotation 90") << 15000.0 << 1500.0 << 0.0 << 270.0 << 90.0;
    QTest::newRow("Arc 270 degree, radiuses 15000, 10000") << 15000.0 << 10000.0 << 0.0 << 270.0 << 0.0;

    QTest::newRow("Arc less than 360 degree, radiuses 100, 50") << 100.0 << 50.0 << 0.0 << 340.0 << 0.0;
    QTest::newRow("Arc less than 360 degree, radiuses 150, 400") << 150.0 << 400.0 << 0.0 << 340.0 << 0.0;
    QTest::newRow("Arc less than 360 degree, radiuses 1500, 800") << 1500.0 << 800.0 << 0.0 << 340.0 << 0.0;
    QTest::newRow("Arc less than 360 degree, radiuses 12000, 1200, rotation 30")
        << 12000.0 << 1200.0 << 0.0 << 340.0 << 30.0;
    QTest::newRow("Arc less than 360 degree, radiuses 12000, 10000") << 15000.0 << 10000.0 << 0.0 << 340.0 << 0.0;

    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses 100, 50") << 100.0 << 50.0 << 90.0 << 135.0 << 0.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses 150, 400") << 150.0 << 400.0 << 90.0 << 135.0 << 0.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses 1500, 800")
        << 1500.0 << 800.0 << 90.0 << 135.0 << 0.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses 13000, 1000")
        << 15000.0 << 1000.0 << 90.0 << 135.0 << 0.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses 15000, 10000")
        << 15000.0 << 10000.0 << 90.0 << 135.0 << 0.0;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VEllipticalArc::TestGetPoints1()
{
    // Any point must satisfy the equation of ellipse
    QFETCH(qreal, radius1);
    QFETCH(qreal, radius2);
    QFETCH(qreal, startAngle);
    QFETCH(qreal, endAngle);
    QFETCH(qreal, rotationAngle);

    const VPointF center;
    VEllipticalArc arc(center, radius1, radius2, startAngle, endAngle, rotationAngle);
    arc.SetApproximationScale(maxCurveApproximationScale);

    QVector<QPointF> points = arc.GetPoints();
    if (qFuzzyIsNull(rotationAngle))
    { // equation of ellipse will be different when rotation angle isn't 0 so we can't use this test in this case
        const qreal eps = 0.05;

        for (auto p : points)
        {
            const qreal equationRes = p.rx() * p.rx() / (radius1 * radius1) + p.ry() * p.ry() / (radius2 * radius2);
            const qreal diff = qAbs(equationRes - 1);
            // cppcheck-suppress unreadVariable
            const QString errorMsg = u"Broken the first rule. Any point must satisfy the equation of ellipse."
                                     u"diff = '%1' > eps = '%2'"_s.number(diff)
                                         .number(eps);
            QVERIFY2(diff <= eps, qUtf8Printable(errorMsg));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VEllipticalArc::TestGetPoints2()
{
    // Distance from the any point to the focus1 plus distance from this point to the focus2 should be the same.
    QFETCH(qreal, radius1);
    QFETCH(qreal, radius2);
    QFETCH(qreal, startAngle);
    QFETCH(qreal, endAngle);
    QFETCH(qreal, rotationAngle);

    const VPointF center;
    VEllipticalArc arc(center, radius1, radius2, startAngle, endAngle, rotationAngle);
    arc.SetApproximationScale(maxCurveApproximationScale);
    QVector<QPointF> points = arc.GetPoints();

    const qreal c = qSqrt(qAbs(radius2 * radius2 - radius1 * radius1));
    // distance from the center to the focus

    QPointF focus1 = static_cast<QPointF>(center);
    QPointF focus2 = static_cast<QPointF>(center);

    if (radius1 < radius2)
    {
        focus1.setY(focus1.ry() + c);
        QLineF line(static_cast<QPointF>(center), focus1);
        line.setAngle(line.angle() + rotationAngle);
        focus1 = line.p2();

        focus2.setY(focus2.ry() - c);
        line.setP2(focus2);
        line.setAngle(line.angle() + rotationAngle);
        focus2 = line.p2();
    }
    else
    {
        focus1.setX(focus1.rx() + c);
        QLineF line(static_cast<QPointF>(center), focus1);
        line.setAngle(line.angle() + rotationAngle);
        focus1 = line.p2();

        focus2.setX(focus2.rx() - c);
        line.setP2(focus2);
        line.setAngle(line.angle() + rotationAngle);
        focus2 = line.p2();
    }

    QPointF ellipsePoint(center.x() + radius1, center.y());
    QLineF line(static_cast<QPointF>(center), ellipsePoint);
    line.setAngle(line.angle() + rotationAngle);
    ellipsePoint = line.p2();

    const QLineF distance1(focus1, ellipsePoint);
    const QLineF distance2(focus2, ellipsePoint);

    const qreal distance = distance1.length() + distance2.length();
    const qreal eps = distance * 1.1 / 100; // computing error 1.1 % from origin distance
    for (int i = 0; i < points.size(); ++i)
    {
        const QLineF rLine1(focus1, points.at(i));
        const QLineF rLine2(focus2, points.at(i));
        const qreal resultingDistance = rLine1.length() + rLine2.length();
        const qreal diff = qAbs(resultingDistance - distance);
        // cppcheck-suppress unreadVariable
        const QString errorMsg = u"Broken the first rule, part 2. Distance from the any point to the focus1"
                                 u" plus distance from this point to the focus2 should be the same. Problem"
                                 u" with point '%1'. The disired distance is '%2', but resulting distance"
                                 u" is '%3'. Difference is '%4' and it biggest than eps '%5')"_s.number(i)
                                     .number(distance)
                                     .number(resultingDistance)
                                     .number(diff)
                                     .number(eps);
        QVERIFY2(diff <= eps, qUtf8Printable(errorMsg));
    }
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VEllipticalArc::TestGetPoints3()
{
    // Compare full ellipse square with square of VAbstractDetail
    QFETCH(qreal, radius1);
    QFETCH(qreal, radius2);
    QFETCH(qreal, startAngle);
    QFETCH(qreal, endAngle);
    QFETCH(qreal, rotationAngle);

    const VPointF center;
    VEllipticalArc arc(center, radius1, radius2, startAngle, endAngle, rotationAngle);
    arc.SetApproximationScale(maxCurveApproximationScale);
    QVector<QPointF> points = arc.GetPoints();

    if (VFuzzyComparePossibleNulls(arc.AngleArc(), 360.0))
    { // calculated full ellipse square
        const qreal ellipseSquare = M_PI * radius1 * radius2;
        const qreal epsSquare = ellipseSquare * 1.7 / 100; // computing error 1.7 % from origin square
        const qreal arcSquare = qAbs(VAbstractPiece::SumTrapezoids(points) / 2.0);
        const qreal diffSquare = qAbs(ellipseSquare - arcSquare);
        // cppcheck-suppress unreadVariable
        const QString errorMsg1 = u"Broken the second rule. Interpolation has too big computing error. "
                                  u"Difference ='%1' bigger than eps = '%2'."_s.arg(diffSquare)
                                      .arg(epsSquare);
        QVERIFY2(diffSquare <= epsSquare, qUtf8Printable(errorMsg1));
    }
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_VEllipticalArc::TestGetPoints4()
{
    // Compare real full ellipse length with calculated
    QFETCH(qreal, radius1);
    QFETCH(qreal, radius2);
    QFETCH(qreal, startAngle);
    QFETCH(qreal, endAngle);
    QFETCH(qreal, rotationAngle);

    const VPointF center;
    VEllipticalArc arc(center, radius1, radius2, startAngle, endAngle, rotationAngle);
    arc.SetApproximationScale(maxCurveApproximationScale);

    if (VFuzzyComparePossibleNulls(arc.AngleArc(), 360.0))
    { // calculated full ellipse length
        const qreal h = ((radius1 - radius2) * (radius1 - radius2)) / ((radius1 + radius2) * (radius1 + radius2));
        const qreal ellipseLength = M_PI * (radius1 + radius2) * (1 + 3 * h / (10 + qSqrt(4 - 3 * h)));
        const qreal epsLength = ToPixel(1, Unit::Mm); // computing error
        const qreal arcLength = VEllipticalArc(center, radius1, radius2, 0, 360, 0).GetLength();
        const qreal diffLength = qAbs(arcLength - ellipseLength);
        // cppcheck-suppress unreadVariable
        const QString errorMsg2 = u"Difference between real and computing lengthes "
                                  u"(diff = '%1') bigger than eps = '%2'."_s.arg(diffLength)
                                      .arg(epsLength);
        QVERIFY2(diffLength <= epsLength, qUtf8Printable(errorMsg2));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestGetPoints5_data()
{
    TestData();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestGetPoints5()
{
    // Test if first and last point still have same angle
    QFETCH(qreal, radius1);
    QFETCH(qreal, radius2);
    QFETCH(qreal, startAngle);
    QFETCH(qreal, endAngle);
    QFETCH(qreal, rotationAngle);

    const VPointF center;
    VEllipticalArc arc(center, radius1, radius2, startAngle, endAngle, rotationAngle);
    arc.SetApproximationScale(maxCurveApproximationScale);

    const qreal stAngle = VEllipticalArc::OptimizeAngle(arc.GetStartAngle() + arc.GetRotationAngle());
    const qreal enAngle = VEllipticalArc::OptimizeAngle(arc.GetEndAngle() + arc.GetRotationAngle());

    qreal f1 = QLineF(static_cast<QPointF>(center), arc.GetP1()).angle();
    if ((qFuzzyIsNull(f1) && VFuzzyComparePossibleNulls(360, stAngle)) ||
        (VFuzzyComparePossibleNulls(360, f1) && qFuzzyIsNull(stAngle)))
    {
        f1 = stAngle;
    }

    qreal f2 = QLineF(static_cast<QPointF>(center), arc.GetP2()).angle();
    if ((qFuzzyIsNull(f2) && VFuzzyComparePossibleNulls(360, enAngle)) ||
        (VFuzzyComparePossibleNulls(360, f2) && qFuzzyIsNull(enAngle)))
    {
        f2 = enAngle;
    }

    QCOMPARE(f1, stAngle);
    QCOMPARE(f2, enAngle);

    QVector<QPointF> points = arc.GetPoints();

    if (points.size() > 2 && qFuzzyIsNull(rotationAngle))
    {
        const qreal testAccuracy = ToPixel(1.5, Unit::Mm);
        ComparePointsDistance(arc.GetP1(), points.constFirst(), testAccuracy);
        ComparePointsDistance(arc.GetP2(), points.constLast(), testAccuracy);

        const qreal eps = 0.15;

        f1 = QLineF(static_cast<QPointF>(center), points.constFirst()).angle();
        QVERIFY2(f1 - stAngle <= eps, qUtf8Printable(QStringLiteral("f1: %1; expected: %2").arg(f1).arg(stAngle)));

        f2 = QLineF(static_cast<QPointF>(center), points.constLast()).angle();
        QVERIFY2(f2 - enAngle <= eps, qUtf8Printable(QStringLiteral("f2: %1; expected: %2").arg(f2).arg(enAngle)));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestRotation_data()
{
    QTest::addColumn<QPointF>("center");
    QTest::addColumn<qreal>("radius1");
    QTest::addColumn<qreal>("radius2");
    QTest::addColumn<qreal>("startAngle");
    QTest::addColumn<qreal>("endAngle");
    QTest::addColumn<qreal>("rotationAngle");
    QTest::addColumn<QPointF>("rotatePoint");
    QTest::addColumn<qreal>("degrees");
    QTest::addColumn<QString>("prefix");

    QTest::newRow("Test el arc 1") << QPointF() << 10. << 20.0 << 1. << 91. << 0. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 2") << QPointF() << 10. << 20.0 << 0. << 90. << 0. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 3.2") << QPointF(10, 10) << 10. << 20.0 << 0. << 90. << 0. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 3.1") << QPointF(10, 10) << 10. << 20.0 << 1. << 91. << 0. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 3") << QPointF(10, 10) << 10. << 20.0 << 1. << 91. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 4") << QPointF(10, 10) << 10. << 20.0 << 0. << 90. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 5") << QPointF(10, 10) << 10. << 20.0 << 0. << 180. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 6") << QPointF(10, 10) << 10. << 20.0 << 1. << 181. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 7") << QPointF(10, 10) << 10. << 20.0 << 0. << 270. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 8") << QPointF(10, 10) << 10. << 20.0 << 1. << 271. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 9") << QPointF(10, 10) << 10. << 20.0 << 0. << 360. << 90. << QPointF() << 90. << "_r";
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestRotation()
{
    QFETCH(QPointF, center);
    QFETCH(qreal, radius1);
    QFETCH(qreal, radius2);
    QFETCH(qreal, startAngle);
    QFETCH(qreal, endAngle);
    QFETCH(qreal, rotationAngle);
    QFETCH(QPointF, rotatePoint);
    QFETCH(qreal, degrees);
    QFETCH(QString, prefix);

    const VEllipticalArc arcOrigin(VPointF(center), radius1, radius2, startAngle, endAngle, rotationAngle);
    const VEllipticalArc rotatedArc = arcOrigin.Rotate(rotatePoint, degrees, prefix);

    QVERIFY2(qAbs(arcOrigin.AngleArc() - rotatedArc.AngleArc()) <= 1.6,
             qUtf8Printable(u"a1 = %1, a2 - %2"_s.arg(arcOrigin.AngleArc()).arg(rotatedArc.AngleArc())));

    // cppcheck-suppress unreadVariable
    QString errorLengthMsg = u"Difference between real and computing lengthes bigger than eps = %1. l1 = %2; l2 = %3"_s;
    QVERIFY2(qAbs(arcOrigin.GetLength() - rotatedArc.GetLength()) <= ToPixel(1, Unit::Mm),
             qUtf8Printable(
                 errorLengthMsg.arg(ToPixel(1, Unit::Mm)).arg(arcOrigin.GetLength()).arg(rotatedArc.GetLength())));

    QCOMPARE(arcOrigin.GetRadius1(), rotatedArc.GetRadius1());
    QCOMPARE(arcOrigin.GetRadius2(), rotatedArc.GetRadius2());
    QCOMPARE(arcOrigin.GetRotationAngle(), rotatedArc.GetRotationAngle());
    // cppcheck-suppress unreadVariable
    const QString errorMsg = u"The name doesn't contain the prefix '%1'."_s.arg(prefix);
    QVERIFY2(rotatedArc.name().endsWith(prefix), qUtf8Printable(errorMsg));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestFlip_data()
{
    QTest::addColumn<VEllipticalArc>("elArc");
    QTest::addColumn<QLineF>("axis");
    QTest::addColumn<QString>("prefix");

    const VEllipticalArc elArc(VPointF(), 10., 20.0, 1., 91., 0.);

    QLineF axis(QPointF(600, 30), QPointF(600, 1800));

    QTest::newRow("Vertical axis") << elArc << axis << "a2";

    axis = QLineF(QPointF(600, 30), QPointF(1200, 30));

    QTest::newRow("Horizontal axis") << elArc << axis << "a2";

    axis = QLineF(QPointF(600, 30), QPointF(600, 1800));
    axis.setAngle(45);

    QTest::newRow("Diagonal axis") << elArc << axis << "a2";
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestFlip()
{
    QFETCH(VEllipticalArc, elArc);
    QFETCH(QLineF, axis);
    QFETCH(QString, prefix);

    const VEllipticalArc res = elArc.Flip(axis, prefix);

    // cppcheck-suppress unreadVariable
    const QString errorMsg = QString("The name doesn't contain the prefix '%1'.").arg(prefix);
    QVERIFY2(res.name().endsWith(prefix), qUtf8Printable(errorMsg));

    QCOMPARE(qRound(elArc.GetLength() * -1), qRound(res.GetLength()));
    QCOMPARE(elArc.GetRadius1(), res.GetRadius1());
    QCOMPARE(elArc.GetRadius2(), res.GetRadius2());
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::EmptyArc_data()
{
    QTest::addColumn<qreal>("radius1");
    QTest::addColumn<qreal>("radius2");
    QTest::addColumn<qreal>("length");

    QTest::newRow("Empty elArc") << 0. << 0. << 0.;
    QTest::newRow("Radius1 correct") << 50. << 0. << 50. * 4;
    QTest::newRow("Radius2 correct") << 0. << 30. << 30. * 4;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::EmptyArc()
{
    QFETCH(qreal, radius1);
    QFETCH(qreal, radius2);
    QFETCH(qreal, length);

    VEllipticalArc empty;
    empty.SetApproximationScale(maxCurveApproximationScale);
    empty.SetRadius1(radius1);
    empty.SetRadius2(radius2);

    QVERIFY(qAbs(empty.GetLength() - length) <= ToPixel(1, Unit::Mm));
}
