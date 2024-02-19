/************************************************************************
 **
 **  @file   varc.cpp
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

#include "varc.h"

#include <QLineF>
#include <QPointF>
#include <QtDebug>

#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/def.h"
#include "../vmisc/defglobal.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vmath.h"
#include "vabstractcurve.h"
#include "varc_p.h"
#include "vspline.h"

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VArc default constructor.
 */
VArc::VArc()
  : VAbstractArc(GOType::Arc),
    d(new VArcData)
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VArc constructor.
 * @param center center point.
 * @param radius arc radius.
 * @param f1 start angle (degree).
 * @param f2 end angle (degree).
 */
VArc::VArc(const VPointF &center, qreal radius, const QString &formulaRadius, qreal f1, const QString &formulaF1,
           qreal f2, const QString &formulaF2, quint32 idObject, Draw mode)
  : VAbstractArc(GOType::Arc, center, f1, formulaF1, f2, formulaF2, idObject, mode),
    d(new VArcData(radius, formulaRadius))
{
    CreateName();
    SetFlipped(radius < 0);
}

//---------------------------------------------------------------------------------------------------------------------
VArc::VArc(const VPointF &center, qreal radius, qreal f1, qreal f2)
  : VAbstractArc(GOType::Arc, center, f1, f2, NULL_ID, Draw::Calculation),
    d(new VArcData(radius))
{
    CreateName();
    SetFlipped(radius < 0);
}

//---------------------------------------------------------------------------------------------------------------------
VArc::VArc(qreal length, const QString &formulaLength, const VPointF &center, qreal radius,
           const QString &formulaRadius, qreal f1, const QString &formulaF1, quint32 idObject, Draw mode)
  : VAbstractArc(GOType::Arc, formulaLength, center, f1, formulaF1, idObject, mode),
    d(new VArcData(radius, formulaRadius))
{
    CreateName();
    FindF2(length);
}

//---------------------------------------------------------------------------------------------------------------------
VArc::VArc(qreal length, const VPointF &center, qreal radius, qreal f1)
  : VAbstractArc(GOType::Arc, center, f1, NULL_ID, Draw::Calculation),
    d(new VArcData(radius))
{
    CreateName();
    FindF2(length);
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL_2(VArc, VAbstractArc)

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief operator = assignment operator
 * @param arc arc
 * @return arc
 */
auto VArc::operator=(const VArc &arc) -> VArc &
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
VArc::VArc(VArc &&arc) noexcept
  : VAbstractArc(std::move(arc)),
    d(std::move(arc.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VArc::operator=(VArc &&arc) noexcept -> VArc &
{
    VAbstractArc::operator=(arc);
    std::swap(d, arc.d);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
auto VArc::Rotate(const QPointF &originPoint, qreal degrees, const QString &prefix) const -> VArc
{
    const VPointF center = GetCenter().Rotate(originPoint, degrees);

    const QPointF p1 = VPointF::RotatePF(originPoint, GetP1(), degrees);
    const QPointF p2 = VPointF::RotatePF(originPoint, GetP2(), degrees);

    const qreal f1 = QLineF(static_cast<QPointF>(center), p1).angle();
    const qreal f2 = QLineF(static_cast<QPointF>(center), p2).angle();

    VArc arc(center, d->radius, f1, f2);
    arc.setName(name() + prefix);

    if (not GetAliasSuffix().isEmpty())
    {
        arc.SetAliasSuffix(GetAliasSuffix() + prefix);
    }

    arc.SetColor(GetColor());
    arc.SetPenStyle(GetPenStyle());
    arc.SetFlipped(IsFlipped());
    arc.SetApproximationScale(GetApproximationScale());
    return arc;
}

//---------------------------------------------------------------------------------------------------------------------
auto VArc::Flip(const QLineF &axis, const QString &prefix) const -> VArc
{
    const VPointF center = GetCenter().Flip(axis);

    const QPointF p1 = VPointF::FlipPF(axis, GetP1());
    const QPointF p2 = VPointF::FlipPF(axis, GetP2());

    const qreal f1 = QLineF(static_cast<QPointF>(center), p1).angle();
    const qreal f2 = QLineF(static_cast<QPointF>(center), p2).angle();

    VArc arc(center, d->radius, f1, f2);
    arc.setName(name() + prefix);

    if (not GetAliasSuffix().isEmpty())
    {
        arc.SetAliasSuffix(GetAliasSuffix() + prefix);
    }

    arc.SetColor(GetColor());
    arc.SetPenStyle(GetPenStyle());
    arc.SetFlipped(not IsFlipped());
    arc.SetApproximationScale(GetApproximationScale());
    return arc;
}

//---------------------------------------------------------------------------------------------------------------------
auto VArc::Move(qreal length, qreal angle, const QString &prefix) const -> VArc
{
    const VPointF center = GetCenter().Move(length, angle);

    const QPointF p1 = VPointF::MovePF(GetP1(), length, angle);
    const QPointF p2 = VPointF::MovePF(GetP2(), length, angle);

    const qreal f1 = QLineF(static_cast<QPointF>(center), p1).angle();
    const qreal f2 = QLineF(static_cast<QPointF>(center), p2).angle();

    VArc arc(center, d->radius, f1, f2);
    arc.setName(name() + prefix);

    if (not GetAliasSuffix().isEmpty())
    {
        arc.SetAliasSuffix(GetAliasSuffix() + prefix);
    }

    arc.SetColor(GetColor());
    arc.SetPenStyle(GetPenStyle());
    arc.SetFlipped(IsFlipped());
    arc.SetApproximationScale(GetApproximationScale());
    return arc;
}

//---------------------------------------------------------------------------------------------------------------------
VArc::~VArc() = default;

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetLength return arc length.
 * @return length.
 */
auto VArc::GetLength() const -> qreal
{
    qreal length = qAbs(d->radius) * qDegreesToRadians(AngleArc());
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
auto VArc::GetP1() const -> QPointF
{
    QPointF const p1(GetCenter().x() + qAbs(d->radius), GetCenter().y());
    QLineF centerP1(static_cast<QPointF>(GetCenter()), p1);
    centerP1.setAngle(GetStartAngle());
    return centerP1.p2();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetP2 return point associated with end angle.
 * @return точку point.
 */
auto VArc::GetP2() const -> QPointF
{
    QPointF const p2(GetCenter().x() + qAbs(d->radius), GetCenter().y());
    QLineF centerP2(static_cast<QPointF>(GetCenter()), p2);
    centerP2.setAngle(GetEndAngle());
    return centerP2.p2();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetPoints return list of points needed for drawing arc.
 * @return list of points
 */
auto VArc::GetPoints() const -> QVector<QPointF>
{
    if (qFuzzyIsNull(d->radius))
    {
        return {GetCenter().toQPointF()};
    }

    QVector<QPointF> points;
    QVector<qreal> sectionAngle;

    QPointF pStart = IsFlipped() ? GetP2() : GetP1();

    {
        qreal angle = AngleArc();

        if (qFuzzyIsNull(angle))
        {
            points.append(pStart);
            return points;
        }

        if (angle > 360 || angle < 0)
        { // Filter incorect value
            QLineF dummy(0, 0, 100, 0);
            dummy.setAngle(angle);
            angle = dummy.angle();
        }

        const qreal angleInterpolation = 45; // degree
        const int sections = qFloor(angle / angleInterpolation);
        sectionAngle.reserve(sections + 1);
        for (int i = 0; i < sections; ++i)
        {
            sectionAngle.append(angleInterpolation);
        }

        const qreal tail = angle - sections * angleInterpolation;
        if (tail > 0)
        {
            sectionAngle.append(tail);
        }
    }

    for (int i = 0; i < sectionAngle.size(); ++i)
    {
        const qreal lDistance = qAbs(d->radius) * 4.0 / 3.0 * qTan(qDegreesToRadians(sectionAngle.at(i)) * 0.25);

        const QPointF center = static_cast<QPointF>(GetCenter());

        QLineF lineP1P2(pStart, center);
        lineP1P2.setAngle(lineP1P2.angle() - 90.0);
        lineP1P2.setLength(lDistance);

        QLineF lineP4P3(center, pStart);
        lineP4P3.setAngle(lineP4P3.angle() + sectionAngle.at(i));
        lineP4P3.setLength(qAbs(d->radius)); // in case of computing error
        lineP4P3 = QLineF(lineP4P3.p2(), center);
        lineP4P3.setAngle(lineP4P3.angle() + 90.0);
        lineP4P3.setLength(lDistance);

        VSpline spl(VPointF(pStart), lineP1P2.p2(), lineP4P3.p2(), VPointF(lineP4P3.p1()), 1.0);
        spl.SetApproximationScale(GetApproximationScale());
        QVector<QPointF> splPoints = spl.GetPoints();
        if (not splPoints.isEmpty() && i != sectionAngle.size() - 1)
        {
            splPoints.removeLast();
        }

        points << splPoints;
        pStart = lineP4P3.p1();
    }
    return IsFlipped() ? Reverse(points) : points;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CutArc cut arc into two arcs.
 * @param length length first arc.
 * @param arc1 first arc.
 * @param arc2 second arc.
 * @param pointName cutting point name.
 * @return point cutting
 */
auto VArc::CutArc(qreal length, VArc &arc1, VArc &arc2, const QString &pointName) const -> QPointF
{
    const qreal fullLength = GetLength();

    if (qFuzzyIsNull(fullLength) || qFuzzyIsNull(d->radius))
    {
        arc1 = *this;
        arc2 = *this;
        return GetCenter().toQPointF();
    }

    if (qFuzzyIsNull(length) || qFuzzyIsNull(length + fullLength))
    {
        arc1 = VArc(GetCenter(), d->radius, d->formulaRadius, GetStartAngle(), GetFormulaF1(), GetStartAngle(),
                    GetFormulaF1(), getIdObject(), getMode());
        arc1.SetApproximationScale(GetApproximationScale());
        arc1.SetFlipped(IsFlipped());
        arc1.SetAllowEmpty(true);

        arc2 = *this;

        return GetP1();
    }

    if (VFuzzyComparePossibleNulls(length, fullLength))
    {
        arc1 = *this;

        arc2 = VArc(GetCenter(), d->radius, d->formulaRadius, GetEndAngle(), GetFormulaF2(), GetEndAngle(),
                    GetFormulaF2(), getIdObject(), getMode());
        arc2.SetApproximationScale(GetApproximationScale());
        arc2.SetFlipped(IsFlipped());
        arc2.SetAllowEmpty(true);

        return GetP2();
    }

    QLineF const line =
        not IsFlipped() ? CutPoint(length, fullLength, pointName) : CutPointFlipped(length, fullLength, pointName);

    arc1 = VArc(GetCenter(), d->radius, d->formulaRadius, GetStartAngle(), GetFormulaF1(), line.angle(),
                QString().setNum(line.angle()), getIdObject(), getMode());
    arc1.SetApproximationScale(GetApproximationScale());
    arc1.SetFlipped(IsFlipped());

    arc2 = VArc(GetCenter(), d->radius, d->formulaRadius, line.angle(), QString().setNum(line.angle()), GetEndAngle(),
                GetFormulaF2(), getIdObject(), getMode());
    arc2.SetApproximationScale(GetApproximationScale());
    arc2.SetFlipped(IsFlipped());
    return line.p2();
}

//---------------------------------------------------------------------------------------------------------------------
auto VArc::CutArc(qreal length, const QString &pointName) const -> QPointF
{
    VArc arc1;
    VArc arc2;
    return CutArc(length, arc1, arc2, pointName);
}

//---------------------------------------------------------------------------------------------------------------------
auto VArc::OptimalApproximationScale(qreal radius, qreal f1, qreal f2, qreal tolerance) -> qreal
{
    if (qFuzzyIsNull(radius))
    {
        return maxCurveApproximationScale;
    }

    const qreal expectedCurvature = 1 / qAbs(radius);
    qreal scale = minCurveApproximationScale;

    do
    {
        VArc arc(VPointF(), radius, f1, f2);
        arc.SetApproximationScale(scale);
        qreal const curvature = Curvature(arc.GetPoints());

        if (expectedCurvature - curvature <= expectedCurvature * tolerance)
        {
            return scale;
        }
        scale += 0.1;
    } while (scale <= maxCurveApproximationScale);

    return maxCurveApproximationScale;
}

//---------------------------------------------------------------------------------------------------------------------
void VArc::CreateName()
{
    QString name = ARC_ + this->GetCenter().name();
    const QString nameStr = QStringLiteral("_%1");

    if (getMode() == Draw::Modeling && getIdObject() != NULL_ID)
    {
        name += nameStr.arg(getIdObject());
    }
    else if (VAbstractCurve::id() != NULL_ID)
    {
        name += nameStr.arg(VAbstractCurve::id());
    }

    if (GetDuplicate() > 0)
    {
        name += nameStr.arg(GetDuplicate());
    }

    setName(name);
}

//---------------------------------------------------------------------------------------------------------------------
void VArc::CreateAlias()
{
    const QString aliasSuffix = GetAliasSuffix();
    if (aliasSuffix.isEmpty())
    {
        SetAlias(QString());
        return;
    }

    SetAlias(ARC_ + aliasSuffix);
}

//---------------------------------------------------------------------------------------------------------------------
void VArc::FindF2(qreal length)
{
    SetFlipped(length < 0 || d->radius < 0);

    if (qAbs(length) >= qAbs(MaxLength()))
    {
        length = MaxLength();
    }

    qreal arcAngle = qAbs(qRadiansToDegrees(qAbs(length) / qAbs(d->radius)));

    if (IsFlipped())
    {
        arcAngle = arcAngle * -1;
    }

    QLineF startAngle(0, 0, 100, 0);
    startAngle.setAngle(GetStartAngle() + arcAngle); // We use QLineF just because it is easy way to correct angle value

    SetFormulaF2(QString::number(startAngle.angle()), startAngle.angle());
}

//---------------------------------------------------------------------------------------------------------------------
auto VArc::MaxLength() const -> qreal
{
    return M_2PI * d->radius;
}

//---------------------------------------------------------------------------------------------------------------------
auto VArc::CutPoint(qreal length, qreal fullLength, const QString &pointName) const -> QLineF
{
    if (length < 0)
    {
        length = fullLength + length;
    }

    if (length < 0)
    {
        QString errorMsg;
        if (not pointName.isEmpty())
        {
            errorMsg = tr("Curve '%1'. Length of a cut segment (%2) is too small. Optimize it to minimal value.")
                           .arg(name(), pointName);
        }
        else
        {
            errorMsg =
                tr("Curve '%1'. Length of a cut segment is too small. Optimize it to minimal value.").arg(name());
        }
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractApplication::warningMessageSignature + errorMsg;
    }
    else if (length > fullLength)
    {
        QString errorMsg;
        if (not pointName.isEmpty())
        {
            errorMsg = tr("Curve '%1'. Length of a cut segment (%2) is too big. Optimize it to maximal value.")
                           .arg(name(), pointName);
        }
        else
        {
            errorMsg = tr("Curve '%1'. Length of a cut segment is too big. Optimize it to maximal value.").arg(name());
        }
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractApplication::warningMessageSignature + errorMsg;
    }

    if (qFuzzyIsNull(d->radius))
    {
        return {static_cast<QPointF>(GetCenter()), static_cast<QPointF>(GetCenter())};
    }

    length = qBound(0.0, length, fullLength);

    QLineF line(static_cast<QPointF>(GetCenter()), GetP1());
    line.setAngle(line.angle() + qRadiansToDegrees(length / d->radius));
    return line;
}

//---------------------------------------------------------------------------------------------------------------------
auto VArc::CutPointFlipped(qreal length, qreal fullLength, const QString &pointName) const -> QLineF
{
    if (length > 0)
    {
        length = fullLength + length;
    }

    if (length < fullLength)
    {
        QString errorMsg;
        if (not pointName.isEmpty())
        {
            errorMsg = tr("Curve '%1'. Length of a cut segment (%2) is too small. Optimize it to minimal value.")
                           .arg(name(), pointName);
        }
        else
        {
            errorMsg =
                tr("Curve '%1'. Length of a cut segment is too small. Optimize it to minimal value.").arg(name());
        }
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractApplication::warningMessageSignature + errorMsg;
    }
    else if (length > 0)
    {
        QString errorMsg;
        if (not pointName.isEmpty())
        {
            errorMsg = tr("Curve '%1'. Length of a cut segment (%2) is too big. Optimize it to maximal value.")
                           .arg(name(), errorMsg);
        }
        else
        {
            errorMsg = tr("Curve '%1'. Length of a cut segment is too big. Optimize it to maximal value.").arg(name());
        }
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractApplication::warningMessageSignature + errorMsg;
    }

    if (qFuzzyIsNull(d->radius))
    {
        return {static_cast<QPointF>(GetCenter()), static_cast<QPointF>(GetCenter())};
    }

    length = qBound(fullLength, length, 0.0);

    QLineF line(static_cast<QPointF>(GetCenter()), GetP1());
    line.setAngle(line.angle() - qRadiansToDegrees(qAbs(length) / d->radius));
    return line;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetRadius return arc radius.
 * @return radius.
 */
auto VArc::GetFormulaRadius() const -> QString
{
    return d->formulaRadius;
}

//---------------------------------------------------------------------------------------------------------------------
void VArc::SetFormulaRadius(const QString &formula, qreal value)
{
    d->formulaRadius = formula;
    d->radius = value;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetRadius return formula for radius.
 * @return string with formula.
 */
auto VArc::GetRadius() const -> qreal
{
    return d->radius;
}
