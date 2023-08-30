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
#include <QFileInfo>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QGuiApplication>
#include <QIcon>
#include <QPainter>
#include <QScreen>
#include <QtDebug>

#include "../layout/vplayout.h"
#include "../layout/vppiece.h"
#include "../layout/vpsheet.h"
#include "../undocommands/vpundooriginmove.h"
#include "../undocommands/vpundopiecerotate.h"
#include "../vmisc/theme/vscenestylesheet.h"
#include "../vmisc/theme/vtheme.h"
#include "../vwidgets/global.h"

namespace
{
constexpr qreal penWidth = 2;

const qreal centerRadius1 = 5;
const qreal centerRadius2 = 10;

//---------------------------------------------------------------------------------------------------------------------
auto TransformationOrigin(const VPLayoutPtr &layout, const QRectF &boundingRect) -> VPTransformationOrigon
{
    SCASSERT(layout != nullptr)
    VPSheetPtr sheet = layout->GetFocusedSheet();
    if (not sheet.isNull())
    {
        return sheet->TransformationOrigin();
    }

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    VPTransformationOrigon origin;
    origin.origin = boundingRect.center();
    origin.custom = false;

    // cppcheck-suppress unknownMacro
    QT_WARNING_POP

    return origin;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VPGraphicsTransformationOrigin::VPGraphicsTransformationOrigin(const VPLayoutPtr &layout, QGraphicsItem *parent)
  : QGraphicsObject(parent),
    m_layout(layout)
{
    SCASSERT(m_layout != nullptr)
    setCursor(Qt::OpenHandCursor);
    setZValue(1000);
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
    constexpr qreal halfPenWidth = penWidth / 2.;
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

    QPen pen(CurrentColor(), penWidth / scale, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);

    painter->setPen(pen);

    if (m_originVisible)
    {
        painter->save();
        painter->setBrush(QBrush(CurrentColor()));
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
    if ((event->button() == Qt::LeftButton))
    {
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    }
    else
    {
        // perform the default behaviour
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
    // perform the default behaviour
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
    m_hoverMode = true;
    QGraphicsObject::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsTransformationOrigin::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_hoverMode = false;
    QGraphicsObject::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsTransformationOrigin::RotationCenter(QPainter *painter) const -> QPainterPath
{
    QPainterPath path;

    const qreal scale = SceneScale(scene());
    qreal radius = centerRadius1 / scale;
    VPTransformationOrigon transformationOrigin = TransformationOrigin(m_layout, QRectF());
    QRectF rect(transformationOrigin.origin.x() - radius, transformationOrigin.origin.y() - radius, radius * 2.,
                radius * 2.);

    QPainterPath center1;
    center1.addEllipse(rect);

    if (painter != nullptr)
    {
        painter->save();
        painter->setBrush(QBrush(CurrentColor()));
        painter->drawPath(Center1());
        painter->restore();
    }
    path.addPath(center1);

    radius = centerRadius2 / scale;
    rect = QRectF(transformationOrigin.origin.x() - radius, transformationOrigin.origin.y() - radius, radius * 2.,
                  radius * 2.);

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
    qreal radius = centerRadius1 / scale;
    VPTransformationOrigon transformationOrigin = TransformationOrigin(m_layout, QRectF());
    QRectF rect(transformationOrigin.origin.x() - radius, transformationOrigin.origin.y() - radius, radius * 2.,
                radius * 2.);

    QPainterPath center1;
    center1.addEllipse(rect);

    return center1;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsTransformationOrigin::Center2() const -> QPainterPath
{
    const qreal scale = SceneScale(scene());
    qreal radius = centerRadius2 / scale;
    VPTransformationOrigon transformationOrigin = TransformationOrigin(m_layout, QRectF());
    QRectF rect = QRectF(transformationOrigin.origin.x() - radius, transformationOrigin.origin.y() - radius,
                         radius * 2., radius * 2.);

    QPainterPath center2;
    center2.addEllipse(rect);

    return center2;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsTransformationOrigin::CurrentColor() const -> QColor
{
    return m_hoverMode ? VSceneStylesheet::ManualLayoutStyle().PieceHandleHoverColor()
                       : VSceneStylesheet::ManualLayoutStyle().PieceHandleColor();
}

// VPGraphicsPieceControls
//---------------------------------------------------------------------------------------------------------------------
VPGraphicsPieceControls::VPGraphicsPieceControls(const VPLayoutPtr &layout, QGraphicsItem *parent)
  : QGraphicsObject(parent),
    m_layout(layout)
{
    SCASSERT(m_layout != nullptr)
    setCursor(QCursor(Qt::OpenHandCursor));
    setZValue(100);
    setFlag(QGraphicsItem::ItemIgnoresTransformations);
    setAcceptHoverEvents(true);

    InitPixmaps();
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
    m_controlsVisible = not m_pieceRect.isNull();

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
    QRectF boundingRect;

    auto HandlerBoundingRect = [this, &boundingRect](VPHandleCorner corner, VPHandleCornerType type, QPointF pos)
    {
        QPixmap handler = HandlerPixmap(m_handleCorner == corner, type);
        boundingRect = boundingRect.united(QRectF(pos, handler.size() / handler.devicePixelRatio()));
    };

    HandlerBoundingRect(VPHandleCorner::TopLeft, VPHandleCornerType::TopLeft, TopLeftHandlerPosition());
    HandlerBoundingRect(VPHandleCorner::TopRight, VPHandleCornerType::TopRight, TopRightHandlerPosition());
    HandlerBoundingRect(VPHandleCorner::BottomRight, VPHandleCornerType::BottomRight, BottomRightHandlerPosition());
    HandlerBoundingRect(VPHandleCorner::BottomLeft, VPHandleCornerType::BottomLeft, BottomLeftHandlerPosition());

    return boundingRect;
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

    if (m_controlsVisible)
    {
        painter->drawPixmap(TopLeftHandlerPosition(),
                            HandlerPixmap(m_handleCorner == VPHandleCorner::TopLeft, VPHandleCornerType::TopLeft));

        painter->drawPixmap(TopRightHandlerPosition(),
                            HandlerPixmap(m_handleCorner == VPHandleCorner::TopRight, VPHandleCornerType::TopRight));

        painter->drawPixmap(BottomRightHandlerPosition(), HandlerPixmap(m_handleCorner == VPHandleCorner::BottomRight,
                                                                        VPHandleCornerType::BottomRight));

        painter->drawPixmap(BottomLeftHandlerPosition(), HandlerPixmap(m_handleCorner == VPHandleCorner::BottomLeft,
                                                                       VPHandleCornerType::BottomLeft));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPieceControls::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
    {
        m_rotationStartPoint = event->scenePos();
        m_rotationSum = 0;
        m_controlsVisible = false;
        m_handleCorner = SelectedHandleCorner(event->pos());
        m_ignorePieceTransformation = true;
        prepareGeometryChange();

        if (m_handleCorner != VPHandleCorner::Invalid)
        {
            setCursor(QCursor(Qt::ClosedHandCursor));
            event->accept();
        }
        else
        {
            QGraphicsView *view = ItemView();
            if (view != nullptr)
            {
                setCursor(view->viewport()->cursor()); // clazy:exclude=clazy-qt6-deprecated-api-fixes
            }
            event->ignore();
        }
    }
    else
    {
        QGraphicsObject::mousePressEvent(event);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPieceControls::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    PrepareTransformationOrigin(event->modifiers() & Qt::ShiftModifier);

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
        QList<VPPiecePtr> pieces = SelectedPieces();

        VPLayoutPtr layout = m_layout.toStrongRef();
        if (not layout.isNull())
        {
            CorrectRotationSum(layout, rotationOrigin, rotateOn);

            if (pieces.size() == 1)
            {
                auto *command = new VPUndoPieceRotate(pieces.constFirst(), rotationOrigin, rotateOn, m_rotationSum,
                                                      allowChangeMerge);
                layout->UndoStack()->push(command);
            }
            else if (pieces.size() > 1)
            {
                auto *command =
                    new VPUndoPiecesRotate(pieces, rotationOrigin, rotateOn, m_rotationSum, allowChangeMerge);
                layout->UndoStack()->push(command);
            }
        }
    }

    if (m_originSaved && m_savedOrigin.custom)
    {
        QLineF line(rotationOrigin.origin, m_savedOrigin.origin);
        line.setAngle(line.angle() + rotateOn);
        m_savedOrigin.origin = line.p2();
    }

    m_rotationStartPoint = rotationNewPoint;
    allowChangeMerge = true;
    QGraphicsObject::mouseMoveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPieceControls::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_controlsVisible = true;
        m_ignorePieceTransformation = false;

        if (SelectedHandleCorner(event->pos()) != VPHandleCorner::Invalid)
        {
            setCursor(QCursor(Qt::OpenHandCursor));
        }
        else
        {
            QGraphicsView *view = ItemView();
            if (view != nullptr)
            {
                setCursor(view->viewport()->cursor()); // clazy:exclude=clazy-qt6-deprecated-api-fixes
            }
        }

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
void VPGraphicsPieceControls::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    UpdateCursor(SelectedHandleCorner(event->pos()));
    QGraphicsObject::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPieceControls::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    UpdateCursor(SelectedHandleCorner(event->pos()));
    QGraphicsObject::hoverMoveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPieceControls::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    UpdateCursor(VPHandleCorner::Invalid);
    QGraphicsObject::hoverLeaveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPieceControls::InitPixmaps()
{
    m_handlePixmaps.clear();
    m_handleHoverPixmaps.clear();
    m_handlePaths.clear();

    auto InitPixmap = [this](VPHandleCornerType type, const QString &imageName)
    {
        const QString resource = QStringLiteral("icon");

        const QString fileName = QStringLiteral("32x32/%1.png").arg(imageName);
        QPixmap handlePixmap = VTheme::GetPixmapResource(resource, fileName);

        if (QGuiApplication::primaryScreen()->devicePixelRatio() >= 2)
        {
            const QString fileName2x = QStringLiteral("32x32/%1@2x.png").arg(imageName);
            const QString fileName2xHover = QStringLiteral("32x32/%1-hover@2x.png").arg(imageName);

            m_handlePixmaps.insert(type, VTheme::GetPixmapResource(resource, fileName2x));
            m_handleHoverPixmaps.insert(type, VTheme::GetPixmapResource(resource, fileName2xHover));
        }
        else
        {
            const QString fileNameHover = QStringLiteral("32x32/%1-hover.png").arg(imageName);

            m_handlePixmaps.insert(type, handlePixmap);
            m_handleHoverPixmaps.insert(type, VTheme::GetPixmapResource(resource, fileNameHover));
        }

        QPainterPath p = PixmapToPainterPath(handlePixmap);
        p.setFillRule(Qt::WindingFill);
        p.closeSubpath();
        m_handlePaths.insert(type, p);
    };

    InitPixmap(VPHandleCornerType::TopLeft, QStringLiteral("rotate-top-left"));
    InitPixmap(VPHandleCornerType::TopRight, QStringLiteral("rotate-top-right"));
    InitPixmap(VPHandleCornerType::BottomRight, QStringLiteral("rotate-bottom-right"));
    InitPixmap(VPHandleCornerType::BottomLeft, QStringLiteral("rotate-bottom-left"));
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::TopLeftHandlerPosition() const -> QPointF
{
    return ControllersRect().topLeft();
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::TopRightHandlerPosition() const -> QPointF
{
    QRectF rect = ControllersRect();
    QPixmap handler = m_handlePixmaps.value(VPHandleCornerType::TopRight);
    QSize size = handler.size() / handler.devicePixelRatio();
    return {rect.topLeft().x() + (rect.width() - size.width()), rect.topLeft().y()};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::BottomRightHandlerPosition() const -> QPointF
{
    QRectF rect = ControllersRect();
    QPixmap handler = m_handlePixmaps.value(VPHandleCornerType::BottomRight);
    QSize size = handler.size() / handler.devicePixelRatio();
    return {rect.topLeft().x() + (rect.width() - size.width()), rect.topLeft().y() + (rect.height() - size.height())};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::BottomLeftHandlerPosition() const -> QPointF
{
    QRectF rect = ControllersRect();
    QPixmap handler = m_handlePixmaps.value(VPHandleCornerType::BottomLeft);
    QSize size = handler.size() / handler.devicePixelRatio();
    return {rect.topLeft().x(), rect.topLeft().y() + (rect.height() - size.height())};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::ControllerPath(VPHandleCornerType type, QPointF pos) const -> QPainterPath
{
    QTransform t;
    t.translate(pos.x(), pos.y());

    QPainterPath controller = m_handlePaths.value(type);

    controller = t.map(controller);

    return controller;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::TopLeftControl() const -> QPainterPath
{
    return ControllerPath(VPHandleCornerType::TopLeft, TopLeftHandlerPosition());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::TopRightControl() const -> QPainterPath
{
    return ControllerPath(VPHandleCornerType::TopRight, TopRightHandlerPosition());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::BottomRightControl() const -> QPainterPath
{
    return ControllerPath(VPHandleCornerType::BottomRight, BottomRightHandlerPosition());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::BottomLeftControl() const -> QPainterPath
{
    return ControllerPath(VPHandleCornerType::BottomLeft, BottomLeftHandlerPosition());
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPieceControls::SetIgnorePieceTransformation(bool newIgnorePieceTransformation)
{
    m_ignorePieceTransformation = newIgnorePieceTransformation;
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
auto VPGraphicsPieceControls::ControllersRect() const -> QRectF
{
    const qreal scale = SceneScale(scene());
    QPixmap handler = m_handlePixmaps.value(VPHandleCornerType::TopLeft);
    QRectF pieceRect = m_pieceRect;

    pieceRect = QRectF(pieceRect.topLeft() * scale, QSizeF(pieceRect.width() * scale, pieceRect.height() * scale));
    QRectF rect = pieceRect;

    if (pieceRect.width() < handler.width())
    {
        qreal diff = handler.width() - pieceRect.width();
        rect.adjust(0, 0, diff, 0);
    }

    if (pieceRect.height() < handler.height())
    {
        qreal diff = handler.height() - pieceRect.height();
        rect.adjust(0, 0, 0, diff);
    }

    const qreal gap = 2;
    rect.adjust(-(handler.width() + gap), -(handler.height() + gap), handler.width() + gap, handler.height() + gap);

    return rect;
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
    for (const auto &item : selectedPieces)
    {
        if (not item.isNull())
        {
            rect = rect.united(item->MappedDetailBoundingRect());
        }
    }

    return rect;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::ItemView() -> QGraphicsView *
{
    QGraphicsScene *scene = this->scene();
    if (scene != nullptr)
    {
        QList<QGraphicsView *> views = scene->views();
        if (not views.isEmpty())
        {
            return views.at(0);
        }
    }

    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPieceControls::UpdateCursor(VPHandleCorner corner)
{
    m_handleCorner = corner;

    if (m_handleCorner != VPHandleCorner::Invalid)
    {
        setCursor(QCursor(Qt::OpenHandCursor));
    }
    else
    {
        QGraphicsView *view = ItemView();
        if (view != nullptr)
        {
            setCursor(view->viewport()->cursor()); // clazy:exclude=clazy-qt6-deprecated-api-fixes
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPieceControls::PrepareTransformationOrigin(bool shiftPressed)
{
    if (shiftPressed && m_handleCorner != VPHandleCorner::Invalid)
    {
        if (m_originSaved)
        {
            return;
        }

        VPLayoutPtr layout = m_layout.toStrongRef();
        if (layout.isNull())
        {
            return;
        }

        VPSheetPtr sheet = layout->GetFocusedSheet();
        if (sheet.isNull())
        {
            return;
        }

        m_savedOrigin = sheet->TransformationOrigin();
        m_originSaved = true;
        m_pieceRect = PiecesBoundingRect(m_selectedPieces);

        VPTransformationOrigon origin;
        origin.custom = true;

        switch (m_handleCorner)
        {
            case VPHandleCorner::TopLeft:
                origin.origin = m_pieceRect.bottomRight();
                break;
            case VPHandleCorner::TopRight:
                origin.origin = m_pieceRect.bottomLeft();
                break;
            case VPHandleCorner::BottomRight:
                origin.origin = m_pieceRect.topLeft();
                break;
            case VPHandleCorner::BottomLeft:
                origin.origin = m_pieceRect.topRight();
                break;
            case VPHandleCorner::Invalid:
            default:
                break;
        }

        sheet->SetTransformationOrigin(origin);
        emit TransformationOriginChanged();
    }
    else
    {
        if (not m_originSaved)
        {
            return;
        }

        VPLayoutPtr layout = m_layout.toStrongRef();
        if (layout.isNull())
        {
            return;
        }

        VPSheetPtr sheet = layout->GetFocusedSheet();
        if (sheet.isNull())
        {
            return;
        }

        if (not m_savedOrigin.custom)
        {
            m_pieceRect = PiecesBoundingRect(m_selectedPieces);
            m_savedOrigin.origin = m_pieceRect.center();
        }
        sheet->SetTransformationOrigin(m_savedOrigin);
        emit TransformationOriginChanged();
        m_originSaved = false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPGraphicsPieceControls::CorrectRotationSum(const VPLayoutPtr &layout,
                                                 const VPTransformationOrigon &rotationOrigin, qreal rotateOn)
{
    if (layout.isNull())
    {
        return;
    }

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
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::SelectedHandleCorner(const QPointF &pos) const -> VPHandleCorner
{
    QMap<VPHandleCorner, QPainterPath> corners{
        {VPHandleCorner::TopLeft, TopLeftControl()},
        {VPHandleCorner::TopRight, TopRightControl()},
        {VPHandleCorner::BottomRight, BottomRightControl()},
        {VPHandleCorner::BottomLeft, BottomLeftControl()},
    };

    QPainterPath circle;
    circle.addEllipse(pos.x() - 4, pos.y() - 4, 8, 8);

    auto CheckCorner = [circle](const QPainterPath &handler)
    { return handler.intersects(circle) || handler.contains(circle); };

    auto i = corners.constBegin();
    while (i != corners.constEnd())
    {
        if (CheckCorner(i.value()))
        {
            return i.key();
        }
        ++i;
    }

    return VPHandleCorner::Invalid;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPGraphicsPieceControls::HandlerPixmap(bool hover, VPHandleCornerType type) const -> QPixmap
{
    return hover ? m_handleHoverPixmaps.value(type) : m_handlePixmaps.value(type);
}
