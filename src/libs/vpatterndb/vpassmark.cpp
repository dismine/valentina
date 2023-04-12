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

#include "vpassmark.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../ifc/exception/vexceptioninvalidnotch.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/varc.h"
#include "../vlayout/vrawsapoint.h"
#include "../vpatterndb/testpassmark.h"

const qreal VPassmark::passmarkRadiusFactor = 0.45;

namespace
{
//---------------------------------------------------------------------------------------------------------------------
PassmarkStatus GetSeamPassmarkSAPoint(const VPiecePassmarkData &passmarkData, const QVector<QPointF> &seamAllowance,
                                      QPointF &point)
{
    bool needRollback = false; // no need for rollback
    QVector<VRawSAPoint> ekvPoints;
    ekvPoints = VAbstractPiece::EkvPoint(ekvPoints, passmarkData.previousSAPoint, passmarkData.passmarkSAPoint,
                                         passmarkData.nextSAPoint, passmarkData.passmarkSAPoint, passmarkData.saWidth,
                                         &needRollback);

    if (needRollback && not seamAllowance.isEmpty())
    {
        ekvPoints.clear();
        ekvPoints += VRawSAPoint(seamAllowance.at(seamAllowance.size()-1));
    }

    if (ekvPoints.isEmpty())
    { // Just in case
        return PassmarkStatus::Error; // Something wrong
    }

    point = ConstFirst(ekvPoints);
    return needRollback ? PassmarkStatus::Rollback : PassmarkStatus::Common;
}

//---------------------------------------------------------------------------------------------------------------------
bool FixNotchPoint(const QVector<QPointF> &seamAllowance, const QPointF &notchBase, QPointF *notch)
{
    bool fixed = true;
    if (not VAbstractCurve::IsPointOnCurve(seamAllowance, *notch))
    {
        fixed = false;
        QLineF axis = QLineF(notchBase, *notch);
        axis.setLength(ToPixel(50, Unit::Cm));
        const QVector<QPointF> points = VAbstractCurve::CurveIntersectLine(seamAllowance, axis);

        if (points.size() > 0)
        {
            if (points.size() == 1)
            {
                *notch = points.at(0);
                fixed = true;
            }
            else
            {
                QMap<qreal, int> forward;

                for ( qint32 i = 0; i < points.size(); ++i )
                {
                    if (points.at(i) == notchBase)
                    { // Always seek unique intersection
                        continue;
                    }

                    const QLineF length(notchBase, points.at(i));
                    if (qAbs(length.angle() - axis.angle()) < 0.1)
                    {
                        forward.insert(length.length(), i);
                    }
                }


                // Closest point is not always want we need. First return point in forward direction if exists.
                if (not forward.isEmpty())
                {
                    *notch = points.at(forward.first());
                    fixed = true;
                }
            }
        }
    }
    else
    { // Fixing distortion
        QLineF axis = QLineF(notchBase, *notch);
        axis.setLength(axis.length() + accuracyPointOnLine * 10);
        const QVector<QPointF> points = VAbstractCurve::CurveIntersectLine(seamAllowance, axis);
        if (points.size() == 1)
        {
            *notch = ConstFirst(points);
        }
    }

    return fixed;
}
const qreal passmarkGap = MmToPixel(1.5);

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> CreateOnePassmarkLines(const QLineF &line)
{
    return QVector<QLineF>({line});
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> CreateTwoPassmarkLines(const QLineF &line, const QVector<QPointF> &seamAllowance)
{
    QPointF l1p1;
    {
        QLineF line1 = line;
        line1.setAngle(line1.angle() + 90);
        line1.setLength(passmarkGap/2.);
        l1p1 = line1.p2();
    }

    QPointF l2p1;
    {
        QLineF line2 = line;
        line2.setAngle(line2.angle() - 90);
        line2.setLength(passmarkGap/2.);
        l2p1 = line2.p2();
    }

    QPointF l1p2;
    {
        QLineF line1 = QLineF(line.p2(), line.p1());
        line1.setAngle(line1.angle() - 90);
        line1.setLength(passmarkGap/2.);
        l1p2 = line1.p2();
    }

    QPointF l2p2;
    {
        QLineF line2 = QLineF(line.p2(), line.p1());
        line2.setAngle(line2.angle() + 90);
        line2.setLength(passmarkGap/2.);
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
QVector<QLineF> CreateThreePassmarkLines(const QLineF &line, const QVector<QPointF> &seamAllowance)
{
    QPointF l1p1;
    {
        QLineF line1 = line;
        line1.setAngle(line1.angle() + 90);
        line1.setLength(passmarkGap);
        l1p1 = line1.p2();
    }

    QPointF l2p1;
    {
        QLineF line2 = line;
        line2.setAngle(line2.angle() - 90);
        line2.setLength(passmarkGap);
        l2p1 = line2.p2();
    }

    QPointF l1p2;
    {
        QLineF line1 = QLineF(line.p2(), line.p1());
        line1.setAngle(line1.angle() - 90);
        line1.setLength(passmarkGap);
        l1p2 = line1.p2();
    }

    QPointF l2p2;
    {
        QLineF line2 = QLineF(line.p2(), line.p1());
        line2.setAngle(line2.angle() + 90);
        line2.setLength(passmarkGap);
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
QVector<QLineF> CreateTMarkPassmark(const QLineF &line)
{
    QPointF p1;
    {
        QLineF tmpLine = QLineF(line.p2(), line.p1());
        tmpLine.setAngle(tmpLine.angle() - 90);
        tmpLine.setLength(line.length() * 0.75 / 2);
        p1 = tmpLine.p2();
    }

    QPointF p2;
    {
        QLineF tmpLine = QLineF(line.p2(), line.p1());
        tmpLine.setAngle(tmpLine.angle() + 90);
        tmpLine.setLength(line.length() * 0.75 / 2);
        p2 = tmpLine.p2();
    }

    QVector<QLineF> lines;
    lines.append(line);
    lines.append(QLineF(p1, p2));
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> CreateVMarkPassmark(const QLineF &line)
{
    QLineF l1 = line;
    l1.setAngle(l1.angle() - 35);

    QLineF l2 = line;
    l2.setAngle(l2.angle() + 35);

    QVector<QLineF> lines;
    lines.append(l1);
    lines.append(l2);
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> CreateVMark2Passmark(const QLineF &line, const QVector<QPointF> &seamAllowance)
{
    QLineF l1 = QLineF(line.p2(), line.p1());
    l1.setAngle(l1.angle() + 35);

    QLineF l2 = QLineF(line.p2(), line.p1());
    l2.setAngle(l2.angle() - 35);

    QVector<QLineF> lines;
    lines.append(VPassmark::FindIntersection(l1, seamAllowance));
    lines.append(VPassmark::FindIntersection(l2, seamAllowance));
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> PointsToSegments(const QVector<QPointF> &points)
{
    QVector<QLineF> lines;
    if (points.size() >= 2)
    {
        for (int i=0; i < points.size()-1; ++i)
        {
            QLineF segment = QLineF(points.at(i), points.at(i+1));
            if (segment.length() > 0)
            {
                lines.append(segment);
            }
        }
    }
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> CreateUMarkPassmark(const QLineF &line, const QVector<QPointF> &seamAllowance)
{
    const qreal radius = line.length() * VPassmark::passmarkRadiusFactor;

    QLineF baseLine = line;
    baseLine.setLength(baseLine.length() - radius); // keep defined depth

    QPointF l1p1;
    {
        QLineF line1 = baseLine;
        line1.setAngle(line1.angle() + 90);
        line1.setLength(radius);
        l1p1 = line1.p2();
    }

    QPointF l2p1;
    {
        QLineF line2 = baseLine;
        line2.setAngle(line2.angle() - 90);
        line2.setLength(radius);
        l2p1 = line2.p2();
    }

    QPointF l1p2;
    {
        QLineF line1 = QLineF(baseLine.p2(), baseLine.p1());
        line1.setAngle(line1.angle() - 90);
        line1.setLength(radius);
        l1p2 = line1.p2();
    }

    QPointF l2p2;
    {
        QLineF line2 = QLineF(baseLine.p2(), baseLine.p1());
        line2.setAngle(line2.angle() + 90);
        line2.setLength(radius);
        l2p2 = line2.p2();
    }

    QLineF axis = QLineF(baseLine.p2(), baseLine.p1());
    axis.setLength(radius);

    QVector<QPointF> points;

    QLineF seg = VPassmark::FindIntersection(QLineF(l2p2, l2p1), seamAllowance);
    seg = QLineF(seg.p2(), seg.p1());
    points.append(seg.p1());
    points.append(seg.p2());

    VArc arc(VPointF(baseLine.p2()), radius, QLineF(baseLine.p2(), l2p2).angle(), QLineF(baseLine.p2(), l1p2).angle());
    arc.SetApproximationScale(10);
    points += arc.GetPoints();

    seg = VPassmark::FindIntersection(QLineF(l1p2, l1p1), seamAllowance);
    seg = QLineF(seg.p2(), seg.p1());
    points.append(seg.p2());
    points.append(seg.p1());

    return PointsToSegments(points);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> CreateBoxMarkPassmark(const QLineF &line, const QVector<QPointF> &seamAllowance)
{
    const qreal radius = line.length() * VPassmark::passmarkRadiusFactor;

    QPointF l1p1;
    {
        QLineF line1 = line;
        line1.setAngle(line1.angle() + 90);
        line1.setLength(radius);
        l1p1 = line1.p2();
    }

    QPointF l2p1;
    {
        QLineF line2 = line;
        line2.setAngle(line2.angle() - 90);
        line2.setLength(radius);
        l2p1 = line2.p2();
    }

    QPointF l1p2;
    {
        QLineF line1 = QLineF(line.p2(), line.p1());
        line1.setAngle(line1.angle() - 90);
        line1.setLength(radius);
        l1p2 = line1.p2();
    }

    QPointF l2p2;
    {
        QLineF line2 = QLineF(line.p2(), line.p1());
        line2.setAngle(line2.angle() + 90);
        line2.setLength(radius);
        l2p2 = line2.p2();
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
QVector<QLineF> CreatePassmarkLines(PassmarkLineType lineType, PassmarkAngleType angleType,
                                    const QVector<QLineF> &lines, const QVector<QPointF> &seamAllowance,
                                    PassmarkSide side)
{
    if (lines.isEmpty())
    {
        return QVector<QLineF>();
    }

    QVector<QLineF> passmarksLines;

    auto CreateLinesWithCorrection = [&passmarksLines, side, angleType, lines, seamAllowance]
            (QVector<QLineF> (*create)(const QLineF &, const QVector<QPointF> &))
    {
        if (angleType == PassmarkAngleType::Straightforward)
        {
            passmarksLines += (*create)(ConstFirst(lines), seamAllowance);
        }
        else
        {
            if (side == PassmarkSide::All || side == PassmarkSide::Left)
            {
                passmarksLines += (*create)(ConstFirst(lines), seamAllowance);
            }

            if (side == PassmarkSide::All || side == PassmarkSide::Right)
            {
                passmarksLines += (*create)(ConstLast(lines), seamAllowance);
            }
        }
    };

    auto CreateLines = [&passmarksLines, side, angleType, lines](QVector<QLineF> (*create)(const QLineF &))
    {
        if (angleType == PassmarkAngleType::Straightforward)
        {
            passmarksLines += (*create)(ConstFirst(lines));
        }
        else
        {
            if (side == PassmarkSide::All || side == PassmarkSide::Left)
            {
                passmarksLines += (*create)(ConstFirst(lines));
            }

            if (side == PassmarkSide::All || side == PassmarkSide::Right)
            {
                passmarksLines += (*create)(ConstLast(lines));
            }
        }
    };

    if (angleType == PassmarkAngleType::Straightforward
            || angleType == PassmarkAngleType::Intersection
            || angleType == PassmarkAngleType::IntersectionOnlyLeft
            || angleType == PassmarkAngleType::IntersectionOnlyRight
            || angleType == PassmarkAngleType::Intersection2
            || angleType == PassmarkAngleType::Intersection2OnlyLeft
            || angleType == PassmarkAngleType::Intersection2OnlyRight)
    {
        switch (lineType)
        {
            case PassmarkLineType::TwoLines:
                CreateLinesWithCorrection(CreateTwoPassmarkLines);
                break;
            case PassmarkLineType::ThreeLines:
                CreateLinesWithCorrection(CreateThreePassmarkLines);
                break;
            case PassmarkLineType::TMark:
                CreateLines(CreateTMarkPassmark);
                break;
            case PassmarkLineType::VMark:
                CreateLines(CreateVMarkPassmark);
                break;
            case PassmarkLineType::VMark2:
                CreateLinesWithCorrection(CreateVMark2Passmark);
                break;
            case PassmarkLineType::UMark:
                CreateLinesWithCorrection(CreateUMarkPassmark);
                break;
            case PassmarkLineType::BoxMark:
                CreateLinesWithCorrection(CreateBoxMarkPassmark);
                break;
            case PassmarkLineType::OneLine:
            default:
                CreateLines(CreateOnePassmarkLines);
                break;
        }
    }
    else
    {
        switch (lineType)
        {
            case PassmarkLineType::TMark:
                passmarksLines += CreateTMarkPassmark(ConstFirst(lines));
                break;
            case PassmarkLineType::OneLine:
            case PassmarkLineType::TwoLines:
            case PassmarkLineType::ThreeLines:
            case PassmarkLineType::VMark:
            case PassmarkLineType::VMark2:
            case PassmarkLineType::UMark:
            case PassmarkLineType::BoxMark:
            default:
                passmarksLines.append(ConstFirst(lines));
                break;
        }
    }

    return passmarksLines;
}

//---------------------------------------------------------------------------------------------------------------------
auto PassmarkLength(const VPiecePassmarkData &passmarkData, qreal width, bool &ok) -> qreal
{
    qreal length = 0;
    if (not passmarkData.passmarkSAPoint.IsManualPasskmarkLength())
    {
        if (passmarkData.globalPassmarkLength > accuracyPointOnLine)
        {
            ok = true;
            return passmarkData.globalPassmarkLength;
        }

        length = qMin(width * VSAPoint::passmarkFactor, VSAPoint::maxPassmarkLength);

        if (length <= accuracyPointOnLine)
        {
            const QString errorMsg = QObject::tr("Found null notch for point '%1' in piece '%2'. Length is less "
                                                 "than minimal allowed.")
                    .arg(passmarkData.nodeName, passmarkData.pieceName);
            VAbstractApplication::VApp()->IsPedantic()
                    ? throw VException(errorMsg)
                    : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;

            ok = false;
            return length;
        }

        ok = true;
        return length;
    }

    length = passmarkData.passmarkSAPoint.GetPasskmarkLength();

    ok = true;
    return length;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> PassmarkBisectorBaseLine(PassmarkStatus seamPassmarkType, const VPiecePassmarkData &passmarkData,
                                         const QPointF &seamPassmarkSAPoint, const QVector<QPointF> &seamAllowance)
{
    QLineF edge1;
    QLineF edge2;

    if (seamPassmarkType == PassmarkStatus::Common)
    {
        if (passmarkData.passmarkSAPoint.GetAngleType() == PieceNodeAngle::ByFirstEdgeSymmetry)
        {
            edge1 = QLineF(seamPassmarkSAPoint, seamAllowance.at(seamAllowance.size() - 2));
            edge2 = QLineF(seamPassmarkSAPoint, seamAllowance.at(1));
        }
        else
        {
            const QLineF bigLine1 = VAbstractPiece::ParallelLine(passmarkData.previousSAPoint,
                                                                 passmarkData.passmarkSAPoint, passmarkData.saWidth );
            const QLineF bigLine2 = VAbstractPiece::ParallelLine(passmarkData.passmarkSAPoint, passmarkData.nextSAPoint,
                                                                 passmarkData.saWidth );

            edge1 = QLineF(seamPassmarkSAPoint, bigLine1.p1());
            edge2 = QLineF(seamPassmarkSAPoint, bigLine2.p2());
        }
    }
    else if(seamPassmarkType == PassmarkStatus::Rollback)
    {
        edge1 = QLineF(seamPassmarkSAPoint, seamAllowance.at(seamAllowance.size() - 2));
        edge2 = QLineF(seamPassmarkSAPoint, seamAllowance.at(1));
    }
    else
    { // Should never happen
        return QVector<QLineF>();
    }

    bool ok = false;
    const qreal length = PassmarkLength(passmarkData, passmarkData.passmarkSAPoint.MaxLocalSA(passmarkData.saWidth),
                                        ok);
    if (not ok)
    {
        return QVector<QLineF>();
    }

    edge1.setAngle(edge1.angle() + edge1.angleTo(edge2)/2.);
    edge1.setLength(length);

    return QVector<QLineF>({edge1});
}

//---------------------------------------------------------------------------------------------------------------------
QPainterPath PassmarkToPath(const QVector<QLineF> &passmark)
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
}

//------------------------------VPiecePassmarkData---------------------------------------------------------------------
QJsonObject VPiecePassmarkData::toJson() const
{
    QJsonObject dataObject
    {
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
        {"globalPassmarkLength", static_cast<qreal>(globalPassmarkLength)},
    };

    return dataObject;
}

//---------------------------------------------------------------------------------------------------------------------
VPassmark::VPassmark()
{}

//---------------------------------------------------------------------------------------------------------------------
VPassmark::VPassmark(const VPiecePassmarkData &data)
    : m_data(data),
      m_null(false)
{
    // Correct distorsion
    if (VGObject::IsPointOnLineSegment(m_data.passmarkSAPoint, m_data.previousSAPoint,
                                       m_data.nextSAPoint))
    {
        const QPointF p = VGObject::CorrectDistortion(m_data.passmarkSAPoint, m_data.previousSAPoint,
                                                      m_data.nextSAPoint);
        m_data.passmarkSAPoint.setX(p.x());
        m_data.passmarkSAPoint.setY(p.y());
    }
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> VPassmark::FullPassmark(const VPiece &piece, const VContainer *data) const
{
    if (m_null)
    {
        return {};
    }

    if (not piece.IsSeamAllowanceBuiltIn())
    {
        QVector<QLineF> lines;
        lines += SAPassmark(piece, data, PassmarkSide::All);
        if (VAbstractApplication::VApp()->Settings()->IsDoublePassmark()
                && (VAbstractApplication::VApp()->Settings()->IsPieceShowMainPath() || not piece.IsHideMainPath())
                && m_data.isMainPathNode
                && m_data.passmarkAngleType != PassmarkAngleType::Intersection
                && m_data.passmarkAngleType != PassmarkAngleType::IntersectionOnlyLeft
                && m_data.passmarkAngleType != PassmarkAngleType::IntersectionOnlyRight
                && m_data.passmarkAngleType != PassmarkAngleType::Intersection2
                && m_data.passmarkAngleType != PassmarkAngleType::Intersection2OnlyLeft
                && m_data.passmarkAngleType != PassmarkAngleType::Intersection2OnlyRight
                && m_data.isShowSecondPassmark)
        {
            lines += BuiltInSAPassmark(piece, data);
        }
        return lines;
    }

    return BuiltInSAPassmark(piece, data);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> VPassmark::SAPassmark(const VPiece &piece, const VContainer *data, PassmarkSide side) const
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
QVector<QLineF> VPassmark::SAPassmark(const QVector<QPointF> &seamAllowance,
                                      const QVector<QPointF> &rotatedSeamAllowance, PassmarkSide side) const
{
    if (m_null)
    {
        return {};
    }

//    DumpVector(seamAllowance, QStringLiteral("seamAllowance.json.XXXXXX")); // Uncomment for dumping test data
//    DumpVector(seamAllowance, QStringLiteral("rotatedSeamAllowance.json.XXXXXX")); // Uncomment for dumping test data
//    DumpPassmarkData(m_data, QStringLiteral("passmarkData.json.XXXXXX")); // Uncomment for dumping test data

    QVector<QLineF> lines = SAPassmarkBaseLine(seamAllowance, rotatedSeamAllowance, side);
    if (lines.isEmpty())
    {
        return lines;
    }

    lines = CreatePassmarkLines(m_data.passmarkLineType, m_data.passmarkAngleType, lines, seamAllowance, side);
//    DumpPassmarkShape(lines, QStringLiteral("passmarkShape.json.XXXXXX")); // Uncomment for dumping test data
    return lines;
}

//---------------------------------------------------------------------------------------------------------------------
bool VPassmark::IsNull() const
{
    return m_null;
}

//---------------------------------------------------------------------------------------------------------------------
VPiecePassmarkData VPassmark::Data() const
{
    return m_data;
}

//---------------------------------------------------------------------------------------------------------------------
QLineF VPassmark::FindIntersection(const QLineF &line, const QVector<QPointF> &seamAllowance)
{
    QLineF testLine = line;
    testLine.setLength(testLine.length()*10);
    QVector<QPointF> intersections = VAbstractCurve::CurveIntersectLine(seamAllowance, testLine);
    if (not intersections.isEmpty())
    {
        return QLineF(line.p1(), ConstLast(intersections));
    }

    return line;
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> VPassmark::BuiltInSAPassmark(const VPiece &piece, const VContainer *data) const
{
    if (m_null)
    {
        return QVector<QLineF>();
    }

    const QVector<QLineF> lines = BuiltInSAPassmarkBaseLine(piece);
    if (lines.isEmpty())
    {
        return QVector<QLineF>();
    }

    QVector<QPointF> points;
    CastTo(piece.MainPathPoints(data), points);
    return CreatePassmarkLines(m_data.passmarkLineType, m_data.passmarkAngleType, lines, points, PassmarkSide::All);
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> VPassmark::BuiltInSAPassmarkBaseLine(const VPiece &piece) const
{
    if (m_null)
    {
        return QVector<QLineF>();
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
                const QString errorMsg = QObject::tr("Cannot calculate a notch for point '%1' in piece '%2' with built "
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

    edge1.setAngle(edge1.angle() + edge1.angleTo(edge2)/2.);
    edge1.setLength(length);

    return {edge1};
}

//---------------------------------------------------------------------------------------------------------------------
QVector<QLineF> VPassmark::SAPassmarkBaseLine(const VPiece &piece, const VContainer *data, PassmarkSide side) const
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
QVector<QLineF> VPassmark::SAPassmarkBaseLine(const QVector<QPointF> &seamAllowance,
                                              const QVector<QPointF> &rotatedSeamAllowance, PassmarkSide side) const
{
    if (m_null)
    {
        return {};
    }

    if (rotatedSeamAllowance.size() < 2)
    {
        const QString errorMsg = QObject::tr("Cannot calculate a notch for point '%1' in piece '%2'. Seam allowance is "
                                             "empty.").arg(m_data.nodeName, m_data.pieceName);
        VAbstractApplication::VApp()->IsPedantic() ? throw VExceptionInvalidNotch(errorMsg) :
            qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        return {}; // Something wrong
    }

    QPointF seamPassmarkSAPoint;
    const PassmarkStatus seamPassmarkType = GetSeamPassmarkSAPoint(m_data, rotatedSeamAllowance, seamPassmarkSAPoint);
    if (seamPassmarkType == PassmarkStatus::Error)
    {
        const QString errorMsg = QObject::tr("Cannot calculate a notch for point '%1' in piece '%2'. Cannot find "
                                             "position for a notch.")
                                     .arg(m_data.nodeName, m_data.pieceName);
        VAbstractApplication::VApp()->IsPedantic() ? throw VExceptionInvalidNotch(errorMsg) :
            qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        return {}; // Something wrong
    }

    const QVector<QPointF>& path = (m_data.passmarkAngleType == PassmarkAngleType::Straightforward ||
                                    m_data.passmarkAngleType == PassmarkAngleType::Bisector)
                                       ? rotatedSeamAllowance
                                       : seamAllowance;

    if (not FixNotchPoint(path, m_data.passmarkSAPoint, &seamPassmarkSAPoint))
    {
        const QString errorMsg = QObject::tr("Cannot calculate a notch for point '%1' in piece '%2'. Unable to fix a "
                                             "notch position.")
                                     .arg(m_data.nodeName, m_data.pieceName);
        VAbstractApplication::VApp()->IsPedantic() ? throw VExceptionInvalidNotch(errorMsg) :
            qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
    }

    auto PassmarkIntersection = [this, path] (QLineF line, qreal width)
    {
        line.setLength(line.length()*100); // Hope 100 is enough

//        DumpVector(path, QStringLiteral("points.json.XXXXXX")); // Uncomment for dumping test data

        const QVector<QPointF> intersections = VAbstractCurve::CurveIntersectLine(path, line);

//        DumpVector(intersections, QStringLiteral("intersections.json.XXXXXX")); // Uncomment for dumping test data

        if (not intersections.isEmpty())
        {
            if (ConstLast(intersections) != m_data.passmarkSAPoint)
            {
                line = QLineF(ConstLast(intersections), m_data.passmarkSAPoint);

                bool ok = false;
                const qreal length = PassmarkLength(m_data, width, ok);
                if (not ok)
                {
                    return QLineF();
                }
                line.setLength(length);

                return line;
            }

            const QString errorMsg = QObject::tr("Cannot calculate a notch for point '%1' in piece '%2'. Notch "
                                                 "collapse.")
                    .arg(m_data.nodeName, m_data.pieceName);
            VAbstractApplication::VApp()->IsPedantic() ? throw VExceptionInvalidNotch(errorMsg) :
                                          qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        }
        else
        {
            const QString errorMsg = QObject::tr("Cannot calculate a notch for point '%1' in piece '%2'. Cannot find "
                                                 "intersection.")
                    .arg(m_data.nodeName, m_data.pieceName);
            VAbstractApplication::VApp()->IsPedantic() ? throw VExceptionInvalidNotch(errorMsg) :
                                              qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        }

        return QLineF();
    };

    if (m_data.passmarkAngleType == PassmarkAngleType::Straightforward)
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
    else if (m_data.passmarkAngleType == PassmarkAngleType::Bisector)
    {
        return PassmarkBisectorBaseLine(seamPassmarkType, m_data, seamPassmarkSAPoint, path);
    }
    else if (m_data.passmarkAngleType == PassmarkAngleType::Intersection
             || m_data.passmarkAngleType == PassmarkAngleType::IntersectionOnlyLeft
             || m_data.passmarkAngleType == PassmarkAngleType::IntersectionOnlyRight)
    {
        QVector<QLineF> lines;
        if ((m_data.passmarkAngleType == PassmarkAngleType::Intersection
                || m_data.passmarkAngleType == PassmarkAngleType::IntersectionOnlyLeft)
                && (side == PassmarkSide::All || side == PassmarkSide::Left))
        {
            // first passmark
            lines += PassmarkIntersection(QLineF(m_data.nextSAPoint, m_data.passmarkSAPoint),
                                          m_data.passmarkSAPoint.GetSABefore(m_data.saWidth));
        }

        if ((m_data.passmarkAngleType == PassmarkAngleType::Intersection
                || m_data.passmarkAngleType == PassmarkAngleType::IntersectionOnlyRight)
                && (side == PassmarkSide::All || side == PassmarkSide::Right))
        {
            // second passmark
            lines += PassmarkIntersection(QLineF(m_data.previousSAPoint, m_data.passmarkSAPoint),
                                          m_data.passmarkSAPoint.GetSAAfter(m_data.saWidth));
        }

        return lines;
    }
    else if (m_data.passmarkAngleType == PassmarkAngleType::Intersection2
             || m_data.passmarkAngleType == PassmarkAngleType::Intersection2OnlyLeft
             || m_data.passmarkAngleType == PassmarkAngleType::Intersection2OnlyRight)
    {
        QVector<QLineF> lines;
        if ((m_data.passmarkAngleType == PassmarkAngleType::Intersection2
                || m_data.passmarkAngleType == PassmarkAngleType::Intersection2OnlyLeft)
                && (side == PassmarkSide::All || side == PassmarkSide::Left))
        {
            // first passmark
            QLineF line(m_data.passmarkSAPoint, m_data.previousSAPoint);
            line.setAngle(line.angle()-90);
            lines += PassmarkIntersection(line, m_data.passmarkSAPoint.GetSABefore(m_data.saWidth));
        }

        if ((m_data.passmarkAngleType == PassmarkAngleType::Intersection2
                || m_data.passmarkAngleType == PassmarkAngleType::Intersection2OnlyRight)
                && (side == PassmarkSide::All || side == PassmarkSide::Right))
        {
            // second passmark
            QLineF line(m_data.passmarkSAPoint, m_data.nextSAPoint);
            line.setAngle(line.angle()+90);
            lines += PassmarkIntersection(line, m_data.passmarkSAPoint.GetSAAfter(m_data.saWidth));
        }

        return lines;
    }

    return {};
}

//---------------------------------------------------------------------------------------------------------------------
QPainterPath VPassmark::SAPassmarkPath(const VPiece &piece, const VContainer *data, PassmarkSide side) const
{
    return PassmarkToPath(SAPassmark(piece, data, side));
}

//---------------------------------------------------------------------------------------------------------------------
QPainterPath VPassmark::BuiltInSAPassmarkPath(const VPiece &piece, const VContainer *data) const
{
    return PassmarkToPath(BuiltInSAPassmark(piece, data));
}
