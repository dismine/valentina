/************************************************************************
 **
 **  @file   tst_vlabelarrangeengine.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 5, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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

#ifndef TST_VLABELARRANGEENGINE_H
#define TST_VLABELARRANGEENGINE_H

#include <QObject>

class TST_VLabelArrangeEngine : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit TST_VLabelArrangeEngine(QObject *parent = nullptr);
    ~TST_VLabelArrangeEngine() override = default;

private slots:
    void TestArrange_Empty();
    void TestArrange_SingleLabel_NoCoverAnchor();
    void TestArrange_TwoNonOverlapping_Unchanged();
    void TestArrange_TwoOverlapping_Separated();
    void TestArrange_LabelCoveringAnchor_MovedAway();
    void TestArrange_Idempotent();
    void TestArrange_TooCloseAnchors_GracefulDegradation();
    void TestArrange_LabelCoveringOtherAnchor_MovedAway();
    void TestArrange_NoLabelCoversAnyAnchor();
    void TestArrange_AnchorRadiusRespected();

private:
    Q_DISABLE_COPY_MOVE(TST_VLabelArrangeEngine) // NOLINT
};

#endif // TST_VLABELARRANGEENGINE_H
