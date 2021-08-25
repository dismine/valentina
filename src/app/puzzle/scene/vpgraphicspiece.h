/************************************************************************
 **
 **  @file   vpgraphicspiece.h
 **  @author Ronan Le Tiec
 **  @date   4 5, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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

#ifndef VPGRAPHICSPIECE_H
#define VPGRAPHICSPIECE_H

#include <QGraphicsItem>
#include <QCursor>

#include "scenedef.h"
#include "../layout/layoutdef.h"

class VPGraphicsPiece : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit VPGraphicsPiece(const VPPiecePtr &piece, QGraphicsItem *parent = nullptr);
    ~VPGraphicsPiece() = default;

    /**
     * @brief GetPiece Returns the piece that corresponds to the graphics piece
     * @return the piece
     */
    auto GetPiece() -> VPPiecePtr;

    virtual int type() const override {return Type;}
    enum { Type = UserType + static_cast<int>(PGraphicsItem::Piece)};

signals:
    void HideTransformationHandles(bool hide);
    void PieceTransformationChanged();

public slots:
    void on_RefreshPiece(const VPPiecePtr &piece);

protected:
    auto boundingRect() const -> QRectF override;
    auto shape() const -> QPainterPath override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    auto itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant override;

    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    Q_DISABLE_COPY(VPGraphicsPiece)
    VPPieceWeakPtr m_piece;

    QPainterPath m_cuttingLine{};
    QPainterPath m_seamLine{};
    QPainterPath m_grainline{};
    QPainterPath m_internalPaths{};
    QPainterPath m_passmarks{};
    QPainterPath m_placeLabels{};

    QPointF m_moveStartPoint{};
    QPointF m_rotationStartPoint{};

    QCursor m_rotateCursor{};

    bool allowChangeMerge{false};

    void PaintPiece(QPainter *painter=nullptr);

    void GroupMove(const QPointF &pos);
};

#endif // VPGRAPHICSPIECE_H
