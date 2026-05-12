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
#include "../vmisc/vabstractapplication.h"
#include "vabstractcurve.h"
#include "vellipticalarc_p.h"
#include "vsplinepath.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

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
    SetReversed(radius1 < 0 || radius2 < 0);
}

//---------------------------------------------------------------------------------------------------------------------
VEllipticalArc::VEllipticalArc(const VPointF &center, qreal radius1, qreal radius2, qreal f1, qreal f2,
                               qreal rotationAngle)
  : VAbstractArc(GOType::EllipticalArc, center, f1, f2, NULL_ID, Draw::Calculation),
    d(new VEllipticalArcData(radius1, radius2, rotationAngle))
{
    VEllipticalArc::CreateName();
    SetReversed(radius1 < 0 || radius2 < 0);
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
    VPointF center = GetCenter().Rotate(originPoint, degrees);
    center.setName("X"_L1);

    VEllipticalArc elArc(center, d->radius1, d->radius2, GetStartAngle(), GetEndAngle(), d->rotationAngle + degrees);
    if (!name.isEmpty())
    {
        elArc.setName(elArc.GetTypeHead() + name);
    }
    elArc.SetColor(GetColor());
    elArc.SetPenStyle(GetPenStyle());
    elArc.SetFlipped(IsFlipped());
    elArc.SetApproximationScale(GetApproximationScale());
    elArc.SetDerivative(true);
    return elArc;
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::Flip(const QLineF &axis, const QString &name) const -> VEllipticalArc
{
    // Calculate rotation correction between vertical and original axis
    const qreal axisAngle = axis.angle();
    const qreal rotationCorrection = 90.0 - axisAngle;

    // Create vertical axis through the same point as original axis
    QLineF const verticalAxis(axis.p1(), axis.p1() + QPointF(0, -100));

    // Flip p1 and p2 using the VERTICAL axis
    const QPointF p1 = VPointF::FlipPF(verticalAxis, GetP(GetStartAngle(), false));
    const QPointF p2 = VPointF::FlipPF(verticalAxis, GetP(GetEndAngle(), false));

    // Flip center using the VERTICAL axis
    VPointF flippedCenter = GetCenter().Flip(verticalAxis);

    // Calculate angles from flipped center to flipped points
    const qreal f1 = QLineF(static_cast<QPointF>(flippedCenter), p1).angle();
    const qreal f2 = QLineF(static_cast<QPointF>(flippedCenter), p2).angle();

    // Apply rotation correction
    const qreal newRotationAngle = -(d->rotationAngle + 2.0 * rotationCorrection);

    // Calculate correct center for original axis
    flippedCenter = GetCenter().Flip(axis);
    flippedCenter.setName("X"_L1);

    VEllipticalArc elArc(flippedCenter, d->radius1, d->radius2, f1, f2, newRotationAngle);
    if (!name.isEmpty())
    {
        elArc.setName(elArc.GetTypeHead() + name);
    }
    elArc.SetColor(GetColor());
    elArc.SetPenStyle(GetPenStyle());
    elArc.SetFlipped(not IsFlipped());
    elArc.SetApproximationScale(GetApproximationScale());
    elArc.SetDerivative(true);
    return elArc;
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::Move(qreal length, qreal angle, const QString &name) const -> VEllipticalArc
{
    const VPointF center = GetCenter().Move(length, angle);

    VEllipticalArc elArc(center, d->radius1, d->radius2, GetStartAngle(), GetEndAngle(), d->rotationAngle);
    if (!name.isEmpty())
    {
        elArc.setName(elArc.GetTypeHead() + name);
    }
    elArc.SetColor(GetColor());
    elArc.SetPenStyle(GetPenStyle());
    elArc.SetFlipped(IsFlipped());
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

    if (IsNegative())
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
    return GetP(GetStartAngle());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetP2 return point associated with end angle.
 * @return point.
 */
auto VEllipticalArc::GetP2() const -> QPointF
{
    return GetP(GetEndAngle());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetPoints return list of points needed for drawing arc.
 * @return list of points
 */
auto VEllipticalArc::GetPoints() const -> QVector<QPointF>
{
    const QPointF center = GetCenter().toQPointF();

    if (qFuzzyIsNull(d->radius1) && qFuzzyIsNull(d->radius2))
    {
        return {center};
    }

    return ToSplinePath().GetPoints();
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::ToSplinePath() const -> VSplinePath
{
    // 1. Radii check
    if (qFuzzyIsNull(d->radius1) && qFuzzyIsNull(d->radius2))
    {
        return VSplinePath();
    }

    const auto rawCenter = static_cast<QPointF>(GetCenter());

    // 2. Rotation of ellipse (Qt uses degrees CCW)
    QTransform ellipseTf;
    ellipseTf.translate(rawCenter.x(), rawCenter.y());
    ellipseTf.rotate(-d->rotationAngle);                 // <-- Qt CCW rotation
    ellipseTf.scale(qAbs(d->radius1), qAbs(d->radius2)); // <-- Scale unit circle to ellipse

    // 3. Angle logic
    // Helper lambda to convert Visual Angle -> Parametric Angle
    auto ToParametric = [&](qreal visualDegrees) -> qreal
    {
        // 1. Convert to radians
        qreal const localRad = qDegreesToRadians(visualDegrees);

        // 2. Apply the scaling factor to the tangent
        //    Math: tan(t) = (r1 / r2) * tan(theta)
        //    We use atan2 for correct quadrant handling
        qreal const y = qAbs(d->radius1) * qSin(localRad);
        qreal const x = qAbs(d->radius2) * qCos(localRad);

        qreal const paramRad = qAtan2(y, x);
        qreal paramDeg = qRadiansToDegrees(paramRad);

        // 3. Normalize to [0, 360)
        if (paramDeg < 0.0)
        {
            paramDeg += 360.0;
        }

        return paramDeg;
    };

    qreal const startDeg = ToParametric(GetStartAngle());
    qreal const endDeg = ToParametric(GetEndAngle());
    qreal const sweepDeg = AngleArc(startDeg, endDeg); // always signed
    qreal const direction = IsNegative() ? -1.0 : 1.0;

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
auto VEllipticalArc::GetMidpoint() const -> VPointF
{
    const double midAngleRad = qDegreesToRadians(GetStartAngle() + AngleArc() / 2.0);

    // Qt's Y-axis is inverted compared to standard math
    const VPointF center = GetCenter();
    const double x = center.x() + d->radius1 * qCos(midAngleRad);
    const double y = center.y() - d->radius2 * qSin(midAngleRad); // note the minus

    VPointF pos(QPointF(x, y));
    pos.setId(id());
    pos.setName(ObjectName());
    return pos;
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
        if (const QString headless = fullName.startsWith(prefix) ? Sliced(fullName, prefix.length()) : fullName;
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
    if (length < 0 || d->radius1 < 0 || d->radius2 < 0)
    {
        SetReversed(true);
    }

    if (qAbs(length) > qAbs(MaxLength()))
    {
        length = MaxLength();
    }

    Q_RELAXED_CONSTEXPR qreal eps = ToPixel(0.001, Unit::Mm);
    VPointF const center = GetCenter();
    QLineF radius1(center.x(), center.y(), center.x() + qAbs(d->radius1), center.y());

    qreal const sign = IsReversed() ? -1.0 : 1.0;
    qreal step = 180.0;
    radius1.setAngle(GetStartAngle() + sign * step);
    qreal endAngle = radius1.angle();

    SetFormulaF2(QString::number(endAngle), endAngle);

    while (step > 0.0001)
    {
        qreal const lenBez = qAbs(GetLength());

        if (qAbs(lenBez - qAbs(length)) <= eps)
        {
            break;
        }

        if (lenBez > qAbs(length))
        {
            step /= 2.0;
            radius1.setAngle(endAngle - sign * step);
        }
        else
        {
            step /= 2.0;
            radius1.setAngle(endAngle + sign * step);
        }

        endAngle = radius1.angle();
        SetFormulaF2(QString::number(endAngle), endAngle);
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
        newArc1.SetReversed(IsReversed());
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
        newArc2.SetReversed(IsReversed());
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
    newArc1.SetReversed(IsReversed());
    newArc1.SetAllowEmpty(true);

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
    newArc2.SetReversed(IsReversed());
    newArc2.SetAllowEmpty(true);

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

    if (IsReversed())
    {
        ellipseLength *= -1;
    }

    return ellipseLength;
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::GetP(qreal angle, bool addRotation) const -> QPointF
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

    if (addRotation)
    {
        line2.setAngle(line2.angle() + d->rotationAngle);
    }
    return line2.p2() + GetCenter().toQPointF();
}

//---------------------------------------------------------------------------------------------------------------------
auto VEllipticalArc::CorrectCutLength(qreal length, qreal fullLength, const QString &pointName) const -> qreal
{
    qreal len = length;

    auto TooSmallWarning = [this, pointName]() -> void
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

    auto TooBigWarning = [this, pointName]() -> void
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

    if (!IsNegative())
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
