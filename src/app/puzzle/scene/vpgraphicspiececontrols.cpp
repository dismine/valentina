/************************************************************************
 **
 **  @file   vpgraphicspiececontrols.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   2 8, 2021
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2021 Valentina project
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
#include "vpgraphicspiececontrols.h"

#include <QCursor>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QIcon>
#include <QPainter>
#include <QtDebug>

#include "../vmisc/compatibility.h"
#include "../vwidgets/global.h"
#include "../layout/vplayout.h"
#include "../layout/vppiece.h"
#include "../undocommands/vpundopiecerotate.h"
#include "../undocommands/vpundooriginmove.h"
#include "vpgraphicspiece.h"

namespace
{
const qreal arcStartAngle = 105;
const qreal arcAngle = 25;
const qreal arcRadius = 15;

const qreal arrowTail = 2;
const qreal arrowSide = 2;
const qreal arrowLength = 4;
const qreal arrow1Angle = 13;
const qreal arrow2Angle = 38;

constexpr qreal penWidth = 2;

const qreal centerRadius1 = 5;
const qreal centerRadius2 = 10;

const QColor defaultColor = Qt::black;
const QColor hoverColor = Qt::green;

enum class HandleCorner : int
{
    Invalid = 0,
    TopLeft = 1,
    TopRight = 2,
    BottomRight = 3,
    BottomLeft = 4
};

auto TransformationOrigin(const VPLayoutPtr &layout, const QRectF &boundingRect) -> VPTransformationOrigon
{
    SCASSERT(layout != nullptr)
    VPSheetPtr sheet = layout->GetFocusedSheet();
    if (not sheet.isNull())
    {
        return sheet->TransformationOrigin();
    }

    VPTransformationOrigon origin;
    origin.origin = boundingRect.center();
    origin.custom = false;

    return origin;
}
}  // namespace

//---------------------------------------------------------------------------------------------------------------------
VPGraphicsTransformationOrigin::VPGraphicsTransformationOrigin(const VPLayoutPtr &layout, QGraphicsItem *parent)
    : QGraphicsObject(parent),
      m_layout(layout),
      m_color(defaultColor)
{
    SCASSERT(m_layout != nullptr)
    setCursor(Qt::OpenHandCursor);
    setZValue(1);
    setAcceptHoverEvents(true);

    connect(layout.data(), &VPLayout::TransformationOriginChanged, this,
            &VPGraphicsTransformationOrigin::SetTransformationOrigin);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsTransformationOrigin::SetTransformationOrigin()
{
    prepareGeometryChange();
    update();
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsTransformationOrigin::on_HideHandles(bool hide)
{
    m_originVisible = not hide;
    prepareGeometryChange();
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsTransformationOrigin::on_ShowOrigin(bool show)
{
    setVisible(show);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsTransformationOrigin::boundingRect() const -> QRectF
{
    constexpr qreal halfPenWidth = penWidth/2.;
    return Center2().boundingRect().adjusted(-halfPenWidth, -halfPenWidth, halfPenWidth, halfPenWidth);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsTransformationOrigin::shape() const -> QPainterPath
{
    return Center2();
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsTransformationOrigin::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    const qreal scale = SceneScale(scene());

    QPen pen(m_color, penWidth/scale, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    painter->setPen(pen);

    if (m_originVisible)
    {
        painter->save();
        painter->setBrush(QBrush(m_color));
        painter->drawPath(Center1());
        painter->restore();

        painter->save();
        painter->setBrush(QBrush());
        painter->drawPath(Center2());
        painter->restore();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsTransformationOrigin::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // change the cursor when clicking the left button
    if((event->button() == Qt::LeftButton))
    {
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    }
    else
    {
        //perform the default behaviour
        QGraphicsObject::mousePressEvent(event);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsTransformationOrigin::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    VPLayoutPtr layout = m_layout.toStrongRef();
    if (not layout.isNull())
    {
        VPSheetPtr sheet = layout->GetFocusedSheet();
        if (not sheet.isNull())
        {
            VPTransformationOrigon origin = sheet->TransformationOrigin();
            origin.origin = event->scenePos();
            origin.custom = true;

            auto *command = new VPUndoOriginMove(sheet, origin, m_allowChangeMerge);
            layout->UndoStack()->push(command);
        }
        prepareGeometryChange();
    }

    m_allowChangeMerge = true;
    QGraphicsObject::mouseMoveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsTransformationOrigin::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    //perform the default behaviour
    QGraphicsItem::mouseReleaseEvent(event);

    // change the cursor when clicking left button
    if (event->button() == Qt::LeftButton)
    {
        setCursor(Qt::OpenHandCursor);
        m_allowChangeMerge = false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsTransformationOrigin::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_color = hoverColor;
    QGraphicsObject::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsTransformationOrigin::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_color = defaultColor;
    QGraphicsObject::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsTransformationOrigin::RotationCenter(QPainter *painter) const -> QPainterPath
{
    QPainterPath path;

    const qreal scale = SceneScale(scene());
    qreal radius = centerRadius1/scale;
    VPTransformationOrigon transformationOrigin = TransformationOrigin(m_layout, QRectF());
    QRectF rect(transformationOrigin.origin.x()-radius, transformationOrigin.origin.y()-radius, radius*2., radius*2.);

    QPainterPath center1;
    center1.addEllipse(rect);

    if (painter != nullptr)
    {
        painter->save();
        painter->setBrush(QBrush(m_color));
        painter->drawPath(Center1());
        painter->restore();
    }
    path.addPath(center1);

    radius = centerRadius2/scale;
    rect = QRectF(transformationOrigin.origin.x()-radius, transformationOrigin.origin.y()-radius, radius*2., radius*2.);

    QPainterPath center2;
    center2.addEllipse(rect);

    if (painter != nullptr)
    {
        painter->save();
        painter->setBrush(QBrush());
        painter->drawPath(Center2());
        painter->restore();
    }
    path.addPath(center2);

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsTransformationOrigin::Center1() const -> QPainterPath
{
    const qreal scale = SceneScale(scene());
    qreal radius = centerRadius1/scale;
    VPTransformationOrigon transformationOrigin = TransformationOrigin(m_layout, QRectF());
    QRectF rect(transformationOrigin.origin.x()-radius, transformationOrigin.origin.y()-radius, radius*2., radius*2.);

    QPainterPath center1;
    center1.addEllipse(rect);

    return center1;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsTransformationOrigin::Center2() const -> QPainterPath
{
    const qreal scale = SceneScale(scene());
    qreal radius = centerRadius2/scale;
    VPTransformationOrigon transformationOrigin = TransformationOrigin(m_layout, QRectF());
    QRectF rect = QRectF(transformationOrigin.origin.x()-radius, transformationOrigin.origin.y()-radius, radius*2.,
                         radius*2.);

    QPainterPath center2;
    center2.addEllipse(rect);

    return center2;
}

// VPGraphicsPieceControls
//---------------------------------------------------------------------------------------------------------------------
VPGraphicsPieceControls::VPGraphicsPieceControls(const VPLayoutPtr &layout, QGraphicsItem *parent)
    : QGraphicsObject(parent),
      m_layout(layout)
{
    SCASSERT(m_layout != nullptr)
    QPixmap cursor_pixmap = QIcon("://puzzleicon/svg/cursor_rotate.svg").pixmap(QSize(32,32));
    setCursor(QCursor(cursor_pixmap, 16, 16));
    setZValue(1);
    setAcceptHoverEvents(true);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPieceControls::on_UpdateControls()
{
    if (m_ignorePieceTransformation)
    {
        return;
    }

    m_selectedPieces = SelectedPieces();
    m_pieceRect = PiecesBoundingRect(m_selectedPieces);
    setVisible(not m_pieceRect.isNull());

    if (not m_pieceRect.isNull())
    {
        VPLayoutPtr layout = m_layout.toStrongRef();
        if (not layout.isNull())
        {
            VPSheetPtr sheet = layout->GetFocusedSheet();
            if (not sheet.isNull())
            {
                VPTransformationOrigon origin = sheet->TransformationOrigin();
                if (not origin.custom)
                {
                    origin.origin = m_pieceRect.center();
                    sheet->SetTransformationOrigin(origin);
                    emit TransformationOriginChanged();
                }
            }
        }
    }

    emit ShowOrigin(not m_pieceRect.isNull());
    prepareGeometryChange();
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPieceControls::on_HideHandles(bool hide)
{
    m_controlsVisible = not hide;
    update();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::boundingRect() const -> QRectF
{
    constexpr qreal halfPenWidth = penWidth/2.;
    return Handles().boundingRect().adjusted(-halfPenWidth, -halfPenWidth, halfPenWidth, halfPenWidth);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::shape() const -> QPainterPath
{
    return Handles();
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPieceControls::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(widget);
    Q_UNUSED(option);

    const qreal scale = SceneScale(scene());

    QPen pen(defaultColor, penWidth/scale, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    painter->setPen(pen);

    if (m_controlsVisible)
    {
        TopLeftControl(painter);
        TopRightControl(painter);
        BottomLeftControl(painter);
        BottomRightControl(painter);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPieceControls::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_rotationStartPoint = event->scenePos();
        m_rotationSum = 0;
        m_controlsVisible = false;
        m_handleCorner = SelectedHandleCorner(event->scenePos());
        m_ignorePieceTransformation = true;
        prepareGeometryChange();
    }
    else
    {
        QGraphicsObject::mousePressEvent(event);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPieceControls::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if((event->modifiers() & Qt::ShiftModifier) != 0U
            && static_cast<HandleCorner>(m_handleCorner) != HandleCorner::Invalid)
    {
        if (not m_originSaved)
        {
            VPLayoutPtr layout = m_layout.toStrongRef();
            if (not layout.isNull())
            {
                VPSheetPtr sheet = layout->GetFocusedSheet();
                if (not sheet.isNull())
                {
                    m_savedOrigin = sheet->TransformationOrigin();
                    m_originSaved = true;
                    m_pieceRect = PiecesBoundingRect(m_selectedPieces);

                    VPTransformationOrigon origin;
                    origin.custom = true;

                    if (static_cast<HandleCorner>(m_handleCorner) == HandleCorner::TopLeft)
                    {
                        origin.origin = m_pieceRect.topLeft();
                    }
                    else if (static_cast<HandleCorner>(m_handleCorner) == HandleCorner::TopRight)
                    {
                        origin.origin = m_pieceRect.topRight();
                    }
                    else if (static_cast<HandleCorner>(m_handleCorner) == HandleCorner::BottomRight)
                    {
                        origin.origin = m_pieceRect.bottomRight();
                    }
                    else if (static_cast<HandleCorner>(m_handleCorner) == HandleCorner::BottomLeft)
                    {
                        origin.origin = m_pieceRect.bottomLeft();
                    }

                    sheet->SetTransformationOrigin(origin);
                    emit TransformationOriginChanged();
                }
            }
        }
    }
    else
    {
        if (m_originSaved)
        {
            VPLayoutPtr layout = m_layout.toStrongRef();
            if (not layout.isNull())
            {
                VPSheetPtr sheet = layout->GetFocusedSheet();
                if (sheet != nullptr)
                {
                    if (not m_savedOrigin.custom)
                    {
                        m_pieceRect = PiecesBoundingRect(m_selectedPieces);
                        m_savedOrigin.origin = m_pieceRect.center();
                    }
                    sheet->SetTransformationOrigin(m_savedOrigin);
                    emit TransformationOriginChanged();
                }
                m_originSaved = false;
            }
        }
    }

    QPointF rotationNewPoint = event->scenePos();

    // get the angle from the center to the initial click point
    VPTransformationOrigon rotationOrigin = TransformationOrigin(m_layout, m_pieceRect);
    QLineF initPosition(rotationOrigin.origin, m_rotationStartPoint);
    QLineF initRotationPosition(rotationOrigin.origin, rotationNewPoint);

    qreal rotateOn = initPosition.angleTo(initRotationPosition);

    if (rotateOn > 180)
    {
        rotateOn = rotateOn - 360.;
    }

    if (not qFuzzyIsNull(rotateOn))
    {
        auto PreparePieces = [this]()
        {
            QList<VPPiecePtr> pieces;
            VPLayoutPtr layout = m_layout.toStrongRef();
            if (not layout.isNull())
            {
                VPSheetPtr sheet = layout->GetFocusedSheet();
                if (not sheet.isNull())
                {
                    pieces = sheet->GetSelectedPieces();
                }
            }

            return pieces;
        };

        QList<VPPiecePtr> pieces = PreparePieces();

        VPLayoutPtr layout = m_layout.toStrongRef();
        if (not layout.isNull())
        {
            if (layout->LayoutSettings().GetFollowGrainline() && not rotationOrigin.custom)
            {
                if (m_rotationSum > 90 || m_rotationSum < -90)
                {
                    m_rotationSum = rotateOn;
                }
                else
                {
                    m_rotationSum += rotateOn;
                }
            }
            else
            {
                m_rotationSum = rotateOn;
            }

            if (pieces.size() == 1)
            {
                auto *command = new VPUndoPieceRotate(pieces.first(), rotationOrigin, rotateOn, m_rotationSum,
                                                      allowChangeMerge);
                layout->UndoStack()->push(command);
            }
            else if (pieces.size() > 1)
            {
                auto *command = new VPUndoPiecesRotate(pieces, rotationOrigin, rotateOn, m_rotationSum,
                                                       allowChangeMerge);
                layout->UndoStack()->push(command);
            }
        }
    }

    if (m_originSaved && m_savedOrigin.custom)
    {
        QLineF line(rotationOrigin.origin, m_savedOrigin.origin);
        line.setAngle(line.angle()+rotateOn);
        m_savedOrigin.origin = line.p2();
    }

    m_rotationStartPoint = rotationNewPoint;
    allowChangeMerge = true;
    QGraphicsObject::mouseMoveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPieceControls::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        m_controlsVisible = true;
        m_ignorePieceTransformation = false;

        if (m_originSaved)
        {
            VPLayoutPtr layout = m_layout.toStrongRef();
            if (not layout.isNull())
            {
                VPSheetPtr sheet = layout->GetFocusedSheet();
                if (not sheet.isNull())
                {
                    if (not m_savedOrigin.custom)
                    {
                        m_pieceRect = PiecesBoundingRect(m_selectedPieces);
                        m_savedOrigin.origin = m_pieceRect.center();
                    }
                    sheet->SetTransformationOrigin(m_savedOrigin);
                    emit TransformationOriginChanged();
                }
                m_originSaved = false;
            }
        }

        on_UpdateControls();
        allowChangeMerge = false;
    }
    QGraphicsObject::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPieceControls::SetIgnorePieceTransformation(bool newIgnorePieceTransformation)
{
    m_ignorePieceTransformation = newIgnorePieceTransformation;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::TopLeftControl(QPainter *painter) const -> QPainterPath
{
    return Controller(QTransform(), painter);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::TopRightControl(QPainter *painter) const -> QPainterPath
{
    QTransform t;
    t.scale(-1, 1);
    t.translate(-(m_pieceRect.topLeft().x() * 2. + m_pieceRect.width()), 0);

    return Controller(t, painter);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::BottomLeftControl(QPainter *painter) const -> QPainterPath
{
    QTransform t;
    t.scale(1, -1);
    t.translate(0, -(m_pieceRect.topLeft().y() * 2. + m_pieceRect.height()));

    return Controller(t, painter);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::BottomRightControl(QPainter *painter) const -> QPainterPath
{
    QTransform t;
    t.scale(-1, -1);
    t.translate(-(m_pieceRect.topLeft().x() * 2. + m_pieceRect.width()),
                -(m_pieceRect.topLeft().y() * 2. + m_pieceRect.height()));

    return Controller(t, painter);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::Handles() const -> QPainterPath
{
    QPainterPath path;

    path.addPath(TopLeftControl());
    path.addPath(TopRightControl());
    path.addPath(BottomLeftControl());
    path.addPath(BottomRightControl());

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::Controller(const QTransform &t, QPainter *painter) const -> QPainterPath
{
    if (painter != nullptr)
    {
        QPen pen = painter->pen();
        pen.setColor(defaultColor);
        painter->setPen(pen);
    }

    QBrush colorBrush(defaultColor);

    QPainterPath controller = ArrowPath();
    controller = t.map(controller);

    if (painter != nullptr)
    {
        painter->save();
        painter->setBrush(colorBrush);
        painter->drawPath(controller);
        painter->restore();
    }

    return controller;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::ControllersRect() const -> QRectF
{
    const qreal scale = SceneScale(scene());

    const qreal gap = 2;
    QRectF rect = m_pieceRect;
    const qreal minWidth = arcRadius/scale+gap;
    const qreal minHeight = arcRadius/scale+gap;

    if (m_pieceRect.width() < minWidth)
    {
        qreal diff = minWidth - m_pieceRect.width();
        rect.adjust(-diff/2., 0, diff/2., 0);
    }

    if (m_pieceRect.height() < minHeight)
    {
        qreal diff = minHeight - m_pieceRect.height();
        rect.adjust(0, -diff/2., 0, diff/2.);
    }

    return rect;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::ArrowPath() const -> QPainterPath
{
    const qreal scale = SceneScale(scene())/2;
    QPainterPath arrow;

    QRectF pieceRect = ControllersRect();

    QLineF start(pieceRect.topLeft().x(), pieceRect.topLeft().y(),
                 pieceRect.topLeft().x(), pieceRect.topLeft().y() - (arcRadius+1)/scale);
    start.setAngle(arcStartAngle);

    arrow.moveTo(start.p2());

    QLineF baseLine(start.p2(), QPointF(start.p2().x()+arrowTail/scale, start.p2().y()));
    baseLine.setAngle(arrow1Angle);
    arrow.lineTo(baseLine.p2());

    QLineF leftSide = QLineF(baseLine.p2(), baseLine.p1());
    leftSide.setLength(arrowSide/scale);
    leftSide.setAngle(leftSide.angle()-90);
    arrow.lineTo(leftSide.p2());

    start = QLineF(pieceRect.topLeft().x(), pieceRect.topLeft().y(),
                   pieceRect.topLeft().x(), pieceRect.topLeft().y() - arcRadius/scale);
    start.setAngle(arcStartAngle);

    baseLine = QLineF(start.p2(), QPointF(start.p2().x()+(arrowTail+arrowLength)/scale, start.p2().y()));
    baseLine.setAngle(arrow1Angle);
    arrow.lineTo(baseLine.p2());

    start = QLineF(pieceRect.topLeft().x(), pieceRect.topLeft().y(),
                   pieceRect.topLeft().x(), pieceRect.topLeft().y() - (arcRadius-1)/scale);
    start.setAngle(arcStartAngle);

    baseLine = QLineF(start.p2(), QPointF(start.p2().x()+arrowTail/scale, start.p2().y()));
    baseLine.setAngle(arrow1Angle);

    QLineF rightSide = QLineF(baseLine.p2(), baseLine.p1());
    rightSide.setLength(arrowSide/scale);
    rightSide.setAngle(rightSide.angle()+90);
    arrow.lineTo(rightSide.p2());

    arrow.lineTo(baseLine.p2());
    arrow.lineTo(start.p2());

    // arc 1
    QRectF arc1Rect(pieceRect.topLeft().x()-(arcRadius-1)/scale, pieceRect.topLeft().y()-(arcRadius-1)/scale,
                    (arcRadius-1)/scale*2, (arcRadius-1)/scale*2);
    arrow.arcTo(arc1Rect, arcStartAngle, arcAngle);

    // arrow 2

    start = QLineF(pieceRect.topLeft().x(), pieceRect.topLeft().y(),
                   pieceRect.topLeft().x(), pieceRect.topLeft().y() - (arcRadius-1)/scale);
    start.setAngle(arcStartAngle+arcAngle);

    baseLine = QLineF(start.p2(), QPointF(start.p2().x()+arrowTail/scale, start.p2().y()));
    baseLine.setAngle(arrow2Angle+180);
    arrow.lineTo(baseLine.p2());

    leftSide = QLineF(baseLine.p2(), baseLine.p1());
    leftSide.setLength(arrowSide/scale);
    leftSide.setAngle(leftSide.angle()-90);
    arrow.lineTo(leftSide.p2());

    start = QLineF(pieceRect.topLeft().x(), pieceRect.topLeft().y(),
                   pieceRect.topLeft().x(), pieceRect.topLeft().y() - arcRadius/scale);
    start.setAngle(arcStartAngle+arcAngle);

    baseLine = QLineF(start.p2(), QPointF(start.p2().x()+(arrowTail+arrowLength)/scale, start.p2().y()));
    baseLine.setAngle(arrow2Angle+180);
    arrow.lineTo(baseLine.p2());

    start = QLineF(pieceRect.topLeft().x(), pieceRect.topLeft().y(),
                   pieceRect.topLeft().x(), pieceRect.topLeft().y() - (arcRadius+1)/scale);
    start.setAngle(arcStartAngle+arcAngle);

    baseLine = QLineF(start.p2(), QPointF(start.p2().x()+arrowTail/scale, start.p2().y()));
    baseLine.setAngle(arrow2Angle+180);

    rightSide = QLineF(baseLine.p2(), baseLine.p1());
    rightSide.setLength(arrowSide/scale);
    rightSide.setAngle(rightSide.angle()+90);
    arrow.lineTo(rightSide.p2());

    arrow.lineTo(baseLine.p2());
    arrow.lineTo(start.p2());

    // arc 2
    QRectF arc2Rect(pieceRect.topLeft().x()-(arcRadius+1)/scale, pieceRect.topLeft().y()-(arcRadius+1)/scale,
                    (arcRadius+1)/scale*2, (arcRadius+1)/scale*2);
    QPainterPath arc;
    start = QLineF(pieceRect.topLeft().x(), pieceRect.topLeft().y(),
                 pieceRect.topLeft().x(), pieceRect.topLeft().y() - (arcRadius+1)/scale);
    start.setAngle(arcStartAngle);

    arc.moveTo(start.p2());
    arc.arcTo(arc2Rect, arcStartAngle, arcAngle);
    arrow.addPath(arc.toReversed());

    return arrow;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::SelectedPieces() const -> QList<VPPiecePtr>
{
    QList<VPPiecePtr> pieces;

    VPLayoutPtr layout = m_layout.toStrongRef();
    if (not layout.isNull())
    {
        VPSheetPtr sheet = layout->GetFocusedSheet();
        if (not sheet.isNull())
        {
            pieces = sheet->GetSelectedPieces();
        }
    }

    return pieces;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::PiecesBoundingRect(const QList<VPPiecePtr> &selectedPieces) -> QRectF
{
    QRectF rect;
    for (const auto& item : selectedPieces)
    {
        if (not item.isNull())
        {
            rect = rect.united(item->MappedDetailBoundingRect());
        }
    }

    return rect;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::SelectedHandleCorner(const QPointF &pos) const -> int
{
    if (TopLeftControl().boundingRect().contains(pos))
    {
        return static_cast<int>(HandleCorner::BottomRight);
    }

    if (TopRightControl().boundingRect().contains(pos))
    {
        return static_cast<int>(HandleCorner::BottomLeft);
    }

    if (BottomLeftControl().boundingRect().contains(pos))
    {
        return static_cast<int>(HandleCorner::TopRight);
    }

    if (BottomRightControl().boundingRect().contains(pos))
    {
        return static_cast<int>(HandleCorner::TopLeft);
    }

    return static_cast<int>(HandleCorner::Invalid);
}
