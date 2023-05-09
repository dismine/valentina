/************************************************************************
 **
 **  @file   vgrainline.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 4, 2023
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2023 Valentina project
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
#include "vpiecegrainline.h"
#include "../vgeometry/vabstractcurve.h"
#include "qmath.h"
#include "vpiecegrainline_p.h"
#include <algorithm>

#include <QTransform>

namespace
{
constexpr qreal arrowAngle = M_PI/9;
constexpr int arrowLength = 15;
}

// VPieceGrainlinePrivate
//---------------------------------------------------------------------------------------------------------------------
auto VPieceGrainlinePrivate::MainLine(const QPointF &p1, qreal length, qreal angle) -> QLineF
{
    QPointF pt2(p1.x() + length * cos(angle), p1.y() - length * sin(angle));
    return {p1, pt2};
}

// VPieceGrainline
//---------------------------------------------------------------------------------------------------------------------
VPieceGrainline::VPieceGrainline()
    :d(new VPieceGrainlinePrivate)
{}

//---------------------------------------------------------------------------------------------------------------------
VPieceGrainline::~VPieceGrainline() //NOLINT(modernize-use-equals-default)
{}

//---------------------------------------------------------------------------------------------------------------------
VPieceGrainline::VPieceGrainline(const QLineF &mainLine, GrainlineArrowDirection arrowType)
    :d (new VPieceGrainlinePrivate(mainLine, arrowType))
{}

//---------------------------------------------------------------------------------------------------------------------
VPieceGrainline::VPieceGrainline(const QPointF &p1, qreal length, qreal angle, GrainlineArrowDirection arrowType)
    :d (new VPieceGrainlinePrivate(VPieceGrainlinePrivate::MainLine(p1, length, angle), arrowType))
{}

//---------------------------------------------------------------------------------------------------------------------
VPieceGrainline::VPieceGrainline(const VPieceGrainline &other) //NOLINT(modernize-use-equals-default)
    :d (other.d)
{}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceGrainline::operator=(const VPieceGrainline &grainline) -> VPieceGrainline &
{
    if ( &grainline == this )
    {
        return *this;
    }
    d = grainline.d;
    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VPieceGrainline::VPieceGrainline(VPieceGrainline &&grainline) Q_DECL_NOTHROW
    : d(std::move(grainline.d))
{}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceGrainline::operator=(VPieceGrainline &&grainline) Q_DECL_NOTHROW -> VPieceGrainline &
{
    std::swap(d, grainline.d);
    return *this;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
auto VPieceGrainline::GetMainLine() const -> QLineF
{
    return d->m_mainLine;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceGrainline::SetMainLine(const QLineF &mainLine)
{
    d->m_mainLine = mainLine;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceGrainline::GetArrowType() const -> GrainlineArrowDirection
{
    return d->m_arrowType;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceGrainline::SetArrowType(GrainlineArrowDirection arrowType)
{
    d->m_arrowType = arrowType;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceGrainline::IsEnabled() const -> bool
{
    return d->m_enabled;
}

//---------------------------------------------------------------------------------------------------------------------
void VPieceGrainline::SetEnabled(bool enabled)
{
    d->m_enabled = enabled;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceGrainline::SecondaryLine() const -> QLineF
{
    QLineF mainLine = GetMainLine();
    QLineF secondaryLine = mainLine;
    QTransform t;
    t.translate(mainLine.center().x(), mainLine.center().y());
    t.rotate(90);
    t.translate(-mainLine.center().x(), -mainLine.center().y());
    secondaryLine = t.map(secondaryLine);
    return secondaryLine;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceGrainline::IsFourWays() const -> bool
{
    return d->m_arrowType == GrainlineArrowDirection::fourWays ||
           d->m_arrowType == GrainlineArrowDirection::twoWaysUpLeft ||
           d->m_arrowType == GrainlineArrowDirection::twoWaysUpRight ||
           d->m_arrowType == GrainlineArrowDirection::twoWaysDownLeft ||
           d->m_arrowType == GrainlineArrowDirection::twoWaysDownRight ||
           d->m_arrowType == GrainlineArrowDirection::threeWaysUpDownLeft ||
           d->m_arrowType == GrainlineArrowDirection::threeWaysUpDownRight ||
           d->m_arrowType == GrainlineArrowDirection::threeWaysUpLeftRight ||
           d->m_arrowType == GrainlineArrowDirection::threeWaysDownLeftRight;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceGrainline::IsArrowUpEnabled() const -> bool
{
    return d->m_arrowType == GrainlineArrowDirection::oneWayUp ||
           d->m_arrowType == GrainlineArrowDirection::twoWaysUpDown ||
           d->m_arrowType == GrainlineArrowDirection::twoWaysUpLeft ||
           d->m_arrowType == GrainlineArrowDirection::twoWaysUpRight ||
           d->m_arrowType == GrainlineArrowDirection::threeWaysUpDownLeft ||
           d->m_arrowType == GrainlineArrowDirection::threeWaysUpDownRight ||
           d->m_arrowType == GrainlineArrowDirection::threeWaysUpLeftRight ||
           d->m_arrowType == GrainlineArrowDirection::fourWays;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceGrainline::IsArrowDownEnabled() const -> bool
{
    return d->m_arrowType == GrainlineArrowDirection::oneWayDown ||
           d->m_arrowType == GrainlineArrowDirection::twoWaysUpDown ||
           d->m_arrowType == GrainlineArrowDirection::twoWaysDownLeft ||
           d->m_arrowType == GrainlineArrowDirection::twoWaysDownRight ||
           d->m_arrowType == GrainlineArrowDirection::threeWaysUpDownLeft ||
           d->m_arrowType == GrainlineArrowDirection::threeWaysUpDownRight ||
           d->m_arrowType == GrainlineArrowDirection::threeWaysDownLeftRight ||
           d->m_arrowType == GrainlineArrowDirection::fourWays;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceGrainline::IsArrowLeftEnabled() const -> bool
{
    return d->m_arrowType == GrainlineArrowDirection::fourWays ||
           d->m_arrowType == GrainlineArrowDirection::twoWaysUpLeft ||
           d->m_arrowType == GrainlineArrowDirection::twoWaysDownLeft ||
           d->m_arrowType == GrainlineArrowDirection::threeWaysUpDownLeft ||
           d->m_arrowType == GrainlineArrowDirection::threeWaysUpLeftRight ||
           d->m_arrowType == GrainlineArrowDirection::threeWaysDownLeftRight;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceGrainline::IsArrowRightEnabled() const -> bool
{
    return d->m_arrowType == GrainlineArrowDirection::fourWays ||
           d->m_arrowType == GrainlineArrowDirection::twoWaysUpRight ||
           d->m_arrowType == GrainlineArrowDirection::twoWaysDownRight ||
           d->m_arrowType == GrainlineArrowDirection::threeWaysUpDownRight ||
           d->m_arrowType == GrainlineArrowDirection::threeWaysUpLeftRight ||
           d->m_arrowType == GrainlineArrowDirection::threeWaysDownLeftRight;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceGrainline::ArrowUp() const -> QPolygonF
{
    const QLineF mainLine = GetMainLine();
    const qreal rotation = M_PI + qDegreesToRadians(mainLine.angle());
    const QPointF pt = mainLine.p2();

    return {
        pt,
        QPointF(pt.x() + arrowLength * cos(rotation + arrowAngle), pt.y() - arrowLength * sin(rotation + arrowAngle)),
        QPointF(pt.x() + arrowLength * cos(rotation - arrowAngle), pt.y() - arrowLength * sin(rotation - arrowAngle)),
        pt};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceGrainline::ArrowDown() const -> QPolygonF
{
    const QLineF mainLine = GetMainLine();
    const qreal rotation = qDegreesToRadians(mainLine.angle());
    const QPointF pt = mainLine.p1();

    return {
        pt,
        QPointF(pt.x() + arrowLength * cos(rotation + arrowAngle), pt.y() - arrowLength * sin(rotation + arrowAngle)),
        QPointF(pt.x() + arrowLength * cos(rotation - arrowAngle), pt.y() - arrowLength * sin(rotation - arrowAngle)),
        pt};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceGrainline::ArrowLeft() const -> QPolygonF
{
    const qreal rotation = 3 * M_PI / 2 + qDegreesToRadians(GetMainLine().angle());
    const QPointF pt = SecondaryLine().p1();

    return {
        pt,
        QPointF(pt.x() + arrowLength * cos(rotation - arrowAngle), pt.y() - arrowLength * sin(rotation - arrowAngle)),
        QPointF(pt.x() + arrowLength * cos(rotation + arrowAngle), pt.y() - arrowLength * sin(rotation + arrowAngle)),
        pt};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceGrainline::ArrowRight() const -> QPolygonF
{
    const qreal rotation = M_PI / 2 + qDegreesToRadians(GetMainLine().angle());
    const QPointF pt = SecondaryLine().p2();

    return {
        pt,
        QPointF(pt.x() + arrowLength * cos(rotation + arrowAngle), pt.y() - arrowLength * sin(rotation + arrowAngle)),
        QPointF(pt.x() + arrowLength * cos(rotation - arrowAngle), pt.y() - arrowLength * sin(rotation - arrowAngle)),
        pt};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceGrainline::Shape() const -> GrainlineShape
{
    const QLineF mainLine = GetMainLine();
    if (mainLine.isNull())
    {
        return {};
    }

    // main arrow
    QVector<QPointF> arrow1;

    if (IsArrowDownEnabled())
    {
        arrow1 << ArrowDown();
    }
    else
    {
        arrow1 << mainLine.p1();
    }

    if (IsArrowUpEnabled())
    {
        arrow1 << ArrowUp();
    }
    else
    {
        arrow1 << mainLine.p2();
    }

    if (IsFourWays())
    {
        // secondary arrow
        QVector<QPointF> arrow2;
        const QLineF secondaryLine = SecondaryLine();

        if (IsArrowLeftEnabled())
        {
            arrow2 << ArrowLeft();
        }
        else
        {
            arrow2 << secondaryLine.p1();
        }

        if (IsArrowRightEnabled())
        {
            arrow2 << ArrowRight();
        }
        else
        {
            arrow2 << secondaryLine.p2();
        }

        return {arrow1, arrow2};
    }

    return {arrow1};
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VPieceGrainline::IsContained checks, if all ends of the grainline, starting at pt, are contained in
 * parent widget.
 * @param boundingRect bounding rect of piece
 * @param dX horizontal translation needed to put the arrow inside parent item
 * @param dY vertical translation needed to put the arrow inside parent item
 * @return true, if all ends of the grainline, starting at pt, are contained in the bounding rect of piece and
 * false otherwise.
 */
auto VPieceGrainline::IsContained(const QRectF &boundingRect, qreal &dX, qreal &dY) const -> bool
{
    dX = 0;
    dY = 0;

    const QLineF mainLine = GetMainLine();
    QVector<QPointF> apt = {mainLine.p1(), mainLine.p2()};
    if (IsFourWays())
    {
        const QLineF secondaryLine = SecondaryLine();
        apt.append(secondaryLine.p1());
        apt.append(secondaryLine.p2());
    }

    // single point differences
    qreal dPtX;
    qreal dPtY;
    bool bInside = true;

    for (auto item : apt)
    {
        dPtX = 0;
        dPtY = 0;
        if (boundingRect.contains(item))
        {
            continue;
        }

        if (item.x() < boundingRect.left())
        {
            dPtX = boundingRect.left() - item.x();
        }
        else if (item.x() > boundingRect.right())
        {
            dPtX = boundingRect.right() - item.x();
        }
        if (item.y() < boundingRect.top())
        {
            dPtY = boundingRect.top() - item.y();
        }
        else if (item.y() > boundingRect.bottom())
        {
            dPtY = boundingRect.bottom() - item.y();
        }

        if (fabs(dPtX) > fabs(dX))
        {
            dX = dPtX;
        }
        if (fabs(dPtY) > fabs(dY))
        {
            dY = dPtY;
        }

        bInside = false;
    }
    return bInside;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceGrainline::IsPositionValid(const QVector<QPointF> &contourPoints) const -> bool
{
    QVector<QLineF> grainLine;
    QLineF mainLine = GetMainLine();
    if (IsFourWays ())
    {
        grainLine = {mainLine, SecondaryLine()};
    }

    grainLine = {mainLine};

    for (auto line : grainLine)
    {
        QVector<QPointF> points = VAbstractCurve::CurveIntersectLine(contourPoints, line);
        for (auto &point : points)
        {
            if (not VFuzzyComparePoints (line.p1 (), point) && not VFuzzyComparePoints (line.p2 (), point))
            {
                return false;
            }
        }
    }

    QPainterPath grainLinePath;
    for (auto line : grainLine)
    {
        grainLinePath.addPath(VGObject::PainterPath(QVector<QPointF>{line.p1(), line.p2()}));
    }
    const QPainterPath contourPath = VGObject::PainterPath(contourPoints);
    return contourPath.contains(grainLinePath);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPieceGrainline::IsShapeValid() const -> bool
{
    GrainlineShape shape = Shape();
    return std::all_of(shape.cbegin(), shape.cend(), [](const auto &subShape) { return not subShape.isEmpty(); });
}

// Friend functions
//---------------------------------------------------------------------------------------------------------------------
auto operator<<(QDataStream &dataStream, const VPieceGrainline &grainline) -> QDataStream &
{
    dataStream << *grainline.d;
    return dataStream;
}

//---------------------------------------------------------------------------------------------------------------------
auto operator>>(QDataStream &dataStream, VPieceGrainline &grainline) -> QDataStream &
{
    dataStream >> *grainline.d;
    return dataStream;
}
