/************************************************************************
 **
 **  @file   vellipticalarc.cpp
 **  @author Valentina Zhuravska <zhuravska19(at)gmail.com>
 **  @date   February 1, 2016
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

#include "vellipticalarc.h"

#include <optional>
#include <QLineF>
#include <QPainterPath>
#include <QPoint>
#include <QtDebug>

#include "../ifc/ifcdef.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/def.h"
#include "../vmisc/exception/vexception.h"
#include "../vmisc/fpm/fixed.hpp"
#include "../vmisc/fpm/math.hpp"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vmath.h"
#include "vabstractcurve.h"
#include "vellipticalarc_p.h"
#include "vsplinepath.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
constexpr qreal tolerance = accuracyPointOnLine / 8;

// Because of overflow we cannot generate arcs more than maxRadius
constexpr int maxRadius = 10000;

//---------------------------------------------------------------------------------------------------------------------
auto VLen(fpm::fixed_16_16 x, fpm::fixed_16_16 y) -> fpm::fixed_16_16
{
    x = fpm::abs(x);
    y = fpm::abs(y);
    if (x > y)
    {
        return x + qMax(y / 8, y / 2 - x / 8);
    }

    return y + qMax(x / 8, x / 2 - y / 8);
}

//---------------------------------------------------------------------------------------------------------------------
auto AuxRadius(fpm::fixed_16_16 xP, fpm::fixed_16_16 yP, fpm::fixed_16_16 xQ, fpm::fixed_16_16 yQ) -> fpm::fixed_16_16
{
    fpm::fixed_16_16 const dP = VLen(xP, yP);
    fpm::fixed_16_16 const dQ = VLen(xQ, yQ);
    fpm::fixed_16_16 const dJ = VLen(xP + xQ, yP + yQ);
    fpm::fixed_16_16 const dK = VLen(xP - xQ, yP - yQ);
    fpm::fixed_16_16 const r1 = qMax(dP, dQ);
    fpm::fixed_16_16 const r2 = qMax(dJ, dK);
    return qMax(r1 + r1 / 16, r2 - r2 / 4);
}

//---------------------------------------------------------------------------------------------------------------------
auto AngularInc(fpm::fixed_16_16 xP, fpm::fixed_16_16 yP, fpm::fixed_16_16 xQ, fpm::fixed_16_16 yQ,
                fpm::fixed_16_16 flatness) -> int
{

    fpm::fixed_16_16 const r = AuxRadius(xP, yP, xQ, yQ);
    fpm::fixed_16_16 err2{r >> 3};
    // 2nd-order term
    fpm::fixed_16_16 err4{r >> 7};
    // 4th-order term
    const int kmax = qRound(0.5 * std::log2(maxSceneSize / (8. * tolerance)));
    for (int k = 0; k < kmax; ++k)
    {
        if (flatness >= err2 + err4)
        {
            return k;
        }
        err2 >>= 2;
        err4 >>= 4;
    }
    return kmax;
}

//---------------------------------------------------------------------------------------------------------------------
inline void CircleGen(fpm::fixed_16_16 &u, fpm::fixed_16_16 &v, uint k)
{
    u -= v >> k;
    v += u >> k;
}

//---------------------------------------------------------------------------------------------------------------------
auto InitialValue(fpm::fixed_16_16 u0, fpm::fixed_16_16 v0, uint k) -> fpm::fixed_16_16
{
    uint const shift = 2 * k + 3;
    fpm::fixed_16_16 w{u0 >> shift};

    fpm::fixed_16_16 U0 = u0 - w + (v0 >> (k + 1));
    w >>= (shift + 1);
    U0 -= w;
    w >>= shift;
    U0 -= w;
    return U0;
}

//---------------------------------------------------------------------------------------------------------------------
auto EllipseCore(fpm::fixed_16_16 xC, fpm::fixed_16_16 yC, fpm::fixed_16_16 xP, fpm::fixed_16_16 yP,
                 fpm::fixed_16_16 xQ, fpm::fixed_16_16 yQ, fpm::fixed_16_16 sweep, fpm::fixed_16_16 flatness)
    -> QVector<QPointF>
{
    uint const k = qMin(static_cast<uint>(AngularInc(xP, yP, xQ, yQ, flatness)), 16U);
    const auto count = static_cast<std::uint32_t>(sweep.raw_value()) >> (16 - k);

    QVector<QPointF> arc;
    arc.reserve(static_cast<int>(count) + 1);

    // Arc start point
    arc.append({static_cast<qreal>(xP + xC), static_cast<qreal>(yP + yC)});

    xQ = InitialValue(xQ, xP, k);
    yQ = InitialValue(yQ, yP, k);

    for (uint i = 0; i < count; ++i)
    {
        CircleGen(xQ, xP, k);
        CircleGen(yQ, yP, k);
        arc.append({static_cast<qreal>(xP + xC), static_cast<qreal>(yP + yC)});
    }

    return arc;
}

//---------------------------------------------------------------------------------------------------------------------
auto EllipticArcPoints(QPointF c, qreal radius1, qreal radius2, qreal astart, qreal asweep, qreal approximationScale)
    -> QVector<QPointF>
{
    fpm::fixed_16_16 const xC{c.x()};
    fpm::fixed_16_16 const yC{c.y()};

    fpm::fixed_16_16 xP{c.x() + radius1};
    fpm::fixed_16_16 yP{c.y()};

    fpm::fixed_16_16 xQ{c.x()};
    fpm::fixed_16_16 yQ{c.y() - radius2};

    xP -= xC;
    yP -= yC;
    xQ -= xC;
    yQ -= yC;

    if (not qFuzzyIsNull(astart))
    {
        // Set new conjugate diameter end points P’ and Q’
        fpm::fixed_16_16 const cosa{cos(astart)};
        fpm::fixed_16_16 const sina{sin(astart)};
        fpm::fixed_16_16 const x{xP * cosa + xQ * sina};
        fpm::fixed_16_16 const y{yP * cosa + yQ * sina};

        xQ = xQ * cosa - xP * sina;
        yQ = yQ * cosa - yP * sina;
        xP = x;
        yP = y;
    }

    // If sweep angle is negative, switch direction
    if (asweep < 0)
    {
        xQ = -xQ;
        yQ = -yQ;
        asweep = -asweep;
    }

    if (approximationScale < minCurveApproximationScale || approximationScale > maxCurveApproximationScale)
    {
        approximationScale = VAbstractApplication::VApp()->GlobalCurveApproximationScale();
    }

    fpm::fixed_16_16 const flatness{maxCurveApproximationScale / approximationScale * tolerance};
    fpm::fixed_16_16 const swangle{asweep};
    QVector<QPointF> arc = EllipseCore(xC, yC, xP, yP, xQ, yQ, swangle, flatness);

    // Arc end point
    fpm::fixed_16_16 const cosb{qCos(asweep)};
    fpm::fixed_16_16 const sinb{qSin(asweep)};
    xP = xP * cosb + xQ * sinb;
    yP = yP * cosb + yQ * sinb;
    arc.append({static_cast<qreal>(xP + xC), static_cast<qreal>(yP + yC)});

    return arc;
}

//---------------------------------------------------------------------------------------------------------------------
auto JoinVectors(const QVector<QPointF> &v1, const QVector<QPointF> &v2) -> QVector<QPointF>
{
    QVector<QPointF> v;
    v.reserve(v1.size() + v2.size());

    v = v1;

    constexpr qreal accuracy = MmToPixel(0.0001);

    for (auto p : v2)
    {
        if (not VFuzzyComparePoints(v.constLast(), p, accuracy))
        {
            v.append(p);
        }
    }

    return v;
}

//---------------------------------------------------------------------------------------------------------------------
auto IsBoundedIntersection(QLineF::IntersectType type, QPointF p, const QLineF &segment1, const QLineF &segment2)
    -> bool
{
    return type == QLineF::BoundedIntersection
           || (type == QLineF::UnboundedIntersection && IsPointOnLineSegment(p, segment1.p1(), segment2.p1())
               && IsPointOnLineSegment(p, segment2.p1(), segment2.p2()));
}

//---------------------------------------------------------------------------------------------------------------------
auto FindBoundedIntersection(const QLineF &boundary, const QLineF &edge) -> std::optional<QPointF>
{
    QPointF p;
    // IsBoundedIntersection contains the logic for edge-case accuracy fixes
    if (QLineF::IntersectType const type = boundary.intersects(edge, &p); IsBoundedIntersection(type, p, edge, boundary))
    {
        return p;
    }
    return std::nullopt;
}

//---------------------------------------------------------------------------------------------------------------------
auto NormalizeArcPointOrder(QVector<QPointF> &points, const QLineF &start, const QLineF &end, bool &begin)
    -> std::optional<QVector<QPointF>>
{
    for (int i = 0; i < points.size() - 1; ++i)
    {
        if (QLineF const edge(points.at(i), points.at(i + 1)); auto p_opt = FindBoundedIntersection(start, edge))
        {
            QPointF const p = *p_opt;

            QVector<QPointF> const head = points.mid(0, i + 1);
            QVector<QPointF> tail = points.mid(i + 1, -1);

            tail = JoinVectors({p}, tail);
            points = JoinVectors(tail, head);
            points = JoinVectors(points, {p});

            if (VFuzzyComparePossibleNulls(start.angle(), end.angle()))
            {
                return points;
            }

            begin = false;
            break;
        }
    }

    return std::nullopt;
}

//---------------------------------------------------------------------------------------------------------------------
auto BuildArcSegment(const QVector<QPointF> &points, const QLineF &start, const QLineF &end, bool begin)
    -> QVector<QPointF>
{
    QVector<QPointF> arc;
    arc.reserve(points.size());

    for (int i = 0; i < points.size() - 1; ++i)
    {
        if (QLineF const edge(points.at(i), points.at(i + 1)); begin)
        {
            if (auto p_opt = FindBoundedIntersection(start, edge))
            {
                arc.append(*p_opt);
                begin = false;
            }
        }
        else
        {
            if (auto p_opt = FindBoundedIntersection(end, edge))
            {
                arc.append(points.at(i));
                arc.append(*p_opt);
                break;
            }

            arc.append(points.at(i));
        }
    }

    return arc;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VEllipticalArc default constructor.
 */
VEllipticalArc::VEllipticalArc()
  : VAbstractArc(GOType::EllipticalArc),
    d(new VEllipticalArcData)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VEllipticalArc constructor.
 * @param center center point.
 * @param radius1 arc major radius.
 * @param radius2 arc minor radius.
 * @param f1 start angle (degree).
 * @param f2 end angle (degree).
 */
VEllipticalArc::VEllipticalArc(const VPointF &center, qreal radius1, qreal radius2, const QString &formulaRadius1,
                               const QString &formulaRadius2, qreal f1, const QString &formulaF1, qreal f2,
                               const QString &formulaF2, qreal rotationAngle, const QString &formulaRotationAngle,
                               quint32 idObject, Draw mode)
  : VAbstractArc(GOType::EllipticalArc, center, f1, formulaF1, f2, formulaF2, idObject, mode),
    d(new VEllipticalArcData(radius1, radius2, formulaRadius1, formulaRadius2, rotationAngle, formulaRotationAngle))
{
    VEllipticalArc::CreateName();
    SetFlipped(radius1 < 0 || radius2 < 0);
}

//---------------------------------------------------------------------------------------------------------------------
VEllipticalArc::VEllipticalArc(const VPointF &center, qreal radius1, qreal radius2, qreal f1, qreal f2,
                               qreal rotationAngle)
  : VAbstractArc(GOType::EllipticalArc, center, f1, f2, NULL_ID, Draw::Calculation),
    d(new VEllipticalArcData(radius1, radius2, rotationAngle))
{
    VEllipticalArc::CreateName();
    SetFlipped(radius1 < 0 || radius2 < 0);
}

//---------------------------------------------------------------------------------------------------------------------
VEllipticalArc::VEllipticalArc(qreal length, const QString &formulaLength, const VPointF &center, qreal radius1,
                               qreal radius2, const QString &formulaRadius1, const QString &formulaRadius2, qreal f1,
                               const QString &formulaF1, qreal rotationAngle, const QString &formulaRotationAngle,
                               quint32 idObject, Draw mode)
  : VAbstractArc(GOType::EllipticalArc, formulaLength, center, f1, formulaF1, idObject, mode),
    d(new VEllipticalArcData(radius1, radius2, formulaRadius1, formulaRadius2, rotationAngle, formulaRotationAngle))
{
    VEllipticalArc::CreateName();
    FindF2(length);
}

//---------------------------------------------------------------------------------------------------------------------
VEllipticalArc::VEllipticalArc(qreal length, const VPointF &center, qreal radius1, qreal radius2, qreal f1,
                               qreal rotationAngle)
  : VAbstractArc(GOType::EllipticalArc, center, f1, NULL_ID, Draw::Calculation),
    d(new VEllipticalArcData(radius1, radius2, rotationAngle))
{
    VEllipticalArc::CreateName();
    FindF2(length);
    SetFormulaLength(QString::number(length));
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL_2(VEllipticalArc, VAbstractArc)

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief operator = assignment operator
 * @param arc arc
 * @return arc
 */
auto VEllipticalArc::operator=(const VEllipticalArc &arc) -> VEllipticalArc &
{
    if (&arc == this)
    {
        return *this;
    }
    VAbstractArc::operator=(arc);
    d = arc.d;
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VEllipticalArc::VEllipticalArc(VEllipticalArc &&arc) noexcept
  : VAbstractArc(std::move(arc)),
    d(std::move(arc.d)) // NOLINT(bugprone-use-after-move)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::operator=(VEllipticalArc &&arc) noexcept -> VEllipticalArc &
{
    VAbstractArc::operator=(arc);
    std::swap(d, arc.d);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::Rotate(QPointF originPoint, qreal degrees, const QString &name) const -> VEllipticalArc
{
    originPoint = d->m_transform.inverted().map(originPoint);

    QTransform t = d->m_transform;
    t.translate(originPoint.x(), originPoint.y());
    t.rotate(-degrees);
    t.translate(-originPoint.x(), -originPoint.y());

    VPointF center = VAbstractArc::GetCenter();
    center.setName("X"_L1);
    VEllipticalArc elArc(center,
                         d->radius1,
                         d->radius2,
                         VAbstractArc::GetStartAngle(),
                         VAbstractArc::GetEndAngle(),
                         d->rotationAngle);
    if (!name.isEmpty())
    {
        elArc.setName(elArc.GetTypeHead() + name);
    }
    elArc.SetColor(GetColor());
    elArc.SetPenStyle(GetPenStyle());
    elArc.SetFlipped(IsFlipped());
    elArc.SetTransform(t);
    elArc.SetApproximationScale(GetApproximationScale());
    elArc.SetDerivative(true);
    return elArc;
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::Flip(const QLineF &axis, const QString &name) const -> VEllipticalArc
{
    VPointF center = VAbstractArc::GetCenter();
    center.setName("X"_L1);
    VEllipticalArc elArc(center,
                         d->radius1,
                         d->radius2,
                         VAbstractArc::GetStartAngle(),
                         VAbstractArc::GetEndAngle(),
                         d->rotationAngle);
    if (!name.isEmpty())
    {
        elArc.setName(elArc.GetTypeHead() + name);
    }
    elArc.SetColor(GetColor());
    elArc.SetPenStyle(GetPenStyle());
    elArc.SetFlipped(not IsFlipped());
    elArc.SetTransform(d->m_transform * VGObject::FlippingMatrix(d->m_transform.inverted().map(axis)));
    elArc.SetApproximationScale(GetApproximationScale());
    elArc.SetDerivative(true);
    return elArc;
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::Move(qreal length, qreal angle, const QString &name) const -> VEllipticalArc
{
    const VPointF oldCenter = VAbstractArc::GetCenter();
    const VPointF center = oldCenter.Move(length, angle);

    const QPointF position =
        d->m_transform.inverted().map(center.toQPointF()) - d->m_transform.inverted().map(oldCenter.toQPointF());

    QTransform t = d->m_transform;
    t.translate(position.x(), position.y());

    VEllipticalArc elArc(oldCenter, d->radius1, d->radius2, VAbstractArc::GetStartAngle(), VAbstractArc::GetEndAngle(),
                         d->rotationAngle);
    if (!name.isEmpty())
    {
        elArc.setName(elArc.GetTypeHead() + name);
    }
    elArc.SetColor(GetColor());
    elArc.SetPenStyle(GetPenStyle());
    elArc.SetFlipped(IsFlipped());
    elArc.SetTransform(t);
    elArc.SetApproximationScale(GetApproximationScale());
    elArc.SetDerivative(true);
    return elArc;
}

//---------------------------------------------------------------------------------------------------------------------
VEllipticalArc::~VEllipticalArc() = default;

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetLength return arc length.
 * @return length.
 */
auto VEllipticalArc::GetLength() const -> qreal
{
    qreal length = PathLength(GetPoints());

    if (IsFlipped())
    {
        length *= -1;
    }

    return length;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetP1 return point associated with start angle.
 * @return point.
 */
auto VEllipticalArc::GetP1() const -> QPointF
{
    return GetTransform().map(GetP(VAbstractArc::GetStartAngle()));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetP2 return point associated with end angle.
 * @return point.
 */
auto VEllipticalArc::GetP2() const -> QPointF
{
    return GetTransform().map(GetP(VAbstractArc::GetEndAngle()));
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::GetTransform() const -> QTransform
{
    return d->m_transform;
}

//---------------------------------------------------------------------------------------------------------------------
void VEllipticalArc::SetTransform(const QTransform &matrix, bool combine)
{
    d->m_transform = combine ? d->m_transform * matrix : matrix;
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::GetCenter() const -> VPointF
{
    VPointF center = VAbstractArc::GetCenter();
    const QPointF p = d->m_transform.map(center.toQPointF());
    center.setX(p.x());
    center.setY(p.y());
    return center;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetPoints return list of points needed for drawing arc.
 * @return list of points
 */
auto VEllipticalArc::GetPoints() const -> QVector<QPointF>
{
    const QPointF center = VAbstractArc::GetCenter().toQPointF();

    if (qFuzzyIsNull(d->radius1) && qFuzzyIsNull(d->radius2))
    {
        return {center};
    }

    // Don't work with 0 radius. Always make it bigger than 0.
    Q_RELAXED_CONSTEXPR qreal threshold = ToPixel(0.001, Unit::Mm);
    qreal radius1 = qMax(qAbs(d->radius1), threshold);
    qreal radius2 = qMax(qAbs(d->radius2), threshold);
    qreal const max = qMax(qAbs(d->radius1), qAbs(d->radius2));
    qreal scale = 1;

    if (max > maxRadius)
    {
        scale = max / maxRadius;
        radius1 /= scale;
        radius2 /= scale;
    }

    // Generate complete ellipse because angles are not correct and have to be fixed manually
    QVector<QPointF> points = EllipticArcPoints(center, radius1, radius2, 0.0, M_2PI, GetApproximationScale());
    points = ArcPoints(points);

    QTransform t = d->m_transform;
    t.translate(center.x(), center.y());
    if (not VFuzzyComparePossibleNulls(scale, 1))
    {
        // Because fixed 16.16 type has limitations it is very easy to get overflow error.
        // To avoid this we calculate an arc for scaled radiuses and then scale up to original size.
        t.scale(scale, scale);
    }
    t.rotate(-d->rotationAngle);
    t.translate(-center.x(), -center.y());

    std::transform(points.begin(), points.end(), points.begin(), [&t](const QPointF &point) { return t.map(point); });

    return IsFlipped() ? Reverse(points) : points;
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::GetStartAngle() const -> qreal
{
    return QLineF(GetCenter().toQPointF(), GetP1()).angle() - d->rotationAngle;
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::GetEndAngle() const -> qreal
{
    return QLineF(GetCenter().toQPointF(), GetP2()).angle() - d->rotationAngle;
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::ToSplinePath() const -> VSplinePath
{
    // 1. Radii check
    if (qFuzzyIsNull(d->radius1) || qFuzzyIsNull(d->radius2))
    {
        return VSplinePath();
    }

    const QPointF center = static_cast<QPointF>(GetCenter());

    // 2. Rotation of ellipse (Qt uses degrees CCW)
    QTransform ellipseTf;
    ellipseTf.translate(center.x(), center.y());
    ellipseTf.rotate(-d->rotationAngle);     // <-- Qt CCW rotation
    ellipseTf.scale(d->radius1, d->radius2); // <-- Scale unit circle to ellipse

    // 3. Angle logic
    // Helper lambda to convert Visual Angle -> Parametric Angle
    auto ToParametric = [&](qreal visualDegrees) -> qreal
    {
        // 1. Convert to radians
        qreal const localRad = qDegreesToRadians(visualDegrees);

        // 2. Apply the scaling factor to the tangent
        //    Math: tan(t) = (r1 / r2) * tan(theta)
        //    We use atan2 for correct quadrant handling
        qreal const y = d->radius1 * qSin(localRad);
        qreal const x = d->radius2 * qCos(localRad);

        qreal const paramRad = qAtan2(y, x);

        return qRadiansToDegrees(paramRad);
    };

    qreal startDeg = ToParametric(GetStartAngle());
    qreal const endDeg = ToParametric(GetEndAngle());
    qreal const sweepDeg = AngleArc(startDeg, endDeg); // always signed
    qreal direction = 1.0;

    // If flipped, reverse parametrization
    if (IsFlipped())
    {
        startDeg = endDeg;
        direction = -1.0;
    }

    // 4. Split arc into chunks
    QVector<qreal> segments;
    qreal remaining = qAbs(sweepDeg);
    const qreal angleInterpolation = GetApproximationScale() < 5.0 ? 90. : 45.; // degree

    while (remaining > angleInterpolation)
    {
        segments.append(angleInterpolation);
        remaining -= angleInterpolation;
    }

    if (remaining > 0.01)
    {
        segments.append(remaining);
    }

    // 5. Build curves
    QVector<VSpline> allSegments;
    allSegments.reserve(segments.size());

    qreal currentDeg = startDeg;

    // 4. Loop through segments
    for (int i = 0; i < segments.size(); ++i)
    {
        const qreal segDeg = segments.at(i) * direction;

        // --- Points on unit circle (parametric angle, degrees) ---
        const qreal a1 = currentDeg;
        const qreal a2 = currentDeg + segDeg;

        // Start/end unit circle points via QLineF
        const QPointF u1 = QLineF::fromPolar(1.0, a1).p2();
        const QPointF u2 = QLineF::fromPolar(1.0, a2).p2();

        // kappa factor
        const qreal k = (4.0 / 3.0) * qTan(qDegreesToRadians(segDeg) / 4.0);

        // Control Points (Unit)
        QPointF const cp1_unit(u1.x() + k * u1.y(), u1.y() - k * u1.x());
        QPointF const cp2_unit(u2.x() - k * u2.y(), u2.y() + k * u2.x());

        // Transform unit → ellipse world coords
        const QPointF p1 = ellipseTf.map(u1);
        const QPointF p4 = ellipseTf.map(u2);
        const QPointF c1 = ellipseTf.map(cp1_unit);
        const QPointF c2 = ellipseTf.map(cp2_unit);

        const QString p1Name = QStringLiteral("%1_seg%2_p1").arg(name()).arg(i);
        const QString p4Name = QStringLiteral("%1_seg%2_p4").arg(name()).arg(i);

        allSegments.append(VSpline(VPointF(p1, p1Name), c1, c2, VPointF(p4, p4Name)));

        currentDeg += segDeg;
    }

    if (allSegments.size() == 1)
    {
        QString const midPointName = QStringLiteral("%1_seg1_mid").arg(name());
        VSpline const only = allSegments.constFirst();
        VSpline left;
        VSpline right;
        only.CutSplineAtParam(0.5, left, right, midPointName);
        allSegments = {left, right};
    }

    VSplinePath path(allSegments);
    path.SetStrict(true);
    path.SetApproximationScale(GetApproximationScale());

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
void VEllipticalArc::CreateName()
{
    if (!IsDerivative())
    {
        setName(GetTypeHead() + HeadlessName());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VEllipticalArc::CreateAlias()
{
    const QString aliasSuffix = GetAliasSuffix();
    if (aliasSuffix.isEmpty())
    {
        SetAlias(QString());
        return;
    }

    SetAlias(GetTypeHead() + aliasSuffix);
}

//---------------------------------------------------------------------------------------------------------------------
void VEllipticalArc::SetNameSuffix(const QString &suffix)
{
    setName(GetTypeHead() + suffix);
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::HeadlessName() const -> QString
{
    if (IsDerivative())
    {
        const QString fullName = name();
        const QString prefix = GetTypeHead();
        if (const QString headless = fullName.startsWith(prefix) ? fullName.sliced(prefix.length()) : fullName;
            !headless.isEmpty())
        {
            return headless;
        }
    }

    QString name = GetCenter().name();
    if (getMode() == Draw::Modeling && getIdObject() != NULL_ID)
    {
        name += u"_%1"_s.arg(getIdObject());
    }
    else if (VAbstractCurve::id() != NULL_ID)
    {
        name += u"_%1"_s.arg(VAbstractCurve::id());
    }

    if (GetDuplicate() > 0)
    {
        name += u"_%1"_s.arg(GetDuplicate());
    }

    return name;
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::GetTypeHead() const -> QString
{
    return ELARC_;
}

//---------------------------------------------------------------------------------------------------------------------
void VEllipticalArc::FindF2(qreal length)
{
    qreal gap = 180;
    if (length < 0 || d->radius1 < 0 || d->radius2 < 0)
    {
        SetFlipped(true);
        gap = -gap;
    }
    else if (qAbs(length) > qAbs(MaxLength()))
    {
        length = MaxLength();
    }

    // We need to calculate the second angle
    // first approximation of angle between start and end angles

    VPointF const center = GetCenter();
    QLineF radius1(center.x(), center.y(), center.x() + qAbs(d->radius1), center.y());
    radius1.setAngle(GetStartAngle());
    radius1.setAngle(radius1.angle() + gap);
    qreal endAngle = radius1.angle();

    // we need to set the end angle, because we want to use GetLength()
    SetFormulaF2(QString::number(endAngle), endAngle);

    qreal lenBez = GetLength(); // first approximation of length

    Q_RELAXED_CONSTEXPR qreal eps = ToPixel(0.001, Unit::Mm);

    while (qAbs(lenBez - length) > eps)
    {
        gap = gap / 2;
        if (qAbs(gap) < 0.0001)
        {
            break;
        }

        if (qAbs(lenBez) > qAbs(length))
        { // we selected too big end angle
            radius1.setAngle(endAngle - qAbs(gap));
        }
        else
        { // we selected too little end angle
            radius1.setAngle(endAngle + qAbs(gap));
        }
        endAngle = radius1.angle();
        // we need to set d->f2, because we use it when we calculate GetLength
        SetFormulaF2(QString::number(endAngle), endAngle);
        lenBez = GetLength();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::DoCutArc(qreal length, VAbstractArc *arc1, VAbstractArc *arc2, const QString &pointName) const
    -> QPointF
{
    auto *vArc1Ptr = dynamic_cast<VEllipticalArc *>(arc1);
    auto *vArc2Ptr = dynamic_cast<VEllipticalArc *>(arc2);

    // Check if the cast succeeded for both.
    if ((vArc1Ptr == nullptr) || (vArc2Ptr == nullptr))
    {
        // Handle error case (e.g., trying to cut a VArc into VEllipticalArc objects)
        qWarning() << "VEllipticalArc::DoCutArc received VAbstractArc references that are not VEllipticalArc objects.";
        return {};
    }

    const qreal fullLength = GetLength();

    if (qFuzzyIsNull(fullLength) || (qFuzzyIsNull(d->radius1) && qFuzzyIsNull(d->radius2)))
    {
        *vArc1Ptr = *this;
        *vArc2Ptr = *this;
        return GetCenter().toQPointF();
    }

    if (qFuzzyIsNull(length) || qFuzzyIsNull(length + fullLength))
    {
        VEllipticalArc newArc1(GetCenter(),
                               d->radius1,
                               d->radius2,
                               d->formulaRadius1,
                               d->formulaRadius2,
                               GetStartAngle(),
                               GetFormulaF1(),
                               GetStartAngle(),
                               GetFormulaF1(),
                               d->rotationAngle,
                               GetFormulaRotationAngle(),
                               getIdObject(),
                               getMode());
        newArc1.SetApproximationScale(GetApproximationScale());
        newArc1.SetFlipped(IsFlipped());
        newArc1.SetAllowEmpty(true);

        *vArc1Ptr = newArc1;
        *vArc2Ptr = *this;

        return GetP1();
    }

    if (VFuzzyComparePossibleNulls(length, fullLength))
    {
        *vArc1Ptr = *this;

        VEllipticalArc newArc2(GetCenter(),
                               d->radius1,
                               d->radius2,
                               d->formulaRadius1,
                               d->formulaRadius2,
                               GetEndAngle(),
                               GetFormulaF2(),
                               GetEndAngle(),
                               GetFormulaF2(),
                               d->rotationAngle,
                               GetFormulaRotationAngle(),
                               getIdObject(),
                               getMode());
        newArc2.SetApproximationScale(GetApproximationScale());
        newArc2.SetFlipped(IsFlipped());
        newArc2.SetAllowEmpty(true);

        *vArc2Ptr = newArc2;

        return GetP2();
    }

    qreal const len = CorrectCutLength(length, fullLength, pointName);

    // the first arc has given length and startAngle just like in the origin arc
    VEllipticalArc newArc1(len,
                           QString().setNum(length),
                           GetCenter(),
                           d->radius1,
                           d->radius2,
                           d->formulaRadius1,
                           d->formulaRadius2,
                           GetStartAngle(),
                           GetFormulaF1(),
                           d->rotationAngle,
                           GetFormulaRotationAngle(),
                           getIdObject(),
                           getMode());
    newArc1.SetApproximationScale(GetApproximationScale());
    newArc1.SetFlipped(IsFlipped());

    *vArc1Ptr = newArc1;

    // the second arc has startAngle just like endAngle of the first arc
    // and it has endAngle just like endAngle of the origin arc
    VEllipticalArc newArc2(GetCenter(),
                           d->radius1,
                           d->radius2,
                           d->formulaRadius1,
                           d->formulaRadius2,
                           vArc1Ptr->GetEndAngle(),
                           vArc1Ptr->GetFormulaF2(),
                           GetEndAngle(),
                           GetFormulaF2(),
                           d->rotationAngle,
                           GetFormulaRotationAngle(),
                           getIdObject(),
                           getMode());
    newArc2.SetApproximationScale(GetApproximationScale());
    newArc2.SetFlipped(IsFlipped());

    *vArc2Ptr = newArc2;

    return vArc1Ptr->GetP2();
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::DoCutArcByLength(qreal length, const QString &pointName) const -> QPointF
{
    VEllipticalArc arc1;
    VEllipticalArc arc2;
    return VAbstractArc::CutArc(length, &arc1, &arc2, pointName);
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::MaxLength() const -> qreal
{
    const qreal h = qPow(qAbs(d->radius1) - qAbs(d->radius2), 2) / qPow(qAbs(d->radius1) + qAbs(d->radius2), 2);
    qreal ellipseLength = M_PI * (qAbs(d->radius1) + qAbs(d->radius2)) * (1 + 3 * h / (10 + qSqrt(4 - 3 * h)));

    if (d->radius1 < 0 || d->radius2 < 0)
    {
        ellipseLength *= -1;
    }

    return ellipseLength;
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::GetP(qreal angle) const -> QPointF
{
    if (qFuzzyIsNull(d->radius1) && qFuzzyIsNull(d->radius2))
    {
        return GetCenter().toQPointF();
    }

    QLineF line(0, 0, 100, 0);
    line.setAngle(angle);

    const qreal a = not qFuzzyIsNull(d->radius1) ? line.p2().x() / qAbs(d->radius1) : 0;
    const qreal b = not qFuzzyIsNull(d->radius2) ? line.p2().y() / qAbs(d->radius2) : 0;
    const qreal k = qSqrt(a * a + b * b);

    if (qFuzzyIsNull(k))
    {
        return GetCenter().toQPointF();
    }

    QPointF const p(line.p2().x() / k, line.p2().y() / k);

    QLineF line2(QPointF(), p);
    SCASSERT(VFuzzyComparePossibleNulls(line2.angle(), line.angle()))

    line2.setAngle(line2.angle() + d->rotationAngle);
    return line2.p2() + VAbstractArc::GetCenter().toQPointF();
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::ArcPoints(QVector<QPointF> points) const -> QVector<QPointF>
{
    if (points.size() < 2 || (qFuzzyIsNull(d->radius1) && qFuzzyIsNull(d->radius2)))
    {
        return points;
    }

    if (const qreal angleDiff = qAbs(GetStartAngle() - GetEndAngle());
        (VFuzzyComparePossibleNulls(angleDiff, 0) || VFuzzyComparePossibleNulls(angleDiff, 360)) && IsAllowEmpty())
    {
        return {GetP1()};
    }

    QPointF const center = VAbstractArc::GetCenter().toQPointF();
    qreal const radius = qMax(qAbs(d->radius1), qAbs(d->radius2)) * 2;

    QLineF start(center.x(), center.y(), center.x() + radius, center.y());
    start.setAngle(VAbstractArc::GetStartAngle());

    QLineF end(center.x(), center.y(), center.x() + radius, center.y());
    end.setAngle(VAbstractArc::GetEndAngle());

    bool begin = true;

    if (start.angle() >= end.angle())
    {
        if (auto reordered = NormalizeArcPointOrder(points, start, end, begin))
        {
            return *reordered;
        }
    }

    QVector<QPointF> const arc = BuildArcSegment(points, start, end, begin);
    return arc.isEmpty() ? points : arc;
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::CorrectCutLength(qreal length, qreal fullLength, const QString &pointName) const -> qreal
{
    qreal len = length;

    auto TooSmallWarning = [this, pointName]()
    {
        QString errorMsg;
        if (not pointName.isEmpty())
        {
            errorMsg = QObject::tr("Curve '%1'. Length of a cut segment (%2) is too small. Optimize it to minimal "
                                   "value.")
                           .arg(name(), pointName);
        }
        else
        {
            errorMsg = QObject::tr("Curve '%1'. Length of a cut segment is too small. Optimize it to minimal value.")
                           .arg(name());
        }
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractApplication::warningMessageSignature + errorMsg;
    };

    auto TooBigWarning = [this, pointName]()
    {
        QString errorMsg;
        if (not pointName.isEmpty())
        {
            errorMsg = QObject::tr("Curve '%1'. Length of a cut segment (%2) is too big. Optimize it to maximal value.")
                           .arg(name(), pointName);
        }
        else
        {
            errorMsg = QObject::tr("Curve '%1'. Length of a cut segment is too big. Optimize it to maximal value.")
                           .arg(name());
        }
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractApplication::warningMessageSignature + errorMsg;
    };

    if (!IsFlipped())
    {
        if (length < 0)
        {
            len = 0;
            TooSmallWarning();
        }
        else if (length > fullLength)
        {
            len = fullLength;
            TooBigWarning();
        }
    }
    else
    {
        if (length < fullLength)
        {
            len = fullLength;
            TooSmallWarning();
        }
        else if (length > 0)
        {
            len = 0;
            TooBigWarning();
        }
    }

    return len;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetFormulaRadius1 return formula for major radius.
 * @return radius.
 */
auto VEllipticalArc::GetFormulaRadius1() const -> QString
{
    return d->formulaRadius1;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetFormulaRadius2 return formula for minor radius.
 * @return radius.
 */
auto VEllipticalArc::GetFormulaRadius2() const -> QString
{
    return d->formulaRadius2;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetFormulaRotationAngle return formula for rotation angle.
 * @return rotationAngle.
 */
auto VEllipticalArc::GetFormulaRotationAngle() const -> QString
{
    return d->formulaRotationAngle;
}

//---------------------------------------------------------------------------------------------------------------------
void VEllipticalArc::SetFormulaRadius1(const QString &formula, qreal value)
{
    d->formulaRadius1 = formula;
    d->radius1 = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VEllipticalArc::SetRadius1(qreal value)
{
    d->formulaRadius1 = QString::number(value);
    d->radius1 = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VEllipticalArc::SetFormulaRadius2(const QString &formula, qreal value)
{
    d->formulaRadius2 = formula;
    d->radius2 = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VEllipticalArc::SetRadius2(qreal value)
{
    d->formulaRadius2 = QString::number(value);
    d->radius2 = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VEllipticalArc::SetFormulaRotationAngle(const QString &formula, qreal value)
{
    d->formulaRotationAngle = formula;
    d->rotationAngle = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VEllipticalArc::SetRotationAngle(qreal value)
{
    d->formulaRotationAngle = QString::number(value);
    d->rotationAngle = value;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetRadius1 return elliptical arc major radius.
 * @return string with formula.
 */
auto VEllipticalArc::GetRadius1() const -> qreal
{
    return d->radius1;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetRadius2 return elliptical arc minor radius.
 * @return string with formula.
 */
auto VEllipticalArc::GetRadius2() const -> qreal
{
    return d->radius2;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetRotationAngle return rotation angle.
 * @return rotationAngle.
 */
auto VEllipticalArc::GetRotationAngle() const -> qreal
{
    return d->rotationAngle;
}
