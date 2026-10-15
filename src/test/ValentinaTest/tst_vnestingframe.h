/************************************************************************
 **
 **  @file   tst_vnestingframe.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 October, 2026
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

#ifndef TST_VNESTINGFRAME_H
#define TST_VNESTINGFRAME_H

#include <QObject>

class TST_VNestingFrame : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit TST_VNestingFrame(QObject *parent = nullptr);

private slots:
    void LocalToStripTurnsGrainlineToX();
    void RoundTripIsTranslationOnly_data();
    void RoundTripIsTranslationOnly();
    void IdentityPlacementSitsAtStripOrigin();
    void Rotation180KeepsBoundingBox();
    void MirrorFlipsHandedness();
    void FlippedFlag();
    void RequestShape();
    void RequestSkipsDegeneratePieces();
    void RequestFlippingRules();
    void PlanComplete();
    void RequestRotationsAndQuantity();
    void PlanCompleteWithQuantities();
    void GrainlineAlignment_data();
    void GrainlineAlignment();
    void GrainlineAlignmentIdentityWithoutGrainline();
    void LayoutBaseForceFlipped();
    void ResultPaperPlacesPieces();
    void ResultPaperCopies();
    void ExpandQuantityFlipsSymmetricalCopies();
    void RequestGroupsAndFixed();
    void FixedPolygonRoundTrip();
    void RequestSkipsByIndex();
    void ContourUsesSeamAllowance();
    void RequestWritesOptions();
    void RequestArea();
    void RequestWritesRolls();
    void RequestMergesIdenticalPieces();
    void ResultPapersSplitByRoll();
};

#endif // TST_VNESTINGFRAME_H
