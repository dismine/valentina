/************************************************************************
 **
 **  @file   vpatternrecipe.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 7, 2019
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2019 Valentina project
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
#ifndef VPATTERNRECIPE_H
#define VPATTERNRECIPE_H

#include "../ifc/xml/vdomdocument.h"

class VContainer;
class VAbstractPattern;
class VMeasurement;
class VIncrement;
class VToolRecord;
class VFormula;
class VAbstractOperation;
struct VFinalMeasurement;

class VPatternRecipe : public VDomDocument
{
    Q_DECLARE_TR_FUNCTIONS(VPatternRecipe) // NOLINT
public:
    explicit VPatternRecipe(VAbstractPattern *pattern, QObject *parent = nullptr);
    virtual ~VPatternRecipe() = default;

private:
    Q_DISABLE_COPY_MOVE(VPatternRecipe) // NOLINT

    VAbstractPattern *m_pattern;

    auto Prerequisite() -> QDomElement;

    auto Measurements() -> QDomElement;
    auto Measurement(const QSharedPointer<VMeasurement> &m) -> QDomElement;

    auto Increments() -> QDomElement;
    auto PreviewCalculations() -> QDomElement;
    auto Increment(const QSharedPointer<VIncrement> &incr) -> QDomElement;

    auto Content() -> QDomElement;

    auto Draft(const QDomElement &draft) -> QDomElement;
    auto Step(const VToolRecord &tool, const VContainer &data) -> QDomElement;

    auto FinalMeasurements() -> QDomElement;
    auto FinalMeasurement(const VFinalMeasurement &fm, const VContainer &data) -> QDomElement;

    auto BasePoint(const VToolRecord &record) -> QDomElement;
    auto EndLine(const VToolRecord &record) -> QDomElement;
    auto Line(const VToolRecord &record) -> QDomElement;
    auto AlongLine(const VToolRecord &record) -> QDomElement;
    auto ShoulderPoint(const VToolRecord &record) -> QDomElement;
    auto Normal(const VToolRecord &record) -> QDomElement;
    auto Bisector(const VToolRecord &record) -> QDomElement;
    auto LineIntersect(const VToolRecord &record) -> QDomElement;
    auto Spline(const VToolRecord &record) -> QDomElement;
    auto CubicBezier(const VToolRecord &record) -> QDomElement;
    auto Arc(const VToolRecord &record) -> QDomElement;
    auto ArcWithLength(const VToolRecord &record) -> QDomElement;
    auto SplinePath(const VToolRecord &record) -> QDomElement;
    auto CubicBezierPath(const VToolRecord &record) -> QDomElement;
    auto PointOfContact(const VToolRecord &record) -> QDomElement;
    auto Height(const VToolRecord &record) -> QDomElement;
    auto Triangle(const VToolRecord &record) -> QDomElement;
    auto PointOfIntersection(const VToolRecord &record) -> QDomElement;
    auto CutArc(const VToolRecord &record) -> QDomElement;
    auto CutSpline(const VToolRecord &record) -> QDomElement;
    auto CutSplinePath(const VToolRecord &record) -> QDomElement;
    auto LineIntersectAxis(const VToolRecord &record) -> QDomElement;
    auto CurveIntersectAxis(const VToolRecord &record) -> QDomElement;
    auto PointOfIntersectionArcs(const VToolRecord &record) -> QDomElement;
    auto PointOfIntersectionCircles(const VToolRecord &record) -> QDomElement;
    auto PointOfIntersectionCurves(const VToolRecord &record) -> QDomElement;
    auto PointFromCircleAndTangent(const VToolRecord &record) -> QDomElement;
    auto PointFromArcAndTangent(const VToolRecord &record) -> QDomElement;
    auto TrueDarts(const VToolRecord &record) -> QDomElement;
    auto EllipticalArc(const VToolRecord &record) -> QDomElement;
    auto Rotation(const VToolRecord &record, const VContainer &data) -> QDomElement;
    auto FlippingByLine(const VToolRecord &record, const VContainer &data) -> QDomElement;
    auto FlippingByAxis(const VToolRecord &record, const VContainer &data) -> QDomElement;
    auto Move(const VToolRecord &record, const VContainer &data) -> QDomElement;
    auto EllipticalArcWithLength(const VToolRecord &record) -> QDomElement;

    void Formula(QDomElement &step, const VFormula &formula, const QString &formulaStr, const QString &formulaValue);

    template <typename T>
    void ToolAttributes(QDomElement &step, T* tool);

    template <typename T>
    void LineAttributes(QDomElement &step, T* tool);

    template <typename T>
    void CurveAttributes(QDomElement &step, T* tool);

    template <typename T>
    void CutCurveAttributes(QDomElement &step, T* tool);

    auto GroupOperationSource(VAbstractOperation *tool, quint32 id, const VContainer &data) -> QDomElement;
};

#endif // VPATTERNRECIPE_H
