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

namespace
{
struct CutResult
{
    QPointF cutPoint{};
    VEllipticalArc arc1{};
    VEllipticalArc arc2{};
};

//---------------------------------------------------------------------------------------------------------------------
auto MakeArc(qreal r1, qreal r2, qreal f1Deg, qreal f2Deg, qreal rotDeg) -> VEllipticalArc
{
    return {VPointF(), r1, r2, f1Deg, f2Deg, rotDeg};
}

//---------------------------------------------------------------------------------------------------------------------
auto DoCut(const VEllipticalArc &arc, qreal length) -> CutResult
{
    VEllipticalArc a1;
    VEllipticalArc a2;
    return {.cutPoint = arc.CutArc(length, &a1, &a2, QString()), .arc1 = a1, .arc2 = a2};
}

} // namespace

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

    QTest::newRow("Test case 1, +r, +r") << QPointF() << 100. << 200. << 0. << 90.0 << 0.;
    QTest::newRow("Test case 1, -r, +r") << QPointF() << -100. << 200. << 0. << 90.0 << 0.;
    QTest::newRow("Test case 1, +r, -r") << QPointF() << 100. << -200. << 0. << 90.0 << 0.;
    QTest::newRow("Test case 1, -r, -r") << QPointF() << -100. << -200. << 0. << 90.0 << 0.;

    QTest::newRow("Test case 2, +r, +r") << QPointF() << 100. << 200. << 0. << 180.0 << 0.;
    QTest::newRow("Test case 2, -r, +r") << QPointF() << -100. << 200. << 0. << 180.0 << 0.;
    QTest::newRow("Test case 2, +r, -r") << QPointF() << 100. << -200. << 0. << 180.0 << 0.;
    QTest::newRow("Test case 2, -r, -r") << QPointF() << -100. << -200. << 0. << 180.0 << 0.;

    QTest::newRow("Test case 3, +r, +r") << QPointF() << 100. << 200. << 0. << 270.0 << 0.;
    QTest::newRow("Test case 3, -r, +r") << QPointF() << -100. << 200. << 0. << 270.0 << 0.;
    QTest::newRow("Test case 3, +r, -r") << QPointF() << 100. << -200. << 0. << 270.0 << 0.;
    QTest::newRow("Test case 3, -r, -r") << QPointF() << -100. << -200. << 0. << 270.0 << 0.;

    QTest::newRow("Test case 4, +r, +r") << QPointF() << 100. << 200. << 0. << 360.0 << 0.;
    QTest::newRow("Test case 4, -r, +r") << QPointF() << -100. << 200. << 0. << 360.0 << 0.;
    QTest::newRow("Test case 4, +r, -r") << QPointF() << 100. << -200. << 0. << 360.0 << 0.;
    QTest::newRow("Test case 4, -r, -r") << QPointF() << -100. << -200. << 0. << 360.0 << 0.;

    QTest::newRow("Test case 5, +r, +r") << QPointF(10, 10) << 100. << 200. << 0. << 90.0 << 80.;
    QTest::newRow("Test case 5, -r, +r") << QPointF(10, 10) << -100. << 200. << 0. << 90.0 << 80.;
    QTest::newRow("Test case 5, +r, -r") << QPointF(10, 10) << 100. << -200. << 0. << 90.0 << 80.;
    QTest::newRow("Test case 5, -r, -r") << QPointF(10, 10) << -100. << -200. << 0. << 90.0 << 80.;

    QTest::newRow("Test case 6, +r, +r") << QPointF(10, 10) << 100. << 200. << 0. << 180.0 << 80.;
    QTest::newRow("Test case 6, -r, +r") << QPointF(10, 10) << -100. << 200. << 0. << 180.0 << 80.;
    QTest::newRow("Test case 6, +r, -r") << QPointF(10, 10) << 100. << -200. << 0. << 180.0 << 80.;
    QTest::newRow("Test case 6, -r, -r") << QPointF(10, 10) << -100. << -200. << 0. << 180.0 << 80.;

    QTest::newRow("Test case 7, +r, +r") << QPointF(10, 10) << 100. << 200. << 0. << 270.0 << 80.;
    QTest::newRow("Test case 7, -r, +r") << QPointF(10, 10) << -100. << 200. << 0. << 270.0 << 80.;
    QTest::newRow("Test case 7, +r, -r") << QPointF(10, 10) << 100. << -200. << 0. << 270.0 << 80.;
    QTest::newRow("Test case 7, -r, -r") << QPointF(10, 10) << -100. << -200. << 0. << 270.0 << 80.;

    QTest::newRow("Test case 8, +r, +r") << QPointF(10, 10) << 100. << 200. << 0. << 360.0 << 80.;
    QTest::newRow("Test case 8, -r, +r") << QPointF(10, 10) << -100. << 200. << 0. << 360.0 << 80.;
    QTest::newRow("Test case 8, +r, -r") << QPointF(10, 10) << 100. << -200. << 0. << 360.0 << 80.;
    QTest::newRow("Test case 8, -r, -r") << QPointF(10, 10) << -100. << -200. << 0. << 360.0 << 80.;

    QTest::newRow("Test case 9, +r, +r") << QPointF() << 100. << 200. << 0. << 90.0 << 80.;
    QTest::newRow("Test case 9, -r, +r") << QPointF() << -100. << 200. << 0. << 90.0 << 80.;
    QTest::newRow("Test case 9, +r, -r") << QPointF() << 100. << -200. << 0. << 90.0 << 80.;
    QTest::newRow("Test case 9, -r, -r") << QPointF() << -100. << -200. << 0. << 90.0 << 80.;

    QTest::newRow("Test case 10, +r, +r") << QPointF() << 100. << 200. << 0. << 180.0 << 80.;
    QTest::newRow("Test case 10, -r, +r") << QPointF() << -100. << 200. << 0. << 180.0 << 80.;
    QTest::newRow("Test case 10, +r, -r") << QPointF() << 100. << -200. << 0. << 180.0 << 80.;
    QTest::newRow("Test case 10, -r, -r") << QPointF() << -100. << -200. << 0. << 180.0 << 80.;

    QTest::newRow("Test case 11, +r, +r") << QPointF() << 100. << 200. << 0. << 270.0 << 80.;
    QTest::newRow("Test case 11, -r, +r") << QPointF() << -100. << 200. << 0. << 270.0 << 80.;
    QTest::newRow("Test case 11, +r, -r") << QPointF() << 100. << -200. << 0. << 270.0 << 80.;
    QTest::newRow("Test case 11, -r, -r") << QPointF() << -100. << -200. << 0. << 270.0 << 80.;

    QTest::newRow("Test case 12, +r, +r") << QPointF() << 100. << 200. << 0. << 360.0 << 80.;
    QTest::newRow("Test case 12, -r, +r") << QPointF() << -100. << 200. << 0. << 360.0 << 80.;
    QTest::newRow("Test case 12, +r, -r") << QPointF() << 100. << -200. << 0. << 360.0 << 80.;
    QTest::newRow("Test case 12, -r, -r") << QPointF() << -100. << -200. << 0. << 360.0 << 80.;

    QTest::newRow("Test case 13, +r, +r") << QPointF(10, 10) << 100. << 200. << 0. << 90.0 << 80.;
    QTest::newRow("Test case 13, -r, +r") << QPointF(10, 10) << -100. << 200. << 0. << 90.0 << 80.;
    QTest::newRow("Test case 13, +r, -r") << QPointF(10, 10) << 100. << -200. << 0. << 90.0 << 80.;
    QTest::newRow("Test case 13, -r, -r") << QPointF(10, 10) << -100. << -200. << 0. << 90.0 << 80.;

    QTest::newRow("Test case 14, +r, +r") << QPointF(10, 10) << 100. << 200. << 0. << 180.0 << 80.;
    QTest::newRow("Test case 14, -r, +r") << QPointF(10, 10) << -100. << 200. << 0. << 180.0 << 80.;
    QTest::newRow("Test case 14, +r, -r") << QPointF(10, 10) << 100. << -200. << 0. << 180.0 << 80.;
    QTest::newRow("Test case 14, -r, -r") << QPointF(10, 10) << -100. << -200. << 0. << 180.0 << 80.;

    QTest::newRow("Test case 15, +r, +r") << QPointF(10, 10) << 100. << 200. << 0. << 270.0 << 80.;
    QTest::newRow("Test case 15, -r, +r") << QPointF(10, 10) << -100. << 200. << 0. << 270.0 << 80.;
    QTest::newRow("Test case 15, +r, -r") << QPointF(10, 10) << 100. << -200. << 0. << 270.0 << 80.;
    QTest::newRow("Test case 15, -r, -r") << QPointF(10, 10) << -100. << -200. << 0. << 270.0 << 80.;

    QTest::newRow("Test case 16, +r, +r") << QPointF(10, 10) << 100. << 200. << 0. << 360.0 << 80.;
    QTest::newRow("Test case 16, -r, +r") << QPointF(10, 10) << -100. << 200. << 0. << 360.0 << 80.;
    QTest::newRow("Test case 16, +r, -r") << QPointF(10, 10) << 100. << -200. << 0. << 360.0 << 80.;
    QTest::newRow("Test case 16, -r, -r") << QPointF(10, 10) << -100. << -200. << 0. << 360.0 << 80.;
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
    arc1.SetApproximationScale(maxCurveApproximationScale);
    const qreal length1 = arc1.GetLength();

    VEllipticalArc arc2(length1, center, radius1, radius2, f1, rotationAngle);
    arc2.SetApproximationScale(maxCurveApproximationScale);
    const qreal length2 = arc2.GetLength();

    Q_RELAXED_CONSTEXPR qreal lengthEps = ToPixel(0.45, Unit::Mm); // computing error

    // cppcheck-suppress unreadVariable
    QString const errorLengthMsg =
        u"Difference between real and computing lengthes bigger than eps = %1. l1 = %2; l2 = %3"_s;
    QVERIFY2(qAbs(length2 - length1) <= lengthEps,
             qUtf8Printable(errorLengthMsg.arg(lengthEps).arg(length2).arg(length1)));
    QVERIFY2(qAbs(length1 - length2) <= lengthEps,
             qUtf8Printable(errorLengthMsg.arg(lengthEps).arg(length2).arg(length1)));

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
void TST_VEllipticalArc::ArcByLength_data()
{
    QTest::addColumn<qreal>("radius1");
    QTest::addColumn<qreal>("radius2");
    QTest::addColumn<qreal>("f1");
    QTest::addColumn<qreal>("f2");
    QTest::addColumn<qreal>("rotationAngle");
    QTest::addColumn<bool>("reversed");
    QTest::addColumn<int>("direction");

    QTest::newRow("+r, +r, +length") << 100. << 200. << 1. << 181. << 0. << false << 1;
    QTest::newRow("+r, +r, -length") << 100. << 200. << 1. << 181. << 0. << true << -1;
    QTest::newRow("-r, +r, +length") << -100. << 200. << 1. << 181. << 0. << true << -1;
    QTest::newRow("-r, +r, -length") << -100. << 200. << 1. << 181. << 0. << true << -1;
    QTest::newRow("+r, -r, +length") << 100. << -200. << 1. << 181. << 0. << true << -1;
    QTest::newRow("+r, -r, -length") << 100. << -200. << 1. << 181. << 0. << true << -1;
    QTest::newRow("-r, -r, +length") << -100. << -200. << 1. << 181. << 0. << true << -1;
    QTest::newRow("-r, -r, -length") << -100. << -200. << 1. << 181. << 0. << true << -1;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::ArcByLength()
{
    QFETCH(qreal, radius1);
    QFETCH(qreal, radius2);
    QFETCH(qreal, f1);
    QFETCH(qreal, f2);
    QFETCH(qreal, rotationAngle);
    QFETCH(bool, reversed);
    QFETCH(int, direction);

    const VPointF center;

    // Full ellipse
    const qreal h = ((qAbs(radius1) - qAbs(radius2)) * (qAbs(radius1) - qAbs(radius2))) /
                    ((qAbs(radius1) + qAbs(radius2)) * (qAbs(radius1) + qAbs(radius2)));
    const qreal length =
        (M_PI * (qAbs(radius1) + qAbs(radius2)) * (1 + 3 * h / (10 + qSqrt(4 - 3 * h))) / 2) * direction;
    VEllipticalArc const arc(length, center, radius1, radius2, f1, rotationAngle);

    Q_RELAXED_CONSTEXPR qreal eps = ToPixel(0.45, Unit::Mm); // computing error
    // cppcheck-suppress unreadVariable
    const auto errorMsg =
        QStringLiteral("Difference between real and computing lengthes bigger than eps = %1.  v1 = %2; v2 = %3");

    QVERIFY2(qAbs(arc.GetLength() - length) <= eps, qUtf8Printable(errorMsg.arg(eps).arg(arc.GetLength()).arg(length)));

    const qreal angleEps = 0.4;
    QVERIFY2(arc.GetEndAngle() - f2 <= angleEps, qUtf8Printable(errorMsg.arg(eps).arg(arc.GetEndAngle()).arg(f2)));

    QCOMPARE(arc.IsFlipped(), false);
    QCOMPARE(arc.IsReversed(), reversed);
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
    QTest::newRow("Full circle: radiuses -10, 20; start 0") << -10.0 << 20.0 << 0.0 << 360.0 << 0.0;
    QTest::newRow("Full circle: radiuses 10, -20; start 0") << 10.0 << -20.0 << 0.0 << 360.0 << 0.0;
    QTest::newRow("Full circle: radiuses -10, -20; start 0") << -10.0 << -20.0 << 0.0 << 360.0 << 0.0;

    QTest::newRow("Full circle: radiuses 150, 200; start 0") << 150.0 << 200.0 << 0.0 << 360.0 << 0.0;
    QTest::newRow("Full circle: radiuses -150, 200; start 0") << -150.0 << 200.0 << 0.0 << 360.0 << 0.0;
    QTest::newRow("Full circle: radiuses 150, -200; start 0") << 150.0 << -200.0 << 0.0 << 360.0 << 0.0;
    QTest::newRow("Full circle: radiuses -150, -200; start 0") << -150.0 << -200.0 << 0.0 << 360.0 << 0.0;

    QTest::newRow("Full circle: radiuses 150, 200, rotation 30; start 0") << 150.0 << 200.0 << 0.0 << 360.0 << 30.0;
    QTest::newRow("Full circle: radiuses -150, 200, rotation 30; start 0") << -150.0 << 200.0 << 0.0 << 360.0 << 30.0;
    QTest::newRow("Full circle: radiuses 150, -200, rotation 30; start 0") << 150.0 << -200.0 << 0.0 << 360.0 << 30.0;
    QTest::newRow("Full circle: radiuses -150, -200, rotation 30; start 0") << -150.0 << -200.0 << 0.0 << 360.0 << 30.0;

    QTest::newRow("Full circle: radiuses 1500, 1000; start 0") << 1500.0 << 1000.0 << 0.0 << 360.0 << 0.0;
    QTest::newRow("Full circle: radiuses -1500, 1000; start 0") << -1500.0 << 1000.0 << 0.0 << 360.0 << 0.0;
    QTest::newRow("Full circle: radiuses 1500, -1000; start 0") << 1500.0 << -1000.0 << 0.0 << 360.0 << 0.0;
    QTest::newRow("Full circle: radiuses -1500, -1000; start 0") << -1500.0 << -1000.0 << 0.0 << 360.0 << 0.0;

    QTest::newRow("Full circle: radiuses 1500, 1000, rotation 50; start 0") << 1500.0 << 1000.0 << 0.0 << 360.0 << 50.0;
    QTest::newRow("Full circle: radiuses -1500, 1000, rotation 50; start 0")
        << -1500.0 << 1000.0 << 0.0 << 360.0 << 50.0;
    QTest::newRow("Full circle: radiuses 1500, -1000, rotation 50; start 0")
        << 1500.0 << -1000.0 << 0.0 << 360.0 << 50.0;
    QTest::newRow("Full circle: radiuses -1500, -1000, rotation 50; start 0")
        << -1500.0 << -1000.0 << 0.0 << 360.0 << 50.0;

    QTest::newRow("Full circle: radiuses 15000, 10000, rotation 90; start 0")
        << 15000.0 << 10000.0 << 0.0 << 360.0 << 90.0;
    QTest::newRow("Full circle: radiuses -15000, 10000, rotation 90; start 0")
        << -15000.0 << 10000.0 << 0.0 << 360.0 << 90.0;
    QTest::newRow("Full circle: radiuses 15000, -10000, rotation 90; start 0")
        << 15000.0 << -10000.0 << 0.0 << 360.0 << 90.0;
    QTest::newRow("Full circle: radiuses -15000, -10000, rotation 90; start 0")
        << -15000.0 << -10000.0 << 0.0 << 360.0 << 90.0;

    QTest::newRow("Full circle: radiuses 10, 20; start 90") << 10.0 << 20.0 << 90.0 << 90.0 << 0.0;
    QTest::newRow("Full circle: radiuses -10, 20; start 90") << -10.0 << 20.0 << 90.0 << 90.0 << 0.0;
    QTest::newRow("Full circle: radiuses 10, -20; start 90") << 10.0 << -20.0 << 90.0 << 90.0 << 0.0;
    QTest::newRow("Full circle: radiuses -10, -20; start 90") << -10.0 << -20.0 << 90.0 << 90.0 << 0.0;

    QTest::newRow("Full circle: radiuses 150, 200; start 90") << 150.0 << 200.0 << 90.0 << 90.0 << 0.0;
    QTest::newRow("Full circle: radiuses -150, 200; start 90") << -150.0 << 200.0 << 90.0 << 90.0 << 0.0;
    QTest::newRow("Full circle: radiuses 150, -200; start 90") << 150.0 << -200.0 << 90.0 << 90.0 << 0.0;
    QTest::newRow("Full circle: radiuses -150, -200; start 90") << -150.0 << -200.0 << 90.0 << 90.0 << 0.0;

    QTest::newRow("Full circle: radiuses 150, 200, rotation 30; start 90") << 150.0 << 200.0 << 90.0 << 90.0 << 30.0;
    QTest::newRow("Full circle: radiuses -150, 200, rotation 30; start 90") << -150.0 << 200.0 << 90.0 << 90.0 << 30.0;
    QTest::newRow("Full circle: radiuses 150, -200, rotation 30; start 90") << 150.0 << -200.0 << 90.0 << 90.0 << 30.0;
    QTest::newRow("Full circle: radiuses -150, -200, rotation 30; start 90")
        << -150.0 << -200.0 << 90.0 << 90.0 << 30.0;

    QTest::newRow("Full circle: radiuses 1500, 1000; start 90") << 1500.0 << 1000.0 << 90.0 << 90.0 << 0.0;
    QTest::newRow("Full circle: radiuses -1500, 1000; start 90") << 1500.0 << -1000.0 << 90.0 << 90.0 << 0.0;
    QTest::newRow("Full circle: radiuses 1500, -1000; start 90") << 1500.0 << -1000.0 << 90.0 << 90.0 << 0.0;
    QTest::newRow("Full circle: radiuses -1500, -1000; start 90") << -1500.0 << -1000.0 << 90.0 << 90.0 << 0.0;

    QTest::newRow("Full circle: radiuses 1500, 1000, rotation 50; start 90")
        << 1500.0 << 1000.0 << 90.0 << 90.0 << 50.0;
    QTest::newRow("Full circle: radiuses 1500, 1000, rotation 50; start 90")
        << 1500.0 << 1000.0 << 90.0 << 90.0 << 50.0;
    QTest::newRow("Full circle: radiuses 1500, 1000, rotation 50; start 90")
        << 1500.0 << 1000.0 << 90.0 << 90.0 << 50.0;
    QTest::newRow("Full circle: radiuses 1500, 1000, rotation 50; start 90")
        << 1500.0 << 1000.0 << 90.0 << 90.0 << 50.0;

    QTest::newRow("Full circle: radiuses 15000, 10000, rotation 90; start 90")
        << 15000.0 << 10000.0 << 90.0 << 90.0 << 90.0;
    QTest::newRow("Full circle: radiuses -15000, 10000, rotation 90; start 90")
        << -15000.0 << 10000.0 << 90.0 << 90.0 << 90.0;
    QTest::newRow("Full circle: radiuses 15000, -10000, rotation 90; start 90")
        << 15000.0 << -10000.0 << 90.0 << 90.0 << 90.0;
    QTest::newRow("Full circle: radiuses -15000, -10000, rotation 90; start 90")
        << -15000.0 << -10000.0 << 90.0 << 90.0 << 90.0;

    QTest::newRow("Arc less than 45 degree, radiuses 100, 50") << 100.0 << 50.0 << 0.0 << 10.5 << 0.0;
    QTest::newRow("Arc less than 45 degree, radiuses -100, 50") << -100.0 << 50.0 << 0.0 << 10.5 << 0.0;
    QTest::newRow("Arc less than 45 degree, radiuses 100, -50") << 100.0 << -50.0 << 0.0 << 10.5 << 0.0;
    QTest::newRow("Arc less than 45 degree, radiuses -100, -50") << -100.0 << -50.0 << 0.0 << 10.5 << 0.0;

    QTest::newRow("Arc less than 45 degree, radiuses 150, 50, rotation 180") << 150.0 << 50.0 << 0.0 << 10.5 << 180.0;
    QTest::newRow("Arc less than 45 degree, radiuses -150, 50, rotation 180") << -150.0 << 50.0 << 0.0 << 10.5 << 180.0;
    QTest::newRow("Arc less than 45 degree, radiuses 150, -50, rotation 180") << 150.0 << -50.0 << 0.0 << 10.5 << 180.0;
    QTest::newRow("Arc less than 45 degree, radiuses -150, -50, rotation 180")
        << -150.0 << -50.0 << 0.0 << 10.5 << 180.0;

    QTest::newRow("Arc less than 45 degree, radiuses 1500, 800, rotation 90") << 1500.0 << 800.0 << 0.0 << 10.5 << 90.0;
    QTest::newRow("Arc less than 45 degree, radiuses -1500, 800, rotation 90")
        << -1500.0 << 800.0 << 0.0 << 10.5 << 90.0;
    QTest::newRow("Arc less than 45 degree, radiuses 1500, -800, rotation 90")
        << 1500.0 << -800.0 << 0.0 << 10.5 << 90.0;
    QTest::newRow("Arc less than 45 degree, radiuses -1500, -800, rotation 90")
        << -1500.0 << -800.0 << 0.0 << 10.5 << 90.0;

    QTest::newRow("Arc less than 45 degree, radiuses 50000, 10000, rotation 40")
        << 50000.0 << 10000.0 << 0.0 << 10.5 << 40.0;
    QTest::newRow("Arc less than 45 degree, radiuses -50000, 10000, rotation 40")
        << 50000.0 << 10000.0 << 0.0 << 10.5 << 40.0;
    QTest::newRow("Arc less than 45 degree, radiuses 50000, -10000, rotation 40")
        << 50000.0 << 10000.0 << 0.0 << 10.5 << 40.0;
    QTest::newRow("Arc less than 45 degree, radiuses -50000, -10000, rotation 40")
        << -50000.0 << -10000.0 << 0.0 << 10.5 << 40.0;

    QTest::newRow("Arc less than 45 degree, radiuses 15000, 10000") << 15000.0 << 10000.0 << 0.0 << 10.5 << 0.0;
    QTest::newRow("Arc less than 45 degree, radiuses -15000, 10000") << -15000.0 << 10000.0 << 0.0 << 10.5 << 0.0;
    QTest::newRow("Arc less than 45 degree, radiuses 15000, -10000") << 15000.0 << -10000.0 << 0.0 << 10.5 << 0.0;
    QTest::newRow("Arc less than 45 degree, radiuses -15000, -10000") << -15000.0 << -10000.0 << 0.0 << 10.5 << 0.0;

    QTest::newRow("Arc 45 degree, radiuses 100, 50, rotation 45") << 100.0 << 50.0 << 0.0 << 45.0 << 45.0;
    QTest::newRow("Arc 45 degree, radiuses -100, 50, rotation 45") << -100.0 << 50.0 << 0.0 << 45.0 << 45.0;
    QTest::newRow("Arc 45 degree, radiuses 100, -50, rotation 45") << 100.0 << -50.0 << 0.0 << 45.0 << 45.0;
    QTest::newRow("Arc 45 degree, radiuses -100, -50, rotation 45") << -100.0 << -50.0 << 0.0 << 45.0 << 45.0;

    QTest::newRow("Arc 45 degree, radiuses 150, 15, rotation 30") << 150.0 << 15.0 << 0.0 << 45.0 << 30.0;
    QTest::newRow("Arc 45 degree, radiuses -150, 15, rotation 30") << -150.0 << 15.0 << 0.0 << 45.0 << 30.0;
    QTest::newRow("Arc 45 degree, radiuses 150, -15, rotation 30") << 150.0 << -15.0 << 0.0 << 45.0 << 30.0;
    QTest::newRow("Arc 45 degree, radiuses -150, -15, rotation 30") << -150.0 << -15.0 << 0.0 << 45.0 << 30.0;

    QTest::newRow("Arc 45 degree, radiuses 1500, 150, rotation 45") << 1500.0 << 150.0 << 0.0 << 45.0 << 45.0;
    QTest::newRow("Arc 45 degree, radiuses -1500, 150, rotation 45") << -1500.0 << 150.0 << 0.0 << 45.0 << 45.0;
    QTest::newRow("Arc 45 degree, radiuses 1500, -150, rotation 45") << 1500.0 << -150.0 << 0.0 << 45.0 << 45.0;
    QTest::newRow("Arc 45 degree, radiuses -1500, -150, rotation 45") << -1500.0 << -150.0 << 0.0 << 45.0 << 45.0;

    QTest::newRow("Arc 45 degree, radiuses 15000, 15000") << 15000.0 << 15000.0 << 0.0 << 45.0 << 0.0;
    QTest::newRow("Arc 45 degree, radiuses -15000, 15000") << -15000.0 << 15000.0 << 0.0 << 45.0 << 0.0;
    QTest::newRow("Arc 45 degree, radiuses 15000, -15000") << 15000.0 << -15000.0 << 0.0 << 45.0 << 0.0;
    QTest::newRow("Arc 45 degree, radiuses -15000, -15000") << -15000.0 << -15000.0 << 0.0 << 45.0 << 0.0;

    QTest::newRow("Arc 45 degree, radiuses 15000, 10000, rotation 270") << 15000.0 << 10000.0 << 0.0 << 45.0 << 270.0;
    QTest::newRow("Arc 45 degree, radiuses -15000, 10000, rotation 270") << -15000.0 << 10000.0 << 0.0 << 45.0 << 270.0;
    QTest::newRow("Arc 45 degree, radiuses 15000, -10000, rotation 270") << 15000.0 << -10000.0 << 0.0 << 45.0 << 270.0;
    QTest::newRow("Arc 45 degree, radiuses -15000, -10000, rotation 270")
        << -15000.0 << -10000.0 << 0.0 << 45.0 << 270.0;

    QTest::newRow("Arc less than 90 degree, radiuses 100, 400, rotation 50") << 100.0 << 400.0 << 0.0 << 75.0 << 50.0;
    QTest::newRow("Arc less than 90 degree, radiuses -100, 400, rotation 50") << -100.0 << 400.0 << 0.0 << 75.0 << 50.0;
    QTest::newRow("Arc less than 90 degree, radiuses 100, -400, rotation 50") << 100.0 << -400.0 << 0.0 << 75.0 << 50.0;
    QTest::newRow("Arc less than 90 degree, radiuses -100, -400, rotation 50")
        << -100.0 << -400.0 << 0.0 << 75.0 << 50.0;

    QTest::newRow("Arc less than 90 degree, radiuses 150, 400, rotation 90") << 150.0 << 400.0 << 0.0 << 75.0 << 90.0;
    QTest::newRow("Arc less than 90 degree, radiuses -150, 400, rotation 90") << -150.0 << 400.0 << 0.0 << 75.0 << 90.0;
    QTest::newRow("Arc less than 90 degree, radiuses 150, -400, rotation 90") << 150.0 << -400.0 << 0.0 << 75.0 << 90.0;
    QTest::newRow("Arc less than 90 degree, radiuses -150, -400, rotation 90")
        << -150.0 << -400.0 << 0.0 << 75.0 << 90.0;

    QTest::newRow("Arc less than 90 degree, radiuses 1500, 50000, rotation 180")
        << 1500.0 << 50000.0 << 0.0 << 75.0 << 180.0;
    QTest::newRow("Arc less than 90 degree, radiuses -1500, 50000, rotation 180")
        << -1500.0 << 50000.0 << 0.0 << 75.0 << 180.0;
    QTest::newRow("Arc less than 90 degree, radiuses 1500, -50000, rotation 180")
        << 1500.0 << -50000.0 << 0.0 << 75.0 << 180.0;
    QTest::newRow("Arc less than 90 degree, radiuses -1500, -50000, rotation 180")
        << -1500.0 << -50000.0 << 0.0 << 75.0 << 180.0;

    QTest::newRow("Arc less than 90 degree, radiuses 50000, 5000, rotation 30")
        << 50000.0 << 5000.0 << 0.0 << 75.0 << 30.0;
    QTest::newRow("Arc less than 90 degree, radiuses -50000, 5000, rotation 30")
        << -50000.0 << 5000.0 << 0.0 << 75.0 << 30.0;
    QTest::newRow("Arc less than 90 degree, radiuses 50000, -5000, rotation 30")
        << 50000.0 << -5000.0 << 0.0 << 75.0 << 30.0;
    QTest::newRow("Arc less than 90 degree, radiuses -50000, -5000, rotation 30")
        << -50000.0 << -5000.0 << 0.0 << 75.0 << 30.0;

    QTest::newRow("Arc less than 90 degree, radiuses 90000, 50000, rotation 30")
        << 90000.0 << 50000.0 << 0.0 << 75.0 << 30.0;
    QTest::newRow("Arc less than 90 degree, radiuses -90000, 50000, rotation 30")
        << -90000.0 << 50000.0 << 0.0 << 75.0 << 30.0;
    QTest::newRow("Arc less than 90 degree, radiuses 90000, -50000, rotation 30")
        << 90000.0 << -50000.0 << 0.0 << 75.0 << 30.0;
    QTest::newRow("Arc less than 90 degree, radiuses -90000, -50000, rotation 30")
        << -90000.0 << -50000.0 << 0.0 << 75.0 << 30.0;

    QTest::newRow("Arc 90 degree, radiuses 100, 50, rotation 30") << 100.0 << 50.0 << 0.0 << 90.0 << 30.0;
    QTest::newRow("Arc 90 degree, radiuses -100, 50, rotation 30") << -100.0 << 50.0 << 0.0 << 90.0 << 30.0;
    QTest::newRow("Arc 90 degree, radiuses 100, -50, rotation 30") << 100.0 << -50.0 << 0.0 << 90.0 << 30.0;
    QTest::newRow("Arc 90 degree, radiuses -100, -50, rotation 30") << -100.0 << -50.0 << 0.0 << 90.0 << 30.0;

    QTest::newRow("Arc 90 degree, radiuses 150, 400") << 150.0 << 400.0 << 0.0 << 90.0 << 0.0;
    QTest::newRow("Arc 90 degree, radiuses -150, 400") << -150.0 << 400.0 << 0.0 << 90.0 << 0.0;
    QTest::newRow("Arc 90 degree, radiuses 150, -400") << 150.0 << -400.0 << 0.0 << 90.0 << 0.0;
    QTest::newRow("Arc 90 degree, radiuses -150, -400") << -150.0 << -400.0 << 0.0 << 90.0 << 0.0;

    QTest::newRow("Arc 90 degree, radiuses 1500, 800, rotation 70") << 1500.0 << 800.0 << 0.0 << 90.0 << 70.0;
    QTest::newRow("Arc 90 degree, radiuses -1500, 800, rotation 70") << -1500.0 << 800.0 << 0.0 << 90.0 << 70.0;
    QTest::newRow("Arc 90 degree, radiuses 1500, -800, rotation 70") << 1500.0 << -800.0 << 0.0 << 90.0 << 70.0;
    QTest::newRow("Arc 90 degree, radiuses -1500, -800, rotation 70") << -1500.0 << -800.0 << 0.0 << 90.0 << 70.0;

    QTest::newRow("Arc 90 degree, radiuses 15000, 1500, rotation 30") << 15000.0 << 1500.0 << 0.0 << 90.0 << 30.0;
    QTest::newRow("Arc 90 degree, radiuses -15000, 1500, rotation 30") << -15000.0 << 1500.0 << 0.0 << 90.0 << 30.0;
    QTest::newRow("Arc 90 degree, radiuses 15000, -1500, rotation 30") << 15000.0 << -1500.0 << 0.0 << 90.0 << 30.0;
    QTest::newRow("Arc 90 degree, radiuses -15000, -1500, rotation 30") << -15000.0 << -1500.0 << 0.0 << 90.0 << 30.0;

    QTest::newRow("Arc 90 degree, radiuses 15000, 14000, rotation 235") << 15000.0 << 14000.0 << 0.0 << 90.0 << 235.0;
    QTest::newRow("Arc 90 degree, radiuses -15000, 14000, rotation 235") << -15000.0 << 14000.0 << 0.0 << 90.0 << 235.0;
    QTest::newRow("Arc 90 degree, radiuses 15000, -14000, rotation 235") << 15000.0 << -14000.0 << 0.0 << 90.0 << 235.0;
    QTest::newRow("Arc 90 degree, radiuses -15000, -14000, rotation 235")
        << -15000.0 << -14000.0 << 0.0 << 90.0 << 235.0;

    QTest::newRow("Arc less than 135 degree, radiuses 100, 50, rotation 60") << 100.0 << 50.0 << 0.0 << 110.6 << 60.0;
    QTest::newRow("Arc less than 135 degree, radiuses -100, 50, rotation 60") << -100.0 << 50.0 << 0.0 << 110.6 << 60.0;
    QTest::newRow("Arc less than 135 degree, radiuses 100, -50, rotation 60") << 100.0 << -50.0 << 0.0 << 110.6 << 60.0;
    QTest::newRow("Arc less than 135 degree, radiuses -100, -50, rotation 60")
        << -100.0 << -50.0 << 0.0 << 110.6 << 60.0;

    QTest::newRow("Arc less than 135 degree, radiuses 150, 400, rotation 300")
        << 150.0 << 400.0 << 0.0 << 110.6 << 300.0;
    QTest::newRow("Arc less than 135 degree, radiuses -150, 400, rotation 300")
        << -150.0 << 400.0 << 0.0 << 110.6 << 300.0;
    QTest::newRow("Arc less than 135 degree, radiuses 150, -400, rotation 300")
        << 150.0 << -400.0 << 0.0 << 110.6 << 300.0;
    QTest::newRow("Arc less than 135 degree, radiuses -150, -400, rotation 300")
        << -150.0 << -400.0 << 0.0 << 110.6 << 300.0;

    QTest::newRow("Arc less than 135 degree, radiuses 1500, 800, rotation 360")
        << 1500.0 << 800.0 << 0.0 << 110.6 << 360.0;
    QTest::newRow("Arc less than 135 degree, radiuses -1500, 800, rotation 360")
        << -1500.0 << 800.0 << 0.0 << 110.6 << 360.0;
    QTest::newRow("Arc less than 135 degree, radiuses 1500, -800, rotation 360")
        << 1500.0 << -800.0 << 0.0 << 110.6 << 360.0;
    QTest::newRow("Arc less than 135 degree, radiuses -1500, -800, rotation 360")
        << -1500.0 << -800.0 << 0.0 << 110.6 << 360.0;

    QTest::newRow("Arc less than 135 degree, radiuses 15000, 1500, rotation 290")
        << 15000.0 << 1500.0 << 0.0 << 110.6 << 290.0;
    QTest::newRow("Arc less than 135 degree, radiuses -15000, 1500, rotation 290")
        << -15000.0 << 1500.0 << 0.0 << 110.6 << 290.0;
    QTest::newRow("Arc less than 135 degree, radiuses 15000, -1500, rotation 290")
        << 15000.0 << -1500.0 << 0.0 << 110.6 << 290.0;
    QTest::newRow("Arc less than 135 degree, radiuses -15000, -1500, rotation 290")
        << -15000.0 << -1500.0 << 0.0 << 110.6 << 290.0;

    QTest::newRow("Arc less than 135 degree, radiuses 15000, 1500") << 15000.0 << 1500.0 << 0.0 << 110.6 << 0.0;
    QTest::newRow("Arc less than 135 degree, radiuses -15000, 1500") << -15000.0 << 1500.0 << 0.0 << 110.6 << 0.0;
    QTest::newRow("Arc less than 135 degree, radiuses 15000, -1500") << 15000.0 << -1500.0 << 0.0 << 110.6 << 0.0;
    QTest::newRow("Arc less than 135 degree, radiuses -15000, -1500") << -15000.0 << -1500.0 << 0.0 << 110.6 << 0.0;

    QTest::newRow("Arc 135 degree, radiuses 100, 50") << 100.0 << 50.0 << 0.0 << 135.0 << 0.0;
    QTest::newRow("Arc 135 degree, radiuses -100, 50") << -100.0 << 50.0 << 0.0 << 135.0 << 0.0;
    QTest::newRow("Arc 135 degree, radiuses 100, -50") << 100.0 << -50.0 << 0.0 << 135.0 << 0.0;
    QTest::newRow("Arc 135 degree, radiuses -100, -50") << -100.0 << -50.0 << 0.0 << 135.0 << 0.0;

    QTest::newRow("Arc 135 degree, radiuses 150, 400") << 150.0 << 400.0 << 0.0 << 135.0 << 0.0;
    QTest::newRow("Arc 135 degree, radiuses -150, 400") << -150.0 << 400.0 << 0.0 << 135.0 << 0.0;
    QTest::newRow("Arc 135 degree, radiuses 150, -400") << 150.0 << -400.0 << 0.0 << 135.0 << 0.0;
    QTest::newRow("Arc 135 degree, radiuses -150, -400") << -150.0 << -400.0 << 0.0 << 135.0 << 0.0;

    QTest::newRow("Arc 135 degree, radiuses 1500, 800") << 1500.0 << 800.0 << 0.0 << 135.0 << 0.0;
    QTest::newRow("Arc 135 degree, radiuses -1500, 800") << -1500.0 << 800.0 << 0.0 << 135.0 << 0.0;
    QTest::newRow("Arc 135 degree, radiuses 1500, -800") << 1500.0 << -800.0 << 0.0 << 135.0 << 0.0;
    QTest::newRow("Arc 135 degree, radiuses -1500, -800") << -1500.0 << -800.0 << 0.0 << 135.0 << 0.0;

    QTest::newRow("Arc 135 degree, radiuses 15000, 1500, rotation 20") << 15000.0 << 1500.0 << 0.0 << 135.0 << 20.0;
    QTest::newRow("Arc 135 degree, radiuses -15000, 1500, rotation 20") << -15000.0 << 1500.0 << 0.0 << 135.0 << 20.0;
    QTest::newRow("Arc 135 degree, radiuses 15000, -1500, rotation 20") << 15000.0 << -1500.0 << 0.0 << 135.0 << 20.0;
    QTest::newRow("Arc 135 degree, radiuses -15000, -1500, rotation 20") << -15000.0 << -1500.0 << 0.0 << 135.0 << 20.0;

    QTest::newRow("Arc 135 degree, radiuses 15000, 10000") << 15000.0 << 10000.0 << 0.0 << 135.0 << 0.0;
    QTest::newRow("Arc 135 degree, radiuses -15000, 10000") << -15000.0 << 10000.0 << 0.0 << 135.0 << 0.0;
    QTest::newRow("Arc 135 degree, radiuses 15000, -10000") << 15000.0 << -10000.0 << 0.0 << 135.0 << 0.0;
    QTest::newRow("Arc 135 degree, radiuses -15000, -10000") << -15000.0 << -10000.0 << 0.0 << 135.0 << 0.0;

    QTest::newRow("Arc less than 180 degree, radiuses 100, 50") << 100.0 << 50.0 << 0.0 << 160.7 << 0.0;
    QTest::newRow("Arc less than 180 degree, radiuses -100, 50") << -100.0 << 50.0 << 0.0 << 160.7 << 0.0;
    QTest::newRow("Arc less than 180 degree, radiuses 100, -50") << 100.0 << -50.0 << 0.0 << 160.7 << 0.0;
    QTest::newRow("Arc less than 180 degree, radiuses -100, -50") << -100.0 << -50.0 << 0.0 << 160.7 << 0.0;

    QTest::newRow("Arc less than 180 degree, radiuses 150, 400") << 150.0 << 400.0 << 0.0 << 160.7 << 0.0;
    QTest::newRow("Arc less than 180 degree, radiuses -150, 400") << -150.0 << 400.0 << 0.0 << 160.7 << 0.0;
    QTest::newRow("Arc less than 180 degree, radiuses 150, -400") << 150.0 << -400.0 << 0.0 << 160.7 << 0.0;
    QTest::newRow("Arc less than 180 degree, radiuses -150, -400") << -150.0 << -400.0 << 0.0 << 160.7 << 0.0;

    QTest::newRow("Arc less than 180 degree, radiuses 1500, 800") << 1500.0 << 800.0 << 0.0 << 160.7 << 0.0;
    QTest::newRow("Arc less than 180 degree, radiuses -1500, 800") << -1500.0 << 800.0 << 0.0 << 160.7 << 0.0;
    QTest::newRow("Arc less than 180 degree, radiuses 1500, -800") << 1500.0 << -800.0 << 0.0 << 160.7 << 0.0;
    QTest::newRow("Arc less than 180 degree, radiuses -1500, -800") << -1500.0 << -800.0 << 0.0 << 160.7 << 0.0;

    QTest::newRow("Arc less than 180 degree, radiuses 15000, 1500, rotation 270")
        << 15000.0 << 1500.0 << 0.0 << 160.7 << 270.0;
    QTest::newRow("Arc less than 180 degree, radiuses -15000, 1500, rotation 270")
        << -15000.0 << 1500.0 << 0.0 << 160.7 << 270.0;
    QTest::newRow("Arc less than 180 degree, radiuses 15000, -1500, rotation 270")
        << 15000.0 << -1500.0 << 0.0 << 160.7 << 270.0;
    QTest::newRow("Arc less than 180 degree, radiuses -15000, -1500, rotation 270")
        << -15000.0 << -1500.0 << 0.0 << 160.7 << 270.0;

    QTest::newRow("Arc less than 180 degree, radiuses 15000, 10000") << 15000.0 << 10000.0 << 0.0 << 160.7 << 0.0;
    QTest::newRow("Arc less than 180 degree, radiuses -15000, 10000") << -15000.0 << 10000.0 << 0.0 << 160.7 << 0.0;
    QTest::newRow("Arc less than 180 degree, radiuses 15000, -10000") << 15000.0 << -10000.0 << 0.0 << 160.7 << 0.0;
    QTest::newRow("Arc less than 180 degree, radiuses -15000, -10000") << -15000.0 << -10000.0 << 0.0 << 160.7 << 0.0;

    QTest::newRow("Arc 180 degree, radiuses 100, 50") << 100.0 << 50.0 << 0.0 << 180.0 << 0.0;
    QTest::newRow("Arc 180 degree, radiuses -100, 50") << -100.0 << 50.0 << 0.0 << 180.0 << 0.0;
    QTest::newRow("Arc 180 degree, radiuses 100, -50") << 100.0 << -50.0 << 0.0 << 180.0 << 0.0;
    QTest::newRow("Arc 180 degree, radiuses -100, -50") << -100.0 << -50.0 << 0.0 << 180.0 << 0.0;

    QTest::newRow("Arc 180 degree, radiuses 150, 400") << 150.0 << 400.0 << 0.0 << 180.0 << 0.0;
    QTest::newRow("Arc 180 degree, radiuses -150, 400") << -150.0 << 400.0 << 0.0 << 180.0 << 0.0;
    QTest::newRow("Arc 180 degree, radiuses 150, -400") << 150.0 << -400.0 << 0.0 << 180.0 << 0.0;
    QTest::newRow("Arc 180 degree, radiuses -150, -400") << -150.0 << -400.0 << 0.0 << 180.0 << 0.0;

    QTest::newRow("Arc 180 degree, radiuses 1500, 800") << 1500.0 << 800.0 << 0.0 << 180.0 << 0.0;
    QTest::newRow("Arc 180 degree, radiuses -1500, 800") << -1500.0 << 800.0 << 0.0 << 180.0 << 0.0;
    QTest::newRow("Arc 180 degree, radiuses 1500, -800") << 1500.0 << -800.0 << 0.0 << 180.0 << 0.0;
    QTest::newRow("Arc 180 degree, radiuses -1500, -800") << -1500.0 << -800.0 << 0.0 << 180.0 << 0.0;

    QTest::newRow("Arc 180 degree, radiuses 15000, 1500, rotation 60") << 15000.0 << 1500.0 << 0.0 << 180.0 << 60.0;
    QTest::newRow("Arc 180 degree, radiuses -15000, 1500, rotation 60") << -15000.0 << 1500.0 << 0.0 << 180.0 << 60.0;
    QTest::newRow("Arc 180 degree, radiuses 15000, -1500, rotation 60") << 15000.0 << -1500.0 << 0.0 << 180.0 << 60.0;
    QTest::newRow("Arc 180 degree, radiuses -15000, -1500, rotation 60") << -15000.0 << -1500.0 << 0.0 << 180.0 << 60.0;

    QTest::newRow("Arc 180 degree, radiuses 15000, 10000") << 15000.0 << 10000.0 << 0.0 << 180.0 << 0.0;
    QTest::newRow("Arc 180 degree, radiuses -15000, 10000") << -15000.0 << 10000.0 << 0.0 << 180.0 << 0.0;
    QTest::newRow("Arc 180 degree, radiuses 15000, -10000") << 15000.0 << -10000.0 << 0.0 << 180.0 << 0.0;
    QTest::newRow("Arc 180 degree, radiuses -15000, -10000") << -15000.0 << -10000.0 << 0.0 << 180.0 << 0.0;

    QTest::newRow("Arc less than 270 degree, radiuses 100, 50") << 100.0 << 50.0 << 0.0 << 150.3 << 0.0;
    QTest::newRow("Arc less than 270 degree, radiuses -100, 50") << -100.0 << 50.0 << 0.0 << 150.3 << 0.0;
    QTest::newRow("Arc less than 270 degree, radiuses 100, -50") << 100.0 << -50.0 << 0.0 << 150.3 << 0.0;
    QTest::newRow("Arc less than 270 degree, radiuses -100, -50") << -100.0 << -50.0 << 0.0 << 150.3 << 0.0;

    QTest::newRow("Arc less than 270 degree, radiuses 150, 400") << 150.0 << 400.0 << 0.0 << 150.3 << 0.0;
    QTest::newRow("Arc less than 270 degree, radiuses -150, 400") << -150.0 << 400.0 << 0.0 << 150.3 << 0.0;
    QTest::newRow("Arc less than 270 degree, radiuses 150, -400") << 150.0 << -400.0 << 0.0 << 150.3 << 0.0;
    QTest::newRow("Arc less than 270 degree, radiuses -150, -400") << -150.0 << -400.0 << 0.0 << 150.3 << 0.0;

    QTest::newRow("Arc less than 270 degree, radiuses 1500, 800") << 1500.0 << 800.0 << 0.0 << 150.3 << 0.0;
    QTest::newRow("Arc less than 270 degree, radiuses -1500, 800") << -1500.0 << 800.0 << 0.0 << 150.3 << 0.0;
    QTest::newRow("Arc less than 270 degree, radiuses 1500, -800") << 1500.0 << -800.0 << 0.0 << 150.3 << 0.0;
    QTest::newRow("Arc less than 270 degree, radiuses -1500, -800") << -1500.0 << -800.0 << 0.0 << 150.3 << 0.0;

    QTest::newRow("Arc less than 270 degree, radiuses 15000, 1500, rotation 20")
        << 15000.0 << 1500.0 << 0.0 << 150.3 << 20.0;
    QTest::newRow("Arc less than 270 degree, radiuses -15000, 1500, rotation 20")
        << -15000.0 << 1500.0 << 0.0 << 150.3 << 20.0;
    QTest::newRow("Arc less than 270 degree, radiuses 15000, -1500, rotation 20")
        << 15000.0 << -1500.0 << 0.0 << 150.3 << 20.0;
    QTest::newRow("Arc less than 270 degree, radiuses -15000, -1500, rotation 20")
        << -15000.0 << -1500.0 << 0.0 << 150.3 << 20.0;

    QTest::newRow("Arc less than 270 degree, radiuses 15000, 10000") << 15000.0 << 10000.0 << 0.0 << 150.3 << 0.0;
    QTest::newRow("Arc less than 270 degree, radiuses -15000, 10000") << -15000.0 << 10000.0 << 0.0 << 150.3 << 0.0;
    QTest::newRow("Arc less than 270 degree, radiuses 15000, -10000") << 15000.0 << -10000.0 << 0.0 << 150.3 << 0.0;
    QTest::newRow("Arc less than 270 degree, radiuses -15000, -10000") << -15000.0 << -10000.0 << 0.0 << 150.3 << 0.0;

    QTest::newRow("Arc 270 degree, radiuses 100, 50") << 100.0 << 50.0 << 0.0 << 270.0 << 0.0;
    QTest::newRow("Arc 270 degree, radiuses -100, 50") << -100.0 << 50.0 << 0.0 << 270.0 << 0.0;
    QTest::newRow("Arc 270 degree, radiuses 100, -50") << 100.0 << -50.0 << 0.0 << 270.0 << 0.0;
    QTest::newRow("Arc 270 degree, radiuses -100, -50") << -100.0 << -50.0 << 0.0 << 270.0 << 0.0;

    QTest::newRow("Arc 270 degree, radiuses 150, 400") << 150.0 << 400.0 << 0.0 << 270.0 << 0.0;
    QTest::newRow("Arc 270 degree, radiuses -150, 400") << -150.0 << 400.0 << 0.0 << 270.0 << 0.0;
    QTest::newRow("Arc 270 degree, radiuses 150, -400") << 150.0 << -400.0 << 0.0 << 270.0 << 0.0;
    QTest::newRow("Arc 270 degree, radiuses -150, -400") << -150.0 << -400.0 << 0.0 << 270.0 << 0.0;

    QTest::newRow("Arc 270 degree, radiuses 1500, 800") << 1500.0 << 800.0 << 0.0 << 270.0 << 0.0;
    QTest::newRow("Arc 270 degree, radiuses -1500, 800") << -1500.0 << 800.0 << 0.0 << 270.0 << 0.0;
    QTest::newRow("Arc 270 degree, radiuses 1500, -800") << 1500.0 << -800.0 << 0.0 << 270.0 << 0.0;
    QTest::newRow("Arc 270 degree, radiuses -1500, -800") << -1500.0 << -800.0 << 0.0 << 270.0 << 0.0;

    QTest::newRow("Arc 270 degree, radiuses 15000, 1500, rotation 90") << 15000.0 << 1500.0 << 0.0 << 270.0 << 90.0;
    QTest::newRow("Arc 270 degree, radiuses -15000, 1500, rotation 90") << -15000.0 << 1500.0 << 0.0 << 270.0 << 90.0;
    QTest::newRow("Arc 270 degree, radiuses 15000, -1500, rotation 90") << 15000.0 << -1500.0 << 0.0 << 270.0 << 90.0;
    QTest::newRow("Arc 270 degree, radiuses -15000, -1500, rotation 90") << -15000.0 << -1500.0 << 0.0 << 270.0 << 90.0;

    QTest::newRow("Arc 270 degree, radiuses 15000, 10000") << 15000.0 << 10000.0 << 0.0 << 270.0 << 0.0;
    QTest::newRow("Arc 270 degree, radiuses -15000, 10000") << -15000.0 << 10000.0 << 0.0 << 270.0 << 0.0;
    QTest::newRow("Arc 270 degree, radiuses 15000, -10000") << 15000.0 << -10000.0 << 0.0 << 270.0 << 0.0;
    QTest::newRow("Arc 270 degree, radiuses -15000, -10000") << -15000.0 << -10000.0 << 0.0 << 270.0 << 0.0;

    QTest::newRow("Arc less than 360 degree, radiuses 100, 50") << 100.0 << 50.0 << 0.0 << 340.0 << 0.0;
    QTest::newRow("Arc less than 360 degree, radiuses -100, 50") << -100.0 << 50.0 << 0.0 << 340.0 << 0.0;
    QTest::newRow("Arc less than 360 degree, radiuses 100, -50") << 100.0 << -50.0 << 0.0 << 340.0 << 0.0;
    QTest::newRow("Arc less than 360 degree, radiuses -100, -50") << -100.0 << -50.0 << 0.0 << 340.0 << 0.0;

    QTest::newRow("Arc less than 360 degree, radiuses 150, 400") << 150.0 << 400.0 << 0.0 << 340.0 << 0.0;
    QTest::newRow("Arc less than 360 degree, radiuses -150, 400") << -150.0 << 400.0 << 0.0 << 340.0 << 0.0;
    QTest::newRow("Arc less than 360 degree, radiuses 150, -400") << 150.0 << -400.0 << 0.0 << 340.0 << 0.0;
    QTest::newRow("Arc less than 360 degree, radiuses -150, -400") << -150.0 << -400.0 << 0.0 << 340.0 << 0.0;

    QTest::newRow("Arc less than 360 degree, radiuses 1500, 800") << 1500.0 << 800.0 << 0.0 << 340.0 << 0.0;
    QTest::newRow("Arc less than 360 degree, radiuses -1500, 800") << -1500.0 << 800.0 << 0.0 << 340.0 << 0.0;
    QTest::newRow("Arc less than 360 degree, radiuses 1500, -800") << 1500.0 << -800.0 << 0.0 << 340.0 << 0.0;
    QTest::newRow("Arc less than 360 degree, radiuses -1500, -800") << -1500.0 << -800.0 << 0.0 << 340.0 << 0.0;

    QTest::newRow("Arc less than 360 degree, radiuses 12000, 1200, rotation 30")
        << 12000.0 << 1200.0 << 0.0 << 340.0 << 30.0;
    QTest::newRow("Arc less than 360 degree, radiuses -12000, 1200, rotation 30")
        << -12000.0 << 1200.0 << 0.0 << 340.0 << 30.0;
    QTest::newRow("Arc less than 360 degree, radiuses 12000, -1200, rotation 30")
        << 12000.0 << -1200.0 << 0.0 << 340.0 << 30.0;
    QTest::newRow("Arc less than 360 degree, radiuses -12000, -1200, rotation 30")
        << -12000.0 << -1200.0 << 0.0 << 340.0 << 30.0;

    QTest::newRow("Arc less than 360 degree, radiuses 12000, 10000") << 12000.0 << 10000.0 << 0.0 << 340.0 << 0.0;
    QTest::newRow("Arc less than 360 degree, radiuses -12000, 10000") << -12000.0 << 10000.0 << 0.0 << 340.0 << 0.0;
    QTest::newRow("Arc less than 360 degree, radiuses 12000, -10000") << 12000.0 << -10000.0 << 0.0 << 340.0 << 0.0;
    QTest::newRow("Arc less than 360 degree, radiuses -12000, -10000") << -12000.0 << -10000.0 << 0.0 << 340.0 << 0.0;

    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses 100, 50") << 100.0 << 50.0 << 90.0 << 135.0 << 0.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses -100, 50") << -100.0 << 50.0 << 90.0 << 135.0 << 0.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses 100, -50") << 100.0 << -50.0 << 90.0 << 135.0 << 0.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses -100, -50")
        << -100.0 << -50.0 << 90.0 << 135.0 << 0.0;

    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses 150, 400") << 150.0 << 400.0 << 90.0 << 135.0 << 0.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses -150, 400")
        << -150.0 << 400.0 << 90.0 << 135.0 << 0.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses 150, -400")
        << 150.0 << -400.0 << 90.0 << 135.0 << 0.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses -150, -400")
        << -150.0 << -400.0 << 90.0 << 135.0 << 0.0;

    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses 1500, 800")
        << 1500.0 << 800.0 << 90.0 << 135.0 << 0.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses -1500, 800")
        << -1500.0 << 800.0 << 90.0 << 135.0 << 0.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses 1500, -800")
        << 1500.0 << -800.0 << 90.0 << 135.0 << 0.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses -1500, -800")
        << -1500.0 << -800.0 << 90.0 << 135.0 << 0.0;

    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses 13000, 1000")
        << 13000.0 << 1000.0 << 90.0 << 135.0 << 0.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses -13000, 1000")
        << -13000.0 << 1000.0 << 90.0 << 135.0 << 0.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses 13000, -1000")
        << 13000.0 << -1000.0 << 90.0 << 135.0 << 0.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses -13000, -1000")
        << -13000.0 << -1000.0 << 90.0 << 135.0 << 0.0;

    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses 15000, 10000")
        << 15000.0 << 10000.0 << 90.0 << 135.0 << 0.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses -15000, 10000")
        << -15000.0 << 10000.0 << 90.0 << 135.0 << 0.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses 15000, -10000")
        << 15000.0 << -10000.0 << 90.0 << 135.0 << 0.0;
    QTest::newRow("Arc start 90 degree, angle 45 degree, radiuses -15000, -10000")
        << -15000.0 << -10000.0 << 90.0 << 135.0 << 0.0;
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

    QVector<QPointF> const points = arc.GetPoints();
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
    QVector<QPointF> const points = arc.GetPoints();

    const qreal c = qSqrt(qAbs(radius2 * radius2 - radius1 * radius1));
    // distance from the center to the focus

    auto focus1 = static_cast<QPointF>(center);
    auto focus2 = static_cast<QPointF>(center);

    if (qAbs(radius1) < qAbs(radius2))
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

    QPointF ellipsePoint(center.x() + qAbs(radius1), center.y());
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
    QVector<QPointF> const points = arc.GetPoints();

    if (VFuzzyComparePossibleNulls(arc.AngleArc(), 360.0))
    { // calculated full ellipse square
        const qreal ellipseSquare = M_PI * qAbs(radius1) * qAbs(radius2);
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

    VEllipticalArc arc(center, radius1, radius2, startAngle, endAngle, 0);
    arc.SetApproximationScale(maxCurveApproximationScale);
    const qreal arcLength = arc.GetLength();

    VEllipticalArc fullArc(center, radius1, radius2, startAngle, endAngle, rotationAngle);
    fullArc.SetApproximationScale(maxCurveApproximationScale);
    const qreal arcLengthFull = fullArc.GetLength();

    if (VFuzzyComparePossibleNulls(arc.AngleArc(), 360.0))
    { // calculated full ellipse length
        Q_RELAXED_CONSTEXPR qreal epsLength = ToPixel(1, Unit::Mm);
        const qreal diffLength = qAbs(arcLength - arcLengthFull);
        const QString errorMsg2 = u"Difference between full arc lengths "
                                  u"(diff = '%1') bigger than eps = '%2'."_s.arg(diffLength)
                                      .arg(epsLength);
        QVERIFY2(diffLength <= epsLength, qUtf8Printable(errorMsg2));
    }
    else
    {
        QSKIP("Not full circumference");
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

    QVector<QPointF> const points = arc.GetPoints();

    if (points.size() > 2 && qFuzzyIsNull(rotationAngle))
    {
        Q_RELAXED_CONSTEXPR qreal testAccuracy = ToPixel(1.5, Unit::Mm);

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

    QTest::newRow("Test el arc 1, +r, +r") << QPointF() << 10. << 20.0 << 1. << 91. << 0. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 1, -r, +r") << QPointF() << -10. << 20.0 << 1. << 91. << 0. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 1, +r, -r") << QPointF() << 10. << -20.0 << 1. << 91. << 0. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 1, -r, -r") << QPointF() << -10. << -20.0 << 1. << 91. << 0. << QPointF() << 90. << "_r";

    QTest::newRow("Test el arc 2, +r, +r") << QPointF() << 10. << 20.0 << 0. << 90. << 0. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 2, -r, +r") << QPointF() << -10. << 20.0 << 0. << 90. << 0. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 2, +r, -r") << QPointF() << 10. << -20.0 << 0. << 90. << 0. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 2, -r, -r") << QPointF() << -10. << -20.0 << 0. << 90. << 0. << QPointF() << 90. << "_r";

    QTest::newRow("Test el arc 3.2, +r, +r")
        << QPointF(10, 10) << 10. << 20.0 << 0. << 90. << 0. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 3.2, -r, +r")
        << QPointF(10, 10) << -10. << 20.0 << 0. << 90. << 0. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 3.2, +r, -r")
        << QPointF(10, 10) << 10. << -20.0 << 0. << 90. << 0. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 3.2, -r, -r")
        << QPointF(10, 10) << -10. << -20.0 << 0. << 90. << 0. << QPointF() << 90. << "_r";

    QTest::newRow("Test el arc 3.1, +r, +r")
        << QPointF(10, 10) << 10. << 20.0 << 1. << 91. << 0. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 3.1, -r, +r")
        << QPointF(10, 10) << -10. << 20.0 << 1. << 91. << 0. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 3.1, +r, -r")
        << QPointF(10, 10) << 10. << -20.0 << 1. << 91. << 0. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 3.1, -r, -r")
        << QPointF(10, 10) << -10. << -20.0 << 1. << 91. << 0. << QPointF() << 90. << "_r";

    QTest::newRow("Test el arc 3, +r, +r")
        << QPointF(10, 10) << 10. << 20.0 << 1. << 91. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 3, -r, +r")
        << QPointF(10, 10) << -10. << 20.0 << 1. << 91. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 3, +r, -r")
        << QPointF(10, 10) << 10. << -20.0 << 1. << 91. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 3, -r, -r")
        << QPointF(10, 10) << -10. << -20.0 << 1. << 91. << 90. << QPointF() << 90. << "_r";

    QTest::newRow("Test el arc 4, +r, +r")
        << QPointF(10, 10) << 10. << 20.0 << 0. << 90. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 4, -r, +r")
        << QPointF(10, 10) << -10. << 20.0 << 0. << 90. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 4, +r, -r")
        << QPointF(10, 10) << 10. << -20.0 << 0. << 90. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 4, -r, -r")
        << QPointF(10, 10) << -10. << -20.0 << 0. << 90. << 90. << QPointF() << 90. << "_r";

    QTest::newRow("Test el arc 5, +r, +r")
        << QPointF(10, 10) << 10. << 20.0 << 0. << 180. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 5, -r, +r")
        << QPointF(10, 10) << -10. << 20.0 << 0. << 180. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 5, +r, -r")
        << QPointF(10, 10) << 10. << -20.0 << 0. << 180. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 5, -r, -r")
        << QPointF(10, 10) << -10. << -20.0 << 0. << 180. << 90. << QPointF() << 90. << "_r";

    QTest::newRow("Test el arc 6, +r, +r")
        << QPointF(10, 10) << 10. << 20.0 << 1. << 181. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 6, -r, +r")
        << QPointF(10, 10) << -10. << 20.0 << 1. << 181. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 6, +r, -r")
        << QPointF(10, 10) << 10. << -20.0 << 1. << 181. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 6, -r, -r")
        << QPointF(10, 10) << -10. << -20.0 << 1. << 181. << 90. << QPointF() << 90. << "_r";

    QTest::newRow("Test el arc 7, +r, +r")
        << QPointF(10, 10) << 10. << 20.0 << 0. << 270. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 7, -r, +r")
        << QPointF(10, 10) << -10. << 20.0 << 0. << 270. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 7, +r, -r")
        << QPointF(10, 10) << 10. << -20.0 << 0. << 270. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 7, -r, -r")
        << QPointF(10, 10) << -10. << -20.0 << 0. << 270. << 90. << QPointF() << 90. << "_r";

    QTest::newRow("Test el arc 8, +r, +r")
        << QPointF(10, 10) << 10. << 20.0 << 1. << 271. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 8, -r, +r")
        << QPointF(10, 10) << -10. << 20.0 << 1. << 271. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 8, +r, -r")
        << QPointF(10, 10) << 10. << -20.0 << 1. << 271. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 8, -r, -r")
        << QPointF(10, 10) << -10. << -20.0 << 1. << 271. << 90. << QPointF() << 90. << "_r";

    QTest::newRow("Test el arc 9, +r, +r")
        << QPointF(10, 10) << 10. << 20.0 << 0. << 360. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 9, -r, +r")
        << QPointF(10, 10) << -10. << 20.0 << 0. << 360. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 9, +r, -r")
        << QPointF(10, 10) << 10. << -20.0 << 0. << 360. << 90. << QPointF() << 90. << "_r";
    QTest::newRow("Test el arc 9, -r, -r")
        << QPointF(10, 10) << -10. << -20.0 << 0. << 360. << 90. << QPointF() << 90. << "_r";
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
    QString const errorLengthMsg =
        u"Difference between real and computing lengthes bigger than eps = %1. l1 = %2; l2 = %3"_s;
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
    QTest::addColumn<qreal>("radius1");
    QTest::addColumn<qreal>("radius2");
    QTest::addColumn<QLineF>("axis");
    QTest::addColumn<QString>("prefix");

    QLineF axis(QPointF(600, 30), QPointF(600, 1800));

    QTest::newRow("Vertical axis, +r, +r") << 10. << 20.0 << axis << "a2";
    QTest::newRow("Vertical axis, -r, +r") << -10. << 20.0 << axis << "a2";
    QTest::newRow("Vertical axis, +r, -r") << 10. << -20.0 << axis << "a2";
    QTest::newRow("Vertical axis, -r, -r") << -10. << -20.0 << axis << "a2";

    axis = QLineF(QPointF(600, 30), QPointF(1200, 30));

    QTest::newRow("Horizontal axis, +r, +r") << 10. << 20.0 << axis << "a2";
    QTest::newRow("Horizontal axis, -r, +r") << -10. << 20.0 << axis << "a2";
    QTest::newRow("Horizontal axis, +r, -r") << 10. << -20.0 << axis << "a2";
    QTest::newRow("Horizontal axis, -r, -r") << -10. << -20.0 << axis << "a2";

    axis = QLineF(QPointF(600, 30), QPointF(600, 1800));
    axis.setAngle(45);

    QTest::newRow("Diagonal axis, +r, +r") << 10. << 20.0 << axis << "a2";
    QTest::newRow("Diagonal axis, -r, +r") << -10. << 20.0 << axis << "a2";
    QTest::newRow("Diagonal axis, +r, -r") << 10. << -20.0 << axis << "a2";
    QTest::newRow("Diagonal axis, -r, -r") << -10. << -20.0 << axis << "a2";
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestFlip()
{
    QFETCH(qreal, radius1);
    QFETCH(qreal, radius2);
    QFETCH(QLineF, axis);
    QFETCH(QString, prefix);

    const VEllipticalArc elArc(VPointF(), radius1, radius2, 1., 91., 0.);
    const qreal length1 = qAbs(elArc.GetLength());

    const VEllipticalArc res = elArc.Flip(axis, prefix);
    const qreal length2 = qAbs(res.GetLength());

    // cppcheck-suppress unreadVariable
    const auto errorMsg = QStringLiteral("The name doesn't contain the prefix '%1'.").arg(prefix);
    QVERIFY2(res.name().endsWith(prefix), qUtf8Printable(errorMsg));

    QString const errorLengthMsg
        = u"Difference between original and flipped lengthes bigger than eps = %1. l1 = %2; l2 = %3"_s;
    QVERIFY2(qAbs(length1 - length2) <= ToPixel(1, Unit::Mm),
             qUtf8Printable(errorLengthMsg.arg(ToPixel(1, Unit::Mm)).arg(length1).arg(length2)));

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

    const qreal calcLength = qAbs(empty.GetLength());

    QString const errorLengthMsg
        = u"Difference between original and expected lengthes bigger than eps = %1. l1 = %2; l2 = %3"_s;
    QVERIFY2(qAbs(calcLength - length) <= ToPixel(1, Unit::Mm),
             qUtf8Printable(errorLengthMsg.arg(ToPixel(1, Unit::Mm)).arg(calcLength).arg(length)));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestCutArc_data()
{
    QTest::addColumn<qreal>("r1");
    QTest::addColumn<qreal>("r2");
    QTest::addColumn<qreal>("f1");
    QTest::addColumn<qreal>("f2");
    QTest::addColumn<qreal>("rotDeg");
    QTest::addColumn<qreal>("cutFraction");

    // Circles
    QTest::newRow("circle_quarter_at_half") << 100.0 << 100.0 << 0.0 << 90.0 << 0.0 << 0.50;
    QTest::newRow("circle_half_at_quarter") << 100.0 << 100.0 << 0.0 << 180.0 << 0.0 << 0.25;
    QTest::newRow("circle_threequarter") << 100.0 << 100.0 << 0.0 << 270.0 << 0.0 << 0.75;
    QTest::newRow("circle_full_10pct") << 100.0 << 100.0 << 0.0 << 360.0 << 0.0 << 0.10;
    QTest::newRow("circle_full_90pct") << 100.0 << 100.0 << 0.0 << 360.0 << 0.0 << 0.90;
    QTest::newRow("circle_near_start") << 100.0 << 100.0 << 0.0 << 360.0 << 0.0 << 0.01;
    QTest::newRow("circle_near_end") << 100.0 << 100.0 << 0.0 << 360.0 << 0.0 << 0.99;

    // Regular ellipses
    QTest::newRow("ellipse_r50_r30") << 50.0 << 30.0 << 30.0 << 150.0 << 0.0 << 0.33;
    QTest::newRow("ellipse_half_at_30pct") << 60.0 << 40.0 << 0.0 << 180.0 << 0.0 << 0.30;
    QTest::newRow("ellipse_partial_arc") << 80.0 << 50.0 << 30.0 << 120.0 << 0.0 << 0.70;
    QTest::newRow("ellipse_full_50pct") << 100.0 << 60.0 << 0.0 << 360.0 << 0.0 << 0.50;
    QTest::newRow("ellipse_nearly_circular") << 80.0 << 79.0 << 0.0 << 359.9 << 0.0 << 0.50;

    // Thin / eccentric ellipses
    QTest::newRow("thin_ellipse_half") << 200.0 << 20.0 << 0.0 << 180.0 << 0.0 << 0.40;
    QTest::newRow("very_thin_quarter") << 300.0 << 10.0 << 0.0 << 90.0 << 45.0 << 0.20;

    // Scale extremes
    QTest::newRow("large_radii") << 500.0 << 300.0 << 10.0 << 350.0 << 30.0 << 0.60;
    QTest::newRow("small_radii") << 5.0 << 3.0 << 0.0 << 270.0 << 0.0 << 0.80;

    // Rotation angle sweep (same geometry, only rotation varies)
    QTest::newRow("rot_0") << 100.0 << 60.0 << 0.0 << 270.0 << 0.0 << 0.50;
    QTest::newRow("rot_30") << 100.0 << 60.0 << 0.0 << 270.0 << 30.0 << 0.50;
    QTest::newRow("rot_45") << 100.0 << 60.0 << 0.0 << 270.0 << 45.0 << 0.50;
    QTest::newRow("rot_90") << 100.0 << 60.0 << 0.0 << 270.0 << 90.0 << 0.50;
    QTest::newRow("rot_135") << 100.0 << 60.0 << 0.0 << 270.0 << 135.0 << 0.50;
    QTest::newRow("rot_180") << 100.0 << 60.0 << 0.0 << 270.0 << 180.0 << 0.50;
    QTest::newRow("rot_270") << 100.0 << 60.0 << 0.0 << 270.0 << 270.0 << 0.50;
    QTest::newRow("rot_359") << 100.0 << 60.0 << 0.0 << 270.0 << 359.0 << 0.50;
    QTest::newRow("rotated_ellipse_33pct") << 120.0 << 70.0 << 10.0 << 350.0 << 90.0 << 0.33;
    QTest::newRow("rotated_ellipse_45deg") << 80.0 << 40.0 << 0.0 << 180.0 << 45.0 << 0.50;
    QTest::newRow("rotated_45_50pct") << 60.0 << 40.0 << 45.0 << 315.0 << 60.0 << 0.50;

    // ── Reversed arcs (clockwise sweep via negative radius) ──────────────────
    // A negative r1 or r2 flips the sweep direction. Mirrors the forward cases
    // above to exercise the reversed-direction branch in FindF2 and sign
    // handling inside DoCutArc / DoCutArcByLength.
    QTest::newRow("rev_neg_r1_circle_quarter") << -100.0 << 100.0 << 0.0 << 90.0 << 0.0 << 0.50;
    QTest::newRow("rev_neg_r1_circle_half") << -100.0 << 100.0 << 0.0 << 180.0 << 0.0 << 0.25;
    QTest::newRow("rev_neg_r1_circle_threequart") << -100.0 << 100.0 << 0.0 << 270.0 << 0.0 << 0.75;
    QTest::newRow("rev_neg_r1_near_start") << -100.0 << 100.0 << 0.0 << 360.0 << 0.0 << 0.01;
    QTest::newRow("rev_neg_r1_near_end") << -100.0 << 100.0 << 0.0 << 360.0 << 0.0 << 0.99;
    QTest::newRow("rev_neg_r2_circle_half") << 100.0 << -100.0 << 0.0 << 180.0 << 0.0 << 0.50;
    QTest::newRow("rev_neg_r1_ellipse_r50_r30") << -50.0 << 30.0 << 30.0 << 150.0 << 0.0 << 0.33;
    QTest::newRow("rev_neg_r2_ellipse_partial") << 80.0 << -50.0 << 30.0 << 120.0 << 0.0 << 0.70;
    QTest::newRow("rev_neg_r1_thin_ellipse") << -200.0 << 20.0 << 0.0 << 180.0 << 0.0 << 0.40;
    QTest::newRow("rev_neg_r2_large_radii") << 500.0 << -300.0 << 10.0 << 350.0 << 30.0 << 0.60;
    QTest::newRow("rev_neg_r1_small_radii") << -5.0 << 3.0 << 0.0 << 270.0 << 0.0 << 0.80;
    QTest::newRow("rev_neg_r1_rot_45") << -100.0 << 60.0 << 0.0 << 270.0 << 45.0 << 0.50;
    QTest::newRow("rev_neg_r1_rot_90") << -100.0 << 60.0 << 0.0 << 270.0 << 90.0 << 0.50;
    QTest::newRow("rev_neg_r1_rot_180") << -100.0 << 60.0 << 0.0 << 270.0 << 180.0 << 0.50;
    QTest::newRow("rev_neg_r2_rotated_ellipse") << 120.0 << -70.0 << 10.0 << 350.0 << 90.0 << 0.33;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestCutArc()
{
    QFETCH(qreal, r1);
    QFETCH(qreal, r2);
    QFETCH(qreal, f1);
    QFETCH(qreal, f2);
    QFETCH(qreal, rotDeg);
    QFETCH(qreal, cutFraction);

    const VEllipticalArc arc = MakeArc(r1, r2, f1, f2, rotDeg);
    const qreal total = arc.GetLength();
    const qreal cutLen = total * cutFraction;

    const auto [cutPoint, arc1, arc2] = DoCut(arc, cutLen);

    // 1. Cut point lies on the junction of both sub-arcs
    QVERIFY2(VFuzzyComparePoints(cutPoint, arc1.GetP2()), qPrintable(QStringLiteral("cutPoint != arc1.P2")));
    QVERIFY2(VFuzzyComparePoints(cutPoint, arc2.GetP1()), qPrintable(QStringLiteral("cutPoint != arc2.P1")));

    // 2. Length conservation: arc1 + arc2 == original
    const qreal sum = arc1.GetLength() + arc2.GetLength();
    const qreal diff = qAbs(sum - total);
    QVERIFY2(diff < accuracyPointOnLine,
             qPrintable(
                 QStringLiteral("length not conserved: orig=%1  arc1+arc2=%2  diff=%3").arg(total).arg(sum).arg(diff)));

    // 3. Each piece matches the requested split
    QVERIFY2(qAbs(arc1.GetLength() - cutLen) < accuracyPointOnLine,
             qPrintable(QStringLiteral("arc1 length: expected=%1  got=%2").arg(cutLen).arg(arc1.GetLength())));
    QVERIFY2(qAbs(arc2.GetLength() - (total - cutLen)) < accuracyPointOnLine,
             qPrintable(QStringLiteral("arc2 length: expected=%1  got=%2").arg(total - cutLen).arg(arc2.GetLength())));

    // 4. Endpoint continuity: orig.P1 → cutPoint → orig.P2
    QVERIFY2(VFuzzyComparePoints(arc.GetP1(), arc1.GetP1()), "arc1.P1 != original.P1");
    QVERIFY2(VFuzzyComparePoints(arc.GetP2(), arc2.GetP2()), "arc2.P2 != original.P2");
    QVERIFY2(VFuzzyComparePoints(arc1.GetP2(), arc2.GetP1()), "junction discontinuous");

    // 5. Rotation angle is propagated into both children
    QVERIFY2(VFuzzyComparePossibleNulls(arc1.GetRotationAngle(), rotDeg),
             qPrintable(QStringLiteral("arc1 lost rotation angle: expected=%1  got=%2")
                            .arg(rotDeg)
                            .arg(arc1.GetRotationAngle())));
    QVERIFY2(VFuzzyComparePossibleNulls(arc2.GetRotationAngle(), rotDeg),
             qPrintable(QStringLiteral("arc2 lost rotation angle: expected=%1  got=%2")
                            .arg(rotDeg)
                            .arg(arc2.GetRotationAngle())));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestCutArc_ZeroLength_data()
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestCutArc_ZeroLength()
{
    for (const VEllipticalArc &arc : {MakeArc(100.0, 100.0, 0.0, 180.0, 0.0),   // forward
                                      MakeArc(-100.0, 100.0, 0.0, 180.0, 0.0)}) // reversed (neg r1)
    {
        const auto [cp, arc1, arc2] = DoCut(arc, 0.0);

        QVERIFY2(VFuzzyComparePoints(cp, arc.GetP1()), "zero-cut: cutPoint != P1");
        QVERIFY2(arc1.GetLength() < accuracyPointOnLine,
                 qPrintable(QStringLiteral("zero-cut: arc1 length=%1").arg(arc1.GetLength())));
        QVERIFY2(qAbs(arc2.GetLength() - arc.GetLength()) < accuracyPointOnLine, "zero-cut: arc2 length != original");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestCutArc_FullLength_data()
{
    QTest::addColumn<qreal>("r1");
    QTest::addColumn<qreal>("r2");
    QTest::addColumn<qreal>("f1");
    QTest::addColumn<qreal>("f2");
    QTest::addColumn<qreal>("rotDeg");

    QTest::newRow("forward") << 100.0 << 100.0 << 0.0 << 270.0 << 0.0;
    QTest::newRow("reversed (neg r1)") << -100.0 << 100.0 << 0.0 << 270.0 << 0.0;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestCutArc_FullLength()
{
    QFETCH(qreal, r1);
    QFETCH(qreal, r2);
    QFETCH(qreal, f1);
    QFETCH(qreal, f2);
    QFETCH(qreal, rotDeg);

    const VEllipticalArc arc = MakeArc(r1, r2, f1, f2, rotDeg);

    const qreal total = arc.GetLength();
    const auto [cp, arc1, arc2] = DoCut(arc, total);

    QVERIFY2(VFuzzyComparePoints(cp, arc.GetP2()), "full-cut: cutPoint != P2");
    QVERIFY2(qAbs(arc1.GetLength() - total) < accuracyPointOnLine, "full-cut: arc1 length != total");
    QVERIFY2(arc2.GetLength() < accuracyPointOnLine,
             qPrintable(QStringLiteral("full-cut: arc2 length=%1").arg(arc2.GetLength())));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestCutArc_LengthExceedsArc_data()
{
    QTest::addColumn<qreal>("r1");
    QTest::addColumn<qreal>("r2");
    QTest::addColumn<qreal>("f1");
    QTest::addColumn<qreal>("f2");
    QTest::addColumn<qreal>("rotDeg");

    QTest::newRow("forward") << 50.0 << 30.0 << 0.0 << 180.0 << 0.0;
    QTest::newRow("reversed (neg r1)") << -50.0 << 30.0 << 0.0 << 180.0 << 0.0;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestCutArc_LengthExceedsArc()
{
    QFETCH(qreal, r1);
    QFETCH(qreal, r2);
    QFETCH(qreal, f1);
    QFETCH(qreal, f2);
    QFETCH(qreal, rotDeg);

    const VEllipticalArc arc = MakeArc(r1, r2, f1, f2, rotDeg);

    const qreal total = arc.GetLength();
    const auto [cp, arc1, arc2] = DoCut(arc, total * 2.0);

    QVERIFY2(arc1.GetLength() <= total + accuracyPointOnLine, "exceed-cut: arc1 exceeds original");
    QVERIFY2(qFuzzyIsNull(arc2.GetLength()), "exceed-cut: arc2 not degenerate after clamp");
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestCutArc_HalfLength_data()
{
    QTest::addColumn<qreal>("r1");
    QTest::addColumn<qreal>("r2");
    QTest::addColumn<qreal>("f1");
    QTest::addColumn<qreal>("f2");
    QTest::addColumn<qreal>("rotDeg");

    // Symmetric circle cut exactly in half: both arcs must be equal.
    // Holds for both winding directions.
    QTest::newRow("forward") << 100.0 << 100.0 << 0.0 << 360.0 << 0.0;
    QTest::newRow("reversed (neg r1)") << -100.0 << 100.0 << 0.0 << 360.0 << 0.0;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestCutArc_HalfLength()
{
    QFETCH(qreal, r1);
    QFETCH(qreal, r2);
    QFETCH(qreal, f1);
    QFETCH(qreal, f2);
    QFETCH(qreal, rotDeg);

    const VEllipticalArc arc = MakeArc(r1, r2, f1, f2, rotDeg);

    const auto [cp, arc1, arc2] = DoCut(arc, arc.GetLength() / 2.0);

    QVERIFY2(qAbs(arc1.GetLength() - arc2.GetLength()) < accuracyPointOnLine,
             qPrintable(QStringLiteral("half-cut: arc1=%1  arc2=%2").arg(arc1.GetLength()).arg(arc2.GetLength())));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestCutArc_RepeatedCut_Consistent_data()
{
    QTest::addColumn<qreal>("r1");
    QTest::addColumn<qreal>("r2");
    QTest::addColumn<qreal>("f1");
    QTest::addColumn<qreal>("f2");
    QTest::addColumn<qreal>("rotDeg");

    // Cut at 1/3, then cut the first piece at its own midpoint.
    // Three resulting pieces must sum to the original and form an unbroken chain.
    // Run for both forward and reversed arc directions.
    QTest::newRow("forward") << 100.0 << 70.0 << 0.0 << 270.0 << 15.0;
    QTest::newRow("reversed (neg r1)") << -100.0 << 70.0 << 0.0 << 270.0 << 15.0;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VEllipticalArc::TestCutArc_RepeatedCut_Consistent()
{
    QFETCH(qreal, r1);
    QFETCH(qreal, r2);
    QFETCH(qreal, f1);
    QFETCH(qreal, f2);
    QFETCH(qreal, rotDeg);

    const VEllipticalArc arc = MakeArc(r1, r2, f1, f2, rotDeg);

    const qreal total = arc.GetLength();

    const auto [cp1, a1, a2] = DoCut(arc, total / 3.0);
    const auto [cp2, b1, b2] = DoCut(a1, a1.GetLength() / 2.0);

    // Length: three pieces sum to original
    const qreal sum3 = b1.GetLength() + b2.GetLength() + a2.GetLength();
    QVERIFY2(qAbs(sum3 - total) < accuracyPointOnLine,
             qPrintable(QStringLiteral("repeated-cut: 3-piece sum=%1  original=%2").arg(sum3).arg(total)));

    // Endpoint chain: b1.P1 → b2 → a2.P2
    QVERIFY2(VFuzzyComparePoints(b1.GetP1(), arc.GetP1()), "repeated-cut: chain start broken");
    QVERIFY2(VFuzzyComparePoints(b1.GetP2(), b2.GetP1()), "repeated-cut: inner junction broken");
    QVERIFY2(VFuzzyComparePoints(b2.GetP2(), a2.GetP1()), "repeated-cut: outer junction broken");
    QVERIFY2(VFuzzyComparePoints(a2.GetP2(), arc.GetP2()), "repeated-cut: chain end broken");
}
