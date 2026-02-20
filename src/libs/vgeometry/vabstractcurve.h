/************************************************************************
 **
 **  @file   vabstractcurve.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 6, 2014
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

#ifndef VABSTRACTCURVE_H
#define VABSTRACTCURVE_H

#include <QPointF>
#include <QSharedDataPointer>
#include <QString>
#include <QTypeInfo>
#include <QVector>
#include <QtGlobal>

#include "../vmisc/typedef.h"
#include "vgeometrydef.h"
#include "vgobject.h"

using DirectionArrow = QPair<QLineF, QLineF>;

class QPainterPath;
class VAbstractCurveData;
class VSplinePath;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")
QT_WARNING_DISABLE_GCC("-Wsuggest-final-methods")

class VAbstractCurve : public VGObject
{
public:
    explicit VAbstractCurve(const GOType &type, const quint32 &idObject = NULL_ID,
                            const Draw &mode = Draw::Calculation);
    VAbstractCurve(const VAbstractCurve &curve);
    ~VAbstractCurve() override;

    auto operator=(const VAbstractCurve &curve) -> VAbstractCurve &;

    VAbstractCurve(VAbstractCurve &&curve) noexcept;
    auto operator=(VAbstractCurve &&curve) noexcept -> VAbstractCurve &;

    virtual auto GetPoints() const -> QVector<QPointF> = 0;
    static auto GetSegmentPoints(const QVector<QPointF> &points, const QPointF &begin, const QPointF &end, bool reverse,
                                 QString &error) -> QVector<QPointF>;
    auto GetSegmentPoints(const QPointF &begin, const QPointF &end, bool reverse,
                          const QString &piece = QString()) const -> QVector<QPointF>;

    virtual auto GetPath() const -> QPainterPath;
    virtual auto GetLength() const -> qreal = 0;
    auto GetLengthByPoint(const QPointF &point) const -> qreal;
    virtual auto IntersectLine(const QLineF &line) const -> QVector<QPointF>;
    virtual auto IsIntersectLine(const QLineF &line) const -> bool;

    static auto IsPointOnCurve(const QVector<QPointF> &points, const QPointF &p) -> bool;
    auto IsPointOnCurve(const QPointF &p) const -> bool;

    auto ClosestPoint(QPointF scenePoint) const -> QPointF;

    virtual auto GetStartAngle() const -> qreal = 0;
    virtual auto GetEndAngle() const -> qreal = 0;

    auto GetDuplicate() const -> quint32;
    void SetDuplicate(quint32 number);

    auto GetColor() const -> QString;
    void SetColor(const QString &color);

    auto GetPenStyle() const -> QString;
    void SetPenStyle(const QString &penStyle);

    auto GetApproximationScale() const -> qreal;
    void SetApproximationScale(qreal value);

    auto IsDerivative() const -> bool;
    void SetDerivative(bool value);

    static auto PathLength(const QVector<QPointF> &path) -> qreal;

    static auto CurveIntersectLine(const QVector<QPointF> &points, const QLineF &line) -> QVector<QPointF>;
    static auto CurveIntersectAxis(const QPointF &point, qreal angle, const QVector<QPointF> &curvePoints,
                                   QPointF *intersectionPoint) -> bool;

    virtual auto GetMainNameForHistory() const -> QString = 0;
    virtual auto NameForHistory(const QString &toolName) const -> QString = 0;
    virtual auto DirectionArrows() const -> QVector<DirectionArrow>;
    static auto ShowDirection(const QVector<DirectionArrow> &arrows, qreal width) -> QPainterPath;

    static auto LengthCurveDirectionArrow() -> qreal;

    void SetAliasSuffix(const QString &aliasSuffix) override;

    virtual auto GetTypeHead() const -> QString = 0;
    virtual void SetNameSuffix(const QString &suffix) = 0;

    static auto Curvature(const QVector<QPointF> &vertices) -> double;

    virtual auto Offset(qreal distance, const QString &name = QString()) const -> VSplinePath = 0;
    virtual auto Outline(const QVector<qreal> &distances, const QString &name = QString()) const -> VSplinePath = 0;

    virtual auto HeadlessName() const -> QString = 0;

protected:
    virtual void CreateName() = 0;
    virtual void CreateAlias() = 0;

private:
    QSharedDataPointer<VAbstractCurveData> d;

    static auto FromBegin(const QVector<QPointF> &points, const QPointF &begin, bool *ok = nullptr) -> QVector<QPointF>;
    static auto ToEnd(const QVector<QPointF> &points, const QPointF &end, bool *ok = nullptr) -> QVector<QPointF>;
};

QT_WARNING_POP

#if QT_VERSION < QT_VERSION_CHECK(6, 7, 0)
Q_DECLARE_TYPEINFO(VAbstractCurve, Q_MOVABLE_TYPE); // NOLINT
#endif

#endif // VABSTRACTCURVE_H
