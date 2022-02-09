/************************************************************************
 **
 **  @file   vpgraphicspiececontrols.h
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
#ifndef VPGRAPHICSPIECECONTROLS_H
#define VPGRAPHICSPIECECONTROLS_H

#include <QColor>
#include <QGraphicsObject>

#include "scenedef.h"
#include "../layout/vpsheet.h"

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 13, 0)

class VPLayout;
class VPGraphicsPiece;
class QGraphicsView;

enum class VPHandleCorner : int
{
    Invalid = 0,
    TopLeft = 1,
    TopRight = 2,
    BottomRight = 3,
    BottomLeft = 4
};

enum class VPHandleCornerType
{
    TopLeft,
    TopRight,
    BottomRight,
    BottomLeft
};

class VPGraphicsTransformationOrigin : public QGraphicsObject
{
    Q_OBJECT // NOLINT
public:
    explicit VPGraphicsTransformationOrigin(const VPLayoutPtr &layout, QGraphicsItem * parent = nullptr);

    auto type() const -> int override {return Type;}
    enum { Type = UserType + static_cast<int>(PGraphicsItem::TransformationOrigin)};

public slots:
    void SetTransformationOrigin();
    void on_HideHandles(bool hide);
    void on_ShowOrigin(bool show);

protected:
    auto boundingRect() const -> QRectF override;
    auto shape() const -> QPainterPath override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    Q_DISABLE_COPY_MOVE(VPGraphicsTransformationOrigin) // NOLINT

    bool            m_originVisible{true};
    VPLayoutWeakPtr m_layout{};
    QColor          m_color;
    bool            m_allowChangeMerge{false};

    auto RotationCenter(QPainter *painter = nullptr) const -> QPainterPath;
    auto Center1() const -> QPainterPath;
    auto Center2() const -> QPainterPath;
};

class VPGraphicsPieceControls : public QGraphicsObject
{
    Q_OBJECT // NOLINT
public:
    explicit VPGraphicsPieceControls(const VPLayoutPtr &layout, QGraphicsItem * parent = nullptr);

    auto type() const -> int override {return Type;}
    enum { Type = UserType + static_cast<int>(PGraphicsItem::Handles)};

    void SetIgnorePieceTransformation(bool newIgnorePieceTransformation);

signals:
    void ShowOrigin(bool show);
    void TransformationOriginChanged();

public slots:
    void on_UpdateControls();
    void on_HideHandles(bool hide);

protected:
    auto boundingRect() const -> QRectF override;
    auto shape() const -> QPainterPath override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

private:
    Q_DISABLE_COPY_MOVE(VPGraphicsPieceControls) // NOLINT

    QRectF          m_pieceRect{};
    QPointF         m_rotationStartPoint{};
    qreal           m_rotationSum{0};
    bool            m_controlsVisible{false};
    VPLayoutWeakPtr m_layout{};
    VPHandleCorner  m_handleCorner{VPHandleCorner::Invalid};
    VPTransformationOrigon m_savedOrigin{};
    bool            m_originSaved{false};
    bool            allowChangeMerge{false};
    QList<VPPiecePtr> m_selectedPieces{};
    bool            m_ignorePieceTransformation{false};

    QMap<VPHandleCornerType, QPixmap> m_handlePixmaps{};
    QMap<VPHandleCornerType, QPixmap> m_handleHoverPixmaps{};
    QMap<VPHandleCornerType, QPainterPath> m_handlePaths{};

    void InitPixmaps();

    auto TopLeftHandlerPosition() const -> QPointF;
    auto TopRightHandlerPosition() const -> QPointF;
    auto BottomRightHandlerPosition() const -> QPointF;
    auto BottomLeftHandlerPosition() const -> QPointF;

    auto ControllerPath(VPHandleCornerType type, QPointF pos) const -> QPainterPath;
    auto TopLeftControl() const -> QPainterPath;
    auto TopRightControl() const -> QPainterPath;
    auto BottomRightControl() const -> QPainterPath;
    auto BottomLeftControl() const -> QPainterPath;

    auto Handles() const -> QPainterPath;
    auto ControllersRect() const -> QRectF;

    auto SelectedHandleCorner(const QPointF &pos) const -> VPHandleCorner;

    auto HandlerPixmap(bool hover, VPHandleCornerType type) const -> QPixmap;

    auto SelectedPieces() const -> QList<VPPiecePtr>;
    static auto PiecesBoundingRect(const QList<VPPiecePtr> &selectedPieces) -> QRectF;

    auto ItemView() -> QGraphicsView *;

    void UpdateCursor(VPHandleCorner corner);

    void PrepareTransformationOrigin(bool shiftPressed);
    void CorrectRotationSum(const VPLayoutPtr &layout, const VPTransformationOrigon &rotationOrigin, qreal rotateOn);
};

#endif // VPGRAPHICSPIECECONTROLS_H
