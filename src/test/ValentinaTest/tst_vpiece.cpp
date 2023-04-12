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
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiece.h"
#include "../vpatterndb/vpassmark.h"
#include "../vmisc/vabstractvalapplication.h"

#include <QtTest>
#include "../vpatterndb/vpiece.h"

//---------------------------------------------------------------------------------------------------------------------
TST_VPiece::TST_VPiece(QObject *parent)
    :AbstractTest(parent)
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
        QVector<QPointF> origPoints = AbstractTest::VectorFromJson<QPointF>(QStringLiteral("://Issue_620/output.json"));

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

    auto ASSERT_TEST_CASE = [this](const char *title, const QString &passmarkData, const QString &seamAllowance,
                                   const QString &rotatedSeamAllowance, const QString &shape)
    {
        QT_WARNING_PUSH
        QT_WARNING_DISABLE_GCC("-Wnoexcept")

        VPiecePassmarkData inputPassmarkData;
        AbstractTest::PassmarkDataFromJson(passmarkData, inputPassmarkData);

        QVector<QPointF> inputSeamAllowance = AbstractTest::VectorFromJson<QPointF>(seamAllowance);
        QVector<QPointF> inputRotatedSeamAllowance = AbstractTest::VectorFromJson<QPointF>(rotatedSeamAllowance);

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
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VPiece::TestSAPassmark()
{
    QFETCH(VPiecePassmarkData, passmarkData);
    QFETCH(QVector<QPointF>, seamAllowance);
    QFETCH(QVector<QPointF>, rotatedSeamAllowance);
    QFETCH(QVector<QLineF>, expectedResult);

    VPassmark passmark(passmarkData);

    CompareLinesDistance(passmark.SAPassmark(seamAllowance, rotatedSeamAllowance, PassmarkSide::All), expectedResult);
}
