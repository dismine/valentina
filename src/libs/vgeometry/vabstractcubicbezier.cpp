/************************************************************************
 **
 **  @file   vabstractcubicbezier.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 3, 2016
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

#include "vabstractcubicbezier.h"

#include <QFuture>
#include <QLineF>
#include <QMessageLogger>
#include <QPoint>
#include <QtConcurrent>
#include <QtDebug>

#include "../ifc/ifcdef.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/def.h"
#include "../vmisc/exception/vexception.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vmath.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CalcSqDistance calculate squared distance.
 * @param p1 coordinate of first point.
 * @param p2 coordinate of second point.
 * @return squared length.
 */
inline auto CalcSqDistance(QPointF p1, QPointF p2) -> qreal
{
    const qreal dx = p2.x() - p1.x();
    const qreal dy = p2.y() - p1.y();
    return dx * dx + dy * dy;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief PointBezier_r find spline point using four point of spline.
 * @param p1 coordinate of first point.
 * @param p2 coordinate of first control point.
 * @param p3 coordinate of second control point.
 * @param p4 coordinate of last point.
 * @param level level of recursion. In the begin 0.
 * @param approximationScale curve approximation scale.
 */
auto PointBezier_r(QPointF p1, QPointF p2, QPointF p3, QPointF p4, qint16 level, qreal approximationScale)
    -> QVector<QPointF>
{
    static constexpr double curve_collinearity_epsilon = 1e-30;
    static constexpr double curve_angle_tolerance_epsilon = 0.01;
    static constexpr double m_angle_tolerance = 0.0;
    static constexpr qint16 curve_recursion_limit = 32;
    static constexpr double m_cusp_limit = 0.0;

    if (level > curve_recursion_limit)
    {
        return {};
    }

    double m_distance_tolerance_square;

    m_distance_tolerance_square = 0.5 / approximationScale;
    m_distance_tolerance_square *= m_distance_tolerance_square;

    // Calculate all the mid-points of the line segments
    //----------------------
    const double x12 = (p1.x() + p2.x()) / 2;
    const double y12 = (p1.y() + p2.y()) / 2;
    const double x23 = (p2.x() + p3.x()) / 2;
    const double y23 = (p2.y() + p3.y()) / 2;
    const double x34 = (p3.x() + p4.x()) / 2;
    const double y34 = (p3.y() + p4.y()) / 2;
    const double x123 = (x12 + x23) / 2;
    const double y123 = (y12 + y23) / 2;
    const double x234 = (x23 + x34) / 2;
    const double y234 = (y23 + y34) / 2;
    const double x1234 = (x123 + x234) / 2;
    const double y1234 = (y123 + y234) / 2;

    // Try to approximate the full cubic curve by a single straight line
    //------------------
    const double dx = p4.x() - p1.x();
    const double dy = p4.y() - p1.y();

    double d2 = fabs((p2.x() - p4.x()) * dy - (p2.y() - p4.y()) * dx);

    QVector<QPointF> points;

    switch (double d3 = fabs((p3.x() - p4.x()) * dy - (p3.y() - p4.y()) * dx);
            (static_cast<int>(d2 > curve_collinearity_epsilon) << 1) + static_cast<int>(d3 > curve_collinearity_epsilon))
    {
        case 0:
        {
            // All collinear OR p1==p4
            //----------------------
            if (double k = dx * dx + dy * dy; k < 0.000000001)
            {
                d2 = CalcSqDistance(p1, p2);
                d3 = CalcSqDistance(p4, p3);
            }
            else
            {
                k = 1 / k;
                {
                    const double da1 = p2.x() - p1.x();
                    const double da2 = p2.y() - p1.y();
                    d2 = k * (da1 * dx + da2 * dy);
                }
                {
                    const double da1 = p3.x() - p1.x();
                    const double da2 = p3.y() - p1.y();
                    d3 = k * (da1 * dx + da2 * dy);
                }
                if (d2 > 0 && d2 < 1 && d3 > 0 && d3 < 1)
                {
                    // Simple collinear case, 1---2---3---4
                    // We can leave just two endpoints
                    return points;
                }
                if (d2 <= 0)
                {
                    d2 = CalcSqDistance(p2, p1);
                }
                else if (d2 >= 1)
                {
                    d2 = CalcSqDistance(p2, p4);
                }
                else
                {
                    d2 = CalcSqDistance(p2, {p1.x() + d2 * dx, p1.y() + d2 * dy});
                }

                if (d3 <= 0)
                {
                    d3 = CalcSqDistance(p3, p1);
                }
                else if (d3 >= 1)
                {
                    d3 = CalcSqDistance(p3, p4);
                }
                else
                {
                    d3 = CalcSqDistance(p3, {p1.x() + d3 * dx, p1.y() + d3 * dy});
                }
            }
            if (d2 > d3)
            {
                if (d2 < m_distance_tolerance_square)
                {
                    points.append(p2);
                    return points;
                }
            }
            else
            {
                if (d3 < m_distance_tolerance_square)
                {
                    points.append(p3);
                    return points;
                }
            }
            break;
        }
        case 1:
        {
            // p1,p2,p4 are collinear, p3 is significant
            //----------------------
            if (d3 * d3 <= m_distance_tolerance_square * (dx * dx + dy * dy))
            {
                if (m_angle_tolerance < curve_angle_tolerance_epsilon)
                {
                    points.append(QPointF(x23, y23));
                    return points;
                }

                // Angle Condition
                //----------------------
                double da1 = fabs(atan2(p4.y() - p3.y(), p4.x() - p3.x()) - atan2(p3.y() - p2.y(), p3.x() - p2.x()));
                if (da1 >= M_PI)
                {
                    da1 = M_2PI - da1;
                }

                if (da1 < m_angle_tolerance)
                {
                    points.append(p2);
                    points.append(p3);
                    return points;
                }

                if ((m_cusp_limit > 0.0 || m_cusp_limit < 0.0) && da1 > m_cusp_limit)
                {
                    points.append(p3);
                    return points;
                }
            }
            break;
        }
        case 2:
        {
            // p1,p3,p4 are collinear, p2 is significant
            //----------------------
            if (d2 * d2 <= m_distance_tolerance_square * (dx * dx + dy * dy))
            {
                if (m_angle_tolerance < curve_angle_tolerance_epsilon)
                {
                    points.append(QPointF(x23, y23));
                    return points;
                }

                // Angle Condition
                //----------------------
                double da1 = fabs(atan2(p3.y() - p2.y(), p3.x() - p2.x()) - atan2(p2.y() - p1.y(), p2.x() - p1.x()));
                if (da1 >= M_PI)
                {
                    da1 = M_2PI - da1;
                }

                if (da1 < m_angle_tolerance)
                {
                    points.append(p2);

                    points.append(p3);
                    return points;
                }

                if ((m_cusp_limit > 0.0 || m_cusp_limit < 0.0) && da1 > m_cusp_limit)
                {
                    points.append(p2);
                    return points;
                }
            }
            break;
        }
        case 3:
        {
            // Regular case
            //-----------------
            if ((d2 + d3) * (d2 + d3) <= m_distance_tolerance_square * (dx * dx + dy * dy))
            {
                // If the curvature doesn't exceed the distance_tolerance value
                // we tend to finish subdivisions.
                //----------------------
                if (m_angle_tolerance < curve_angle_tolerance_epsilon)
                {
                    points.append(QPointF(x23, y23));
                    return points;
                }

                // Angle & Cusp Condition
                //----------------------
                const double k = atan2(p3.y() - p2.y(), p3.x() - p2.x());
                double da1 = fabs(k - atan2(p2.y() - p1.y(), p2.x() - p1.x()));
                double da2 = fabs(atan2(p4.y() - p3.y(), p4.x() - p3.x()) - k);
                if (da1 >= M_PI)
                {
                    da1 = M_2PI - da1;
                }
                if (da2 >= M_PI)
                {
                    da2 = M_2PI - da2;
                }

                if (da1 + da2 < m_angle_tolerance)
                {
                    // Finally we can stop the recursion
                    //----------------------

                    points.append(QPointF(x23, y23));
                    return points;
                }

                if (m_cusp_limit > 0.0 || m_cusp_limit < 0.0)
                {
                    if (da1 > m_cusp_limit)
                    {
                        points.append(p2);
                        return points;
                    }

                    if (da2 > m_cusp_limit)
                    {
                        points.append(p3);
                        return points;
                    }
                }
            }
            break;
        }
        default:
            break;
    }

    // Continue subdivision
    //----------------------
    const auto nextLevel = static_cast<qint16>(level + 1);
    return PointBezier_r(p1, {x12, y12}, {x123, y123}, {x1234, y1234}, nextLevel, approximationScale)
           + PointBezier_r({x1234, y1234}, {x234, y234}, {x34, y34}, p4, nextLevel, approximationScale);
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VAbstractCubicBezier::VAbstractCubicBezier(const GOType &type, const quint32 &idObject, const Draw &mode)
  : VAbstractBezier(type, idObject, mode)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCubicBezier::operator=(const VAbstractCubicBezier &curve) -> VAbstractCubicBezier &
{
    if (&curve == this)
    {
        return *this;
    }
    VAbstractBezier::operator=(curve);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CutSpline cut spline.
 * @param length length first spline
 * @param spl1p2 second point of first spline
 * @param spl1p3 third point of first spline
 * @param spl2p2 second point of second spline
 * @param spl2p3 third point of second spline
 * @param pointName cutting point name.
 * @return point of cutting. This point is forth point of first spline and first point of second spline.
 */
auto VAbstractCubicBezier::CutSpline(qreal length, QPointF &spl1p2, QPointF &spl1p3, QPointF &spl2p2, QPointF &spl2p3,
                                     const QString &pointName) const -> QPointF
{
    // Always need return two splines, so we must correct wrong length.
    const qreal fullLength = GetLength();

    if (qFuzzyIsNull(fullLength))
    {
        spl1p2 = spl1p3 = spl2p2 = spl2p3 = static_cast<QPointF>(GetP1());

        return static_cast<QPointF>(GetP1());
    }

    if (length < 0)
    {
        length = fullLength + length;
    }

    if (length < 0)
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
    }
    else if (length > fullLength)
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
    }

    length = qBound(0.0, length, fullLength);

    const qreal parT = GetParmT(length);

    return CutSplineAtParam(parT, spl1p2, spl1p3, spl2p2, spl2p3);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCubicBezier::CutSplineAtParam(
    qreal t, QPointF &spl1p2, QPointF &spl1p3, QPointF &spl2p2, QPointF &spl2p3) const -> QPointF
{
    t = qBound(0.0, t, 1.0);

    QLineF seg1_2(static_cast<QPointF>(GetP1()), GetControlPoint1());
    seg1_2.setLength(seg1_2.length() * t);
    const QPointF p12 = seg1_2.p2();

    QLineF seg2_3(GetControlPoint1(), GetControlPoint2());
    seg2_3.setLength(seg2_3.length() * t);
    const QPointF p23 = seg2_3.p2();

    QLineF seg12_23(p12, p23);
    seg12_23.setLength(seg12_23.length() * t);
    const QPointF p123 = seg12_23.p2();

    QLineF seg3_4(GetControlPoint2(), static_cast<QPointF>(GetP4()));
    seg3_4.setLength(seg3_4.length() * t);
    const QPointF p34 = seg3_4.p2();

    QLineF seg23_34(p23, p34);
    seg23_34.setLength(seg23_34.length() * t);
    const QPointF p234 = seg23_34.p2();

    QLineF seg123_234(p123, p234);
    seg123_234.setLength(seg123_234.length() * t);
    const QPointF p1234 = seg123_234.p2();

    spl1p2 = p12;
    spl1p3 = p123;
    spl2p2 = p234;
    spl2p3 = p34;
    return p1234;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCubicBezier::NameForHistory(const QString &toolName) const -> QString
{
    QString const name = toolName + GetMainNameForHistory();

    QString alias;

    if (not GetAliasSuffix().isEmpty())
    {
        alias = QStringLiteral("%1 %2").arg(toolName, GetAliasSuffix());
    }

    return not alias.isEmpty() ? QStringLiteral("%1 (%2)").arg(alias, name) : name;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCubicBezier::GetMainNameForHistory() const -> QString
{
    QString name = QStringLiteral(" %1_%2").arg(GetP1().name(), GetP4().name());
    if (GetDuplicate() > 0)
    {
        name += QStringLiteral("_%1").arg(GetDuplicate());
    }
    return name;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCubicBezier::GetParmT(qreal length) const -> qreal
{
    const qreal base = GetRealLength();
    if (length < 0)
    {
        return 0;
    }

    if (length > base)
    {
        length = base;
    }

    constexpr qreal eps = ToPixel(0.00001, Unit::Mm);
    qreal parT = 0.5;
    qreal step = parT;
    qreal splLength = 0;

    do
    {
        splLength = RealLengthByT(parT);
        step /= 2.0;

        if (qFuzzyIsNull(step))
        {
            break;
        }

        splLength > length ? parT -= step : parT += step;
    } while (qAbs(splLength - length) > eps);

    return parT;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractCubicBezier::CreateName()
{
    if (!IsDerivative())
    {
        setName(GetTypeHead() + HeadlessName());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractCubicBezier::CreateAlias()
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
void VAbstractCubicBezier::SetNameSuffix(const QString &suffix)
{
    setName(GetTypeHead() + suffix);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCubicBezier::HeadlessName() const -> QString
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

    QString name = u"%1_%2"_s.arg(GetP1().name(), GetP4().name());
    if (GetDuplicate() > 0)
    {
        name += u"_%1"_s.arg(GetDuplicate());
    }
    return name;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCubicBezier::GetTypeHead() const -> QString
{
    return SPL_;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetCubicBezierPoints return list with cubic bezier curve points.
 * @param p1 first spline point.
 * @param p2 first control point.
 * @param p3 second control point.
 * @param p4 last spline point.
 * @param approximationScale curve approximation scale.
 * @return list of points.
 */
auto VAbstractCubicBezier::GetCubicBezierPoints(const QPointF &p1, const QPointF &p2, const QPointF &p3,
                                                const QPointF &p4, qreal approximationScale) -> QVector<QPointF>
{
    if (approximationScale < minCurveApproximationScale || approximationScale > maxCurveApproximationScale)
    {
        approximationScale = VAbstractApplication::VApp()->GlobalCurveApproximationScale();
    }

    QVector<QPointF> pvector;
    pvector.append(p1);
    pvector += PointBezier_r(p1, p2, p3, p4, 0, approximationScale);
    pvector.append(p4);

    return pvector;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief LengthBezier return spline length using 4 spline point.
 * @param p1 first spline point
 * @param p2 first control point.
 * @param p3 second control point.
 * @param p4 last spline point.
 * @param approximationScale curve approximation scale.
 * @return length.
 */
auto VAbstractCubicBezier::LengthBezier(const QPointF &p1, const QPointF &p2, const QPointF &p3, const QPointF &p4,
                                        qreal approximationScale) -> qreal
{
    return PathLength(GetCubicBezierPoints(p1, p2, p3, p4, approximationScale));
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCubicBezier::RealLengthByT(qreal t) const -> qreal
{
    if (t < 0 || t > 1)
    {
        qDebug() << "Wrong value t.";
        return 0;
    }
    QLineF seg1_2(static_cast<QPointF>(GetP1()), GetControlPoint1());
    seg1_2.setLength(seg1_2.length() * t);
    const QPointF p12 = seg1_2.p2();

    QLineF seg2_3(GetControlPoint1(), GetControlPoint2());
    seg2_3.setLength(seg2_3.length() * t);
    const QPointF p23 = seg2_3.p2();

    QLineF seg12_23(p12, p23);
    seg12_23.setLength(seg12_23.length() * t);
    const QPointF p123 = seg12_23.p2();

    QLineF seg3_4(GetControlPoint2(), static_cast<QPointF>(GetP4()));
    seg3_4.setLength(seg3_4.length() * t);
    const QPointF p34 = seg3_4.p2();

    QLineF seg23_34(p23, p34);
    seg23_34.setLength(seg23_34.length() * t);
    const QPointF p234 = seg23_34.p2();

    QLineF seg123_234(p123, p234);
    seg123_234.setLength(seg123_234.length() * t);
    const QPointF p1234 = seg123_234.p2();

    return LengthBezier(static_cast<QPointF>(GetP1()), p12, p123, p1234, maxCurveApproximationScale);
}
