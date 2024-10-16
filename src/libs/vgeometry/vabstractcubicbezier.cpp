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

#include "../ifc/exception/vexception.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/def.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vmath.h"

namespace
{
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CalcSqDistance calculate squared distance.
 * @param x1 х coordinate first point.
 * @param y1 у coordinate first point.
 * @param x2 х coordinate second point.
 * @param y2 у coordinate second point.
 * @return squared length.
 */
inline auto CalcSqDistance(qreal x1, qreal y1, qreal x2, qreal y2) -> qreal
{
    const qreal dx = x2 - x1;
    const qreal dy = y2 - y1;
    return dx * dx + dy * dy;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief PointBezier_r find spline point using four point of spline.
 * @param x1 х coordinate first point.
 * @param y1 у coordinate first point.
 * @param x2 х coordinate first control point.
 * @param y2 у coordinate first control point.
 * @param x3 х coordinate second control point.
 * @param y3 у coordinate second control point.
 * @param x4 х coordinate last point.
 * @param y4 у coordinate last point.
 * @param level level of recursion. In the begin 0.
 * @param points spline points coordinates.
 * @param approximationScale curve approximation scale.
 */
auto PointBezier_r(qreal x1, qreal y1, qreal x2, qreal y2, qreal x3, qreal y3, qreal x4, qreal y4, qint16 level,
                   QVector<QPointF> points, qreal approximationScale) -> QVector<QPointF>
{
    if (points.size() >= 2)
    {
        for (int i = 1; i < points.size(); ++i)
        {
            if (points.at(i - 1) == points.at(i))
            {
                qDebug("All neighbors points in path must be unique.");
            }
        }
    }

    const double curve_collinearity_epsilon = 1e-30;
    const double curve_angle_tolerance_epsilon = 0.01;
    const double m_angle_tolerance = 0.0;
    enum curve_recursion_limit_e
    {
        curve_recursion_limit = 32
    };
    const double m_cusp_limit = 0.0;

    double m_approximation_scale = approximationScale;
    if (m_approximation_scale < minCurveApproximationScale || m_approximation_scale > maxCurveApproximationScale)
    {
        m_approximation_scale = VAbstractApplication::VApp()->Settings()->GetCurveApproximationScale();
    }

    double m_distance_tolerance_square;

    m_distance_tolerance_square = 0.5 / m_approximation_scale;
    m_distance_tolerance_square *= m_distance_tolerance_square;

    if (level > curve_recursion_limit)
    {
        return points;
    }

    // Calculate all the mid-points of the line segments
    //----------------------
    const double x12 = (x1 + x2) / 2;
    const double y12 = (y1 + y2) / 2;
    const double x23 = (x2 + x3) / 2;
    const double y23 = (y2 + y3) / 2;
    const double x34 = (x3 + x4) / 2;
    const double y34 = (y3 + y4) / 2;
    const double x123 = (x12 + x23) / 2;
    const double y123 = (y12 + y23) / 2;
    const double x234 = (x23 + x34) / 2;
    const double y234 = (y23 + y34) / 2;
    const double x1234 = (x123 + x234) / 2;
    const double y1234 = (y123 + y234) / 2;

    // Try to approximate the full cubic curve by a single straight line
    //------------------
    const double dx = x4 - x1;
    const double dy = y4 - y1;

    double d2 = fabs((x2 - x4) * dy - (y2 - y4) * dx);
    double d3 = fabs((x3 - x4) * dy - (y3 - y4) * dx);

    switch ((static_cast<int>(d2 > curve_collinearity_epsilon) << 1) +
            static_cast<int>(d3 > curve_collinearity_epsilon))
    {
        case 0:
        {
            // All collinear OR p1==p4
            //----------------------
            double k = dx * dx + dy * dy;
            if (k < 0.000000001)
            {
                d2 = CalcSqDistance(x1, y1, x2, y2);
                d3 = CalcSqDistance(x4, y4, x3, y3);
            }
            else
            {
                k = 1 / k;
                {
                    const double da1 = x2 - x1;
                    const double da2 = y2 - y1;
                    d2 = k * (da1 * dx + da2 * dy);
                }
                {
                    const double da1 = x3 - x1;
                    const double da2 = y3 - y1;
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
                    d2 = CalcSqDistance(x2, y2, x1, y1);
                }
                else if (d2 >= 1)
                {
                    d2 = CalcSqDistance(x2, y2, x4, y4);
                }
                else
                {
                    d2 = CalcSqDistance(x2, y2, x1 + d2 * dx, y1 + d2 * dy);
                }

                if (d3 <= 0)
                {
                    d3 = CalcSqDistance(x3, y3, x1, y1);
                }
                else if (d3 >= 1)
                {
                    d3 = CalcSqDistance(x3, y3, x4, y4);
                }
                else
                {
                    d3 = CalcSqDistance(x3, y3, x1 + d3 * dx, y1 + d3 * dy);
                }
            }
            if (d2 > d3)
            {
                if (d2 < m_distance_tolerance_square)
                {
                    points.append(QPointF(x2, y2));
                    return points;
                }
            }
            else
            {
                if (d3 < m_distance_tolerance_square)
                {
                    points.append(QPointF(x3, y3));
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
                double da1 = fabs(atan2(y4 - y3, x4 - x3) - atan2(y3 - y2, x3 - x2));
                if (da1 >= M_PI)
                {
                    da1 = M_2PI - da1;
                }

                if (da1 < m_angle_tolerance)
                {
                    points.append(QPointF(x2, y2));
                    points.append(QPointF(x3, y3));
                    return points;
                }

                if ((m_cusp_limit > 0.0 || m_cusp_limit < 0.0) && da1 > m_cusp_limit)
                {
                    points.append(QPointF(x3, y3));
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
                double da1 = fabs(atan2(y3 - y2, x3 - x2) - atan2(y2 - y1, x2 - x1));
                if (da1 >= M_PI)
                {
                    da1 = M_2PI - da1;
                }

                if (da1 < m_angle_tolerance)
                {
                    points.append(QPointF(x2, y2));

                    points.append(QPointF(x3, y3));
                    return points;
                }

                if ((m_cusp_limit > 0.0 || m_cusp_limit < 0.0) && da1 > m_cusp_limit)
                {
                    points.append(QPointF(x2, y2));
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
                const double k = atan2(y3 - y2, x3 - x2);
                double da1 = fabs(k - atan2(y2 - y1, x2 - x1));
                double da2 = fabs(atan2(y4 - y3, x4 - x3) - k);
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
                        points.append(QPointF(x2, y2));
                        return points;
                    }

                    if (da2 > m_cusp_limit)
                    {
                        points.append(QPointF(x3, y3));
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
    auto BezierTailPoints = [x1234, y1234, x234, y234, x34, y34, x4, y4, level, approximationScale]()
    {
        QVector<QPointF> const tail;
        return PointBezier_r(x1234, y1234, x234, y234, x34, y34, x4, y4, static_cast<qint16>(level + 1), tail,
                             approximationScale);
    };

    auto BezierPoints = [x1, y1, x12, y12, x123, y123, x1234, y1234, level, points, approximationScale]()
    {
        return PointBezier_r(x1, y1, x12, y12, x123, y123, x1234, y1234, static_cast<qint16>(level + 1), points,
                             approximationScale);
    };

    if (level < 1)
    {
        QFuture<QVector<QPointF>> const futureBezier = QtConcurrent::run(BezierPoints);
        const QVector<QPointF> tail = BezierTailPoints();
        return futureBezier.result() + tail;
    }
    return BezierPoints() + BezierTailPoints();
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

    QLineF seg1_2(static_cast<QPointF>(GetP1()), GetControlPoint1());
    seg1_2.setLength(seg1_2.length() * parT);
    const QPointF p12 = seg1_2.p2();

    QLineF seg2_3(GetControlPoint1(), GetControlPoint2());
    seg2_3.setLength(seg2_3.length() * parT);
    const QPointF p23 = seg2_3.p2();

    QLineF seg12_23(p12, p23);
    seg12_23.setLength(seg12_23.length() * parT);
    const QPointF p123 = seg12_23.p2();

    QLineF seg3_4(GetControlPoint2(), static_cast<QPointF>(GetP4()));
    seg3_4.setLength(seg3_4.length() * parT);
    const QPointF p34 = seg3_4.p2();

    QLineF seg23_34(p23, p34);
    seg23_34.setLength(seg23_34.length() * parT);
    const QPointF p234 = seg23_34.p2();

    QLineF seg123_234(p123, p234);
    seg123_234.setLength(seg123_234.length() * parT);
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
    QString name = toolName + QStringLiteral(" %1_%2").arg(GetP1().name(), GetP4().name());
    if (GetDuplicate() > 0)
    {
        name += QStringLiteral("_%1").arg(GetDuplicate());
    }

    QString alias;

    if (not GetAliasSuffix().isEmpty())
    {
        alias = QStringLiteral("%1 %2").arg(toolName, GetAliasSuffix());
    }

    return not alias.isEmpty() ? QStringLiteral("%1 (%2)").arg(alias, name) : name;
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
    QString name = SPL_ + QStringLiteral("%1_%2").arg(GetP1().name(), GetP4().name());
    if (GetDuplicate() > 0)
    {
        name += QStringLiteral("_%1").arg(GetDuplicate());
    }

    setName(name);
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

    SetAlias(SPL_ + aliasSuffix);
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
    QVector<QPointF> pvector;
    pvector.append(p1);
    pvector =
        PointBezier_r(p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y(), p4.x(), p4.y(), 0, pvector, approximationScale);
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
