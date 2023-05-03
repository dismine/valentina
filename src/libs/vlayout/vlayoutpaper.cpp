/************************************************************************
 **
 **  @file   vlayoutpaper.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 1, 2015
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

#include "vlayoutpaper.h"

#include <QBrush>
#include <QCoreApplication>
#include <QGraphicsRectItem>
#include <QGraphicsScene>
#include <QList>
#include <QPen>
#include <QPointF>
#include <QRect>
#include <QRectF>
#include <QThread>
#include <QThreadPool>
#include <QVector>
#include <Qt>
#include <QtAlgorithms>

#ifdef LAYOUT_DEBUG
#include <QMutex>
#endif

#include "vbestsquare.h"
#include "vcontour.h"
#include "vlayoutpiece.h"
#include "vlayoutpaper_p.h"
#include "vposition.h"
#include "../ifc/exception/vexceptionterminatedposition.h"
#include "../vmisc/compatibility.h"

//---------------------------------------------------------------------------------------------------------------------
VLayoutPaper::VLayoutPaper()
    :d(new VLayoutPaperData)
{}

//---------------------------------------------------------------------------------------------------------------------
VLayoutPaper::VLayoutPaper(int height, int width, qreal layoutWidth)
    :d(new VLayoutPaperData(height, width, layoutWidth))
{}

//---------------------------------------------------------------------------------------------------------------------
VLayoutPaper::VLayoutPaper(const VLayoutPaper &paper)
    :d (paper.d)
{}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPaper::operator=(const VLayoutPaper &paper) -> VLayoutPaper &
{
    if ( &paper == this )
    {
        return *this;
    }
    d = paper.d;
    return *this;
}

#ifdef Q_COMPILER_RVALUE_REFS
//---------------------------------------------------------------------------------------------------------------------
VLayoutPaper::VLayoutPaper(VLayoutPaper &&paper) Q_DECL_NOTHROW
    :d (std::move(paper.d))
{}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPaper::operator=(VLayoutPaper &&paper) Q_DECL_NOTHROW->VLayoutPaper &
{
    std::swap(d, paper.d);
    return *this;
}
#endif

//---------------------------------------------------------------------------------------------------------------------
VLayoutPaper::~VLayoutPaper()
{}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPaper::GetHeight() const -> int
{
    return d->globalContour.GetHeight();
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPaper::SetHeight(int height)
{
    d->globalContour.SetHeight(height);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPaper::GetWidth() const -> int
{
    return d->globalContour.GetWidth();
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPaper::SetWidth(int width)
{
    d->globalContour.SetWidth(width);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPaper::GetLayoutWidth() const -> qreal
{
    return d->layoutWidth;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPaper::SetLayoutWidth(qreal width)
{
    if (width >= 0)
    {
        d->layoutWidth = width;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPaper::GetShift() const -> qreal
{
    return d->globalContour.GetShift();
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPaper::SetShift(qreal shift)
{
    d->globalContour.SetShift(shift);
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPaper::GetRotate() const -> bool
{
    return d->globalRotate;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPaper::SetRotate(bool value)
{
    d->globalRotate = value;
    d->localRotate = d->globalRotate;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPaper::GetFollowGrainline() const -> bool
{
    return d->followGrainline;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPaper::SetFollowGrainline(bool value)
{
    d->followGrainline = value;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPaper::GetRotationNumber() const -> int
{
    return d->globalRotationNumber;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPaper::SetRotationNumber(int value)
{
    d->globalRotationNumber = value;

    if (d->globalRotationNumber > 360 || d->globalRotationNumber < 1)
    {
        d->globalRotationNumber = 2;
    }

    d->localRotationNumber = d->globalRotationNumber;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPaper::IsSaveLength() const -> bool
{
    return d->saveLength;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPaper::SetSaveLength(bool value)
{
    d->saveLength = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPaper::SetPaperIndex(quint32 index)
{
    d->paperIndex = index;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPaper::IsOriginPaperPortrait() const -> bool
{
    return d->originPaperOrientation;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPaper::SetOriginPaperPortrait(bool portrait)
{
    d->originPaperOrientation = portrait;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPaper::ArrangeDetail(const VLayoutPiece &detail, std::atomic_bool &stop) -> bool
{
    if (detail.LayoutEdgesCount() < 3 || detail.DetailEdgesCount() < 3)
    {
        return false;//Not enough edges
    }

    if ((detail.IsForceFlipping() || detail.IsForbidFlipping()) && not d->globalRotate)
    { // Compensate forbidden flipping by rotating. 180 degree will be enough.
        d->localRotate = true;
        d->localRotationNumber = 2;
    }
    else
    { // Return to global values if was changed
        d->localRotate = d->globalRotate;
        d->localRotationNumber = d->globalRotationNumber;
    }

#ifdef LAYOUT_DEBUG
    QMutex mutex;
#endif

    VPositionData data;
    data.gContour = d->globalContour;
    data.detail = detail;
    data.rotate = d->localRotate;
    data.rotationNumber = d->localRotationNumber;
    data.followGrainline = d->followGrainline;
    data.positionsCache = d->positionsCache;
    data.isOriginPaperOrientationPortrait = d->originPaperOrientation;
#ifdef LAYOUT_DEBUG
    data.details = d->details;
    data.mutex = &mutex;
#endif

    const VBestSquare result = VPosition::ArrangeDetail(data, &stop, d->saveLength);
#ifdef LAYOUT_DEBUG
    return SaveResult(result, detail, &mutex);
#else
    return SaveResult(result, detail);
#endif
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPaper::Count() const -> vsizetype
{
    return d->details.count();
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPaper::SaveResult(const VBestSquare &bestResult, const VLayoutPiece &detail
#ifdef LAYOUT_DEBUG
                              ,
                              QMutex *mutex
#endif
                              ) -> bool
{
    if (bestResult.HasValidResult())
    {
        VLayoutPiece workDetail = detail;
        workDetail.SetMatrix(bestResult.Matrix());// Don't forget set matrix
        workDetail.SetMirror(bestResult.Mirror());

        if (d->saveLength)
        {
            d->globalContour.CeateEmptySheetContour();
        }

        const QVector<QPointF> newGContour = d->globalContour.UniteWithContour(workDetail, bestResult.GContourEdge(),
                                                                               bestResult.DetailEdge(),
                                                                               bestResult.Type());
        if (newGContour.isEmpty())
        {
            return false;
        }
        d->details.append(workDetail);
        d->globalContour.SetContour(newGContour);

        VCachedPositions positionChache;
        QVector<QPointF> layoutPoints = workDetail.GetMappedLayoutAllowancePoints();
        positionChache.boundingRect = VLayoutPiece::BoundingRect(layoutPoints);
        positionChache.layoutAllowancePath = VGObject::PainterPath(layoutPoints);
        d->positionsCache.append(positionChache);

#ifdef LAYOUT_DEBUG
#   ifdef SHOW_BEST
        VPosition::DumpFrame(d->globalContour, workDetail, mutex, d->details);
#   endif
#endif
    }
    else if (bestResult.IsTerminatedByException())
    {
        throw VExceptionTerminatedPosition(bestResult.ReasonTerminatedByException());
    }

    return bestResult.HasValidResult(); // Do we have the best result?
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPaper::GetPaperItem(bool autoCropLength, bool autoCropWidth, bool textAsPaths) const -> QGraphicsRectItem *
{
    int height = d->globalContour.GetHeight();
    int width = d->globalContour.GetWidth();

    if (autoCropLength || autoCropWidth)
    {
        QScopedPointer<QGraphicsScene> scene(new QGraphicsScene());
        QList<QGraphicsItem *> list = GetItemDetails(textAsPaths);
        for (auto item : list)
        {
            scene->addItem(item);
        }

        const QRect boundingRect = scene->itemsBoundingRect().toRect();

        if (autoCropLength)
        {
            if (d->globalContour.IsPortrait())
            {
                height = boundingRect.height() + boundingRect.y() + 1;
            }
            else
            {
                width = boundingRect.width() + boundingRect.x() + 1;
            }
        }

        if (autoCropWidth)
        {
            if (d->globalContour.IsPortrait())
            {
                width = boundingRect.width() + boundingRect.x() + 1;
            }
            else
            {
                height = boundingRect.height() + boundingRect.y() + 1;
            }
        }
    }

    auto *paper = new QGraphicsRectItem(QRectF(0, 0, width, height));
    paper->setPen(QPen(Qt::black, 1));
    paper->setBrush(QBrush(Qt::white));
    return paper;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPaper::GetGlobalContour() const -> QGraphicsPathItem *
{
    // contour
    const QVector<QPointF> points = d->globalContour.GetContour();

    QPainterPath path;
    if (points.size() > 0)
    {
        path.moveTo(points.at(0));
        for (auto point : points)
        {
            path.lineTo(point);
        }
    }

    const qreal radius = 1;
    for (auto point : points)
    {
        path.addEllipse(point.x()-radius, point.y()-radius, radius*2, radius*2);
    }

    for (int i=0; i < points.size()-1; ++i)
    {
        QLineF line(points.at(i), points.at(i+1));
        line.setLength(line.length()/2);

        path.moveTo(line.p2());
        QLineF side1(line.p2(), line.p1());
        side1.setAngle(side1.angle()+35);
        side1.setLength(3);
        path.lineTo(side1.p2());

        path.moveTo(line.p2());
        QLineF side2(line.p2(), line.p1());
        side2.setAngle(side2.angle()-35);
        side2.setLength(3);
        path.lineTo(side2.p2());
    }

    QGraphicsPathItem *item = new QGraphicsPathItem(path);
    QPen pen = item->pen();
    pen.setWidthF(0.25);
    item->setPen(pen);

    return item;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPaper::GetItemDetails(bool textAsPaths) const -> QList<QGraphicsItem *>
{
    QList<QGraphicsItem *> list;
    list.reserve(d->details.count());
    for (auto &detail : d->details)
    {
        list.append(detail.GetItem(textAsPaths));
    }
    return list;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPaper::GetDetails() const -> QVector<VLayoutPiece>
{
    return d->details;
}

//---------------------------------------------------------------------------------------------------------------------
void VLayoutPaper::SetDetails(const QVector<VLayoutPiece> &details)
{
    d->details = details;
}

//---------------------------------------------------------------------------------------------------------------------
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
void VLayoutPaper::SetDetails(const QList<VLayoutPiece> &details)
{
    d->details = ConvertToVector(details);
}
#endif

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPaper::DetailsBoundingRect() const -> QRectF
{
    QRectF rec;
    for (const auto &detail : d->details)
    {
        rec = rec.united(detail.MappedDetailBoundingRect());
    }

    return rec;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLayoutPaper::Efficiency() const -> qreal
{
    qreal efficiency = 0;
    for (const auto &detail : d->details)
    {
        efficiency += static_cast<qreal>(detail.Square());
    }

    const QRectF boundingRect = DetailsBoundingRect();

    return efficiency / (boundingRect.width() * boundingRect.height()) * 100.0;
}
