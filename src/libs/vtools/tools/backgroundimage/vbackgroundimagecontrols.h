/************************************************************************
 **
 **  @file   vbackgroundimagecontrols.h
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
#ifndef VBACKGROUNDIMAGECONTROLS_H
#define VBACKGROUNDIMAGECONTROLS_H

#include <QGraphicsObject>
#include <QUuid>

#include "../vmisc/def.h"
#include "../ifc/xml/vbackgroundpatternimage.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 13, 0)

class VAbstractPattern;
class QScreen;

enum class BITransformationType {Scale, Rotate, Unknown};

enum class BIHandleCorner : int
{
    Invalid,
    TopLeft,
    Top,
    TopRight,
    Right,
    BottomRight,
    Bottom,
    BottomLeft,
    Left
};

enum class BIHandleCornerType
{
    ScaleTopLeftBottomRight,
    ScaleTopBottom,
    ScaleTopRightBottomLeft,
    ScaleRightLeft,
    RotateTopLeft,
    RotateTopRight,
    RotateBottomRight,
    RotateBottomLeft
};

class VBackgroundImageControls : public QGraphicsObject
{
    Q_OBJECT // NOLINT
public:
    explicit VBackgroundImageControls(VAbstractPattern *doc, QGraphicsItem * parent = nullptr);
    ~VBackgroundImageControls() override = default;

    auto type() const -> int override {return Type;}
    enum { Type = UserType + static_cast<int>(Tool::BackgroundImageControls)};

    auto Id() const -> const QUuid &;

signals:
    void ActiveImageChanged(const QUuid &id);

public slots:
    void ActivateControls(const QUuid &id);
    void DeactivateControls(QGraphicsItem* item);
    void UpdateControls();

protected:
    auto boundingRect() const -> QRectF override;
    auto shape() const -> QPainterPath override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private slots:
    void ScreenChanged();

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VBackgroundImageControls) // NOLINT

    QUuid m_id{};
    VAbstractPattern *m_doc;

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    VBackgroundPatternImage m_image{};

    QT_WARNING_POP

    BITransformationType m_tranformationType{BITransformationType::Unknown};

    QMap<BIHandleCornerType, QPixmap> m_handlePixmaps{};
    QMap<BIHandleCornerType, QPixmap> m_handleHoverPixmaps{};
    QMap<BIHandleCornerType, QPixmap> m_handleDisabledPixmaps{};
    QMap<BIHandleCornerType, QPainterPath> m_handlePaths{};

    BIHandleCorner m_handleCornerHover{BIHandleCorner::Invalid};

    bool m_controlsVisible{true};
    bool m_allowChangeMerge{false};
    bool m_transformationApplied{false};

    QPointF m_scaleDiff{};
    QTransform m_originalMatrix{};
    QRectF m_imageBoundingRect{};
    QRectF m_imageScreenBoundingRect{};
    QPointF m_rotationStartPoint{};

    bool m_showOrigin{false};
    QPointF m_originPos{};
    bool m_originSaved{false};
    bool m_customOrigin{false};

    void InitPixmaps();

    auto TopLeftHandlerPosition() const -> QPointF;
    auto TopHandlerPosition() const -> QPointF;
    auto TopRightHandlerPosition() const -> QPointF;
    auto RightHandlerPosition() const -> QPointF;
    auto BottomRightHandlerPosition() const -> QPointF;
    auto BottomHandlerPosition() const -> QPointF;
    auto BottomLeftHandlerPosition() const -> QPointF;
    auto LeftHandlerPosition() const -> QPointF;

    auto ControllerPath(BIHandleCornerType type, QPointF pos) const -> QPainterPath;
    auto ScaleTopLeftControl() const -> QPainterPath;
    auto ScaleTopControl() const -> QPainterPath;
    auto ScaleTopRightControl() const -> QPainterPath;
    auto ScaleRightControl() const -> QPainterPath;
    auto ScaleBottomRightControl() const -> QPainterPath;
    auto ScaleBottomControl() const -> QPainterPath;
    auto ScaleBottomLeftControl() const -> QPainterPath;
    auto ScaleLeftControl() const -> QPainterPath;

    auto RotateTopLeftControl() const -> QPainterPath;
    auto RotateTopRightControl() const -> QPainterPath;
    auto RotateBottomRightControl() const -> QPainterPath;
    auto RotateBottomLeftControl() const -> QPainterPath;

    auto ScaleByTopLeft(QGraphicsSceneMouseEvent * event) const -> QTransform;
    auto ScaleByTop(QGraphicsSceneMouseEvent * event) const -> QTransform;
    auto ScaleByTopRight(QGraphicsSceneMouseEvent * event) const -> QTransform;
    auto ScaleByRight(QGraphicsSceneMouseEvent * event) const -> QTransform;
    auto ScaleByBottomRight(QGraphicsSceneMouseEvent * event) const -> QTransform;
    auto ScaleByBottom(QGraphicsSceneMouseEvent * event) const -> QTransform;
    auto ScaleByBottomLeft(QGraphicsSceneMouseEvent * event) const -> QTransform;
    auto ScaleByLeft(QGraphicsSceneMouseEvent * event) const -> QTransform;

    auto Handles() const -> QPainterPath;
    auto ControllersRect() const -> QRectF;

    auto SelectedHandleCorner(const QPointF &pos) const -> BIHandleCorner;

    auto HandlerPixmap(bool hover, BIHandleCornerType type) const -> QPixmap;

    void ShowOrigin(const QPointF &pos);
    auto OriginCircle1() const -> QPainterPath;
    auto OriginCircle2() const -> QPainterPath;
    auto OriginPath() const -> QPainterPath;

    void ScaleImage(QGraphicsSceneMouseEvent * event);
    void RotateImage(QGraphicsSceneMouseEvent * event);

    void UpdateCursor(BIHandleCorner corner);
};

#endif // VBACKGROUNDIMAGECONTROLS_H
