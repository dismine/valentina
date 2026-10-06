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

#include "../vlayout/vlayoutpiece.h"
#include "../vlayout/vlayoutpiecepath.h"
#include "../vlayout/vtextmanager.h"
#include "../vpatterndb/floatItemData/vpiecelabeldata.h"
#include "../vpatterndb/vpiece.h"
#include <QImage>
#include <QPainter>
#include <QPolygonF>
#include <QtTest>

using namespace Qt::Literals::StringLiterals;

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

//---------------------------------------------------------------------------------------------------------------------
// 10x10 cm square, seam allowance 1 cm, buffer 2 cm. All coordinates in px.
auto MakeSquarePiece(const QSharedPointer<VContainer> &data) -> VPiece
{
    const qreal side = ToPixel(10, Unit::Cm);
    const quint32 a = data->AddGObject(new VPointF(0, 0, QStringLiteral("A"), 0, 0));
    const quint32 b = data->AddGObject(new VPointF(side, 0, QStringLiteral("B"), 0, 0));
    const quint32 c = data->AddGObject(new VPointF(side, side, QStringLiteral("C"), 0, 0));
    const quint32 d = data->AddGObject(new VPointF(0, side, QStringLiteral("D"), 0, 0));

    VPiece piece;
    piece.SetName(QStringLiteral("Square"));
    for (quint32 const id : {a, b, c, d})
    {
        piece.GetPath().Append(VPieceNode(id, Tool::NodePoint));
    }
    piece.SetSeamAllowance(true);
    piece.SetFormulaSAWidth(QStringLiteral("1"), 1);
    piece.SetBufferName(QStringLiteral("Square buffer"));
    piece.SetFormulaBufferVisible(QStringLiteral("1"), 1);
    piece.SetFormulaBufferWidth(QStringLiteral("2"), 2);
    return piece;
}

//---------------------------------------------------------------------------------------------------------------------
auto BoundingRect(const QVector<VLayoutPoint> &points) -> QRectF
{
    QVector<QPointF> casted;
    CastTo(points, casted);
    return QPolygonF(casted).boundingRect();
}

//---------------------------------------------------------------------------------------------------------------------
auto SameRect(const QRectF &actual, const QRectF &expected) -> bool
{
    constexpr qreal eps = 0.01;
    return qAbs(actual.left() - expected.left()) < eps && qAbs(actual.top() - expected.top()) < eps
           && qAbs(actual.right() - expected.right()) < eps && qAbs(actual.bottom() - expected.bottom()) < eps;
}

//---------------------------------------------------------------------------------------------------------------------
auto RectToString(const QRectF &rect) -> QString
{
    return QStringLiteral("(%1, %2, %3, %4)").arg(rect.left()).arg(rect.top()).arg(rect.right()).arg(rect.bottom());
}

//---------------------------------------------------------------------------------------------------------------------
auto ProblemsFor(const QSharedPointer<VContainer> &data, VPiece piece, const VPieceOffsetLine &line) -> QStringList
{
    piece.SetOffsetLines({line});
    return piece.OffsetLineProblems(data.data());
}

//---------------------------------------------------------------------------------------------------------------------
auto ContainsProblem(const QStringList &problems, const QString &needle) -> bool
{
    return problems.size() == 1 && problems.constFirst().contains(needle);
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

//---------------------------------------------------------------------------------------------------------------------
// The buffer of a 10 cm square with 1 cm seam allowance and 2 cm buffer is a 16 cm square around it.
void TST_VPiece::BufferRectangle()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    const VPiece piece = MakeSquarePiece(data);
    const qreal off = ToPixel(3, Unit::Cm);
    const qreal side = ToPixel(10, Unit::Cm);
    const QRectF expected(-off, -off, side + 2 * off, side + 2 * off);
    const QRectF actual = BoundingRect(piece.BufferAllowancePoints(data.data()));
    QVERIFY2(SameRect(actual, expected), qUtf8Printable(RectToString(actual)));
    QVERIFY(piece.BufferProblems(data.data()).isEmpty());
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::BufferZeroWidthReturnsBase()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    VPiece piece = MakeSquarePiece(data);
    piece.SetFormulaBufferWidth(QStringLiteral("-5"), -5);
    QCOMPARE(piece.GetBufferWidth(), 0.0);
    const QRectF actual = BoundingRect(piece.BufferAllowancePoints(data.data()));
    const QRectF expected = BoundingRect(piece.FullSeamAllowancePoints(data.data()));
    QVERIFY2(SameRect(actual, expected), qUtf8Printable(RectToString(actual)));
    QCOMPARE(piece.BufferProblems(data.data()).size(), 1);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::BufferWithoutSeamAllowance()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    VPiece piece = MakeSquarePiece(data);
    piece.SetSeamAllowance(false);
    const qreal off = ToPixel(2, Unit::Cm);
    const qreal side = ToPixel(10, Unit::Cm);
    const QRectF expected(-off, -off, side + 2 * off, side + 2 * off);
    const QRectF actual = BoundingRect(piece.BufferAllowancePoints(data.data()));
    QVERIFY2(SameRect(actual, expected), qUtf8Printable(RectToString(actual)));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::AsBufferStripsExtras()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));

    VPiece piece = MakeSquarePiece(data);
    QVector<VPieceNode> nodes = piece.GetPath().GetNodes();
    nodes[1].SetPassmark(true);
    piece.GetPath().SetNodes(nodes);
    piece.SetInternalPaths({42});
    piece.SetPlaceLabels({43});
    piece.SetUUID(QUuid::createUuid());
    piece.GetPieceLabelData().SetBufferMaterial(3);
    piece.SetOffsetLines({VPieceOffsetLine{.formulaWidth = QStringLiteral("0.3")}});

    const VPiece buffer = piece.AsBuffer();
    QCOMPARE(buffer.GetName(), QStringLiteral("Square buffer"));
    QVERIFY(buffer.GetUUID() != piece.GetUUID());
    QCOMPARE(buffer.GetUUID(), piece.AsBuffer().GetUUID()); // deterministic
    QVERIFY(buffer.GetInternalPaths().isEmpty());
    QVERIFY(buffer.GetPlaceLabels().isEmpty());
    QVERIFY(buffer.GetOffsetLines().isEmpty());
    const QVector<VPieceNode> bufferNodes = buffer.GetPath().GetNodes();
    QVERIFY(std::none_of(bufferNodes.cbegin(),
                         bufferNodes.cend(),
                         [](const VPieceNode &node) { return node.IsPassmark(); }));
    QCOMPARE(buffer.GetPieceLabelData().PieceMaterial(true), 3);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::PieceMaterialSelection()
{
    VPieceLabelData data;
    data.SetNoBufferMaterial(1);
    data.SetWithBufferMaterial(2);
    data.SetBufferMaterial(99); // clamped
    QCOMPARE(data.PieceMaterial(false), 1);
    QCOMPARE(data.PieceMaterial(true), 2);
    QCOMPARE(data.GetBufferMaterial(), 20);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::PieceMaterialPlaceholder()
{
    const Unit unit = Unit::Cm;
    const VContainer data(nullptr, &unit, VContainer::UniqueNamespace());

    VPieceLabelInfo info(data);
    info.patternMaterials = {{1, QStringLiteral("Fabric")}, {2, QStringLiteral("Fusing")}};
    info.labelData.SetLabelTemplate({{.line = QStringLiteral("M:%pMaterial%")}});
    info.pieceMaterial = 2;

    VTextManager manager;
    manager.UpdatePieceLabelInfo(info);
    QCOMPARE(manager.GetSourceLinesCount(), 1);
    QCOMPARE(manager.GetSourceLine(0).qsText, QStringLiteral("M:Fusing"));

    info.pieceMaterial = 0; // <empty>
    manager.UpdatePieceLabelInfo(info);
    QCOMPARE(manager.GetSourceLine(0).qsText, QStringLiteral("M:"));
}

//---------------------------------------------------------------------------------------------------------------------
// A 0.3 cm offset line around a 10 cm square is a 10.6 cm square.
void TST_VPiece::OffsetLineFull()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    const VPiece piece = MakeSquarePiece(data);
    const VPieceOffsetLine line{.formulaWidth = QStringLiteral("0.3")};

    const QVector<VLayoutPoint> points = piece.OffsetLinePoints(data.data(), line);
    const qreal off = ToPixel(0.3, Unit::Cm);
    const qreal side = ToPixel(10, Unit::Cm);
    const QRectF expected(-off, -off, side + 2 * off, side + 2 * off);
    const QRectF actual = BoundingRect(points);
    QVERIFY2(SameRect(actual, expected), qUtf8Printable(RectToString(actual)));
    QVERIFY(points.size() >= 5);
    QVERIFY(VFuzzyComparePoints(points.constFirst(), points.constLast())); // closed
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::OffsetLineHiddenWithoutSeamAllowance()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    VPiece piece = MakeSquarePiece(data);
    const VPieceOffsetLine line{.formulaWidth = QStringLiteral("0.3")};

    piece.SetSeamAllowance(false);
    QVERIFY(piece.OffsetLinePoints(data.data(), line).isEmpty());

    piece.SetSeamAllowance(true);
    piece.SetSeamAllowanceBuiltIn(true);
    QVERIFY(piece.OffsetLinePoints(data.data(), line).isEmpty());
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::OffsetLineInvisible()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    const VPiece piece = MakeSquarePiece(data);
    const VPieceOffsetLine line{.formulaWidth = QStringLiteral("0.3"), .formulaVisible = QStringLiteral("0")};
    QVERIFY(not piece.IsOffsetLineVisible(data.data(), line));
    QVERIFY(piece.OffsetLinePoints(data.data(), line).isEmpty());
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::OffsetLineBadFormula()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    const VPiece piece = MakeSquarePiece(data);
    const VPieceOffsetLine line{.formulaWidth = QStringLiteral("#missing")};
    QCOMPARE(piece.OffsetLineWidth(data.data(), line), 0.0);
    QVERIFY(piece.OffsetLinePoints(data.data(), line).isEmpty());
}

//---------------------------------------------------------------------------------------------------------------------
// Partial B -> C: the right side only. The line continues straight past both ends until it meets the cut line
// (1 cm SA): x = side + off from y = -sa to y = side + sa.
void TST_VPiece::OffsetLinePartial()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    const VPiece piece = MakeSquarePiece(data);
    const QVector<VPieceNode> nodes = piece.GetPath().GetNodes();
    const VPieceOffsetLine line{.start = nodes.at(1).GetId(),
                                .end = nodes.at(2).GetId(),
                                .formulaWidth = QStringLiteral("0.3")};

    const QVector<VLayoutPoint> points = piece.OffsetLinePoints(data.data(), line);
    const qreal off = ToPixel(0.3, Unit::Cm);
    const qreal sa = ToPixel(1, Unit::Cm);
    const qreal side = ToPixel(10, Unit::Cm);
    const QRectF expected(side + off, -sa, 0, side + 2 * sa);
    const QRectF actual = BoundingRect(points);
    QVERIFY2(SameRect(actual, expected), qUtf8Printable(RectToString(actual)));
    QVERIFY(qAbs(points.constFirst().y() + sa) < 0.01);         // on the top cut line
    QVERIFY(qAbs(points.constLast().y() - (side + sa)) < 0.01); // on the bottom cut line
}

//---------------------------------------------------------------------------------------------------------------------
// Partial C -> B follows the main path direction: C -> D -> A -> B, i.e. three sides. Both ends continue straight
// (to the right) until they meet the right cut line at x = side + sa.
void TST_VPiece::OffsetLinePartialDirection()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    const VPiece piece = MakeSquarePiece(data);
    const QVector<VPieceNode> nodes = piece.GetPath().GetNodes();
    const VPieceOffsetLine line{.start = nodes.at(2).GetId(),
                                .end = nodes.at(1).GetId(),
                                .formulaWidth = QStringLiteral("0.3")};

    const QVector<VLayoutPoint> points = piece.OffsetLinePoints(data.data(), line);
    const QRectF actual = BoundingRect(points);
    const qreal off = ToPixel(0.3, Unit::Cm);
    const qreal sa = ToPixel(1, Unit::Cm);
    const qreal side = ToPixel(10, Unit::Cm);
    const QRectF expected(-off, -off, side + sa + off, side + 2 * off);
    QVERIFY2(SameRect(actual, expected), qUtf8Printable(RectToString(actual)));
    QVERIFY(qAbs(points.constFirst().x() - (side + sa)) < 0.01);
    QVERIFY(qAbs(points.constLast().x() - (side + sa)) < 0.01);
}

//---------------------------------------------------------------------------------------------------------------------
// L-shaped piece, the partial line ends at the inner (concave) corner E(5, 5). The end of the line must be the offset
// corner next to E (5.3, 5.3), not a point on a far side of the piece. Continuing straight from there crosses the seam
// line, so the line is reported.
void TST_VPiece::OffsetLinePartialConcaveEnd()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    VPiece piece;
    piece.SetName(QStringLiteral("L"));
    const QVector<QPointF> corners{{0, 0}, {10, 0}, {10, 5}, {5, 5}, {5, 10}, {0, 10}};
    QVector<quint32> ids;
    for (int i = 0; i < corners.size(); ++i)
    {
        ids.append(data->AddGObject(new VPointF(ToPixel(corners.at(i).x(), Unit::Cm),
                                                ToPixel(corners.at(i).y(), Unit::Cm),
                                                QStringLiteral("P%1").arg(i),
                                                0,
                                                0)));
        piece.GetPath().Append(VPieceNode(ids.constLast(), Tool::NodePoint));
    }
    piece.SetSeamAllowance(true);
    piece.SetFormulaSAWidth(QStringLiteral("1"), 1);

    const VPieceOffsetLine line{.start = ids.at(4), .end = ids.at(3), .formulaWidth = QStringLiteral("0.3")};
    const QVector<VLayoutPoint> points = piece.OffsetLinePoints(data.data(), line);
    QVERIFY(points.size() >= 4);
    const QPointF p2 = points.at(points.size() - 2);
    const QPointF expected(ToPixel(5.3, Unit::Cm), ToPixel(5.3, Unit::Cm));
    QVERIFY2(VFuzzyComparePoints(p2, expected), qUtf8Printable(u"(%1, %2)"_s.arg(p2.x()).arg(p2.y())));

    piece.SetOffsetLines({line});
    QVERIFY(ContainsProblem(piece.OffsetLineProblems(data.data()), QStringLiteral("not between")));
}

//---------------------------------------------------------------------------------------------------------------------
// Fold on the left side D -> A: the full line becomes an open half that ends on the fold.
void TST_VPiece::OffsetLineMirrored()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    VPiece piece = MakeSquarePiece(data);
    const QVector<VPieceNode> nodes = piece.GetPath().GetNodes();
    piece.SetMirrorLineStartPoint(nodes.at(3).GetId());
    piece.SetMirrorLineEndPoint(nodes.at(0).GetId());

    const QVector<VLayoutPoint> points = piece.OffsetLinePoints(data.data(),
                                                                VPieceOffsetLine{
                                                                    .formulaWidth = QStringLiteral("0.3")});
    const qreal off = ToPixel(0.3, Unit::Cm);
    const qreal side = ToPixel(10, Unit::Cm);
    const QRectF expected(0, -off, side + off, side + 2 * off);
    const QRectF actual = BoundingRect(points);
    QVERIFY2(SameRect(actual, expected), qUtf8Printable(RectToString(actual)));
    QVERIFY(not VFuzzyComparePoints(points.constFirst(), points.constLast())); // open

    const QStringList problems = ProblemsFor(data, piece, {.formulaWidth = QStringLiteral("0.3")});
    QVERIFY2(problems.isEmpty(), qUtf8Printable(problems.join('\n')));
}

//---------------------------------------------------------------------------------------------------------------------
// A partial line that ends at a node of the mirror line stops on the mirror line itself, so the mirrored copy meets it.
// The mirror line is oblique to the last edge here, which is where the cut line of the half piece stops short of or
// runs past the mirror line.
void TST_VPiece::OffsetLinePartialEndsOnMirrorLine()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    const quint32 a = data->AddGObject(new VPointF(0, 0, QStringLiteral("A"), 0, 0));
    const quint32 b = data->AddGObject(
        new VPointF(ToPixel(10, Unit::Cm), ToPixel(2, Unit::Cm), QStringLiteral("B"), 0, 0));
    const quint32 c = data->AddGObject(
        new VPointF(ToPixel(6, Unit::Cm), ToPixel(8, Unit::Cm), QStringLiteral("C"), 0, 0));

    VPiece piece;
    piece.SetName(QStringLiteral("Triangle"));
    for (quint32 const id : {a, b, c})
    {
        piece.GetPath().Append(VPieceNode(id, Tool::NodePoint));
    }
    piece.SetSeamAllowance(true);
    piece.SetFormulaSAWidth(QStringLiteral("1"), 1);
    piece.SetMirrorLineStartPoint(c);
    piece.SetMirrorLineEndPoint(a);
    piece.SetShowFullPiece(true);

    const VPieceOffsetLine line{.start = b, .end = c, .formulaWidth = QStringLiteral("0.3")};
    const QVector<VLayoutPoint> points = piece.OffsetLinePoints(data.data(), line);
    QVERIFY(points.size() >= 2);

    const QLineF mirror = piece.SeamMirrorLine(data.data());
    const QPointF last = points.constLast();
    const QLineF normal = QLineF(mirror.p1(), last);
    const qreal distance = qAbs(QLineF(mirror.p1(), last).length() * qSin(qDegreesToRadians(mirror.angleTo(normal))));
    QVERIFY2(distance < 0.01, qUtf8Printable(QStringLiteral("Distance to the mirror line %1").arg(distance)));

    // A half piece keeps its own cut line: the end must reach it, whatever the fold does
    piece.SetShowFullPiece(false);
    const QVector<VLayoutPoint> half = piece.OffsetLinePoints(data.data(), line);
    QVERIFY(half.size() >= 2);
    QVector<QPointF> cut;
    CastTo(piece.SeamAllowancePoints(data.data()), cut);
    cut.append(cut.constFirst());
    qreal nearest = std::numeric_limits<qreal>::max();
    for (int i = 0; i < cut.size() - 1; ++i)
    {
        const QLineF edge(cut.at(i), cut.at(i + 1));
        QLineF perpendicular = edge.normalVector();
        perpendicular.translate(half.constLast() - perpendicular.p1());
        QPointF foot;
        if (edge.intersects(perpendicular, &foot) != QLineF::NoIntersection
            && QLineF(edge.p1(), foot).length() + QLineF(foot, edge.p2()).length() < edge.length() + 0.01)
        {
            nearest = qMin(nearest, QLineF(half.constLast(), foot).length());
        }
    }
    QVERIFY2(nearest < 0.01, qUtf8Printable(QStringLiteral("Distance to the cut line %1").arg(nearest)));
}

//---------------------------------------------------------------------------------------------------------------------
// The piece icon is painted with a fill brush. An open internal path (an offset line is one) must not be filled, or the
// fill of its implicit closing chord paints over the contour lines it crosses.
void TST_VPiece::MiniatureDoesNotFillInternalPaths()
{
    VLayoutPiece piece;
    piece.SetContourPoints(
        {VLayoutPoint(0, 0), VLayoutPoint(100, 0), VLayoutPoint(100, 100), VLayoutPoint(0, 100), VLayoutPoint(0, 0)});
    // The chord from the last to the first point encloses the contour edge x = 100 between y = 50 and y = 90.
    piece.SetInternalPaths({VLayoutPiecePath({VLayoutPoint(50, 10), VLayoutPoint(50, 90), VLayoutPoint(150, 90)})});

    QImage image(200, 200, QImage::Format_ARGB32);
    image.fill(Qt::white);
    {
        QPainter painter(&image);
        painter.setPen(QPen(Qt::red, 2));
        painter.setBrush(Qt::blue);
        piece.DrawMiniature(painter, false);
    }

    QCOMPARE(image.pixelColor(100, 70), QColor(Qt::red));
}

//---------------------------------------------------------------------------------------------------------------------
// The same nodes and the same evaluated width make a duplicate, however the formulas are written. A different width, the
// opposite direction or an invisible twin is a different line.
void TST_VPiece::OffsetLineDuplicate()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    VPiece piece = MakeSquarePiece(data);
    const QVector<VPieceNode> nodes = piece.GetPath().GetNodes();
    const quint32 b = nodes.at(1).GetId();
    const quint32 c = nodes.at(2).GetId();

    const auto problems = [&piece, &data](const QVector<VPieceOffsetLine> &lines)
    {
        piece.SetOffsetLines(lines);
        return piece.OffsetLineProblems(data.data());
    };

    const VPieceOffsetLine full{.formulaWidth = QStringLiteral("0.3")};
    const VPieceOffsetLine partial{.start = b, .end = c, .formulaWidth = QStringLiteral("0.3")};

    QStringList found = problems({full, full});
    QVERIFY2(found.size() == 1 && found.constFirst().contains(u"Offset line #2"_s)
                 && found.constFirst().contains(u"duplicates offset line #1"_s),
             qUtf8Printable(found.join('\n')));

    found = problems({partial, {.start = b, .end = c, .formulaWidth = QStringLiteral("0.6/2")}});
    QVERIFY2(found.size() == 1 && found.constFirst().contains(u"duplicates offset line #1"_s),
             qUtf8Printable(found.join('\n')));

    QVERIFY(problems({partial, {.start = b, .end = c, .formulaWidth = QStringLiteral("0.4")}}).isEmpty());
    QVERIFY(problems({partial, {.start = c, .end = b, .formulaWidth = QStringLiteral("0.3")}}).isEmpty());
    QVERIFY(problems({partial, full}).isEmpty());
    QVERIFY(
        problems({partial,
                  {.start = b, .end = c, .formulaWidth = QStringLiteral("0.3"), .formulaVisible = QStringLiteral("0")}})
            .isEmpty());
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::OffsetLineProblemsValid()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    const VPiece piece = MakeSquarePiece(data);
    const QVector<VPieceNode> nodes = piece.GetPath().GetNodes();
    QStringList problems = ProblemsFor(data, piece, {.formulaWidth = QStringLiteral("0.3")});
    QVERIFY2(problems.isEmpty(), qUtf8Printable(problems.join('\n')));
    problems = ProblemsFor(data,
                           piece,
                           {.start = nodes.at(1).GetId(),
                            .end = nodes.at(2).GetId(),
                            .formulaWidth = QStringLiteral("0.3")});
    QVERIFY2(problems.isEmpty(), qUtf8Printable(problems.join('\n')));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::OffsetLineTooWide()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    const VPiece piece = MakeSquarePiece(data); // SA 1 cm
    QVERIFY(ContainsProblem(ProblemsFor(data, piece, {.formulaWidth = QStringLiteral("1.5")}),
                            QStringLiteral("not between")));
    QVERIFY(ContainsProblem(ProblemsFor(data, piece, {.formulaWidth = QStringLiteral("1")}),
                            QStringLiteral("not between")));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::OffsetLineZeroWidth()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    const VPiece piece = MakeSquarePiece(data);
    QVERIFY(ContainsProblem(ProblemsFor(data, piece, {.formulaWidth = QStringLiteral("0")}),
                            QStringLiteral("greater than 0")));
    QVERIFY(ContainsProblem(ProblemsFor(data, piece, {.formulaWidth = QStringLiteral("-1")}),
                            QStringLiteral("greater than 0")));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::OffsetLineMissingNode()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    const VPiece piece = MakeSquarePiece(data);
    const QVector<VPieceNode> nodes = piece.GetPath().GetNodes();
    const quint32 stranger = data->AddGObject(new VPointF(5, 5, QStringLiteral("X"), 0, 0));
    QVERIFY(ContainsProblem(ProblemsFor(data,
                                        piece,
                                        {.start = stranger,
                                         .end = nodes.at(2).GetId(),
                                         .formulaWidth = QStringLiteral("0.3")}),
                            QStringLiteral("not a valid main path point")));
    QVERIFY(
        ContainsProblem(ProblemsFor(data,
                                    piece,
                                    {.start = 9999, .end = nodes.at(2).GetId(), .formulaWidth = QStringLiteral("0.3")}),
                        QStringLiteral("not a valid main path point")));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::OffsetLineSameNodes()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    const VPiece piece = MakeSquarePiece(data);
    const quint32 b = piece.GetPath().GetNodes().at(1).GetId();
    QVERIFY(ContainsProblem(ProblemsFor(data, piece, {.start = b, .end = b, .formulaWidth = QStringLiteral("0.3")}),
                            QStringLiteral("not a valid main path point")));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::OffsetLineExcludedNode()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    VPiece piece = MakeSquarePiece(data);
    QVector<VPieceNode> nodes = piece.GetPath().GetNodes();
    nodes[1].SetExcluded(true);
    piece.GetPath().SetNodes(nodes);
    QVERIFY(ContainsProblem(ProblemsFor(data,
                                        piece,
                                        {.start = nodes.at(1).GetId(),
                                         .end = nodes.at(2).GetId(),
                                         .formulaWidth = QStringLiteral("0.3")}),
                            QStringLiteral("not a valid main path point")));
}

//---------------------------------------------------------------------------------------------------------------------
// Degenerate piece: all nodes at one point -> no contour -> "empty".
void TST_VPiece::OffsetLineEmpty()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    VPiece piece;
    piece.SetName(QStringLiteral("Dot"));
    for (int i = 0; i < 4; ++i)
    {
        piece.GetPath().Append(
            VPieceNode(data->AddGObject(new VPointF(0, 0, QStringLiteral("P%1").arg(i), 0, 0)), Tool::NodePoint));
    }
    piece.SetSeamAllowance(true);
    piece.SetFormulaSAWidth(QStringLiteral("1"), 1);

    QVERIFY(
        ContainsProblem(ProblemsFor(data, piece, {.formulaWidth = QStringLiteral("0.3")}), QStringLiteral("is empty")));
}

//---------------------------------------------------------------------------------------------------------------------
// Fold on D -> A; a partial line C -> B runs C -> D -> A -> B, i.e. along the fold, where the offset is 0.
void TST_VPiece::OffsetLineAcrossFold()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    VPiece piece = MakeSquarePiece(data);
    const QVector<VPieceNode> nodes = piece.GetPath().GetNodes();
    piece.SetMirrorLineStartPoint(nodes.at(3).GetId());
    piece.SetMirrorLineEndPoint(nodes.at(0).GetId());
    const QStringList problems = ProblemsFor(data,
                                             piece,
                                             {.start = nodes.at(2).GetId(),
                                              .end = nodes.at(1).GetId(),
                                              .formulaWidth = QStringLiteral("0.3")});
    QCOMPARE(problems.size(), 1); // "empty" or "not between" are both acceptable; no crash, exactly one message
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::OffsetLineProblemsSkipped()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    VPiece piece = MakeSquarePiece(data);
    QVERIFY(ProblemsFor(data, piece, {.formulaWidth = QStringLiteral("0"), .formulaVisible = QStringLiteral("0")})
                .isEmpty());
    piece.SetSeamAllowance(false);
    QVERIFY(ProblemsFor(data, piece, {.formulaWidth = QStringLiteral("0")}).isEmpty());
    piece.SetSeamAllowance(true);
    piece.SetSeamAllowanceBuiltIn(true);
    QVERIFY(ProblemsFor(data, piece, {.formulaWidth = QStringLiteral("0")}).isEmpty());
}

//---------------------------------------------------------------------------------------------------------------------
// Geometry the problem check rejects must never be drawn or exported.
void TST_VPiece::OffsetLineInvalidNodesNoPoints()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    VPiece piece = MakeSquarePiece(data);
    QVector<VPieceNode> nodes = piece.GetPath().GetNodes();
    const quint32 b = nodes.at(1).GetId();
    const quint32 c = nodes.at(2).GetId();

    // Same node
    QVERIFY(
        piece.OffsetLinePoints(data.data(), {.start = b, .end = b, .formulaWidth = QStringLiteral("0.3")}).isEmpty());

    // A point lying on the seam line that is not a node of the main path
    const quint32 onSeam = data->AddGObject(
        new VPointF(ToPixel(10, Unit::Cm), ToPixel(5, Unit::Cm), QStringLiteral("M"), 0, 0));
    QVERIFY(piece.OffsetLinePoints(data.data(), {.start = onSeam, .end = c, .formulaWidth = QStringLiteral("0.3")})
                .isEmpty());

    // Excluded node
    nodes[1].SetExcluded(true);
    piece.GetPath().SetNodes(nodes);
    QVERIFY(
        piece.OffsetLinePoints(data.data(), {.start = b, .end = c, .formulaWidth = QStringLiteral("0.3")}).isEmpty());
}

//---------------------------------------------------------------------------------------------------------------------
// A duplicated piece gets new node ids; partial offset lines must follow them, full lines stay full.
void TST_VPiece::OffsetLineRemapNodes()
{
    VPiece piece;
    piece.SetOffsetLines({{.formulaWidth = QStringLiteral("0.3")},
                          {.start = 1, .end = 2, .formulaWidth = QStringLiteral("0.2")},
                          {.start = 1, .end = 7, .formulaWidth = QStringLiteral("0.2")}});

    piece.RemapOffsetLineNodes({{1, 11}, {2, 12}});

    const QVector<VPieceOffsetLine> lines = piece.GetOffsetLines();
    QCOMPARE(lines.size(), 3);
    QVERIFY(lines.at(0).IsFull());
    QCOMPARE(lines.at(1).start, 11U);
    QCOMPARE(lines.at(1).end, 12U);
    QCOMPARE(lines.at(2).start, 11U);
    QCOMPARE(lines.at(2).end, 7U); // unknown id kept, reported as a problem later
}

//---------------------------------------------------------------------------------------------------------------------
// A warning names the line the same way the dialog list does, so the user can find it.
void TST_VPiece::OffsetLineProblemNamesLine()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    const VPiece piece = MakeSquarePiece(data);
    const quint32 b = piece.GetPath().GetNodes().at(1).GetId();

    const VPieceOffsetLine partial{.start = b, .end = b, .formulaWidth = QStringLiteral("0.3")};
    QCOMPARE(VPiece::OffsetLineName(data.data(), piece.GetPath().GetNodes(), partial), u"B \u2192 B, 0.3"_s);
    QVERIFY(ContainsProblem(ProblemsFor(data, piece, partial), u"Offset line #1 (B \u2192 B, 0.3)"_s));

    const VPieceOffsetLine full{.formulaWidth = QStringLiteral("0")};
    QVERIFY(ContainsProblem(ProblemsFor(data, piece, full),
                            u"Offset line #1 (%1)"_s.arg(
                                VPiece::OffsetLineName(data.data(), piece.GetPath().GetNodes(), full))));

    const VPieceOffsetLine missing{.start = 9999, .end = b, .formulaWidth = QStringLiteral("0.3")};
    QVERIFY(VPiece::OffsetLineName(data.data(), piece.GetPath().GetNodes(), missing).startsWith(u"<"_s));
}

//---------------------------------------------------------------------------------------------------------------------
// An end that is not a usable node of the piece is marked and, where possible, named: by the live object if it still
// exists, otherwise by the last known name stored with the record.
void TST_VPiece::OffsetLineNameMarksMissingPoints()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    VPiece piece = MakeSquarePiece(data);
    QVector<VPieceNode> nodes = piece.GetPath().GetNodes();
    const quint32 b = nodes.at(1).GetId();
    const quint32 c = nodes.at(2).GetId();
    const QString w = QStringLiteral("0.3");

    // Gone from the pattern, no hint: nothing to name
    QCOMPARE(VPiece::OffsetLineName(data.data(), nodes, {.start = b, .end = 9999, .formulaWidth = w}),
             u"B \u2192 <missing>, 0.3"_s);

    // Gone from the pattern, last known name stored
    QCOMPARE(VPiece::OffsetLineName(data.data(), nodes, {.start = b, .end = 9999, .formulaWidth = w, .endName = u"Q"_s}),
             u"B \u2192 <missing: Q>, 0.3"_s);

    // Not in the piece any more but still in the pattern: live name
    const quint32 stranger = data->AddGObject(new VPointF(5, 5, QStringLiteral("X"), 0, 0));
    QCOMPARE(VPiece::OffsetLineName(data.data(), nodes, {.start = stranger, .end = c, .formulaWidth = w}),
             u"<missing: X> \u2192 C, 0.3"_s);

    // Excluded from the piece
    nodes[2].SetExcluded(true);
    QCOMPARE(VPiece::OffsetLineName(data.data(), nodes, {.start = b, .end = c, .formulaWidth = w}),
             u"B \u2192 <missing: C>, 0.3"_s);

    // A full line has no ends to mark, and ignores stray hints
    QCOMPARE(VPiece::OffsetLineName(data.data(), nodes, {.formulaWidth = w, .startName = u"Z"_s}), u"Full, 0.3"_s);

    // The problem text carries the marker too
    piece.GetPath().SetNodes(nodes);
    piece.SetOffsetLines({{.start = b, .end = 9999, .formulaWidth = w, .endName = u"Q"_s}});
    const QStringList problems = piece.OffsetLineProblems(data.data());
    QVERIFY2(problems.size() == 1 && problems.constFirst().contains(u"B \u2192 <missing: Q>, 0.3"_s),
             qUtf8Printable(problems.join('\n')));
}

//---------------------------------------------------------------------------------------------------------------------
// The live name wins over the stored one, so a renamed point shows its new name.
void TST_VPiece::OffsetLineNameFollowsRename()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    const VPiece piece = MakeSquarePiece(data);
    const QVector<VPieceNode> nodes = piece.GetPath().GetNodes();
    const VPieceOffsetLine line{.start = nodes.at(1).GetId(),
                                .end = nodes.at(2).GetId(),
                                .formulaWidth = QStringLiteral("0.3"),
                                .startName = u"OldB"_s,
                                .endName = u"OldC"_s};
    QCOMPARE(VPiece::OffsetLineName(data.data(), nodes, line), u"B \u2192 C, 0.3"_s);
}

//---------------------------------------------------------------------------------------------------------------------
// Names are refreshed from live objects; where an object is gone the last known name is kept.
void TST_VPiece::OffsetLineRefreshNames()
{
    const Unit unit = Unit::Cm;
    QSharedPointer<VContainer> data(new VContainer(nullptr, &unit, VContainer::UniqueNamespace()));
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    VPiece piece = MakeSquarePiece(data);
    const QVector<VPieceNode> nodes = piece.GetPath().GetNodes();
    const QString w = QStringLiteral("0.3");
    piece.SetOffsetLines(
        {{.start = nodes.at(1).GetId(), .end = nodes.at(2).GetId(), .formulaWidth = w, .startName = u"OldB"_s},
         {.start = nodes.at(1).GetId(), .end = 9999, .formulaWidth = w, .endName = u"Q"_s},
         {.formulaWidth = w, .startName = u"Stray"_s}});

    piece.RefreshOffsetLineNames(data.data());

    const QVector<VPieceOffsetLine> lines = piece.GetOffsetLines();
    QCOMPARE(lines.at(0).startName, u"B"_s);
    QCOMPARE(lines.at(0).endName, u"C"_s);
    QCOMPARE(lines.at(1).startName, u"B"_s);
    QCOMPARE(lines.at(1).endName, u"Q"_s);    // 9999 doesn't exist: keep the last known name
    QVERIFY(lines.at(2).startName.isEmpty()); // full lines carry no names
    QVERIFY(lines.at(2).endName.isEmpty());
}
