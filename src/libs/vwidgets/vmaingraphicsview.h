/************************************************************************
 **
 **  @file   vmaingraphicsview.h
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

#ifndef VMAINGRAPHICSVIEW_H
#define VMAINGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QPointer>
#include <QRectF>
#include <QString>
#include <QtGlobal>

/*!
 * This class adds ability to zoom QGraphicsView using mouse wheel. The point under cursor
 * remains motionless while it's possible.
 *
 * When the user starts scrolling, this class remembers original scene position and
 * keeps it until scrolling is completed. It's better than getting original scene position at
 * each scrolling step because that approach leads to position errors due to before-mentioned
 * positioning restrictions.
 *
 * When zommed using scroll, this class emits zoomed() signal.
 *
 * Usage:
 *
 *   new Graphics_view_zoom(view);
 *
 * The object will be deleted automatically when the view is deleted.
 *
 * You can set keyboard modifiers used for zooming using set_modified(). Zooming will be
 * performed only on exact match of modifiers combination. The default modifier is Ctrl.
 *
 * You can change zoom velocity by calling set_zoom_factor_base().
 * Zoom coefficient is calculated as zoom_factor_base^angle_delta
 * (see QWheelEvent::angleDelta).
 * The default zoom factor base is 1.0015.
 */

class QTimeLine;
class QGestureEvent;
class QPinchGesture;

class GraphicsViewZoom : public QObject
{
    Q_OBJECT // NOLINT

public:
    explicit GraphicsViewZoom(QGraphicsView *view);
    void gentle_zoom(double factor);
    void set_modifiers(Qt::KeyboardModifiers modifiers);
    void set_zoom_factor_base(double value);
    void InitScrollingAnimation();

signals:
    void zoomed();

public slots:
    void VerticalScrollingTime(qreal x);
    void HorizontalScrollingTime(qreal x);
    void animFinished();

protected:
    auto eventFilter(QObject *object, QEvent *event) -> bool override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(GraphicsViewZoom) // NOLINT
    QGraphicsView *_view;
    Qt::KeyboardModifiers _modifiers{Qt::ControlModifier};
    double _zoom_factor_base{1.0015};
    QPointF target_scene_pos{};
    QPointF target_viewport_pos{};
    QPointer<QTimeLine> verticalScrollAnim{};
    /** @brief _numScheduledVerticalScrollings keep number scheduled vertical scrollings. */
    qreal _numScheduledVerticalScrollings{0};
    QPointer<QTimeLine> horizontalScrollAnim{};
    /** @brief _numScheduledHorizontalScrollings keep number scheduled horizontal scrollings. */
    qreal _numScheduledHorizontalScrollings{0};

    static void FictiveSceneRect(QGraphicsScene *sc, QGraphicsView *view);

    void StartVerticalScrollings(QWheelEvent *wheel_event);
    void StartHorizontalScrollings(QWheelEvent *wheel_event);

    auto GestureEvent(QGestureEvent *event) -> bool;
    void PinchTriggered(QPinchGesture *gesture);
};

/**
 * @brief The VMainGraphicsView class main scene view.
 */
class VMainGraphicsView : public QGraphicsView
{
    Q_OBJECT // NOLINT

public:
    explicit VMainGraphicsView(QWidget *parent = nullptr);
    void setShowToolOptions(bool value);
    void AllowRubberBand(bool value);

    static void NewSceneRect(QGraphicsScene *sc, QGraphicsView *view, QGraphicsItem *item = nullptr);
    static auto SceneVisibleArea(QGraphicsView *view) -> QRectF;

    static auto MinScale() -> qreal;
    static auto MaxScale() -> qreal;

    void EnsureItemVisibleWithDelay(const QGraphicsItem *item, unsigned long msecs, int xmargin = 5, int ymargin = 5);

    void EnsureVisibleWithDelay(const QRectF &rect, unsigned long msecs, int xmargin = 50, int ymargin = 50);
    void EnsureVisibleWithDelay(const QGraphicsItem *item, unsigned long msecs, int xmargin = 50, int ymargin = 50);

    static const unsigned long scrollDelay;

    void setCurrentCursorShape();

    void SetAntialiasing(bool value);

    auto IsOpenGLRender() const -> bool;

signals:
    /**
     * @brief MouseRelease help catch mouse release event.
     *
     * Usefull when you need show dialog after working with tool visualization.
     */
    void MouseRelease();
    void itemClicked(QGraphicsItem *item);
    void ScaleChanged(qreal scale);
    void ZoomFitBestCurrent();

public slots:
    void Zoom(qreal scale);
    void ZoomIn();
    void ZoomOut();
    void ZoomOriginal();
    void ZoomFitBest();
    void ResetScrollingAnimation();
    void EnsureToolVisible(const QRectF &rect);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private:
    Q_DISABLE_COPY_MOVE(VMainGraphicsView) // NOLINT
    GraphicsViewZoom *zoom{nullptr};
    bool showToolOptions{true};
    bool isAllowRubberBand{true};
    QPoint m_ptStartPos{};
    QCursor m_oldCursor{};
    Qt::CursorShape m_currentCursor{Qt::ArrowCursor};
};

#endif // VMAINGRAPHICSVIEW_H
