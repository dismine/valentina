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

class VPLayout;
class VPGraphicsPiece;

class VPGraphicsTransformationOrigin : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit VPGraphicsTransformationOrigin(const VPLayoutPtr &layout, QGraphicsItem * parent = nullptr);

    virtual int type() const override {return Type;}
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
    Q_DISABLE_COPY(VPGraphicsTransformationOrigin)

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
    Q_OBJECT
public:
    explicit VPGraphicsPieceControls(const VPLayoutPtr &layout, QGraphicsItem * parent = nullptr);

    virtual int type() const override {return Type;}
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

    void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    Q_DISABLE_COPY(VPGraphicsPieceControls)
    QRectF          m_pieceRect{};
    QPointF         m_rotationStartPoint{};
    bool            m_controlsVisible{true};
    VPLayoutWeakPtr m_layout{};
    int             m_handleCorner{0};
    VPTransformationOrigon m_savedOrigin{};
    bool            m_originSaved{false};
    bool            allowChangeMerge{false};
    QVector<VPGraphicsPiece *> m_selectedPieces{};
    bool            m_ignorePieceTransformation{false};

    auto TopLeftControl(QPainter *painter = nullptr) const -> QPainterPath;
    auto TopRightControl(QPainter *painter = nullptr) const -> QPainterPath;
    auto BottomLeftControl(QPainter *painter = nullptr) const -> QPainterPath;
    auto BottomRightControl(QPainter *painter = nullptr) const -> QPainterPath;

    auto Handles() const -> QPainterPath;
    auto Controller(const QTransform &t, QPainter *painter = nullptr) const -> QPainterPath;
    auto ControllersRect() const -> QRectF;

    auto ArrowPath() const -> QPainterPath;

    auto HandleCorner(const QPointF &pos) const -> int;

    auto SelectedPieces() const -> QVector<VPGraphicsPiece *>;
    static auto PiecesBoundingRect(const QVector<VPGraphicsPiece *> &selectedPieces) -> QRectF;
};

#endif // VPGRAPHICSPIECECONTROLS_H
