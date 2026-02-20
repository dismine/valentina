/************************************************************************
 **
 **  @file   vspline.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#ifndef VSPLINE_H
#define VSPLINE_H

#include <QLineF>
#include <QMetaType>
#include <QPointF>
#include <QSharedDataPointer>
#include <QString>
#include <QTypeInfo>
#include <QVector>
#include <QtGlobal>

#include "vabstractcubicbezier.h"
#include "vpointf.h"

class VSplineData;
struct VSplinePair;
class VSplinePath;

/**
 * @brief VSpline class that implements the spline.
 */
class VSpline final : public VAbstractCubicBezier
{
public:
    VSpline();
    VSpline(const VSpline &spline);
    VSpline(const VPointF &p1, const VPointF &p4, qreal angle1, qreal angle2, qreal kAsm1, qreal kAsm2, qreal kCurve,
            quint32 idObject = 0, Draw mode = Draw::Calculation);
    VSpline(const VPointF &p1, const QPointF &p2, const QPointF &p3, const VPointF &p4, quint32 idObject = 0,
            Draw mode = Draw::Calculation);
    VSpline(const VPointF &p1, const VPointF &p4, qreal angle1, const QString &angle1Formula, qreal angle2,
            const QString &angle2Formula, qreal c1Length, const QString &c1LengthFormula, qreal c2Length,
            const QString &c2LengthFormula, quint32 idObject = 0, Draw mode = Draw::Calculation);
    ~VSpline() override;

    auto Rotate(const QPointF &originPoint, qreal degrees, const QString &name = QString()) const -> VSpline;
    auto Flip(const QLineF &axis, const QString &name = QString()) const -> VSpline;
    auto Move(qreal length, qreal angle, const QString &name = QString()) const -> VSpline;
    auto Offset(qreal distance, const QString &name = QString()) const -> VSplinePath override;
    auto Outline(const QVector<qreal> &distances, const QString &name = QString()) const -> VSplinePath override;

    auto OffsetPath(qreal distance) const -> QVector<VSpline>;
    auto OutlinePath(const QVector<qreal> &distances) const -> QVector<VSpline>;

    auto operator=(const VSpline &spline) -> VSpline &;

    VSpline(VSpline &&spline) noexcept;
    auto operator=(VSpline &&spline) noexcept -> VSpline &;

    auto GetP1() const -> VPointF override;
    void SetP1(const VPointF &p);

    auto GetP2() const -> VPointF override;
    void SetP2(const QPointF &p);

    auto GetP3() const -> VPointF override;
    void SetP3(const QPointF &p);

    auto GetP4() const -> VPointF override;
    void SetP4(const VPointF &p);

    auto GetStartAngle() const -> qreal override;
    auto GetEndAngle() const -> qreal override;

    auto GetStartAngleFormula() const -> QString;
    auto GetEndAngleFormula() const -> QString;

    void SetStartAngle(qreal angle, const QString &formula);
    void SetEndAngle(qreal angle, const QString &formula);

    auto GetC1Length() const -> qreal override;
    auto GetC2Length() const -> qreal override;

    auto GetC1LengthFormula() const -> QString;
    auto GetC2LengthFormula() const -> QString;

    void SetC1Length(qreal length, const QString &formula);
    void SetC2Length(qreal length, const QString &formula);

    auto GetLength() const -> qreal override;
    auto GetKasm1() const -> qreal;
    auto GetKasm2() const -> qreal;
    auto GetKcurve() const -> qreal;

    using VAbstractCubicBezier::CutSpline;
    auto CutSpline(qreal length, VSpline &spl1, VSpline &spl2, const QString &pointName) const -> QPointF;
    using VAbstractCubicBezier::CutSplineAtParam;
    auto CutSplineAtParam(qreal t, VSpline &left, VSpline &right, const QString &midPointName = QString()) const
        -> QPointF;

    auto GetPoints() const -> QVector<QPointF> override;
    // cppcheck-suppress unusedFunction
    static auto SplinePoints(const QPointF &p1, const QPointF &p4, qreal angle1, qreal angle2, qreal kAsm1, qreal kAsm2,
                             qreal kCurve, qreal approximationScale) -> QVector<QPointF>;
    auto ParamT(const QPointF &pBt) const -> qreal;

    auto ToJson() const -> QJsonObject override;

    static void SmoothJoints(QVector<VSpline> &path);

protected:
    auto GetControlPoint1() const -> QPointF override;
    auto GetControlPoint2() const -> QPointF override;
    auto GetRealLength() const -> qreal override;

private:
    QSharedDataPointer<VSplineData> d;
    static auto CalcT(qreal curveCoord1, qreal curveCoord2, qreal curveCoord3, qreal curveCoord4, qreal pointCoord)
        -> QVector<qreal>;
    static auto Cubic(QVector<qreal> &x, qreal a, qreal b, qreal c) -> qint32;
    static auto Sign(long double ld) -> int;

    auto Reduce() const -> QVector<VSpline>;
    auto Extrema() const -> QVector<double>;
    auto ComputeDerivatives() const -> QVector<QVector<QPointF>>;
    auto Hull(double t) const -> QVector<QPointF>;
    auto SplitRange(double t1) const -> VSplinePair;
    auto SplitRange(double t1, double t2) const -> VSpline;
    auto IsSimple() const -> bool;
    auto Normal(double t) const -> QVector2D;
    auto PointAt(double t) const -> QPointF;
    auto Derivative(double t) const -> QPointF;
    auto OffsetCurve_r(double distance) const -> QVector<VSpline>;
    auto OffsetPoint(double t, double d) const -> QPointF;
    auto IsLinear() const -> bool;
    auto Scale(double distance) const -> VSpline;
    auto Scale(const std::function<qreal(qreal)> &distanceFn, bool functionMode = true) const -> VSpline;
    auto OutlineCurve(const QVector<qreal> &distances) const -> QVector<VSpline>;
    auto IsClockwise() const -> bool;
    void ScaleEndpoints(const QVector<QPointF> &pts,
                        QVector<QPointF> &np,
                        const std::function<qreal(qreal)> &distanceFn) const;
    auto TryFindFocalPoint(QPointF &o) const -> bool;
    auto ScaleControlPointsWithFocalPoint(const QVector<QPointF> &pts,
                                          QVector<QPointF> &np,
                                          const QPointF &focalPoint,
                                          const std::function<qreal(qreal)> &distanceFn,
                                          bool functionMode) const -> bool;
    auto ScaleControlPointsFunctionMode(const QVector<QPointF> &pts,
                                        QVector<QPointF> &np,
                                        const QPointF &focalPoint,
                                        const std::function<qreal(qreal)> &distanceFn) const -> bool;
    auto ScaleControlPointsNormalMode(const QVector<QPointF> &pts, QVector<QPointF> &np, const QPointF &focalPoint) const
        -> bool;
};

Q_DECLARE_METATYPE(VSpline)                  // NOLINT
Q_DECLARE_TYPEINFO(VSpline, Q_MOVABLE_TYPE); // NOLINT

#endif // VSPLINE_H
