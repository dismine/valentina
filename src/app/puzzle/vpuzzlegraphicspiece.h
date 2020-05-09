/************************************************************************
 **
 **  @file   vpuzzlegraphicspiece.h
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

#ifndef VPUZZLEGRAPHICSPIECE_H
#define VPUZZLEGRAPHICSPIECE_H

#include <QGraphicsItem>

class VPuzzlePiece;

class VPuzzleGraphicsPiece : public QGraphicsObject
{
    Q_OBJECT
public:
    VPuzzleGraphicsPiece(VPuzzlePiece *piece, QGraphicsItem *parent = nullptr);
    ~VPuzzleGraphicsPiece();
    void Init();

    /**
     * @brief GetPiece Returns the piece that corresponds to the graphics piece
     * @return the piece
     */
    VPuzzlePiece* GetPiece();

public slots:
    /**
     * @brief on_PieceSelectionChanged Slot called when the piece selection was changed
     */
    void on_PieceSelectionChanged();

    /**
     * @brief on_PiecePositionChanged Slot called when the piece position was changed
     */
    void on_PiecePositionChanged();

    /**
     * @brief on_PieceRotationChanged Slot called when the piece rotation was changed
     */
    void on_PieceRotationChanged();

protected:
    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget) override;

    void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent * event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private slots:
    /**
     * @brief on_ActionPieceMovedToLayer Slot called when the piece is moved via the
     * context menu to anoter layer
     */
    void on_ActionPieceMovedToLayer();

private:
    Q_DISABLE_COPY(VPuzzleGraphicsPiece)
    VPuzzlePiece *m_piece;

    QPainterPath m_cuttingLine;
    QPainterPath m_seamLine;
    QPainterPath m_grainline;

    QPointF m_rotationStartPoint;

};

#endif // VPUZZLEGRAPHICSPIECE_H
