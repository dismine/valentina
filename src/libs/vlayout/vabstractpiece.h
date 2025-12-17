/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 11, 2016
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

#ifndef VABSTRACTPIECE_H
#define VABSTRACTPIECE_H

#include <QCoreApplication>
#include <QDebug>
#include <QPainterPath>
#include <QPointF>
#include <QSharedDataPointer>
#include <QtGlobal>
#include <algorithm>

#include "../ifc/exception/vexception.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vgeometry/vgobject.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/testpath.h"
#include "../vmisc/vabstractapplication.h"
#include "vrawsapoint.h"
#include "vsapoint.h"

class VAbstractPieceData;
class QPainterPath;
class VGrainlineData;
class VContainer;
class VRawSAPoint;
class VLayoutPlaceLabel;
class VLayoutPoint;

template <typename T> struct IsLayoutPoint
{
    static const bool value = false;
};

template <> struct IsLayoutPoint<VLayoutPoint>
{
    static const bool value = true;
};

using PlaceLabelImg = QVector<QVector<VLayoutPoint>>;

class VAbstractPiece
{
    Q_DECLARE_TR_FUNCTIONS(VAbstractPiece) // NOLINT

public:
    VAbstractPiece();
    VAbstractPiece(const VAbstractPiece &piece);
    virtual ~VAbstractPiece();

    auto operator=(const VAbstractPiece &piece) -> VAbstractPiece &;

    VAbstractPiece(VAbstractPiece &&piece) noexcept;
    auto operator=(VAbstractPiece &&piece) noexcept -> VAbstractPiece &;

    auto GetName() const -> QString;
    void SetName(const QString &value);

    auto IsForbidFlipping() const -> bool;
    void SetForbidFlipping(bool value);

    auto IsForceFlipping() const -> bool;
    void SetForceFlipping(bool value);

    auto IsSymmetricalCopy() const -> bool;
    void SetSymmetricalCopy(bool value);

    auto IsFollowGrainline() const -> bool;
    void SetFollowGrainline(bool value);

    auto IsSeamAllowance() const -> bool;
    void SetSeamAllowance(bool value);

    auto IsSeamAllowanceBuiltIn() const -> bool;
    void SetSeamAllowanceBuiltIn(bool value);

    auto IsHideMainPath() const -> bool;
    void SetHideMainPath(bool value);

    auto IsSewLineOnDrawing() const -> bool;
    void SetSewLineOnDrawing(bool value);

    auto IsShowFullPiece() const -> bool;
    void SetShowFullPiece(bool value);

    auto GetSAWidth() const -> qreal;
    void SetSAWidth(qreal value);

    auto GetMx() const -> qreal;
    void SetMx(qreal value);

    auto GetMy() const -> qreal;
    void SetMy(qreal value);

    auto GetPriority() const -> uint;
    void SetPriority(uint value);

    auto GetFoldLineType() const -> FoldLineType;
    void SetFoldLineType(FoldLineType lineType);

    auto GetFoldLineSvgFontSize() const -> unsigned int;
    void SetFoldLineSvgFontSize(unsigned int size);

    auto IsFoldLineLabelFontItalic() const -> bool;
    void SetFoldLineLabelFontItalic(bool value);

    auto IsFoldLineLabelFontBold() const -> bool;
    void SetFoldLineLabelFontBold(bool value);

    auto GetFoldLineLabel() const -> QString;
    void SetFoldLineLabel(const QString &value);

    auto GetFoldLineLabelAlignment() const -> int;
    void SetFoldLineLabelAlignment(int alignment);

    auto IsShowMirrorLine() const -> bool;
    void SetShowMirrorLine(bool show);

    auto GetUUID() const -> QUuid;
    void SetUUID(const QUuid &uuid);
    void SetUUID(const QString &uuid);

    /**
     * @brief GetUniqueID returns unique piece id. Combines UUID and gradation label.
     * @return unique piece id.
     */
    virtual auto GetUniqueID() const -> QString;

    static auto Equidistant(QVector<VSAPoint> points, qreal width, const QString &name) -> QVector<VLayoutPoint>;
    static auto SumTrapezoids(const QVector<QPointF> &points) -> qreal;
    template <class T> static auto CheckLoops(QVector<T> points) -> QVector<T>;
    static auto EkvPoint(QVector<VRawSAPoint> points, const VSAPoint &p1Line1, const VSAPoint &p2Line1,
                         const VSAPoint &p1Line2, const VSAPoint &p2Line2, qreal width, bool *needRollback = nullptr)
        -> QVector<VRawSAPoint>;
    static auto ParallelLine(const VSAPoint &p1, const VSAPoint &p2, qreal width) -> QLineF;
    static auto IsAllowanceValid(const QVector<QPointF> &base, const QVector<QPointF> &allowance) -> bool;
    template <class T>
    static auto IsInsidePolygon(const QVector<T> &path, const QVector<T> &polygon, qreal accuracy = accuracyPointOnLine)
        -> bool;

    template<class T>
    static QVector<T> CorrectPathDistortion(QVector<T> path);

    template <class T>
    static auto CorrectEquidistantPoints(const QVector<T> &points, bool removeFirstAndLast = true) -> QVector<T>;

    static auto RollbackSeamAllowance(QVector<VRawSAPoint> points, const QLineF &cuttingEdge, bool *success)
        -> QVector<VRawSAPoint>;

    static auto GrainlineMainLine(const VGrainlineData &geom, const VContainer *pattern, const QRectF &boundingRect)
        -> QLineF;

    friend auto operator<<(QDataStream &dataStream, const VAbstractPiece &piece) -> QDataStream &;
    friend auto operator>>(QDataStream &dataStream, VAbstractPiece &piece) -> QDataStream &;

    static auto PlaceLabelShape(const VLayoutPlaceLabel &label) -> PlaceLabelImg;
    static auto LabelShapePath(const VLayoutPlaceLabel &label) -> QPainterPath;
    static auto LabelShapePath(const PlaceLabelImg &shape) -> QPainterPath;

    template <class T>
    static auto RemoveDublicates(const QVector<T> &points, bool removeFirstAndLast = true) -> QVector<T>;

    template <class T>
    static auto SubdividePath(const QVector<T> &boundary, const QPointF &p, QVector<T> &sub1, QVector<T> &sub2) -> bool;

    template <class T> static auto MirrorPath(const QVector<T> &points, const QLineF &mirrorLine) -> QVector<T>;

    template<class T>
    static auto FullSeamPath(QVector<T> points,
                             const QLineF &mirrorLine,
                             const QString &pieceName,
                             const QString &mirrorP1Label = QString(),
                             const QString &mirrorP2Label = QString()) -> QVector<T>;
    template<class T>
    static auto FullSeamAllowancePath(const QVector<T> &points,
                                      QLineF mirrorLine,
                                      const QString &pieceName,
                                      const QString &mirrorP1Label = QString(),
                                      const QString &mirrorP2Label = QString()) -> QVector<T>;

    template <class T>
    static auto MapVector(QVector<T> points, const QTransform &matrix, bool mirror = false) -> QVector<T>;

    template <typename T> static auto MapPoint(T obj, const QTransform &matrix) -> T;

    auto SeamAllowanceMirrorLine(const QLineF &seamMirrorLine, const QVector<QPointF> &seamAllowancePoints) const
        -> QLineF;

protected:
    static auto IsEkvPointOnLine(const QPointF &iPoint, const QPointF &prevPoint, const QPointF &nextPoint) -> bool;
    static auto IsEkvPointOnLine(const VSAPoint &iPoint, const VSAPoint &prevPoint, const VSAPoint &nextPoint) -> bool;
    template <class T>
    static auto CheckPointOnLine(QVector<T> &points, const T &iPoint, const T &prevPoint, const T &nextPoint) -> bool;

    static auto IsItemContained(const QRectF &parentBoundingRect, const QVector<QPointF> &shape, qreal &dX, qreal &dY)
        -> bool;
    static auto CorrectPosition(const QRectF &parentBoundingRect, QVector<QPointF> points) -> QVector<QPointF>;
    static auto FindGrainlineGeometry(const VGrainlineData &geom, const VContainer *pattern, qreal &length,
                                      qreal &rotationAngle, QPointF &pos) -> bool;
    template <class T> static auto ComparePoints(QVector<T> &points, const T &p1, const T &p2, qreal accuracy) -> bool;
    template <class T> static auto CompareFirstAndLastPoints(QVector<T> &points, qreal accuracy) -> void;
    template <class T> static auto CheckLoop(const QVector<T> &points, bool &loopFound) -> QVector<T>;
    template <class T>
    static auto IntersectionPoint(QPointF crosPoint, const T &l1p1, const T &l1p2, const T &l2p1, const T &l2p2) -> T;

private:
    QSharedDataPointer<VAbstractPieceData> d;

    template <typename T> static auto MakeTurnPoint(const QPointF &p) -> T;

    template <class T>
    static auto CorrectSAMirrolLine(const QVector<T> &points, const QLineF &mirrorLine, bool &reverse) -> QLineF;
};

Q_DECLARE_TYPEINFO(VAbstractPiece, Q_MOVABLE_TYPE); // NOLINT

//---------------------------------------------------------------------------------------------------------------------
template <class T>
inline auto VAbstractPiece::CheckPointOnLine(QVector<T> &points, const T &iPoint, const T &prevPoint,
                                             const T &nextPoint) -> bool
{
    if (iPoint.TurnPoint() || (iPoint.CurvePoint() && !prevPoint.CurvePoint()) ||
        !VGObject::IsPointOnLineSegment(iPoint, prevPoint, nextPoint) ||
        !IsEkvPointOnLine(iPoint, prevPoint, nextPoint))
    {
        points.append(iPoint);
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
template <>
inline auto VAbstractPiece::CheckPointOnLine<QPointF>(QVector<QPointF> &points, const QPointF &iPoint,
                                                      const QPointF &prevPoint, const QPointF &nextPoint) -> bool
{
    if (!VGObject::IsPointOnLineSegment(iPoint, prevPoint, nextPoint, accuracyPointOnLine / 4.))
    {
        points.append(iPoint);
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
template<class T>
QVector<T> VAbstractPiece::CorrectPathDistortion(QVector<T> path)
{
    if (path.size() < 3)
    {
        return path;
    }

    vsizetype prev = -1;
    for (vsizetype i = 0; i < path.size(); ++i)
    {
        if (prev == -1)
        {
            i == 0 ? prev = path.size() - 1 : prev = i-1;
        }

        vsizetype next = i + 1;
        if (i == path.size() - 1)
        {
            next = 0;
        }

        const QPointF &iPoint = path.at(i);
        const QPointF &prevPoint = path.at(prev);
        const QPointF &nextPoint = path.at(next);

        if (VGObject::IsPointOnLineSegment(iPoint, prevPoint, nextPoint))
        {
            const QPointF p = VGObject::CorrectDistortion(iPoint, prevPoint, nextPoint);
            path[i].setX(p.x());
            path[i].setY(p.y());
        }
    }

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CorrectEquidistantPoints clear equivalent points and remove point on line from equdistant.
 * @param points list of points equdistant.
 * @return corrected list.
 */
template <class T>
inline auto VAbstractPiece::CorrectEquidistantPoints(const QVector<T> &points, bool removeFirstAndLast) -> QVector<T>
{
    //    DumpVector(points, QStringLiteral("input.json.XXXXXX")); // Uncomment for dumping test data
    if (points.size() < 4) // Better don't check if only three points. We can destroy equidistant.
    {
        qDebug() << "Only three points.";
        return points;
    }

    // Clear equivalent points
    QVector<T> buf1 = RemoveDublicates(points, removeFirstAndLast);

    if (buf1.size() < 3)
    {
        return buf1;
    }

    vsizetype prev = -1;
    vsizetype next = -1;

    QVector<T> buf2;
    // Remove point on line
    for (qint32 i = 0; i < buf1.size(); ++i)
    { // In this case we alwayse will have bounded intersection, so all is need is to check if point is on line.
      // Unfortunatelly QLineF::intersect can't be used in this case because of the floating-point accuraccy problem.
        if (prev == -1)
        {
            if (i == 0)
            {
                prev = buf1.size() - 1;
                const T &prevPoint = buf1.at(prev);
                const T &iPoint = buf1.at(i);
                if (iPoint == prevPoint)
                {
                    prev = buf1.size() - 2;
                }
            }
            else
            {
                prev = i - 1;
            }
        }

        if (i == buf1.size() - 1)
        {
            next = 0;
            const T &nextPoint = buf1.at(next);
            const T &iPoint = buf1.at(i);
            if (iPoint == nextPoint)
            {
                next = 1;
            }
        }
        else
        {
            next = i + 1;
        }

        const T &iPoint = buf1.at(i);
        const T &prevPoint = buf1.at(prev);
        const T &nextPoint = buf1.at(next);

        if (not CheckPointOnLine(buf2, iPoint, prevPoint, nextPoint))
        {
            prev = -1;
        }
    }

    if (not buf2.isEmpty() && buf2.constFirst() != buf2.constLast())
    {
        buf2.append(buf2.constFirst());
    }

    buf2 = RemoveDublicates(buf2, false);

    //    DumpVector(buf2, QStringLiteral("output.json.XXXXXX")); // Uncomment for dumping test data
    return buf2;
}

//---------------------------------------------------------------------------------------------------------------------
template <class T>
inline auto VAbstractPiece::RemoveDublicates(const QVector<T> &points, bool removeFirstAndLast) -> QVector<T>
{
    if (points.size() < 4)
    {
        return points;
    }

    QVector<T> p;
    p.reserve(points.size());

    p.append(points.constFirst());

    // Default accuracy is not enough
    constexpr qreal accuracy = MmToPixel(0.5);

    for (int i = 0; i < points.size(); ++i)
    {
        for (int j = i + 1; j < points.size(); ++j)
        {
            if (not ComparePoints(p, points.at(i), points.at(j), accuracy))
            {
                i = j - 1;
                break;
            }
        }
    }

    if (removeFirstAndLast && not p.isEmpty() && p.size() > 1)
    {
        // Path can't be closed
        // See issue #686
        CompareFirstAndLastPoints(p, accuracy);
    }

    return p;
}

//---------------------------------------------------------------------------------------------------------------------
template <class T>
inline auto VAbstractPiece::ComparePoints(QVector<T> &points, const T &p1, const T &p2, qreal accuracy) -> bool
{
    qreal testAccuracy = accuracy;
    if (p2.TurnPoint())
    {
        testAccuracy = accuracyPointOnLine;
    }

    if (not VFuzzyComparePoints(p1, p2, testAccuracy))
    {
        points.append(p2);
        return false;
    }

    if (not points.isEmpty() && p2.TurnPoint())
    {
        points.last().SetTurnPoint(true);
    }

    if (not points.isEmpty() && p2.CurvePoint())
    {
        points.last().SetCurvePoint(true);
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
template <>
inline auto VAbstractPiece::ComparePoints(QVector<VRawSAPoint> &points, const VRawSAPoint &p1, const VRawSAPoint &p2,
                                          qreal accuracy) -> bool
{
    qreal testAccuracy = accuracy;
    if ((p1.Primary() && p2.Primary()) || p2.TurnPoint())
    {
        testAccuracy = accuracyPointOnLine;
    }

    if (not VFuzzyComparePoints(p1, p2, testAccuracy))
    {
        points.append(p2);
        return false;
    }

    if (not points.isEmpty() && p2.TurnPoint())
    {
        points.last().SetTurnPoint(true);
    }

    if (not points.isEmpty() && p2.CurvePoint())
    {
        points.last().SetCurvePoint(true);
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
template <>
inline auto VAbstractPiece::ComparePoints<QPointF>(QVector<QPointF> &points, const QPointF &p1, const QPointF &p2,
                                                   qreal accuracy) -> bool
{
    if (not VFuzzyComparePoints(p1, p2, accuracy))
    {
        points.append(p2);
        return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> inline auto VAbstractPiece::CompareFirstAndLastPoints(QVector<T> &points, qreal accuracy) -> void
{
    if (points.isEmpty())
    {
        return;
    }

    const T &first = points.constFirst();
    const T &last = points.constLast();

    qreal testAccuracy = accuracy;
    if (last.TurnPoint())
    {
        testAccuracy = accuracyPointOnLine;
    }

    if (VFuzzyComparePoints(first, last, testAccuracy))
    {
        points.removeLast();

        if (last.TurnPoint())
        {
            points.last().SetTurnPoint(true);
        }

        if (last.CurvePoint())
        {
            points.last().SetCurvePoint(true);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <> inline auto VAbstractPiece::CompareFirstAndLastPoints(QVector<VRawSAPoint> &points, qreal accuracy) -> void
{
    if (points.isEmpty())
    {
        return;
    }

    const VRawSAPoint &first = points.constFirst();
    const VRawSAPoint &last = points.constLast();

    qreal testAccuracy = accuracy;
    if ((first.Primary() && last.Primary()) || last.TurnPoint())
    {
        testAccuracy = accuracyPointOnLine;
    }

    if (VFuzzyComparePoints(first, last, testAccuracy))
    {
        points.removeLast();

        if (last.TurnPoint())
        {
            points.last().SetTurnPoint(true);
        }

        if (last.CurvePoint())
        {
            points.last().SetCurvePoint(true);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <>
inline auto VAbstractPiece::CompareFirstAndLastPoints<QPointF>(QVector<QPointF> &points, qreal accuracy) -> void
{
    if (points.isEmpty())
    {
        return;
    }

    if (VFuzzyComparePoints(points.constFirst(), points.constLast(), accuracy))
    {
        points.removeLast();
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class T>
inline auto VAbstractPiece::IsInsidePolygon(const QVector<T> &path, const QVector<T> &polygon, qreal accuracy) -> bool
{
    // Edges must not intersect
    for (auto i = 0; i < path.count(); ++i)
    {
        int nextI = -1;
        if (i < path.count() - 1)
        {
            nextI = i + 1;
        }
        else
        {
            nextI = 0;
        }

        QLineF baseSegment(path.at(i), path.at(nextI));
        if (baseSegment.isNull())
        {
            continue;
        }

        for (auto j = 0; j < polygon.count(); ++j)
        {
            int nextJ = -1;
            if (j < polygon.count() - 1)
            {
                nextJ = j + 1;
            }
            else
            {
                nextJ = 0;
            }

            QLineF allowanceSegment(polygon.at(j), polygon.at(nextJ));
            if (allowanceSegment.isNull())
            {
                continue;
            }

            QPointF crosPoint;
            const auto type = baseSegment.intersects(allowanceSegment, &crosPoint);

            if (type == QLineF::BoundedIntersection && not VFuzzyComparePoints(baseSegment.p1(), crosPoint, accuracy) &&
                not VFuzzyComparePoints(baseSegment.p2(), crosPoint, accuracy) &&
                not VGObject::IsPointOnLineviaPDP(allowanceSegment.p1(), baseSegment.p1(), baseSegment.p2(),
                                                  accuracy) &&
                not VGObject::IsPointOnLineviaPDP(allowanceSegment.p2(), baseSegment.p1(), baseSegment.p2(), accuracy))
            {
                return false;
            }
        }
    }

    // Just instersection edges is not enough. The base must be inside of the allowance.
    QPolygonF allowancePolygon(polygon);
    return std::all_of(path.begin(), path.end(),
                       [allowancePolygon](const T &point)
                       { return allowancePolygon.containsPoint(point, Qt::WindingFill); });
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CheckLoops seek and delete loops in equidistant.
 * @param points vector of points of equidistant.
 * @return vector of points of equidistant.
 */
template <class T> inline auto VAbstractPiece::CheckLoops(QVector<T> points) -> QVector<T>
{
    //    DumpVector(points, QStringLiteral("input.json.XXXXXX")); // Uncomment for dumping test data

    /*If we got less than 4 points no need seek loops.*/
    if (points.size() < 4)
    {
        return points;
    }

    bool loopFound = false;
    const int maxLoops = 10000; // limit number of loops to be removed

    for (qint32 i = 0; i < maxLoops; ++i)
    {
        points = CheckLoop(points, loopFound);
        if (not loopFound)
        {
            break;
        }
    }

    //    DumpVector(ekvPoints, QStringLiteral("output.json.XXXXXX")); // Uncomment for dumping test data
    return points;
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> inline auto VAbstractPiece::CheckLoop(const QVector<T> &points, bool &loopFound) -> QVector<T>
{
    loopFound = false;

    const bool pathClosed = (points.constFirst() == points.constLast());

    QVector<T> ekvPoints;
    ekvPoints.reserve(points.size());

    for (qint32 i = 0; i < points.size(); ++i)
    {
        /*Last three points no need to check.*/
        /*Triangle can not contain a loop*/
        if (loopFound || i > points.size() - 4)
        {
            ekvPoints.append(points.at(i));
            continue;
        }

        enum LoopIntersectType
        {
            NoIntersection,
            BoundedIntersection,
            ParallelIntersection
        };

        QPointF crosPoint;
        LoopIntersectType status = NoIntersection;
        const QLineF line1(points.at(i), points.at(i + 1));

        const int limit = pathClosed && i == 0 ? 2 : 1;
        qint32 j;
        for (j = i + 2; j < points.size() - limit; ++j)
        {
            QLineF line2(points.at(j), points.at(j + 1));

            const QLineF::IntersectType intersect = line1.intersects(line2, &crosPoint);
            if (intersect == QLineF::NoIntersection)
            { // According to the documentation QLineF::NoIntersection indicates that the lines do not intersect;
                // i.e. they are parallel. But parallel also mean they can be on the same line.
                // Method IsLineSegmentOnLineSegment will check it.
                if (VGObject::IsLineSegmentOnLineSegment(line1, line2))
                { // Now we really sure that segments are on the same line and have real intersections.
                    status = ParallelIntersection;
                    break;
                }
            }
            else if (intersect == QLineF::BoundedIntersection)
            {
                status = BoundedIntersection;
                break;
            }
        }

        switch (status)
        {
            case ParallelIntersection:
                /*We have found a loop.*/
                ekvPoints.append(points.at(i));
                ekvPoints.append(points.at(j + 1));
                i = j + 1; // Skip a loop
                loopFound = true;
                break;
            case BoundedIntersection:
                ekvPoints.append(points.at(i));
                ekvPoints.append(
                    IntersectionPoint(crosPoint, points.at(i), points.at(i + 1), points.at(j), points.at(j + 1)));
                i = j;
                loopFound = true;
                break;
            case NoIntersection:
                /*We have not found loop.*/
                ekvPoints.append(points.at(i));
                break;
            default:
                break;
        }
    }
    return ekvPoints;
}

//---------------------------------------------------------------------------------------------------------------------
template <class T>
inline auto VAbstractPiece::IntersectionPoint(QPointF crosPoint, const T &l1p1, const T &l1p2, const T &l2p1,
                                              const T &l2p2) -> T
{
    T point(crosPoint);

    if ((l1p1.CurvePoint() && l1p2.CurvePoint()) || (l2p1.CurvePoint() && l2p2.CurvePoint()) ||
        (l1p1.CurvePoint() && l2p2.CurvePoint()))
    {
        point.SetCurvePoint(true);
    }

    if ((l1p1.TurnPoint() && l1p2.TurnPoint()) || (l2p1.TurnPoint() && l2p2.TurnPoint()) ||
        (l1p1.TurnPoint() && l2p2.TurnPoint()))
    {
        point.SetTurnPoint(true);
    }

    return point;
}

//---------------------------------------------------------------------------------------------------------------------
template <>
inline auto VAbstractPiece::IntersectionPoint<QPointF>(QPointF crosPoint, const QPointF & /*unused*/,
                                                       const QPointF & /*unused*/, const QPointF & /*unused*/,
                                                       const QPointF & /*unused*/) -> QPointF
{
    return crosPoint;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline auto VAbstractPiece::MakeTurnPoint(const QPointF &p) -> T
{
    if constexpr (!IsLayoutPoint<T>::value)
    {
        return p;
    }
    else
    {
        T breakPoint(p);
        breakPoint.SetTurnPoint(true);
        return breakPoint;
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class T>
inline auto VAbstractPiece::SubdividePath(const QVector<T> &boundary, const QPointF &p, QVector<T> &sub1,
                                          QVector<T> &sub2) -> bool
{
    if (boundary.size() < 2)
    {
        return false;
    }

    bool found = false;
    sub1.clear();
    sub2.clear();

    for (qint32 i = 0; i < boundary.count() - 1; ++i)
    {
        if (found)
        {
            if (not VFuzzyComparePoints(boundary.at(i), p))
            {
                sub2.append(boundary.at(i));
            }

            if (i + 1 == boundary.count() - 1)
            {
                sub2.append(boundary.at(i + 1));
            }
            continue;
        }

        if (!VGObject::IsPointOnLineSegment(p,
                                            boundary.at(i).ToQPointF(),
                                            boundary.at(i + 1).ToQPointF()))
        {
            sub1.append(boundary.at(i));
            continue;
        }

        if (not VFuzzyComparePoints(boundary.at(i), p))
        {
            sub1.append(boundary.at(i));
        }

        QPointF const correctedPoint = VGObject::ClosestPoint(QLineF(boundary.at(i).ToQPointF(),
                                                                     boundary.at(i + 1).ToQPointF()),
                                                              p);

        sub1.append(MakeTurnPoint<T>(correctedPoint));
        sub2.append(MakeTurnPoint<T>(correctedPoint));

        if (i + 1 == boundary.count() - 1 && not VFuzzyComparePoints(boundary.at(i + 1), p))
        {
            sub2.append(boundary.at(i + 1));
        }

        found = true;
    }

    if (not found)
    {
        sub1.clear();
    }

    return found;
}

//---------------------------------------------------------------------------------------------------------------------
template <class T>
inline auto VAbstractPiece::MirrorPath(const QVector<T> &points, const QLineF &mirrorLine) -> QVector<T>
{
    QVector<T> flipped;
    flipped.reserve(points.size() * 2);

    const QTransform matrix = VGObject::FlippingMatrix(mirrorLine);

    for (const auto &p : points)
    {
        flipped.append(matrix.map(p));
    }

    return flipped;
}

//---------------------------------------------------------------------------------------------------------------------
template <>
inline auto VAbstractPiece::MirrorPath<VLayoutPoint>(const QVector<VLayoutPoint> &points, const QLineF &mirrorLine)
    -> QVector<VLayoutPoint>
{
    QVector<VLayoutPoint> flipped;
    flipped.reserve(points.size());

    const QTransform matrix = VGObject::FlippingMatrix(mirrorLine);

    for (const auto &p : points)
    {
        VLayoutPoint tmp = p;
        QPointF const flippedPoint = matrix.map(p.ToQPointF());
        tmp.setX(flippedPoint.x());
        tmp.setY(flippedPoint.y());
        flipped.append(tmp);
    }

    return flipped;
}

//---------------------------------------------------------------------------------------------------------------------
template<class T>
inline auto VAbstractPiece::FullSeamPath(QVector<T> points,
                                         const QLineF &mirrorLine,
                                         const QString &pieceName,
                                         const QString &mirrorP1Label,
                                         const QString &mirrorP2Label) -> QVector<T>
{
    // DumpVector(points, QStringLiteral("input.json.XXXXXX")); // Uncomment for dumping test data

    if (mirrorLine.isNull())
    {
        return points;
    }

    points = RemoveDublicates(points, false);

    if (points.size() <= 3)
    {
        return points;
    }

    // Function expects not closed path
    if (points.constFirst() == points.constLast())
    {
        points.removeLast();
    }

    QVector<T> base;
    base.reserve(points.size());

    if (VFuzzyComparePoints(points.constFirst(), mirrorLine.p2()) &&
        VFuzzyComparePoints(points.constLast(), mirrorLine.p1()))
    {
        base = points;
    }
    else
    {
        QVector<T> sub1;
        QVector<T> sub2;
        if (!VAbstractPiece::SubdividePath(points, mirrorLine.p1(), sub1, sub2))
        {
            const QString errorMsg = !mirrorP1Label.isEmpty()
                                         ? QObject::tr("Piece '%1'. Unable to generate full seam path at %2.")
                                               .arg(pieceName, mirrorP1Label)
                                         : QObject::tr("Piece '%1'. Unable to generate full seam path.").arg(pieceName);
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractApplication::warningMessageSignature + errorMsg;
            return points;
        }

        QVector<T> sub3;
        QVector<T> sub4;
        if (!VAbstractPiece::SubdividePath(points, mirrorLine.p2(), sub3, sub4))
        {
            const QString errorMsg = !mirrorP2Label.isEmpty()
                                         ? QObject::tr("Piece '%1'. Unable to generate full seam path at %2.")
                                               .arg(pieceName, mirrorP2Label)
                                         : QObject::tr("Piece '%1'. Unable to generate full seam path.").arg(pieceName);
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractApplication::warningMessageSignature + errorMsg;
            return points;
        }

        base = sub4 + sub1;
    }

    QVector<T> fullPath = MirrorPath(base, mirrorLine);
    std::reverse(fullPath.begin(), fullPath.end());
    fullPath += base;

    // DumpVector(fullPath, QStringLiteral("output.json.XXXXXX")); // Uncomment for dumping test data

    return fullPath;
}

//---------------------------------------------------------------------------------------------------------------------
template<class T>
inline auto VAbstractPiece::FullSeamAllowancePath(const QVector<T> &points,
                                                  QLineF mirrorLine,
                                                  const QString &pieceName,
                                                  const QString &mirrorP1Label,
                                                  const QString &mirrorP2Label) -> QVector<T>
{
    // DumpVector(points, QStringLiteral("input.json.XXXXXX")); // Uncomment for dumping test data

    if (mirrorLine.isNull() || points.size() <= 3)
    {
        return points;
    }

    bool reverse = false;
    mirrorLine = CorrectSAMirrolLine(points, mirrorLine, reverse);

    QVector<T> sub1;
    QVector<T> sub2;
    if (!VAbstractPiece::SubdividePath(points, mirrorLine.p1(), sub1, sub2))
    {
        const QString errorMsg = !mirrorP1Label.isEmpty()
                                     ? QObject::tr("Piece '%1'. Unable to generate full seam allowance path at %2.")
                                           .arg(pieceName, mirrorP1Label)
                                     : QObject::tr("Piece '%1'. Unable to generate full seam allowance path.")
                                           .arg(pieceName);
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractApplication::warningMessageSignature + errorMsg;
        return points;
    }

    QVector<QPointF> subPath;
    CastTo(sub2, subPath);

    std::reverse(sub2.begin(), sub2.end());

    QVector<T> sub3;
    QVector<T> sub4;
    if (!VAbstractPiece::SubdividePath(sub2, mirrorLine.p2(), sub3, sub4))
    {
        const QString errorMsg = !mirrorP2Label.isEmpty()
                                     ? QObject::tr("Piece '%1'. Unable to generate full seam allowance path at %2.")
                                           .arg(pieceName, mirrorP2Label)
                                     : QObject::tr("Piece '%1'. Unable to generate full seam allowance path.")
                                           .arg(pieceName);
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractApplication::warningMessageSignature + errorMsg;
        return points;
    }

    QVector<T> base;
    base.reserve(points.size());

    if (!reverse)
    {
        std::reverse(sub3.begin(), sub3.end());
        base = sub3 + sub1;
    }
    else
    {
        std::reverse(sub4.begin(), sub4.end());
        base = sub4;
    }

    QVector<T> fullPath = MirrorPath(base, mirrorLine);
    std::reverse(fullPath.begin(), fullPath.end());
    fullPath += base;

    // DumpVector(fullPath, QStringLiteral("output.json.XXXXXX")); // Uncomment for dumping test data

    return fullPath;
}

//---------------------------------------------------------------------------------------------------------------------
template <class T>
inline auto VAbstractPiece::MapVector(QVector<T> points, const QTransform &matrix, bool mirror) -> QVector<T>
{
    std::transform(points.begin(), points.end(), points.begin(),
                   [&matrix](const T &point) { return MapPoint(point, matrix); });
    if (mirror)
    {
        std::reverse(points.begin(), points.end());
    }
    return points;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> auto VAbstractPiece::MapPoint(T obj, const QTransform &matrix) -> T
{
    if constexpr (!IsLayoutPoint<T>::value)
    {
        return matrix.map(obj);
    }
    else
    {
        auto p = matrix.map(obj);
        obj.setX(p.x());
        obj.setY(p.y());
        return obj;
    }
}

//---------------------------------------------------------------------------------------------------------------------
template <class T>
inline auto VAbstractPiece::CorrectSAMirrolLine(const QVector<T> &points, const QLineF &mirrorLine, bool &reverse)
    -> QLineF
{
    if (points.isEmpty() || points.size() < 2)
    {
        reverse = false;
        return mirrorLine;
    }

    for (qint32 i = 0; i < points.count() - 1; ++i)
    {
        if (VGObject::IsPointOnLineSegment(mirrorLine.p1(), points.at(i), points.at(i + 1)))
        {
            reverse = false;
            return mirrorLine;
        }

        if (VGObject::IsPointOnLineSegment(mirrorLine.p2(), points.at(i), points.at(i + 1)))
        {
            reverse = true;
            return {mirrorLine.p2(), mirrorLine.p1()};
        }
    }

    reverse = false;
    return mirrorLine;
}

#endif // VABSTRACTPIECE_H
