/************************************************************************
 **
 **  @file   tst_vdetail.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   9 1, 2016
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

#include "tst_vpiece.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpassmark.h"
#include "../vpatterndb/vpiece.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vpatterndb/vpiecepath.h"

#include "../vpatterndb/vpiece.h"
#include <QtTest>

namespace
{
//---------------------------------------------------------------------------------------------------------------------
// A piece references a Draw::Modeling copy of a curve whose idObject points back at the original curve.
auto AddModelingCurveCopy(const QSharedPointer<VContainer> &data, quint32 curveId) -> quint32
{
    auto *copy = new VSpline(*data->GeometricObject<VSpline>(curveId));
    copy->setMode(Draw::Modeling);
    copy->setIdObject(curveId);
    return data->AddGObject(copy);
}

} // namespace

//---------------------------------------------------------------------------------------------------------------------
TST_VPiece::TST_VPiece(QObject *parent)
  : AbstractTest(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::Issue620()
{
    try
    {
        // See file <root>/src/app/share/collection/bugs/Issue_#620.vit
        // Check main path
        const Unit unit = Unit::Cm;
        QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
        VAbstractValApplication::VApp()->SetPatternUnits(unit);

        VPiece detail;
        AbstractTest::PieceFromJson(QStringLiteral("://Issue_620/input.json"), detail, data);

        QVector<QPointF> pointsEkv;
        CastTo(detail.MainPathPoints(data.data()), pointsEkv);
        QVector<QPointF> const origPoints = AbstractTest::VectorFromJson<QPointF>(
            QStringLiteral("://Issue_620/output.json"));

        // Begin comparison
        ComparePaths(pointsEkv, origPoints);
    }
    catch (const VException &e)
    {
        QFAIL(qUtf8Printable(e.ErrorMessage()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::TestSAPassmark_data()
{
    QTest::addColumn<VPiecePassmarkData>("passmarkData");
    QTest::addColumn<QVector<QPointF>>("seamAllowance");
    QTest::addColumn<QVector<QPointF>>("rotatedSeamAllowance");
    QTest::addColumn<QVector<QLineF>>("expectedResult");

    auto ASSERT_TEST_CASE = [this](const char *title,
                                   const QString &passmarkData,
                                   const QString &seamAllowance,
                                   const QString &rotatedSeamAllowance,
                                   const QString &shape)
    {
        QT_WARNING_PUSH
        QT_WARNING_DISABLE_GCC("-Wnoexcept")

        VPiecePassmarkData inputPassmarkData;
        AbstractTest::PassmarkDataFromJson(passmarkData, inputPassmarkData);

        QVector<QPointF> const inputSeamAllowance = AbstractTest::VectorFromJson<QPointF>(seamAllowance);
        QVector<QPointF> const inputRotatedSeamAllowance = AbstractTest::VectorFromJson<QPointF>(rotatedSeamAllowance);

        QVector<QLineF> inputOutputShape;
        AbstractTest::PassmarkShapeFromJson(shape, inputOutputShape);

        QTest::newRow(title) << inputPassmarkData << inputSeamAllowance << inputRotatedSeamAllowance
                             << inputOutputShape;

        QT_WARNING_POP
    };

    // See file src/app/share/collection/bugs/Issue_#924.val
    ASSERT_TEST_CASE("Test 1.",
                     QStringLiteral("://Issue_924_Test_1/passmarkData.json"),
                     QStringLiteral("://Issue_924_Test_1/seamAllowance.json"),
                     QStringLiteral("://Issue_924_Test_1/rotatedSeamAllowance.json"),
                     QStringLiteral("://Issue_924_Test_1/passmarkShape.json"));

    // See file src/app/share/collection/bugs/Issue_#924.val
    ASSERT_TEST_CASE("Test 2.",
                     QStringLiteral("://Issue_924_Test_2/passmarkData.json"),
                     QStringLiteral("://Issue_924_Test_2/seamAllowance.json"),
                     QStringLiteral("://Issue_924_Test_2/rotatedSeamAllowance.json"),
                     QStringLiteral("://Issue_924_Test_2/passmarkShape.json"));

    // See file src/app/share/collection/bugs/incorrect_notch.val
    ASSERT_TEST_CASE("Piece.",
                     QStringLiteral("://incorrect_notch/passmarkData.json"),
                     QStringLiteral("://incorrect_notch/seamAllowance.json"),
                     QStringLiteral("://incorrect_notch/rotatedSeamAllowance.json"),
                     QStringLiteral("://incorrect_notch/passmarkShape.json"));

    // See file src/app/share/collection/truezerobug.val
    ASSERT_TEST_CASE("Detail 2",
                     QStringLiteral("://true_zero_width_notches/passmarkData.json"),
                     QStringLiteral("://true_zero_width_notches/seamAllowance.json"),
                     QStringLiteral("://true_zero_width_notches/rotatedSeamAllowance.json"),
                     QStringLiteral("://true_zero_width_notches/passmarkShape.json"));

    // Straightforward passmark with a manual angle whose axis grazes the seam allowance near a corner (the axis end
    // point is already on the curve and produces several nearby intersections). The notch must follow the manual angle.
    // See file valentina_private_collection/bugs/notch_manual_angle/Jilet_rozwantajuwalniy.val
    // (private collection)
    ASSERT_TEST_CASE("Manual passmark angle.",
                     QStringLiteral("://manual_passmark_angle/passmarkData.json"),
                     QStringLiteral("://manual_passmark_angle/seamAllowance.json"),
                     QStringLiteral("://manual_passmark_angle/rotatedSeamAllowance.json"),
                     QStringLiteral("://manual_passmark_angle/passmarkShape.json"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::TestSAPassmark()
{
    QFETCH(VPiecePassmarkData, passmarkData);
    QFETCH(QVector<QPointF>, seamAllowance);
    QFETCH(QVector<QPointF>, rotatedSeamAllowance);
    QFETCH(QVector<QLineF>, expectedResult);

    VPassmark const passmark(passmarkData);

    CompareLinesDistance(passmark.SAPassmark(seamAllowance, rotatedSeamAllowance, PassmarkSide::All), expectedResult);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::TestSeamLineTurnPoints()
{
    try
    {
        // See file valentina_private_collection/bugs/shirtv2.val
        const Unit unit = Unit::Cm;
        QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
        VAbstractValApplication::VApp()->SetPatternUnits(unit);

        VPiece detail;
        AbstractTest::PieceFromJson(QStringLiteral("://shirtv2_seam_line/input.json"), detail, data);

        QVector<VLayoutPoint> const seamLine = detail.MainPathPoints(data.data());
        QVector<QPointF> pointsEkv;
        CastTo(TurnPointList(seamLine), pointsEkv);
        QVector<VLayoutPoint> const turnPoints = AbstractTest::VectorFromJson<VLayoutPoint>(
            QStringLiteral("://shirtv2_seam_line/output.json"));
        QVector<QPointF> origPoints;
        CastTo(turnPoints, origPoints);

        // Begin comparison
        ComparePaths(pointsEkv, origPoints);
    }
    catch (const VException &e)
    {
        QFAIL(qUtf8Printable(e.ErrorMessage()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
// A point node lying on a curve that runs through it (a cut point) must not add a corner to the seam allowance. With a
// wide allowance the joint produced a spike because the point was treated as a reflex corner.
// See file src/app/share/collection/bugs/point_on_curve_joint.val
void TST_VPiece::PointOnCurveJointNoSpike()
{
    try
    {
        const Unit unit = Unit::Cm;
        QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
        VAbstractValApplication::VApp()->SetPatternUnits(unit);

        VPiece detail;
        AbstractTest::PieceFromJson(QStringLiteral("://point_on_curve_joint/input.json"), detail, data);

        QVector<QPointF> pointsEkv;
        CastTo(detail.SeamAllowancePoints(data.data()), pointsEkv);
        QVector<QPointF> origPoints;
        CastTo(AbstractTest::VectorFromJson<VLayoutPoint>(QStringLiteral("://point_on_curve_joint/output.json")),
               origPoints);

        // Begin comparison
        ComparePaths(pointsEkv, origPoints);
    }
    catch (const VException &e)
    {
        QFAIL(qUtf8Printable(e.ErrorMessage()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
// Only a point lying on the curve shared by both neighbours is a smooth joint. A point between two different curves
// is a real corner, and a user-chosen angle type is always respected.
void TST_VPiece::PointOnCurveJointAngleType()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    const VPointF p1(0, 0, QStringLiteral("P1"));
    const VPointF p4(100, 100, QStringLiteral("P4"));
    const quint32 splineId = data->AddGObject(new VSpline(p1, p4, 200, 30, 1, 1, 1));
    const QVector<QPointF> curvePoints = data->GeometricObject<VSpline>(splineId)->GetPoints();

    const quint32 startId = data->AddGObject(new VPointF(p1.toQPointF(), QStringLiteral("S"), 0, 0));
    const quint32 cutId = data->AddGObject(
        new VPointF(curvePoints.at(curvePoints.size() / 2), QStringLiteral("C"), 0, 0));
    const quint32 endId = data->AddGObject(new VPointF(p4.toQPointF(), QStringLiteral("E"), 0, 0));

    QVector<VPieceNode> nodes{VPieceNode(startId, Tool::NodePoint),
                              VPieceNode(AddModelingCurveCopy(data, splineId), Tool::NodeSpline),
                              VPieceNode(cutId, Tool::NodePoint),
                              VPieceNode(AddModelingCurveCopy(data, splineId), Tool::NodeSpline),
                              VPieceNode(endId, Tool::NodePoint)};

    QCOMPARE(VPiecePath::PreparePointEkv(nodes, 2, data.data()).GetAngleType(), PieceNodeAngle::ByLengthCurve);
    QCOMPARE(VPiecePath::PreparePointEkv(nodes, 0, data.data()).GetAngleType(), PieceNodeAngle::ByLength);

    nodes[2].SetAngleType(PieceNodeAngle::ByPointsIntersection);
    QCOMPARE(VPiecePath::PreparePointEkv(nodes, 2, data.data()).GetAngleType(), PieceNodeAngle::ByPointsIntersection);
    nodes[2].SetAngleType(PieceNodeAngle::ByLength);

    // Two different curves meeting at the point
    const quint32 otherSplineId = data->AddGObject(new VSpline(p1, p4, 200, 30, 1, 1, 1));
    nodes[3] = VPieceNode(AddModelingCurveCopy(data, otherSplineId), Tool::NodeSpline);
    QCOMPARE(VPiecePath::PreparePointEkv(nodes, 2, data.data()).GetAngleType(), PieceNodeAngle::ByLength);
}
