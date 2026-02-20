/************************************************************************
 **
 **  @file   vabstractcubicbezier.h
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

#ifndef VABSTRACTCUBICBEZIER_H
#define VABSTRACTCUBICBEZIER_H

#include <QPointF>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "vabstractbezier.h"
#include "vgeometrydef.h"

class VPointF;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")

class VAbstractCubicBezier : public VAbstractBezier
{
public:
    explicit VAbstractCubicBezier(const GOType &type, const quint32 &idObject = NULL_ID,
                                  const Draw &mode = Draw::Calculation);
    VAbstractCubicBezier(const VAbstractCubicBezier &curve) = default;
    auto operator=(const VAbstractCubicBezier &curve) -> VAbstractCubicBezier &;
    ~VAbstractCubicBezier() override = default;

    VAbstractCubicBezier(VAbstractCubicBezier &&curve) noexcept = default;
    auto operator=(VAbstractCubicBezier &&curve) noexcept -> VAbstractCubicBezier & = default;

    virtual auto GetP1() const -> VPointF = 0;
    virtual auto GetP2() const -> VPointF = 0;
    virtual auto GetP3() const -> VPointF = 0;
    virtual auto GetP4() const -> VPointF = 0;

    auto CutSpline(qreal length, QPointF &spl1p2, QPointF &spl1p3, QPointF &spl2p2, QPointF &spl2p3,
                   const QString &pointName) const -> QPointF;
    auto CutSplineAtParam(qreal t, QPointF &spl1p2, QPointF &spl1p3, QPointF &spl2p2, QPointF &spl2p3) const -> QPointF;

    auto NameForHistory(const QString &toolName) const -> QString override;
    auto GetMainNameForHistory() const -> QString override;

    auto GetParmT(qreal length) const -> qreal;
    auto RealLengthByT(qreal t) const -> qreal;

    auto HeadlessName() const -> QString override;

    auto GetTypeHead() const -> QString override;
    void SetNameSuffix(const QString &suffix) override;

protected:
    void CreateName() override;
    void CreateAlias() override;

    static auto GetCubicBezierPoints(const QPointF &p1, const QPointF &p2, const QPointF &p3, const QPointF &p4,
                                     qreal approximationScale) -> QVector<QPointF>;
    static auto LengthBezier(const QPointF &p1, const QPointF &p2, const QPointF &p3, const QPointF &p4,
                             qreal approximationScale) -> qreal;

    virtual auto GetControlPoint1() const -> QPointF = 0;
    virtual auto GetControlPoint2() const -> QPointF = 0;
    virtual auto GetRealLength() const -> qreal = 0;
};

QT_WARNING_POP

#endif // VABSTRACTCUBICBEZIER_H
