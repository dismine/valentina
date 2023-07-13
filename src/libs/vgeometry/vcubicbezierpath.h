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
    VCubicBezierPath(const QVector<VPointF> &points, quint32 idObject = 0, Draw mode = Draw::Calculation);
    auto Rotate(const QPointF &originPoint, qreal degrees, const QString &prefix = QString()) const -> VCubicBezierPath;
    auto Flip(const QLineF &axis, const QString &prefix = QString()) const -> VCubicBezierPath;
    auto Move(qreal length, qreal angle, const QString &prefix = QString()) const -> VCubicBezierPath;
    virtual ~VCubicBezierPath();

    auto operator=(const VCubicBezierPath &curve) -> VCubicBezierPath &;
#ifdef Q_COMPILER_RVALUE_REFS
    VCubicBezierPath(VCubicBezierPath &&curve) noexcept;
    auto operator=(VCubicBezierPath &&curve) noexcept -> VCubicBezierPath &;
#endif

    auto operator[](vsizetype indx) -> VPointF &;

    auto at(vsizetype indx) const -> const VPointF &;

    void append(const VPointF &point);

    virtual auto CountSubSpl() const -> vsizetype override;
    virtual auto CountPoints() const -> vsizetype override;
    virtual void Clear() override;
    virtual auto GetSpline(vsizetype index) const -> VSpline override;
    virtual auto GetStartAngle() const -> qreal override;
    virtual auto GetEndAngle() const -> qreal override;

    virtual auto GetC1Length() const -> qreal override;
    virtual auto GetC2Length() const -> qreal override;

    virtual auto GetSplinePath() const -> QVector<VSplinePoint> override;
    auto GetCubicPath() const -> QVector<VPointF>;

    static auto CountSubSpl(vsizetype size) -> vsizetype;
    static auto SubSplOffset(vsizetype subSplIndex) -> vsizetype;
    static auto SubSplPointsCount(vsizetype countSubSpl) -> vsizetype;

protected:
    virtual auto FirstPoint() const -> VPointF override;
    virtual auto LastPoint() const -> VPointF override;

private:
    QSharedDataPointer<VCubicBezierPathData> d;
};

Q_DECLARE_TYPEINFO(VCubicBezierPath, Q_MOVABLE_TYPE); // NOLINT

#endif // VCUBICBEZIERPATH_H
