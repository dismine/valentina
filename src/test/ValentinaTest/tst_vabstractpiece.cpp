/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   17 11, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#include "tst_vabstractpiece.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vpointf.h"
#include "../vlayout/vabstractpiece.h"
#include "../vlayout/vrawsapoint.h"

#include <QPointF>
#include <QVector>

#include <QtTest>

//---------------------------------------------------------------------------------------------------------------------
TST_VAbstractPiece::TST_VAbstractPiece(QObject *parent)
  : AbstractTest(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::EquidistantRemoveLoop_data()
{
    QTest::addColumn<QVector<VSAPoint>>("points");
    QTest::addColumn<qreal>("width");
    QTest::addColumn<QVector<QPointF>>("ekvOrig");

    auto ASSERT_TEST_CASE = [](const char *title, const QString &input, const QString &output, qreal width)
    {
        QVector<VSAPoint> const inputPoints = AbstractTest::VectorFromJson<VSAPoint>(input);
        QVector<QPointF> const outputPoints = AbstractTest::VectorFromJson<QPointF>(output);
        QTest::newRow(title) << inputPoints << width << outputPoints;
    };

    // See file src/app/share/collection/test/seamtest1.val
    ASSERT_TEST_CASE("Seam test 1. Piece. By angle.", QStringLiteral("://seamtest1_by_angle/input.json"),
                     QStringLiteral("://seamtest1_by_angle/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/test/seamtest1.val
    ASSERT_TEST_CASE("Seam test 1. Piece. By angle 2.", QStringLiteral("://seamtest1_by_angle_2/input.json"),
                     QStringLiteral("://seamtest1_by_angle_2/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/test/seamtest1.val
    ASSERT_TEST_CASE("Seam test 1. Piece. By intersection.", QStringLiteral("://seamtest1_by_intersection/input.json"),
                     QStringLiteral("://seamtest1_by_intersection/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/test/seamtest1.val
    ASSERT_TEST_CASE("Seam test 1. Piece. By first edge symmetry.",
                     QStringLiteral("://seamtest1_by_first_edge_symmetry/input.json"),
                     QStringLiteral("://seamtest1_by_first_edge_symmetry/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/test/seamtest1.val
    ASSERT_TEST_CASE("Seam test 1. Piece. By second edge symmetry.",
                     QStringLiteral("://seamtest1_by_second_edge_symmetry/input.json"),
                     QStringLiteral("://seamtest1_by_second_edge_symmetry/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/test/seamtest1.val
    ASSERT_TEST_CASE("Seam test 1. Piece. By first right angle.",
                     QStringLiteral("://seamtest1_by_first_right_angle/input.json"),
                     QStringLiteral("://seamtest1_by_first_right_angle/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/test/seamtest1.val
    ASSERT_TEST_CASE("Seam test 1. Piece. By second right angle.",
                     QStringLiteral("://seamtest1_by_second_right_angle/input.json"),
                     QStringLiteral("://seamtest1_by_second_right_angle/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/test/seamtest2.val
    ASSERT_TEST_CASE("Seam test 2", QStringLiteral("://seamtest2/input.json"),
                     QStringLiteral("://seamtest2/output.json"), 37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/test/seamtest3.val
    ASSERT_TEST_CASE("Seam test 3", QStringLiteral("://seamtest3/input.json"),
                     QStringLiteral("://seamtest3/output.json"), 37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // These are two real cases where equdistant has loop.
    // See issue #298. Segmented Curve isn't selected in Seam Allowance tool.
    // https://bitbucket.org/dismine/valentina/issue/298/segmented-curve-isnt-selected-in-seam
    // See file src/app/share/collection/TestPuzzle.val
    // Code should clean loops in path.
    ASSERT_TEST_CASE("Issue 298. Case1", QStringLiteral("://Issue_298_case1/input.json"),
                     QStringLiteral("://Issue_298_case1/output.json"), 75.59055118110237 /*seam allowance width*/);

    ASSERT_TEST_CASE("Issue 298. Case2", QStringLiteral("://Issue_298_case2/input.json"),
                     QStringLiteral("://Issue_298_case2/output.json"), 37.795275590551185 /*seam allowance width*/);

    // See issue #548. Bug Detail tool. Case when seam allowance is wrong.
    // https://bitbucket.org/dismine/valentina/issues/548/bug-detail-tool-case-when-seam-allowance
    // Files: Steampunk_trousers.val and marie.vit
    // Actually buggy detail see in file src/app/share/collection/bugs/Steampunk_trousers_issue_#548.val
    // Code should clean loops in path.
    ASSERT_TEST_CASE("Issue 548. Case1", QStringLiteral("://Issue_548_case1/input.json"),
                     QStringLiteral("://Issue_548_case1/output.json"),
                     11.338582677165354 /*seam allowance width (0.3 cm)*/);

    ASSERT_TEST_CASE("Issue 548. Case2", QStringLiteral("://Issue_548_case2/input.json"),
                     QStringLiteral("://Issue_548_case2/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    ASSERT_TEST_CASE("Issue 548. Case3", QStringLiteral("://Issue_548_case3/input.json"),
                     QStringLiteral("://Issue_548_case3/output.json"),
                     75.59055118110237 /*seam allowance width (2.0 cm)*/);

    // See file src/app/share/collection/bugs/Issue_#646.val
    ASSERT_TEST_CASE("Issue 646.", QStringLiteral("://Issue_646/input.json"),
                     QStringLiteral("://Issue_646/output.json"), 37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/bugs/Issue_#923.val
    ASSERT_TEST_CASE("Issue 923. Test 1", QStringLiteral("://Issue_923_test1/input.json"),
                     QStringLiteral("://Issue_923_test1/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/bugs/Issue_#923.val
    ASSERT_TEST_CASE("Issue 923. Test 2", QStringLiteral("://Issue_923_test2/input.json"),
                     QStringLiteral("://Issue_923_test2/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/bugs/Issue_#923.val
    ASSERT_TEST_CASE("Issue 923. Test 3", QStringLiteral("://Issue_923_test3/input.json"),
                     QStringLiteral("://Issue_923_test3/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/bugs/Issue_#923.val
    ASSERT_TEST_CASE("Issue 923. Test 4", QStringLiteral("://Issue_923_test4/input.json"),
                     QStringLiteral("://Issue_923_test4/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/bugs/Issue_#923.val
    ASSERT_TEST_CASE("Issue 923. Test 5", QStringLiteral("://Issue_923_test5/input.json"),
                     QStringLiteral("://Issue_923_test5/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/bugs/Issue_#923.val
    ASSERT_TEST_CASE("Issue 923. Test 6", QStringLiteral("://Issue_923_test6/input.json"),
                     QStringLiteral("://Issue_923_test6/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/bugs/Issue_#923.val
    ASSERT_TEST_CASE("Issue 923. Test 1.1", QStringLiteral("://Issue_923_test1_1/input.json"),
                     QStringLiteral("://Issue_923_test1_1/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/bugs/Issue_#923.val
    ASSERT_TEST_CASE("Issue 923. Test 2.2", QStringLiteral("://Issue_923_test2_2/input.json"),
                     QStringLiteral("://Issue_923_test2_2/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/bugs/Issue_#923.val
    ASSERT_TEST_CASE("Issue 923. Test 3.3", QStringLiteral("://Issue_923_test3_3/input.json"),
                     QStringLiteral("://Issue_923_test3_3/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/bugs/Issue_#923.val
    ASSERT_TEST_CASE("Issue 923. Test 4.4", QStringLiteral("://Issue_923_test4_4/input.json"),
                     QStringLiteral("://Issue_923_test4_4/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/bugs/Issue_#923.val
    ASSERT_TEST_CASE("Issue 923. Test 5.5", QStringLiteral("://Issue_923_test5_5/input.json"),
                     QStringLiteral("://Issue_923_test5_5/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/bugs/Issue_#923.val
    ASSERT_TEST_CASE("Issue 923. Test 6.6", QStringLiteral("://Issue_923_test6_6/input.json"),
                     QStringLiteral("://Issue_923_test6_6/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/bugs/Issue_#923_test7.val
    ASSERT_TEST_CASE("Issue 923. Test 7.1", QStringLiteral("://Issue_923_test7_1/input.json"),
                     QStringLiteral("://Issue_923_test7_1/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/bugs/Issue_#923_test7.val
    ASSERT_TEST_CASE("Issue 923. Test 7.2", QStringLiteral("://Issue_923_test7_2/input.json"),
                     QStringLiteral("://Issue_923_test7_2/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/bugs/Issue_#923_test7.val
    ASSERT_TEST_CASE("Issue 923. Test 7.3", QStringLiteral("://Issue_923_test7_3/input.json"),
                     QStringLiteral("://Issue_923_test7_3/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/bugs/loop_by_intersection.val
    ASSERT_TEST_CASE("Loop for angle by intersection", QStringLiteral("://loop_by_intersection/input.json"),
                     QStringLiteral("://loop_by_intersection/output.json"),
                     39.685039370078741 /*seam allowance width (1.05 cm)*/);

    // See file valentina_private_collection/bugs/loop_start_point_on_line/loop_start_point_on_line.val
    // (private collection)
    ASSERT_TEST_CASE("Loop for start point on line", QStringLiteral("://loop_start_point_on_line/input.json"),
                     QStringLiteral("://loop_start_point_on_line/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file valentina_private_collection/bugs/DP_6_hem/DP_6_hem.val
    ASSERT_TEST_CASE("By length", QStringLiteral("://DP_6_hem_by_length/input.json"),
                     QStringLiteral("://DP_6_hem_by_length/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file valentina_private_collection/bugs/DP_6_hem/DP_6_hem.val
    ASSERT_TEST_CASE("By intersection", QStringLiteral("://DP_6_hem_by_intersection/input.json"),
                     QStringLiteral("://DP_6_hem_by_intersection/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file valentina_private_collection/bugs/DP_6_hem/DP_6_hem.val
    ASSERT_TEST_CASE("By first edge symmetry", QStringLiteral("://DP_6_hem_by_first_edge_symmetry/input.json"),
                     QStringLiteral("://DP_6_hem_by_first_edge_symmetry/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file valentina_private_collection/bugs/DP_6_hem/DP_6_hem.val
    ASSERT_TEST_CASE("By second edge symmetry", QStringLiteral("://DP_6_hem_by_second_edge_symmetry/input.json"),
                     QStringLiteral("://DP_6_hem_by_second_edge_symmetry/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file valentina_private_collection/bugs/DP_6_hem/DP_6_hem.val
    ASSERT_TEST_CASE("By first edge right angle", QStringLiteral("://DP_6_hem_by_first_edge_right_angle/input.json"),
                     QStringLiteral("://DP_6_hem_by_first_edge_right_angle/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file valentina_private_collection/bugs/DP_6_hem/DP_6_hem.val
    ASSERT_TEST_CASE("By second edge right angle", QStringLiteral("://DP_6_hem_by_second_edge_right_angle/input.json"),
                     QStringLiteral("://DP_6_hem_by_second_edge_right_angle/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file valentina_private_collection/bugs/DP_6/DP_6.val
    ASSERT_TEST_CASE("Hem by seam allowance", QStringLiteral("://DP_6/input.json"),
                     QStringLiteral("://DP_6/output.json"), 37.795275590551185 /*seam allowance width (1.0 cm)*/);

    // See file src/app/share/collection/bugs/smart_pattern_#36.val
    ASSERT_TEST_CASE("Incorrect position of a notch", QStringLiteral("://smart_pattern_#36/input.json"),
                     QStringLiteral("://smart_pattern_#36/output.json"),
                     30.236220472440944 /*seam allowance width (0.8 cm)*/);

    // See file valentina_private_collection/bugs/winter_coat/winter_coat.val
    ASSERT_TEST_CASE("Incorrect seam allowance", QStringLiteral("://winter_coat/input.json"),
                     QStringLiteral("://winter_coat/output.json"),
                     37.795275590551185 /*seam allowance width (1.0 cm)*/);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::EquidistantRemoveLoop() const
{
    QFETCH(QVector<VSAPoint>, points);
    QFETCH(qreal, width);
    QFETCH(QVector<QPointF>, ekvOrig);

    QVector<QPointF> ekv;
    CastTo(VAbstractPiece::Equidistant(points, width, QString()), ekv);

    // Begin comparison
    ComparePaths(ekv, ekvOrig);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::LayoutAllowanceRemoveLoop_data()
{
    QTest::addColumn<QVector<VSAPoint>>("points");
    QTest::addColumn<qreal>("width");
    QTest::addColumn<QVector<QPointF>>("ekvOrig");

    auto ASSERT_TEST_CASE = [](const char *title, const QString &input, const QString &output, qreal width)
    {
        QVector<VSAPoint> const inputPoints = AbstractTest::VectorFromJson<VSAPoint>(input);
        QVector<QPointF> const outputPoints = AbstractTest::VectorFromJson<QPointF>(output);
        QTest::newRow(title) << inputPoints << width << outputPoints;
    };

    // See file valentina_private_collection/bugs/smart_pattern_#58/smart_pattern_#58.val
    ASSERT_TEST_CASE("Loop in layout allowance", QStringLiteral("://smart_pattern_#58/input.json"),
                     QStringLiteral("://smart_pattern_#58/output.json"),
                     18.897637795275593 /*seam allowance width (0.5 cm)*/);

    // See file src/app/share/collection/bugs/smart_pattern_#99.val
    ASSERT_TEST_CASE("Incorrect fix of distortion", QStringLiteral("://smart_pattern_#99/input.json"),
                     QStringLiteral("://smart_pattern_#99/output.json"), 28.346456692913389 /*seam allowance width*/);

    // See file valentina_private_collection/test/smart_pattern_#120/smart_pattern_#120.val
    ASSERT_TEST_CASE("Piece 1", QStringLiteral("://smart_pattern_#120_piece_1/input.json"),
                     QStringLiteral("://smart_pattern_#120_piece_1/output.json"), 37.795275590551185);

    // See file valentina_private_collection/test/smart_pattern_#120/smart_pattern_#120.val
    ASSERT_TEST_CASE("Piece 2", QStringLiteral("://smart_pattern_#120_piece_2/input.json"),
                     QStringLiteral("://smart_pattern_#120_piece_2/output.json"), 37.795275590551185);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::LayoutAllowanceRemoveLoop() const
{
    QFETCH(QVector<VSAPoint>, points);
    QFETCH(qreal, width);
    QFETCH(QVector<QPointF>, ekvOrig);

    QVector<QPointF> ekv;
    CastTo(VAbstractPiece::Equidistant(points, width, QString()), ekv);

    // Begin comparison
    ComparePaths(ekv, ekvOrig);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::SumTrapezoids() const
{
    // Case3 checks that the method 'SumTrapezoids' returns negative value for three clockwise allocated points
    // Case4 checks that the method 'SumTrapezoids' returns positive value for three counterclock-wise allocated points
    // Case5 checks that the method 'SumTrapezoids' returns 0 for one point
    Case3();
    Case4();
    Case5();
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::TestAreaCalculation_data()
{
    QTest::addColumn<QVector<QPointF>>("path");
    QTest::addColumn<qreal>("excpectedArea");

    {
        QVector<QPointF> const rect{QPointF(), QPointF(100, 0), QPointF(100, 50), QPointF(0, 50), QPointF()};

        QTest::newRow("Rectangle 100 x 50") << rect << 100.0 * 50.0;
    }

    {
        VArc circle(VPointF(100, 100, QChar('C'), 0, 0), 100, 35, 35);
        circle.SetApproximationScale(maxCurveApproximationScale);
        QVector<QPointF> circlePath = circle.GetPoints();
        if (not circlePath.isEmpty() && not VFuzzyComparePoints(circlePath.constFirst(), circlePath.constLast()))
        {
            circlePath.append(circlePath.constFirst());
        }
        QTest::newRow("Circle radis 100") << circlePath << M_PI * pow(100, 2);
    }

    {
        VPointF const sectorCenter(100, 100, QChar('C'), 0, 0);
        VArc sector(sectorCenter, 100, 15, 45);
        sector.SetApproximationScale(maxCurveApproximationScale);
        QVector<QPointF> sectorPath = sector.GetPoints();
        sectorPath.append(sectorCenter.toQPointF());
        sectorPath.append(sectorPath.constFirst());
        QTest::newRow("Sector radius 100, 30 degree") << sectorPath << (M_PI * pow(100, 2) * 30.0) / 360.0;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::TestAreaCalculation() const
{
    QFETCH(QVector<QPointF>, path);
    QFETCH(qreal, excpectedArea);

    const qreal result = qAbs(VAbstractPiece::SumTrapezoids(path) / 2.0);
    QVERIFY(qAbs(result - excpectedArea) < MmToPixel(1.0));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::RawPathRemoveLoop_data() const
{
    QTest::addColumn<QVector<VRawSAPoint>>("path");
    QTest::addColumn<QVector<QPointF>>("expect");

    auto ASSERT_TEST_CASE = [](const char *title, const QString &input, const QString &output)
    {
        QVector<VRawSAPoint> const inputPoints = AbstractTest::VectorFromJson<VRawSAPoint>(input);
        QVector<QPointF> const outputPoints = AbstractTest::VectorFromJson<QPointF>(output);
        QTest::newRow(title) << inputPoints << outputPoints;
    };

    // See file src/app/share/collection/bugs/smart_pattern_#112.val (private collection)
    ASSERT_TEST_CASE("Loop intersection", QStringLiteral("://smart_pattern_#112/input.json"),
                     QStringLiteral("://smart_pattern_#112/output.json"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::RawPathRemoveLoop() const
{
    QFETCH(QVector<VRawSAPoint>, path);
    QFETCH(QVector<QPointF>, expect);

    QVector<QPointF> res;
    CastTo(VAbstractPiece::CheckLoops(path), res);
    ComparePaths(res, expect);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::PathRemoveLoop_data() const
{
    QTest::addColumn<QVector<QPointF>>("path");
    QTest::addColumn<QVector<QPointF>>("expect");

    QVector<QPointF> path;
    path << QPointF(10, 10);
    path << QPointF(20, 10);
    path << QPointF(20, 20);
    path << QPointF(10, 20);
    path << QPointF(10, 10);
    QTest::newRow("Correct closed a path (four unique points)") << path << path;

    path.removeLast();

    QTest::newRow("Correct unclosed a path (four unique points)") << path << path;

    path.clear();
    path << QPointF(0, 10);
    path << QPointF(10, 10);
    path << QPointF(20, 10);
    path << QPointF(20, 20);
    path << QPointF(10, 20);
    path << QPointF(0, 20);
    path << QPointF(0, 10);
    QTest::newRow("Correct closed a path (six unique points)") << path << path;

    path.removeLast();

    QTest::newRow("Correct unclosed a path (six unique points)") << path << path;

    path.clear();
    path << QPointF(20, 10);
    path << QPointF(20, 20);
    path << QPointF(10, 10);
    path << QPointF(10, 20);
    path << QPointF(20, 10);

    QVector<QPointF> res;
    res << QPointF(20, 10);
    res << QPointF(20, 20);
    res << QPointF(15, 15);
    res << QPointF(20, 10);
    QTest::newRow("One loop, closed a path (four unique points)") << path << res;

    path.clear();
    path << QPointF(20, 10);
    path << QPointF(20, 20);
    path << QPointF(10, 10);
    path << QPointF(0, 20);
    path << QPointF(0, 10);
    path << QPointF(10, 20);
    path << QPointF(20, 10);

    res.clear();
    res << QPointF(20, 10);
    res << QPointF(20, 20);
    res << QPointF(15, 15);
    res << QPointF(20, 10);
    QTest::newRow("Two loops, closed a path (six unique points)") << path << res;

    path.clear();
    path << QPointF(20, 10);
    path << QPointF(20, 20);
    path << QPointF(10, 10);
    path << QPointF(0, 10);
    path << QPointF(0, 20);
    path << QPointF(10, 20);
    path << QPointF(20, 10);

    res.clear();
    res << QPointF(20, 10);
    res << QPointF(20, 20);
    res << QPointF(15, 15);
    res << QPointF(20, 10);
    QTest::newRow("One loop, the first loop, closed a path (six unique points)") << path << res;

    path.clear();
    path << QPointF(20, 10);
    path << QPointF(20, 20);
    path << QPointF(10, 20);
    path << QPointF(0, 10);
    path << QPointF(0, 20);
    path << QPointF(10, 10);
    path << QPointF(20, 10);

    res.clear();
    res << QPointF(20, 10);
    res << QPointF(20, 20);
    res << QPointF(10, 20);
    res << QPointF(5, 15);
    res << QPointF(10, 10);
    res << QPointF(20, 10);
    QTest::newRow("One loop, the second loop, closed a path (six unique points)") << path << res;

    path.removeLast();
    res.removeLast();

    QTest::newRow("One loop, the second loop, unclosed a path (six unique points)") << path << res;

    path.clear();
    path << QPointF(100.96979100571033, 1797.6153764073072);
    path << QPointF(168.3888427659865, 1807.2395034187866);
    path << QPointF(206.78076137364403, 1812.2910842036706);
    path << QPointF(239.1630793382262, 1815.951361623424);
    path << QPointF(267.5320085054171, 1818.4827543754482);
    path << QPointF(293.9502505847841, 1820.144031725603);
    path << QPointF(320.48133946750147, 1821.175819320443);
    path << QPointF(364.5960626489172, 1822.0507669842166);
    path << QPointF(400.66867742260206, 1822.488188976378);
    path << QPointF(623.3126833308274, 1822.488188976378);
    path << QPointF(653.5489038032683, 2162.6456692913384);
    path << QPointF(570.545584385708, 2162.6456692913384);
    path << QPointF(600.7818048581489, 1822.488188976378);
    path << QPointF(1001.3385826771654, 1822.488188976378);
    path << QPointF(1001.3385826771654, 2680.44094488189);
    path << QPointF(-22.11646613738226, 2680.44094488189);
    path << QPointF(100.96979100571033, 1797.6153764073072);

    res.clear();
    res << QPointF(100.96979100571033, 1797.6153764073072);
    res << QPointF(168.3888427659865, 1807.2395034187866);
    res << QPointF(206.78076137364403, 1812.2910842036706);
    res << QPointF(239.1630793382262, 1815.951361623424);
    res << QPointF(267.5320085054171, 1818.4827543754482);
    res << QPointF(293.9502505847841, 1820.144031725603);
    res << QPointF(320.48133946750147, 1821.175819320443);
    res << QPointF(364.5960626489172, 1822.0507669842166);
    res << QPointF(400.66867742260206, 1822.488188976378);
    res << QPointF(1001.3385826771654, 1822.488188976378);
    res << QPointF(1001.3385826771654, 2680.44094488189);
    res << QPointF(-22.11646613738226, 2680.44094488189);
    res << QPointF(100.96979100571033, 1797.6153764073072);

    // See the file "collection/bugs/Issue_#493.val"
    QTest::newRow("Test case issue #493") << path << res;

    path.clear();
    path << QPointF(-685.2149804319953, -3568.7982439212556);
    path << QPointF(-700.7415523087261, -3623.900571239949);
    path << QPointF(-675.4694480627154, -3639.3631430823175);
    path << QPointF(-684.7497934439581, -3631.3546395862268);
    path << QPointF(-683.1356602239256, -3633.2868478418427);
    path << QPointF(-686.8764821039574, -3627.927414863926);
    path << QPointF(-684.7670104817863, -3631.587853202178);
    path << QPointF(-682.2386030572435, -3636.8469922361573);
    path << QPointF(-676.4708011186385, -3650.307478525872);
    path << QPointF(-666.3050989871189, -3676.5286567894937);
    path << QPointF(-654.0449409043066, -3710.198553447806);
    path << QPointF(-640.1333287371614, -3750.0101920374505);
    path << QPointF(-617.0729873733014, -3818.3303697354913);
    path << QPointF(-583.8128392515604, -3920.9726624886944);
    path << QPointF(-550.5307668482033, -4027.6970214479597);
    path << QPointF(-527.4164674104215, -4104.7034088569535);
    path << QPointF(-513.4302533332675, -4152.73879565781);
    path << QPointF(-501.0373006826446, -4196.767296675345);
    path << QPointF(-490.59311078227046, -4235.660899517831);
    path << QPointF(-477.25724163384456, -4288.293444470835);
    path << QPointF(-405.3839593893572, -4272.013803282615);
    path << QPointF(-545.9786893428341, -3568.830152982464);
    path << QPointF(-685.2149804319953, -3568.7982439212556);

    res.clear();
    res << QPointF(-685.2149804319953, -3568.7982439212556);
    res << QPointF(-700.7415523087261, -3623.900571239949);
    res << QPointF(-683.3457668881176, -3634.5440688767967);
    res << QPointF(-682.2386030572435, -3636.8469922361573);
    res << QPointF(-676.4708011186385, -3650.307478525872);
    res << QPointF(-666.3050989871189, -3676.5286567894937);
    res << QPointF(-654.0449409043066, -3710.198553447806);
    res << QPointF(-640.1333287371614, -3750.0101920374505);
    res << QPointF(-617.0729873733014, -3818.3303697354913);
    res << QPointF(-583.8128392515604, -3920.9726624886944);
    res << QPointF(-550.5307668482033, -4027.6970214479597);
    res << QPointF(-527.4164674104215, -4104.7034088569535);
    res << QPointF(-513.4302533332675, -4152.73879565781);
    res << QPointF(-501.0373006826446, -4196.767296675345);
    res << QPointF(-490.59311078227046, -4235.660899517831);
    res << QPointF(-477.25724163384456, -4288.293444470835);
    res << QPointF(-405.3839593893572, -4272.013803282615);
    res << QPointF(-545.9786893428341, -3568.830152982464);
    res << QPointF(-685.2149804319953, -3568.7982439212556);

    // See the file "collection/bugs/Issue_#515.val"
    // Check a seam allowance path.
    // The curve that causes the issue is the first in the list.
    QTest::newRow("Test case issue #515. Big loop in seam allowance path.") << path << res;

    path.clear();
    path << QPointF(-449.6699112298347, -4243.2921010175705);
    path << QPointF(-576.966638263205, -3606.6183279948636);
    path << QPointF(-656.9465284876832, -3606.6183279948636);
    path << QPointF(-656.5996104603414, -3606.6000783462687);
    path << QPointF(-655.7439133016985, -3607.1236310612317);
    path << QPointF(-654.129780081666, -3609.0558393168476);
    path << QPointF(-651.3154902471701, -3613.939306009108);
    path << QPointF(-647.8207651830382, -3621.2084054506768);
    path << QPointF(-641.4701586077349, -3636.0289997859454);
    path << QPointF(-630.9244502073004, -3663.23035747934);
    path << QPointF(-618.4465305467888, -3697.4982896415795);
    path << QPointF(-604.3873016966293, -3737.732371148936);
    path << QPointF(-581.1891087215608, -3806.460957656939);
    path << QPointF(-547.7936207285052, -3909.520915257629);
    path << QPointF(-514.3891332445846, -4016.6378180116963);
    path << QPointF(-491.17181635142833, -4093.9874129706236);
    path << QPointF(-477.094588519539, -4142.335384784734);
    path << QPointF(-464.5941701318652, -4186.745679830414);
    path << QPointF(-454.0214632588362, -4226.117872983938);

    res.clear();
    res << QPointF(-449.6699112298347, -4243.2921010175705);
    res << QPointF(-576.966638263205, -3606.6183279948636);
    res << QPointF(-656.5697831440032, -3606.6183279948636);
    res << QPointF(-655.7439133016985, -3607.1236310612317);
    res << QPointF(-654.129780081666, -3609.0558393168476);
    res << QPointF(-651.3154902471701, -3613.939306009108);
    res << QPointF(-647.8207651830382, -3621.2084054506768);
    res << QPointF(-641.4701586077349, -3636.0289997859454);
    res << QPointF(-630.9244502073004, -3663.23035747934);
    res << QPointF(-618.4465305467888, -3697.4982896415795);
    res << QPointF(-604.3873016966293, -3737.732371148936);
    res << QPointF(-581.1891087215608, -3806.460957656939);
    res << QPointF(-547.7936207285052, -3909.520915257629);
    res << QPointF(-514.3891332445846, -4016.6378180116963);
    res << QPointF(-491.17181635142833, -4093.9874129706236);
    res << QPointF(-477.094588519539, -4142.335384784734);
    res << QPointF(-464.5941701318652, -4186.745679830414);
    res << QPointF(-454.0214632588362, -4226.117872983938);

    // See the file "collection/bugs/Issue_#515.val"
    // Check a seam allowance path.
    // The curve that causes the issue is the last in the list.
    QTest::newRow("Test case issue #515. Small loop in seam allowance path.") << path << res;

    path.clear();
    path << QPointF(1229.6503937007876, 937.6667716535435);
    path << QPointF(203.08931117793543, 937.6667716535435);
    path << QPointF(459.7677349767701, -2166.704563141019);
    path << QPointF(1229.6503937007876, -1990.077167189857);
    path << QPointF(1229.6503937007876, -555.2466141732282);
    path << QPointF(920.1053824527112, -555.2466141732282);
    path << QPointF(887.034516310979, -63.90803149606281);
    path << QPointF(816.3607592795726, -63.908031496062826);
    path << QPointF(780.7580397937137, -592.8627210002539);
    path << QPointF(816.0241340748559, -1202.917917917055);
    path << QPointF(887.3711415156957, -1202.917917917055);
    path << QPointF(920.4420076574283, -630.8371653543306);
    path << QPointF(1229.6503937007876, -630.8371653543306);
    path << QPointF(1229.6503937007876, 937.6667716535435);

    res.clear();
    res << QPointF(1229.6503937007876, 937.6667716535435);
    res << QPointF(203.08931117793543, 937.6667716535435);
    res << QPointF(459.7677349767702, -2166.704563141019);
    res << QPointF(1229.6503937007876, -1990.077167189857);
    res << QPointF(1229.6503937007876, 937.6667716535435);

    // See the file "collection/bugs/Issue_#603.val"
    // Point H1 is first in the list
    QTest::newRow("Test issue 603.") << path << res;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::PathRemoveLoop() const
{
    QFETCH(QVector<QPointF>, path);
    QFETCH(QVector<QPointF>, expect);

    QVector<QPointF> const res = VAbstractPiece::CheckLoops(path);
    ComparePaths(res, expect);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::PathLoopsCase_data() const
{
    QTest::addColumn<QVector<QPointF>>("path");
    QTest::addColumn<QVector<QPointF>>("expect");

    QVector<QPointF> path;
    path << QPointF(61.86670866141733, 446.92270866141735);
    path << QPointF(650.6504606788366, 473.2192016666484);
    path << QPointF(649.4426552757304, 480.5376973511262);
    path << QPointF(646.5769170924987, 501.9977838630714);
    path << QPointF(644.6382908004568, 523.6358081043691);
    path << QPointF(643.4592698551749, 551.9888717674471);
    path << QPointF(642.9134698671897, 584.1776423714557);
    path << QPointF(643.1914832622404, 613.2382010061506);
    path << QPointF(644.2199668178571, 639.3780275889782);
    path << QPointF(645.9255773115714, 662.8046020373845);
    path << QPointF(648.2349715209137, 683.7254042688159);
    path << QPointF(651.0748062234152, 702.3479142007185);
    path << QPointF(654.3717381966065, 718.8796117505387);
    path << QPointF(658.0524242180187, 733.5279768357226);
    path << QPointF(662.0435210651824, 746.5004893737165);
    path << QPointF(666.2716855156286, 758.0046292819667);
    path << QPointF(670.6635743468883, 768.2478764779191);
    path << QPointF(677.400406718071, 781.7952098705392);
    path << QPointF(691.6740007010135, 806.2608114022295);
    path << QPointF(694.5877745571677, 810.2150054671212);
    path << QPointF(699.9560352035193, 816.1706553153153);
    path << QPointF(708.9007628091615, 824.0594196166176);
    path << QPointF(719.3794725391945, 831.7499791040799);
    path << QPointF(730.9568541500198, 839.0942359684872);
    path << QPointF(743.1975973980386, 845.9440924006244);
    path << QPointF(755.6663920396528, 852.1514505912763);
    path << QPointF(767.9279278312633, 857.568212731228);
    path << QPointF(779.5468945292718, 862.046281011264);
    path << QPointF(790.0879818900794, 865.4375576221694);
    path << QPointF(799.115879670088, 867.5939447547289);
    path << QPointF(804.5608128209333, 868.2650594004886);
    path << QPointF(807.5317661719646, 868.2782441618697);
    path << QPointF(809.8795601157717, 867.8994015359809);
    path << QPointF(811.5497808719051, 867.1100192966705);
    path << QPointF(812.4880146599148, 865.8915852177861);
    path << QPointF(812.6398476993509, 864.2255870731761);
    path << QPointF(811.9508662097637, 862.0935126366886);
    path << QPointF(810.3666564107034, 859.4768496821717);
    path << QPointF(806.3216663321919, 854.66911491981);
    path << QPointF(802.0871811023624, 850.6707401574804);
    path << QPointF(799.4598981526765, 850.6707401574804);
    path << QPointF(802.0871811023624, 1653.9337322834645);
    path << QPointF(61.86670866141733, 1653.9337322834645);

    QVector<QPointF> res;
    res << QPointF(61.86670866141733, 446.92270866141735);
    res << QPointF(650.6504606788366, 473.2192016666484);
    res << QPointF(649.4426552757304, 480.5376973511262);
    res << QPointF(646.5769170924987, 501.9977838630714);
    res << QPointF(644.6382908004568, 523.6358081043691);
    res << QPointF(643.4592698551749, 551.9888717674471);
    res << QPointF(642.9134698671897, 584.1776423714557);
    res << QPointF(643.1914832622404, 613.2382010061506);
    res << QPointF(644.2199668178571, 639.3780275889782);
    res << QPointF(645.9255773115714, 662.8046020373845);
    res << QPointF(648.2349715209137, 683.7254042688159);
    res << QPointF(651.0748062234152, 702.3479142007185);
    res << QPointF(654.3717381966065, 718.8796117505387);
    res << QPointF(658.0524242180187, 733.5279768357226);
    res << QPointF(662.0435210651824, 746.5004893737165);
    res << QPointF(666.2716855156286, 758.0046292819667);
    res << QPointF(670.6635743468883, 768.2478764779191);
    res << QPointF(677.400406718071, 781.7952098705392);
    res << QPointF(691.6740007010135, 806.2608114022295);
    res << QPointF(694.5877745571677, 810.2150054671212);
    res << QPointF(699.9560352035193, 816.1706553153153);
    res << QPointF(708.9007628091615, 824.0594196166176);
    res << QPointF(719.3794725391945, 831.7499791040799);
    res << QPointF(730.9568541500198, 839.0942359684872);
    res << QPointF(743.1975973980386, 845.9440924006244);
    res << QPointF(755.6663920396528, 852.1514505912763);
    res << QPointF(767.9279278312633, 857.568212731228);
    res << QPointF(779.5468945292718, 862.046281011264);
    res << QPointF(790.0879818900794, 865.4375576221694);
    res << QPointF(799.115879670088, 867.5939447547289);
    res << QPointF(799.5154110117976, 867.6431889469776);
    res << QPointF(802.0871811023624, 1653.9337322834645);
    res << QPointF(61.86670866141733, 1653.9337322834645);

    // See file "collection/bugs/Issue_#609_case1.val"
    // Clear a main path. Bound intersection. External loop. Outside a loop. Start point Ф1.
    QTest::newRow("Issue 609. Case1a") << path << res;

    path.clear();
    path << QPointF(-365.68188649000314, -2143.126579528016);
    path << QPointF(-195.75487873249062, -2116.7935769656237);
    path << QPointF(-195.75487873249062, -1836.0319480765759);
    path << QPointF(-233.39027086052477, -1838.4849618976993);
    path << QPointF(-231.15080237392075, -1855.5915146519483);
    path << QPointF(-225.84473077299972, -1889.4811404382626);
    path << QPointF(-219.39861487985402, -1922.986407729537);
    path << QPointF(-211.6695159016421, -1955.9990283342697);
    path << QPointF(-204.87723909172885, -1980.439660924953);
    path << QPointF(-199.87970909142098, -1996.6270828437923);
    path << QPointF(-194.48099536000245, -2012.6451713592935);
    path << QPointF(-188.65032933731845, -2028.5246588116781);
    path << QPointF(-182.36812965707693, -2044.2602109802488);
    path << QPointF(-175.61499879935675, -2059.8462252736344);
    path << QPointF(-168.3717693169516, -2075.2768492268588);
    path << QPointF(-160.6424572210866, -2090.5008865466684);
    path << QPointF(-150.22847685877994, -2109.7385074212525);
    path << QPointF(194.23861004296444, -2056.3576305273214);
    path << QPointF(302.4787663409577, -1301.003761061316);
    path << QPointF(279.86810151275455, -1288.330749878147);
    path << QPointF(-641.7062267185897, -2051.118466118487);
    path << QPointF(-365.68188649000314, -2143.126579528016);

    res.clear();
    res << QPointF(-365.68188649000314, -2143.126579528016);
    res << QPointF(-195.75487873249062, -2116.7935769656237);
    res << QPointF(-195.75487873249062, -2008.8655346469059);
    res << QPointF(-194.48099536000245, -2012.6451713592935);
    res << QPointF(-188.65032933731845, -2028.5246588116781);
    res << QPointF(-182.36812965707693, -2044.2602109802488);
    res << QPointF(-175.61499879935675, -2059.8462252736344);
    res << QPointF(-168.3717693169516, -2075.2768492268588);
    res << QPointF(-160.6424572210866, -2090.5008865466684);
    res << QPointF(-150.22847685877994, -2109.7385074212525);
    res << QPointF(194.23861004296444, -2056.3576305273214);
    res << QPointF(302.4787663409577, -1301.003761061316);
    res << QPointF(279.86810151275455, -1288.330749878147);
    res << QPointF(-641.7062267185897, -2051.118466118487);
    res << QPointF(-365.68188649000314, -2143.126579528016);

    // See file "collection/bugs/Issue_#609_case2.val"
    // Clear an equdistant. Bound intersection. Internal loop. Outside a loop. Start point А2.
    QTest::newRow("Issue 609. Case2b") << path << res;

    path.clear();
    path << QPointF(0, 10);
    path << QPointF(5, 10);
    path << QPointF(7.5, 15);
    path << QPointF(2.5, 15);
    path << QPointF(5, 10);
    path << QPointF(10, 10);
    path << QPointF(10, 20);
    path << QPointF(0, 20);
    path << QPointF(0, 10);

    res.clear();
    res << QPointF(0, 10);
    res << QPointF(10, 10);
    res << QPointF(10, 20);
    res << QPointF(0, 20);
    res << QPointF(0, 10);

    QTest::newRow("Internal loop. Invalid case.") << path << res;

    path.clear();
    path << QPointF(0, 10);
    path << QPointF(5, 10);
    path << QPointF(10, 0);
    path << QPointF(0, 0);
    path << QPointF(5, 10);
    path << QPointF(10, 10);
    path << QPointF(10, 20);
    path << QPointF(0, 20);
    path << QPointF(0, 10);

    res.clear();
    res << QPointF(0, 10);
    res << QPointF(10, 10);
    res << QPointF(10, 20);
    res << QPointF(0, 20);
    res << QPointF(0, 10);

    QTest::newRow("External loop. Invalid case.") << path << res;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::PathLoopsCase() const
{
    QFETCH(QVector<QPointF>, path);
    QFETCH(QVector<QPointF>, expect);

    const QVector<QPointF> res = VAbstractPiece::CheckLoops(path);
    ComparePaths(res, expect);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::BrokenDetailEquidistant_data()
{
    QTest::addColumn<QVector<VSAPoint>>("points");
    QTest::addColumn<qreal>("width");
    QTest::addColumn<QVector<QPointF>>("ekvOrig");

    auto ASSERT_TEST_CASE = [](const char *title, const QString &input, const QString &output, qreal width)
    {
        QVector<VSAPoint> const inputPoints = AbstractTest::VectorFromJson<VSAPoint>(input);
        QVector<QPointF> const outputPoints = AbstractTest::VectorFromJson<QPointF>(output);
        QTest::newRow(title) << inputPoints << width << outputPoints;
    };

    // See the file "collection/bugs/Issue_#604.val" (since 0.5.0)
    ASSERT_TEST_CASE("Issue #604.", QStringLiteral("://Issue_604/input.json"),
                     QStringLiteral("://Issue_604/output.json"), 11.338582677165354 /*seam allowance width*/);

    // See the file "collection/bugs/Issue_#627.val"
    ASSERT_TEST_CASE("Issue #627.", QStringLiteral("://Issue_627/input.json"),
                     QStringLiteral("://Issue_627/output.json"), 56.692913385826778 /*seam allowance width*/);

    // See the file "collection/bugs/Issue_#687.val"
    ASSERT_TEST_CASE("Issue #687.", QStringLiteral("://Issue_687/input.json"),
                     QStringLiteral("://Issue_687/output.json"), 37.795275590551185 /*seam allowance width*/);

    // See private test cases in file jacket_issue_#767.val, piece "Fabric_TopCollar"
    // Curve approximation scale 0.5
    ASSERT_TEST_CASE("Issue #767. Fabric_TopCollar.", QStringLiteral("://Issue_767_Fabric_TopCollar/input.json"),
                     QStringLiteral("://Issue_767_Fabric_TopCollar/output.json"),
                     37.795275590551185 /*seam allowance width*/);

    // See private test cases in file jacket_issue_#767.val, piece "SkinFusing_TopCollar_Notch"
    // Curve approximation scale 0.5
    ASSERT_TEST_CASE("Issue #767. SkinFusing_TopCollar_Notch.",
                     QStringLiteral("://Issue_767_Fabric_SkinFusing_TopCollar_Notch/input.json"),
                     QStringLiteral("://Issue_767_Fabric_SkinFusing_TopCollar_Notch/output.json"),
                     37.795275590551185 /*seam allowance width*/);

    // See private test cases in file collection/bugs/Issue_#883_case1.val, piece "ledge"
    ASSERT_TEST_CASE("Issue #883. Piece 'ledge'", QStringLiteral("://Issue_883_ledge/input.json"),
                     QStringLiteral("://Issue_883_ledge/output.json"), 37.795275590551185 /*seam allowance width*/);

    // See private test cases in file collection/bugs/Issue_#883_case1.val, piece "prong"
    ASSERT_TEST_CASE("Issue #883. Piece 'prong'", QStringLiteral("://Issue_883_prong/input.json"),
                     QStringLiteral("://Issue_883_prong/output.json"), 37.795275590551185 /*seam allowance width*/);

    // See the file "collection/bugs/smart_pattern_#145.val"
    ASSERT_TEST_CASE("Issue #145", QStringLiteral("://smart_pattern_#145/input.json"),
                     QStringLiteral("://smart_pattern_#145/output.json"), 37.795275590551185 /*seam allowance width*/);

    // See private test cases in file valentina_private_collection/bugs/women_jacket/women_jacket.val
    // Point A38, size 32
    ASSERT_TEST_CASE("women_jacket_A38", QStringLiteral("://women_jacket/input.json"),
                     QStringLiteral("://women_jacket/output.json"), 37.795275590551185 /*seam allowance width*/);

    // See private test cases in file valentina_private_collection/bugs/hood_1/hood_1.val
    ASSERT_TEST_CASE("hood_1", QStringLiteral("://hood_1/input.json"), QStringLiteral("://hood_1/output.json"),
                     37.795275590551185 /*seam allowance width*/);

    // See private test cases in file valentina_private_collection/bugs/hood_2/hood_2.val
    ASSERT_TEST_CASE("hood_2", QStringLiteral("://hood_2/input.json"), QStringLiteral("://hood_2/output.json"),
                     37.795275590551185 /*seam allowance width*/);

    // See the file "collection/bugs/smart_pattern_#184_case1.val"
    ASSERT_TEST_CASE("Issue #184 case 1", QStringLiteral("://smart_pattern_#184_case1/input.json"),
                     QStringLiteral("://smart_pattern_#184_case1/output.json"),
                     0.88157480314960635 /*seam allowance width*/);

    // See the file "collection/bugs/smart_pattern_#184_case2.val"
    ASSERT_TEST_CASE("Issue #184 case 2", QStringLiteral("://smart_pattern_#184_case2/input.json"),
                     QStringLiteral("://smart_pattern_#184_case2/output.json"),
                     37.795275590551185 /*seam allowance width*/);

    // See the file "valentina_private_collection/bugs/custom_seam_allwance_exclude/custom_seam_allwance_exclude.val"
    ASSERT_TEST_CASE("Piece 1. CSA Exclude", QStringLiteral("://custom_seam_allwance_exclude_p1/input.json"),
                     QStringLiteral("://custom_seam_allwance_exclude_p1/output.json"),
                     37.795275590551185 /*seam allowance width 1 cm*/);

    // See the file "valentina_private_collection/bugs/custom_seam_allwance_exclude/custom_seam_allwance_exclude.val"
    ASSERT_TEST_CASE("Piece 2. CSA Exclude", QStringLiteral("://custom_seam_allwance_exclude_p2/input.json"),
                     QStringLiteral("://custom_seam_allwance_exclude_p2/output.json"),
                     37.795275590551185 /*seam allowance width 1 cm*/);

    // See the file "valentina_private_collection/bugs/25L Knitting Bag.val"
    ASSERT_TEST_CASE("Base", QStringLiteral("://25L_Knitting_Bag/input.json"),
                     QStringLiteral("://25L_Knitting_Bag/output.json"), 36 /*seam allowance width*/);

    // See the file "valentina_private_collection/bugs/prong/650b.val"
    ASSERT_TEST_CASE("Coat", QStringLiteral("://prong/input.json"), QStringLiteral("://prong/output.json"),
                     37.795275590551185 /*seam allowance width*/);

    // See the file "collection/bugs/prong_acute_angle.val"
    ASSERT_TEST_CASE("Coat", QStringLiteral("://prong_acute_angle/input.json"),
                     QStringLiteral("://prong_acute_angle/output.json"), 37.795275590551185 /*seam allowance width*/);

    // See the file "valentina_private_collection/bugs/Trousers Block/1.228 Men's Trousers Block.val"
    // X:B180 Y:B6
    ASSERT_TEST_CASE("Big Poc interfac",
                     QStringLiteral("://trousers_block/input.json"),
                     QStringLiteral("://trousers_block/output.json"),
                     37.795275590551185 /*seam allowance width*/);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::BrokenDetailEquidistant() const
{
    QFETCH(QVector<VSAPoint>, points);
    QFETCH(qreal, width);
    QFETCH(QVector<QPointF>, ekvOrig);

    QVector<QPointF> ekv;
    CastTo(VAbstractPiece::Equidistant(points, width, QString()), ekv); // Take result

    // Begin comparison
    ComparePaths(ekv, ekvOrig);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::EquidistantAngleType_data()
{
    QTest::addColumn<QVector<VSAPoint>>("points");
    QTest::addColumn<qreal>("width");
    QTest::addColumn<QVector<QPointF>>("ekvOrig");

    auto ASSERT_TEST_CASE = [](const char *title, const QString &input, const QString &output, qreal width)
    {
        QVector<VSAPoint> const inputPoints = AbstractTest::VectorFromJson<VSAPoint>(input);
        QVector<QPointF> const outputPoints = AbstractTest::VectorFromJson<QPointF>(output);
        QTest::newRow(title) << inputPoints << width << outputPoints;
    };

    // See the file "collection/bugs/Issue_#880.val"
    ASSERT_TEST_CASE("Issue #880. Piece: Detail.", QStringLiteral("://Issue_880_Detail/input.json"),
                     QStringLiteral("://Issue_880_Detail/output.json"), 37.795275590551185 /*seam allowance width*/);

    // See the file "collection/bugs/Issue_#880.val"
    ASSERT_TEST_CASE("Issue #880. Piece: Detail_1.", QStringLiteral("://Issue_880_Detail_1/input.json"),
                     QStringLiteral("://Issue_880_Detail_1/output.json"), 75.59055118110237 /*seam allowance width*/);

    // See file src/app/share/collection/bugs/Issue_#937.val
    ASSERT_TEST_CASE("Issue 937. Case1", QStringLiteral("://Issue_937_case_1/input.json"),
                     QStringLiteral("://Issue_937_case_1/output.json"), 37.795275590551185 /*seam allowance width*/);

    // See file src/app/share/collection/bugs/Issue_#937a.val
    ASSERT_TEST_CASE("Issue 937. Case1a", QStringLiteral("://Issue_937_case_1a/input.json"),
                     QStringLiteral("://Issue_937_case_1a/output.json"), 37.795275590551185 /*seam allowance width*/);

    // See file src/app/share/collection/bugs/Issue_#937.val
    ASSERT_TEST_CASE("Issue 937. Case2", QStringLiteral("://Issue_937_case_2/input.json"),
                     QStringLiteral("://Issue_937_case_2/output.json"), 37.795275590551185 /*seam allowance width*/);

    // See file src/app/share/collection/bugs/Issue_#937.val
    ASSERT_TEST_CASE("Issue 937. Case3", QStringLiteral("://Issue_937_case_3/input.json"),
                     QStringLiteral("://Issue_937_case_3/output.json"), 37.795275590551185 /*seam allowance width*/);

    // See file src/app/share/collection/bugs/Issue_#937.val
    ASSERT_TEST_CASE("Issue 937. Case4", QStringLiteral("://Issue_937_case_4/input.json"),
                     QStringLiteral("://Issue_937_case_4/output.json"), 37.795275590551185 /*seam allowance width*/);

    // See file src/app/share/collection/bugs/Issue_#937_case5.val
    ASSERT_TEST_CASE("Issue 937. Case5", QStringLiteral("://Issue_937_case_5/input.json"),
                     QStringLiteral("://Issue_937_case_5/output.json"), 37.795275590551185 /*seam allowance width*/);

    // See file valentina_private_collection/bugs/busty/busty.val
    ASSERT_TEST_CASE("Busty", QStringLiteral("://busty/input.json"), QStringLiteral("://busty/output.json"),
                     0 /*seam allowance width*/);

    // See file valentina_private_collection/bugs/panties/panties.val
    ASSERT_TEST_CASE("Panties. Piece Case 1", QStringLiteral("://panties_case1/input.json"),
                     QStringLiteral("://panties_case1/output.json"), 26.45669291338583 /*seam allowance width*/);

    // See file valentina_private_collection/bugs/panties/panties.val
    ASSERT_TEST_CASE("Panties. Piece Case 2", QStringLiteral("://panties_case2/input.json"),
                     QStringLiteral("://panties_case2/output.json"), 26.45669291338583 /*seam allowance width*/);

    // See file valentina_private_collection/bugs/smart_pattern_#113/smart_pattern_#113.val
    ASSERT_TEST_CASE("Эдит 6", QStringLiteral("://smart_pattern_#113/input.json"),
                     QStringLiteral("://smart_pattern_#113/output.json"), 0.56692913385826771 /*seam allowance width*/);

    // See file valentina_private_collection/bugs/smart_pattern_#118/smart_pattern_#118.val
    ASSERT_TEST_CASE("Ретро стринги 3", QStringLiteral("://smart_pattern_#118/input.json"),
                     QStringLiteral("://smart_pattern_#118/output.json"), 26.45669291338583 /*seam allowance width*/);

    // See file valentina_private_collection/bugs/smart_pattern_#133/smart_pattern_#133.val
    ASSERT_TEST_CASE("Эдит 1", QStringLiteral("://smart_pattern_#133/input.json"),
                     QStringLiteral("://smart_pattern_#133/output.json"), 26.45669291338583 /*seam allowance width*/);

    // See file valentina_private_collection/bugs/hat/hat.val
    ASSERT_TEST_CASE("hat", QStringLiteral("://hat/input.json"), QStringLiteral("://hat/output.json"),
                     26.45669291338583 /*seam allowance width*/);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::EquidistantAngleType() const
{
    QFETCH(QVector<VSAPoint>, points);
    QFETCH(qreal, width);
    QFETCH(QVector<QPointF>, ekvOrig);

    QVector<QPointF> ekv;
    CastTo(VAbstractPiece::Equidistant(points, width, QString()), ekv); // Take result

    // Begin comparison
    ComparePaths(ekv, ekvOrig);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::CorrectEquidistantPoints_data()
{
    QTest::addColumn<QVector<QPointF>>("points");
    QTest::addColumn<QVector<QPointF>>("expect");
    QTest::addColumn<bool>("removeFirstAndLast");

    QVector<QPointF> points;
    points.append(QPointF(-741.7894588053705, 1065.7336503858917));
    points.append(QPointF(-759.696551643576, -115.81420543069257));
    points.append(QPointF(-278.17249953019325, -217.1037453126913));
    points.append(QPointF(-244.64654130659474, 1077.9548221866635));
    points.append(QPointF(-741.7894588053705, 1065.7336503858917));

    QVector<QPointF> expect;
    expect.append(QPointF(-741.7894588053705, 1065.7336503858917));
    expect.append(QPointF(-759.696551643576, -115.81420543069257));
    expect.append(QPointF(-278.17249953019325, -217.1037453126913));
    expect.append(QPointF(-244.64654130659474, 1077.9548221866635));
    expect.append(QPointF(-741.7894588053705, 1065.7336503858917));

    QTest::newRow("Closed seam allowance. Last point equal first.") << points << expect << false;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::CorrectEquidistantPoints() const
{
    QFETCH(QVector<QPointF>, points);
    QFETCH(QVector<QPointF>, expect);
    QFETCH(bool, removeFirstAndLast);

    const QVector<QPointF> res = VAbstractPiece::CorrectEquidistantPoints(points, removeFirstAndLast);

    // Begin comparison
    ComparePaths(res, expect);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::TestCorrectEquidistantPoints_data()
{
    QTest::addColumn<QVector<QPointF>>("before");
    QTest::addColumn<QVector<QPointF>>("expect");

    auto ASSERT_TEST_CASE = [](const char *title, const QString &input, const QString &output)
    {
        QVector<QPointF> const inputPoints = AbstractTest::VectorFromJson<QPointF>(input);
        QVector<QPointF> const outputPoints = AbstractTest::VectorFromJson<QPointF>(output);
        QTest::newRow(title) << inputPoints << outputPoints;
    };

    // See file src/app/share/collection/bugs/Steampunk_trousers_issue_#548.val
    ASSERT_TEST_CASE("Test case issue #548", QStringLiteral("://Issue_548/input.json"),
                     QStringLiteral("://Issue_548/output.json"));

    // See the file "collection/bugs/Issue_#642.val"
    ASSERT_TEST_CASE("Test case issue #642", QStringLiteral("://Issue_642/input.json"),
                     QStringLiteral("://Issue_642/output.json"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::TestCorrectEquidistantPoints() const
{
    QFETCH(QVector<QPointF>, before);
    QFETCH(QVector<QPointF>, expect);

    QVector<QPointF> const after = VAbstractPiece::CorrectEquidistantPoints(before);
    ComparePaths(after, expect);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::PossibleInfiniteClearLoops_data() const
{
    QTest::addColumn<QVector<QPointF>>("path");
    QTest::addColumn<QVector<QPointF>>("expect");

    QVector<QPointF> path;
    path << QPointF(-632.849625504129, 4084.1574803149606);
    path << QPointF(-1063.1826742777307, 4084.157480314961);
    path << QPointF(-1064.731669053407, 3971.857538662673);
    path << QPointF(-1065.9335577016843, 3768.725259939178);
    path << QPointF(-1066.3064907565738, 3389.730086730216);
    path << QPointF(-1064.2157605861642, 2590.593584914368);
    path << QPointF(-1063.7002274798474, 2353.011824334406);
    path << QPointF(-1063.5730500368766, 2323.3511600791016);
    path << QPointF(-1061.2277890821085, 2265.404475234159);
    path << QPointF(-1056.6870752377622, 2208.99511667452);
    path << QPointF(-1050.3195580618494, 2154.1593979456843);
    path << QPointF(-1042.4881941434103, 2100.7853411906426);
    path << QPointF(-1033.5464923745499, 2048.7106381836265);
    path << QPointF(-1018.9140925348769, 1972.9298208633923);
    path << QPointF(-998.7073870145719, 1875.5283486595902);
    path << QPointF(-984.743852909139, 1804.5663725038373);
    path << QPointF(-976.5700399899772, 1758.126392406043);
    path << QPointF(-969.6857320440495, 1712.1364207718836);
    path << QPointF(-964.4132563582218, 1666.50830864049);
    path << QPointF(-961.0600449902615, 1621.1025089368645);
    path << QPointF(-959.9236450523174, 1575.5823241167095);
    path << QPointF(-960.3877919929658, 1553.2712054750111);
    path << QPointF(-1000.3562458795624, 801.8642724069981);
    path << QPointF(-954.5715133777537, 803.7344759255401);
    path << QPointF(-946.442743379301, 803.5582812808202);
    path << QPointF(-934.1053985018102, 802.2501117597377);
    path << QPointF(-917.288618935481, 798.3566841198939);
    path << QPointF(-900.5071817401116, 792.3119964649148);
    path << QPointF(-883.7147885675736, 784.220662585902);
    path << QPointF(-867.1260743218684, 774.3001167278098);
    path << QPointF(-850.9648599763401, 762.8198974992881);
    path << QPointF(-835.447350962678, 750.0867405201303);
    path << QPointF(-820.7766612220335, 736.4325408317882);
    path << QPointF(-807.1431378915719, 722.2070808806341);
    path << QPointF(-794.7272455389146, 707.7755421404892);
    path << QPointF(-783.7035642522673, 693.5210466193306);
    path << QPointF(-774.2455412061727, 679.8541880091894);
    path << QPointF(-766.5311200750891, 667.2357322468891);
    path << QPointF(-760.7485749482415, 656.23004395692);
    path << QPointF(-756.9262152588784, 647.247947787771);
    path << QPointF(-755.9943940773794, 644.3431344612932);
    path << QPointF(-754.3168933441153, 638.3087537000382);
    path << QPointF(-751.4615751472361, 624.0253762908213);
    path << QPointF(-749.4776362727711, 608.4387853126539);
    path << QPointF(-748.2151911179384, 591.5292381129454);
    path << QPointF(-746.9479910213331, 542.1593888936636);
    path << QPointF(-745.6944329547541, 462.8644227516688);
    path << QPointF(-746.1883646034397, 475.16878663047623);
    path << QPointF(-746.2871460461391, 402.8988101963411);
    path << QPointF(-746.6738371191002, 321.4553283654042);
    path << QPointF(-748.4567648247277, 258.62151802315526);
    path << QPointF(-750.7314894991164, 217.2362191509505);
    path << QPointF(-754.2100293185232, 176.77268647996522);
    path << QPointF(-759.1411805604763, 137.7638902918793);
    path << QPointF(-767.9139385171914, 85.8954360711639);
    path << QPointF(-193.4615393890781, -82.57772819388018);
    path << QPointF(-182.84881484970134, -44.475315462966144);
    path << QPointF(-181.7932001092176, -42.33768474083549);
    path << QPointF(-179.7528132464526, -39.70760839675285);
    path << QPointF(-175.73839974706917, -36.02046568979708);
    path << QPointF(-169.4580785677048, -31.6599263589561);
    path << QPointF(-160.89874469571305, -27.009347306192694);
    path << QPointF(-150.1810493329971, -22.35672237054398);
    path << QPointF(-137.46941549286484, -17.896128595814076);
    path << QPointF(-122.93209920666818, -13.753254741366646);
    path << QPointF(-106.85016673407813, -10.037145814867706);
    path << QPointF(-79.81702769536884, -5.199728384059749);
    path << QPointF(-38.971630241556646, -0.5839317964807336);
    path << QPointF(6.260161746717735, 1.9210182870405856);
    path << QPointF(69.51425695996898, 2.8396504979139623);
    path << QPointF(-7.937007874015748, 1590.5905795945973);
    path << QPointF(-7.937007874015748, 2353.1338582677163);
    path << QPointF(-7.937007874015748, 4084.157480314961);
    path << QPointF(-335.49606299212604, 4084.157480314961);
    path << QPointF(-335.49606299212604, 2116.117122071213);
    path << QPointF(-260.2892300303194, 1589.6692913385828);
    path << QPointF(-297.70078740157464, 1327.7883897398);
    path << QPointF(-335.11234477282926, 1589.6692913385828);
    path << QPointF(-259.9055118110237, 2116.1171220712135);
    path << QPointF(-259.9055118110237, 4084.1574803149606);
    path << QPointF(-708.4401766852314, 4084.157480314961);
    path << QPointF(-708.4401766852314, 2022.3251007369636);
    path << QPointF(-660.76090517769, 1570.7716535433071);
    path << QPointF(-670.6449010946802, 1477.1638385065562);
    path << QPointF(-680.5288970116704, 1570.7716535433071);
    path << QPointF(-632.849625504129, 2022.3251007369636);
    path << QPointF(-632.849625504129, 4084.1574803149606);

    QVector<QPointF> expect;
    expect << QPointF(-632.849625504129, 4084.1574803149606);
    expect << QPointF(-708.4401766852314, 4084.1574803149606);
    expect << QPointF(-708.4401766852314, 2022.3251007369636);
    expect << QPointF(-670.6449010946802, 1664.3794685800578);
    expect << QPointF(-632.849625504129, 2022.3251007369636);
    expect << QPointF(-632.849625504129, 4084.1574803149606);

    // See the file "collection/bugs/possible_inf_loop.val"
    QTest::newRow("Possible infinite loop") << path << expect;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::PossibleInfiniteClearLoops() const
{
    QFETCH(QVector<QPointF>, path);
    QFETCH(QVector<QPointF>, expect);

    QVector<QPointF> const res = VAbstractPiece::CheckLoops(path);
    ComparePaths(res, expect);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::IsAllowanceValid_data() const
{
    QTest::addColumn<QVector<QPointF>>("base");
    QTest::addColumn<QVector<QPointF>>("allowance");
    QTest::addColumn<bool>("valid");

    auto ASSERT_TEST_CASE = [](const char *title, const QString &base, const QString &allowance, bool valid)
    {
        QVector<QPointF> const basePoints = AbstractTest::VectorFromJson<QPointF>(base);
        QVector<QPointF> const allowancePoints = AbstractTest::VectorFromJson<QPointF>(allowance);
        QTest::newRow(title) << basePoints << allowancePoints << valid;
    };

    // See file src/app/share/collection/valid_allowance/case3.val (private collection)
    ASSERT_TEST_CASE("Valid seam allowance. Case 3", QStringLiteral("://valid_allowance_case3/base.json"),
                     QStringLiteral("://valid_allowance_case3/allowance.json"), true);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::IsAllowanceValid() const
{
    QFETCH(QVector<QPointF>, base);
    QFETCH(QVector<QPointF>, allowance);
    QFETCH(bool, valid);

    bool const result = VAbstractPiece::IsAllowanceValid(base, allowance);
    QVERIFY(result == valid);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::TestFullSeamPath_data() const
{
    QTest::addColumn<QVector<VLayoutPoint>>("base");
    QTest::addColumn<QVector<VLayoutPoint>>("fullPath");
    QTest::addColumn<QLineF>("mirrorLine");

    auto ASSERT_TEST_CASE =
        [](const char *title, const QString &base, const QString &fullPath, const QLineF &mirrorLine)
    {
        QVector<VLayoutPoint> const basePoints = AbstractTest::VectorFromJson<VLayoutPoint>(base);
        QVector<VLayoutPoint> const fullPathPoints = AbstractTest::VectorFromJson<VLayoutPoint>(fullPath);
        QTest::newRow(title) << basePoints << fullPathPoints << mirrorLine;
    };

    QLineF mirrorLine(QPointF(785.9055118110236, 417.95262992125987), QPointF(29.9999999999999, 606.9290078740157));

    // See file src/app/share/collection/bugs/fold_line.val
    ASSERT_TEST_CASE("Piece full path. Case 1", QStringLiteral("://full_seam_path_case_1/input.json"),
                     QStringLiteral("://full_seam_path_case_1/output.json"), mirrorLine);

    ASSERT_TEST_CASE("Piece full path. Case 2", QStringLiteral("://full_seam_path_case_2/input.json"),
                     QStringLiteral("://full_seam_path_case_2/output.json"), mirrorLine);

    ASSERT_TEST_CASE("Piece full path. Case 3", QStringLiteral("://full_seam_path_case_3/input.json"),
                     QStringLiteral("://full_seam_path_case_3/output.json"), mirrorLine);

    ASSERT_TEST_CASE("Piece full path. Case 4", QStringLiteral("://full_seam_path_case_4/input.json"),
                     QStringLiteral("://full_seam_path_case_4/output.json"), mirrorLine);

    // See file valentina_private_collection/bugs/full_piece/Basic_Darted_Bodice_Block.val (private collection)
    mirrorLine = QLineF(QPointF(37.79527559055022, 4578.897637795276), QPointF(37.795275590551185, 655.1181102362208));

    ASSERT_TEST_CASE("Basic Darted Bodice Block", QStringLiteral("://full_seam_path_case_5/input.json"),
                     QStringLiteral("://full_seam_path_case_5/output.json"), mirrorLine);

    // See file valentina_private_collection/bugs/blazer_for_women_with_one_button/Blazer for women with one button.val
    // (private collection)
    mirrorLine =
        QLineF(QPointF(-2214.4917699559637, 2232.951572921326), QPointF(-1914.963009612374, 2349.0997891917677));

    ASSERT_TEST_CASE("Blazer for women with one button. Full seam path case 1",
                     QStringLiteral("://full_seam_path_blazer_for_women_with_one_button_case_1/input.json"),
                     QStringLiteral("://full_seam_path_blazer_for_women_with_one_button_case_1/output.json"),
                     mirrorLine);

    ASSERT_TEST_CASE("Blazer for women with one button. Full seam path case 2",
                     QStringLiteral("://full_seam_path_blazer_for_women_with_one_button_case_2/input.json"),
                     QStringLiteral("://full_seam_path_blazer_for_women_with_one_button_case_2/output.json"),
                     mirrorLine);

    ASSERT_TEST_CASE("Blazer for women with one button. Full seam path case 3",
                     QStringLiteral("://full_seam_path_blazer_for_women_with_one_button_case_3/input.json"),
                     QStringLiteral("://full_seam_path_blazer_for_women_with_one_button_case_3/output.json"),
                     mirrorLine);

    ASSERT_TEST_CASE("Blazer for women with one button. Full seam path case 4",
                     QStringLiteral("://full_seam_path_blazer_for_women_with_one_button_case_4/input.json"),
                     QStringLiteral("://full_seam_path_blazer_for_women_with_one_button_case_4/output.json"),
                     mirrorLine);

    // See file valentina_private_collection/bugs/capa_over/capa_over.val (private collection)
    ASSERT_TEST_CASE(
        "Molde", QStringLiteral("://capa_over/input.json"), QStringLiteral("://capa_over/output.json"),
        QLineF(QPointF(30.000000000000068, 101.31401574803158), QPointF(1367.9527559055118, 101.31401574803158)));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::TestFullSeamPath() const
{
    QFETCH(QVector<VLayoutPoint>, base);
    QFETCH(QVector<VLayoutPoint>, fullPath);
    QFETCH(QLineF, mirrorLine);

    QVector<QPointF> res;
    CastTo(VAbstractPiece::FullSeamPath(base, mirrorLine, QStringLiteral("Test")), res);

    QVector<QPointF> expected;
    CastTo(fullPath, expected);

    ComparePaths(res, expected);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::TestFullSeamAllowancePath_data() const
{
    QTest::addColumn<QVector<VLayoutPoint>>("base");
    QTest::addColumn<QVector<VLayoutPoint>>("fullPath");
    QTest::addColumn<QLineF>("mirrorLine");

    auto ASSERT_TEST_CASE =
        [](const char *title, const QString &base, const QString &fullPath, const QLineF &mirrorLine)
    {
        QVector<VLayoutPoint> const basePoints = AbstractTest::VectorFromJson<VLayoutPoint>(base);
        QVector<VLayoutPoint> const fullPathPoints = AbstractTest::VectorFromJson<VLayoutPoint>(fullPath);
        QTest::newRow(title) << basePoints << fullPathPoints << mirrorLine;
    };

    QLineF mirrorLine(QPointF(823.7007874015749, 408.503811023622), QPointF(-7.795275590551569, 616.3778267716535));

    // See file src/app/share/collection/bugs/fold_line.val
    ASSERT_TEST_CASE("Piece full path. Case 1", QStringLiteral("://full_seam_allowance_path_case_1/input.json"),
                     QStringLiteral("://full_seam_allowance_path_case_1/output.json"), mirrorLine);

    ASSERT_TEST_CASE("Piece full path. Case 2", QStringLiteral("://full_seam_allowance_path_case_2/input.json"),
                     QStringLiteral("://full_seam_allowance_path_case_2/output.json"), mirrorLine);

    ASSERT_TEST_CASE("Piece full path. Case 3", QStringLiteral("://full_seam_allowance_path_case_3/input.json"),
                     QStringLiteral("://full_seam_allowance_path_case_3/output.json"), mirrorLine);

    ASSERT_TEST_CASE("Piece full path. Case 4", QStringLiteral("://full_seam_allowance_path_case_4/input.json"),
                     QStringLiteral("://full_seam_allowance_path_case_4/output.json"), mirrorLine);

    // See file valentina_private_collection/bugs/full_piece/Basic_Darted_Bodice_Block.val (private collection)
    mirrorLine = QLineF(QPointF(37.79527559055033, 4616.693036243291), QPointF(37.79527559055132, 604.4628631137784));

    ASSERT_TEST_CASE("Basic Darted Bodice Block", QStringLiteral("://full_seam_allowance_path_case_5/input.json"),
                     QStringLiteral("://full_seam_allowance_path_case_5/output.json"), mirrorLine);

    // See file valentina_private_collection/bugs/blazer_for women_with_one_button/Blazer for women with one button.val
    mirrorLine =
        QLineF(QPointF(-2242.6828986255887, 2222.0199037691837), QPointF(-1886.7717217796137, 2360.0315200625723));

    ASSERT_TEST_CASE("Blazer for women with one button. Full seam allowance path case 1",
                     QStringLiteral("://full_seam_allowance_path_blazer_for_women_with_one_button_case_1/input.json"),
                     QStringLiteral("://full_seam_allowance_path_blazer_for_women_with_one_button_case_1/output.json"),
                     mirrorLine);

    ASSERT_TEST_CASE("Blazer for women with one button. Full seam allowance path case 2",
                     QStringLiteral("://full_seam_allowance_path_blazer_for_women_with_one_button_case_2/input.json"),
                     QStringLiteral("://full_seam_allowance_path_blazer_for_women_with_one_button_case_2/output.json"),
                     mirrorLine);

    ASSERT_TEST_CASE("Blazer for women with one button. Full seam allowance path case 3",
                     QStringLiteral("://full_seam_allowance_path_blazer_for_women_with_one_button_case_3/input.json"),
                     QStringLiteral("://full_seam_allowance_path_blazer_for_women_with_one_button_case_3/output.json"),
                     mirrorLine);

    ASSERT_TEST_CASE("Blazer for women with one button. Full seam allowance path case 4",
                     QStringLiteral("://full_seam_allowance_path_blazer_for_women_with_one_button_case_4/input.json"),
                     QStringLiteral("://full_seam_allowance_path_blazer_for_women_with_one_button_case_4/output.json"),
                     mirrorLine);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::TestFullSeamAllowancePath() const
{
    QFETCH(QVector<VLayoutPoint>, base);
    QFETCH(QVector<VLayoutPoint>, fullPath);
    QFETCH(QLineF, mirrorLine);

    QVector<QPointF> res;
    CastTo(VAbstractPiece::FullSeamAllowancePath(base, mirrorLine, QStringLiteral("Test")), res);

    QVector<QPointF> expected;
    CastTo(fullPath, expected);

    ComparePaths(res, expected);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::TestSeamLineTurnPoints_data() const
{
    QTest::addColumn<QVector<VSAPoint>>("points");
    QTest::addColumn<qreal>("width");
    QTest::addColumn<QVector<QPointF>>("turnPoints");

    auto ASSERT_TEST_CASE = [](const char *title, const QString &input, const QString &output, qreal width)
    {
        try
        {
            QVector<VSAPoint> const inputPoints = AbstractTest::VectorFromJson<VSAPoint>(input);
            QVector<VLayoutPoint> const turnPoints = AbstractTest::VectorFromJson<VLayoutPoint>(output);
            QVector<QPointF> outputPoints;
            CastTo(turnPoints, outputPoints);
            QTest::newRow(title) << inputPoints << width << outputPoints;
        }
        catch (const VException &e)
        {
            QFAIL(qUtf8Printable(e.ErrorMessage()));
        }
    };

    // See file valentina_private_collection/bugs/shirtv2/shirtv2.val
    ASSERT_TEST_CASE("fabric_cuff_L", QStringLiteral("://shirtv2_seam_allowance_line/input.json"),
                     QStringLiteral("://shirtv2_seam_allowance_line/output.json"),
                     37.795275590551185 /*seam allowance width*/);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::TestSeamLineTurnPoints() const
{
    QFETCH(QVector<VSAPoint>, points);
    QFETCH(qreal, width);
    QFETCH(QVector<QPointF>, turnPoints);

    QVector<VLayoutPoint> const ekv = VAbstractPiece::Equidistant(points, width, QString());
    QVector<QPointF> tPoints;
    CastTo(TurnPointList(ekv), tPoints); // Take result

    // Begin comparison
    ComparePaths(tPoints, turnPoints);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::Case3() const
{
    const QVector<QPointF> points = InputPointsCase3a(); // Input points.

    const qreal result = VAbstractPiece::SumTrapezoids(points);
    QVERIFY(result < 0);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::Case4() const
{
    const QVector<QPointF> points = InputPointsCase4a(); // Input points.

    const qreal result = VAbstractPiece::SumTrapezoids(points);
    QVERIFY(result > 0);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPiece::Case5() const
{
    const QVector<QPointF> points = InputPointsCase5a(); // Input points.

    const qreal result = VAbstractPiece::SumTrapezoids(points);
    QVERIFY(qFuzzyIsNull(result));
}

//---------------------------------------------------------------------------------------------------------------------
auto TST_VAbstractPiece::InputPointsCase3a() const -> QVector<QPointF>
{
    QVector<QPointF> points;

    points += QPointF(35, 35);
    points += QPointF(50, 50);
    points += QPointF(15, 50);

    return points;
}

//---------------------------------------------------------------------------------------------------------------------
auto TST_VAbstractPiece::InputPointsCase4a() const -> QVector<QPointF>
{
    QVector<QPointF> points;

    points += QPointF(15, 15);
    points += QPointF(15, 50);
    points += QPointF(50, 50);

    return points;
}

//---------------------------------------------------------------------------------------------------------------------
auto TST_VAbstractPiece::InputPointsCase5a() const -> QVector<QPointF>
{
    QVector<QPointF> points;

    points += QPointF(35, 35);

    return points;
}
