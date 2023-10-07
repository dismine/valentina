/************************************************************************
 **
 **  @file   vabstractcubicbezierpath.cpp
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

#include "vabstractcubicbezierpath.h"
#include "vsplinepoint.h"

#include <QPainterPath>
#include <QtDebug>

#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../vmisc/def.h"
#include "vabstractapplication.h"
#include "vpointf.h"
#include "vspline.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
VAbstractCubicBezierPath::VAbstractCubicBezierPath(const GOType &type, const quint32 &idObject, const Draw &mode)
  : VAbstractBezier(type, idObject, mode)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCubicBezierPath::operator=(const VAbstractCubicBezierPath &curve) -> VAbstractCubicBezierPath &
{
    if (&curve == this)
    {
        return *this;
    }
    VAbstractBezier::operator=(curve);
    return *this;
}

//---------------------------------------------------------------------------------------------------------------------
VAbstractCubicBezierPath::~VAbstractCubicBezierPath()
{
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetPath return QPainterPath which reprezent spline path.
 * @return path.
 */
auto VAbstractCubicBezierPath::GetPath() const -> QPainterPath
{
    QPainterPath painterPath;
    for (qint32 i = 1; i <= CountSubSpl(); ++i)
    {
        painterPath.addPath(GetSpline(i).GetPath());
    }
    return painterPath;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetPathPoints return list of points what located on path.
 * @return list.
 */
auto VAbstractCubicBezierPath::GetPoints() const -> QVector<QPointF>
{
    QVector<QPointF> pathPoints;
    for (qint32 i = 1; i <= CountSubSpl(); ++i)
    {
        if (not pathPoints.isEmpty())
        {
            pathPoints.removeLast();
        }

        pathPoints += GetSpline(i).GetPoints();
    }
    return pathPoints;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief GetLength return length of spline path.
 * @return length.
 */
auto VAbstractCubicBezierPath::GetLength() const -> qreal
{
    qreal length = 0;
    for (qint32 i = 1; i <= CountSubSpl(); ++i)
    {
        length += GetSpline(i).GetLength();
    }
    return length;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCubicBezierPath::DirectionArrows() const -> QVector<DirectionArrow>
{
    QVector<DirectionArrow> arrows;
    for (qint32 i = 1; i <= CountSubSpl(); ++i)
    {
        arrows += GetSpline(i).DirectionArrows();
    }
    return arrows;
}

//---------------------------------------------------------------------------------------------------------------------
auto VAbstractCubicBezierPath::Segment(const QPointF &p) const -> int
{
    int index = -1;
    for (qint32 i = 1; i <= CountSubSpl(); ++i)
    {
        const qreal t = GetSpline(i).ParamT(p);
        if (qFuzzyIsNull(t) || not qFuzzyCompare(t, -1))
        {
            index = i;
            break;
        }
    }
    return index;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CutSplinePath cut spline path into two. This method don't return two spline path. You must create spline
 * paths by yourself.
 * Example:
 * QPointF spl1p2, spl1p3, spl2p2, spl2p3;
 * qint32 p1 = 0, p2 = 0;
 * QPointF point = splPath->CutSplinePath(length, p1, p2, spl1p2, spl1p3, spl2p2, spl2p3);
 *
 * VSplinePoint splP1 = splPath->at(p1);
 * VSplinePoint splP2 = splPath->at(p2);
 * VSpline spl1 = VSpline(splP1.P(), spl1p2, spl1p3, *p, splPath->GetKCurve());
 * VSpline spl2 = VSpline(*p, spl2p2, spl2p3, splP2.P(), splPath->GetKCurve());
 * @param length length first spline path.
 * @param p1 index first spline point in list.
 * @param p2 index second spline point in list.
 * @param spl1p2 first control point first spline.
 * @param spl1p3 second control point first spline.
 * @param spl2p2 first control point second spline.
 * @param spl2p3 second control point second spline.
 * @param pointName cutting point name.
 * @return cutting point.
 */
auto VAbstractCubicBezierPath::CutSplinePath(qreal length, qint32 &p1, qint32 &p2, QPointF &spl1p2, QPointF &spl1p3,
                                             QPointF &spl2p2, QPointF &spl2p3, const QString &pointName) const
    -> QPointF
{
    if (CountSubSpl() < 1)
    {
        throw VException(tr("Can't cut this spline"));
    }

    // Always need return two spline paths, so we must correct wrong length.
    qreal fullLength = GetLength();

    if (fullLength <= minLength)
    {
        p1 = p2 = -1;
        spl1p2 = spl1p3 = spl2p2 = spl2p3 = QPointF();

        const QString errorMsg = tr("Unable to cut curve '%1'. The curve is too short.").arg(name());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractApplication::warningMessageSignature + errorMsg;

        return {};
    }

    const qreal maxLength = fullLength - minLength;

    if (length < minLength)
    {
        length = minLength;

        QString errorMsg;
        if (not pointName.isEmpty())
        {
            errorMsg = tr("Curve '%1'. Length of a cut segment (%2) is too small. Optimize it to minimal value.")
                           .arg(name(), pointName);
        }
        else
        {
            errorMsg =
                tr("Curve '%1'. Length of a cut segment is too small. Optimize it to minimal value.").arg(name());
        }
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractApplication::warningMessageSignature + errorMsg;
    }
    else if (length > maxLength)
    {
        length = maxLength;

        QString errorMsg;
        if (not pointName.isEmpty())
        {
            errorMsg = tr("Curve '%1'. Length of a cut segment (%2) is too big. Optimize it to maximal value.")
                           .arg(name(), pointName);
        }
        else
        {
            errorMsg = tr("Curve '%1'. Length of a cut segment is too big. Optimize it to maximal value.").arg(name());
        }
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VException(errorMsg)
            : qWarning() << VAbstractApplication::warningMessageSignature + errorMsg;
    }

    fullLength = 0;
    for (qint32 i = 1; i <= CountSubSpl(); ++i)
    {
        const VSpline spl = GetSpline(i);
        const qreal splLength = spl.GetLength();
        fullLength += splLength;
        if (fullLength > length)
        {
            p1 = i - 1;
            p2 = i;
            const QPointF point =
                spl.CutSpline(length - (fullLength - splLength), spl1p2, spl1p3, spl2p2, spl2p3, pointName);

            const QVector<VSplinePoint> points = GetSplinePath();

            if (p1 > 0)
            {
                const VSplinePoint &splP1 = points.at(p1);
                QLineF line(splP1.P().toQPointF(), spl1p2);
                if (qFuzzyIsNull(line.length()))
                {
                    spl1p2.rx() += ToPixel(0.1, Unit::Mm);
                    QLineF line(splP1.P().toQPointF(), spl1p2);
                    line.setLength(ToPixel(0.1, Unit::Mm));
                    line.setAngle(splP1.Angle1() + 180);
                    spl1p2 = line.p2();
                }
            }

            if (p2 < points.size() - 1)
            {
                const VSplinePoint &splP2 = points.at(p2);
                QLineF line(splP2.P().toQPointF(), spl2p3);
                if (qFuzzyIsNull(line.length()))
                {
                    spl2p3.rx() += ToPixel(0.1, Unit::Mm);
                    QLineF line(splP2.P().toQPointF(), spl2p3);
                    line.setAngle(splP2.Angle2() + 180);
                    spl2p3 = line.p2();
                }
            }

            return point;
        }
    }
    p1 = p2 = -1;
    spl1p2 = spl1p3 = spl2p2 = spl2p3 = QPointF();
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief NameForHistory helps to create name for dialog History.
 * @param toolName first part of name. Like 'Spline path' or 'Cubic Bezier path'.
 * @return name of curve for history records.
 */
auto VAbstractCubicBezierPath::NameForHistory(const QString &toolName) const -> QString
{
    QString name = toolName;
    if (CountPoints() > 0)
    {
        name += u" %1"_s.arg(FirstPoint().name());
        if (CountSubSpl() >= 1)
        {
            name += u"_%1"_s.arg(LastPoint().name());
        }

        if (GetDuplicate() > 0)
        {
            name += u"_%1"_s.arg(GetDuplicate());
        }
    }

    QString alias;

    if (not GetAliasSuffix().isEmpty())
    {
        alias = u"%1 %2"_s.arg(toolName, GetAliasSuffix());
    }

    return not alias.isEmpty() ? u"%1 (%2)"_s.arg(alias, name) : name;
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractCubicBezierPath::CreateName()
{
    QString name;
    if (CountPoints() > 0)
    {
        name = splPath;
        name.append(u"_%1"_s.arg(FirstPoint().name()));
        if (CountSubSpl() >= 1)
        {
            name.append(u"_%1"_s.arg(LastPoint().name()));

            if (GetDuplicate() > 0)
            {
                name += u"_%1"_s.arg(GetDuplicate());
            }
        }
    }
    setName(name);
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractCubicBezierPath::CreateAlias()
{
    const QString aliasSuffix = GetAliasSuffix();
    if (aliasSuffix.isEmpty())
    {
        SetAlias(QString());
        return;
    }

    SetAlias(splPath + '_' + aliasSuffix);
}
