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
class VSplinePath final : public VAbstractCubicBezierPath
{
    Q_DECLARE_TR_FUNCTIONS(VSplinePath) // NOLINT

public:
    explicit VSplinePath(quint32 idObject = 0, Draw mode = Draw::Calculation);
    explicit VSplinePath(const QVector<VFSplinePoint> &points, qreal kCurve = 1, quint32 idObject = 0,
                         Draw mode = Draw::Calculation);
    explicit VSplinePath(const QVector<VSplinePoint> &points, quint32 idObject = 0, Draw mode = Draw::Calculation);
    explicit VSplinePath(const QVector<VSpline> &path, quint32 idObject = 0, Draw mode = Draw::Calculation);
    VSplinePath(const VSplinePath &splPath);
    ~VSplinePath() override;

    auto Rotate(const QPointF &originPoint, qreal degrees, const QString &name = QString()) const -> VSplinePath;
    auto Flip(const QLineF &axis, const QString &name = QString()) const -> VSplinePath;
    auto Move(qreal length, qreal angle, const QString &name = QString()) const -> VSplinePath;
    auto Offset(qreal distance, const QString &name = QString()) const -> VSplinePath override;
    auto Outline(const QVector<qreal> &distances, const QString &name = QString()) const -> VSplinePath override;

    auto operator[](vsizetype indx) -> VSplinePoint &;
    auto operator=(const VSplinePath &path) -> VSplinePath &;

    VSplinePath(VSplinePath &&splPath) noexcept;
    auto operator=(VSplinePath &&path) noexcept -> VSplinePath &;

    void append(const VSplinePoint &point);

    auto CountSubSpl() const -> vsizetype override;
    auto CountPoints() const -> vsizetype override;
    void Clear() override;
    auto GetSpline(vsizetype index) const -> VSpline override;

    auto GetSplinePath() const -> QVector<VSplinePoint> override;
    auto GetFSplinePath() const -> QVector<VFSplinePoint>;

    auto GetStartAngle() const -> qreal override;
    auto GetEndAngle() const -> qreal override;

    auto GetC1Length() const -> qreal override;
    auto GetC2Length() const -> qreal override;

    void UpdatePoint(qint32 indexSpline, const SplinePointPosition &pos, const VSplinePoint &point);
    auto GetSplinePoint(qint32 indexSpline, SplinePointPosition pos) const -> VSplinePoint;

    auto at(vsizetype indx) const -> const VSplinePoint &;

    auto ToJson() const -> QJsonObject override;

    auto DirectionArrows() const -> QVector<DirectionArrow> override;

    void SetStrict(bool strict);

    auto NameForHistory(const QString &toolName) const -> QString override;

    void SetMainNameForHistory(const QString &name);
    auto GetMainNameForHistory() const -> QString override;

    auto FirstPoint() const -> VPointF override;
    auto LastPoint() const -> VPointF override;

private:
    QSharedDataPointer<VSplinePathData> d;
};

Q_DECLARE_TYPEINFO(VSplinePath, Q_MOVABLE_TYPE); // NOLINT

#endif // VSPLINEPATH_H
