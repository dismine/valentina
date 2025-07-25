/************************************************************************
 **
 **  @file   vmaingraphicsview.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
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

#include "vmaingraphicsview.h"

#include <QApplication>
#include <QCursor>
#include <QEvent>
#include <QFlags>
#include <QGestureEvent>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QLineF>
#include <QList>
#include <QMessageLogger>
#include <QMimeData>
#include <QMimeDatabase>
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QPainter>
#include <QPoint>
#include <QScreen>
#include <QScrollBar>
#include <QThread>
#include <QTimeLine>
#include <QTransform>
#include <QWheelEvent>
#include <QWidget>
#include <QtMath>

#include "../ifc/xml/utils.h"
#include "../vmisc/compatibility.h"
#include "../vmisc/def.h"
#include "../vmisc/literals.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "global.h"
#include "vcontrolpointspline.h"
#include "vmaingraphicsscene.h"
#include "vsimplecurve.h"

namespace
{
auto ScrollingSteps(QWheelEvent *wheel_event) -> qreal
{
    SCASSERT(wheel_event != nullptr)

    const QPoint numPixels = wheel_event->pixelDelta();
    const QPoint numDegrees = wheel_event->angleDelta() / 8;
    qreal numSteps = 0;
    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();

    if (not numPixels.isNull())
    {
        const qreal mouseScale = settings->GetSensorMouseScale();
        numSteps = (numPixels.x() == 0 ? numPixels.y() : numPixels.x()) / mouseScale;
    }
    else if (not numDegrees.isNull())
    {
        const qreal mouseScale = settings->GetWheelMouseScale();
        numSteps = (numPixels.x() == 0 ? numDegrees.y() : numDegrees.x()) / 15. * mouseScale;
    }

    return numSteps;
}

//---------------------------------------------------------------------------------------------------------------------
auto PrepareScrolling(qreal scheduledScrollings, QWheelEvent *wheel_event) -> qreal
{
    const qreal numSteps = ScrollingSteps(wheel_event);

    if (qFuzzyIsNull(numSteps))
    {
        return scheduledScrollings; // Just ignore
    }

    if (std::signbit(scheduledScrollings) != std::signbit(numSteps))
    { // if user moved the wheel in another direction, we reset previously scheduled scalings
        scheduledScrollings = numSteps;
    }
    else
    {
        scheduledScrollings += numSteps;
    }

    scheduledScrollings *= VAbstractApplication::VApp()->Settings()->GetScrollingAcceleration();

    return scheduledScrollings;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief PrioritizeItems helps prioritize some items over others.
 *
 * In some cases we want items like curve handle points to be selected over other items on scene.
 * @param list list of scene items under a mouse pointer
 * @return prioritized list where prioritized items goes first
 */
auto PrioritizeItems(const QList<QGraphicsItem *> &list) -> QList<QGraphicsItem *>
{
    QList<QGraphicsItem *> prioritized;
    QList<QGraphicsItem *> nonPrioritized;
    for (auto *item : list)
    {
        item && item->type() == VControlPointSpline::Type ? prioritized.append(item) : nonPrioritized.append(item);
    }

    return prioritized + nonPrioritized;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
GraphicsViewZoom::GraphicsViewZoom(QGraphicsView *view)
  : QObject(view),
    _view(view)
{
    _view->viewport()->installEventFilter(this);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // Disabled because of bug QTBUG-103935
    _view->viewport()->setAttribute(Qt::WA_AcceptTouchEvents, false);
#endif
    _view->viewport()->grabGesture(Qt::PinchGesture);
    _view->setMouseTracking(true);

    InitScrollingAnimation();
}

//---------------------------------------------------------------------------------------------------------------------
void GraphicsViewZoom::gentle_zoom(double factor)
{
    // We need to check current scale factor because in Windows we have an error when we zoom in or zoom out to much.
    // See issue #532: Unexpected error occurs when zoom out image.
    // factor > 1 for zoomIn and factor < 1 for zoomOut.
    const qreal m11 = _view->transform().m11();

    if ((factor > 1 && m11 <= VMainGraphicsView::MaxScale()) || (factor < 1 && m11 >= VMainGraphicsView::MinScale()))
    {
        _view->scale(factor, factor);
        if (factor < 1)
        {
            // Because QGraphicsView centers the picture when it's smaller than the view. And QGraphicsView's scrolls
            // boundaries don't allow to put any picture point at any viewport position we will provide fictive scene
            // size. Temporary and bigger than view, scene size will help position an image under cursor.
            FictiveSceneRect(_view->scene(), _view);
        }
        _view->centerOn(target_scene_pos);
        QPointF const delta_viewport_pos =
            target_viewport_pos - QPointF(_view->viewport()->width() / 2.0, _view->viewport()->height() / 2.0);
        QPointF const viewport_center = _view->mapFromScene(target_scene_pos) - delta_viewport_pos;
        _view->centerOn(_view->mapToScene(viewport_center.toPoint()));
        // In the end we just set correct scene size
        VMainGraphicsView::NewSceneRect(_view->scene(), _view);
        emit zoomed();
    }
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void GraphicsViewZoom::set_modifiers(Qt::KeyboardModifiers modifiers)
{
    _modifiers = modifiers;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void GraphicsViewZoom::set_zoom_factor_base(double value)
{
    _zoom_factor_base = value;
}

//---------------------------------------------------------------------------------------------------------------------
void GraphicsViewZoom::InitScrollingAnimation()
{
    delete verticalScrollAnim.data();

    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();

    verticalScrollAnim = new QTimeLine(settings->GetScrollingDuration(), this);
    verticalScrollAnim->setUpdateInterval(settings->GetScrollingUpdateInterval());

    connect(verticalScrollAnim.data(), &QTimeLine::valueChanged, this, &GraphicsViewZoom::VerticalScrollingTime);
    connect(verticalScrollAnim.data(), &QTimeLine::finished, this, &GraphicsViewZoom::animFinished);

    delete horizontalScrollAnim.data();

    horizontalScrollAnim = new QTimeLine(settings->GetScrollingDuration(), this);
    horizontalScrollAnim->setUpdateInterval(settings->GetScrollingUpdateInterval());

    connect(horizontalScrollAnim.data(), &QTimeLine::valueChanged, this, &GraphicsViewZoom::HorizontalScrollingTime);
    connect(horizontalScrollAnim.data(), &QTimeLine::finished, this, &GraphicsViewZoom::animFinished);
}

//---------------------------------------------------------------------------------------------------------------------
void GraphicsViewZoom::VerticalScrollingTime(qreal x)
{
    const qreal scroll = _numScheduledVerticalScrollings * x;
    _numScheduledVerticalScrollings -= scroll;
    _view->verticalScrollBar()->setValue(qRound(_view->verticalScrollBar()->value() - scroll));
}

//---------------------------------------------------------------------------------------------------------------------
void GraphicsViewZoom::HorizontalScrollingTime(qreal x)
{
    const qreal scroll = _numScheduledHorizontalScrollings * x;
    _numScheduledHorizontalScrollings -= scroll;
    _view->horizontalScrollBar()->setValue(qRound(_view->horizontalScrollBar()->value() - scroll));
}

//---------------------------------------------------------------------------------------------------------------------
void GraphicsViewZoom::animFinished()
{
    _numScheduledVerticalScrollings = 0;
    _numScheduledHorizontalScrollings = 0;

    /*
     * In moust cases cursor position on view doesn't change, but for scene after scrolling position will be different.
     * We are goint to check changes and save new value.
     * If don't do that we will zoom using old value cursor position on scene. It is not what we expect.
     * Almoust the same we do in method GraphicsViewZoom::eventFilter.
     */
    const QPoint pos = _view->mapFromGlobal(QCursor::pos());
    const QPointF delta = target_scene_pos - _view->mapToScene(pos);
    if (qAbs(delta.x()) > 5 || qAbs(delta.y()) > 5)
    {
        target_viewport_pos = pos;
        target_scene_pos = _view->mapToScene(pos);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto GraphicsViewZoom::eventFilter(QObject *object, QEvent *event) -> bool
{
    if (event->type() == QEvent::MouseMove)
    {
        /*
         * Here we are saving cursor position on view and scene.
         * This data need for gentle_zoom().
         * Almoust the same we do in method GraphicsViewZoom::animFinished.
         */
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
        auto *mouse_event = static_cast<QMouseEvent *>(event);
        QPointF const delta = target_viewport_pos - mouse_event->pos();
        if (qAbs(delta.x()) > 5 || qAbs(delta.y()) > 5)
        {
            target_viewport_pos = mouse_event->pos();
            target_scene_pos = _view->mapToScene(mouse_event->pos());
        }
        return false;
    }

    if (event->type() == QEvent::Wheel)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
        if (auto *wheel_event = static_cast<QWheelEvent *>(event))
        {
            const QPoint numDegrees = wheel_event->angleDelta();
            if (numDegrees.x() == 0)
            {
                if (QGuiApplication::keyboardModifiers() == _modifiers)
                {
                    gentle_zoom(qPow(_zoom_factor_base, numDegrees.y()));
                    return true;
                }

                if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
                {
                    StartHorizontalScrollings(wheel_event);
                    return true;
                }

                StartVerticalScrollings(wheel_event);
                return true;
            }

            if (QGuiApplication::keyboardModifiers() == _modifiers)
            {
                return true; // ignore
            }

            StartHorizontalScrollings(wheel_event);
            return true;
        }
    }
    else if (event->type() == QEvent::Gesture)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-static-cast-downcast)
        return GestureEvent(static_cast<QGestureEvent *>(event));
    }

    return QObject::eventFilter(object, event);
}

//---------------------------------------------------------------------------------------------------------------------
void GraphicsViewZoom::FictiveSceneRect(QGraphicsScene *sc, QGraphicsView *view)
{
    SCASSERT(sc != nullptr)
    SCASSERT(view != nullptr)

    // Calculate view rect
    // to receive the currently visible area, map the widgets bounds to the scene
    const QPointF a = view->mapToScene(0, 0);
    const QPointF b = view->mapToScene(view->viewport()->width(), view->viewport()->height());
    auto viewRect = QRectF(a, b);

    // Scale view
    QLineF topLeftRay(viewRect.center(), viewRect.topLeft());
    topLeftRay.setLength(topLeftRay.length() * 2);

    QLineF bottomRightRay(viewRect.center(), viewRect.bottomRight());
    bottomRightRay.setLength(bottomRightRay.length() * 2);

    viewRect = QRectF(topLeftRay.p2(), bottomRightRay.p2());

    // Calculate scene rect
    const QRectF sceneRect = sc->sceneRect();

    // Unite two rects
    const QRectF newRect = sceneRect.united(viewRect);

    sc->setSceneRect(newRect);
}

//---------------------------------------------------------------------------------------------------------------------
void GraphicsViewZoom::StartVerticalScrollings(QWheelEvent *wheel_event)
{
    if (not wheel_event->pixelDelta().isNull())
    { // Native scrolling animation
        _view->verticalScrollBar()->setValue(qCeil(_view->verticalScrollBar()->value() - ScrollingSteps(wheel_event)));
        animFinished();
    }
    else
    {
        _numScheduledVerticalScrollings = PrepareScrolling(_numScheduledVerticalScrollings, wheel_event);

        if (verticalScrollAnim->state() != QTimeLine::Running)
        {
            verticalScrollAnim->start();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void GraphicsViewZoom::StartHorizontalScrollings(QWheelEvent *wheel_event)
{
    if (not wheel_event->pixelDelta().isNull())
    { // Native scrolling animation
        _view->horizontalScrollBar()->setValue(
            qCeil(_view->horizontalScrollBar()->value() - ScrollingSteps(wheel_event)));
        animFinished();
    }
    else
    {
        _numScheduledHorizontalScrollings = PrepareScrolling(_numScheduledHorizontalScrollings, wheel_event);

        if (horizontalScrollAnim->state() != QTimeLine::Running)
        {
            horizontalScrollAnim->start();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto GraphicsViewZoom::GestureEvent(QGestureEvent *event) -> bool
{
    if (QGesture *pinch = event->gesture(Qt::PinchGesture))
    {
        PinchTriggered(static_cast<QPinchGesture *>(pinch)); // NOLINT(cppcoreguidelines-pro-type-static-cast-downcast)
        return true;
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
void GraphicsViewZoom::PinchTriggered(QPinchGesture *gesture)
{
    QPinchGesture::ChangeFlags const changeFlags = gesture->changeFlags();
    if (changeFlags & QPinchGesture::ScaleFactorChanged)
    {
        qreal const currentStepScaleFactor = gesture->lastScaleFactor();
        gentle_zoom(currentStepScaleFactor);
    }
}

const unsigned long VMainGraphicsView::scrollDelay = 160;

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VMainGraphicsView constructor.
 * @param parent parent object.
 */
VMainGraphicsView::VMainGraphicsView(QWidget *parent)
  : QGraphicsView(parent)
{
    setAcceptDrops(true);

    VCommonSettings *settings = VAbstractApplication::VApp()->Settings();
    if (settings && settings->IsOpenGLRender())
    {
        auto *viewport = new QOpenGLWidget();
        QSurfaceFormat fmt;
        fmt.setSamples(settings->GetGraphicalOutput() ? 10 : 0);
        fmt.setStencilBufferSize(8);
        viewport->setFormat(fmt);

        setViewport(viewport);
    }

    zoom = new GraphicsViewZoom(this);

    this->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    this->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    this->setInteractive(true);
    SetAntialiasing(true);

    connect(zoom, &GraphicsViewZoom::zoomed, this, [this]() { emit ScaleChanged(transform().m11()); });
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::Zoom(qreal scale)
{
    qreal const factor = qBound(MinScale(), scale, MaxScale());
    QTransform transform = this->transform();
    transform.setMatrix(factor, transform.m12(), transform.m13(), transform.m21(), factor, transform.m23(),
                        transform.m31(), transform.m32(), transform.m33());
    this->setTransform(transform);
    VMainGraphicsView::NewSceneRect(this->scene(), this);
    emit ScaleChanged(this->transform().m11());
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::ZoomIn()
{
    // We need to check current scale factor because in Windows we have an error when we zoom in or zoom out to much.
    // See issue #532: Unexpected error occurs when zoom out image.
    if (this->transform().m11() <= MaxScale())
    {
        scale(1.1, 1.1);
        VMainGraphicsView::NewSceneRect(this->scene(), this);
        emit ScaleChanged(transform().m11());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::ZoomOut()
{
    // We need to check current scale factor because in Windows we have an error when we zoom in or zoom out to much.
    // See issue #532: Unexpected error occurs when zoom out image.
    if (this->transform().m11() >= MinScale())
    {
        scale(1.0 / 1.1, 1.0 / 1.1);
        VMainGraphicsView::NewSceneRect(this->scene(), this);
        emit ScaleChanged(transform().m11());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::ZoomOriginal()
{
    QTransform trans = this->transform();
    trans.setMatrix(1.0, trans.m12(), trans.m13(), trans.m21(), 1.0, trans.m23(), trans.m31(), trans.m32(),
                    trans.m33());
    this->setTransform(trans);
    VMainGraphicsView::NewSceneRect(this->scene(), this);
    emit ScaleChanged(transform().m11());
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::ZoomFitBest()
{
    auto *currentScene = qobject_cast<VMainGraphicsScene *>(scene());
    SCASSERT(currentScene)
    currentScene->SetOriginsVisible(false);
    const QRectF rect = currentScene->VisibleItemsBoundingRect();
    currentScene->SetOriginsVisible(true);
    if (rect.isEmpty())
    {
        return;
    }

    VMainGraphicsView::NewSceneRect(scene(), this);

    this->fitInView(rect, Qt::KeepAspectRatio);
    QTransform transform = this->transform();

    const qreal factor = qBound(MinScale(), transform.m11(), MaxScale());
    transform.setMatrix(factor, transform.m12(), transform.m13(), transform.m21(), factor, transform.m23(),
                        transform.m31(), transform.m32(), transform.m33());
    this->setTransform(transform);
    emit ScaleChanged(this->transform().m11());
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::ResetScrollingAnimation()
{
    zoom->InitScrollingAnimation();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief mousePressEvent handle mouse press events.
 * @param event mouse press event.
 */
void VMainGraphicsView::mousePressEvent(QMouseEvent *event)
{
    switch (event->button())
    {
        case Qt::LeftButton:
        {
            if (isAllowRubberBand)
            {
                QGraphicsView::setDragMode(QGraphicsView::RubberBandDrag);
            }

            if (showToolOptions)
            {
                bool success = false;
                const QList<QGraphicsItem *> list = PrioritizeItems(items(event->pos()));
                for (auto *item : list)
                {
                    if (item && item->type() > QGraphicsItem::UserType && item->type() <= VSimpleCurve::Type)
                    {
                        emit itemClicked(item);
                        success = true;
                        break;
                    }
                }

                if (not success)
                {
                    emit itemClicked(nullptr);
                }
            }
            break;
        }
        case Qt::MiddleButton:
        {
            auto *scene = qobject_cast<VMainGraphicsScene *>(this->scene());
            const QList<QGraphicsItem *> list = items(event->pos());
            if (list.isEmpty() || (scene && scene->IsNonInteractive()))
            { // Only when the user clicks on the scene background or non interactive scene
                m_ptStartPos = event->pos();
                m_oldCursor = viewport()->cursor();
                QGraphicsView::setDragMode(QGraphicsView::ScrollHandDrag);
                event->accept();
                viewport()->setCursor(Qt::ClosedHandCursor);
            }
            break;
        }
        default:
            break;
    }
    QGraphicsView::mousePressEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    // Hack to fix problem with mouse cursor. Looks like after we switch cursor back it is rewrited back by a dialog.
    // Because no real way to catch this call we will check state for each move and compare to excpected state.
    if (dragMode() != QGraphicsView::ScrollHandDrag)
    {
        QCursor const cur = viewport()->cursor();
        // No way to restore bitmap from shape and we really don't need this for now.
        if (m_currentCursor != Qt::BitmapCursor && cur.shape() == Qt::BitmapCursor &&
            cur.pixmap().cacheKey() != QPixmapFromCache(cursorArrowOpenHand).cacheKey() &&
            cur.pixmap().cacheKey() != QPixmapFromCache(cursorArrowCloseHand).cacheKey())
        {
            viewport()->setCursor(m_currentCursor);
        }
    }

    if (dragMode() == QGraphicsView::ScrollHandDrag)
    {
        QScrollBar *hBar = horizontalScrollBar();
        QScrollBar *vBar = verticalScrollBar();
        const QPoint delta = event->pos() - m_ptStartPos;
        hBar->setValue(hBar->value() + (isRightToLeft() ? delta.x() : -delta.x()));
        vBar->setValue(vBar->value() - delta.y());
        m_ptStartPos = event->pos();
    }
    else
    {
        QGraphicsView::mouseMoveEvent(event);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief mouseReleaseEvent handle mouse release events.
 * @param event mouse release event.
 */
void VMainGraphicsView::mouseReleaseEvent(QMouseEvent *event)
{
    QGraphicsView::mouseReleaseEvent(event); // First because need to hide a rubber band
    QGraphicsView::setDragMode(QGraphicsView::NoDrag);
    if (event->button() == Qt::MiddleButton)
    {
        viewport()->setCursor(m_oldCursor);
    }
    if (event->button() == Qt::LeftButton)
    {
        emit MouseRelease();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton &&
        VAbstractApplication::VApp()->Settings()->IsDoubleClickZoomFitBestCurrentPP())
    {
        emit ZoomFitBestCurrent();
    }

    QGraphicsView::mouseDoubleClickEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *mime = event->mimeData();

    if (const auto *currentScene = qobject_cast<VMainGraphicsScene *>(scene());
        currentScene != nullptr && currentScene->AcceptDrop() && mime != nullptr && mime->hasText())
    {
        if (QUrl const urlPath(mime->text().simplified()); urlPath.isLocalFile())
        {
            const QString fileName = urlPath.toLocalFile();
            if (QFileInfo const f(fileName); f.exists() && IsMimeTypeImage(QMimeDatabase().mimeTypeForFile(fileName)))
            {
                event->acceptProposedAction();
            }
            else
            {
                event->ignore();
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    const QMimeData *mime = event->mimeData();

    if (const auto *currentScene = qobject_cast<VMainGraphicsScene *>(scene());
        currentScene != nullptr && currentScene->AcceptDrop() && mime != nullptr && mime->hasText())
    {
        if (QUrl const urlPath(mime->text().simplified()); urlPath.isLocalFile())
        {
            const QString fileName = urlPath.toLocalFile();
            if (QFileInfo const f(fileName); f.exists() && IsMimeTypeImage(QMimeDatabase().mimeTypeForFile(fileName)))
            {
                event->acceptProposedAction();
            }
            else
            {
                event->ignore();
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::dropEvent(QDropEvent *event)
{
    const QMimeData *mime = event->mimeData();

    if (auto *currentScene = qobject_cast<VMainGraphicsScene *>(scene());
        currentScene != nullptr && currentScene->AcceptDrop() && mime != nullptr && mime->hasText())
    {
        if (QUrl const urlPath(mime->text().simplified()); urlPath.isLocalFile())
        {
            const QString fileName = urlPath.toLocalFile();
            if (QFileInfo const f(fileName); f.exists() && IsMimeTypeImage(QMimeDatabase().mimeTypeForFile(fileName)))
            {
                emit currentScene->AddBackgroundImage(mapToScene(DropEventPos(event)), fileName);
                event->acceptProposedAction();
            }
            else
            {
                event->ignore();
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMainGraphicsView::MinScale() -> qreal
{
    const QRect screenRect = QGuiApplication::primaryScreen()->availableGeometry();
    const qreal screenSize = qMin(screenRect.width(), screenRect.height());

    return screenSize / maxSceneSize;
}

//---------------------------------------------------------------------------------------------------------------------
auto VMainGraphicsView::MaxScale() -> qreal
{
    const QRect screenRect = QGuiApplication::primaryScreen()->availableGeometry();
    const qreal screenSize = qMin(screenRect.width(), screenRect.height());

    return maxSceneSize / screenSize;
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::EnsureItemVisibleWithDelay(const QGraphicsItem *item, unsigned long msecs, int xmargin,
                                                   int ymargin)
{
    SCASSERT(item != nullptr)
    const qreal scale = SceneScale(item->scene());

    const QRectF viewRect = VMainGraphicsView::SceneVisibleArea(this);
    const QRectF itemRect = item->mapToScene(item->boundingRect()).boundingRect();

    // If item's rect is bigger than view's rect ensureVisible works very unstable.
    if (itemRect.height() + 2 * ymargin < viewRect.height() && itemRect.width() + 2 * xmargin < viewRect.width())
    {
        EnsureVisibleWithDelay(item, msecs, xmargin, ymargin);
    }
    else
    {
        // Ensure visible only small rect around a cursor
        auto *currentScene = qobject_cast<VMainGraphicsScene *>(item->scene());
        SCASSERT(currentScene);
        const QPointF cursorPosition = currentScene->getScenePos();
        EnsureVisibleWithDelay(
            QRectF(cursorPosition.x() - 5 / scale, cursorPosition.y() - 5 / scale, 10 / scale, 10 / scale), msecs);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::EnsureVisibleWithDelay(const QRectF &rect, unsigned long msecs, int xmargin, int ymargin)
{
    const int hbar = horizontalScrollBar()->value();
    const int vbar = verticalScrollBar()->value();

    ensureVisible(rect, xmargin, ymargin);

    if (hbar != horizontalScrollBar()->value() || vbar != verticalScrollBar()->value())
    {
        QThread::msleep(msecs);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::EnsureVisibleWithDelay(const QGraphicsItem *item, unsigned long msecs, int xmargin, int ymargin)
{
    const int hbar = horizontalScrollBar()->value();
    const int vbar = verticalScrollBar()->value();

    ensureVisible(item, xmargin, ymargin);

    if (hbar != horizontalScrollBar()->value() || vbar != verticalScrollBar()->value())
    {
        QThread::msleep(msecs);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::setCurrentCursorShape()
{
    m_currentCursor = viewport()->cursor().shape();
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::SetAntialiasing(bool value)
{
    setRenderHint(QPainter::Antialiasing, value);
    setRenderHint(QPainter::SmoothPixmapTransform, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VMainGraphicsView::IsOpenGLRender() const -> bool
{
    auto *viewport = qobject_cast<QOpenGLWidget *>(this->viewport());
    return viewport != nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::setShowToolOptions(bool value)
{
    showToolOptions = value;
}

//---------------------------------------------------------------------------------------------------------------------
void VMainGraphicsView::AllowRubberBand(bool value)
{
    isAllowRubberBand = value;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief NewSceneRect calculate scene rect what contains all items and doesn't less that size of scene view.
 * @param sc scene.
 * @param view view.
 */
void VMainGraphicsView::NewSceneRect(QGraphicsScene *sc, QGraphicsView *view, QGraphicsItem *item)
{
    SCASSERT(sc != nullptr)
    SCASSERT(view != nullptr)

    if (item == nullptr)
    {
        // Calculate view rect
        const QRectF viewRect = SceneVisibleArea(view);

        // Calculate scene rect
        auto *currentScene = qobject_cast<VMainGraphicsScene *>(sc);
        SCASSERT(currentScene)
        const QRectF itemsRect = currentScene->VisibleItemsBoundingRect();

        // Unite two rects
        sc->setSceneRect(itemsRect.united(viewRect));
    }
    else
    {
        QRectF rect = item->sceneBoundingRect();
        const QList<QGraphicsItem *> children = item->childItems();
        for (auto *child : children)
        {
            if (child->isVisible())
            {
                rect = rect.united(child->sceneBoundingRect());
            }
        }

        if (not sc->sceneRect().contains(rect))
        {
            sc->setSceneRect(sc->sceneRect().united(rect));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VMainGraphicsView::SceneVisibleArea(QGraphicsView *view) -> QRectF
{
    SCASSERT(view != nullptr)
    // to receive the currently visible area, map the widgets bounds to the scene
    return {view->mapToScene(0, 0), view->mapToScene(view->width(), view->height())};
}
