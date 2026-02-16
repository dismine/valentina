/************************************************************************
 **
 **  @file   vcubicbezierpath.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 3, 2016
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

#ifndef VCUBICBEZIERPATH_H
#define VCUBICBEZIERPATH_H

#include <QCoreApplication>
#include <QPointF>
#include <QSharedDataPointer>
#include <QString>
#include <QTypeInfo>
#include <QVector>
#include <QtGlobal>

#include "vabstractcubicbezierpath.h"
#include "vpointf.h"

class VCubicBezierPathData;

class VCubicBezierPath final : public VAbstractCubicBezierPath
{
    Q_DECLARE_TR_FUNCTIONS(VCubicBezierPath) // NOLINT

public:
    explicit VCubicBezierPath(quint32 idObject = 0, Draw mode = Draw::Calculation);
    VCubicBezierPath(const VCubicBezierPath &curve);
    explicit VCubicBezierPath(const QVector<VPointF> &points, quint32 idObject = 0, Draw mode = Draw::Calculation);
    ~VCubicBezierPath() override;

    auto Rotate(const QPointF &originPoint, qreal degrees, const QString &name = QString()) const -> VCubicBezierPath;
    auto Flip(const QLineF &axis, const QString &name = QString()) const -> VCubicBezierPath;
    auto Move(qreal length, qreal angle, const QString &name = QString()) const -> VCubicBezierPath;
    auto Offset(qreal distance, const QString &name = QString()) const -> VSplinePath override;
    auto Outline(const QVector<qreal> &distances, const QString &name = QString()) const -> VSplinePath override;

    auto operator=(const VCubicBezierPath &curve) -> VCubicBezierPath &;

    VCubicBezierPath(VCubicBezierPath &&curve) noexcept;
    auto operator=(VCubicBezierPath &&curve) noexcept -> VCubicBezierPath &;

    auto operator[](vsizetype indx) -> VPointF &;

    auto at(vsizetype indx) const -> const VPointF &;

    void append(const VPointF &point);

    auto CountSubSpl() const -> vsizetype override;
    auto CountPoints() const -> vsizetype override;
    void Clear() override;
    auto GetSpline(vsizetype index) const -> VSpline override;
    auto GetStartAngle() const -> qreal override;
    auto GetEndAngle() const -> qreal override;

    auto GetC1Length() const -> qreal override;
    auto GetC2Length() const -> qreal override;

    auto GetSplinePath() const -> QVector<VSplinePoint> override;
    auto GetCubicPath() const -> QVector<VPointF>;

    static auto CountSubSpl(vsizetype size) -> vsizetype;
    static auto SubSplOffset(vsizetype subSplIndex) -> vsizetype;
    static auto SubSplPointsCount(vsizetype countSubSpl) -> vsizetype;

    auto FirstPoint() const -> VPointF override;
    auto LastPoint() const -> VPointF override;

private:
    QSharedDataPointer<VCubicBezierPathData> d;
};

Q_DECLARE_TYPEINFO(VCubicBezierPath, Q_MOVABLE_TYPE); // NOLINT

#endif // VCUBICBEZIERPATH_H
