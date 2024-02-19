/************************************************************************
 **
 **  @file   vsplinepoint.h
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

#ifndef VSPLINEPOINT_H
#define VSPLINEPOINT_H

#include <QMetaType>
#include <QSharedDataPointer>
#include <QString>
#include <QTypeInfo>
#include <QtGlobal>

#include "vpointf.h"

class VFSplinePointData;

/**
 * @brief The VFSplinePoint class keep information about point in spline path. Each point have two angles and two
 * coefficient. Point represent at the same time first and last point of a spline.
 */
class VFSplinePoint
{
public:
    VFSplinePoint();
    VFSplinePoint(const VPointF &pSpline, qreal kAsm1, qreal angle1, qreal kAsm2, qreal angle2);
    VFSplinePoint(const VFSplinePoint &point);
    ~VFSplinePoint();

    auto operator=(const VFSplinePoint &point) -> VFSplinePoint &;

    VFSplinePoint(VFSplinePoint &&point) noexcept;
    auto operator=(VFSplinePoint &&point) noexcept -> VFSplinePoint &;

    auto P() const -> VPointF;
    void SetP(const VPointF &value);
    auto Angle1() const -> qreal;
    void SetAngle1(const qreal &value);
    void SetAngle2(const qreal &value);
    auto Angle2() const -> qreal;
    auto KAsm1() const -> qreal;
    void SetKAsm1(const qreal &value);
    auto KAsm2() const -> qreal;
    void SetKAsm2(const qreal &value);

protected:
    QSharedDataPointer<VFSplinePointData> d;
};

Q_DECLARE_METATYPE(VFSplinePoint)                  // NOLINT
Q_DECLARE_TYPEINFO(VFSplinePoint, Q_MOVABLE_TYPE); // NOLINT

class VSplinePointData;

/**
 * @brief The VSplinePoint class keep information about point in spline path. Each point have two angles and two
 * lengths. Point represent at the same time first and last point of a spline.
 */
class VSplinePoint
{
public:
    VSplinePoint();
    VSplinePoint(const VPointF &pSpline, qreal angle1, const QString &angle1F, qreal angle2, const QString &angle2F,
                 qreal length1, const QString &length1F, qreal length2, const QString &length2F);
    VSplinePoint(const VSplinePoint &point);
    ~VSplinePoint();

    auto operator=(const VSplinePoint &point) -> VSplinePoint &;

    VSplinePoint(VSplinePoint &&point) noexcept;
    auto operator=(VSplinePoint &&point) noexcept -> VSplinePoint &;

    auto P() const -> VPointF;
    void SetP(const VPointF &value);

    auto Angle1() const -> qreal;
    auto Angle1Formula() const -> QString;
    void SetAngle1(const qreal &value, const QString &angle1F);

    auto Angle2() const -> qreal;
    auto Angle2Formula() const -> QString;
    void SetAngle2(const qreal &value, const QString &angle2F);

    auto Length1() const -> qreal;
    auto Length1Formula() const -> QString;
    void SetLength1(const qreal &value, const QString &length1F);

    auto Length2() const -> qreal;
    auto Length2Formula() const -> QString;
    void SetLength2(const qreal &value, const QString &length2F);

    auto IsMovable() const -> bool;

    auto ToJson() const -> QJsonObject;

protected:
    QSharedDataPointer<VSplinePointData> d;
};

Q_DECLARE_METATYPE(VSplinePoint)                  // NOLINT
Q_DECLARE_TYPEINFO(VSplinePoint, Q_MOVABLE_TYPE); // NOLINT

#endif // VSPLINEPOINT_H
