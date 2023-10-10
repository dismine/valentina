/************************************************************************
 **
 **  @file   vposition.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 1, 2015
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

#include "vposition.h"

#include <QDir>
#include <QFutureWatcher>
#include <QImage>
#include <QLineF>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPicture>
#include <QPointF>
#include <QPolygonF>
#include <QRect>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <QThreadPool>
#include <QtConcurrent>
#include <functional>

#include "../ifc/exception/vexception.h"
#include "../vlayout/vlayoutpoint.h"
#include "../vmisc/def.h"
#include "vpiecegrainline.h"

namespace
{
#ifdef LAYOUT_DEBUG
//---------------------------------------------------------------------------------------------------------------------
QPainterPath ShowDirection(const QLineF &edge)
{
    const int arrowLength = 14;
    QPainterPath path;
    if (edge.length() / arrowLength < 5)
    {
        return path;
    }

    QLineF arrow = edge;
    arrow.setLength(edge.length() / 2.0);

    // Reverse line because we want start arrow from this point
    arrow = QLineF(arrow.p2(), arrow.p1());
    const qreal angle = arrow.angle(); // we each time change line angle, better save original angle value
    arrow.setLength(arrowLength);      // arrow length in pixels

    arrow.setAngle(angle - 35);
    path.moveTo(arrow.p1());
    path.lineTo(arrow.p2());

    arrow.setAngle(angle + 35);
    path.moveTo(arrow.p1());
    path.lineTo(arrow.p2());
    return path;
}

//---------------------------------------------------------------------------------------------------------------------
QPainterPath DumpContour(const QVector<QPointF> &points)
{
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    if (points.count() >= 2)
    {
        for (qint32 i = 0; i < points.count() - 1; ++i)
        {
            path.moveTo(points.at(i));
            path.lineTo(points.at(i + 1));
        }
        path.lineTo(points.at(0));

#ifdef SHOW_DIRECTION
        for (qint32 i = 0; i < points.count() - 1; ++i)
        {
            path.addPath(ShowDirection(QLineF(points.at(i), points.at(i + 1))));
        }
#endif

#ifdef SHOW_VERTICES
        for (qint32 i = 0; i < points.count(); ++i)
        {
            path.addRect(points.at(i).x() - 3, points.at(i).y() - 3, 6, 6);
        }
#endif
    }
    return path;
}

//---------------------------------------------------------------------------------------------------------------------
QPainterPath DumpDetails(const QVector<VLayoutPiece> &details)
{
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    if (details.count() > 0)
    {
        for (auto &detail : details)
        {
            path.addPath(detail.ContourPath());
        }
    }
    return path;
}
#endif
} // anonymous namespace

//---------------------------------------------------------------------------------------------------------------------
VPosition::VPosition(const VPositionData &data, std::atomic_bool *stop, bool saveLength)
  : m_isValid(true),
    m_bestResult(VBestSquare(data.gContour.GetSize(), saveLength, data.isOriginPaperOrientationPortrait)),
    m_data(data),
    stop(stop)
{
    if (m_data.rotationNumber > 360 || m_data.rotationNumber < 1)
    {
        m_data.rotationNumber = 2;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPosition::run()
{
    if (not m_isValid)
    {
        return;
    }

    if (stop->load())
    {
        return;
    }

    try
    {
        for (int i = 1; i <= m_data.detail.LayoutEdgesCount(); ++i)
        {
            if (stop->load())
            {
                return;
            }

            m_data.i = i;
            FindBestPosition();

            if (stop->load())
            {
                return;
            }
        }
    }
    catch (const VException &e)
    {
        m_bestResult.TerminatedByException(QStringLiteral("%1\n\n%2").arg(e.ErrorMessage(), e.DetailedInformation()));
        return;
    }
    catch (std::exception &e)
    {
        m_bestResult.TerminatedByException(QString::fromLatin1(e.what()));
        return;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPosition::getBestResult() const -> VBestSquare
{
    return m_bestResult;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPosition::ArrangeDetail(const VPositionData &data, std::atomic_bool *stop, bool saveLength) -> VBestSquare
{
    VBestSquare bestResult(data.gContour.GetSize(), saveLength, data.isOriginPaperOrientationPortrait);

    if (stop->load())
    {
        return bestResult;
    }

    // First need set size of paper
    if (data.gContour.GetHeight() <= 0 || data.gContour.GetWidth() <= 0)
    {
        return bestResult;
    }

    const VLayoutPiece detail = data.detail;
    const auto detailEdgesCount = detail.LayoutEdgesCount();
    if (detailEdgesCount < 3 || detail.DetailEdgesCount() < 3)
    {
        return bestResult; // Not enough edges
    }

    QFutureWatcher<VBestSquare> watcher;
    QVector<VPosition> jobs;
    jobs.reserve(data.gContour.GlobalEdgesCount());

    for (int j = 1; j <= data.gContour.GlobalEdgesCount(); ++j)
    {
        VPositionData linkedData = data;
        linkedData.j = j;

        jobs.append(VPosition(linkedData, stop, saveLength));
    }

    Q_ASSERT(not jobs.isEmpty());

    std::function<VBestSquare(VPosition position)> Nest = [](VPosition position)
    {
        position.run();
        return position.getBestResult();
    };

    watcher.setFuture(QtConcurrent::mapped(jobs, Nest));

    QEventLoop wait;
    QObject::connect(&watcher, &QFutureWatcher<VBestSquare>::finished, &wait, &QEventLoop::quit);
    wait.exec();

    if (stop->load())
    {
        return bestResult;
    }

    QList<VBestSquare> results = watcher.future().results();
    for (auto &result : results)
    {
        bestResult.NewResult(result);
    }

    return bestResult;
}

//---------------------------------------------------------------------------------------------------------------------
void VPosition::SaveCandidate(VBestSquare &bestResult, const VLayoutPiece &detail, int globalI, int detJ, BestFrom type)
{
    if (bestResult.IsSaveLength())
    {
        m_data.gContour.CeateEmptySheetContour();
    }

    QVector<QPointF> newGContour = m_data.gContour.UniteWithContour(detail, globalI, detJ, type);
    newGContour.append(newGContour.constFirst());
    const QSizeF size = QPolygonF(newGContour).boundingRect().size();
    const QRectF boundingRect = detail.MappedDetailBoundingRect();
    const qreal depthPosition = m_data.isOriginPaperOrientationPortrait ? boundingRect.y() : boundingRect.x();
    const qreal sidePosition = m_data.isOriginPaperOrientationPortrait ? boundingRect.x() : boundingRect.y();

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    VBestSquareResData data;

    QT_WARNING_POP

    data.bestSize = size;
    data.globalI = globalI;              // Edge of global contour
    data.detJ = detJ;                    // Edge of detail
    data.resMatrix = detail.GetMatrix(); // Matrix for rotation and translation detail
    data.resMirror = detail.IsMirror();
    data.type = type;
    data.depthPosition = depthPosition;
    data.sidePosition = sidePosition;

    bestResult.NewResult(data);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPosition::CheckCombineEdges(VLayoutPiece &detail, int j, int &dEdge) -> bool
{
    const QLineF globalEdge = m_data.gContour.GlobalEdge(j);
    bool flagMirror = false;
    bool flagSquare = false;

    CombineEdges(detail, globalEdge, dEdge);

#ifdef LAYOUT_DEBUG
#ifdef SHOW_COMBINE
    DumpFrame(m_data.gContour, detail, m_data.mutex, m_data.details);
#endif
#endif

    CrossingType type = CrossingType::Intersection;
    if (not detail.IsForceFlipping() && SheetContains(detail.MappedDetailBoundingRect()))
    {
        if (not m_data.gContour.GetContour().isEmpty())
        {
            type = Crossing(detail);
        }
        else
        {
            type = CrossingType::NoIntersection;
        }
    }

    switch (type)
    {
        case CrossingType::EdgeError:
            return false;
        case CrossingType::Intersection:
            detail.Mirror(globalEdge);
            flagMirror = true;
            break;
        case CrossingType::NoIntersection:
            flagSquare = true;
            break;
        default:
            break;
    }

    if (flagMirror && not detail.IsForbidFlipping())
    {
#ifdef LAYOUT_DEBUG
#ifdef SHOW_MIRROR
        DumpFrame(m_data.gContour, detail, m_data.mutex, m_data.details);
#endif
#endif

        EdgeIndex layoutEdge = detail.LayoutEdgeByPoint(globalEdge.p2());
        if (not layoutEdge.has_value())
        {
            return false;
        }

        dEdge = *layoutEdge;
        CrossingType type = CrossingType::Intersection;
        if (SheetContains(detail.MappedDetailBoundingRect()))
        {
            type = Crossing(detail);
        }

        switch (type)
        {
            case CrossingType::EdgeError:
                return false;
            case CrossingType::Intersection:
                flagSquare = false;
                break;
            case CrossingType::NoIntersection:
                flagSquare = true;
                break;
            default:
                break;
        }
    }
    return flagSquare;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPosition::CheckRotationEdges(VLayoutPiece &detail, int j, int dEdge, qreal angle) const -> bool
{
    const QLineF globalEdge = m_data.gContour.GlobalEdge(j);
    bool flagSquare = false;

    if (detail.IsForceFlipping())
    {
        detail.Mirror(m_data.followGrainline || detail.IsFollowGrainline() ? QLineF(10, 10, 10, 100) : globalEdge);
    }

    RotateEdges(detail, globalEdge, dEdge, angle);

#ifdef LAYOUT_DEBUG
#ifdef SHOW_ROTATION
    DumpFrame(m_data.gContour, detail, m_data.mutex, m_data.details);
#endif
#endif

    CrossingType type = CrossingType::Intersection;
    if (SheetContains(detail.MappedDetailBoundingRect()))
    {
        type = Crossing(detail);
    }

    switch (type)
    {
        case CrossingType::EdgeError:
            return false;
        case CrossingType::Intersection:
            flagSquare = false;
            break;
        case CrossingType::NoIntersection:
            flagSquare = true;
            break;
        default:
            break;
    }
    return flagSquare;
}

//---------------------------------------------------------------------------------------------------------------------
void VPosition::RotateOnAngle(qreal angle)
{
    // We should use copy of the detail.
    VLayoutPiece workDetail = m_data.detail;

    if (CheckRotationEdges(workDetail, m_data.j, m_data.i, angle))
    {
        if (stop->load())
        {
            return;
        }

#ifdef LAYOUT_DEBUG
#ifdef SHOW_CANDIDATE_BEST
        DumpFrame(m_data.gContour, workDetail, m_data.mutex, m_data.details);
#endif
#endif

        SaveCandidate(m_bestResult, workDetail, m_data.j, m_data.i, BestFrom::Rotation);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPosition::Crossing(const VLayoutPiece &detail) const -> VPosition::CrossingType
{
    if (m_data.positionsCache.isEmpty())
    {
        return CrossingType::NoIntersection;
    }

    const QVector<QPointF> layoutPoints = detail.GetMappedLayoutAllowancePoints();
    const QRectF layoutBoundingRect = VLayoutPiece::BoundingRect(layoutPoints);
    const QPainterPath layoutAllowancePath = VGObject::PainterPath(layoutPoints);

    QVector<QPointF> contourPoints;
    CastTo(detail.IsSeamAllowance() && not detail.IsSeamAllowanceBuiltIn() ? detail.GetMappedSeamAllowancePoints()
                                                                           : detail.GetMappedContourPoints(),
           contourPoints);
    const QRectF detailBoundingRect = VLayoutPiece::BoundingRect(contourPoints);
    const QPainterPath contourPath = VGObject::PainterPath(contourPoints);

    for (const auto &position : m_data.positionsCache)
    {
        if (position.boundingRect.intersects(layoutBoundingRect) ||
            position.boundingRect.contains(detailBoundingRect) || detailBoundingRect.contains(position.boundingRect))
        {
            if (position.layoutAllowancePath.contains(contourPath) ||
                contourPath.contains(position.layoutAllowancePath) ||
                position.layoutAllowancePath.intersects(layoutAllowancePath))
            {
                return CrossingType::Intersection;
            }
        }
    }

    return CrossingType::NoIntersection;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPosition::SheetContains(const QRectF &rect) const -> bool
{
    const QRectF bRect(-accuracyPointOnLine, -accuracyPointOnLine, m_data.gContour.GetWidth() + accuracyPointOnLine,
                       m_data.gContour.GetHeight() + accuracyPointOnLine);
    return bRect.contains(rect);
}

//---------------------------------------------------------------------------------------------------------------------
void VPosition::CombineEdges(VLayoutPiece &detail, const QLineF &globalEdge, int dEdge)
{
    QLineF detailEdge = detail.LayoutEdge(dEdge);

    // Find distance between two edges for two begin vertex.
    const qreal dx = globalEdge.x2() - detailEdge.x2();
    const qreal dy = globalEdge.y2() - detailEdge.y2();

    detailEdge.translate(dx, dy); // Use values for translate detail edge.

    angle_between = globalEdge.angleTo(detailEdge); // Seek angle between two edges.

    // Now we move detail to position near to global contour edge.
    detail.Translate(dx, dy);
    if (not qFuzzyIsNull(angle_between) || not qFuzzyCompare(angle_between, 360))
    {
        detail.Rotate(detailEdge.p2(), -angle_between);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPosition::RotateEdges(VLayoutPiece &detail, const QLineF &globalEdge, int dEdge, qreal angle)
{
    QLineF detailEdge = detail.LayoutEdge(dEdge);
    // Now we move detail to position near to global contour edge.
    detail.Translate(globalEdge.x2() - detailEdge.x2(), globalEdge.y2() - detailEdge.y2());
    detail.Rotate(globalEdge.p2(), angle);
}

//---------------------------------------------------------------------------------------------------------------------
void VPosition::Rotate(int number)
{
    const qreal step = 360.0 / number;
    qreal startAngle = 0;
    if (VFuzzyComparePossibleNulls(angle_between, 360))
    {
        startAngle = step;
    }

    qreal angle = startAngle;
    while (angle < 360)
    {
        if (stop->load())
        {
            return;
        }

        RotateOnAngle(angle);

        angle = angle + step;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPosition::FollowGrainline()
{
    if (stop->load())
    {
        return;
    }

    VPieceGrainline pieceGrainline = m_data.detail.GetGrainline();
    QLineF detailGrainline(10, 10, 100, 10);
    detailGrainline.setAngle(pieceGrainline.GetMainLine().angle());

    if (m_data.detail.IsForceFlipping())
    {
        VLayoutPiece workDetail = m_data.detail; // We need copy for temp change
        workDetail.Mirror(m_data.followGrainline || workDetail.IsFollowGrainline()
                              ? QLineF(10, 10, 10, 100)
                              : m_data.gContour.GlobalEdge(m_data.j));
        detailGrainline = workDetail.GetMatrix().map(detailGrainline);
    }

    if (stop->load())
    {
        return;
    }

    const qreal angle = detailGrainline.angleTo(FabricGrainline());

    if (pieceGrainline.IsArrowUpEnabled())
    {
        RotateOnAngle(angle);
    }

    if (stop->load())
    {
        return;
    }

    if (pieceGrainline.IsArrowDownEnabled())
    {
        RotateOnAngle(angle + 180);
    }

    if (stop->load())
    {
        return;
    }

    if (pieceGrainline.IsArrowLeftEnabled())
    {
        RotateOnAngle(angle + 90);
    }

    if (stop->load())
    {
        return;
    }

    if (pieceGrainline.IsArrowRightEnabled())
    {
        RotateOnAngle(angle - 90);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPosition::FindBestPosition()
{
    if (stop->load())
    {
        return;
    }

    if (m_data.detail.IsGrainlineEnabled() && (m_data.followGrainline || m_data.detail.IsFollowGrainline()))
    {
        FollowGrainline();
        return;
    }

    // We should use copy of the detail.
    VLayoutPiece workDetail = m_data.detail;

    int dEdge = m_data.i; // For mirror detail edge will be different
    if (CheckCombineEdges(workDetail, m_data.j, dEdge))
    {
        if (stop->load())
        {
            return;
        }

#ifdef LAYOUT_DEBUG
#ifdef SHOW_CANDIDATE_BEST
        DumpFrame(m_data.gContour, workDetail, m_data.mutex, m_data.details);
#endif
#endif

        SaveCandidate(m_bestResult, workDetail, m_data.j, dEdge, BestFrom::Combine);
    }

    if (stop->load())
    {
        return;
    }

    if (m_data.rotate)
    {
        Rotate(m_data.rotationNumber);
    }
}

#ifdef LAYOUT_DEBUG
//---------------------------------------------------------------------------------------------------------------------
void VPosition::DumpFrame(const VContour &contour, const VLayoutPiece &detail, QMutex *mutex,
                          const QVector<VLayoutPiece> &details = QVector<VLayoutPiece>())
{
    auto Bias = [](int length, int maxLength)
    { return length < maxLength && length * 2 < maxLength ? length : maxLength - length; };

    const int biasWidth = Bias(contour.GetWidth(), QIMAGE_MAX);
    const int biasHeight = Bias(contour.GetHeight(), QIMAGE_MAX);

    QPicture picture;
    QPainter paint;
    paint.begin(&picture);

    paint.setPen(QPen(Qt::black, 6, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    QPainterPath p;
    if (contour.GetContour().isEmpty())
    {
        p = DumpContour(contour.CutEmptySheetEdge());
        p.translate(biasWidth / 2, biasHeight / 2);
        paint.drawPath(p);
    }
    else
    {
        p = DumpContour(contour.GetContour());
        p.translate(biasWidth / 2, biasHeight / 2);
        paint.drawPath(p);
    }

#ifdef SHOW_CANDIDATE
    paint.setPen(QPen(Qt::darkGreen, 6, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    p = DumpContour(detail.GetLayoutAllowancePoints());
    p.translate(biasWidth / 2, biasHeight / 2);
    paint.drawPath(p);
#else
    Q_UNUSED(detail)
    Q_UNUSED(details)
#endif

#ifdef ARRANGED_DETAILS
    paint.setPen(QPen(Qt::blue, 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    p = DumpDetails(details);
    p.translate(biasWidth / 2, biasHeight / 2);
    paint.drawPath(p);
#else
    Q_UNUSED(details)
#endif

    // Calculate bounding rect before draw sheet rect
    const QRect pictureRect = picture.boundingRect();

    // Sheet
#ifdef SHOW_SHEET
    paint.setPen(QPen(Qt::darkRed, 15, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
    paint.drawRect(QRectF(biasWidth / 2, biasHeight / 2, contour.GetWidth(), contour.GetHeight()));
#endif

    paint.end();

    // Dump frame to image
    // Note. If program was build with Address Sanitizer possible crashes. Address Sanitizer doesn't support big
    // allocations. See page https://bitbucket.org/dismine/valentina/wiki/developers/Address_Sanitizer
    QImage frameImage(pictureRect.width() + biasWidth, pictureRect.height() + biasHeight, QImage::Format_RGB32);

    if (frameImage.isNull())
    {
        return;
    }

    frameImage.fill(Qt::white);

    QPainter paintFrameImage;
    paintFrameImage.begin(&frameImage);
    paintFrameImage.drawPicture(0, 0, picture);
    paintFrameImage.end();

    QMutexLocker locker(mutex);

    static int frame = 0;
    ++frame;

    const QString path = QDir::homePath() + QStringLiteral("/LayoutDebug/%1.png").arg(frame);
    frameImage.save(path);
}
#endif
