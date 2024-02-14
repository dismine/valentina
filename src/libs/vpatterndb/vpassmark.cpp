/************************************************************************
 **
 **  @file   vpassmark.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   1 5, 2019
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2019 Valentina project
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

#include <QPainterPath>

#include "../ifc/exception/vexceptioninvalidnotch.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/varc.h"
#include "../vlayout/vrawsapoint.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/testpath.h" // do not remove
#include "../vmisc/vabstractvalapplication.h"
#include "../vpatterndb/testpassmark.h" // do not remove
#include "vgeometrydef.h"
#include "vpassmark.h"

#include <QJsonObject>
#include <QtMath>

const qreal VPassmark::passmarkRadiusFactor = 0.45;

namespace
{
const qreal passmarkGap = MmToPixel(1.5);

//---------------------------------------------------------------------------------------------------------------------
auto GetSeamPassmarkSAPoint(const VPiecePassmarkData &passmarkData, const QVector<QPointF> &seamAllowance,
                            QPointF &point) -> PassmarkStatus
{
    bool needRollback = false; // no need for rollback
    QVector<VRawSAPoint> ekvPoints;
    ekvPoints = VAbstractPiece::EkvPoint(ekvPoints, passmarkData.previousSAPoint, passmarkData.passmarkSAPoint,
                                         passmarkData.nextSAPoint, passmarkData.passmarkSAPoint, passmarkData.saWidth,
                                         &needRollback);

    if (needRollback && not seamAllowance.isEmpty())
    {
        ekvPoints.clear();
        ekvPoints += VRawSAPoint(seamAllowance.at(seamAllowance.size() - 1));
    }

    if (ekvPoints.isEmpty())
    {                                 // Just in case
        return PassmarkStatus::Error; // Something wrong
    }

    point = ekvPoints.constFirst().ToQPointF();
    return needRollback ? PassmarkStatus::Rollback : PassmarkStatus::Common;
}

//---------------------------------------------------------------------------------------------------------------------
auto PointsToSegments(const QVector<QPointF> &points) -> QVector<QLineF>
{
    QVector<QLineF> lines;
    if (points.size() >= 2)
    {
        lines.reserve(points.size() - 1);
        for (int i = 0; i < points.size() - 1; ++i)
        {
            QLineF segment = QLineF(points.at(i), points.at(i + 1));
            if (segment.length() > 0)
            {
                lines.append(segment);
            }
        }
    }
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
auto PassmarkLength(const VPiecePassmarkData &passmarkData, qreal width, bool &ok) -> qreal
{
    auto ValidateLength = [passmarkData](qreal length)
    {
        if (length <= accuracyPointOnLine)
        {
            const QString errorMsg = QObject::tr("Found null notch for point '%1' in piece '%2'. Length is less "
                                                 "than minimal allowed.")
                                         .arg(passmarkData.nodeName, passmarkData.pieceName);
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;

            return false;
        }
        return true;
    };

    qreal length = 0;
    if (not passmarkData.passmarkSAPoint.IsManualPasskmarkLength())
    {
        if (passmarkData.globalPassmarkLength > accuracyPointOnLine)
        {
            ok = true;
            return passmarkData.globalPassmarkLength;
        }

        length = qMin(width * VSAPoint::passmarkFactor, VSAPoint::maxPassmarkLength);
        if (not ValidateLength(length))
        {
            ok = false;
            return length;
        }

        ok = true;
        return length;
    }

    length = passmarkData.passmarkSAPoint.GetPasskmarkLength();
    if (not ValidateLength(length))
    {
        ok = false;
        return length;
    }

    ok = true;
    return length;
}

//---------------------------------------------------------------------------------------------------------------------
auto PassmarkAngle(const VPiecePassmarkData &passmarkData, qreal angle) -> qreal
{
    return passmarkData.passmarkSAPoint.IsManualPasskmarkAngle() ? passmarkData.passmarkSAPoint.GetPasskmarkAngle()
                                                                 : angle;
}

//---------------------------------------------------------------------------------------------------------------------
auto PassmarkWidth(const VPiecePassmarkData &passmarkData, qreal width) -> qreal
{
    auto ValidateWidth = [passmarkData](qreal width)
    {
        if (qAbs(width) <= accuracyPointOnLine)
        {
            const QString errorMsg =
                QObject::tr("Error: notch for point '%1' in piece '%2'. Width is less than minimal allowed.")
                    .arg(passmarkData.nodeName, passmarkData.pieceName);
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        }
    };

    if (not passmarkData.passmarkSAPoint.IsManualPasskmarkWidth())
    {
        if (qAbs(passmarkData.globalPassmarkWidth) > accuracyPointOnLine)
        {
            return passmarkData.globalPassmarkWidth;
        }

        ValidateWidth(width);
        return width;
    }

    ValidateWidth(passmarkData.passmarkSAPoint.GetPasskmarkWidth());
    return passmarkData.passmarkSAPoint.GetPasskmarkWidth();
}

//---------------------------------------------------------------------------------------------------------------------
auto FixNotchPoint(const QVector<QPointF> &seamAllowance, const VPiecePassmarkData &data, QPointF *notch) -> bool
{
    QLineF axis = QLineF(data.passmarkSAPoint, *notch);

    if (data.passmarkAngleType == PassmarkAngleType::Straightforward)
    {
        axis.setAngle(PassmarkAngle(data, axis.angle()));
    }

    // Point is on seam allowance
    if (VAbstractCurve::IsPointOnCurve(seamAllowance, *notch))
    { // Fixing distortion
        axis.setLength(axis.length() + accuracyPointOnLine * 10);
        const QVector<QPointF> points = VAbstractCurve::CurveIntersectLine(seamAllowance, axis);
        if (points.size() == 1)
        {
            *notch = points.constFirst();
        }
        return true;
    }

    // Point is not on seam allowance
    axis.setLength(ToPixel(500, Unit::Cm));
    const QVector<QPointF> points = VAbstractCurve::CurveIntersectLine(seamAllowance, axis);

    if (points.empty())
    {
        return false;
    }

    if (points.size() == 1)
    {
        *notch = points.at(0);
        return true;
    }

    QMap<qreal, int> forward;

    for (qint32 i = 0; i < points.size(); ++i)
    {
        if (points.at(i) == data.passmarkSAPoint)
        { // Always seek unique intersection
            continue;
        }

        const QLineF length(data.passmarkSAPoint, points.at(i));
        if (qAbs(length.angle() - axis.angle()) < 0.1)
        {
            forward.insert(length.length(), i);
        }
    }

    // Closest point is not always want we need. First return point in forward direction if exists.
    if (not forward.isEmpty())
    {
        *notch = points.at(forward.first());
        return true;
    }

    return false;
}

//---------------------------------------------------------------------------------------------------------------------
auto CreateOnePassmarkLines(const VPiecePassmarkData &passmarkData, const QLineF &line) -> QVector<QLineF>
{
    Q_UNUSED(passmarkData);
    return {line};
}

//---------------------------------------------------------------------------------------------------------------------
auto CreateTwoPassmarkLines(const VPiecePassmarkData &passmarkData, const QLineF &line,
                            const QVector<QPointF> &seamAllowance) -> QVector<QLineF>
{
    const qreal width = PassmarkWidth(passmarkData, passmarkGap);

    QPointF l1p1;
    {
        QLineF line1 = line;
        line1.setAngle(line1.angle() + 90);
        line1.setLength(width / 2.);
        l1p1 = line1.p2();
    }

    QPointF l2p1;
    {
        QLineF line2 = line;
        line2.setAngle(line2.angle() - 90);
        line2.setLength(width / 2.);
        l2p1 = line2.p2();
    }

    QPointF l1p2;
    {
        QLineF line1 = QLineF(line.p2(), line.p1());
        line1.setAngle(line1.angle() - 90);
        line1.setLength(width / 2.);
        l1p2 = line1.p2();
    }

    QPointF l2p2;
    {
        QLineF line2 = QLineF(line.p2(), line.p1());
        line2.setAngle(line2.angle() + 90);
        line2.setLength(width / 2.);
        l2p2 = line2.p2();
    }

    QVector<QLineF> lines;
    QLineF seg = VPassmark::FindIntersection(QLineF(l1p2, l1p1), seamAllowance);
    lines.append(QLineF(seg.p2(), seg.p1()));

    seg = VPassmark::FindIntersection(QLineF(l2p2, l2p1), seamAllowance);
    lines.append(QLineF(seg.p2(), seg.p1()));
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
auto CreateThreePassmarkLines(const VPiecePassmarkData &passmarkData, const QLineF &line,
                              const QVector<QPointF> &seamAllowance) -> QVector<QLineF>
{
    const qreal width = PassmarkWidth(passmarkData, passmarkGap);

    QPointF l1p1;
    {
        QLineF line1 = line;
        line1.setAngle(line1.angle() + 90);
        line1.setLength(width / 2.);
        l1p1 = line1.p2();
    }

    QPointF l2p1;
    {
        QLineF line2 = line;
        line2.setAngle(line2.angle() - 90);
        line2.setLength(width / 2.);
        l2p1 = line2.p2();
    }

    QPointF l1p2;
    {
        QLineF line1 = QLineF(line.p2(), line.p1());
        line1.setAngle(line1.angle() - 90);
        line1.setLength(width / 2.);
        l1p2 = line1.p2();
    }

    QPointF l2p2;
    {
        QLineF line2 = QLineF(line.p2(), line.p1());
        line2.setAngle(line2.angle() + 90);
        line2.setLength(width / 2.);
        l2p2 = line2.p2();
    }

    QVector<QLineF> lines;
    QLineF seg = VPassmark::FindIntersection(QLineF(l1p2, l1p1), seamAllowance);
    lines.append(QLineF(seg.p2(), seg.p1()));

    lines.append(line);

    seg = VPassmark::FindIntersection(QLineF(l2p2, l2p1), seamAllowance);
    lines.append(QLineF(seg.p2(), seg.p1()));
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
auto CreateTMarkPassmark(const VPiecePassmarkData &passmarkData, const QLineF &line) -> QVector<QLineF>
{
    const qreal width = PassmarkWidth(passmarkData, line.length() * 0.75);

    QPointF p1;
    {
        QLineF tmpLine = QLineF(line.p2(), line.p1());
        tmpLine.setAngle(tmpLine.angle() - 90);
        tmpLine.setLength(width / 2.);
        p1 = tmpLine.p2();
    }

    QPointF p2;
    {
        QLineF tmpLine = QLineF(line.p2(), line.p1());
        tmpLine.setAngle(tmpLine.angle() + 90);
        tmpLine.setLength(width / 2.);
        p2 = tmpLine.p2();
    }

    return {line, {QLineF(p1, p2).center(), p2}, {p2, p1}};
}

//---------------------------------------------------------------------------------------------------------------------
auto CreateExternalVMarkPassmark(const VPiecePassmarkData &passmarkData, const QLineF &line) -> QVector<QLineF>
{
    constexpr qreal defAngle = 35;
    const qreal defWidth = line.length() * qTan(qDegreesToRadians(defAngle));
    const qreal width = PassmarkWidth(passmarkData, defWidth * 2.);
    const qreal angle = qRadiansToDegrees(qAtan(qAbs(width) / 2. / line.length()));

    QLineF l1 = line;
    l1.setAngle(l1.angle() - angle);

    QLineF l2 = line;
    l2.setAngle(l2.angle() + angle);

    return {{l1.p2(), l1.p1()}, l2};
}

//---------------------------------------------------------------------------------------------------------------------
auto CreateInternalVMarkPassmark(const VPiecePassmarkData &passmarkData, const QLineF &line,
                                 const QVector<QPointF> &seamAllowance) -> QVector<QLineF>
{
    constexpr qreal defAngle = 35;
    const qreal defWidth = line.length() * qTan(qDegreesToRadians(defAngle));
    const qreal width = PassmarkWidth(passmarkData, defWidth * 2.);
    const qreal angle = qRadiansToDegrees(qAtan(qAbs(width) / 2. / line.length()));

    QLineF l1 = QLineF(line.p2(), line.p1());
    l1.setAngle(l1.angle() + angle);
    l1 = VPassmark::FindIntersection(l1, seamAllowance);

    QLineF l2 = QLineF(line.p2(), line.p1());
    l2.setAngle(l2.angle() - angle);

    return {{l1.p2(), l1.p1()}, VPassmark::FindIntersection(l2, seamAllowance)};
}

//---------------------------------------------------------------------------------------------------------------------
auto CreateUMarkPassmark(const VPiecePassmarkData &passmarkData, const QLineF &line,
                         const QVector<QPointF> &seamAllowance) -> QVector<QLineF>
{
    const qreal defWidth = line.length() * VPassmark::passmarkRadiusFactor * 2;
    const qreal width = PassmarkWidth(passmarkData, defWidth);
    const qreal radius = width / 2.;

    QVector<QPointF> points;
    QLineF baseLine = line;

    QPointF l1p1;
    {
        QLineF line = baseLine;
        line.setAngle(line.angle() - 90);
        line.setLength(radius);
        l1p1 = line.p2();
    }

    QPointF l2p1;
    {
        QLineF line = baseLine;
        line.setAngle(line.angle() + 90);
        line.setLength(radius);
        l2p1 = line.p2();
    }

    if (baseLine.length() - radius > accuracyPointOnLine)
    {
        baseLine.setLength(baseLine.length() - radius); // keep defined depth

        QPointF l1p2;
        {
            QLineF line = QLineF(baseLine.p2(), baseLine.p1());
            line.setAngle(line.angle() + 90);
            line.setLength(radius);
            l1p2 = line.p2();
        }

        QPointF l2p2;
        {
            QLineF line = QLineF(baseLine.p2(), baseLine.p1());
            line.setAngle(line.angle() - 90);
            line.setLength(radius);
            l2p2 = line.p2();
        }

        QLineF seg = VPassmark::FindIntersection(QLineF(l1p2, l1p1), seamAllowance);
        seg = QLineF(seg.p2(), seg.p1());
        points.append(seg.p1());
        points.append(seg.p2());

        const qreal f1 = QLineF(baseLine.p2(), l1p2).angle();
        const qreal f2 = QLineF(baseLine.p2(), l2p2).angle();
        VArc arc(VPointF(baseLine.p2()), radius, f1, f2);
        arc.SetApproximationScale(VArc::OptimalApproximationScale(radius, f1, f2, 0.3));
        points += arc.GetPoints();

        seg = VPassmark::FindIntersection(QLineF(l2p2, l2p1), seamAllowance);
        points.append(seg.p1());
        points.append(seg.p2());
    }
    else
    {
        const qreal f1 = QLineF(baseLine.p1(), l1p1).angle();
        const qreal f2 = QLineF(baseLine.p1(), l2p1).angle();
        VArc arc(VPointF(baseLine.p1()), radius, f1, f2);
        arc.SetApproximationScale(VArc::OptimalApproximationScale(radius, f1, f2, 0.3));
        points += arc.GetPoints();
    }

    return PointsToSegments(points);
}

//---------------------------------------------------------------------------------------------------------------------
auto CreateBoxMarkPassmark(const VPiecePassmarkData &passmarkData, const QLineF &line,
                           const QVector<QPointF> &seamAllowance) -> QVector<QLineF>
{
    const qreal defWidth = line.length() * VPassmark::passmarkRadiusFactor;
    const qreal width = PassmarkWidth(passmarkData, defWidth);

    QPointF l1p1;
    {
        QLineF tmp = line;
        tmp.setAngle(tmp.angle() - 90);
        tmp.setLength(width / 2.);
        l1p1 = tmp.p2();
    }

    QPointF l1p2;
    {
        QLineF tmp = QLineF(line.p2(), line.p1());
        tmp.setAngle(tmp.angle() + 90);
        tmp.setLength(width / 2.);
        l1p2 = tmp.p2();
    }

    QPointF l2p1;
    {
        QLineF tmp = line;
        tmp.setAngle(tmp.angle() + 90);
        tmp.setLength(width / 2.);
        l2p1 = tmp.p2();
    }

    QPointF l2p2;
    {
        QLineF tmp = QLineF(line.p2(), line.p1());
        tmp.setAngle(tmp.angle() - 90);
        tmp.setLength(width / 2.);
        l2p2 = tmp.p2();
    }

    QVector<QPointF> points;

    QLineF seg = VPassmark::FindIntersection(QLineF(l1p2, l1p1), seamAllowance);
    points.append(seg.p2());
    points.append(seg.p1());

    seg = VPassmark::FindIntersection(QLineF(l2p2, l2p1), seamAllowance);
    points.append(seg.p1());
    points.append(seg.p2());

    return PointsToSegments(points);
}

//---------------------------------------------------------------------------------------------------------------------
auto CreateCheckMarkPassmark(const VPiecePassmarkData &passmarkData, const QLineF &line,
                             const QVector<QPointF> &seamAllowance) -> QVector<QLineF>
{
    constexpr qreal defAngle = 45;
    qreal defWidth = qAbs(line.length() * qTan(qDegreesToRadians(defAngle)));

    if (not passmarkData.passmarkSAPoint.IsPassmarkClockwiseOpening())
    {
        defWidth *= -1;
    }

    const qreal width = PassmarkWidth(passmarkData, defWidth);
    const qreal angle = qRadiansToDegrees(qAtan(qAbs(width) / line.length()));

    if (width > 0)
    { // clockwise
        QLineF l1(line.p2(), line.p1());
        l1.setAngle(l1.angle() + angle);
        l1 = VPassmark::FindIntersection(l1, seamAllowance);

        return {{l1.p2(), l1.p1()}, {line.p2(), line.p1()}};
    }

    QLineF l2(line.p2(), line.p1());
    l2.setAngle(l2.angle() - angle);
    l2 = VPassmark::FindIntersection(l2, seamAllowance);

    return {line, l2};
}

//---------------------------------------------------------------------------------------------------------------------
auto PassmarkToPath(const QVector<QLineF> &passmark) -> QPainterPath
{
    QPainterPath path;
    if (not passmark.isEmpty())
    {
        for (qint32 i = 0; i < passmark.count(); ++i)
        {
            path.moveTo(passmark.at(i).p1());
            path.lineTo(passmark.at(i).p2());
        }

        path.setFillRule(Qt::WindingFill);
    }
    return path;
}
} // namespace

//------------------------------VPiecePassmarkData---------------------------------------------------------------------
auto VPiecePassmarkData::toJson() const -> QJsonObject
{
    QJsonObject dataObject{
        {"previousSAPoint", previousSAPoint.toJson()},
        {"passmarkSAPoint", passmarkSAPoint.toJson()},
        {"nextSAPoint", nextSAPoint.toJson()},
        {"saWidth", saWidth},
        {"nodeName", nodeName},
        {"pieceName", pieceName},
        {"passmarkLineType", static_cast<int>(passmarkLineType)},
        {"passmarkAngleType", static_cast<int>(passmarkAngleType)},
        {"isMainPathNode", isMainPathNode},
        {"isShowSecondPassmark", isShowSecondPassmark},
        {"passmarkIndex", passmarkIndex},
        {"id", static_cast<qint64>(id)},
        {"globalPassmarkLength", globalPassmarkLength},
        {"globalPassmarkWidth", globalPassmarkWidth},
    };

    return dataObject;
}

//---------------------------------------------------------------------------------------------------------------------
VPassmark::VPassmark(const VPiecePassmarkData &data)
  : m_data(data),
    m_null(false)
{
    // Correct distorsion
    if (VGObject::IsPointOnLineSegment(m_data.passmarkSAPoint, m_data.previousSAPoint, m_data.nextSAPoint))
    {
        const QPointF p =
            VGObject::CorrectDistortion(m_data.passmarkSAPoint, m_data.previousSAPoint, m_data.nextSAPoint);
        m_data.passmarkSAPoint.setX(p.x());
        m_data.passmarkSAPoint.setY(p.y());
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPassmark::FullPassmark(const VPiece &piece, const VContainer *data) const -> QVector<QLineF>
{
    if (m_null)
    {
        return {};
    }

    if (not piece.IsSeamAllowanceBuiltIn())
    {
        QVector<QLineF> lines;
        lines += SAPassmark(piece, data, PassmarkSide::All);
        if (VAbstractApplication::VApp()->Settings()->IsDoublePassmark() &&
            (VAbstractApplication::VApp()->Settings()->IsPieceShowMainPath() || not piece.IsHideMainPath()) &&
            m_data.isMainPathNode && m_data.passmarkAngleType != PassmarkAngleType::Intersection &&
            m_data.passmarkAngleType != PassmarkAngleType::IntersectionOnlyLeft &&
            m_data.passmarkAngleType != PassmarkAngleType::IntersectionOnlyRight &&
            m_data.passmarkAngleType != PassmarkAngleType::Intersection2 &&
            m_data.passmarkAngleType != PassmarkAngleType::Intersection2OnlyLeft &&
            m_data.passmarkAngleType != PassmarkAngleType::Intersection2OnlyRight && m_data.isShowSecondPassmark)
        {
            lines += BuiltInSAPassmark(piece, data);
        }
        return lines;
    }

    return BuiltInSAPassmark(piece, data);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPassmark::SAPassmark(const VPiece &piece, const VContainer *data, PassmarkSide side) const -> QVector<QLineF>
{
    if (m_null)
    {
        return {};
    }

    if (not piece.IsSeamAllowanceBuiltIn())
    {
        // Because rollback cannot be calulated if passmark is not first point in main path we rotate it.
        QVector<QPointF> rotatedSeamAllowance;
        CastTo(piece.SeamAllowancePointsWithRotation(data, m_data.passmarkIndex), rotatedSeamAllowance);

        QVector<QPointF> seamAllowance;
        CastTo(piece.SeamAllowancePoints(data), seamAllowance);

        return SAPassmark(seamAllowance, rotatedSeamAllowance, side);
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPassmark::SAPassmark(const QVector<QPointF> &seamAllowance, const QVector<QPointF> &rotatedSeamAllowance,
                           PassmarkSide side) const -> QVector<QLineF>
{
    if (m_null)
    {
        return {};
    }

    // DumpVector(seamAllowance, QStringLiteral("seamAllowance.json.XXXXXX")); // Uncomment for dumping test data
    // DumpVector(rotatedSeamAllowance,
    //            QStringLiteral("rotatedSeamAllowance.json.XXXXXX"));       // Uncomment for dumping test data
    // DumpPassmarkData(m_data, QStringLiteral("passmarkData.json.XXXXXX")); // Uncomment for dumping test data

    QVector<QLineF> lines = SAPassmarkBaseLine(seamAllowance, rotatedSeamAllowance, side);
    if (lines.isEmpty())
    {
        return lines;
    }

    lines = CreatePassmarkLines(lines, seamAllowance, side);
    // DumpPassmarkShape(lines, QStringLiteral("passmarkShape.json.XXXXXX")); // Uncomment for dumping test data
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPassmark::IsNull() const -> bool
{
    return m_null;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPassmark::Data() const -> VPiecePassmarkData
{
    return m_data;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPassmark::FindIntersection(const QLineF &line, const QVector<QPointF> &seamAllowance) -> QLineF
{
    QLineF testLine = line;
    testLine.setLength(testLine.length() * 10);
    QVector<QPointF> intersections = VAbstractCurve::CurveIntersectLine(seamAllowance, testLine);
    if (not intersections.isEmpty())
    {
        return {line.p1(), intersections.constLast()};
    }

    return line;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPassmark::PassmarkIntersection(const QVector<QPointF> &path, QLineF line, qreal width) const -> QLineF
{
    line.setLength(line.length() * 100); // Hope 100 is enough

    // DumpVector(path, QStringLiteral("points.json.XXXXXX")); // Uncomment for dumping test data

    const QVector<QPointF> intersections = VAbstractCurve::CurveIntersectLine(path, line);

    // DumpVector(intersections, QStringLiteral("intersections.json.XXXXXX")); // Uncomment for dumping test data

    if (not intersections.isEmpty())
    {
        if (intersections.constLast() != m_data.passmarkSAPoint)
        {
            line = QLineF(intersections.constLast(), m_data.passmarkSAPoint);

            bool ok = false;
            const qreal length = PassmarkLength(m_data, width, ok);
            if (not ok)
            {
                return {};
            }
            line.setLength(length);

            return line;
        }

        const QString errorMsg =
            QCoreApplication::translate("VPassmark", "Cannot calculate a notch for point '%1' in piece '%2'. Notch "
                                                     "collapse.")
                .arg(m_data.nodeName, m_data.pieceName);
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VExceptionInvalidNotch(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
    }
    else
    {
        const QString errorMsg = QCoreApplication::translate(
                                     "VPassmark", "Cannot calculate a notch for point '%1' in piece '%2'. Cannot find "
                                                  "intersection.")
                                     .arg(m_data.nodeName, m_data.pieceName);
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VExceptionInvalidNotch(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPassmark::PassmarkStraightforwardBaseLine(const QPointF &seamPassmarkSAPoint) const -> QVector<QLineF>
{
    bool ok = false;
    const qreal length = PassmarkLength(m_data, m_data.passmarkSAPoint.MaxLocalSA(m_data.saWidth), ok);

    if (not ok)
    {
        return {};
    }

    QLineF line = QLineF(seamPassmarkSAPoint, m_data.passmarkSAPoint);
    line.setLength(length);
    return {line};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPassmark::PassmarkBisectorBaseLine(PassmarkStatus seamPassmarkType, const QPointF &seamPassmarkSAPoint,
                                         const QVector<QPointF> &seamAllowance) const -> QVector<QLineF>
{
    QLineF edge1;
    QLineF edge2;

    if (seamPassmarkType == PassmarkStatus::Common)
    {
        if (m_data.passmarkSAPoint.GetAngleType() == PieceNodeAngle::ByFirstEdgeSymmetry)
        {
            edge1 = QLineF(seamPassmarkSAPoint, seamAllowance.at(seamAllowance.size() - 2));
            edge2 = QLineF(seamPassmarkSAPoint, seamAllowance.at(1));
        }
        else
        {
            const QLineF bigLine1 =
                VAbstractPiece::ParallelLine(m_data.previousSAPoint, m_data.passmarkSAPoint, m_data.saWidth);
            const QLineF bigLine2 =
                VAbstractPiece::ParallelLine(m_data.passmarkSAPoint, m_data.nextSAPoint, m_data.saWidth);

            edge1 = QLineF(seamPassmarkSAPoint, bigLine1.p1());
            edge2 = QLineF(seamPassmarkSAPoint, bigLine2.p2());
        }
    }
    else if (seamPassmarkType == PassmarkStatus::Rollback)
    {
        edge1 = QLineF(seamPassmarkSAPoint, seamAllowance.at(seamAllowance.size() - 2));
        edge2 = QLineF(seamPassmarkSAPoint, seamAllowance.at(1));
    }
    else
    { // Should never happen
        return {};
    }

    bool ok = false;
    const qreal length = PassmarkLength(m_data, m_data.passmarkSAPoint.MaxLocalSA(m_data.saWidth), ok);
    if (not ok)
    {
        return {};
    }

    edge1.setAngle(edge1.angle() + edge1.angleTo(edge2) / 2.);
    edge1.setLength(length);

    return {edge1};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPassmark::PassmarkIntersectionBaseLine(const QVector<QPointF> &path, PassmarkSide side) const -> QVector<QLineF>
{
    QVector<QLineF> lines;
    if ((m_data.passmarkAngleType == PassmarkAngleType::Intersection ||
         m_data.passmarkAngleType == PassmarkAngleType::IntersectionOnlyLeft) &&
        (side == PassmarkSide::All || side == PassmarkSide::Left))
    {
        // first passmark
        lines += PassmarkIntersection(path, QLineF(m_data.nextSAPoint, m_data.passmarkSAPoint),
                                      m_data.passmarkSAPoint.GetSABefore(m_data.saWidth));
    }

    if ((m_data.passmarkAngleType == PassmarkAngleType::Intersection ||
         m_data.passmarkAngleType == PassmarkAngleType::IntersectionOnlyRight) &&
        (side == PassmarkSide::All || side == PassmarkSide::Right))
    {
        // second passmark
        lines += PassmarkIntersection(path, QLineF(m_data.previousSAPoint, m_data.passmarkSAPoint),
                                      m_data.passmarkSAPoint.GetSAAfter(m_data.saWidth));
    }

    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPassmark::PassmarkIntersection2BaseLine(const QVector<QPointF> &path, PassmarkSide side) const -> QVector<QLineF>
{
    QVector<QLineF> lines;
    if ((m_data.passmarkAngleType == PassmarkAngleType::Intersection2 ||
         m_data.passmarkAngleType == PassmarkAngleType::Intersection2OnlyLeft) &&
        (side == PassmarkSide::All || side == PassmarkSide::Left))
    {
        // first passmark
        QLineF line(m_data.passmarkSAPoint, m_data.previousSAPoint);
        line.setAngle(line.angle() - 90);
        lines += PassmarkIntersection(path, line, m_data.passmarkSAPoint.GetSABefore(m_data.saWidth));
    }

    if ((m_data.passmarkAngleType == PassmarkAngleType::Intersection2 ||
         m_data.passmarkAngleType == PassmarkAngleType::Intersection2OnlyRight) &&
        (side == PassmarkSide::All || side == PassmarkSide::Right))
    {
        // second passmark
        QLineF line(m_data.passmarkSAPoint, m_data.nextSAPoint);
        line.setAngle(line.angle() + 90);
        lines += PassmarkIntersection(path, line, m_data.passmarkSAPoint.GetSAAfter(m_data.saWidth));
    }

    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPassmark::BuiltInSAPassmark(const VPiece &piece, const VContainer *data) const -> QVector<QLineF>
{
    if (m_null)
    {
        return {};
    }

    const QVector<QLineF> lines = BuiltInSAPassmarkBaseLine(piece);
    if (lines.isEmpty())
    {
        return {};
    }

    QVector<QPointF> points;
    CastTo(piece.MainPathPoints(data), points);
    return CreatePassmarkLines(lines, points, PassmarkSide::All);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPassmark::BuiltInSAPassmarkBaseLine(const VPiece &piece) const -> QVector<QLineF>
{
    if (m_null)
    {
        return {};
    }

    qreal length = 0;
    if (not piece.IsSeamAllowanceBuiltIn())
    {
        bool ok = false;
        length = PassmarkLength(m_data, m_data.passmarkSAPoint.MaxLocalSA(m_data.saWidth), ok);
        if (not ok)
        {
            return {};
        }
    }
    else
    {
        if (m_data.passmarkSAPoint.IsManualPasskmarkLength())
        {
            length = m_data.passmarkSAPoint.GetPasskmarkLength();
        }
        else
        {
            if (m_data.globalPassmarkLength > accuracyPointOnLine)
            {
                length = m_data.globalPassmarkLength;
            }
            else
            {
                const QString errorMsg =
                    QCoreApplication::translate("VPassmark",
                                                "Cannot calculate a notch for point '%1' in piece '%2' with built "
                                                "in seam allowance. User must manually provide length.")
                        .arg(m_data.nodeName, m_data.pieceName);
                VAbstractApplication::VApp()->IsPedantic()
                    ? throw VExceptionInvalidNotch(errorMsg)
                    : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
                return {};
            }
        }
    }

    QLineF edge1 = QLineF(m_data.passmarkSAPoint, m_data.previousSAPoint);
    QLineF edge2 = QLineF(m_data.passmarkSAPoint, m_data.nextSAPoint);

    edge1.setAngle(edge1.angle() + edge1.angleTo(edge2) / 2.);
    edge1.setLength(length);

    return {edge1};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPassmark::SAPassmarkBaseLine(const VPiece &piece, const VContainer *data, PassmarkSide side) const
    -> QVector<QLineF>
{
    if (m_null)
    {
        return {};
    }

    if (not piece.IsSeamAllowanceBuiltIn())
    {
        // Because rollback cannot be calulated if passmark is not first point in main path we rotate it.
        QVector<QPointF> rotatedSeamAllowance;
        CastTo(piece.SeamAllowancePointsWithRotation(data, m_data.passmarkIndex), rotatedSeamAllowance);

        QVector<QPointF> seamAllowance;
        CastTo(piece.SeamAllowancePoints(data), seamAllowance);

        return SAPassmarkBaseLine(seamAllowance, rotatedSeamAllowance, side);
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPassmark::SAPassmarkBaseLine(const QVector<QPointF> &seamAllowance, const QVector<QPointF> &rotatedSeamAllowance,
                                   PassmarkSide side) const -> QVector<QLineF>
{
    if (m_null)
    {
        return {};
    }

    if (rotatedSeamAllowance.size() < 2)
    {
        const QString errorMsg =
            QCoreApplication::translate("VPassmark",
                                        "Cannot calculate a notch for point '%1' in piece '%2'. Seam allowance is "
                                        "empty.")
                .arg(m_data.nodeName, m_data.pieceName);
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VExceptionInvalidNotch(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        return {}; // Something wrong
    }

    QPointF seamPassmarkSAPoint;
    const PassmarkStatus seamPassmarkType = GetSeamPassmarkSAPoint(m_data, rotatedSeamAllowance, seamPassmarkSAPoint);
    if (seamPassmarkType == PassmarkStatus::Error)
    {
        const QString errorMsg = QCoreApplication::translate(
                                     "VPassmark", "Cannot calculate a notch for point '%1' in piece '%2'. Cannot find "
                                                  "position for a notch.")
                                     .arg(m_data.nodeName, m_data.pieceName);
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VExceptionInvalidNotch(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        return {}; // Something wrong
    }

    const QVector<QPointF> &path = (m_data.passmarkAngleType == PassmarkAngleType::Straightforward ||
                                    m_data.passmarkAngleType == PassmarkAngleType::Bisector)
                                       ? rotatedSeamAllowance
                                       : seamAllowance;

    if (not FixNotchPoint(path, m_data, &seamPassmarkSAPoint))
    {
        const QString errorMsg =
            QCoreApplication::translate("VPassmark",
                                        "Cannot calculate a notch for point '%1' in piece '%2'. Unable to fix a "
                                        "notch position.")
                .arg(m_data.nodeName, m_data.pieceName);
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VExceptionInvalidNotch(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
    }

    if (m_data.passmarkAngleType == PassmarkAngleType::Straightforward)
    {
        return PassmarkStraightforwardBaseLine(seamPassmarkSAPoint);
    }

    if (m_data.passmarkAngleType == PassmarkAngleType::Bisector)
    {
        return PassmarkBisectorBaseLine(seamPassmarkType, seamPassmarkSAPoint, path);
    }

    if (m_data.passmarkAngleType == PassmarkAngleType::Intersection ||
        m_data.passmarkAngleType == PassmarkAngleType::IntersectionOnlyLeft ||
        m_data.passmarkAngleType == PassmarkAngleType::IntersectionOnlyRight)
    {
        return PassmarkIntersectionBaseLine(path, side);
    }

    if (m_data.passmarkAngleType == PassmarkAngleType::Intersection2 ||
        m_data.passmarkAngleType == PassmarkAngleType::Intersection2OnlyLeft ||
        m_data.passmarkAngleType == PassmarkAngleType::Intersection2OnlyRight)
    {
        return PassmarkIntersection2BaseLine(path, side);
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPassmark::SAPassmarkPath(const VPiece &piece, const VContainer *data, PassmarkSide side) const -> QPainterPath
{
    return PassmarkToPath(SAPassmark(piece, data, side));
}

//---------------------------------------------------------------------------------------------------------------------
auto VPassmark::BuiltInSAPassmarkPath(const VPiece &piece, const VContainer *data) const -> QPainterPath
{
    return PassmarkToPath(BuiltInSAPassmark(piece, data));
}

//---------------------------------------------------------------------------------------------------------------------
auto VPassmark::CreatePassmarkLines(const QVector<QLineF> &lines, const QVector<QPointF> &seamAllowance,
                                    PassmarkSide side) const -> QVector<QLineF>
{
    if (lines.isEmpty())
    {
        return {};
    }

    auto CreateLinesWithCorrection =
        [this, side, lines, seamAllowance](
            QVector<QLineF> (*create)(const VPiecePassmarkData &passmarkData, const QLineF &, const QVector<QPointF> &))
    {
        if (m_data.passmarkAngleType == PassmarkAngleType::Straightforward)
        {
            return (*create)(m_data, lines.constFirst(), seamAllowance);
        }

        QVector<QLineF> passmarksLines;

        if (side == PassmarkSide::All || side == PassmarkSide::Left)
        {
            passmarksLines += (*create)(m_data, lines.constFirst(), seamAllowance);
        }

        if (side == PassmarkSide::All || side == PassmarkSide::Right)
        {
            passmarksLines += (*create)(m_data, lines.constLast(), seamAllowance);
        }

        return passmarksLines;
    };

    auto CreateLines =
        [this, side, lines](QVector<QLineF> (*create)(const VPiecePassmarkData &passmarkData, const QLineF &))
    {
        if (m_data.passmarkAngleType == PassmarkAngleType::Straightforward)
        {
            return (*create)(m_data, lines.constFirst());
        }

        QVector<QLineF> passmarksLines;

        if (side == PassmarkSide::All || side == PassmarkSide::Left)
        {
            passmarksLines += (*create)(m_data, lines.constFirst());
        }

        if (side == PassmarkSide::All || side == PassmarkSide::Right)
        {
            passmarksLines += (*create)(m_data, lines.constLast());
        }

        return passmarksLines;
    };

    if (m_data.passmarkAngleType != PassmarkAngleType::Bisector)
    {
        switch (m_data.passmarkLineType)
        {
            case PassmarkLineType::TwoLines:
                return CreateLinesWithCorrection(CreateTwoPassmarkLines);
            case PassmarkLineType::ThreeLines:
                return CreateLinesWithCorrection(CreateThreePassmarkLines);
            case PassmarkLineType::TMark:
                return CreateLines(CreateTMarkPassmark);
            case PassmarkLineType::ExternalVMark:
                return CreateLines(CreateExternalVMarkPassmark);
            case PassmarkLineType::InternalVMark:
                return CreateLinesWithCorrection(CreateInternalVMarkPassmark);
            case PassmarkLineType::UMark:
                return CreateLinesWithCorrection(CreateUMarkPassmark);
            case PassmarkLineType::BoxMark:
                return CreateLinesWithCorrection(CreateBoxMarkPassmark);
            case PassmarkLineType::CheckMark:
                return CreateLinesWithCorrection(CreateCheckMarkPassmark);
            case PassmarkLineType::OneLine:
            default:
                return CreateLines(CreateOnePassmarkLines);
        }
    }

    switch (m_data.passmarkLineType)
    {
        case PassmarkLineType::TMark:
            return CreateTMarkPassmark(m_data, lines.constFirst());
        case PassmarkLineType::OneLine:
        case PassmarkLineType::TwoLines:
        case PassmarkLineType::ThreeLines:
        case PassmarkLineType::ExternalVMark:
        case PassmarkLineType::InternalVMark:
        case PassmarkLineType::UMark:
        case PassmarkLineType::BoxMark:
        case PassmarkLineType::CheckMark:
        default:
            return {lines.constFirst()};
    }
}
