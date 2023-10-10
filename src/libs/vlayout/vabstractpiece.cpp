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

#include "vabstractpiece.h"
#include "../ifc/exception/vexception.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vlayoutplacelabel.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/floatItemData/vgrainlinedata.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vpiecegrainline.h"
#include "qline.h"
#include "qmath.h"
#include "vabstractpiece_p.h"
#include "vrawsapoint.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineF>
#include <QPainterPath>
#include <QSet>
#include <QTemporaryFile>
#include <QVector>
#include <QtMath>

using namespace Qt::Literals::StringLiterals;

const qreal maxL = 3.5;

namespace
{
//---------------------------------------------------------------------------------------------------------------------
inline auto IsSameDirection(QPointF p1, QPointF p2, QPointF px) -> bool
{
    return qAbs(QLineF(p1, p2).angle() - QLineF(p1, px).angle()) < 0.001;
}

//---------------------------------------------------------------------------------------------------------------------
// Do we create a point outside of a path?
inline auto IsOutsidePoint(QPointF p1, QPointF p2, QPointF px) -> bool
{
    QLineF seg1(p1, p2);
    QLineF seg2(p1, px);

    return IsSameDirection(p1, p2, px) && seg2.length() >= seg1.length();
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR auto PointPosition(const QPointF &p, const QLineF &line) -> qreal
{
    return (line.p2().x() - line.p1().x()) * (p.y() - line.p1().y()) -
           (line.p2().y() - line.p1().y()) * (p.x() - line.p1().x());
}

//---------------------------------------------------------------------------------------------------------------------
auto AngleByLength(QVector<VRawSAPoint> points, QPointF p1, QPointF p2, QPointF p3, const QLineF &bigLine1, QPointF sp2,
                   const QLineF &bigLine2, const VSAPoint &p, qreal width, bool *needRollback = nullptr)
    -> QVector<VRawSAPoint>
{
    if (needRollback != nullptr)
    {
        *needRollback = false;
    }

    const QPointF sp1 = bigLine1.p1();
    const QPointF sp3 = bigLine2.p2();
    const qreal localWidth = p.MaxLocalSA(width);

    if (IsOutsidePoint(bigLine1.p1(), bigLine1.p2(), sp2) && IsOutsidePoint(bigLine2.p2(), bigLine2.p1(), sp2))
    {
        QLineF line(p2, sp2);
        const qreal length = line.length();
        if (length > localWidth * maxL)
        { // Cutting too long acut angle
            line.setLength(localWidth);
            QLineF cutLine(line.p2(), sp2); // Cut line is a perpendicular
            cutLine.setLength(length);      // Decided to take this length

            // We do not check intersection type because intersection must alwayse exist
            QPointF px;
            cutLine.setAngle(cutLine.angle() + 90);
            QLineF::IntersectType type = Intersects(QLineF(sp1, sp2), cutLine, &px);

            if (type == QLineF::NoIntersection)
            {
                qDebug() << "Couldn't find intersection with cut line.";
            }
            VRawSAPoint sp(px, p.CurvePoint(), p.TurnPoint());
            sp.SetPrimary(true);
            points.append(sp);

            cutLine.setAngle(cutLine.angle() - 180);
            type = Intersects(QLineF(sp2, sp3), cutLine, &px);

            if (type == QLineF::NoIntersection)
            {
                qDebug() << "Couldn't find intersection with cut line.";
            }
            sp = VRawSAPoint(px, p.CurvePoint(), p.TurnPoint());
            sp.SetPrimary(true);
            points.append(sp);
        }
        else
        { // The point just fine
            points.append(VRawSAPoint(sp2, p.CurvePoint(), p.TurnPoint()));
        }
    }
    else
    {
        QLineF edge1(p2, p1);
        QLineF edge2(p2, p3);
        const qreal angle = edge1.angleTo(edge2);

        if (angle > 180 && p.GetAngleType() != PieceNodeAngle::ByLengthCurve)
        {
            if (VGObject::IsPointOnLineSegment(sp2, bigLine2.p1(), bigLine2.p2()))
            {
                QLineF loop(bigLine1.p2(), sp2);
                loop.setLength(loop.length() + accuracyPointOnLine * 2.);
                points.append(VRawSAPoint(loop.p2(), p.CurvePoint(), p.TurnPoint()));
                points.append(VRawSAPoint(sp2, p.CurvePoint(), p.TurnPoint()));
                points.append(VRawSAPoint(bigLine1.p2(), p.CurvePoint(), p.TurnPoint(), true));

                loop = QLineF(bigLine2.p2(), sp2);
                loop.setLength(loop.length() + localWidth);
                points.append(VRawSAPoint(loop.p2(), p.CurvePoint(), p.TurnPoint(), true));
            }
            else
            {
                QLineF loop(sp2, bigLine1.p1());
                loop.setLength(accuracyPointOnLine * 2.);
                points.append(VRawSAPoint(loop.p2(), p.CurvePoint(), p.TurnPoint()));
                points.append(VRawSAPoint(sp2, p.CurvePoint(), p.TurnPoint()));

                loop = QLineF(bigLine1.p1(), sp2);
                loop.setLength(loop.length() + localWidth);
                points.append(VRawSAPoint(loop.p2(), p.CurvePoint(), p.TurnPoint(), true));
                points.append(VRawSAPoint(bigLine2.p1(), p.CurvePoint(), p.TurnPoint(), true));
            }
        }
        else
        {
            if (not IsOutsidePoint(bigLine1.p1(), bigLine1.p2(), sp2))
            {
                if (p.GetAngleType() != PieceNodeAngle::ByLengthCurve)
                {
                    bool success = false;
                    QVector<VRawSAPoint> temp = points;
                    temp.append(VRawSAPoint(bigLine1.p2(), p.CurvePoint(), p.TurnPoint()));
                    temp = VAbstractPiece::RollbackSeamAllowance(temp, bigLine2, &success);

                    if (success)
                    {
                        points = temp;
                    }

                    if (needRollback != nullptr)
                    {
                        *needRollback = not success;
                    }
                }
                else
                {
                    points.append(VRawSAPoint(sp2, p.CurvePoint(), p.TurnPoint()));
                }
            }
            else
            {
                if (p.GetAngleType() != PieceNodeAngle::ByLengthCurve)
                {
                    // Need to create artificial loop
                    QLineF loop1(sp2, sp1);
                    loop1.setLength(loop1.length() * 0.2);

                    // Need for the main path rule
                    points.append(VRawSAPoint(loop1.p2(), p.CurvePoint(), p.TurnPoint()));

                    loop1.setAngle(loop1.angle() + 180);
                    loop1.setLength(localWidth);
                    points.append(VRawSAPoint(loop1.p2(), p.CurvePoint(), p.TurnPoint()));
                    points.append(VRawSAPoint(bigLine2.p1(), p.CurvePoint(), p.TurnPoint()));
                }
                else
                {
                    points.append(VRawSAPoint(sp2, p.CurvePoint(), p.TurnPoint()));
                }
            }
        }
    }
    return points;
}

//---------------------------------------------------------------------------------------------------------------------
auto AngleByIntersection(const QVector<VRawSAPoint> &points, QPointF p1, QPointF p2, QPointF p3, const QLineF &bigLine1,
                         QPointF sp2, const QLineF &bigLine2, const VSAPoint &p, qreal width,
                         bool *needRollback = nullptr) -> QVector<VRawSAPoint>
{
    {
        QLineF edge1(p2, p1);
        QLineF edge2(p2, p3);
        const qreal angle = edge1.angleTo(edge2);

        if (angle > 180)
        {
            return AngleByLength(points, p1, p2, p3, bigLine1, sp2, bigLine2, p, width, needRollback);
        }
    }

    if (needRollback != nullptr)
    {
        *needRollback = false;
    }

    const qreal localWidth = p.MaxLocalSA(width);
    QVector<VRawSAPoint> pointsIntr = points;

    // First point
    QLineF edge2(p2, p3);

    QPointF px;
    QLineF::IntersectType type = Intersects(edge2, bigLine1, &px);

    if (type == QLineF::NoIntersection)
    {
        return AngleByLength(points, p1, p2, p3, bigLine1, sp2, bigLine2, p, width, needRollback);
    }

    if (IsOutsidePoint(bigLine1.p1(), bigLine1.p2(), px))
    {
        if (QLineF(p2, px).length() > localWidth * maxL)
        {
            return AngleByLength(points, p1, p2, p3, bigLine1, sp2, bigLine2, p, width, needRollback);
        }
        pointsIntr.append(VRawSAPoint(px, p.CurvePoint(), p.TurnPoint()));
    }
    else
    { // Because artificial loop can lead to wrong clipping we must rollback current seam allowance points
        bool success = false;
        QVector<VRawSAPoint> temp = pointsIntr;
        temp.append(VRawSAPoint(bigLine1.p2(), p.CurvePoint(), p.TurnPoint()));
        temp = VAbstractPiece::RollbackSeamAllowance(temp, edge2, &success);

        if (success)
        {
            pointsIntr = temp;
        }

        if (needRollback != nullptr)
        {
            *needRollback = not success;
        }
    }

    // Second point
    QLineF edge1(p1, p2);
    type = Intersects(edge1, bigLine2, &px);

    if (type == QLineF::NoIntersection)
    {
        return AngleByLength(points, p1, p2, p3, bigLine1, sp2, bigLine2, p, width, needRollback);
    }

    if (IsOutsidePoint(bigLine2.p2(), bigLine2.p1(), px))
    {
        pointsIntr.append(VRawSAPoint(px, p.CurvePoint(), p.TurnPoint()));
    }
    else
    {
        pointsIntr.append(VRawSAPoint(px, p.CurvePoint(), p.TurnPoint()));

        QLineF allowance(p2, px);
        allowance.setLength(allowance.length() + localWidth * 3.);
        pointsIntr.append(VRawSAPoint(allowance.p2(), p.CurvePoint(), p.TurnPoint()));
        pointsIntr.append(VRawSAPoint(bigLine2.p1(), p.CurvePoint(), p.TurnPoint()));
    }

    return pointsIntr;
}

//---------------------------------------------------------------------------------------------------------------------
auto AngleByFirstSymmetry(const QVector<VRawSAPoint> &points, QPointF p1, QPointF p2, QPointF p3,
                          const QLineF &bigLine1, QPointF sp2, const QLineF &bigLine2, const VSAPoint &p, qreal width,
                          bool *needRollback = nullptr) -> QVector<VRawSAPoint>
{
    {
        QLineF edge1(p2, p1);
        QLineF edge2(p2, p3);
        const qreal angle = edge1.angleTo(edge2);

        if (angle > 180)
        {
            return AngleByLength(points, p1, p2, p3, bigLine1, sp2, bigLine2, p, width, needRollback);
        }
    }

    if (needRollback != nullptr)
    {
        *needRollback = false;
    }

    const QLineF axis = QLineF(p1, p2);

    QLineF sEdge(VPointF::FlipPF(axis, bigLine2.p1()), VPointF::FlipPF(axis, bigLine2.p2()));

    QPointF px1;
    QLineF::IntersectType type = Intersects(sEdge, bigLine1, &px1);

    if (type == QLineF::NoIntersection)
    {
        return AngleByLength(points, p1, p2, p3, bigLine1, sp2, bigLine2, p, width, needRollback);
    }

    QPointF px2;
    type = Intersects(sEdge, bigLine2, &px2);

    if (type == QLineF::NoIntersection)
    {
        return AngleByLength(points, p1, p2, p3, bigLine1, sp2, bigLine2, p, width, needRollback);
    }

    QVector<VRawSAPoint> pointsIntr = points;

    if (IsOutsidePoint(bigLine1.p1(), bigLine1.p2(), px1))
    {
        pointsIntr.append(VRawSAPoint(px1, p.CurvePoint(), p.TurnPoint()));
    }
    else
    { // Because artificial loop can lead to wrong clipping we must rollback current seam allowance points
        bool success = false;
        QVector<VRawSAPoint> temp = pointsIntr;
        temp.append(VRawSAPoint(bigLine1.p2(), p.CurvePoint(), p.TurnPoint()));
        temp = VAbstractPiece::RollbackSeamAllowance(temp, sEdge, &success);

        if (success)
        {
            pointsIntr = temp;
        }

        if (needRollback != nullptr)
        {
            *needRollback = not success;
        }
    }

    if (IsOutsidePoint(bigLine2.p2(), bigLine2.p1(), px2))
    {
        pointsIntr.append(VRawSAPoint(px2, p.CurvePoint(), p.TurnPoint()));
    }
    else
    {
        QLineF allowance(px2, p2);
        allowance.setAngle(allowance.angle() + 90);
        pointsIntr.append(VRawSAPoint(px2, p.CurvePoint(), p.TurnPoint()));
        pointsIntr.append(VRawSAPoint(allowance.p2(), p.CurvePoint(), p.TurnPoint()));
        pointsIntr.append(VRawSAPoint(bigLine2.p1(), p.CurvePoint(), p.TurnPoint()));
    }

    return pointsIntr;
}

//---------------------------------------------------------------------------------------------------------------------
auto AngleBySecondSymmetry(const QVector<VRawSAPoint> &points, QPointF p1, QPointF p2, QPointF p3,
                           const QLineF &bigLine1, QPointF sp2, const QLineF &bigLine2, const VSAPoint &p, qreal width,
                           bool *needRollback = nullptr) -> QVector<VRawSAPoint>
{
    {
        QLineF edge1(p2, p1);
        QLineF edge2(p2, p3);
        const qreal angle = edge1.angleTo(edge2);

        if (angle > 180)
        {
            return AngleByLength(points, p1, p2, p3, bigLine1, sp2, bigLine2, p, width, needRollback);
        }
    }

    if (needRollback != nullptr)
    {
        *needRollback = false;
    }

    const QLineF axis = QLineF(p3, p2);

    QLineF sEdge(VPointF::FlipPF(axis, bigLine1.p1()), VPointF::FlipPF(axis, bigLine1.p2()));

    QPointF px1;
    QLineF::IntersectType type = Intersects(sEdge, bigLine1, &px1);

    if (type == QLineF::NoIntersection)
    {
        return AngleByLength(points, p1, p2, p3, bigLine1, sp2, bigLine2, p, width, needRollback);
    }

    QPointF px2;
    type = Intersects(sEdge, bigLine2, &px2);

    if (type == QLineF::NoIntersection)
    {
        return AngleByLength(points, p1, p2, p3, bigLine1, sp2, bigLine2, p, width, needRollback);
    }

    const qreal localWidth = p.MaxLocalSA(width);
    QVector<VRawSAPoint> pointsIntr = points;

    if (IsOutsidePoint(bigLine1.p1(), bigLine1.p2(), px1))
    {
        pointsIntr.append(VRawSAPoint(px1, p.CurvePoint(), p.TurnPoint()));
    }
    else
    { // Because artificial loop can lead to wrong clipping we must rollback current seam allowance points
        bool success = false;
        QVector<VRawSAPoint> temp = pointsIntr;
        temp.append(VRawSAPoint(bigLine1.p2(), p.CurvePoint(), p.TurnPoint()));
        temp = VAbstractPiece::RollbackSeamAllowance(temp, sEdge, &success);

        if (success)
        {
            pointsIntr = temp;
        }

        if (needRollback != nullptr)
        {
            *needRollback = not success;
        }
    }

    if (IsOutsidePoint(bigLine2.p2(), bigLine2.p1(), px2))
    {
        pointsIntr.append(VRawSAPoint(px2, p.CurvePoint(), p.TurnPoint()));
    }
    else
    {
        QLineF allowance(p2, px2);
        allowance.setLength(p.GetSAAfter(width) * 0.98);
        pointsIntr.append(VRawSAPoint(allowance.p2(), p.CurvePoint(), p.TurnPoint()));
        allowance.setLength(allowance.length() + localWidth * 3.);
        pointsIntr.append(VRawSAPoint(allowance.p2(), p.CurvePoint(), p.TurnPoint()));
        pointsIntr.append(VRawSAPoint(bigLine2.p1(), p.CurvePoint(), p.TurnPoint()));
    }

    return pointsIntr;
}

//---------------------------------------------------------------------------------------------------------------------
auto AngleByFirstRightAngle(const QVector<VRawSAPoint> &points, QPointF p1, QPointF p2, QPointF p3,
                            const QLineF &bigLine1, QPointF sp2, const QLineF &bigLine2, const VSAPoint &p, qreal width,
                            bool *needRollback = nullptr) -> QVector<VRawSAPoint>
{
    {
        QLineF edge1(p2, p1);
        QLineF edge2(p2, p3);
        const qreal angle = edge1.angleTo(edge2);

        if (angle > 270)
        {
            return AngleByLength(points, p1, p2, p3, bigLine1, sp2, bigLine2, p, width, needRollback);
        }
    }

    const qreal localWidth = p.MaxLocalSA(width);
    QVector<VRawSAPoint> pointsRA = points;
    QLineF edge(p1, p2);

    QPointF px;
    QLineF::IntersectType type = Intersects(edge, bigLine2, &px);

    if (type == QLineF::NoIntersection)
    {
        return AngleByLength(points, p1, p2, p3, bigLine1, sp2, bigLine2, p, width, needRollback);
    }

    QLineF seam(px, p1);
    seam.setAngle(seam.angle() - 90);
    seam.setLength(p.GetSABefore(width));

    if (IsOutsidePoint(bigLine2.p2(), bigLine2.p1(), seam.p1()) && IsSameDirection(p1, p2, px))
    {
        if (QLineF(p2, px).length() > localWidth * maxL)
        {
            return AngleByLength(points, p1, p2, p3, bigLine1, sp2, bigLine2, p, width, needRollback);
        }
        pointsRA.append(VRawSAPoint(seam.p2(), p.CurvePoint(), p.TurnPoint()));
        pointsRA.append(VRawSAPoint(seam.p1(), p.CurvePoint(), p.TurnPoint()));
    }
    else
    {
        QLineF edge1(p2, p1);
        QLineF edge2(p2, p3);
        const qreal angle = edge1.angleTo(edge2);

        if (angle > 180)
        {
            return AngleByLength(points, p1, p2, p3, bigLine1, sp2, bigLine2, p, width, needRollback);
        }

        pointsRA.append(VRawSAPoint(seam.p2(), p.CurvePoint(), p.TurnPoint()));

        QLineF loopLine(px, sp2);
        const qreal length = loopLine.length() * 0.98;
        loopLine.setLength(length);

        QLineF tmp(seam.p2(), seam.p1());
        tmp.setLength(tmp.length() + length);

        pointsRA.append(VRawSAPoint(tmp.p2(), p.CurvePoint(), p.TurnPoint()));
        pointsRA.append(VRawSAPoint(loopLine.p2(), p.CurvePoint(), p.TurnPoint()));
    }

    return pointsRA;
}

//---------------------------------------------------------------------------------------------------------------------
auto AngleBySecondRightAngle(QVector<VRawSAPoint> points, QPointF p1, QPointF p2, QPointF p3, const QLineF &bigLine1,
                             QPointF sp2, const QLineF &bigLine2, const VSAPoint &p, qreal width,
                             bool *needRollback = nullptr) -> QVector<VRawSAPoint>
{
    {
        QLineF edge1(p2, p1);
        QLineF edge2(p2, p3);
        const qreal angle = edge1.angleTo(edge2);

        if (angle > 270)
        {
            return AngleByLength(points, p1, p2, p3, bigLine1, sp2, bigLine2, p, width, needRollback);
        }
    }

    if (needRollback != nullptr)
    {
        *needRollback = false;
    }

    const qreal localWidth = p.MaxLocalSA(width);
    QLineF edge(p2, p3);

    QPointF px;
    QLineF::IntersectType type = Intersects(edge, bigLine1, &px);

    if (type == QLineF::NoIntersection)
    {
        return AngleByLength(points, p1, p2, p3, bigLine1, sp2, bigLine2, p, width, needRollback);
    }

    if (IsOutsidePoint(bigLine1.p1(), bigLine1.p2(), px) && IsSameDirection(p3, p2, px))
    {
        if (QLineF(p2, px).length() > localWidth * maxL)
        {
            return AngleByLength(points, p1, p2, p3, bigLine1, sp2, bigLine2, p, width, needRollback);
        }
        points.append(VRawSAPoint(px, p.CurvePoint(), p.TurnPoint()));

        QLineF seam(px, p3);
        seam.setAngle(seam.angle() + 90);
        seam.setLength(p.GetSAAfter(width));
        points.append(VRawSAPoint(seam.p2(), p.CurvePoint(), p.TurnPoint()));

        if (needRollback != nullptr)
        {
            *needRollback = true;
        }
    }
    else
    {
        QLineF edge1(p2, p1);
        QLineF edge2(p2, p3);
        const qreal angle = edge1.angleTo(edge2);

        if (angle > 180)
        {
            return AngleByLength(points, p1, p2, p3, bigLine1, sp2, bigLine2, p, width, needRollback);
        }

        // Because artificial loop can lead to wrong clipping we must rollback current seam allowance points
        bool success = false;
        const auto countBefore = points.size();
        QVector<VRawSAPoint> temp = points;
        temp.append(VRawSAPoint(bigLine1.p2(), p.CurvePoint(), p.TurnPoint()));
        temp = VAbstractPiece::RollbackSeamAllowance(temp, edge, &success);

        if (success)
        {
            points = temp;
            px = points.constLast();
        }

        if (countBefore > 0)
        {
            QLineF seam(px, p3);
            seam.setAngle(seam.angle() + 90);
            seam.setLength(p.GetSAAfter(width));
            points.append(VRawSAPoint(seam.p2(), p.CurvePoint(), p.TurnPoint()));
        }
        else
        {
            if (needRollback != nullptr)
            {
                *needRollback = not success;
            }
            else if (IsSameDirection(bigLine1.p1(), bigLine1.p2(), px))
            {
                points.append(VRawSAPoint(px, p.CurvePoint(), p.TurnPoint()));
                QLineF seam(px, p3);
                seam.setAngle(seam.angle() + 90);
                seam.setLength(p.GetSAAfter(width));
                points.append(VRawSAPoint(seam.p2(), p.CurvePoint(), p.TurnPoint()));
            }
        }
    }

    return points;
}

//---------------------------------------------------------------------------------------------------------------------
auto SingleParallelPoint(const QPointF &p1, const QPointF &p2, qreal angle, qreal width) -> QPointF
{
    QLineF pLine(p1, p2);
    pLine.setAngle(pLine.angle() + angle);
    pLine.setLength(width);
    return pLine.p2();
}

//---------------------------------------------------------------------------------------------------------------------
auto SimpleParallelLine(const QPointF &p1, const QPointF &p2, qreal width) -> QLineF
{
    const QLineF paralel = QLineF(SingleParallelPoint(p1, p2, 90, width), SingleParallelPoint(p2, p1, -90, width));
    return paralel;
}

//---------------------------------------------------------------------------------------------------------------------
auto BisectorLine(const QPointF &p1, const QPointF &p2, const QPointF &p3) -> QLineF
{
    QLineF line1(p2, p1);
    QLineF line2(p2, p3);
    QLineF bLine;

    const qreal angle1 = line1.angleTo(line2);
    const qreal angle2 = line2.angleTo(line1);

    if (angle1 <= angle2)
    {
        bLine = line1;
        bLine.setAngle(bLine.angle() + angle1 / 2.0);
    }
    else
    {
        bLine = line2;
        bLine.setAngle(bLine.angle() + angle2 / 2.0);
    }

    return bLine;
}

//---------------------------------------------------------------------------------------------------------------------
auto AngleBetweenBisectors(const QLineF &b1, const QLineF &b2) -> qreal
{
    const QLineF newB2 = b2.translated(-(b2.p1().x() - b1.p1().x()), -(b2.p1().y() - b1.p1().y()));

    qreal angle1 = newB2.angleTo(b1);
    if (VFuzzyComparePossibleNulls(angle1, 360))
    {
        angle1 = 0;
    }

    qreal angle2 = b1.angleTo(newB2);
    if (VFuzzyComparePossibleNulls(angle2, 360))
    {
        angle2 = 0;
    }

    return qMin(angle1, angle2);
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> auto CorrectPathDistortion(QVector<T> path) -> QVector<T>
{
    if (path.size() < 3)
    {
        return path;
    }

    vsizetype prev = -1;
    for (qint32 i = 0; i < path.size(); ++i)
    {
        if (prev == -1)
        {
            i == 0 ? prev = path.size() - 1 : prev = i - 1;
        }

        int next = i + 1;
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
auto Rollback(QVector<VRawSAPoint> &points, const QLineF &edge) -> bool
{
    bool success = false;
    if (not points.isEmpty())
    {
        points.removeLast();
        points = VAbstractPiece::RollbackSeamAllowance(points, edge, &success);

        if (not points.isEmpty())
        {
            if (points.constLast().toPoint() != points.constFirst().toPoint())
            {
                points.append(points.constFirst()); // Should be always closed
            }
        }
    }
    return success;
}

//---------------------------------------------------------------------------------------------------------------------
void RollbackByLength(QVector<VRawSAPoint> &ekvPoints, const QVector<VSAPoint> &points, qreal width)
{
    const QLineF bigLine1 = VAbstractPiece::ParallelLine(points.at(points.size() - 2), points.at(0), width);

    QVector<VRawSAPoint> temp = ekvPoints;
    temp.insert(ekvPoints.size() - 1, VRawSAPoint(bigLine1.p2(), points.at(0).CurvePoint(), points.at(0).TurnPoint()));
    bool success = Rollback(temp, VAbstractPiece::ParallelLine(points.at(0), points.at(1), width));

    if (success)
    {
        ekvPoints = temp;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void RollbackBySecondEdgeSymmetry(QVector<VRawSAPoint> &ekvPoints, const QVector<VSAPoint> &points, qreal width)
{
    const QLineF axis = QLineF(points.at(points.size() - 1), points.at(1));
    const QLineF bigLine1 = VAbstractPiece::ParallelLine(points.at(points.size() - 2), points.at(0), width);
    QLineF sEdge(VPointF::FlipPF(axis, bigLine1.p1()), VPointF::FlipPF(axis, bigLine1.p2()));

    QVector<VRawSAPoint> temp = ekvPoints;
    temp.insert(ekvPoints.size() - 1, VRawSAPoint(bigLine1.p2(), points.at(0).CurvePoint(), points.at(0).TurnPoint()));
    bool success = Rollback(temp, sEdge);

    if (success)
    {
        ekvPoints = temp;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void RollbackByFirstEdgeSymmetry(QVector<VRawSAPoint> &ekvPoints, const QVector<VSAPoint> &points, qreal width)
{
    const QLineF axis = QLineF(points.at(points.size() - 2), points.at(points.size() - 1));
    const QLineF bigLine2 = VAbstractPiece::ParallelLine(points.at(points.size() - 1), points.at(1), width);
    QLineF sEdge(VPointF::FlipPF(axis, bigLine2.p1()), VPointF::FlipPF(axis, bigLine2.p2()));
    const QLineF bigLine1 = VAbstractPiece::ParallelLine(points.at(points.size() - 2), points.at(0), width);

    QVector<VRawSAPoint> temp = ekvPoints;
    temp.insert(ekvPoints.size() - 1, VRawSAPoint(bigLine1.p2(), points.at(0).CurvePoint(), points.at(0).TurnPoint()));
    bool success = Rollback(temp, sEdge);

    if (success)
    {
        ekvPoints = temp;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void RollbackByPointsIntersection(QVector<VRawSAPoint> &ekvPoints, const QVector<VSAPoint> &points, qreal width)
{
    const QLineF bigLine1 = VAbstractPiece::ParallelLine(points.at(points.size() - 2), points.at(0), width);
    QVector<VRawSAPoint> temp = ekvPoints;
    temp.insert(ekvPoints.size() - 1, VRawSAPoint(bigLine1.p2(), points.at(0).CurvePoint(), points.at(0).TurnPoint()));
    bool success = Rollback(temp, QLineF(points.constLast(), points.at(1)));

    if (success)
    {
        ekvPoints = temp;
    }

    if (ekvPoints.size() > 2)
    { // Fix for the rule of main path
        ekvPoints.removeAt(ekvPoints.size() - 1);
        ekvPoints.prepend(ekvPoints.at(ekvPoints.size() - 1));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void RollbackBySecondEdgeRightAngle(QVector<VRawSAPoint> &ekvPoints, const QVector<VSAPoint> &points, qreal width)
{
    if (not ekvPoints.isEmpty())
    {
        const QLineF edge(points.constLast(), points.at(1));
        const QLineF bigLine1 = VAbstractPiece::ParallelLine(points.at(points.size() - 2), points.at(0), width);

        QPointF px;
        Intersects(edge, bigLine1, &px);

        ekvPoints.removeLast();

        if (IsOutsidePoint(bigLine1.p1(), bigLine1.p2(), px))
        {
            if (ekvPoints.size() > 3)
            {
                const QLineF edge1(ekvPoints.at(ekvPoints.size() - 2), ekvPoints.constLast());
                const QLineF edge2(ekvPoints.at(0), ekvPoints.at(1));

                QPointF crosPoint;
                const QLineF::IntersectType type = Intersects(edge1, edge2, &crosPoint);

                if (type == QLineF::BoundedIntersection)
                {
                    ekvPoints.removeFirst();
                    ekvPoints.removeLast();

                    ekvPoints.append(VRawSAPoint(crosPoint, ekvPoints.at(0).CurvePoint(), ekvPoints.at(0).TurnPoint()));
                }
            }
        }
        else
        {
            bool success = false;
            QVector<VRawSAPoint> temp = ekvPoints;
            temp.append(VRawSAPoint(bigLine1.p2(), ekvPoints.at(0).CurvePoint(), ekvPoints.at(0).TurnPoint()));
            temp = VAbstractPiece::RollbackSeamAllowance(temp, edge, &success);

            if (success)
            {
                ekvPoints = temp;
                px = ekvPoints.constLast();
            }

            QLineF seam(px, points.at(1));
            seam.setAngle(seam.angle() + 90);
            seam.setLength(points.at(0).GetSAAfter(width));
            ekvPoints.append(VRawSAPoint(seam.p2(), ekvPoints.at(0).CurvePoint(), ekvPoints.at(0).TurnPoint()));

            if (not ekvPoints.isEmpty())
            {
                ekvPoints.append(ekvPoints.constFirst());
            }
        }

        if (not ekvPoints.isEmpty())
        {
            if (ekvPoints.constLast().toPoint() != ekvPoints.constFirst().toPoint())
            {
                ekvPoints.append(ekvPoints.constFirst()); // Should be always closed
            }
        }
    }
}
} // namespace

// Friend functions
//---------------------------------------------------------------------------------------------------------------------
auto operator<<(QDataStream &dataStream, const VAbstractPiece &piece) -> QDataStream &
{
    dataStream << *piece.d;
    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
auto operator>>(QDataStream &dataStream, VAbstractPiece &piece) -> QDataStream &
{
    dataStream >> *piece.d;
    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
VAbstractPiece::VAbstractPiece()
  : d(new VAbstractPieceData)
{
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL(VAbstractPiece)

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::operator=(const VAbstractPiece &piece) -> VAbstractPiece &
{
    if (&piece == this)
    {
        return *this;
    }
    d = piece.d;
    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VAbstractPiece::VAbstractPiece(VAbstractPiece &&piece) noexcept
  : d(std::move(piece.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::operator=(VAbstractPiece &&piece) noexcept -> VAbstractPiece &
{
    std::swap(d, piece.d);
    return *this;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
VAbstractPiece::~VAbstractPiece() = default;

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::GetName() const -> QString
{
    return d->m_name;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPiece::SetName(const QString &value)
{
    d->m_name = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::IsForbidFlipping() const -> bool
{
    return d->m_forbidFlipping;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPiece::SetForbidFlipping(bool value)
{
    d->m_forbidFlipping = value;

    if (value)
    {
        SetForceFlipping(not value);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::IsForceFlipping() const -> bool
{
    return d->m_forceFlipping;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPiece::SetForceFlipping(bool value)
{
    d->m_forceFlipping = value;

    if (value)
    {
        SetForbidFlipping(not value);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::IsFollowGrainline() const -> bool
{
    return d->m_followGrainline;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPiece::SetFollowGrainline(bool value)
{
    d->m_followGrainline = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::IsSeamAllowance() const -> bool
{
    return d->m_seamAllowance;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPiece::SetSeamAllowance(bool value)
{
    d->m_seamAllowance = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::IsSeamAllowanceBuiltIn() const -> bool
{
    return d->m_seamAllowanceBuiltIn;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPiece::SetSeamAllowanceBuiltIn(bool value)
{
    d->m_seamAllowanceBuiltIn = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::IsHideMainPath() const -> bool
{
    return d->m_hideMainPath;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPiece::SetHideMainPath(bool value)
{
    d->m_hideMainPath = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::IsSewLineOnDrawing() const -> bool
{
    return d->m_onDrawing;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPiece::SetSewLineOnDrawing(bool value)
{
    d->m_onDrawing = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::GetSAWidth() const -> qreal
{
    return d->m_width;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPiece::SetSAWidth(qreal value)
{
    value >= 0 ? d->m_width = value : d->m_width = 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::Equidistant(QVector<VSAPoint> points, qreal width, const QString &name) -> QVector<VLayoutPoint>
{
    if (width < 0)
    {
        qDebug() << "Width < 0.";
        return {};
    }
    width = qMax(width, VSAPoint::minSAWidth);

    //    DumpVector(points, QStringLiteral("input.json.XXXXXX")); // Uncomment for dumping test data

    // Fix distorsion. Must be done before the correction
    points = CorrectPathDistortion(points);

    points = CorrectEquidistantPoints(points);
    if (points.size() < 3)
    {
        const QString errorMsg =
            QCoreApplication::translate("VAbstractPiece", "Piece '%1'. Not enough points to build seam allowance.")
                .arg(name);
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        return {};
    }

    if (points.constLast().toPoint() != points.constFirst().toPoint())
    {
        points.append(points.at(0)); // Should be always closed
    }

    bool needRollback = false; // no need for rollback
    QVector<VRawSAPoint> ekvPoints;
    for (qint32 i = 0; i < points.size(); ++i)
    {
        if (i == 0)
        { // first point
            ekvPoints = EkvPoint(ekvPoints, points.at(points.size() - 2), points.at(points.size() - 1), points.at(1),
                                 points.at(0), width, &needRollback);
            continue;
        }

        if (i == points.size() - 1)
        { // last point
            if (not ekvPoints.isEmpty())
            {
                ekvPoints.append(ekvPoints.constFirst());
            }
            continue;
        }
        // points in the middle of polyline
        ekvPoints = EkvPoint(ekvPoints, points.at(i - 1), points.at(i), points.at(i + 1), points.at(i), width);
    }

    if (needRollback)
    {
        QT_WARNING_PUSH
        QT_WARNING_DISABLE_GCC("-Wswitch-default")
        // This check helps to find missed angle types in the switch
        Q_STATIC_ASSERT_X(static_cast<int>(PieceNodeAngle::LAST_ONE_DO_NOT_USE) == 7, "Not all types were handled.");
        switch (points.constLast().GetAngleType())
        {
            case PieceNodeAngle::LAST_ONE_DO_NOT_USE:
            case PieceNodeAngle::ByFirstEdgeRightAngle:
                Q_UNREACHABLE(); //-V501
                break;
            case PieceNodeAngle::ByLength:
            case PieceNodeAngle::ByLengthCurve:
                RollbackByLength(ekvPoints, points, width);
                break;
            case PieceNodeAngle::ByFirstEdgeSymmetry:
                RollbackByFirstEdgeSymmetry(ekvPoints, points, width);
                break;
            case PieceNodeAngle::BySecondEdgeSymmetry:
                RollbackBySecondEdgeSymmetry(ekvPoints, points, width);
                break;
            case PieceNodeAngle::ByPointsIntersection:
                RollbackByPointsIntersection(ekvPoints, points, width);
                break;
            case PieceNodeAngle::BySecondEdgeRightAngle:
                RollbackBySecondEdgeRightAngle(ekvPoints, points, width);
                break;
        }
        QT_WARNING_POP
    }

    QVector<VLayoutPoint> cleaned;
    //    Uncomment for debug
    //    CastTo(ekvPoints, cleaned);

    const bool removeFirstAndLast = false;
    ekvPoints = RemoveDublicates(ekvPoints, removeFirstAndLast);
    ekvPoints = CheckLoops(ekvPoints);
    CastTo(ekvPoints, cleaned); // Result path can contain loops
    cleaned = CorrectEquidistantPoints(cleaned, removeFirstAndLast);
    cleaned = CorrectPathDistortion(cleaned);

    //    QVector<QPointF> dump;
    //    CastTo(cleaned, dump);
    //    DumpVector(dump, QStringLiteral("output.json.XXXXXX")); // Uncomment for dumping test data
    return cleaned;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::SumTrapezoids(const QVector<QPointF> &points) -> qreal
{
    // Calculation a polygon area through the sum of the areas of trapezoids
    qreal s, res = 0;
    const auto n = points.size();

    if (n > 2)
    {
        for (int i = 0; i < n; ++i)
        {
            if (i == 0)
            {
                // if i == 0, then y[i-1] replace on y[n-1]
                s = points.at(i).x() * (points.at(n - 1).y() - points.at(i + 1).y());
                res += s;
            }
            else
            {
                if (i == n - 1)
                {
                    // if i == n-1, then y[i+1] replace on y[0]
                    s = points.at(i).x() * (points.at(i - 1).y() - points.at(0).y());
                    res += s;
                }
                else
                {
                    s = points.at(i).x() * (points.at(i - 1).y() - points.at(i + 1).y());
                    res += s;
                }
            }
        }
    }
    return res;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief EkvPoint return seam aloowance points in place of intersection two edges. Last points of two edges should be
 * equal.
 * @param width global seam allowance width.
 * @return seam aloowance points.
 */
auto VAbstractPiece::EkvPoint(QVector<VRawSAPoint> points, const VSAPoint &p1Line1, const VSAPoint &p2Line1,
                              const VSAPoint &p1Line2, const VSAPoint &p2Line2, qreal width, bool *needRollback)
    -> QVector<VRawSAPoint>
{
    if (width < 0)
    { // width can't be < 0
        return {};
    }

    width = qMax(width, VSAPoint::minSAWidth);

    if (p2Line1 != p2Line2)
    {
        qDebug() << "Last points of two lines must be equal.";
        return {}; // Wrong edges
    }

    const QLineF bigLine1 = ParallelLine(p1Line1, p2Line1, width);
    const QLineF bigLine2 = ParallelLine(p2Line2, p1Line2, width);

    if (VFuzzyComparePoints(bigLine1.p2(), bigLine2.p1()))
    {
        points.append(VRawSAPoint(bigLine1.p2(), p2Line1.CurvePoint(), p2Line1.TurnPoint()));
        return points;
    }

    QLineF edge1(p2Line2, p1Line2);
    QLineF edge2(p2Line1, p1Line1);
    qreal a = edge2.angleTo(edge1);

    if (a >= 175 && a <= 185 && not VFuzzyComparePossibleNulls(p2Line1.GetSABefore(width), p2Line1.GetSAAfter(width)))
    {
        QLineF ray = edge2;
        ray.setAngle(ray.angle() - a / 2);
        ray.setLength(width * 2);

        QPointF crosPoint;
        QLineF::IntersectType type = Intersects(ray, bigLine1, &crosPoint);
        if (type != QLineF::NoIntersection)
        {
            points.append(VRawSAPoint(crosPoint, p2Line1.CurvePoint(), p2Line1.TurnPoint()));
        }

        type = Intersects(ray, bigLine2, &crosPoint);
        if (type != QLineF::NoIntersection)
        {
            points.append(VRawSAPoint(crosPoint, p2Line1.CurvePoint(), p2Line1.TurnPoint()));
        }
        return points;
    }

    QPointF crosPoint;
    const QLineF::IntersectType type = Intersects(bigLine1, bigLine2, &crosPoint);

    switch (type)
    { // There are at least three big cases
        case (QLineF::BoundedIntersection):
            // The easiest, real intersection
            points.append(VRawSAPoint(crosPoint, p2Line1.CurvePoint(), p2Line1.TurnPoint()));
            return points;
        case (QLineF::UnboundedIntersection):
        { // Most common case
            /* Case when a path has point on line (both segments lie on the same line) and seam allowance creates
             * prong. */
            auto IsOnLine = [](const QPointF &base, const QPointF &sp1, const QPointF &sp2, qreal accuracy)
            {
                if (not VFuzzyComparePoints(base, sp1))
                {
                    return VGObject::IsPointOnLineviaPDP(sp2, base, sp1, accuracy);
                }

                if (not VFuzzyComparePoints(base, sp2))
                {
                    return VGObject::IsPointOnLineviaPDP(sp1, base, sp2, accuracy);
                }
                return true;
            };
            if (VGObject::IsPointOnLineSegment(p2Line1, p1Line1, p1Line2, ToPixel(0.5, Unit::Mm)) &&
                IsOnLine(p2Line1, bigLine1.p2(), bigLine2.p1(), ToPixel(0.5, Unit::Mm)) &&
                p2Line1.GetAngleType() == PieceNodeAngle::ByLength)
            {
                points.append(VRawSAPoint(bigLine1.p2(), p2Line1.CurvePoint(), p2Line1.TurnPoint()));
                points.append(VRawSAPoint(bigLine2.p1(), p2Line1.CurvePoint(), p2Line1.TurnPoint()));
                return points;
            }

            const qreal localWidth = p2Line1.MaxLocalSA(width);
            QLineF line(p2Line1, crosPoint);

            // Checking two subcases
            const QLineF b1 = BisectorLine(p1Line1, p2Line1, p1Line2);
            const QLineF b2 = BisectorLine(bigLine1.p1(), crosPoint, bigLine2.p2());

            const qreal angle = AngleBetweenBisectors(b1, b2);

            // Comparison bisector angles helps to find direction
            if (angle < 135 || VFuzzyComparePossibleNulls(angle, 135.0)) // Go in a same direction
            {                                                            // Regular equdistant case
                QT_WARNING_PUSH
                QT_WARNING_DISABLE_GCC("-Wswitch-default")
                // This check helps to find missed angle types in the switch
                Q_STATIC_ASSERT_X(static_cast<int>(PieceNodeAngle::LAST_ONE_DO_NOT_USE) == 7,
                                  "Not all types were handled.");
                switch (p2Line1.GetAngleType())
                {
                    case PieceNodeAngle::LAST_ONE_DO_NOT_USE:
                        Q_UNREACHABLE(); //-V501
                        break;
                    case PieceNodeAngle::ByLength:
                    case PieceNodeAngle::ByLengthCurve:
                        return AngleByLength(points, p1Line1, p2Line1, p1Line2, bigLine1, crosPoint, bigLine2, p2Line1,
                                             width, needRollback);
                    case PieceNodeAngle::ByPointsIntersection:
                        return AngleByIntersection(points, p1Line1, p2Line1, p1Line2, bigLine1, crosPoint, bigLine2,
                                                   p2Line1, width, needRollback);
                    case PieceNodeAngle::ByFirstEdgeSymmetry:
                        return AngleByFirstSymmetry(points, p1Line1, p2Line1, p1Line2, bigLine1, crosPoint, bigLine2,
                                                    p2Line1, width, needRollback);
                    case PieceNodeAngle::BySecondEdgeSymmetry:
                        return AngleBySecondSymmetry(points, p1Line1, p2Line1, p1Line2, bigLine1, crosPoint, bigLine2,
                                                     p2Line1, width, needRollback);
                    case PieceNodeAngle::ByFirstEdgeRightAngle:
                        return AngleByFirstRightAngle(points, p1Line1, p2Line1, p1Line2, bigLine1, crosPoint, bigLine2,
                                                      p2Line1, width, needRollback);
                    case PieceNodeAngle::BySecondEdgeRightAngle:
                        return AngleBySecondRightAngle(points, p1Line1, p2Line1, p1Line2, bigLine1, crosPoint, bigLine2,
                                                       p2Line1, width, needRollback);
                }
                QT_WARNING_POP
            }
            else
            { // Different directions
                QLineF bisector(p2Line1, p1Line1);
                bisector.setAngle(b1.angle());

                const qreal result1 = PointPosition(bisector.p2(), QLineF(p1Line1, p2Line1));
                const qreal result2 = PointPosition(bisector.p2(), QLineF(p2Line2, p1Line2));

                if ((result1 < 0 || qFuzzyIsNull(result1)) && (result2 < 0 || qFuzzyIsNull(result2)))
                { // Dart case. A bisector watches outside.
                    QLineF edge1(p1Line1, p2Line1);
                    QLineF edge2(p1Line2, p2Line2);

                    if (qAbs(edge1.length() - edge2.length()) <= qMax(edge1.length(), edge2.length()) * 0.2)
                    {
                        // Classic dart must be symmetrical.
                        // In some cases a point still valid, but ignore if going outside of an equdistant.

                        const QLineF bigEdge = ParallelLine(p1Line1, p1Line2, localWidth);
                        QPointF px;
                        const QLineF::IntersectType type = Intersects(bigEdge, line, &px);
                        if (type != QLineF::BoundedIntersection && line.length() < QLineF(p2Line1, px).length())
                        {
                            points.append(VRawSAPoint(crosPoint, p2Line1.CurvePoint(), p2Line1.TurnPoint()));
                            return points;
                        }
                    }
                    else
                    { // Just an acute angle with big seam allowance
                        if (IsSameDirection(bigLine2.p1(), bigLine2.p2(), crosPoint))
                        {
                            QLineF loop(crosPoint, bigLine1.p1());
                            loop.setAngle(loop.angle() + 180);
                            loop.setLength(accuracyPointOnLine * 2.);
                            points.append(VRawSAPoint(loop.p2(), p2Line1.CurvePoint(), p2Line1.TurnPoint()));
                            points.append(VRawSAPoint(crosPoint, p2Line1.CurvePoint(), p2Line1.TurnPoint()));

                            loop = QLineF(crosPoint, bigLine1.p1());
                            loop.setLength(loop.length() + localWidth * 2.);
                            points.append(VRawSAPoint(loop.p2(), p2Line1.CurvePoint(), p2Line1.TurnPoint(), true));
                        }

                        return points;
                    }
                }
                else
                { // New subcase. This is not a dart. An angle is acute and bisector watch inside.
                    const qreal result1 = PointPosition(crosPoint, QLineF(p1Line1, p2Line1));
                    const qreal result2 = PointPosition(crosPoint, QLineF(p2Line2, p1Line2));

                    if ((result1 < 0 || qFuzzyIsNull(result1)) && (result2 < 0 || qFuzzyIsNull(result2)))
                    { // The cross point is still outside of a piece
                        if (line.length() >= localWidth)
                        {
                            points.append(VRawSAPoint(crosPoint, p2Line1.CurvePoint(), p2Line1.TurnPoint()));
                            return points;
                        }

                        // but not enough far, fix it
                        line.setLength(localWidth);
                        points.append(VRawSAPoint(line.p2(), p2Line1.CurvePoint(), p2Line1.TurnPoint()));
                        return points;
                    }

                    // Wrong cross point, probably inside of a piece. Manually creating correct seam allowance
                    const QLineF bigEdge = SimpleParallelLine(bigLine1.p2(), bigLine2.p1(), localWidth);
                    points.append(VRawSAPoint(bigEdge.p1(), p2Line1.CurvePoint(), p2Line1.TurnPoint()));
                    points.append(VRawSAPoint(bigEdge.p2(), p2Line1.CurvePoint(), p2Line1.TurnPoint()));
                    return points;
                }
            }
            break;
        }
        case (QLineF::NoIntersection):
            /*If we have correct lines this means lines lie on a line or parallel.*/
            points.append(VRawSAPoint(bigLine1.p2(), p2Line1.CurvePoint(), p2Line1.TurnPoint()));
            // Second point for parallel line
            points.append(VRawSAPoint(bigLine2.p1(), p2Line1.CurvePoint(), p2Line1.TurnPoint()));
            return points;
        default:
            break;
    }
    return points;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::ParallelLine(const VSAPoint &p1, const VSAPoint &p2, qreal width) -> QLineF
{
    return {SingleParallelPoint(p1, p2, 90, p1.GetSAAfter(width)),
            SingleParallelPoint(p2, p1, -90, p2.GetSABefore(width))};
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::IsAllowanceValid(const QVector<QPointF> &base, const QVector<QPointF> &allowance) -> bool
{
    if (base.size() < 3 || allowance.size() < 3)
    {
        return false; // Not enough data
    }

    //    DumpVector(base, QStringLiteral("base.json.XXXXXX")); // Uncomment for dumping test data
    //    DumpVector(allowance, QStringLiteral("allowance.json.XXXXXX")); // Uncomment for dumping test data

    // First check direction
    const qreal baseDirection = VPiece::SumTrapezoids(base);
    const qreal allowanceDirection = VPiece::SumTrapezoids(allowance);

    if (baseDirection >= 0 || allowanceDirection >= 0)
    {
        return false; // Wrong direction
    }

    return IsInsidePolygon(base, allowance);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::IsEkvPointOnLine(const QPointF &iPoint, const QPointF &prevPoint, const QPointF &nextPoint) -> bool
{
    return VGObject::IsPointOnLineviaPDP(iPoint, prevPoint, nextPoint, accuracyPointOnLine / 4.);
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::IsEkvPointOnLine(const VSAPoint &iPoint, const VSAPoint &prevPoint, const VSAPoint &nextPoint)
    -> bool
{
    // See bug #671
    const qreal tmpWidth = 10;
    const QLineF bigLine1 = ParallelLine(prevPoint, iPoint, tmpWidth);
    const QLineF bigLine2 = ParallelLine(iPoint, nextPoint, tmpWidth);

    bool seamOnLine = VGObject::IsPointOnLineviaPDP(iPoint, prevPoint, nextPoint);
    bool sa1OnLine = VGObject::IsPointOnLineviaPDP(bigLine1.p2(), bigLine1.p1(), bigLine2.p2());
    bool sa2OnLine = VGObject::IsPointOnLineviaPDP(bigLine2.p1(), bigLine1.p1(), bigLine2.p2());
    bool saDiff = qAbs(prevPoint.GetSAAfter(tmpWidth) - nextPoint.GetSABefore(tmpWidth)) < accuracyPointOnLine;

    // left point that splits a curve
    bool curve = (prevPoint.GetAngleType() == PieceNodeAngle::ByLengthCurve &&
                  iPoint.GetAngleType() == PieceNodeAngle::ByLengthCurve) ||
                 (nextPoint.GetAngleType() == PieceNodeAngle::ByLengthCurve &&
                  iPoint.GetAngleType() == PieceNodeAngle::ByLengthCurve);

    return seamOnLine && sa1OnLine && sa2OnLine && saDiff && not curve;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::GetMx() const -> qreal
{
    return d->m_mx;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPiece::SetMx(qreal value)
{
    d->m_mx = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::GetMy() const -> qreal
{
    return d->m_my;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPiece::SetMy(qreal value)
{
    d->m_my = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::GetPriority() const -> uint
{
    return d->m_priority;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPiece::SetPriority(uint value)
{
    d->m_priority = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::GetUUID() const -> QUuid
{
    return d->m_uuid;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPiece::SetUUID(const QUuid &uuid)
{
    d->m_uuid = uuid;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractPiece::SetUUID(const QString &uuid)
{
    const QUuid temp = QUuid(uuid);
    d->m_uuid = temp.isNull() ? QUuid::createUuid() : temp;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::GetUniqueID() const -> QString
{
    return d->m_uuid.toString();
}

//---------------------------------------------------------------------------------------------------------------------
auto VSAPoint::toJson() const -> QJsonObject
{
    QJsonObject pointObject = VLayoutPoint::toJson();
    pointObject["type"_L1] = "VSAPoint";

    if (not VFuzzyComparePossibleNulls(m_before, -1))
    {
        pointObject["saBefore"_L1] = m_before;
    }

    if (not VFuzzyComparePossibleNulls(m_after, -1))
    {
        pointObject["saAfter"_L1] = m_after;
    }

    if (m_angle != PieceNodeAngle::ByLength)
    {
        pointObject["angle"_L1] = static_cast<int>(m_angle);
    }

    if (m_manualPassmarkLength)
    {
        pointObject["manualPassmarkLength"_L1] = m_manualPassmarkLength;
        pointObject["passmarkLength"_L1] = m_passmarkLength;
    }

    if (m_manualPassmarkWidth)
    {
        pointObject["manualPassmarkWidth"_L1] = m_manualPassmarkWidth;
        pointObject["passmarkWidth"_L1] = m_passmarkWidth;
    }
    else
    {
        pointObject["passmarkClockwiseOpening"_L1] = m_passmarkClockwiseOpening;
    }

    if (m_manualPassmarkAngle)
    {
        pointObject["manualPassmarkAngle"_L1] = m_manualPassmarkAngle;
        pointObject["passmarkAngle"_L1] = m_passmarkAngle;
    }

    return pointObject;
}

//---------------------------------------------------------------------------------------------------------------------
// Because artificial loop can lead to wrong clipping we must rollback current seam allowance points
auto VAbstractPiece::RollbackSeamAllowance(QVector<VRawSAPoint> points, const QLineF &cuttingEdge, bool *success)
    -> QVector<VRawSAPoint>
{
    *success = false;
    QVector<VRawSAPoint> clipped;
    clipped.reserve(points.count() + 1);
    for (auto i = points.count() - 1; i > 0; --i)
    {
        QLineF segment(points.at(i), points.at(i - 1));
        QPointF crosPoint;
        const QLineF::IntersectType type = Intersects(cuttingEdge, segment, &crosPoint);

        if (type != QLineF::NoIntersection && VGObject::IsPointOnLineSegment(crosPoint, segment.p1(), segment.p2()) &&
            IsSameDirection(cuttingEdge.p2(), cuttingEdge.p1(), crosPoint))
        {
            clipped.append(VRawSAPoint(crosPoint, points.at(i).CurvePoint(), points.at(i).TurnPoint()));
            for (auto j = i - 1; j >= 0; --j)
            {
                clipped.append(points.at(j));
            }
            points = Reverse(clipped);
            *success = true;
            break;
        }
    }

    if (not *success && points.size() > 1)
    {
        QPointF crosPoint;
        QLineF secondLast(points.at(points.size() - 2), points.at(points.size() - 1));
        QLineF::IntersectType type = Intersects(secondLast, cuttingEdge, &crosPoint);

        if (type != QLineF::NoIntersection && IsOutsidePoint(secondLast.p1(), secondLast.p2(), crosPoint))
        {
            points.append(VRawSAPoint(crosPoint, points.at(points.size() - 1).CurvePoint(),
                                      points.at(points.size() - 1).TurnPoint()));
            *success = true;
        }
    }

    return points;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::IsItemContained(const QRectF &parentBoundingRect, const QVector<QPointF> &shape, qreal &dX,
                                     qreal &dY) -> bool
{
    dX = 0;
    dY = 0;
    // single point differences
    bool bInside = true;

    for (auto p : shape)
    {
        qreal dPtX = 0;
        qreal dPtY = 0;
        if (not parentBoundingRect.contains(p))
        {
            if (p.x() < parentBoundingRect.left())
            {
                dPtX = parentBoundingRect.left() - p.x();
            }
            else if (p.x() > parentBoundingRect.right())
            {
                dPtX = parentBoundingRect.right() - p.x();
            }

            if (p.y() < parentBoundingRect.top())
            {
                dPtY = parentBoundingRect.top() - p.y();
            }
            else if (p.y() > parentBoundingRect.bottom())
            {
                dPtY = parentBoundingRect.bottom() - p.y();
            }

            if (qAbs(dPtX) > qAbs(dX))
            {
                dX = dPtX;
            }

            if (qAbs(dPtY) > qAbs(dY))
            {
                dY = dPtY;
            }

            bInside = false;
        }
    }
    return bInside;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::CorrectPosition(const QRectF &parentBoundingRect, QVector<QPointF> points) -> QVector<QPointF>
{
    qreal dX = 0;
    qreal dY = 0;
    if (not IsItemContained(parentBoundingRect, points, dX, dY))
    {
        for (auto &point : points)
        {
            point = QPointF(point.x() + dX, point.y() + dY);
        }
    }
    return points;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::FindGrainlineGeometry(const VGrainlineData &geom, const VContainer *pattern, qreal &length,
                                           qreal &rotationAngle, QPointF &pos) -> bool
{
    SCASSERT(pattern != nullptr)

    const quint32 topPin = geom.TopPin();
    const quint32 bottomPin = geom.BottomPin();

    if (topPin != NULL_ID && bottomPin != NULL_ID)
    {
        try
        {
            const auto topPinPoint = pattern->GeometricObject<VPointF>(topPin);
            const auto bottomPinPoint = pattern->GeometricObject<VPointF>(bottomPin);

            QLineF grainline(static_cast<QPointF>(*bottomPinPoint), static_cast<QPointF>(*topPinPoint));
            length = grainline.length();
            rotationAngle = grainline.angle();

            if (not VFuzzyComparePossibleNulls(rotationAngle, 0))
            {
                grainline.setAngle(0);
            }

            pos = grainline.p1();
            rotationAngle = qDegreesToRadians(rotationAngle);

            return true;
        }
        catch (const VExceptionBadId &)
        {
            // do nothing.
        }
    }

    try
    {
        Calculator cal1;
        rotationAngle = cal1.EvalFormula(pattern->DataVariables(), geom.GetRotation());
        rotationAngle = qDegreesToRadians(rotationAngle);

        Calculator cal2;
        length = cal2.EvalFormula(pattern->DataVariables(), geom.GetLength());
        length = ToPixel(length, *pattern->GetPatternUnit());
    }
    catch (qmu::QmuParserError &e)
    {
        Q_UNUSED(e);
        return false;
    }

    const quint32 centerPin = geom.CenterPin();
    if (centerPin != NULL_ID)
    {
        try
        {
            const auto centerPinPoint = pattern->GeometricObject<VPointF>(centerPin);

            QLineF grainline(centerPinPoint->x(), centerPinPoint->y(), centerPinPoint->x() + length / 2.0,
                             centerPinPoint->y());

            grainline.setAngle(qRadiansToDegrees(rotationAngle));
            grainline = QLineF(grainline.p2(), grainline.p1());
            grainline.setLength(length);

            pos = grainline.p2();
        }
        catch (const VExceptionBadId &)
        {
            pos = geom.GetPos();
        }
    }
    else
    {
        pos = geom.GetPos();
    }
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::GrainlineMainLine(const VGrainlineData &geom, const VContainer *pattern,
                                       const QRectF &boundingRect) -> QLineF
{
    SCASSERT(pattern != nullptr)

    QPointF pt1;
    qreal dLen = 0;
    qreal dAng = 0;
    if (not FindGrainlineGeometry(geom, pattern, dLen, dAng, pt1))
    {
        return {};
    }

    QPointF pt2(pt1.x() + dLen * qCos(dAng), pt1.y() - dLen * qSin(dAng));

    VPieceGrainline grainline(QLineF(pt1, pt2), geom.GetArrowType());

    QVector<QPointF> v;
    if (grainline.IsFourWays())
    {
        QLineF mainLine = grainline.GetMainLine();
        QLineF secondaryLine = grainline.SecondaryLine();
        v = {mainLine.p1(), mainLine.p2(), secondaryLine.p1(), secondaryLine.p2()};
    }
    else
    {
        QLineF mainLine = grainline.GetMainLine();
        v = {mainLine.p1(), mainLine.p2()};
    }

    qreal dX = 0;
    qreal dY = 0;
    if (not IsItemContained(boundingRect, v, dX, dY))
    {
        pt1.rx() = +dX;
        pt1.ry() = +dY;

        pt2.rx() = +dX;
        pt2.ry() = +dY;
    }

    return {pt1, pt2};
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::PlaceLabelShape(const VLayoutPlaceLabel &label) -> PlaceLabelImg
{
    auto LayoutPoint = [label](QPointF p, bool turnPoint = false, bool curvePoint = false)
    {
        VLayoutPoint point(label.RotationMatrix().map(p));
        point.SetTurnPoint(turnPoint);
        point.SetCurvePoint(curvePoint);
        return point;
    };

    const QPointF pos = label.Center();
    const QRectF box = label.Box();

    auto SegmentShape = [pos, box, LayoutPoint]()
    {
        QVector<VLayoutPoint> shape{LayoutPoint(QPointF(pos.x(), pos.y() - box.height() / 2.0), true),
                                    LayoutPoint(QPointF(pos.x(), pos.y() + box.height() / 2.0), true)};

        return PlaceLabelImg{shape};
    };

    auto RectangleShape = [pos, box, LayoutPoint]()
    {
        QRectF rect(QPointF(pos.x() - box.width() / 2.0, pos.y() - box.height() / 2.0),
                    QPointF(pos.x() + box.width() / 2.0, pos.y() + box.height() / 2.0));

        QVector<VLayoutPoint> shape{LayoutPoint(rect.topLeft(), true), LayoutPoint(rect.topRight(), true),
                                    LayoutPoint(rect.bottomRight(), true), LayoutPoint(rect.bottomLeft(), true),
                                    LayoutPoint(rect.topLeft(), true)};

        return PlaceLabelImg{shape};
    };

    auto CrossShape = [pos, box, LayoutPoint]()
    {
        QVector<VLayoutPoint> shape1{LayoutPoint(QPointF(pos.x(), pos.y() - box.height() / 2.0), true),
                                     LayoutPoint(QPointF(pos.x(), pos.y() + box.height() / 2.0), true)};

        QVector<VLayoutPoint> shape2{LayoutPoint(QPointF(pos.x() - box.width() / 2.0, pos.y()), true),
                                     LayoutPoint(QPointF(pos.x() + box.width() / 2.0, pos.y()), true)};

        return PlaceLabelImg{shape1, shape2};
    };

    auto TshapedShape = [pos, box, LayoutPoint]()
    {
        QPointF center2(pos.x(), pos.y() + box.height() / 2.0);

        QVector<VLayoutPoint> shape1{LayoutPoint(QPointF(pos.x(), pos.y()), true), LayoutPoint(center2, true)};

        QVector<VLayoutPoint> shape2{LayoutPoint(QPointF(center2.x() - box.width() / 2.0, center2.y()), true),
                                     LayoutPoint(QPointF(center2.x() + box.width() / 2.0, center2.y()), true)};

        return PlaceLabelImg{shape1, shape2};
    };

    auto DoubletreeShape = [pos, box, LayoutPoint]()
    {
        QRectF rect(QPointF(pos.x() - box.width() / 2.0, pos.y() - box.height() / 2.0),
                    QPointF(pos.x() + box.width() / 2.0, pos.y() + box.height() / 2.0));

        QVector<VLayoutPoint> shape1{LayoutPoint(rect.topLeft(), true), LayoutPoint(rect.bottomRight(), true)};

        QVector<VLayoutPoint> shape2{LayoutPoint(rect.topRight(), true), LayoutPoint(rect.bottomLeft(), true)};

        return PlaceLabelImg{shape1, shape2};
    };

    auto CornerShape = [pos, box, LayoutPoint]()
    {
        QVector<VLayoutPoint> shape1{LayoutPoint(QPointF(pos.x(), pos.y()), true),
                                     LayoutPoint(QPointF(pos.x(), pos.y() + box.height() / 2.0), true)};

        QVector<VLayoutPoint> shape2{LayoutPoint(QPointF(pos.x() - box.width() / 2.0, pos.y()), true),
                                     LayoutPoint(QPointF(pos.x(), pos.y()), true)};

        return PlaceLabelImg{shape1, shape2};
    };

    auto TriangleShape = [pos, box, LayoutPoint]()
    {
        QRectF rect(QPointF(pos.x() - box.width() / 2.0, pos.y() - box.height() / 2.0),
                    QPointF(pos.x() + box.width() / 2.0, pos.y() + box.height() / 2.0));

        QVector<VLayoutPoint> shape{LayoutPoint(rect.topLeft(), true), LayoutPoint(rect.topRight(), true),
                                    LayoutPoint(rect.bottomRight(), true), LayoutPoint(rect.topLeft(), true)};

        return PlaceLabelImg{shape};
    };

    auto HshapedShape = [pos, box, LayoutPoint]()
    {
        const QPointF center1(pos.x(), pos.y() - box.height() / 2.0);
        const QPointF center2(pos.x(), pos.y() + box.height() / 2.0);

        QVector<VLayoutPoint> shape1{LayoutPoint(center1, true), LayoutPoint(center2, true)};

        QVector<VLayoutPoint> shape2{LayoutPoint(QPointF(center1.x() - box.width() / 2.0, center1.y()), true),
                                     LayoutPoint(QPointF(center1.x() + box.width() / 2.0, center1.y()), true)};

        QVector<VLayoutPoint> shape3{LayoutPoint(QPointF(center2.x() - box.width() / 2.0, center2.y()), true),
                                     LayoutPoint(QPointF(center2.x() + box.width() / 2.0, center2.y()), true)};

        return PlaceLabelImg{shape1, shape2, shape3};
    };

    auto ButtonShape = [pos, box, LayoutPoint]()
    {
        const qreal radius = qMin(box.width() / 2.0, box.height() / 2.0);
        QVector<VLayoutPoint> shape1{LayoutPoint(QPointF(pos.x(), pos.y() - radius), true),
                                     LayoutPoint(QPointF(pos.x(), pos.y() + radius), true)};

        QVector<VLayoutPoint> shape2{LayoutPoint(QPointF(pos.x() - radius, pos.y()), true),
                                     LayoutPoint(QPointF(pos.x() + radius, pos.y()), true)};

        const qreal circleSize = 0.85;
        VArc arc(VPointF(pos), radius * circleSize, 0, 360);
        arc.SetApproximationScale(10);

        QVector<QPointF> points = arc.GetPoints();
        if (not points.isEmpty() && points.constFirst() != points.constLast())
        {
            points.append(points.constFirst());
        }

        QVector<VLayoutPoint> shape3;
        for (int i = 0; i < points.size(); ++i)
        {
            bool turnPoint = false;
            if (i == 0 || i == points.size() - 1)
            {
                turnPoint = true;
            }
            shape3.append(LayoutPoint(points.at(i), turnPoint, true));
        }

        return PlaceLabelImg{shape1, shape2, shape3};
    };

    auto CircleShape = [pos, box, LayoutPoint]()
    {
        const qreal radius = qMin(box.width() / 2.0, box.height() / 2.0);
        VArc arc(VPointF(pos), radius, 0, 360);
        arc.SetApproximationScale(10);

        QVector<QPointF> points = arc.GetPoints();
        if (not points.isEmpty() && points.constFirst() != points.constLast())
        {
            points.append(points.constFirst());
        }

        QVector<VLayoutPoint> circle;
        for (int i = 0; i < points.size(); ++i)
        {
            bool turnPoint = false;
            if (i == 0 || i == points.size() - 1)
            {
                turnPoint = true;
            }
            circle.append(LayoutPoint(points.at(i), turnPoint, true));
        }

        return PlaceLabelImg{circle};
    };

    switch (label.Type())
    {
        case PlaceLabelType::Segment:
            return SegmentShape();
        case PlaceLabelType::Rectangle:
            return RectangleShape();
        case PlaceLabelType::Cross:
            return CrossShape();
        case PlaceLabelType::Tshaped:
            return TshapedShape();
        case PlaceLabelType::Doubletree:
            return DoubletreeShape();
        case PlaceLabelType::Corner:
            return CornerShape();
        case PlaceLabelType::Triangle:
            return TriangleShape();
        case PlaceLabelType::Hshaped:
            return HshapedShape();
        case PlaceLabelType::Button:
            return ButtonShape();
        case PlaceLabelType::Circle:
            return CircleShape();
        default:
            return {};
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::LabelShapePath(const VLayoutPlaceLabel &label) -> QPainterPath
{
    return LabelShapePath(PlaceLabelShape(label));
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractPiece::LabelShapePath(const PlaceLabelImg &shape) -> QPainterPath
{
    QPainterPath path;
    for (const auto &p : shape)
    {
        if (not p.isEmpty())
        {
            path.moveTo(p.constFirst());
            QVector<QPointF> polygon;
            CastTo(p, polygon);
            path.addPolygon(polygon);
        }
    }
    return path;
}
