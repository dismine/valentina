/************************************************************************
 **
 **  @file   vsplinepoint_p.h
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

#ifndef VSPLINEPOINT_P_H
#define VSPLINEPOINT_P_H

#include <QLineF>
#include <QSharedData>
#include <QtDebug>
#include <QtGlobal>

#include "../qmuparser/qmutokenparser.h"
#include "../vmisc/def.h"
#include "vpointf.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Weffc++")
QT_WARNING_DISABLE_GCC("-Wnon-virtual-dtor")

class VFSplinePointData final : public QSharedData
{
public:
    VFSplinePointData() = default;
    VFSplinePointData(const VPointF &pSpline, qreal kAsm1, qreal angle1, qreal kAsm2, qreal angle2);
    VFSplinePointData(const VFSplinePointData &point) = default;
    ~VFSplinePointData() = default;

    /** @brief pSpline point. */
    VPointF pSpline{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief angle1 first angle spline. */
    qreal angle1{0}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief angle2 second angle spline. */
    qreal angle2{180}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief kAsm1 coefficient of length first control line. */
    qreal kAsm1{1}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief kAsm2 coefficient of length second control line. */
    qreal kAsm2{1}; // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VFSplinePointData) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
inline VFSplinePointData::VFSplinePointData(const VPointF &pSpline, qreal kAsm1, qreal angle1, qreal kAsm2,
                                            qreal angle2)
  : pSpline(pSpline),
    angle1(angle1),
    angle2(angle2),
    kAsm1(kAsm1),
    kAsm2(kAsm2)
{
    if (VFuzzyComparePossibleNulls(angle1, angle2) || not qFuzzyCompare(qAbs(angle1 - angle2), 180))
    {
        this->angle2 = this->angle1 + 180;
    }
}

//--------------------------------------VSplinePointData---------------------------------------------------------------

class VSplinePointData final : public QSharedData
{
public:
    VSplinePointData() = default;
    VSplinePointData(const VPointF &pSpline, qreal angle1, const QString &angle1F, qreal angle2, const QString &angle2F,
                     qreal length1, const QString &length1F, qreal length2, const QString &length2F);
    VSplinePointData(const VSplinePointData &point);
    ~VSplinePointData() = default;

    /** @brief pSpline point. */
    VPointF pSpline{}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief angle1 first angle spline. */
    qreal angle1{0};      // NOLINT(misc-non-private-member-variables-in-classes)
    QString angle1F{'0'}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief angle2 second angle spline. */
    qreal angle2{180};                      // NOLINT(misc-non-private-member-variables-in-classes)
    QString angle2F{QStringLiteral("180")}; // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief length1 length a first control line. */
    qreal length1{ToPixel(0.01, Unit::Mm)}; // NOLINT(misc-non-private-member-variables-in-classes)
    QString length1F{'0'};                  // NOLINT(misc-non-private-member-variables-in-classes)

    /** @brief length2 length a second control line. */
    qreal length2{ToPixel(0.01, Unit::Mm)}; // NOLINT(misc-non-private-member-variables-in-classes)
    QString length2F{'0'};                  // NOLINT(misc-non-private-member-variables-in-classes)

private:
    Q_DISABLE_ASSIGN_MOVE(VSplinePointData) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
inline VSplinePointData::VSplinePointData(const VPointF &pSpline, qreal angle1, const QString &angle1F, qreal angle2,
                                          const QString &angle2F, qreal length1, const QString &length1F, qreal length2,
                                          const QString &length2F)
  : pSpline(pSpline),
    angle1(angle1),
    angle1F(angle1F),
    angle2(angle2),
    angle2F(angle2F),
    length1(length1),
    length1F(length1F),
    length2(length2),
    length2F(length2F)
{
    if (not VFuzzyComparePossibleNulls(qAbs(angle1 - angle2), 180))
    {
        QLineF line(0, 0, 100, 0);

        if (not qmu::QmuTokenParser::IsSingle(angle1F) || qmu::QmuTokenParser::IsSingle(angle2F))
        {
            line.setAngle(angle1 + 180);
            this->angle2 = line.angle();
            this->angle2F = QString::number(line.angle());
        }
        else
        {
            line.setAngle(angle2 + 180);
            this->angle1 = line.angle();
            this->angle1F = QString::number(line.angle());
        }
    }

    if (qFuzzyIsNull(this->length1))
    {
        this->length1 = ToPixel(0.01, Unit::Mm);
    }

    if (qFuzzyIsNull(this->length2))
    {
        this->length2 = ToPixel(0.01, Unit::Mm);
    }
}

//---------------------------------------------------------------------------------------------------------------------
inline VSplinePointData::VSplinePointData(const VSplinePointData &point)
  : QSharedData(point),
    pSpline(point.pSpline),
    angle1(point.angle1),
    angle1F(point.angle1F),
    angle2(point.angle2),
    angle2F(point.angle2F),
    length1(point.length1),
    length1F(point.length1F),
    length2(point.length2),
    length2F(point.length2F)
{
    if (qFuzzyIsNull(this->length1))
    {
        this->length1 = ToPixel(0.01, Unit::Mm);
    }

    if (qFuzzyIsNull(this->length2))
    {
        this->length2 = ToPixel(0.01, Unit::Mm);
    }
}

QT_WARNING_POP

#endif // VSPLINEPOINT_P_H
