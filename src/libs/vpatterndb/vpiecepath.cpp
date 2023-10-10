/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 11, 2016
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

#include "vpiecepath.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/vabstractvalapplication.h"
#include "calculator.h"
#include "vcontainer.h"
#include "vpiecepath_p.h"

#include <QPainterPath>
#include <qnumeric.h>

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto CurvePoint(VSAPoint candidate, const VContainer *data, const VPieceNode &node, const QVector<QPointF> &curvePoints)
    -> VSAPoint
{
    if (node.GetTypeTool() == Tool::NodePoint)
    {
        const QPointF p = static_cast<QPointF>(*data->GeometricObject<VPointF>(node.GetId()));
        if (VAbstractCurve::IsPointOnCurve(curvePoints, p))
        {
            candidate = VSAPoint(p);
            candidate.SetSAAfter(node.GetSAAfter(data, *data->GetPatternUnit()));
            candidate.SetSABefore(node.GetSABefore(data, *data->GetPatternUnit()));
            candidate.SetAngleType(node.GetAngleType());
            candidate.SetTurnPoint(node.IsTurnPoint());
        }
    }
    return candidate;
}

//---------------------------------------------------------------------------------------------------------------------
auto CurveStartPoint(VSAPoint candidate, const VContainer *data, const VPieceNode &node,
                     const QVector<QPointF> &curvePoints) -> VSAPoint
{
    if (node.GetTypeTool() == Tool::NodePoint)
    {
        return CurvePoint(candidate, data, node, curvePoints);
    }

    // See issue #620. Detail path not correct. Previous curve also should cut segment.
    const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(node.GetId());
    const QVector<QPointF> points = curve->GetPoints();

    if (points.isEmpty())
    {
        return candidate;
    }

    QVector<QPointF> intersections;
    for (auto i = 0; i < curvePoints.count() - 1; ++i)
    {
        QLineF segment(curvePoints.at(i), curvePoints.at(i + 1));
        intersections << VAbstractCurve::CurveIntersectLine(points, segment);
    }

    for (auto &p : intersections)
    {
        if (VFuzzyComparePoints(p, curvePoints.constFirst()) || VFuzzyComparePoints(p, curvePoints.constLast()))
        {
            continue;
        }

        candidate = VSAPoint(p);
        candidate.SetTurnPoint(true);
        break;
    }

    return candidate;
}

//---------------------------------------------------------------------------------------------------------------------
auto CurveEndPoint(VSAPoint candidate, const VContainer *data, const VPieceNode &node,
                   const QVector<QPointF> &curvePoints) -> VSAPoint
{
    if (node.GetTypeTool() == Tool::NodePoint)
    {
        return CurvePoint(candidate, data, node, curvePoints);
    }

    // See issue #620. Detail path not correct. Previous curve also should cut segment.
    const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(node.GetId());
    const QVector<QPointF> points = curve->GetPoints();

    if (points.isEmpty())
    {
        return candidate;
    }

    QVector<QPointF> intersections;
    for (auto i = 0; i < curvePoints.count() - 1; ++i)
    {
        QLineF segment(curvePoints.at(i), curvePoints.at(i + 1));
        intersections << VAbstractCurve::CurveIntersectLine(points, segment);
    }

    for (auto &p : intersections)
    {
        if (VFuzzyComparePoints(p, curvePoints.constFirst()) || VFuzzyComparePoints(p, curvePoints.constLast()))
        {
            continue;
        }

        candidate = VSAPoint(p);
        candidate.SetTurnPoint(true);
        break;
    }

    return candidate;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief indexOfNode return index in list node using id object.
 * @param list list nodes detail.
 * @param id object (arc, point, spline, splinePath) id.
 * @return index in list or -1 id can't find.
 */
auto IndexOfNode(const QVector<VPieceNode> &list, quint32 id) -> int
{
    for (int i = 0; i < list.size(); ++i)
    {
        if (list.at(i).GetId() == id)
        {
            return i;
        }
    }
    qDebug() << "Can't find node.";
    return -1;
}

//---------------------------------------------------------------------------------------------------------------------
auto MakePainterPath(const QVector<QPointF> &points) -> QPainterPath
{
    QPainterPath path;

    if (not points.isEmpty())
    {
        path.addPolygon(QPolygonF(points));
        path.setFillRule(Qt::WindingFill);
    }

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
template <class T> auto FindTipDirection(const QVector<T> &points) -> qreal
{
    if (points.size() <= 1)
    {
        return 0;
    }

    const T &first = points.constFirst();

    for (int i = 1; i < points.size(); ++i)
    {
        if (first != points.at(i))
        {
            QLineF line(first, points.at(i));
            line.setAngle(line.angle() + 180);
            return line.angle();
        }
    }

    return 0;
}

//---------------------------------------------------------------------------------------------------------------------
auto IntersectionWithCuttingContour(const QVector<QPointF> &cuttingPath, const QVector<VLayoutPoint> &points,
                                    QPointF *connection) -> bool
{
    if (points.size() <= 1)
    {
        return false;
    }

    const QPointF &first = points.constFirst();

    if (VAbstractCurve::IsPointOnCurve(cuttingPath, first))
    { // Point is already part of a cutting contour
        *connection = first;
        return true;
    }

    return VAbstractCurve::CurveIntersectAxis(first, FindTipDirection(points), cuttingPath, connection);
}

//---------------------------------------------------------------------------------------------------------------------
template <class T>
void AppendCurveSegment(QVector<T> &points, QVector<QPointF> &segment, const VSAPoint &begin, const VSAPoint &end)
{
    points.reserve(points.size() + segment.size());

    for (int i = 0; i < segment.size(); ++i)
    {
        VLayoutPoint lp(segment.at(i));
        if (i == 0)
        {
            lp.SetTurnPoint(VFuzzyComparePoints(lp, begin) ? begin.TurnPoint() : true);
        }
        else if (i == segment.size() - 1)
        {
            lp.SetTurnPoint(VFuzzyComparePoints(lp, end) ? end.TurnPoint() : true);
        }

        lp.SetCurvePoint(true);
        points.append(lp);
    }
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VPiecePath::VPiecePath()
  : d(new VPiecePathData)
{
}

//---------------------------------------------------------------------------------------------------------------------
VPiecePath::VPiecePath(PiecePathType type)
  : d(new VPiecePathData(type))
{
}

//---------------------------------------------------------------------------------------------------------------------
COPY_CONSTRUCTOR_IMPL(VPiecePath)

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::operator=(const VPiecePath &path) -> VPiecePath &
{
    if (&path == this)
    {
        return *this;
    }
    d = path.d;
    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VPiecePath::VPiecePath(VPiecePath &&path) noexcept
  : d(std::move(path.d))
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::operator=(VPiecePath &&path) noexcept -> VPiecePath &
{
    std::swap(d, path.d);
    return *this;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
VPiecePath::~VPiecePath() = default;

//---------------------------------------------------------------------------------------------------------------------
void VPiecePath::Append(const VPieceNode &node)
{
    d->m_nodes.append(node);
}

//---------------------------------------------------------------------------------------------------------------------
void VPiecePath::Clear()
{
    d->m_nodes.clear();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::CountNodes() const -> vsizetype
{
    return d->m_nodes.size();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::operator[](vsizetype indx) -> VPieceNode &
{
    return d->m_nodes[indx];
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::at(vsizetype indx) const -> const VPieceNode &
{
    return d->m_nodes.at(indx);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::GetNodes() const -> QVector<VPieceNode>
{
    return d->m_nodes;
}

//---------------------------------------------------------------------------------------------------------------------
void VPiecePath::SetNodes(const QVector<VPieceNode> &nodes)
{
    d->m_nodes = nodes;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::GetType() const -> PiecePathType
{
    return d->m_type;
}

//---------------------------------------------------------------------------------------------------------------------
void VPiecePath::SetType(PiecePathType type)
{
    d->m_type = type;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::GetName() const -> QString
{
    return d->m_name;
}

//---------------------------------------------------------------------------------------------------------------------
void VPiecePath::SetName(const QString &name)
{
    d->m_name = name;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::GetPenType() const -> Qt::PenStyle
{
    return d->m_penType;
}

//---------------------------------------------------------------------------------------------------------------------
void VPiecePath::SetPenType(const Qt::PenStyle &type)
{
    d->m_penType = type;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::IsCutPath() const -> bool
{
    return d->m_cut;
}

//---------------------------------------------------------------------------------------------------------------------
void VPiecePath::SetCutPath(bool cut)
{
    d->m_cut = cut;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::GetVisibilityTrigger() const -> QString
{
    return d->m_visibilityTrigger;
}

//---------------------------------------------------------------------------------------------------------------------
void VPiecePath::SetVisibilityTrigger(const QString &formula)
{
    d->m_visibilityTrigger = formula;
}

//---------------------------------------------------------------------------------------------------------------------
void VPiecePath::SetFirstToCuttingContour(bool value)
{
    d->m_firstToCuttingContour = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::IsFirstToCuttingContour() const -> bool
{
    return d->m_firstToCuttingContour;
}

//---------------------------------------------------------------------------------------------------------------------
void VPiecePath::SetLastToCuttingContour(bool value)
{
    d->m_lastToCuttingContour = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::IsLastToCuttingContour() const -> bool
{
    return d->m_lastToCuttingContour;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::PathPoints(const VContainer *data, const QVector<QPointF> &cuttingPath) const -> QVector<VLayoutPoint>
{
    QVector<VLayoutPoint> points = NodesToPoints(data, d->m_nodes, GetName());

    if (GetType() == PiecePathType::InternalPath && not cuttingPath.isEmpty() && points.size() > 1)
    {
        QVector<VLayoutPoint> extended = points;

        if (IsFirstToCuttingContour())
        {
            VLayoutPoint firstConnection;
            if (IntersectionWithCuttingContour(cuttingPath, points, &firstConnection))
            {
                firstConnection.SetTurnPoint(true);
                extended.prepend(firstConnection);
            }
            else
            {
                const QString errorMsg = QObject::tr("Error in internal path '%1'. There is no intersection of first "
                                                     "point with cutting contour")
                                             .arg(GetName());
                VAbstractApplication::VApp()->IsPedantic()
                    ? throw VExceptionObjectError(errorMsg)
                    : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
            }
        }

        if (IsLastToCuttingContour())
        {
            VLayoutPoint lastConnection;
            if (IntersectionWithCuttingContour(cuttingPath, Reverse(points), &lastConnection))
            {
                lastConnection.SetTurnPoint(true);
                extended.append(lastConnection);
            }
            else
            {
                const QString errorMsg = QObject::tr("Error in internal path '%1'. There is no intersection of last "
                                                     "point with cutting contour")
                                             .arg(GetName());
                VAbstractApplication::VApp()->IsPedantic()
                    ? throw VExceptionObjectError(errorMsg)
                    : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
            }
        }

        points = extended;
    }

    return points;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::PathNodePoints(const VContainer *data, bool showExcluded) const -> QVector<VPointF>
{
    QVector<VPointF> points;
    for (int i = 0; i < CountNodes(); ++i)
    {
        switch (at(i).GetTypeTool())
        {
            case Tool::NodePoint:
            {
                if (showExcluded || not at(i).IsExcluded())
                {
                    const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(at(i).GetId());
                    points.append(*point);
                }
            }
            break;
            case Tool::NodeArc:
            case Tool::NodeElArc:
            case Tool::NodeSpline:
            case Tool::NodeSplinePath:
            default:
                break;
        }
    }

    return points;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::PathCurvePoints(const VContainer *data) const -> QVector<QVector<QPointF>>
{
    QVector<QVector<QPointF>> curves;
    for (int i = 0; i < CountNodes(); ++i)
    {
        if (at(i).IsExcluded())
        {
            continue; // skip excluded node
        }

        switch (at(i).GetTypeTool())
        {
            case (Tool::NodeArc):
            case (Tool::NodeElArc):
            case (Tool::NodeSpline):
            case (Tool::NodeSplinePath):
            {
                const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(at(i).GetId());

                const QPointF begin = StartSegment(data, i);
                const QPointF end = EndSegment(data, i);

                curves.append(curve->GetSegmentPoints(begin, end, at(i).GetReverse(), GetName()));
                break;
            }
            case (Tool::NodePoint):
            default:
                break;
        }
    }

    return curves;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::SeamAllowancePoints(const VContainer *data, qreal width, bool reverse) const -> QVector<VSAPoint>
{
    SCASSERT(data != nullptr);

    QVector<VSAPoint> pointsEkv;
    for (int i = 0; i < d->m_nodes.size(); ++i)
    {
        const VPieceNode &node = d->m_nodes.at(i);
        if (node.IsExcluded())
        {
            continue; // skip excluded node
        }

        switch (node.GetTypeTool())
        {
            case (Tool::NodePoint):
            {
                pointsEkv.append(PreparePointEkv(node, data));
            }
            break;
            case (Tool::NodeArc):
            case (Tool::NodeElArc):
            case (Tool::NodeSpline):
            case (Tool::NodeSplinePath):
            {
                const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(node.GetId());
                pointsEkv += CurveSeamAllowanceSegment(data, d->m_nodes, curve, i, node.GetReverse(), width, GetName());
            }
            break;
            default:
                qDebug() << "Get wrong tool type. Ignore." << static_cast<char>(node.GetTypeTool());
                break;
        }
    }

    if (reverse)
    {
        pointsEkv = Reverse(pointsEkv);
    }

    return pointsEkv;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::PainterPath(const VContainer *data, const QVector<QPointF> &cuttingPath) const -> QPainterPath
{
    QVector<VLayoutPoint> points = PathPoints(data, cuttingPath);
    QVector<QPointF> casted;
    CastTo(points, casted);
    return MakePainterPath(casted);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::CurvesPainterPath(const VContainer *data) const -> QVector<QPainterPath>
{
    const QVector<QVector<QPointF>> curves = PathCurvePoints(data);
    QVector<QPainterPath> paths;
    paths.reserve(curves.size());

    for (const auto &curve : curves)
    {
        paths.append(MakePainterPath(curve));
    }
    return paths;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::StartSegment(const VContainer *data, const QVector<VPieceNode> &nodes, vsizetype i) -> VSAPoint
{
    if (i < 0 || i > nodes.size() - 1)
    {
        return {};
    }

    const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(nodes.at(i).GetId());

    QVector<QPointF> points = curve->GetPoints();
    if (points.isEmpty())
    {
        return {};
    }

    if (nodes.at(i).GetReverse())
    {
        points = Reverse(points);
    }

    VSAPoint begin(points.constFirst());

    if (nodes.size() > 1)
    {
        const vsizetype index = FindInLoopNotExcludedUp(i, nodes);

        if (index != i && index != -1)
        {
            begin = CurveStartPoint(begin, data, nodes.at(index), points);
        }
    }
    return begin;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::EndSegment(const VContainer *data, const QVector<VPieceNode> &nodes, vsizetype i) -> VSAPoint
{
    if (i < 0 || i > nodes.size() - 1)
    {
        return {};
    }

    const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(nodes.at(i).GetId());

    QVector<QPointF> points = curve->GetPoints();
    if (points.isEmpty())
    {
        return {};
    }

    if (nodes.at(i).GetReverse())
    {
        points = Reverse(points);
    }

    VSAPoint end(points.constLast());

    if (nodes.size() > 2)
    {
        const vsizetype index = FindInLoopNotExcludedDown(i, nodes);

        if (index != i && index != -1)
        {
            end = CurveEndPoint(end, data, nodes.at(index), points);
        }
    }
    return end;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::Dependencies() const -> QList<quint32>
{
    QList<quint32> list;
    list.reserve(d->m_nodes.size());
    for (const auto &node : d->m_nodes)
    {
        list.append(node.GetId());
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::MissingNodes(const VPiecePath &path) const -> QVector<quint32>
{
    if (d->m_nodes.size() == path.CountNodes()) //-V807
    {
        return QVector<quint32>();
    }

    QSet<quint32> set1;
    for (qint32 i = 0; i < d->m_nodes.size(); ++i)
    {
        set1.insert(d->m_nodes.at(i).GetId());
    }

    QSet<quint32> set2;
    for (qint32 j = 0; j < path.CountNodes(); ++j)
    {
        set2.insert(path.at(j).GetId());
    }

    const QList<quint32> set3 = set1.subtract(set2).values();
    QVector<quint32> nodes;
    nodes.reserve(set3.size());
    for (qint32 i = 0; i < set3.size(); ++i)
    {
        const int index = indexOfNode(set3.at(i));
        if (index != -1)
        {
            nodes.append(d->m_nodes.at(index).GetId());
        }
    }

    return nodes;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::NodeName(int nodeIndex, const VContainer *data) const -> QString
{
    return NodeName(d->m_nodes, nodeIndex, data);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::indexOfNode(quint32 id) const -> int
{
    return indexOfNode(d->m_nodes, id);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief NodeOnEdge return nodes located on edge with index.
 * @param index index of edge.
 * @param p1 first node.
 * @param p2 second node.
 */
void VPiecePath::NodeOnEdge(quint32 index, VPieceNode &p1, VPieceNode &p2) const
{
    const QVector<VPieceNode> list = ListNodePoint();
    if (index > static_cast<quint32>(list.size()))
    {
        qDebug() << "Wrong edge index index =" << index;
        return;
    }
    p1 = list.at(static_cast<int>(index));
    if (index + 1 > static_cast<quint32>(list.size()) - 1)
    {
        p2 = list.at(0);
    }
    else
    {
        p2 = list.at(static_cast<int>(index + 1));
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::Contains(quint32 id) const -> bool
{
    for (const auto &node : d->m_nodes)
    {
        if (node.GetId() == id)
        {
            return true;
        }
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief OnEdge checks if two poins located on the edge. Edge is line between two points. If between two points
 * located arcs or splines ignore this.
 * @param p1 id first point.
 * @param p2 id second point.
 * @return true - on edge, false - no.
 */
auto VPiecePath::OnEdge(quint32 p1, quint32 p2) const -> bool
{
    const QVector<VPieceNode> list = ListNodePoint();
    if (list.size() < 2)
    {
        qDebug() << "Not enough points.";
        return false;
    }
    int i = IndexOfNode(list, p1);
    vsizetype j1 = 0, j2 = 0;

    if (i == list.size() - 1)
    {
        j1 = i - 1;
        j2 = 0;
    }
    else if (i == 0)
    {
        j1 = list.size() - 1;
        j2 = i + 1;
    }
    else
    {
        j1 = i - 1;
        j2 = i + 1;
    }

    if (list.at(j1).GetId() == p2 || list.at(j2).GetId() == p2)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Edge return edge index in detail. Edge is line between two points. If between two points
 * located arcs or splines ignore this.
 * @param p1 id first point.
 * @param p2 id second point.
 * @return edge index or -1 if points don't located on edge
 */
auto VPiecePath::Edge(quint32 p1, quint32 p2) const -> vsizetype
{
    if (OnEdge(p1, p2) == false)
    {
        qDebug() << "Points don't on edge.";
        return -1;
    }

    const QVector<VPieceNode> list = ListNodePoint();
    int i = IndexOfNode(list, p1);
    int j = IndexOfNode(list, p2);

    int min = qMin(i, j);

    if (min == 0 && (i == list.size() - 1 || j == list.size() - 1))
    {
        return list.size() - 1;
    }

    return min;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief listNodePoint return list nodes only with points.
 * @return list points node.
 */
auto VPiecePath::ListNodePoint() const -> QVector<VPieceNode>
{
    QVector<VPieceNode> list;
    for (auto &node : d->m_nodes) //-const V807
    {
        if (node.GetTypeTool() == Tool::NodePoint)
        {
            list.append(node);
        }
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RemoveEdge return path without edge with index.
 * @param index idex of edge.
 * @return path without edge with index.
 */
auto VPiecePath::RemoveEdge(quint32 index) const -> VPiecePath
{
    VPiecePath path(*this);
    path.Clear();

    // Edge can be only segment. We ignore all curves inside segments.
    const quint32 edges = static_cast<quint32>(ListNodePoint().size());
    for (quint32 i = 0; i < edges; ++i)
    {
        VPieceNode p1;
        VPieceNode p2;
        this->NodeOnEdge(i, p1, p2);
        const int j1 = this->indexOfNode(p1.GetId());

        if (i == index)
        {
            path.Append(this->at(j1));
        }
        else
        {
            const int j2 = this->indexOfNode(p2.GetId());
            int j = j1;
            do
            {
                // Add "segment" except last point. Inside can be curves too.
                path.Append(this->at(j));
                ++j;

                if (j2 < j1 && j == this->CountNodes())
                {
                    j = 0;
                }
            } while (j != j2);
        }
    }
    return path;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::StartSegment(const VContainer *data, int i) const -> VSAPoint
{
    return StartSegment(data, d->m_nodes, i);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::EndSegment(const VContainer *data, int i) const -> VSAPoint
{
    return EndSegment(data, d->m_nodes, i);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::NodePreviousPoint(const VContainer *data, int i) const -> QPointF
{
    if (i < 0 || i > d->m_nodes.size() - 1)
    {
        return {};
    }

    if (d->m_nodes.size() > 1)
    {
        vsizetype index = 0;
        if (i == 0)
        {
            index = d->m_nodes.size() - 1;
        }
        else
        {
            index = i - 1;
        }

        const VPieceNode &node = d->m_nodes.at(index);
        switch (node.GetTypeTool())
        {
            case (Tool::NodePoint):
                return static_cast<QPointF>(*data->GeometricObject<VPointF>(node.GetId()));
            case (Tool::NodeArc):
            case (Tool::NodeElArc):
            case (Tool::NodeSpline):
            case (Tool::NodeSplinePath):
            {
                const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(node.GetId());

                const VSAPoint begin = StartSegment(data, d->m_nodes, index);
                const VSAPoint end = EndSegment(data, d->m_nodes, index);

                const QVector<QPointF> points = curve->GetSegmentPoints(begin, end, node.GetReverse(), GetName());
                if (points.size() > 1)
                {
                    return points.at(points.size() - 2);
                }
            }
            break;
            default:
                qDebug() << "Get wrong tool type. Ignore." << static_cast<char>(node.GetTypeTool());
                break;
        }
    }

    return QPointF();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::NodeNextPoint(const VContainer *data, int i) const -> QPointF
{
    QPointF point;
    if (i < 0 || i > d->m_nodes.size() - 1)
    {
        return point;
    }

    if (d->m_nodes.size() > 1)
    {
        int index = 0;
        if (i == d->m_nodes.size() - 1)
        {
            index = 0;
        }
        else
        {
            index = i + 1;
        }

        const VPieceNode &node = d->m_nodes.at(index);
        switch (node.GetTypeTool())
        {
            case (Tool::NodePoint):
                return static_cast<QPointF>(*data->GeometricObject<VPointF>(node.GetId()));
            case (Tool::NodeArc):
            case (Tool::NodeElArc):
            case (Tool::NodeSpline):
            case (Tool::NodeSplinePath):
            {
                const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(node.GetId());

                const VSAPoint begin = StartSegment(data, d->m_nodes, index);
                const VSAPoint end = EndSegment(data, d->m_nodes, index);

                const QVector<QPointF> points = curve->GetSegmentPoints(begin, end, node.GetReverse(), GetName());
                if (points.size() > 1)
                {
                    return points.at(1);
                }
            }
            break;
            default:
                qDebug() << "Get wrong tool type. Ignore." << static_cast<char>(node.GetTypeTool());
                break;
        }
    }

    return point;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::IsVisible(const QHash<QString, QSharedPointer<VInternalVariable>> *vars) const -> bool
{
    SCASSERT(vars != nullptr)
    bool visible = true;
    try
    {
        QScopedPointer<Calculator> cal(new Calculator());
        const qreal result = cal->EvalFormula(vars, GetVisibilityTrigger());

        if (qIsInf(result) || qIsNaN(result))
        {
            qWarning() << QObject::tr("Visibility trigger contains error and will be ignored");
        }

        if (qFuzzyIsNull(result))
        {
            visible = false;
        }
    }
    catch (qmu::QmuParserError &e)
    {
        qDebug() << "Parser error: " << e.GetMsg();
        qWarning() << QObject::tr("Visibility trigger contains error and will be ignored");
    }
    return visible;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::indexOfNode(const QVector<VPieceNode> &nodes, quint32 id) -> int
{
    for (int i = 0; i < nodes.size(); ++i)
    {
        if (nodes.at(i).GetId() == id)
        {
            return i;
        }
    }
    qDebug() << "Can't find node.";
    return -1;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::FindInLoopNotExcludedUp(vsizetype start, const QVector<VPieceNode> &nodes) -> vsizetype
{
    if (start < 0 || start >= nodes.size())
    {
        return -1;
    }

    vsizetype i = (start == 0) ? nodes.size() - 1 : start - 1;

    if (i < 0 || i >= nodes.size())
    {
        return -1;
    }

    int checked = 0;
    bool found = false;
    do
    {
        if (not nodes.at(i).IsExcluded())
        {
            found = true;
            break;
        }

        ++checked;
        --i;
        if (i < 0)
        {
            i = nodes.size() - 1;
        }
    } while (checked < nodes.size());

    return (not found) ? -1 : i;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::FindInLoopNotExcludedDown(vsizetype start, const QVector<VPieceNode> &nodes) -> vsizetype
{
    if (start < 0 || start >= nodes.size())
    {
        return -1;
    }

    vsizetype i = (start == nodes.size() - 1) ? 0 : start + 1;

    if (i < 0 || i >= nodes.size())
    {
        return -1;
    }

    int checked = 0;
    bool found = false;
    do
    {
        if (not nodes.at(i).IsExcluded())
        {
            found = true;
            break;
        }

        ++checked;
        ++i;
        if (i >= nodes.size())
        {
            i = 0;
        }
    } while (checked < nodes.size());

    return (not found) ? -1 : i;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::PreparePointEkv(const VPieceNode &node, const VContainer *data) -> VSAPoint
{
    SCASSERT(data != nullptr)

    const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(node.GetId());
    VSAPoint p(point->toQPointF());

    p.SetTurnPoint(node.IsTurnPoint());
    p.SetSAAfter(node.GetSAAfter(data, *data->GetPatternUnit()));
    p.SetSABefore(node.GetSABefore(data, *data->GetPatternUnit()));
    p.SetAngleType(node.GetAngleType());
    p.SetManualPasskmarkLength(node.IsManualPassmarkLength());
    p.SetPasskmarkLength(node.GetPassmarkLength(data, *data->GetPatternUnit()));
    p.SetManualPasskmarkWidth(node.IsManualPassmarkWidth());
    p.SetPasskmarkWidth(node.GetPassmarkWidth(data, *data->GetPatternUnit()));
    p.SetManualPasskmarkAngle(node.IsManualPassmarkAngle());
    p.SetPasskmarkAngle(node.GetPassmarkAngle(data));
    p.SetPassmarkClockwiseOpening(node.IsPassmarkClockwiseOpening());

    return p;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::CurveSeamAllowanceSegment(const VContainer *data, const QVector<VPieceNode> &nodes,
                                           const QSharedPointer<VAbstractCurve> &curve, vsizetype i, bool reverse,
                                           qreal width, const QString &piece) -> QVector<VSAPoint>
{
    const VSAPoint begin = StartSegment(data, nodes, i);
    const VSAPoint end = EndSegment(data, nodes, i);

    const QVector<QPointF> points = curve->GetSegmentPoints(begin, end, reverse, piece);
    if (points.size() < 2)
    {
        return {};
    }

    QVector<VSAPoint> pointsEkv;
    pointsEkv.reserve(points.size());

    qreal w1 = begin.GetSAAfter();
    qreal w2 = end.GetSABefore();
    if (w1 < 0 && w2 < 0)
    { // no local widths
        for (int i = 0; i < points.size(); ++i)
        {
            VSAPoint p(points.at(i));
            p.SetAngleType(PieceNodeAngle::ByLengthCurve);
            p.SetCurvePoint(true);

            if (i == 0)
            { // first point
                p.SetSAAfter(begin.GetSAAfter());
                p.SetSABefore(begin.GetSABefore());
                p.SetAngleType(begin.GetAngleType());
                p.SetTurnPoint(VFuzzyComparePoints(p, begin) ? begin.TurnPoint() : true);
            }
            else if (i == points.size() - 1)
            { // last point
                p.SetSAAfter(end.GetSAAfter());
                p.SetSABefore(end.GetSABefore());
                p.SetAngleType(end.GetAngleType());
                p.SetTurnPoint(VFuzzyComparePoints(p, end) ? end.TurnPoint() : true);
            }

            pointsEkv.append(p);
        }
    }
    else
    {
        if (w1 < 0)
        {
            w1 = width;
        }

        if (w2 < 0)
        {
            w2 = width;
        }

        const qreal wDiff = w2 - w1; // Difference between two local widths
        const qreal fullLength = VAbstractCurve::PathLength(points);

        VSAPoint p(points.at(0)); // First point in the list
        p.SetSAAfter(begin.GetSAAfter());
        p.SetSABefore(begin.GetSABefore());
        p.SetAngleType(begin.GetAngleType());
        p.SetCurvePoint(true);
        p.SetTurnPoint(VFuzzyComparePoints(p, begin) ? begin.TurnPoint() : true);
        pointsEkv.append(p);

        qreal length = 0; // how much we handle

        for (int i = 1; i < points.size(); ++i)
        {
            p = VSAPoint(points.at(i));
            p.SetCurvePoint(true);

            if (i == points.size() - 1)
            { // last point
                p.SetSAAfter(end.GetSAAfter());
                p.SetSABefore(end.GetSABefore());
                p.SetAngleType(end.GetAngleType());
                p.SetTurnPoint(VFuzzyComparePoints(p, end) ? end.TurnPoint() : true);
            }
            else
            {
                length += QLineF(points.at(i - 1), points.at(i)).length();
                const qreal localWidth = w1 + wDiff * (length / fullLength);

                p.SetSAAfter(localWidth);
                p.SetSABefore(localWidth);
                p.SetAngleType(PieceNodeAngle::ByLengthCurve);
            }

            pointsEkv.append(p);
        }
    }

    return pointsEkv;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::NodeName(const QVector<VPieceNode> &nodes, vsizetype nodeIndex, const VContainer *data) -> QString
{
    if (not nodes.isEmpty() && (nodeIndex < 0 || nodeIndex >= nodes.size()))
    {
        return {};
    }

    try
    {
        QSharedPointer<VGObject> obj = data->GetGObject(nodes.at(nodeIndex).GetId());
        return obj->name();
    }
    catch (const VExceptionBadId &)
    {
        // ignore
    }
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPiecePath::NodesToPoints(const VContainer *data, const QVector<VPieceNode> &nodes, const QString &piece)
    -> QVector<VLayoutPoint>
{
    QVector<VLayoutPoint> points;
    for (int i = 0; i < nodes.size(); ++i)
    {
        const VPieceNode &node = nodes.at(i);
        if (node.IsExcluded())
        {
            continue; // skip excluded node
        }

        switch (node.GetTypeTool())
        {
            case (Tool::NodePoint):
            {
                const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(node.GetId());
                VLayoutPoint layoutPoint(point->toQPointF());
                layoutPoint.SetTurnPoint(node.IsTurnPoint());
                points.append(layoutPoint);
            }
            break;
            case (Tool::NodeArc):
            case (Tool::NodeElArc):
            case (Tool::NodeSpline):
            case (Tool::NodeSplinePath):
            {
                const QSharedPointer<VAbstractCurve> curve = data->GeometricObject<VAbstractCurve>(node.GetId());

                const VSAPoint begin = StartSegment(data, nodes, i);
                const VSAPoint end = EndSegment(data, nodes, i);

                QVector<QPointF> segment = curve->GetSegmentPoints(begin, end, node.GetReverse(), piece);
                AppendCurveSegment(points, segment, begin, end);
            }
            break;
            default:
                qDebug() << "Get wrong tool type. Ignore." << static_cast<char>(node.GetTypeTool());
                break;
        }
    }

    return points;
}
