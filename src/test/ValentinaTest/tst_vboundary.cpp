/************************************************************************
 **
 **  @file   tst_vboundary.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 11, 2024
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2024 Valentina project
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
#include "tst_vboundary.h"

#include "../vlayout/vboundary.h"
#include "../vlayout/vlayoutpiecepath.h"

#include <QtTest>

//---------------------------------------------------------------------------------------------------------------------
TST_VBoundary::TST_VBoundary(QObject *parent)
  : AbstractTest(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VBoundary::CombineSeamAllowance_data()
{
    QTest::addColumn<QVector<VLayoutPoint>>("points");
    QTest::addColumn<QString>("pieceName");
    QTest::addColumn<bool>("showFullPiece");
    QTest::addColumn<QLineF>("mirrorLine");
    QTest::addColumn<QVector<VLayoutPassmark>>("passmarks");
    QTest::addColumn<QVector<VLayoutPoint>>("boundaryPoints");

    auto ASSERT_TEST_CASE = [](const char *title,
                               const QString &pointsInput,
                               const QString &pieceName,
                               bool showFullPiece,
                               const QLineF &mirrorLine,
                               const QString &passmarksInput,
                               const QString &boundaryPointsInput)
    {
        QVector<VLayoutPoint> const inputPoints = AbstractTest::VectorFromJson<VLayoutPoint>(pointsInput);
        QVector<VLayoutPassmark> const passmarksPoints = AbstractTest::VectorFromJson<VLayoutPassmark>(passmarksInput);
        QVector<VLayoutPoint> const boundaryPoints = AbstractTest::VectorFromJson<VLayoutPoint>(boundaryPointsInput);
        QTest::newRow(title) << inputPoints << pieceName << showFullPiece << mirrorLine << passmarksPoints
                             << boundaryPoints;
    };

    // See file src/app/share/collection/test/notchbug.val
    ASSERT_TEST_CASE("Notch on mirror line. Case1",
                     QStringLiteral("://mirror_notch_case1/points.json"),
                     "Case 1",
                     true,
                     QLineF({30.000000000000167, -791.4961889763779}, {29.999999999999943, 115.59042519685045}),
                     QStringLiteral("://mirror_notch_case1/passmarks.json"),
                     QStringLiteral("://mirror_notch_case1/boundary.json"));

    // See file src/app/share/collection/test/notchbug.val
    ASSERT_TEST_CASE("Notch on mirror line. Case2",
                     QStringLiteral("://mirror_notch_case2/points.json"),
                     "Case 2",
                     true,
                     QLineF({30.000000000000167, -791.4961889763781}, {29.999999999999943, 115.59042519685045}),
                     QStringLiteral("://mirror_notch_case2/passmarks.json"),
                     QStringLiteral("://mirror_notch_case2/boundary.json"));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VBoundary::CombineSeamAllowance() const
{
    QFETCH(QVector<VLayoutPoint>, points);
    QFETCH(QString, pieceName);
    QFETCH(bool, showFullPiece);
    QFETCH(QLineF, mirrorLine);
    QFETCH(QVector<VLayoutPassmark>, passmarks);
    QFETCH(QVector<VLayoutPoint>, boundaryPoints);

    VBoundary boundary(points, true);
    boundary.SetPieceName(pieceName);

    if (!mirrorLine.isNull() && showFullPiece)
    {
        boundary.SetMirrorLine(mirrorLine);
    }

    const QList<VBoundarySequenceItemData> sequence = boundary.Combine(passmarks, false, false);

    QVector<VLayoutPoint> combinedBoundary;
    for (const auto &item : sequence)
    {
        combinedBoundary += item.item.value<VLayoutPiecePath>().Points();
    }

    QVector<QPointF> castedboundaryPoints;
    CastTo(boundaryPoints, castedboundaryPoints);

    QVector<QPointF> castedCombinedBoundary;
    CastTo(combinedBoundary, castedCombinedBoundary);

    // Begin comparison
    ComparePaths(castedCombinedBoundary, castedboundaryPoints);
}
