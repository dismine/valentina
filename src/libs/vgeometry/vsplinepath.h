/************************************************************************
 **
 **  @file   vsplinepath.h
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

#ifndef VSPLINEPATH_H
#define VSPLINEPATH_H

#include <qcompilerdetection.h>
#include <QCoreApplication>
#include <QPainterPath>
#include <QPointF>
#include <QSharedDataPointer>
#include <QString>
#include <QTypeInfo>
#include <QVector>
#include <QtGlobal>

#include "vabstractcubicbezierpath.h"
#include "vgeometrydef.h"
#include "vpointf.h"
#include "vspline.h"
#include "vsplinepoint.h"

class VSplinePathData;

/**
 * @brief The VSplinePath class keep information about splinePath.
 */
class VSplinePath final :public VAbstractCubicBezierPath
{
    Q_DECLARE_TR_FUNCTIONS(VSplinePath) // NOLINT
public:
    explicit VSplinePath(quint32 idObject = 0, Draw mode = Draw::Calculation);
    VSplinePath(const QVector<VFSplinePoint> &points, qreal kCurve = 1, quint32 idObject = 0,
                Draw mode = Draw::Calculation);
    VSplinePath(const QVector<VSplinePoint> &points, quint32 idObject = 0, Draw mode = Draw::Calculation);
    VSplinePath(const VSplinePath& splPath);
    auto Rotate(const QPointF &originPoint, qreal degrees, const QString &prefix = QString()) const -> VSplinePath;
    auto Flip(const QLineF &axis, const QString &prefix = QString()) const -> VSplinePath;
    auto Move(qreal length, qreal angle, const QString &prefix = QString()) const -> VSplinePath;
    virtual ~VSplinePath() override;

    auto operator[](vsizetype indx) -> VSplinePoint &;
    auto operator=(const VSplinePath &path) -> VSplinePath &;
#ifdef Q_COMPILER_RVALUE_REFS
    VSplinePath(VSplinePath&& splPath) Q_DECL_NOTHROW;
    auto operator=(VSplinePath &&path) Q_DECL_NOTHROW->VSplinePath &;
#endif

    void   append(const VSplinePoint &point);

    virtual auto CountSubSpl() const -> vsizetype override;
    virtual auto CountPoints() const -> vsizetype override;
    virtual void    Clear() override;
    virtual auto GetSpline(vsizetype index) const -> VSpline override;

    virtual auto GetSplinePath() const -> QVector<VSplinePoint> override;
    auto GetFSplinePath() const -> QVector<VFSplinePoint>;

    virtual auto GetStartAngle() const -> qreal override;
    virtual auto GetEndAngle() const -> qreal override;

    virtual auto GetC1Length() const -> qreal override;
    virtual auto GetC2Length() const -> qreal override;

    void         UpdatePoint(qint32 indexSpline, const SplinePointPosition &pos, const VSplinePoint &point);
    auto GetSplinePoint(qint32 indexSpline, SplinePointPosition pos) const -> VSplinePoint;

    auto at(vsizetype indx) const -> const VSplinePoint &;

    virtual auto ToJson() const -> QJsonObject override;

protected:
    virtual auto FirstPoint() const -> VPointF override;
    virtual auto LastPoint() const -> VPointF override;

private:
    QSharedDataPointer<VSplinePathData> d;
};

Q_DECLARE_TYPEINFO(VSplinePath, Q_MOVABLE_TYPE); // NOLINT

#endif // VSPLINEPATH_H
