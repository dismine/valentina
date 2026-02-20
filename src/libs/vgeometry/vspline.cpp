/************************************************************************
 **
 **  @file   vspline.cpp
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

#include "vspline.h"

#include <QJsonObject>
#include <QLineF>
#include <QStringLiteral>
#include <QVector2D>
#include <QtConcurrent>

#include "../ifc/ifcdef.h"
#include "../vmisc/def.h"
#include "../vmisc/defglobal.h"
#include "../vmisc/vmath.h"
#include "vspline_p.h"
#include "vsplinepath.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

struct VSplinePair
{
    QVector<QPointF> left{};
    QVector<QPointF> right{};
};

namespace
{
auto DRoots(const QVector<double> &p) -> QVector<double>
{
    // --- Quadratic case (three points) -------------------------------------
    if (p.size() == 3)
    {
        QVector<double> roots;
        double const a = p.at(0);
        double const b = p.at(1);
        double const c = p.at(2);
        double const d = a - 2.0 * b + c;
        if (!qFuzzyIsNull(d))
        {
            double const underSqrt = b * b - a * c;
            if (underSqrt < 0.0)
            {
                return {}; // no real roots
            }

            double const m1 = -std::sqrt(underSqrt);
            double const m2 = -a + b;
            double const v1 = -(m1 + m2) / d;
            double const v2 = -(-m1 + m2) / d;
            roots.append(v1);
            roots.append(v2);
        }
        else if (!VFuzzyComparePossibleNulls(b, c))
        {
            // d == 0, but b != c
            double const v = (2.0 * b - c) / (2.0 * (b - c));
            roots.append(v);
        }

        return roots;
    }

    // --- Linear case (two points) -----------------------------------------
    if (p.size() == 2)
    {
        QVector<double> roots;
        double const a = p.at(0);
        double const b = p.at(1);
        if (!VFuzzyComparePossibleNulls(a, b))
        {
            double const v = a / (a - b);
            roots.append(v);
        }

        return roots;
    }

    // --- Default: no roots -------------------------------------------------
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto Map(qreal v, qreal ds, qreal de, qreal rs, qreal re) noexcept -> qreal
{
    if (std::abs(de - ds) < 1e-12)
    {
        return rs;
    }
    const qreal d1 = de - ds;
    const qreal d2 = re - rs;
    const qreal v2 = v - ds;
    const qreal r = v2 / d1;

    return rs + d2 * r;
}

//---------------------------------------------------------------------------------------------------------------------
auto Compute(double t, const QVector<QPointF> &points) -> QPointF
{
    if (points.isEmpty())
    {
        return {};
    }

    // t == 0 → first point
    if (t <= 0.0)
    {
        return points.constFirst();
    }

    // t == 1 → last point
    if (t >= 1.0)
    {
        return points.constLast();
    }

    const vsizetype order = points.size() - 1;
    const double mt = 1.0 - t;

    // Constant
    if (order == 0)
    {
        return points.constFirst();
    }

    // Linear
    if (order == 1)
    {
        return {mt * points.at(0).x() + t * points.at(1).x(), mt * points.at(0).y() + t * points.at(1).y()};
    }

    // Quadratic or Cubic
    if (order < 4)
    {
        double const mt2 = mt * mt;
        double const t2 = t * t;
        double a = 0.0;
        double b = 0.0;
        double c = 0.0;
        double d = 0.0;
        QVector<QPointF> p = points;

        if (order == 2)
        {
            // promote to cubic by adding dummy zero point
            p.append(QPointF());
            a = mt2;
            b = mt * t * 2.0;
            c = t2;
        }
        else
        { // order == 3
            a = mt2 * mt;
            b = mt2 * t * 3.0;
            c = mt * t2 * 3.0;
            d = t * t2;
        }

        return {a * p.at(0).x() + b * p.at(1).x() + c * p.at(2).x() + d * p.at(3).x(),
                a * p.at(0).y() + b * p.at(1).y() + c * p.at(2).y() + d * p.at(3).y()};
    }

    // Higher-order case: generic De Casteljau evaluation
    QVector<QPointF> tmp = points;
    while (tmp.size() > 1)
    {
        for (int i = 0; i < tmp.size() - 1; ++i)
        {
            tmp[i].setX(tmp.at(i).x() + (tmp.at(i + 1).x() - tmp.at(i).x()) * t);
            tmp[i].setY(tmp.at(i).y() + (tmp.at(i + 1).y() - tmp.at(i).y()) * t);
        }
        tmp.removeLast();
    }

    return tmp.constFirst();
}

//---------------------------------------------------------------------------------------------------------------------
auto MakePositions(const QVector<qreal> &values) -> QVector<qreal>
{
    vsizetype const n = values.size();
    if (n == 0)
    {
        return {};
    }

    if (n == 1)
    {
        return {0.0}; // trivial
    }

    QVector<qreal> positions;
    positions.resize(n);

    qreal const step = 1.0 / static_cast<qreal>(n - 1);

    for (int i = 0; i < n; ++i)
    {
        positions[i] = step * i;
    }

    return positions;
}

//---------------------------------------------------------------------------------------------------------------------
auto SignedAngle(const QPointF &p1, const QPointF &p2, const QPointF &p3) -> qreal
{
    QPointF const v1 = p2 - p1;
    QPointF const v2 = p3 - p1;

    double const det = v1.x() * v2.y() - v1.y() * v2.x();
    double const dot = v1.x() * v2.x() + v1.y() * v2.y();
    return std::atan2(det, dot); // signed angle in radians
}

//---------------------------------------------------------------------------------------------------------------------
auto LineIntersection(const QPointF &p1, const QPointF &p2, const QPointF &p3, const QPointF &p4, QPointF &out) -> bool
{
    QLineF const l1(p1, p2);
    QLineF const l2(p3, p4);

    QPointF intersection;
    if (QLineF::IntersectionType const type = l1.intersects(l2, &intersection); type == QLineF::NoIntersection)
    {
        return false;
    }

    out = intersection;
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
void ApplyFallbackScaling(const QVector<QPointF> &pts, QVector<QPointF> &np)
{
    // Fallback: Translate control points relative to their endpoints.
    // This preserves the original tangent vectors' lengths and
    // directions relative to the new endpoints.
    // It's a simple, robust, and predictable approximation.

    // np[1] (P2_new) = P1_new + (P2_orig - P1_orig)
    np[1] = np.at(0) + (pts.at(1) - pts.at(0));

    // np[2] (P3_new) = P4_new + (P3_orig - P4_orig)
    np[2] = np.at(3) + (pts.at(2) - pts.at(3));
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VSpline default constructor
 */
VSpline::VSpline()
  : VAbstractCubicBezier(GOType::Spline),
    d(new VSplineData)
{
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL_2(VSpline, VAbstractCubicBezier)

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VSpline constructor.
 * @param p1 first point spline.
 * @param p4 last point spline.
 * @param angle1 angle from first point to first control point.
 * @param angle2 angle from second point to second control point.
 * @param kCurve coefficient of curvature spline.
 * @param kAsm1 coefficient of length first control line.
 * @param kAsm2 coefficient of length second control line.
 */
VSpline::VSpline(const VPointF &p1, const VPointF &p4, qreal angle1, qreal angle2, qreal kAsm1, qreal kAsm2,
                 qreal kCurve, quint32 idObject, Draw mode)
  : VAbstractCubicBezier(GOType::Spline, idObject, mode),
    d(new VSplineData(p1, p4, angle1, angle2, kAsm1, kAsm2, kCurve))
{
    VSpline::CreateName();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VSpline constructor.
 * @param p1 first point spline.
 * @param p2 first control point.
 * @param p3 second control point.
 * @param p4 second point spline.
 */
VSpline::VSpline(const VPointF &p1, const QPointF &p2, const QPointF &p3, const VPointF &p4, quint32 idObject,
                 Draw mode)
  : VAbstractCubicBezier(GOType::Spline, idObject, mode),
    d(new VSplineData(p1, p2, p3, p4))
{
    VSpline::CreateName();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VSpline constructor
 * @param p1 first point spline.
 * @param p4 first control point.
 * @param angle1 angle from first point to first control point.
 * @param angle1Formula formula angle from first point to first control point.
 * @param angle2 angle from second point to second control point.
 * @param angle2Formula formula angle from second point to second control point.
 * @param c1Length length from first point to first control point.
 * @param c1LengthFormula formula length from first point to first control point.
 * @param c2Length length from second point to first control point.
 * @param c2LengthFormula formula length from second point to first control point.
 */
VSpline::VSpline(const VPointF &p1, const VPointF &p4, qreal angle1, const QString &angle1Formula, qreal angle2,
                 const QString &angle2Formula, qreal c1Length, const QString &c1LengthFormula, qreal c2Length,
                 const QString &c2LengthFormula, quint32 idObject, Draw mode)
  : VAbstractCubicBezier(GOType::Spline, idObject, mode),
    d(new VSplineData(p1, p4, angle1, angle1Formula, angle2, angle2Formula, c1Length, c1LengthFormula, c2Length,
                      c2LengthFormula))
{
    VSpline::CreateName();
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::Rotate(const QPointF &originPoint, qreal degrees, const QString &name) const -> VSpline
{
    const VPointF p1 = GetP1().Rotate(originPoint, degrees, "X1"_L1);
    const VPointF p4 = GetP4().Rotate(originPoint, degrees, "X4"_L1);

    const QPointF p2 = VPointF::RotatePF(originPoint, static_cast<QPointF>(GetP2()), degrees);
    const QPointF p3 = VPointF::RotatePF(originPoint, static_cast<QPointF>(GetP3()), degrees);

    VSpline spl(p1, p2, p3, p4);
    if (!name.isEmpty())
    {
        spl.SetNameSuffix(name);
    }
    spl.SetColor(GetColor());
    spl.SetPenStyle(GetPenStyle());
    spl.SetApproximationScale(GetApproximationScale());
    spl.SetDerivative(true);
    return spl;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::Flip(const QLineF &axis, const QString &name) const -> VSpline
{
    const VPointF p1 = GetP1().Flip(axis, "X1"_L1);
    const VPointF p4 = GetP4().Flip(axis, "X4"_L1);

    const QPointF p2 = VPointF::FlipPF(axis, static_cast<QPointF>(GetP2()));
    const QPointF p3 = VPointF::FlipPF(axis, static_cast<QPointF>(GetP3()));

    VSpline spl(p1, p2, p3, p4);
    if (!name.isEmpty())
    {
        spl.SetNameSuffix(name);
    }
    spl.SetColor(GetColor());
    spl.SetPenStyle(GetPenStyle());
    spl.SetApproximationScale(GetApproximationScale());
    spl.SetDerivative(true);
    return spl;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::Move(qreal length, qreal angle, const QString &name) const -> VSpline
{
    const VPointF p1 = GetP1().Move(length, angle, "X1"_L1);
    const VPointF p4 = GetP4().Move(length, angle, "X4"_L1);

    const QPointF p2 = VPointF::MovePF(static_cast<QPointF>(GetP2()), length, angle);
    const QPointF p3 = VPointF::MovePF(static_cast<QPointF>(GetP3()), length, angle);

    VSpline spl(p1, p2, p3, p4);
    if (!name.isEmpty())
    {
        spl.SetNameSuffix(name);
    }
    spl.SetColor(GetColor());
    spl.SetPenStyle(GetPenStyle());
    spl.SetApproximationScale(GetApproximationScale());
    spl.SetDerivative(true);
    return spl;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::OffsetPath(qreal distance) const -> QVector<VSpline>
{
    QVector<VSpline> subSplines = OffsetCurve_r(distance);

    for (auto &spl : subSplines)
    {
        spl.SetColor(GetColor());
        spl.SetPenStyle(GetPenStyle());
        spl.SetApproximationScale(GetApproximationScale());
    }

    return subSplines;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::OutlinePath(const QVector<qreal> &distances) const -> QVector<VSpline>
{
    QVector<VSpline> subSplines = OutlineCurve(distances);

    for (auto &spl : subSplines)
    {
        spl.SetColor(GetColor());
        spl.SetPenStyle(GetPenStyle());
        spl.SetApproximationScale(GetApproximationScale());
    }

    return subSplines;
}

//---------------------------------------------------------------------------------------------------------------------
VSpline::~VSpline() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::Offset(qreal distance, const QString &name) const -> VSplinePath
{
    QVector<VSpline> const subSplines = OffsetPath(distance);

    VSplinePath splPath(subSplines);
    if (!name.isEmpty())
    {
        splPath.SetNameSuffix(name);
        splPath.SetMainNameForHistory(name);
    }

    splPath.SetColor(GetColor());
    splPath.SetPenStyle(GetPenStyle());
    splPath.SetApproximationScale(GetApproximationScale());
    splPath.SetDerivative(true);
    return splPath;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::Outline(const QVector<qreal> &distances, const QString &name) const -> VSplinePath
{
    QVector<VSpline> const subSplines = OutlinePath(distances);

    VSplinePath splPath(subSplines);
    if (!name.isEmpty())
    {
        splPath.SetNameSuffix(name);
        splPath.SetMainNameForHistory(name);
    }

    splPath.SetColor(GetColor());
    splPath.SetPenStyle(GetPenStyle());
    splPath.SetApproximationScale(GetApproximationScale());
    splPath.SetDerivative(true);
    return splPath;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetLength return length of spline.
 * @return length.
 */
auto VSpline::GetLength() const -> qreal
{
    return LengthBezier(static_cast<QPointF>(GetP1()), static_cast<QPointF>(GetP2()), static_cast<QPointF>(GetP3()),
                        static_cast<QPointF>(GetP4()), GetApproximationScale());
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::CutSpline(qreal length, VSpline &spl1, VSpline &spl2, const QString &pointName) const -> QPointF
{
    QPointF spl1p2;
    QPointF spl1p3;
    QPointF spl2p2;
    QPointF spl2p3;
    const QPointF cutPoint = CutSpline(length, spl1p2, spl1p3, spl2p2, spl2p3, pointName);

    spl1 = VSpline(GetP1(), spl1p2, spl1p3, VPointF(cutPoint, pointName));
    spl1.SetApproximationScale(GetApproximationScale());

    spl2 = VSpline(VPointF(cutPoint, pointName), spl2p2, spl2p3, GetP4());
    spl2.SetApproximationScale(GetApproximationScale());
    return cutPoint;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::CutSplineAtParam(qreal t, VSpline &left, VSpline &right, const QString &midPointName) const -> QPointF
{
    QPointF spl1p2;
    QPointF spl1p3;
    QPointF spl2p2;
    QPointF spl2p3;

    const QPointF cutPoint = CutSplineAtParam(t, spl1p2, spl1p3, spl2p2, spl2p3);

    left = VSpline(GetP1(), spl1p2, spl1p3, VPointF(cutPoint, midPointName));
    left.SetApproximationScale(GetApproximationScale());

    right = VSpline(VPointF(cutPoint, midPointName), spl2p2, spl2p3, GetP4());
    right.SetApproximationScale(GetApproximationScale());
    return cutPoint;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetPoints return list with spline points.
 * @return list of points.
 */
auto VSpline::GetPoints() const -> QVector<QPointF>
{
    return GetCubicBezierPoints(static_cast<QPointF>(GetP1()), static_cast<QPointF>(GetP2()),
                                static_cast<QPointF>(GetP3()), static_cast<QPointF>(GetP4()), GetApproximationScale());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SplinePoints return list with spline points.
 * @param p1 first spline point.
 * @param p4 last spline point.
 * @param angle1 angle from first point to first control point.
 * @param angle2 angle from second point to second control point.
 * @param kAsm1 coefficient of length first control line.
 * @param kAsm2 coefficient of length second control line.
 * @param kCurve coefficient of curvature spline.
 * @return list with spline points.
 */
// cppcheck-suppress unusedFunction
auto VSpline::SplinePoints(const QPointF &p1, const QPointF &p4, qreal angle1, qreal angle2, qreal kAsm1, qreal kAsm2,
                           qreal kCurve, qreal approximationScale) -> QVector<QPointF>
{
    QLineF p1pX(p1.x(), p1.y(), p1.x() + 100, p1.y());
    p1pX.setAngle(angle1);
    qreal L = 0;
    qreal radius = 0;
    constexpr qreal angle = 90;
    radius = QLineF(QPointF(p1.x(), p4.y()), p4).length();
    L = kCurve * radius * 4 / 3 * tan(angle * M_PI_4 / 180.0);
    QLineF p1p2(p1.x(), p1.y(), p1.x() + L * kAsm1, p1.y());
    p1p2.setAngle(angle1);
    QLineF p4p3(p4.x(), p4.y(), p4.x() + L * kAsm2, p4.y());
    p4p3.setAngle(angle2);
    QPointF const p2 = p1p2.p2();
    QPointF const p3 = p4p3.p2();
    return GetCubicBezierPoints(p1, p2, p3, p4, approximationScale);
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::operator=(const VSpline &spline) -> VSpline &
{
    if (&spline == this)
    {
        return *this;
    }
    VAbstractCubicBezier::operator=(spline);
    d = spline.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VSpline::VSpline(VSpline &&spline) noexcept
  : VAbstractCubicBezier(std::move(spline)),
    // cppcheck-suppress accessMoved
    d(std::move(spline.d)) // NOLINT(bugprone-use-after-move)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::operator=(VSpline &&spline) noexcept -> VSpline &
{
    VAbstractCubicBezier::operator=(spline);
    std::swap(d, spline.d);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetP1 return first spline point.
 * @return first point.
 */
auto VSpline::GetP1() const -> VPointF
{
    return d->p1;
}

//---------------------------------------------------------------------------------------------------------------------
void VSpline::SetP1(const VPointF &p)
{
    d->p1 = p;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetP2 return first control point.
 * @return first control point.
 */
auto VSpline::GetP2() const -> VPointF
{
    QLineF p1p2(d->p1.x(), d->p1.y(), d->p1.x() + d->c1Length, d->p1.y());
    p1p2.setAngle(d->angle1);
    return VPointF(p1p2.p2());
}

//---------------------------------------------------------------------------------------------------------------------
void VSpline::SetP2(const QPointF &p)
{
    QLineF const p1p2(d->p1.toQPointF(), p);

    d->angle1 = p1p2.angle();
    d->angle1F = QString::number(d->angle1);

    d->c1Length = p1p2.length();
    if (VAbstractValApplication::VApp() != nullptr)
    {
        d->c1LengthF = QString::number(VAbstractValApplication::VApp()->fromPixel(d->c1Length));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetP3 return second control point.
 * @return second control point.
 */
auto VSpline::GetP3() const -> VPointF
{
    QLineF p4p3(d->p4.x(), d->p4.y(), d->p4.x() + d->c2Length, d->p4.y());
    p4p3.setAngle(d->angle2);
    return VPointF(p4p3.p2());
}

//---------------------------------------------------------------------------------------------------------------------
void VSpline::SetP3(const QPointF &p)
{
    QLineF const p4p3(d->p4.toQPointF(), p);

    d->angle2 = p4p3.angle();
    d->angle2F = QString::number(d->angle2);

    d->c2Length = p4p3.length();
    if (VAbstractValApplication::VApp() != nullptr)
    {
        d->c2LengthF = QString::number(VAbstractValApplication::VApp()->fromPixel(d->c2Length));
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetP4 return last spline point.
 * @return остання точка сплайну.
 */
auto VSpline::GetP4() const -> VPointF
{
    return d->p4;
}

//---------------------------------------------------------------------------------------------------------------------
void VSpline::SetP4(const VPointF &p)
{
    d->p4 = p;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetAngle1 return first angle control line.
 * @return angle.
 */
auto VSpline::GetStartAngle() const -> qreal
{
    return d->angle1;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetAngle2 return second angle control line.
 * @return angle.
 */
auto VSpline::GetEndAngle() const -> qreal
{
    return d->angle2;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::GetStartAngleFormula() const -> QString
{
    return d->angle1F;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::GetEndAngleFormula() const -> QString
{
    return d->angle2F;
}

//---------------------------------------------------------------------------------------------------------------------
void VSpline::SetStartAngle(qreal angle, const QString &formula)
{
    d->angle1 = angle;
    d->angle1F = formula;
}

//---------------------------------------------------------------------------------------------------------------------
void VSpline::SetEndAngle(qreal angle, const QString &formula)
{
    d->angle2 = angle;
    d->angle2F = formula;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::GetC1Length() const -> qreal
{
    return d->c1Length;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::GetC2Length() const -> qreal
{
    return d->c2Length;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::GetC1LengthFormula() const -> QString
{
    return d->c1LengthF;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::GetC2LengthFormula() const -> QString
{
    return d->c2LengthF;
}

//---------------------------------------------------------------------------------------------------------------------
void VSpline::SetC1Length(qreal length, const QString &formula)
{
    d->c1Length = length;
    d->c1LengthF = formula;
}

//---------------------------------------------------------------------------------------------------------------------
void VSpline::SetC2Length(qreal length, const QString &formula)
{
    d->c2Length = length;
    d->c2LengthF = formula;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetKasm1 return coefficient of length first control line.
 * @return coefficient.
 */
auto VSpline::GetKasm1() const -> qreal
{
    return QLineF(static_cast<QPointF>(d->p1), static_cast<QPointF>(GetP2())).length() /
           VSplineData::GetL(static_cast<QPointF>(d->p1), static_cast<QPointF>(d->p4), d->kCurve);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetKasm2 return coefficient of length second control line.
 * @return coefficient.
 */
auto VSpline::GetKasm2() const -> qreal
{
    return QLineF(static_cast<QPointF>(d->p4), static_cast<QPointF>(GetP3())).length() /
           VSplineData::GetL(static_cast<QPointF>(d->p1), static_cast<QPointF>(d->p4), d->kCurve);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetKcurve return coefficient of curvature spline.
 * @return coefficient
 */
auto VSpline::GetKcurve() const -> qreal
{
    return d->kCurve;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::Sign(long double ld) -> int
{
    if (qAbs(ld) < 0.00000000001)
    {
        return 0;
    }
    return (ld > 0) ? 1 : -1;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::Reduce() const -> QVector<VSpline>
{
    // --- 1st pass: split on extrema ---------------------------------------
    QVector<double> extrema = this->Extrema();

    // ensure [0, 1] bounds included
    if (extrema.isEmpty() || !qFuzzyIsNull(extrema.constFirst()))
    {
        extrema.prepend(0.0);
    }
    if (!VFuzzyComparePossibleNulls(extrema.constLast(), 1.0))
    {
        extrema.append(1.0);
    }

    struct ExstremaData
    {
        VSpline spl{};
        qreal step{};
    };

    QVector<ExstremaData> pass1;
    pass1.reserve(extrema.size());

    // split curve into first-pass segments
    for (int i = 1; i < extrema.size(); ++i)
    {
        double const t1 = extrema.at(i - 1);
        double const t2 = extrema.at(i);
        VSpline const seg = this->SplitRange(t1, t2);
        // .step = std::clamp((t2 - t1) / 5000.0, 0.00000001, 0.01);
        pass1.append({.spl = seg, .step = 0.01});
    }

    // --- 2nd pass: further split into "simple" segments -------------------
    QVector<VSpline> pass2;
    pass2.reserve(pass1.size() * 1000);

    for (const ExstremaData &data : pass1)
    {
        double t1 = 0.0;
        // Define a precision for floating point checks, related to the step
        const double kPrecision = data.step / 16.0;

        while (t1 < 1.0 - kPrecision)
        {
            // Define the search range for our optimal t2
            double search_low = t1 + data.step;
            double search_high = 1.0;

            if (search_low >= 1.0 - kPrecision)
            {
                // Remaining segment is smaller than one step, just add it
                pass2.append(data.spl.SplitRange(t1, 1.0));
                break; // Finish this pass1 segment
            }

            // 1. Set the minimum step as the initial "last known good" segment.
            // If this segment isn't "simple", the binary search will fail
            // to find a larger one, and this (non-simple) segment
            // will be used, which is the desired behavior.
            VSpline last_good_spl = data.spl.SplitRange(t1, search_low);
            double last_good_t2 = search_low;

            // 2. Binary search for the largest t2
            while (search_high - search_low > kPrecision)
            {
                double const t_mid = search_low + (search_high - search_low) / 2.0;
                VSpline mid_seg = data.spl.SplitRange(t1, t_mid);

                if (mid_seg.IsSimple())
                {
                    // This segment is good. Try a larger one.
                    search_low = t_mid;
                    last_good_spl = std::move(mid_seg);
                    last_good_t2 = t_mid;
                }
                else
                {
                    // This segment is bad. Search in the lower half.
                    search_high = t_mid;
                }
            }

            // 3. Append the best (largest) linear segment we found
            pass2.append(std::move(last_good_spl));

            // 4. Advance t1 for the next search
            t1 = last_good_t2;
        }
    }

    return pass2;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::Extrema() const -> QVector<double>
{
    QVector<QVector<QPointF>> const dpoints = ComputeDerivatives();
    if (dpoints.isEmpty())
    {
        return {};
    }

    QVector<double> roots;

    // We’ll handle both X and Y dimensions
    for (int dim = 0; dim < 2; ++dim)
    {
        // Extract derivative control points for this dimension
        QVector<double> p;
        for (const QPointF &pt : dpoints.at(0))
        {
            p.append(dim == 0 ? pt.x() : pt.y());
        }

        // Find the roots of derivative (quadratic or linear)
        QVector<double> result = DRoots(p);

        if (dpoints.size() > 1)
        {
            QVector<double> p2;
            for (const QPointF &pt : dpoints.at(1))
            {
                p2.append(dim == 0 ? pt.x() : pt.y());
            }
            QVector<double> const extra = DRoots(p2);
            result += extra;
        }

        // Keep only valid t in [0,1]
        QVector<double> filtered;
        for (double const t : std::as_const(result))
        {
            if (t >= 0.0 && t <= 1.0)
            {
                filtered.append(t);
            }
        }

        // Append to global roots list
        for (double const t : std::as_const(filtered))
        {
            roots.append(t);
        }
    }

    // Sort and unique
    std::sort(roots.begin(), roots.end());
    roots.erase(std::unique(roots.begin(),
                            roots.end(),
                            [](double a, double b) -> bool { return VFuzzyComparePossibleNulls(a, b); }),
                roots.end());

    return roots;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::ComputeDerivatives() const -> QVector<QVector<QPointF>>
{
    QVector<QVector<QPointF>> dpoints;

    QVector<QPointF> points{GetP1().toQPointF(), GetP2().toQPointF(), GetP3().toQPointF(), GetP4().toQPointF()};
    int size = static_cast<int>(points.size());
    int c = size - 1;

    while (size > 1)
    {
        QVector<QPointF> list;
        list.reserve(c);

        for (int j = 0; j < c; ++j)
        {
            QPointF dpt;
            dpt.setX(c * (points.at(j + 1).x() - points.at(j).x()));
            dpt.setY(c * (points.at(j + 1).y() - points.at(j).y()));
            list.append(dpt);
        }

        dpoints.append(list);
        points = list;
        size = static_cast<int>(points.size());
        c = size - 1;
    }

    return dpoints;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::Hull(double t) const -> QVector<QPointF>
{
    QVector<QPointF> p{GetP1().toQPointF(), GetP2().toQPointF(), GetP3().toQPointF(), GetP4().toQPointF()};
    // flattened list of all intermediate points
    QVector<QPointF> q;
    q.reserve(9);

    q += p; // push all original control points in order

    // iterative linear interpolation
    while (p.size() > 1)
    {
        QVector<QPointF> next;
        next.reserve(p.size() - 1);
        for (int i = 0; i < p.size() - 1; ++i)
        {
            QPointF const pt(p[i].x() + (p[i + 1].x() - p[i].x()) * t, p[i].y() + (p[i + 1].y() - p[i].y()) * t);
            q.append(pt);
            next.append(pt);
        }
        p = std::move(next);
    }

    return q;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::SplitRange(double t1) const -> VSplinePair
{
    QVector<QPointF> const q = Hull(t1);
    QVector<QPointF> const left{q.at(0), q.at(4), q.at(7), q.at(9)};
    QVector<QPointF> const right{q.at(9), q.at(8), q.at(6), q.at(3)};

    return {left, right};
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::SplitRange(double t1, double t2) const -> VSpline
{
    // Shortcuts --------------------------------------------------------------
    if (qFuzzyIsNull(t1) && !qFuzzyIsNull(t2))
    {
        QVector<QPointF> const left = SplitRange(t2).left;
        VSpline leftSpl{VPointF(left.at(0), "X1"_L1), left.at(1), left.at(2), VPointF(left.at(3), "X4"_L1)};
        leftSpl.SetApproximationScale(GetApproximationScale());
        return leftSpl;
    }
    if (VFuzzyComparePossibleNulls(t2, 1.0))
    {
        QVector<QPointF> const right = SplitRange(t1).right;
        VSpline rightSpl{VPointF(right.at(0), "X1"_L1), right.at(1), right.at(2), VPointF(right.at(3), "X4"_L1)};
        rightSpl.SetApproximationScale(GetApproximationScale());
        return rightSpl;
    }

    auto result = SplitRange(t1);
    double const t2mapped = Map(t2, t1, 1.0, 0.0, 1.0);

    QVector<QPointF> const right = result.right;
    VSpline rightSpl{VPointF(right.at(0), "X1"_L1), right.at(1), right.at(2), VPointF(right.at(3), "X4"_L1)};
    rightSpl.SetApproximationScale(GetApproximationScale());

    QVector<QPointF> const left = rightSpl.SplitRange(t2mapped).left;
    VSpline leftSpl{VPointF(left.at(0), "X1"_L1), left.at(1), left.at(2), VPointF(left.at(3), "X4"_L1)};
    leftSpl.SetApproximationScale(GetApproximationScale());
    return leftSpl;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::IsSimple() const -> bool
{
    {
        QVector<QPointF> const pts{GetP1().toQPointF(), GetP2().toQPointF(), GetP3().toQPointF(), GetP4().toQPointF()};

        double const a1 = SignedAngle(pts.at(0), pts.at(3), pts.at(1));
        double const a2 = SignedAngle(pts.at(0), pts.at(3), pts.at(2));

        // Opposite curvature directions → not simple
        if ((a1 > 0 && a2 < 0) || (a1 < 0 && a2 > 0))
        {
            return false;
        }
    }

    // Compute 2D normals at t = 0 and t = 1
    QVector2D const n1 = Normal(0.0);
    QVector2D const n2 = Normal(1.0);

    // Dot product of normals
    double s = QVector2D::dotProduct(n1, n2);

    // Clamp to prevent NaNs due to rounding
    s = std::clamp(s, -1.0, 1.0);

    // Angle between normals
    double const angle = std::abs(std::acos(s));

    auto GetApproximationAngle = [this]() -> double
    {
        qreal approximationScale = GetApproximationScale();
        if (approximationScale < minCurveApproximationScale || approximationScale > maxCurveApproximationScale)
        {
            approximationScale = VAbstractApplication::VApp()->GlobalCurveApproximationScale();
        }

        if (approximationScale <= 2)
        {
            return 60.;
        }

        if (approximationScale <= 4)
        {
            return 30.;
        }

        if (approximationScale <= 6)
        {
            return 15.;
        }

        if (approximationScale <= 8)
        {
            return 7.5;
        }

        return 3.75;
    };

    // Curve is simple if the normals differ by less than angle value
    return angle < qDegreesToRadians(GetApproximationAngle());
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::Derivative(double t) const -> QPointF
{
    QVector<QVector<QPointF>> const dpoints = ComputeDerivatives();
    if (dpoints.isEmpty())
    {
        return {};
    }

    return Compute(t, dpoints.at(0));
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::OffsetCurve_r(double distance) const -> QVector<VSpline>
{
    QVector<VSpline> result;

    // Linear curve case -------------------------------------
    if (IsLinear())
    {
        QVector<QPointF> const pts{GetP1().toQPointF(), GetP2().toQPointF(), GetP3().toQPointF(), GetP4().toQPointF()};
        QVector2D const n = Normal(0.0);
        QVector<QPointF> coords;
        coords.reserve(pts.size());

        for (const QPointF &p : pts)
        {
            coords.append(QPointF(p.x() + distance * n.x(), p.y() + distance * n.y()));
        }

        VSpline spl(VPointF(coords.at(0), "X1"_L1), coords.at(1), coords.at(2), VPointF(coords.at(3), "X4"_L1));
        spl.SetApproximationScale(GetApproximationScale());
        result.append(spl);
        return result;
    }

    // Non-linear case ---------------------------------------
    QVector<VSpline> reduced = Reduce();
    result.reserve(reduced.size());

    for (const VSpline &seg : std::as_const(reduced))
    {
        if (seg.IsLinear())
        {
            QVector<VSpline> const tmp = seg.OffsetCurve_r(distance);
            result.append(tmp.constFirst());
        }
        else
        {
            result.append(seg.Scale(distance));
        }
    }

    return result;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::OffsetPoint(double t, double d) const -> QPointF
{
    QPointF const c = PointAt(t);
    QVector2D const n = Normal(t);

    return {c.x() + n.x() * d, c.y() + n.y() * d};
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::IsLinear() const -> bool
{
    QPointF const p1 = GetP1().toQPointF();
    QPointF const p2 = GetP2().toQPointF();
    QPointF const p3 = GetP3().toQPointF();
    QPointF const p4 = GetP4().toQPointF();

    return IsPointOnLineviaPDP(p2, p1, p4, MmToPixel(0.000000001))
           && IsPointOnLineviaPDP(p3, p1, p4, MmToPixel(0.000000001));
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::Scale(double distance) const -> VSpline
{
    // numeric distance → wrap into constant function
    auto Fn = [distance](qreal) noexcept -> double { return distance; };
    return Scale(Fn, false);
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::Scale(const std::function<qreal(qreal)> &distanceFn, bool functionMode) const -> VSpline
{
    QVector<QPointF> const pts{GetP1().toQPointF(), GetP2().toQPointF(), GetP3().toQPointF(), GetP4().toQPointF()};
    QVector<QPointF> np(pts.size());

    // Step 1-2: Scale endpoints
    ScaleEndpoints(pts, np, distanceFn);

    // Step 3-4: Handle control points
    QPointF focalPoint;
    if (bool const hasFocalPoint = TryFindFocalPoint(focalPoint); hasFocalPoint)
    {
        if (!ScaleControlPointsWithFocalPoint(pts, np, focalPoint, distanceFn, functionMode))
        {
            ApplyFallbackScaling(pts, np);
        }
    }
    else
    {
        ApplyFallbackScaling(pts, np);
    }

    // --- 5. Create new spline (Unchanged) ---
    VSpline spl(VPointF(np.at(0), "X1"_L1), np.at(1), np.at(2), VPointF(np.at(3), "X4"_L1));
    spl.SetApproximationScale(GetApproximationScale());
    return spl;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::OutlineCurve(const QVector<qreal> &distances) const -> QVector<VSpline>
{
    if (distances.isEmpty())
    {
        return {};
    }

    if (distances.size() == 1)
    {
        return OffsetCurve_r(distances.constFirst());
    }

    QVector<VSpline> const reduced = Reduce();

    QVector<VSpline> result;
    result.reserve(reduced.size());

    QVector<qreal> const positions = MakePositions(distances);

    auto PiecewiseDistance = [=](double v) -> double
    {
        // v is in [0,1]
        for (int i = 0; i < positions.size() - 1; ++i)
        {
            double const t0 = positions[i];
            double const t1 = positions[i + 1];

            if (v >= t0 && v <= t1)
            {
                return Map(v, t0, t1, distances[i], distances[i + 1]);
            }
        }
        return distances.last(); // fallback
    };

    qreal const totalLen = GetLength();
    qreal accumulated = 0.0;

    for (const VSpline &seg : reduced)
    {
        qreal const segLen = seg.GetLength();

        // Normalize length for this segment’s start
        qreal const v0 = accumulated / totalLen;
        qreal const v1 = (accumulated + segLen) / totalLen;

        // Thickness for start and end of this segment
        qreal const dStart = PiecewiseDistance(v0);
        qreal const dEnd = PiecewiseDistance(v1);

        // Convert to linear-distance-function (scaled across v=0..1)
        auto DistanceFn = [dStart, dEnd](qreal t) noexcept -> qreal { return Map(t, 0.0, 1.0, dStart, dEnd); };

        result.append(seg.Scale(DistanceFn));

        accumulated += segLen;
    }

    // Smooth the joints
    SmoothJoints(result);

    return result;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::IsClockwise() const -> bool
{
    QVector<QPointF> const pts{GetP1().toQPointF(), GetP2().toQPointF(), GetP3().toQPointF(), GetP4().toQPointF()};
    double const angle = SignedAngle(pts.at(0), pts.at(3), pts.at(1));
    return angle > 0;
}

//---------------------------------------------------------------------------------------------------------------------
void VSpline::ScaleEndpoints(const QVector<QPointF> &pts,
                             QVector<QPointF> &np,
                             const std::function<qreal(qreal)> &distanceFn) const
{
    // --- 1. Endpoint scale distances r1=r(0), r2=r(1) ---
    qreal const r1 = distanceFn(0.0);
    qreal const r2 = distanceFn(1.0);

    // --- 2. Move end points along their normals (Unchanged) ---
    for (int t = 0; t <= 1; ++t)
    {
        QPointF const p = (t == 0 ? pts.constFirst() : pts.constLast());
        QVector2D const n = Normal(t);
        qreal const distance = (t == 0 ? r1 : r2);
        np[t * (pts.size() - 1)] = QPointF(p.x() + distance * n.x(), p.y() + distance * n.y());
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::TryFindFocalPoint(QPointF &o) const -> bool
{
    auto const v0 = OffsetPoint(0.0, 10.0);
    auto const v1 = OffsetPoint(1.0, 10.0);
    QPointF const c0 = PointAt(0.0);
    QPointF const c1 = PointAt(1.0);

    // Try to find the intersection of the endpoint normals
    return LineIntersection(v0, c0, v1, c1, o);
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::ScaleControlPointsWithFocalPoint(const QVector<QPointF> &pts,
                                               QVector<QPointF> &np,
                                               const QPointF &focalPoint,
                                               const std::function<qreal(qreal)> &distanceFn,
                                               bool functionMode) const -> bool
{
    if (functionMode)
    {
        return ScaleControlPointsFunctionMode(pts, np, focalPoint, distanceFn);
    }
    return ScaleControlPointsNormalMode(pts, np, focalPoint);
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::ScaleControlPointsFunctionMode(const QVector<QPointF> &pts,
                                             QVector<QPointF> &np,
                                             const QPointF &focalPoint,
                                             const std::function<qreal(qreal)> &distanceFn) const -> bool
{
    for (int t = 0; t <= 1; ++t)
    {
        QPointF const pOrig = pts.at(t + 1);

        // Distance for this control point uses t=(k+1)/3
        qreal rc = distanceFn(static_cast<qreal>(t + 1) / 3);

        // If curve orientation is reversed, invert distance
        if (!IsClockwise())
        {
            rc = -rc;
        }

        QPointF ov(pOrig.x() - focalPoint.x(), pOrig.y() - focalPoint.y());
        qreal const m = std::hypot(ov.x(), ov.y());

        if (qFuzzyIsNull(m))
        {
            return false;
        }

        ov /= m;
        np[t + 1] = QPointF(pOrig.x() + rc * ov.x(), pOrig.y() + rc * ov.y());
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::ScaleControlPointsNormalMode(const QVector<QPointF> &pts,
                                           QVector<QPointF> &np,
                                           const QPointF &focalPoint) const -> bool
{
    for (int t = 0; t <= 1; ++t)
    {
        QPointF const p = (t == 0 ? np.constFirst() : np.constLast());
        QPointF const der = Derivative(t);
        QPointF const p2(p.x() + der.x(), p.y() + der.y());

        QPointF intersection;
        if (!LineIntersection(p, p2, focalPoint, pts.at(t + 1), intersection))
        {
            return false; // This intersection failed (e.g., parallel lines)
        }
        np[t + 1] = intersection;
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::Normal(double t) const -> QVector2D
{
    QPointF const der = Derivative(t); // 1. get tangent vector (dx/dt, dy/dt)

    // 2. compute its length (q = sqrt(dx² + dy²))
    qreal const q = std::sqrt(der.x() * der.x() + der.y() * der.y());
    if (qFuzzyIsNull(q))
    {
        return {0, 0}; // degenerate tangent
    }

    // 3. perpendicular rotation (-dy, dx) normalized by length
    return {static_cast<float>(-der.y() / q), static_cast<float>(der.x() / q)};
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::PointAt(double t) const -> QPointF
{
    QVector<QPointF> const pts{GetP1().toQPointF(), GetP2().toQPointF(), GetP3().toQPointF(), GetP4().toQPointF()};
    return Compute(t, pts);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Cubic Cubic equation solution. Real coefficients case.
 *
 * This method use method Vieta-Cardano for eval cubic equations.
 * Cubic equation write in form x3+a*x2+b*x+c=0.
 *
 * Output:
 * 3 real roots -> then x is filled with them;
 * 1 real + 2 complex -> x[0] is real, x[1] is real part of complex roots, x[2] - non-negative imaginary part.
 *
 * @param x solution array (size 3).
 * @param a coefficient
 * @param b coefficient
 * @param c coefficient
 * @return 3 - 3 real roots;
 *         1 - 1 real root + 2 complex;
 *         2 - 1 real root + complex roots imaginary part is zero (i.e. 2 real roots).
 */
auto VSpline::Cubic(QVector<qreal> &x, qreal a, qreal b, qreal c) -> qint32
{
    // To find cubic equation roots in the case of real coefficients, calculated at the beginning
    const qreal q = (pow(a, 2) - 3 * b) / 9.;
    const qreal r = (2 * pow(a, 3) - 9 * a * b + 27. * c) / 54.;
    if (pow(r, 2) < pow(q, 3))
    { // equation has three real roots, use formula Vieta
        const qreal t = acos(r / sqrt(pow(q, 3))) / 3.;
        x.insert(0, -2. * sqrt(q) * cos(t) - a / 3);
        x.insert(1, -2. * sqrt(q) * cos(t + (2 * M_2PI / 3.)) - a / 3.);
        x.insert(2, -2. * sqrt(q) * cos(t - (2 * M_2PI / 3.)) - a / 3.);
        return (3);
    }

    // 1 real root + 2 complex
    // Formula Cardano
    const qreal aa = -Sign(r) * pow(fabs(r) + sqrt(pow(r, 2) - pow(q, 3)), 1. / 3.);
    const qreal bb = Sign(aa) == 0 ? 0 : q / aa;

    x.insert(0, aa + bb - a / 3.);                 // Real root
    x.insert(1, (-0.5) * (aa + bb) - a / 3.);      // Complex root
    x.insert(2, (sqrt(3.) * 0.5) * fabs(aa - bb)); // Complex root
    if (qFuzzyIsNull(x.at(2)))
    {
        return (2);
    }
    return (1);
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::CalcT(qreal curveCoord1, qreal curveCoord2, qreal curveCoord3, qreal curveCoord4, qreal pointCoord)
    -> QVector<qreal>
{
    const qreal a = -curveCoord1 + 3 * curveCoord2 - 3 * curveCoord3 + curveCoord4;
    const qreal b = 3 * curveCoord1 - 6 * curveCoord2 + 3 * curveCoord3;
    const qreal c = -3 * curveCoord1 + 3 * curveCoord2;
    const qreal d = -pointCoord + curveCoord1;

    auto t = QVector<qreal>(3, -1);
    Cubic(t, b / a, c / a, d / a);

    QVector<qreal> retT;
    retT.reserve(t.size());
    for (auto i : std::as_const(t))
    {
        if (i >= 0 && i <= 1)
        {
            retT.append(i);
        }
    }

    return retT;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VSpline::ParamT calculate t coeffient that reprezent point on curve.
 *
 * Each point that belongs to Cubic Bézier curve can be shown by coefficient in interval [0; 1].
 *
 * @param pBt point on curve
 * @return t coeffient that reprezent this point on curve. Return -1 if point doesn't belongs to curve.
 */
auto VSpline::ParamT(const QPointF &pBt) const -> qreal
{
    QVector<qreal> ts;
    // Calculate t coefficient for each axis
    ts += CalcT(GetP1().x(), GetP2().x(), GetP3().x(), GetP4().x(), pBt.x());
    ts += CalcT(GetP1().y(), GetP2().y(), GetP3().y(), GetP4().y(), pBt.y());

    if (ts.isEmpty())
    {
        return -1; // We don't have candidates
    }

    qreal tx = -1;
    qreal eps = 3; // Error calculation

    // In morst case we will have 6 result in interval [0; 1].
    // Here we try find closest to our point.
    for (auto t : std::as_const(ts))
    {
        const auto p0 = static_cast<QPointF>(GetP1());
        const auto p1 = static_cast<QPointF>(GetP2());
        const auto p2 = static_cast<QPointF>(GetP3());
        const auto p3 = static_cast<QPointF>(GetP4());
        // The explicit form of the Cubic Bézier curve
        const qreal pointX = pow(1 - t, 3) * p0.x() + 3 * pow(1 - t, 2) * t * p1.x() +
                             3 * (1 - t) * pow(t, 2) * p2.x() + pow(t, 3) * p3.x();
        const qreal pointY = pow(1 - t, 3) * p0.y() + 3 * pow(1 - t, 2) * t * p1.y() +
                             3 * (1 - t) * pow(t, 2) * p2.y() + pow(t, 3) * p3.y();

        const QLineF line(pBt, QPointF(pointX, pointY));
        if (line.length() <= eps)
        {
            tx = t;
            eps = line.length(); // Next point should be even closest
        }
    }

    return tx;
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::ToJson() const -> QJsonObject
{
    QJsonObject object = VAbstractCubicBezier::ToJson();
    object["aScale"_L1] = GetApproximationScale();
    object["p1"_L1] = GetP1().ToJson();
    object["p4"_L1] = GetP4().ToJson();
    object["angle1"_L1] = GetStartAngle();
    object["angle1Formula"_L1] = GetStartAngleFormula();
    object["angle2"_L1] = GetEndAngle();
    object["angle2Formula"_L1] = GetEndAngleFormula();
    object["c1Length"_L1] = GetC1Length();
    object["c1LengthFormula"_L1] = GetC1LengthFormula();
    object["c2Length"_L1] = GetC2Length();
    object["c2LengthFormula"_L1] = GetC2LengthFormula();
    return object;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress constParameterReference
void VSpline::SmoothJoints(QVector<VSpline> &path)
{
    // Smooth the joints
    // We adjust the control points at the seam between result[i] and result[i+1]
    for (int i = 0; i < path.size() - 1; ++i)
    {
        VSpline &prev = path[i];
        VSpline &next = path[i + 1];

        // Get the join point
        QPointF const anchor = prev.GetP4().toQPointF();

        // Get incoming handle (from previous segment) and outgoing handle (to next segment)
        QPointF const p3 = prev.GetP3().toQPointF();
        QPointF const p2_next = next.GetP2().toQPointF();

        // Vectors from the anchor
        QVector2D vIn(anchor - p3);
        QVector2D vOut(p2_next - anchor);

        // Calculate lengths (to preserve the curvature intensity as much as possible)
        qreal const lenIn = vIn.length();
        qreal const lenOut = vOut.length();

        if (qFuzzyIsNull(lenIn) || qFuzzyIsNull(lenOut))
        {
            continue;
        }

        // Normalize to get directions
        vIn.normalize();
        vOut.normalize();

        // Calculate the average tangent vector
        // We add them. If they are perfectly collinear (smooth), no change happens.
        // If there is a kink, this finds the middle ground.
        QVector2D avgTangent = vIn + vOut;

        if (qFuzzyIsNull(avgTangent.length()))
        {
            // Corner case: 180 degree turn (cusp). Don't smooth.
            continue;
        }
        avgTangent.normalize();

        // Apply the new smooth tangent
        // Move Previous P3
        QPointF const newP3 = anchor - (avgTangent.toPointF() * lenIn);
        prev.SetP3(newP3);

        // Move Next P2
        QPointF const newP2_next = anchor + (avgTangent.toPointF() * lenOut);
        next.SetP2(newP2_next);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::GetControlPoint1() const -> QPointF
{
    return static_cast<QPointF>(GetP2());
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::GetControlPoint2() const -> QPointF
{
    return static_cast<QPointF>(GetP3());
}

//---------------------------------------------------------------------------------------------------------------------
auto VSpline::GetRealLength() const -> qreal
{
    return LengthBezier(static_cast<QPointF>(GetP1()), static_cast<QPointF>(GetP2()), static_cast<QPointF>(GetP3()),
                        static_cast<QPointF>(GetP4()), maxCurveApproximationScale);
}
