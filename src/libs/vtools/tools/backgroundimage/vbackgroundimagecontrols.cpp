/************************************************************************
 **
 **  @file   vbackgroundimagecontrols.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   17 1, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Valentina project
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
#include "vbackgroundimagecontrols.h"

#include <QBitmap>
#include <QFileInfo>
#include <QGraphicsSceneHoverEvent>
#include <QGuiApplication>
#include <QPainter>
#include <QPixmap>
#include <QScreen>
#include <QUndoStack>
#include <QtMath>

#include "../../undocommands/image/rotatebackgroundimage.h"
#include "../../undocommands/image/scalebackgroundimage.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/theme/vtheme.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vwidgets/global.h"
#include "../vwidgets/vmaingraphicsview.h"

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto RectTopPoint(const QRectF &rect) -> QPointF
{
    QLineF edge(rect.topLeft(), rect.topRight());
    edge.setLength(edge.length() / 2.);
    return edge.p2();
}

//---------------------------------------------------------------------------------------------------------------------
auto RectRightPoint(const QRectF &rect) -> QPointF
{
    QLineF edge(rect.topRight(), rect.bottomRight());
    edge.setLength(edge.length() / 2.);
    return edge.p2();
}

//---------------------------------------------------------------------------------------------------------------------
auto RectBottomPoint(const QRectF &rect) -> QPointF
{
    QLineF edge(rect.bottomLeft(), rect.bottomRight());
    edge.setLength(edge.length() / 2.);
    return edge.p2();
}

//---------------------------------------------------------------------------------------------------------------------
auto RectLeftPoint(const QRectF &rect) -> QPointF
{
    QLineF edge(rect.bottomLeft(), rect.topLeft());
    edge.setLength(edge.length() / 2.);
    return edge.p2();
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VBackgroundImageControls::VBackgroundImageControls(VAbstractPattern *doc, QGraphicsItem *parent)
  : QGraphicsObject(parent),
    m_doc(doc)
{
    SCASSERT(doc != nullptr)
    setVisible(false);
    setFlag(QGraphicsItem::ItemIgnoresTransformations);
    setZValue(100);
    setAcceptHoverEvents(true);
    SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);

    InitPixmaps();

    QGuiApplication *guiApp = qGuiApp;
    if (guiApp != nullptr)
    {
        connect(guiApp, &QGuiApplication::primaryScreenChanged, this, &VBackgroundImageControls::ScreenChanged);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageControls::ActivateControls(const QUuid &id)
{
    if (id.isNull())
    {
        m_id = id;
        emit ActiveImageChanged(m_id);
        setVisible(false);
    }
    else if (m_id == id)
    {
        prepareGeometryChange();
        if (m_tranformationType == BITransformationType::Scale)
        {
            m_tranformationType = BITransformationType::Rotate;
            ShowOrigin(m_image.BoundingRect().center());
        }
        else if (m_tranformationType == BITransformationType::Rotate)
        {
            m_tranformationType = BITransformationType::Scale;
            m_showOrigin = false;
        }
    }
    else
    {
        prepareGeometryChange();
        m_id = id;
        m_image = m_doc->GetBackgroundImage(m_id);
        m_tranformationType = BITransformationType::Scale;
        m_showOrigin = false;
        emit ActiveImageChanged(m_id);
        setVisible(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageControls::DeactivateControls(QGraphicsItem *item)
{
    if (m_transformationApplied)
    {
        m_transformationApplied = false;
        return;
    }

    if (item == nullptr)
    {
        setVisible(false);
        m_id = QUuid();
        emit ActiveImageChanged(m_id);
        return;
    }

    if (item->type() == type() || item->type() == UserType + static_cast<int>(Tool::BackgroundPixmapImage) ||
        item->type() == UserType + static_cast<int>(Tool::BackgroundSVGImage))
    {
        return;
    }

    setVisible(false);
    m_id = QUuid();
    emit ActiveImageChanged(m_id);
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageControls::UpdateControls()
{
    if (not isVisible() || m_id.isNull() || m_tranformationType == BITransformationType::Unknown)
    {
        return;
    }

    prepareGeometryChange();
    m_image = m_doc->GetBackgroundImage(m_id);
    if (not m_customOrigin)
    {
        m_originPos = m_image.BoundingRect().center();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::boundingRect() const -> QRectF
{
    QRectF boundingRect;

    auto HandlerBoundingRect = [this, &boundingRect](BIHandleCorner corner, BIHandleCornerType type, QPointF pos)
    {
        QPixmap handler = HandlerPixmap(m_handleCornerHover == corner, type);
        boundingRect = boundingRect.united(QRectF(pos, handler.size() / handler.devicePixelRatio()));
    };

    if (m_tranformationType == BITransformationType::Scale)
    {
        HandlerBoundingRect(BIHandleCorner::TopLeft, BIHandleCornerType::ScaleTopLeftBottomRight,
                            TopLeftHandlerPosition());
        HandlerBoundingRect(BIHandleCorner::Top, BIHandleCornerType::ScaleTopBottom, TopHandlerPosition());
        HandlerBoundingRect(BIHandleCorner::TopRight, BIHandleCornerType::ScaleTopRightBottomLeft,
                            TopRightHandlerPosition());
        HandlerBoundingRect(BIHandleCorner::Right, BIHandleCornerType::ScaleRightLeft, RightHandlerPosition());
        HandlerBoundingRect(BIHandleCorner::BottomRight, BIHandleCornerType::ScaleTopLeftBottomRight,
                            BottomRightHandlerPosition());
        HandlerBoundingRect(BIHandleCorner::Bottom, BIHandleCornerType::ScaleTopBottom, BottomHandlerPosition());
        HandlerBoundingRect(BIHandleCorner::BottomLeft, BIHandleCornerType::ScaleTopRightBottomLeft,
                            BottomLeftHandlerPosition());
        HandlerBoundingRect(BIHandleCorner::Left, BIHandleCornerType::ScaleRightLeft, LeftHandlerPosition());
    }
    else if (m_tranformationType == BITransformationType::Rotate)
    {
        HandlerBoundingRect(BIHandleCorner::TopLeft, BIHandleCornerType::RotateTopLeft, TopLeftHandlerPosition());
        HandlerBoundingRect(BIHandleCorner::TopRight, BIHandleCornerType::RotateTopRight, TopRightHandlerPosition());
        HandlerBoundingRect(BIHandleCorner::BottomRight, BIHandleCornerType::RotateBottomRight,
                            BottomRightHandlerPosition());
        HandlerBoundingRect(BIHandleCorner::BottomLeft, BIHandleCornerType::RotateBottomLeft,
                            BottomLeftHandlerPosition());
    }

    boundingRect = boundingRect.united(OriginPath().boundingRect());

    return boundingRect;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::shape() const -> QPainterPath
{
    QPainterPath shape;
    shape.addPath(Handles());
    shape.addPath(OriginPath());
    return shape;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageControls::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (m_controlsVisible)
    {
        if (m_tranformationType == BITransformationType::Scale)
        {
            painter->drawPixmap(TopLeftHandlerPosition(), HandlerPixmap(m_handleCornerHover == BIHandleCorner::TopLeft,
                                                                        BIHandleCornerType::ScaleTopLeftBottomRight));
            painter->drawPixmap(TopHandlerPosition(), HandlerPixmap(m_handleCornerHover == BIHandleCorner::Top,
                                                                    BIHandleCornerType::ScaleTopBottom));
            painter->drawPixmap(TopRightHandlerPosition(),
                                HandlerPixmap(m_handleCornerHover == BIHandleCorner::TopRight,
                                              BIHandleCornerType::ScaleTopRightBottomLeft));
            painter->drawPixmap(RightHandlerPosition(), HandlerPixmap(m_handleCornerHover == BIHandleCorner::Right,
                                                                      BIHandleCornerType::ScaleRightLeft));
            painter->drawPixmap(BottomRightHandlerPosition(),
                                HandlerPixmap(m_handleCornerHover == BIHandleCorner::BottomRight,
                                              BIHandleCornerType::ScaleTopLeftBottomRight));
            painter->drawPixmap(BottomHandlerPosition(), HandlerPixmap(m_handleCornerHover == BIHandleCorner::Bottom,
                                                                       BIHandleCornerType::ScaleTopBottom));
            painter->drawPixmap(BottomLeftHandlerPosition(),
                                HandlerPixmap(m_handleCornerHover == BIHandleCorner::BottomLeft,
                                              BIHandleCornerType::ScaleTopRightBottomLeft));
            painter->drawPixmap(LeftHandlerPosition(), HandlerPixmap(m_handleCornerHover == BIHandleCorner::Left,
                                                                     BIHandleCornerType::ScaleRightLeft));
        }
        else if (m_tranformationType == BITransformationType::Rotate)
        {
            painter->drawPixmap(TopLeftHandlerPosition(), HandlerPixmap(m_handleCornerHover == BIHandleCorner::TopLeft,
                                                                        BIHandleCornerType::RotateTopLeft));

            painter->drawPixmap(
                TopRightHandlerPosition(),
                HandlerPixmap(m_handleCornerHover == BIHandleCorner::TopRight, BIHandleCornerType::RotateTopRight));

            painter->drawPixmap(BottomRightHandlerPosition(),
                                HandlerPixmap(m_handleCornerHover == BIHandleCorner::BottomRight,
                                              BIHandleCornerType::RotateBottomRight));

            painter->drawPixmap(
                BottomLeftHandlerPosition(),
                HandlerPixmap(m_handleCornerHover == BIHandleCorner::BottomLeft, BIHandleCornerType::RotateBottomLeft));
        }
    }

    const qreal sceneScale = SceneScale(scene());
    painter->save();
    QPen pen = painter->pen();
    pen.setStyle(Qt::DashLine);
    painter->setPen(pen);

    QRectF rect = m_image.BoundingRect();
    rect = QRectF(rect.topLeft() * sceneScale, rect.bottomRight() * sceneScale);

    painter->drawRect(rect.adjusted(-pen.width(), -pen.width(), pen.width(), pen.width()));
    painter->restore();

    if (m_showOrigin)
    {
        painter->save();
        painter->setBrush(pen.brush());
        painter->drawPath(OriginCircle1());
        painter->restore();

        painter->save();
        painter->setBrush(QBrush());
        painter->drawPath(OriginCircle2());
        painter->restore();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageControls::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
    {
        m_transformationApplied = true;
        m_controlsVisible = false;
        m_handleCornerHover = SelectedHandleCorner(event->pos());
        m_rotationStartPoint = event->pos();

        if (m_handleCornerHover != BIHandleCorner::Invalid)
        {
            if (not m_image.Hold())
            {
                SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
            }
            else
            {
                setCursor(VAbstractValApplication::VApp()->getSceneView()->viewport()->cursor());
            }
        }
        else
        {
            setCursor(VAbstractValApplication::VApp()->getSceneView()->viewport()->cursor());
        }

        const qreal sceneScale = SceneScale(scene());
        m_imageBoundingRect = m_image.BoundingRect();
        m_imageScreenBoundingRect =
            QRectF(m_imageBoundingRect.topLeft() * sceneScale,
                   QSizeF(m_imageBoundingRect.width() * sceneScale, m_imageBoundingRect.height() * sceneScale));

        m_originalMatrix = m_image.Matrix();

        switch (m_handleCornerHover)
        {
            case BIHandleCorner::TopLeft:
                m_scaleDiff = event->pos() - m_imageScreenBoundingRect.topLeft();
                break;
            case BIHandleCorner::Top:
                m_scaleDiff = event->pos() - RectTopPoint(m_imageScreenBoundingRect);
                break;
            case BIHandleCorner::TopRight:
                m_scaleDiff = event->pos() - m_imageScreenBoundingRect.topRight();
                break;
            case BIHandleCorner::Right:
                m_scaleDiff = event->pos() - RectRightPoint(m_imageScreenBoundingRect);
                break;
            case BIHandleCorner::BottomRight:
                m_scaleDiff = event->pos() - m_imageScreenBoundingRect.bottomRight();
                break;
            case BIHandleCorner::Bottom:
                m_scaleDiff = event->pos() - RectBottomPoint(m_imageScreenBoundingRect);
                break;
            case BIHandleCorner::BottomLeft:
                m_scaleDiff = event->pos() - m_imageScreenBoundingRect.bottomLeft();
                break;
            case BIHandleCorner::Left:
                m_scaleDiff = event->pos() - RectLeftPoint(m_imageScreenBoundingRect);
                break;
            case BIHandleCorner::Invalid:
            default:
                event->ignore();
                break;
        }

        event->accept();
    }
    else
    {
        QGraphicsObject::mousePressEvent(event);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageControls::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    m_customOrigin = true;

    if (m_tranformationType == BITransformationType::Scale)
    {
        ScaleImage(event);
    }
    else if (m_tranformationType == BITransformationType::Rotate)
    {
        RotateImage(event);
    }
    QGraphicsObject::mouseMoveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageControls::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        if (SelectedHandleCorner(event->pos()) != BIHandleCorner::Invalid)
        {
            if (not m_image.Hold())
            {
                SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
            }
            else
            {
                setCursor(VAbstractValApplication::VApp()->getSceneView()->viewport()->cursor());
            }
        }
        else
        {
            setCursor(VAbstractValApplication::VApp()->getSceneView()->viewport()->cursor());
        }

        m_customOrigin = false;
        m_controlsVisible = true;
        m_allowChangeMerge = false;
        m_originSaved = false;

        if (m_tranformationType == BITransformationType::Scale)
        {
            m_showOrigin = false;
        }
        else if (m_tranformationType == BITransformationType::Rotate)
        {
            ShowOrigin(m_image.BoundingRect().center());
        }

        update();
    }
    else
    {
        QGraphicsObject::mouseReleaseEvent(event);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageControls::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    UpdateCursor(SelectedHandleCorner(event->pos()));
    QGraphicsObject::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageControls::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    UpdateCursor(SelectedHandleCorner(event->pos()));
    QGraphicsObject::hoverMoveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageControls::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    UpdateCursor(BIHandleCorner::Invalid);
    QGraphicsObject::hoverLeaveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageControls::ScreenChanged()
{
    prepareGeometryChange();
    InitPixmaps();
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::Id() const -> const QUuid &
{
    return m_id;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageControls::InitPixmaps()
{
    m_handlePixmaps.clear();
    m_handleHoverPixmaps.clear();
    m_handleDisabledPixmaps.clear();
    m_handlePaths.clear();

    auto InitPixmap = [this](BIHandleCornerType type, const QString &imageName)
    {
        const QString resource = QStringLiteral("icon");

        const QString fileName = QStringLiteral("32x32/%1.png").arg(imageName);
        QPixmap handlePixmap = VTheme::GetPixmapResource(resource, fileName);

        if (QGuiApplication::primaryScreen()->devicePixelRatio() >= 2)
        {
            const QString fileName2x = QStringLiteral("32x32/%1@2x.png").arg(imageName);
            const QString fileName2xHover = QStringLiteral("32x32/%1-hover@2x.png").arg(imageName);
            const QString fileName2xDisabled = QStringLiteral("32x32/%1-disabled@2x.png").arg(imageName);

            m_handlePixmaps.insert(type, VTheme::GetPixmapResource(resource, fileName2x));
            m_handleHoverPixmaps.insert(type, VTheme::GetPixmapResource(resource, fileName2xHover));
            m_handleDisabledPixmaps.insert(type, VTheme::GetPixmapResource(resource, fileName2xDisabled));
        }
        else
        {
            const QString fileNameHover = QStringLiteral("32x32/%1-hover.png").arg(imageName);
            const QString fileNameDisabled = QStringLiteral("32x32/%1-disabled.png").arg(imageName);

            m_handlePixmaps.insert(type, handlePixmap);
            m_handleHoverPixmaps.insert(type, VTheme::GetPixmapResource(resource, fileNameHover));
            m_handleDisabledPixmaps.insert(type, VTheme::GetPixmapResource(resource, fileNameDisabled));
        }

        QPainterPath p = PixmapToPainterPath(handlePixmap);
        p.setFillRule(Qt::WindingFill);
        p.closeSubpath();
        m_handlePaths.insert(type, p);
    };

    InitPixmap(BIHandleCornerType::ScaleTopLeftBottomRight, QStringLiteral("expand2"));
    InitPixmap(BIHandleCornerType::ScaleTopBottom, QStringLiteral("double-arrow-vertical"));
    InitPixmap(BIHandleCornerType::ScaleTopRightBottomLeft, QStringLiteral("expand1"));
    InitPixmap(BIHandleCornerType::ScaleRightLeft, QStringLiteral("double-arrow-horizontal"));
    InitPixmap(BIHandleCornerType::RotateTopLeft, QStringLiteral("rotate-top-left"));
    InitPixmap(BIHandleCornerType::RotateTopRight, QStringLiteral("rotate-top-right"));
    InitPixmap(BIHandleCornerType::RotateBottomRight, QStringLiteral("rotate-bottom-right"));
    InitPixmap(BIHandleCornerType::RotateBottomLeft, QStringLiteral("rotate-bottom-left"));
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::TopLeftHandlerPosition() const -> QPointF
{
    return ControllersRect().topLeft();
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::TopHandlerPosition() const -> QPointF
{
    QRectF rect = ControllersRect();
    QPixmap handler = m_handlePixmaps.value(BIHandleCornerType::ScaleTopBottom);
    QSize size = handler.size() / handler.devicePixelRatio();
    return {rect.topLeft().x() + (rect.width() / 2. - size.width() / 2.), rect.topLeft().y()};
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::TopRightHandlerPosition() const -> QPointF
{
    QRectF rect = ControllersRect();
    QPixmap handler = m_handlePixmaps.value(BIHandleCornerType::ScaleTopRightBottomLeft);
    QSize size = handler.size() / handler.devicePixelRatio();
    return {rect.topLeft().x() + (rect.width() - size.width()), rect.topLeft().y()};
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::RightHandlerPosition() const -> QPointF
{
    QRectF rect = ControllersRect();
    QPixmap handler = m_handlePixmaps.value(BIHandleCornerType::ScaleRightLeft);
    QSize size = handler.size() / handler.devicePixelRatio();
    return {rect.topLeft().x() + (rect.width() - size.width()),
            rect.topLeft().y() + (rect.height() / 2. - size.height() / 2.)};
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::BottomRightHandlerPosition() const -> QPointF
{
    QRectF rect = ControllersRect();
    QPixmap handler = m_handlePixmaps.value(BIHandleCornerType::ScaleTopLeftBottomRight);
    QSize size = handler.size() / handler.devicePixelRatio();
    return {rect.topLeft().x() + (rect.width() - size.width()), rect.topLeft().y() + (rect.height() - size.height())};
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::BottomHandlerPosition() const -> QPointF
{
    QRectF rect = ControllersRect();
    QPixmap handler = m_handlePixmaps.value(BIHandleCornerType::ScaleTopBottom);
    QSize size = handler.size() / handler.devicePixelRatio();
    return {rect.topLeft().x() + (rect.width() / 2. - size.width() / 2.),
            rect.topLeft().y() + (rect.height() - size.height())};
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::BottomLeftHandlerPosition() const -> QPointF
{
    QRectF rect = ControllersRect();
    QPixmap handler = m_handlePixmaps.value(BIHandleCornerType::ScaleTopRightBottomLeft);
    QSize size = handler.size() / handler.devicePixelRatio();
    return {rect.topLeft().x(), rect.topLeft().y() + (rect.height() - size.height())};
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::LeftHandlerPosition() const -> QPointF
{
    QRectF rect = ControllersRect();
    QPixmap handler = m_handlePixmaps.value(BIHandleCornerType::ScaleRightLeft);
    QSize size = handler.size() / handler.devicePixelRatio();
    return {rect.topLeft().x(), rect.topLeft().y() + (rect.height() / 2. - size.height() / 2.)};
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::ControllerPath(BIHandleCornerType type, QPointF pos) const -> QPainterPath
{
    QTransform t;
    t.translate(pos.x(), pos.y());

    QPainterPath controller = m_handlePaths.value(type);

    controller = t.map(controller);

    return controller;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::ScaleTopLeftControl() const -> QPainterPath
{
    return ControllerPath(BIHandleCornerType::ScaleTopLeftBottomRight, TopLeftHandlerPosition());
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::ScaleTopControl() const -> QPainterPath
{
    return ControllerPath(BIHandleCornerType::ScaleTopBottom, TopHandlerPosition());
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::ScaleTopRightControl() const -> QPainterPath
{
    return ControllerPath(BIHandleCornerType::ScaleTopRightBottomLeft, TopRightHandlerPosition());
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::ScaleRightControl() const -> QPainterPath
{
    return ControllerPath(BIHandleCornerType::ScaleRightLeft, RightHandlerPosition());
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::ScaleBottomRightControl() const -> QPainterPath
{
    return ControllerPath(BIHandleCornerType::ScaleTopLeftBottomRight, BottomRightHandlerPosition());
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::ScaleBottomControl() const -> QPainterPath
{
    return ControllerPath(BIHandleCornerType::ScaleTopBottom, BottomHandlerPosition());
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::ScaleBottomLeftControl() const -> QPainterPath
{
    return ControllerPath(BIHandleCornerType::ScaleTopRightBottomLeft, BottomLeftHandlerPosition());
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::ScaleLeftControl() const -> QPainterPath
{
    return ControllerPath(BIHandleCornerType::ScaleRightLeft, LeftHandlerPosition());
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::RotateTopLeftControl() const -> QPainterPath
{
    return ControllerPath(BIHandleCornerType::RotateTopLeft, TopLeftHandlerPosition());
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::RotateTopRightControl() const -> QPainterPath
{
    return ControllerPath(BIHandleCornerType::RotateTopRight, TopRightHandlerPosition());
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::RotateBottomRightControl() const -> QPainterPath
{
    return ControllerPath(BIHandleCornerType::RotateBottomRight, BottomRightHandlerPosition());
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::RotateBottomLeftControl() const -> QPainterPath
{
    return ControllerPath(BIHandleCornerType::RotateBottomLeft, BottomLeftHandlerPosition());
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::ScaleByTopLeft(QGraphicsSceneMouseEvent *event) const -> QTransform
{
    QRectF rectOriginal = m_imageBoundingRect;
    QRectF rect = m_imageScreenBoundingRect;

    const qreal adjustmentX = -rect.topLeft().x();
    const qreal adjustmentY = -rect.topLeft().y();

    rect.translate(adjustmentX, adjustmentY);

    QPointF pos = event->pos();
    pos.rx() += adjustmentX;
    pos.ry() += adjustmentY;

    QRectF nowRect;
    QPointF diff;

    if (event->modifiers() & Qt::ShiftModifier)
    {
        nowRect = rect;

        QPointF centerScalePoint = pos - m_scaleDiff;

        if (centerScalePoint.x() > rect.center().x())
        {
            centerScalePoint.rx() = rect.center().x() - (centerScalePoint.x() - rect.center().x());
        }

        if (centerScalePoint.y() > rect.center().y())
        {
            centerScalePoint.ry() = rect.center().y() - (centerScalePoint.y() - rect.center().y());
        }

        diff = centerScalePoint - rect.topLeft();

        if (event->modifiers() & Qt::ControlModifier)
        {
            nowRect.adjust(diff.x(), diff.y(), -diff.x(), -diff.y());
        }
        else
        {
            qreal move;
            if (diff.x() > 0 && diff.x() >= diff.y())
            {
                move = diff.x();
            }
            else if (diff.y() > 0 && diff.y() >= diff.x())
            {
                move = diff.y();
            }
            else
            {
                move = qMax(diff.x(), diff.y());
            }

            nowRect.adjust(move, move, -move, -move);
        }
    }
    else
    {
        QPointF newScalePoint = pos - m_scaleDiff;

        if (newScalePoint.x() > rect.bottomRight().x())
        {
            newScalePoint.rx() = rect.bottomRight().x() - (newScalePoint.x() - rect.bottomRight().x());
        }

        if (newScalePoint.y() > rect.bottomRight().y())
        {
            newScalePoint.ry() = rect.bottomRight().y() - (newScalePoint.y() - rect.bottomRight().y());
        }

        diff = newScalePoint - rect.topLeft();

        nowRect = QRectF(newScalePoint, rect.bottomRight());
    }

    qreal scaleX = nowRect.width() / rect.width();
    qreal scaleY = nowRect.height() / rect.height();

    if (not(event->modifiers() & Qt::ControlModifier))
    {
        if (diff.x() > 0 && diff.x() >= diff.y())
        {
            scaleY = scaleX;
        }
        else if (diff.y() > 0 && diff.y() >= diff.x())
        {
            scaleX = scaleY;
        }
        else
        {
            scaleX = qMin(scaleX, scaleY);
            scaleY = qMin(scaleX, scaleY);
        }
    }

    QPointF newScalePoint = pos - m_scaleDiff;

    QTransform m;

    QPointF scaleCenterOriginal =
        (event->modifiers() & Qt::ShiftModifier) ? rectOriginal.center() : rectOriginal.bottomRight();

    m.translate(scaleCenterOriginal.x(), scaleCenterOriginal.y());

    m.scale(scaleX, scaleY);

    QPointF scaleCenter = (event->modifiers() & Qt::ShiftModifier) ? rect.center() : rect.bottomRight();

    if (newScalePoint.x() > scaleCenter.x())
    {
        m.scale(-1, 1);
    }

    if (newScalePoint.y() > scaleCenter.y())
    {
        m.scale(1, -1);
    }

    m.translate(-scaleCenterOriginal.x(), -scaleCenterOriginal.y());

    return m;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::ScaleByTop(QGraphicsSceneMouseEvent *event) const -> QTransform
{
    QRectF rectOriginal = m_imageBoundingRect;
    QRectF rect = m_imageScreenBoundingRect;

    const qreal adjustmentX = -rect.topLeft().x();
    const qreal adjustmentY = -rect.topLeft().y();

    rect.translate(adjustmentX, adjustmentY);

    QPointF pos = event->pos();
    pos.rx() += adjustmentX;
    pos.ry() += adjustmentY;

    QRectF nowRect;
    if (event->modifiers() & Qt::ShiftModifier)
    {
        nowRect = rect;

        QPointF centerScalePoint = pos - m_scaleDiff;

        if (centerScalePoint.y() > rect.center().y())
        {
            centerScalePoint.ry() = rect.center().y() - (centerScalePoint.y() - rect.center().y());
        }

        QPointF diff = centerScalePoint - RectTopPoint(rect);

        if (event->modifiers() & Qt::ControlModifier)
        {
            nowRect.adjust(0, diff.y(), 0, -diff.y());
        }
        else
        {
            nowRect.adjust(diff.x(), diff.y(), -diff.x(), -diff.y());
        }
    }
    else
    {
        QPointF newScalePoint = pos - m_scaleDiff;

        if (newScalePoint.y() > RectBottomPoint(rect).y())
        {
            newScalePoint.ry() = RectBottomPoint(rect).y() - (newScalePoint.y() - RectBottomPoint(rect).y());
        }

        nowRect = QRectF(QPointF(rect.topLeft().x(), newScalePoint.y()), rect.bottomRight());
    }

    qreal scaleX = nowRect.height() / rect.height();
    qreal scaleY = nowRect.height() / rect.height();

    if (event->modifiers() & Qt::ControlModifier)
    {
        scaleX = 1;
    }

    QPointF newScalePoint = pos - m_scaleDiff;

    QTransform m;

    QPointF scaleCenterOriginal =
        (event->modifiers() & Qt::ShiftModifier) ? rectOriginal.center() : RectBottomPoint(rectOriginal);

    m.translate(scaleCenterOriginal.x(), scaleCenterOriginal.y());
    m.scale(scaleX, scaleY);

    QPointF scaleCenter = (event->modifiers() & Qt::ShiftModifier) ? rect.center() : rect.bottomRight();

    if (newScalePoint.y() > scaleCenter.y())
    {
        m.scale(1, -1);
    }

    m.translate(-scaleCenterOriginal.x(), -scaleCenterOriginal.y());

    return m;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::ScaleByTopRight(QGraphicsSceneMouseEvent *event) const -> QTransform
{
    QRectF rectOriginal = m_imageBoundingRect;
    QRectF rect = m_imageScreenBoundingRect;

    const qreal adjustmentX = -rect.topLeft().x();
    const qreal adjustmentY = -rect.topLeft().y();

    rect.translate(adjustmentX, adjustmentY);

    QPointF pos = event->pos();
    pos.rx() += adjustmentX;
    pos.ry() += adjustmentY;

    QRectF nowRect;
    QPointF diff;

    if (event->modifiers() & Qt::ShiftModifier)
    {
        nowRect = rect;

        QPointF centerScalePoint = pos - m_scaleDiff;

        if (centerScalePoint.x() < rect.center().x())
        {
            centerScalePoint.rx() = rect.center().x() + (rect.center().x() - centerScalePoint.x());
        }

        if (centerScalePoint.y() > rect.center().y())
        {
            centerScalePoint.ry() = rect.center().y() - (centerScalePoint.y() - rect.center().y());
        }

        diff = centerScalePoint - rect.topRight();

        if (event->modifiers() & Qt::ControlModifier)
        {
            nowRect.adjust(-diff.x(), diff.y(), diff.x(), -diff.y());
        }
        else
        {
            qreal move;
            if (diff.x() < 0 && qAbs(diff.x()) >= qAbs(diff.y()))
            {
                move = diff.x() * -1;
            }
            else if (diff.y() > 0 && qAbs(diff.y()) >= qAbs(diff.x()))
            {
                move = diff.y();
            }
            else
            {
                if (qAbs(diff.x()) <= qAbs(diff.y()))
                {
                    move = diff.x() * -1;
                }
                else
                {
                    move = diff.y();
                }
            }

            nowRect.adjust(move, move, -move, -move);
        }
    }
    else
    {
        QPointF newScalePoint = pos - m_scaleDiff;

        if (newScalePoint.x() < rect.bottomLeft().x())
        {
            newScalePoint.rx() = (rect.bottomLeft().x() - newScalePoint.x()) - rect.bottomLeft().x();
        }

        if (newScalePoint.y() > rect.bottomLeft().y())
        {
            newScalePoint.ry() = rect.bottomLeft().y() - (newScalePoint.y() - rect.bottomLeft().y());
        }

        diff = newScalePoint - rect.topRight();

        nowRect =
            QRectF(QPointF(rect.topLeft().x(), newScalePoint.y()), QPointF(newScalePoint.x(), rect.bottomRight().y()));
    }

    qreal scaleX = nowRect.width() / rect.width();
    qreal scaleY = nowRect.height() / rect.height();

    if (not(event->modifiers() & Qt::ControlModifier))
    {
        if (diff.x() < 0 && qAbs(diff.x()) >= qAbs(diff.y()))
        {
            scaleY = scaleX;
        }
        else if (diff.y() > 0 && qAbs(diff.y()) >= qAbs(diff.x()))
        {
            scaleX = scaleY;
        }
        else
        {
            scaleX = qMin(scaleX, scaleY);
            scaleY = qMin(scaleX, scaleY);
        }
    }

    QPointF newScalePoint = pos - m_scaleDiff;

    QTransform m;
    QPointF scaleCenterOriginal =
        (event->modifiers() & Qt::ShiftModifier) ? rectOriginal.center() : rectOriginal.bottomLeft();

    m.translate(scaleCenterOriginal.x(), scaleCenterOriginal.y());
    m.scale(scaleX, scaleY);

    QPointF scaleCenter = (event->modifiers() & Qt::ShiftModifier) ? rect.center() : rect.bottomLeft();

    if (newScalePoint.x() < scaleCenter.x())
    {
        m.scale(-1, 1);
    }

    if (newScalePoint.y() > scaleCenter.y())
    {
        m.scale(1, -1);
    }

    m.translate(-scaleCenterOriginal.x(), -scaleCenterOriginal.y());

    return m;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::ScaleByRight(QGraphicsSceneMouseEvent *event) const -> QTransform
{
    QRectF rectOriginal = m_imageBoundingRect;
    QRectF rect = m_imageScreenBoundingRect;

    const qreal adjustmentX = -rect.topLeft().x();
    const qreal adjustmentY = -rect.topLeft().y();

    rect.translate(adjustmentX, adjustmentY);

    QPointF pos = event->pos();
    pos.rx() += adjustmentX;
    pos.ry() += adjustmentY;

    QRectF nowRect;

    if (event->modifiers() & Qt::ShiftModifier)
    {
        nowRect = rect;

        QPointF centerScalePoint = pos - m_scaleDiff;

        if (centerScalePoint.x() < rect.center().x())
        {
            centerScalePoint.rx() = rect.center().x() + (rect.center().x() - centerScalePoint.x());
        }

        QPointF diff = centerScalePoint - RectRightPoint(rect);

        if (event->modifiers() & Qt::ControlModifier)
        {
            nowRect.adjust(-diff.x(), 0, diff.x(), 0);
        }
        else
        {
            nowRect.adjust(-diff.x(), -diff.y(), diff.x(), diff.y());
        }
    }
    else
    {
        QPointF newScalePoint = pos - m_scaleDiff;

        if (newScalePoint.x() < RectLeftPoint(rect).x())
        {
            newScalePoint.rx() = (RectLeftPoint(rect).x() - newScalePoint.x()) - RectLeftPoint(rect).x();
        }

        nowRect = QRectF(rect.topLeft(), QPointF(newScalePoint.x(), rect.bottomRight().y()));
    }

    qreal scaleX = nowRect.width() / rect.width();
    qreal scaleY = nowRect.width() / rect.width();

    if (event->modifiers() & Qt::ControlModifier)
    {
        scaleY = 1;
    }

    QPointF newScalePoint = pos - m_scaleDiff;

    QTransform m;

    QPointF scaleCenterOriginal =
        (event->modifiers() & Qt::ShiftModifier) ? rectOriginal.center() : RectLeftPoint(rectOriginal);

    m.translate(scaleCenterOriginal.x(), scaleCenterOriginal.y());
    m.scale(scaleX, scaleY);

    QPointF scaleCenter = (event->modifiers() & Qt::ShiftModifier) ? rect.center() : RectLeftPoint(rect);

    if (newScalePoint.x() < scaleCenter.x())
    {
        m.scale(-1, 1);
    }

    m.translate(-scaleCenterOriginal.x(), -scaleCenterOriginal.y());

    return m;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::ScaleByBottomRight(QGraphicsSceneMouseEvent *event) const -> QTransform
{
    QRectF rectOriginal = m_imageBoundingRect;
    QRectF rect = m_imageScreenBoundingRect;

    // move to origin
    const qreal adjustmentX = -rect.topLeft().x();
    const qreal adjustmentY = -rect.topLeft().y();

    rect.translate(adjustmentX, adjustmentY);

    QPointF pos = event->pos();
    pos.rx() += adjustmentX;
    pos.ry() += adjustmentY;

    QRectF nowRect;
    QPointF diff;

    if (event->modifiers() & Qt::ShiftModifier)
    {
        nowRect = rect;

        QPointF centerScalePoint = pos - m_scaleDiff;

        if (centerScalePoint.x() < rect.center().x())
        {
            centerScalePoint.rx() = rect.center().x() + (rect.center().x() - centerScalePoint.x());
        }

        if (centerScalePoint.y() < rect.center().y())
        {
            centerScalePoint.ry() = rect.center().y() + (rect.center().y() - centerScalePoint.y());
        }

        diff = centerScalePoint - rect.bottomRight();

        if (event->modifiers() & Qt::ControlModifier)
        {
            nowRect.adjust(-diff.x(), -diff.y(), diff.x(), diff.y());
        }
        else
        {
            qreal move;
            if (diff.x() < 0 && diff.x() <= diff.y())
            {
                move = diff.x();
            }
            else if (diff.y() < 0 && diff.y() <= diff.x())
            {
                move = diff.y();
            }
            else
            {
                move = qMin(diff.x(), diff.y());
            }
            nowRect.adjust(-move, -move, move, move);
        }
    }
    else
    {
        // correct the scale point position to match the rect at origin
        QPointF newScalePoint = pos - m_scaleDiff;

        if (newScalePoint.x() < rect.topLeft().x())
        {
            newScalePoint.rx() = rect.topLeft().x() + (rect.topLeft().x() - newScalePoint.x());
        }

        if (newScalePoint.y() < rect.topLeft().y())
        {
            newScalePoint.ry() = rect.topLeft().y() + (rect.topLeft().y() - newScalePoint.y());
        }

        // calculate scale
        diff = newScalePoint - rect.bottomRight();

        nowRect = QRectF(rect.topLeft(), newScalePoint);
    }

    qreal scaleX = nowRect.width() / rect.width();
    qreal scaleY = nowRect.height() / rect.height();

    if (not(event->modifiers() & Qt::ControlModifier))
    {
        if (diff.x() < 0 && diff.x() <= diff.y())
        {
            scaleY = scaleX;
        }
        else if (diff.y() < 0 && diff.y() <= diff.x())
        {
            scaleX = scaleY;
        }
        else
        {
            scaleX = qMin(scaleX, scaleY);
            scaleY = qMin(scaleX, scaleY);
        }
    }

    // prepare transformation
    QPointF newScalePoint = pos - m_scaleDiff;

    QTransform m;

    QPointF scaleCenterOriginal =
        (event->modifiers() & Qt::ShiftModifier) ? rectOriginal.center() : rectOriginal.topLeft();

    m.translate(scaleCenterOriginal.x(), scaleCenterOriginal.y());

    m.scale(scaleX, scaleY);

    QPointF scaleCenter = (event->modifiers() & Qt::ShiftModifier) ? rect.center() : rect.topLeft();

    if (newScalePoint.x() < scaleCenter.x())
    {
        m.scale(-1, 1);
    }

    if (newScalePoint.y() < scaleCenter.y())
    {
        m.scale(1, -1);
    }

    m.translate(-scaleCenterOriginal.x(), -scaleCenterOriginal.y());

    return m;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::ScaleByBottom(QGraphicsSceneMouseEvent *event) const -> QTransform
{
    QRectF rectOriginal = m_imageBoundingRect;
    QRectF rect = m_imageScreenBoundingRect;

    const qreal adjustmentX = -rect.topLeft().x();
    const qreal adjustmentY = -rect.topLeft().y();

    rect.translate(adjustmentX, adjustmentY);

    QPointF pos = event->pos();
    pos.rx() += adjustmentX;
    pos.ry() += adjustmentY;

    QRectF nowRect;
    if (event->modifiers() & Qt::ShiftModifier)
    {
        nowRect = rect;

        QPointF centerScalePoint = pos - m_scaleDiff;

        if (centerScalePoint.y() < rect.center().y())
        {
            centerScalePoint.ry() = rect.center().y() + (rect.center().y() - centerScalePoint.y());
        }

        QPointF diff = centerScalePoint - RectBottomPoint(rect);

        if (event->modifiers() & Qt::ControlModifier)
        {
            nowRect.adjust(0, -diff.y(), 0, diff.y());
        }
        else
        {
            nowRect.adjust(-diff.x(), -diff.y(), diff.x(), diff.y());
        }
    }
    else
    {
        QPointF newScalePoint = pos - m_scaleDiff;

        if (newScalePoint.y() < RectTopPoint(rect).y())
        {
            newScalePoint.ry() = (RectTopPoint(rect).y() - newScalePoint.y()) - RectTopPoint(rect).y();
        }

        nowRect = QRectF(rect.topLeft(), QPointF(rect.bottomRight().x(), newScalePoint.y()));
    }

    qreal scaleX = nowRect.height() / rect.height();
    qreal scaleY = nowRect.height() / rect.height();

    if (event->modifiers() & Qt::ControlModifier)
    {
        scaleX = 1;
    }

    QPointF newScalePoint = pos - m_scaleDiff;

    QTransform m;

    QPointF scaleCenterOriginal =
        (event->modifiers() & Qt::ShiftModifier) ? rectOriginal.center() : RectTopPoint(rectOriginal);

    m.translate(scaleCenterOriginal.x(), scaleCenterOriginal.y());
    m.scale(scaleX, scaleY);

    QPointF scaleCenter = (event->modifiers() & Qt::ShiftModifier) ? rect.center() : RectTopPoint(rect);

    if (newScalePoint.y() < scaleCenter.y())
    {
        m.scale(1, -1);
    }

    m.translate(-scaleCenterOriginal.x(), -scaleCenterOriginal.y());

    return m;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::ScaleByBottomLeft(QGraphicsSceneMouseEvent *event) const -> QTransform
{
    QRectF rectOriginal = m_imageBoundingRect;
    QRectF rect = m_imageScreenBoundingRect;

    const qreal adjustmentX = -rect.topLeft().x();
    const qreal adjustmentY = -rect.topLeft().y();

    rect.translate(adjustmentX, adjustmentY);

    QPointF pos = event->pos();
    pos.rx() += adjustmentX;
    pos.ry() += adjustmentY;

    QRectF nowRect;
    QPointF diff;

    if (event->modifiers() & Qt::ShiftModifier)
    {
        nowRect = rect;

        QPointF centerScalePoint = pos - m_scaleDiff;

        if (centerScalePoint.x() > rect.center().x())
        {
            centerScalePoint.rx() = rect.center().x() - (centerScalePoint.x() - rect.center().x());
        }

        if (centerScalePoint.y() < rect.center().y())
        {
            centerScalePoint.ry() = rect.center().y() + (rect.center().y() - centerScalePoint.y());
        }

        diff = centerScalePoint - rect.bottomLeft();
        qDebug() << diff;

        if (event->modifiers() & Qt::ControlModifier)
        {
            nowRect.adjust(diff.x(), -diff.y(), -diff.x(), diff.y());
        }
        else
        {
            qreal move;
            if (diff.x() > 0 && diff.x() >= qAbs(diff.y()))
            {
                move = diff.x();
            }
            else if (diff.y() < 0 && qAbs(diff.y()) >= qAbs(diff.x()))
            {
                move = diff.y() * -1;
            }
            else
            {
                if (qAbs(diff.x()) <= qAbs(diff.y()))
                {
                    move = diff.x();
                }
                else
                {
                    move = diff.y() * -1;
                }
            }
            qDebug() << move;
            nowRect.adjust(move, move, -move, -move);
        }
    }
    else
    {
        QPointF newScalePoint = pos - m_scaleDiff;

        if (newScalePoint.x() > rect.topRight().x())
        {
            newScalePoint.rx() = rect.topRight().x() - (newScalePoint.x() - rect.topRight().x());
        }

        if (newScalePoint.y() < rect.topRight().y())
        {
            newScalePoint.ry() = (rect.topRight().y() - newScalePoint.y()) - rect.topRight().y();
        }

        diff = newScalePoint - rect.topRight();

        nowRect =
            QRectF(QPointF(newScalePoint.x(), rect.topLeft().y()), QPointF(rect.bottomRight().x(), newScalePoint.y()));
    }

    qreal scaleX = nowRect.width() / rect.width();
    qreal scaleY = nowRect.height() / rect.height();

    if (not(event->modifiers() & Qt::ControlModifier))
    {
        if (diff.x() > 0 && diff.x() >= diff.y())
        {
            scaleY = scaleX;
        }
        else if (diff.y() < 0 && diff.y() >= diff.x())
        {
            scaleX = scaleY;
        }
        else
        {
            scaleX = qMin(scaleX, scaleY);
            scaleY = qMin(scaleX, scaleY);
        }
    }

    QPointF newScalePoint = pos - m_scaleDiff;

    QTransform m;
    QPointF scaleCenterOriginal =
        (event->modifiers() & Qt::ShiftModifier) ? rectOriginal.center() : rectOriginal.topRight();

    m.translate(scaleCenterOriginal.x(), scaleCenterOriginal.y());
    m.scale(scaleX, scaleY);

    QPointF scaleCenter = (event->modifiers() & Qt::ShiftModifier) ? rect.center() : rect.topRight();

    if (newScalePoint.x() > scaleCenter.x())
    {
        m.scale(-1, 1);
    }

    if (newScalePoint.y() < scaleCenter.y())
    {
        m.scale(1, -1);
    }

    m.translate(-scaleCenterOriginal.x(), -scaleCenterOriginal.y());

    return m;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::ScaleByLeft(QGraphicsSceneMouseEvent *event) const -> QTransform
{
    QRectF rectOriginal = m_imageBoundingRect;
    QRectF rect = m_imageScreenBoundingRect;

    const qreal adjustmentX = -rect.topLeft().x();
    const qreal adjustmentY = -rect.topLeft().y();

    rect.translate(adjustmentX, adjustmentY);

    QPointF pos = event->pos();
    pos.rx() += adjustmentX;
    pos.ry() += adjustmentY;

    QRectF nowRect;
    if (event->modifiers() & Qt::ShiftModifier)
    {
        nowRect = rect;

        QPointF centerScalePoint = pos - m_scaleDiff;

        if (centerScalePoint.x() > rect.center().x())
        {
            centerScalePoint.rx() = rect.center().x() - (centerScalePoint.x() - rect.center().x());
        }

        QPointF diff = centerScalePoint - RectLeftPoint(rect);

        if (event->modifiers() & Qt::ControlModifier)
        {
            nowRect.adjust(diff.x(), 0, -diff.x(), 0);
        }
        else
        {
            nowRect.adjust(diff.x(), diff.y(), -diff.x(), -diff.y());
        }
    }
    else
    {
        QPointF newScalePoint = pos - m_scaleDiff;

        if (newScalePoint.x() > RectRightPoint(rect).x())
        {
            newScalePoint.rx() = RectRightPoint(rect).x() - (newScalePoint.x() - RectRightPoint(rect).x());
        }

        nowRect = QRectF(QPointF(newScalePoint.x(), rect.topLeft().y()), rect.bottomRight());
    }

    qreal scaleX = nowRect.width() / rect.width();
    qreal scaleY = nowRect.width() / rect.width();

    if (event->modifiers() & Qt::ControlModifier)
    {
        scaleY = 1;
    }

    QPointF newScalePoint = pos - m_scaleDiff;

    QTransform m;

    QPointF scaleCenterOriginal =
        (event->modifiers() & Qt::ShiftModifier) ? rectOriginal.center() : RectRightPoint(rectOriginal);

    m.translate(scaleCenterOriginal.x(), scaleCenterOriginal.y());
    m.scale(scaleX, scaleY);

    QPointF scaleCenter = (event->modifiers() & Qt::ShiftModifier) ? rect.center() : RectRightPoint(rect);

    if (newScalePoint.x() > scaleCenter.x())
    {
        m.scale(-1, 1);
    }

    m.translate(-scaleCenterOriginal.x(), -scaleCenterOriginal.y());

    return m;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::Handles() const -> QPainterPath
{
    QPainterPath path;

    if (m_tranformationType == BITransformationType::Scale)
    {
        path.addPath(ScaleTopLeftControl());
        path.addPath(ScaleTopControl());
        path.addPath(ScaleTopRightControl());
        path.addPath(ScaleRightControl());
        path.addPath(ScaleBottomRightControl());
        path.addPath(ScaleBottomControl());
        path.addPath(ScaleBottomLeftControl());
        path.addPath(ScaleLeftControl());
    }
    else if (m_tranformationType == BITransformationType::Rotate)
    {
        path.addPath(RotateTopLeftControl());
        path.addPath(RotateTopRightControl());
        path.addPath(RotateBottomRightControl());
        path.addPath(RotateBottomLeftControl());
    }

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::ControllersRect() const -> QRectF
{
    const qreal scale = SceneScale(scene());
    QPixmap handler = m_handlePixmaps.value(BIHandleCornerType::ScaleTopLeftBottomRight);
    QRectF imageRect = m_image.BoundingRect();

    imageRect = QRectF(imageRect.topLeft() * scale, QSizeF(imageRect.width() * scale, imageRect.height() * scale));
    QRectF rect = imageRect;

    if (imageRect.width() < handler.width())
    {
        qreal diff = handler.width() - imageRect.width();
        rect.adjust(0, 0, diff, 0);
    }

    if (imageRect.height() < handler.height())
    {
        qreal diff = handler.height() - imageRect.height();
        rect.adjust(0, 0, 0, diff);
    }

    const qreal gap = 2;
    rect.adjust(-(handler.width() + gap), -(handler.height() + gap), handler.width() + gap, handler.height() + gap);

    return rect;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::SelectedHandleCorner(const QPointF &pos) const -> BIHandleCorner
{
    if (m_image.Hold())
    {
        return BIHandleCorner::Invalid;
    }

    QMap<BIHandleCorner, QPainterPath> corners;

    if (m_tranformationType == BITransformationType::Scale)
    {
        corners = {
            {BIHandleCorner::TopLeft, ScaleTopLeftControl()},         {BIHandleCorner::Top, ScaleTopControl()},
            {BIHandleCorner::TopRight, ScaleTopRightControl()},       {BIHandleCorner::Right, ScaleRightControl()},
            {BIHandleCorner::BottomRight, ScaleBottomRightControl()}, {BIHandleCorner::Bottom, ScaleBottomControl()},
            {BIHandleCorner::BottomLeft, ScaleBottomLeftControl()},   {BIHandleCorner::Left, ScaleLeftControl()},
        };
    }
    else if (m_tranformationType == BITransformationType::Rotate)
    {
        corners = {
            {BIHandleCorner::TopLeft, RotateTopLeftControl()},
            {BIHandleCorner::TopRight, RotateTopRightControl()},
            {BIHandleCorner::BottomRight, RotateBottomRightControl()},
            {BIHandleCorner::BottomLeft, RotateBottomLeftControl()},
        };
    }

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

    return BIHandleCorner::Invalid;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::HandlerPixmap(bool hover, BIHandleCornerType type) const -> QPixmap
{
    if (not m_image.Hold())
    {
        return hover ? m_handleHoverPixmaps.value(type) : m_handlePixmaps.value(type);
    }

    return m_handleDisabledPixmaps.value(type);
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageControls::ShowOrigin(const QPointF &pos)
{
    prepareGeometryChange();
    m_showOrigin = true;
    m_originPos = pos;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::OriginCircle1() const -> QPainterPath
{
    const qreal radius1 = 4;
    QPainterPath path;
    const qreal sceneScale = SceneScale(scene());
    QPointF screeOrigin = m_originPos * sceneScale;
    path.addEllipse({screeOrigin.x() - radius1, screeOrigin.y() - radius1, radius1 * 2., radius1 * 2.});
    return path;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::OriginCircle2() const -> QPainterPath
{
    const qreal radius2 = 8;
    QPainterPath path;
    const qreal sceneScale = SceneScale(scene());
    QPointF screeOrigin = m_originPos * sceneScale;
    path.addEllipse({screeOrigin.x() - radius2, screeOrigin.y() - radius2, radius2 * 2., radius2 * 2.});
    return path;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageControls::OriginPath() const -> QPainterPath
{
    QPainterPath path;

    if (m_showOrigin)
    {
        path.addPath(OriginCircle1());
        path.addPath(OriginCircle2());
    }

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageControls::ScaleImage(QGraphicsSceneMouseEvent *event)
{
    QTransform imageMatrix = m_originalMatrix;
    const bool shiftModifier = event->modifiers() & Qt::ShiftModifier;

    switch (m_handleCornerHover)
    {
        case BIHandleCorner::TopLeft:
            ShowOrigin(shiftModifier ? m_imageBoundingRect.center() : m_imageBoundingRect.bottomRight());
            imageMatrix *= ScaleByTopLeft(event);
            break;
        case BIHandleCorner::Top:
            ShowOrigin(shiftModifier ? m_imageBoundingRect.center() : RectBottomPoint(m_imageBoundingRect));
            imageMatrix *= ScaleByTop(event);
            break;
        case BIHandleCorner::TopRight:
            ShowOrigin(shiftModifier ? m_imageBoundingRect.center() : m_imageBoundingRect.bottomLeft());
            imageMatrix *= ScaleByTopRight(event);
            break;
        case BIHandleCorner::Right:
            ShowOrigin(shiftModifier ? m_imageBoundingRect.center() : RectLeftPoint(m_imageBoundingRect));
            imageMatrix *= ScaleByRight(event);
            break;
        case BIHandleCorner::BottomRight:
            ShowOrigin(shiftModifier ? m_imageBoundingRect.center() : m_imageBoundingRect.topLeft());
            imageMatrix *= ScaleByBottomRight(event);
            break;
        case BIHandleCorner::Bottom:
            ShowOrigin(shiftModifier ? m_imageBoundingRect.center() : RectTopPoint(m_imageBoundingRect));
            imageMatrix *= ScaleByBottom(event);
            break;
        case BIHandleCorner::BottomLeft:
            ShowOrigin(shiftModifier ? m_imageBoundingRect.center() : m_imageBoundingRect.topRight());
            imageMatrix *= ScaleByBottomLeft(event);
            break;
        case BIHandleCorner::Left:
            ShowOrigin(shiftModifier ? m_imageBoundingRect.center() : RectRightPoint(m_imageBoundingRect));
            imageMatrix *= ScaleByLeft(event);
            break;
        case BIHandleCorner::Invalid:
        default:
            break;
    }

    if (m_handleCornerHover != BIHandleCorner::Invalid)
    {
        auto *command = new ScaleBackgroundImage(m_image.Id(), imageMatrix, m_doc, m_allowChangeMerge);
        VAbstractApplication::VApp()->getUndoStack()->push(command);
        m_allowChangeMerge = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageControls::RotateImage(QGraphicsSceneMouseEvent *event)
{
    const bool shiftModifier = event->modifiers() & Qt::ShiftModifier;

    switch (m_handleCornerHover)
    {
        case BIHandleCorner::TopLeft:
            ShowOrigin(shiftModifier ? m_imageBoundingRect.bottomRight() : m_imageBoundingRect.center());
            break;
        case BIHandleCorner::TopRight:
            ShowOrigin(shiftModifier ? m_imageBoundingRect.bottomLeft() : m_imageBoundingRect.center());
            break;
        case BIHandleCorner::BottomRight:
            ShowOrigin(shiftModifier ? m_imageBoundingRect.topLeft() : m_imageBoundingRect.center());
            break;
        case BIHandleCorner::BottomLeft:
            ShowOrigin(shiftModifier ? m_imageBoundingRect.topRight() : m_imageBoundingRect.center());
            break;
        default:
            break;
    }

    if (m_handleCornerHover != BIHandleCorner::Invalid)
    {
        QPointF rotationNewPoint = event->pos();
        const qreal sceneScale = SceneScale(scene());
        QPointF screenOriginPos = m_originPos * sceneScale;

        QLineF initPosition(screenOriginPos, m_rotationStartPoint);
        QLineF initRotationPosition(screenOriginPos, rotationNewPoint);

        qreal rotateOn = initPosition.angleTo(initRotationPosition);

        if (rotateOn > 180)
        {
            rotateOn = rotateOn - 360.;
        }

        if (event->modifiers() & Qt::ControlModifier)
        {
            const qreal sign = std::copysign(1.0, rotateOn);
            const int steps = qFloor(qAbs(rotateOn / 15));
            rotateOn = 15 * steps * sign;
        }

        QTransform imageMatrix = m_originalMatrix;

        QTransform m;
        m.translate(m_originPos.x(), m_originPos.y());
        m.rotate(-rotateOn);
        m.translate(-m_originPos.x(), -m_originPos.y());
        imageMatrix *= m;

        auto *command = new RotateBackgroundImage(m_image.Id(), imageMatrix, m_doc, m_allowChangeMerge);
        VAbstractApplication::VApp()->getUndoStack()->push(command);
        m_allowChangeMerge = true;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageControls::UpdateCursor(BIHandleCorner corner)
{
    m_handleCornerHover = corner;

    if (m_handleCornerHover != BIHandleCorner::Invalid)
    {
        if (not m_image.Hold())
        {
            SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
        }
        else
        {
            setCursor(VAbstractValApplication::VApp()->getSceneView()->viewport()->cursor());
        }
    }
    else
    {
        setCursor(VAbstractValApplication::VApp()->getSceneView()->viewport()->cursor());
    }
}
