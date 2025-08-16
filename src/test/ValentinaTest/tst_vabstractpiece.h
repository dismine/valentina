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

#ifndef TST_VABSTRACTPIECE_H
#define TST_VABSTRACTPIECE_H

#include "../vtest/abstracttest.h"

class VSAPoint;

class TST_VAbstractPiece : public AbstractTest
{
    Q_OBJECT // NOLINT

public:
    explicit TST_VAbstractPiece(QObject *parent = nullptr);

private slots:
    void EquidistantRemoveLoop_data();
    void EquidistantRemoveLoop() const;
    void LayoutAllowanceRemoveLoop_data();
    void LayoutAllowanceRemoveLoop() const;
    void SumTrapezoids() const;
    void TestAreaCalculation_data();
    void TestAreaCalculation() const;
    void RawPathRemoveLoop_data() const;
    void RawPathRemoveLoop() const;
    void PathRemoveLoop_data() const;
    void PathRemoveLoop() const;
    void PathLoopsCase_data() const;
    void PathLoopsCase() const;
    void BrokenDetailEquidistant_data();
    void BrokenDetailEquidistant() const;
    void EquidistantAngleType_data();
    void EquidistantAngleType() const;
    void CorrectEquidistantPoints_data();
    void CorrectEquidistantPoints() const;
    void TestCorrectEquidistantPoints_data();
    void TestCorrectEquidistantPoints() const;
    void PossibleInfiniteClearLoops_data() const;
    void PossibleInfiniteClearLoops() const;
    void IsAllowanceValid_data() const;
    void IsAllowanceValid() const;
    void TestFullSeamPath_data() const;
    void TestFullSeamPath() const;
    void TestFullSeamAllowancePath_data() const;
    void TestFullSeamAllowancePath() const;
    void TestSeamLineTurnPoints_data() const;
    void TestSeamLineTurnPoints() const;
    void TestTrueZeroSeamAllowanceWidth_data() const;
    void TestTrueZeroSeamAllowanceWidth() const;

private:
    auto InputPointsCase3() const -> QVector<VSAPoint>;
    auto OutputPointsCase3() const -> QVector<QPointF>;

    void Case3() const;
    void Case4() const;
    void Case5() const;

    auto InputPointsCase3a() const -> QVector<QPointF>;
    auto InputPointsCase4a() const -> QVector<QPointF>;
    auto InputPointsCase5a() const -> QVector<QPointF>;

    auto InputLoopByIntersectionTest() -> QVector<VSAPoint>;
    auto OutputLoopByIntersectionTest() -> QVector<QPointF>;
};

#endif // TST_VABSTRACTPIECE_H
