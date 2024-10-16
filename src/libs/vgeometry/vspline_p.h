/************************************************************************
 **
 **  @file   vspline_p.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 8, 2014
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

#ifndef VSPLINE_P_H
#define VSPLINE_P_H

#include <QLineF>
#include <QSharedData>
#include <QtMath>

#include "../vmisc/vabstractvalapplication.h"
#include "vpointf.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VSplineData final : public QSharedData
{
public:
    VSplineData() = default;
    VSplineData(const VSplineData &spline) = default;
    VSplineData(const VPointF &p1, const VPointF &p4, qreal angle1, qreal angle2, qreal kAsm1, qreal kAsm2,
                qreal kCurve);
    VSplineData(const VPointF &p1, QPointF p2, QPointF p3, const VPointF &p4);
    VSplineData(const VPointF &p1, const VPointF &p4, qreal angle1, const QString &angle1F, qreal angle2,
                const QString &angle2F, qreal c1Length, const QString &c1LengthF, qreal c2Length,
                const QString &c2LengthF);
    ~VSplineData() = default;

    static auto GetL(const QPointF &p1, const QPointF &p4, qreal kCurve) -> qreal;

    /** @brief p1 first spline point. */
    VPointF p1{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief p4 fourth spline point. */
    VPointF p4{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief angle1 first angle control line. */
    qreal angle1{0}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief angle1F the first control point angle formula*/
    QString angle1F{'0'}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief angle2 second angle control line. */
    qreal angle2{0}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief angle2F the second control point angle formula*/
    QString angle2F{'0'}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief c1Length the length from the first spline point to the first control point. */
    qreal c1Length{0}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief c1LengthF the formula from the first spline point to the first control point. */
    QString c1LengthF{'0'}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief c2Length the length from the fourth spline point to the second control point. */
    qreal c2Length{0}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief c2LengthF the formula length from the fourth spline point to the second control point. */
    QString c2LengthF{'0'}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief kCurve coefficient of curvature spline. */
    qreal kCurve{1}; // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VSplineData) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
inline VSplineData::VSplineData(const VPointF &p1, const VPointF &p4, qreal angle1, qreal angle2, qreal kAsm1,
                                qreal kAsm2, qreal kCurve)
  : p1(p1),
    p4(p4),
    angle1(angle1),
    angle1F(QString::number(angle1)),
    angle2(angle2),
    angle2F(QString::number(angle2)),
    kCurve(kCurve)
{
    const qreal L = GetL(static_cast<QPointF>(p1), static_cast<QPointF>(p4), kCurve);

    QLineF p1p2(p1.x(), p1.y(), p1.x() + L * kAsm1, p1.y());
    p1p2.setAngle(angle1);
    c1Length = p1p2.length();
    if (VAbstractValApplication::VApp())
    {
        c1LengthF = QString::number(VAbstractValApplication::VApp()->fromPixel(c1Length));
    }

    QLineF p4p3(p4.x(), p4.y(), p4.x() + L * kAsm2, p4.y());
    p4p3.setAngle(angle2);
    c2Length = p4p3.length();
    if (VAbstractValApplication::VApp())
    {
        c2LengthF = QString::number(VAbstractValApplication::VApp()->fromPixel(c2Length));
    }
}

//---------------------------------------------------------------------------------------------------------------------
inline VSplineData::VSplineData(const VPointF &p1, QPointF p2, QPointF p3, const VPointF &p4)
  : p1(p1),
    p4(p4)
{
    QLineF const p1p2(static_cast<QPointF>(p1), static_cast<QPointF>(p2));

    angle1 = p1p2.angle();
    angle1F = QString::number(angle1);

    c1Length = p1p2.length();
    if (VAbstractValApplication::VApp())
    {
        c1LengthF = QString::number(VAbstractValApplication::VApp()->fromPixel(c1Length));
    }

    QLineF const p4p3(static_cast<QPointF>(p4), static_cast<QPointF>(p3));

    angle2 = p4p3.angle();
    angle2F = QString::number(angle2);

    c2Length = p4p3.length();
    if (VAbstractValApplication::VApp())
    {
        c2LengthF = QString::number(VAbstractValApplication::VApp()->fromPixel(c2Length));
    }
}

//---------------------------------------------------------------------------------------------------------------------
inline VSplineData::VSplineData(const VPointF &p1, const VPointF &p4, qreal angle1, const QString &angle1F,
                                qreal angle2, const QString &angle2F, qreal c1Length, const QString &c1LengthF,
                                qreal c2Length, const QString &c2LengthF)
  : p1(p1),
    p4(p4),
    angle1(angle1),
    angle1F(angle1F),
    angle2(angle2),
    angle2F(angle2F),
    c1Length(c1Length),
    c1LengthF(c1LengthF),
    c2Length(c2Length),
    c2LengthF(c2LengthF)
{
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VSplineData::GetL(const QPointF &p1, const QPointF &p4, qreal kCurve) -> qreal
{
    static const qreal angle = 90;
    qreal const length = VFuzzyComparePoints(p1, p4) ? accuracyPointOnLine * 2 : QLineF(p1, p4).length();
    const qreal radius = length / M_SQRT2;
    return kCurve * radius * 4 / 3 * qTan(angle * M_PI_4 / 180.0);
}

QT_WARNING_POP

#endif // VSPLINE_P_H
