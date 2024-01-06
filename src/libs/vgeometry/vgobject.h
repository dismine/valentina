/************************************************************************
 **
 **  @file   vgobject.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 12, 2013
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

#ifndef VGOBJECT_H
#define VGOBJECT_H

#include <QPainterPath>
#include <QSharedDataPointer>
#include <QString>
#include <QTypeInfo>
#include <QVector>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "vgeometrydef.h"

class QLineF;
class QPoint;
class QPointF;
class QRectF;
class VGObjectData;
class QTransform;

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")
QT_WARNING_DISABLE_GCC("-Wsuggest-final-methods")

/**
 * @brief The VGObject class keep information graphical objects.
 */
class VGObject
{
public:
    VGObject();
    explicit VGObject(const GOType &type, const quint32 &idObject = 0, const Draw &mode = Draw::Calculation);
    VGObject(const VGObject &obj);

    virtual ~VGObject();

    auto operator=(const VGObject &obj) -> VGObject &;
#ifdef Q_COMPILER_RVALUE_REFS
    VGObject(VGObject &&obj) noexcept;
    auto operator=(VGObject &&obj) noexcept -> VGObject &;
#endif

    auto getIdObject() const -> quint32;
    void setIdObject(const quint32 &value);

    virtual auto name() const -> QString;
    void setName(const QString &name);

    auto getMode() const -> Draw;
    void setMode(const Draw &value);

    auto getType() const -> GOType;
    void setType(const GOType &type);

    auto id() const -> quint32;
    virtual void setId(const quint32 &id);

    virtual void SetAlias(const QString &alias);
    auto GetAlias() const -> QString;

    virtual void SetAliasSuffix(const QString &aliasSuffix);
    auto GetAliasSuffix() const -> QString;

    auto ObjectName() const -> QString;

    auto getIdTool() const -> quint32;

    virtual auto ToJson() const -> QJsonObject;

    static auto BuildLine(const QPointF &p1, const qreal &length, const qreal &angle) -> QLineF;
    static auto BuildRay(const QPointF &firstPoint, const qreal &angle, const QRectF &scRect) -> QPointF;
    static auto BuildAxis(const QPointF &p, const qreal &angle, const QRectF &scRect) -> QLineF;
    static auto BuildAxis(const QPointF &p1, const QPointF &p2, const QRectF &scRect) -> QLineF;

    static auto ContactPoints(const QPointF &p, const QPointF &center, qreal radius, QPointF &p1, QPointF &p2) -> int;
    static auto LineIntersectRect(const QRectF &rec, const QLineF &line) -> QPointF;
    static auto IntersectionCircles(const QPointF &c1, double r1, const QPointF &c2, double r2, QPointF &p1,
                                    QPointF &p2) -> int;
    static auto LineIntersectCircle(const QPointF &center, qreal radius, const QLineF &line, QPointF &p1, QPointF &p2)
        -> qint32;
    static auto ClosestPoint(const QLineF &line, const QPointF &point) -> QPointF;
    static auto addVector(const QPointF &p, const QPointF &p1, const QPointF &p2, qreal k) -> QPointF;
    static void LineCoefficients(const QLineF &line, qreal *a, qreal *b, qreal *c);
    static auto IsPointOnLineSegment(const QPointF &t, const QPointF &p1, const QPointF &p2,
                                     qreal accuracy = accuracyPointOnLine) -> bool;
    static auto IsLineSegmentOnLineSegment(const QLineF &seg1, const QLineF &seg2, qreal accuracy = accuracyPointOnLine)
        -> bool;
    static auto CorrectDistortion(const QPointF &t, const QPointF &p1, const QPointF &p2) -> QPointF;
    static auto IsPointOnLineviaPDP(const QPointF &t, const QPointF &p1, const QPointF &p2,
                                    qreal accuracy = accuracyPointOnLine) -> bool;
    static auto GetLengthContour(const QVector<QPointF> &contour, const QVector<QPointF> &newPoints) -> int;

    template <class T> static auto PainterPath(const QVector<T> &points) -> QPainterPath;

    static auto FlippingMatrix(const QLineF &axis) -> QTransform;

private:
    QSharedDataPointer<VGObjectData> d;

    static auto PointInCircle(const QPointF &p, const QPointF &center, qreal radius) -> int;
};

QT_WARNING_POP

Q_DECLARE_TYPEINFO(VGObject, Q_MOVABLE_TYPE); // NOLINT

//---------------------------------------------------------------------------------------------------------------------
template <class T> inline auto VGObject::PainterPath(const QVector<T> &points) -> QPainterPath
{
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);

    if (not points.isEmpty())
    {
        path.moveTo(points.at(0));
        for (qint32 i = 1; i < points.count(); ++i)
        {
            path.lineTo(points.at(i));
        }
        path.lineTo(points.at(0));
    }

    return path;
}

#endif // VGOBJECT_H
