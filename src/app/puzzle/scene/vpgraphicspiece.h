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

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#include "../vmisc/defglobal.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 13, 0)

class VTextManager;

class VPGraphicsPiece : public QGraphicsObject
{
    Q_OBJECT // NOLINT
public:
    explicit VPGraphicsPiece(const VPPiecePtr &piece, QGraphicsItem *parent = nullptr);
    ~VPGraphicsPiece() override = default;

    /**
     * @brief GetPiece Returns the piece that corresponds to the graphics piece
     * @return the piece
     */
    auto GetPiece() -> VPPiecePtr;

    auto type() const -> int override {return Type;}
    enum { Type = UserType + static_cast<int>(PGraphicsItem::Piece)};

    void SetStickyPoints(const QVector<QPointF> &newStickyPoint);

    void SetTextAsPaths(bool newTextAsPaths);

signals:
    void HideTransformationHandles(bool hide);
    void PieceTransformationChanged();

public slots:
    void on_RefreshPiece(const VPPiecePtr &piece);
    void PieceZValueChanged(const VPPiecePtr &piece);

protected:
    auto boundingRect() const -> QRectF override;
    auto shape() const -> QPainterPath override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void mousePressEvent(QGraphicsSceneMouseEvent * event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    auto itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant override;

    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event) override;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VPGraphicsPiece) // NOLINT
    VPPieceWeakPtr m_piece;

    QPainterPath m_cuttingLine{};
    QPainterPath m_seamLine{};
    QPainterPath m_grainline{};
    QPainterPath m_internalPaths{};
    QPainterPath m_passmarks{};
    QPainterPath m_placeLabels{};

    QPointF m_moveStartPoint{};
    QPointF m_rotationStartPoint{};

    bool m_allowChangeMerge{false};

    QVector<QPointF> m_stickyPoints{};
    QPainterPath m_stickyPath{};

    bool m_hasStickyPosition{false};
    qreal m_stickyTranslateX{0};
    qreal m_stickyTranslateY{0};

    bool m_textAsPaths{false};

    bool m_hoverMode{false};

    QVector<QGraphicsPathItem *> m_labelPathItems{};
    QVector<QGraphicsSimpleTextItem *> m_labelTextItems{};

    void InitLabels();
    void InitPieceLabel(const QVector<QPointF> &labelShape, const VTextManager &tm);
    void PaintPiece(QPainter *painter=nullptr);
    void PaintSeamLine(QPainter *painter, const VPPiecePtr &piece);
    void PaintCuttingLine(QPainter *painter, const VPPiecePtr &piece);
    void PaintGrainline(QPainter *painter, const VPPiecePtr &piece);
    void PaintInternalPaths(QPainter *painter, const VPPiecePtr &piece);
    void PaintPassmarks(QPainter *painter, const VPPiecePtr &piece);
    void PaintPlaceLabels(QPainter *painter, const VPPiecePtr &piece);
    void PaintStickyPath(QPainter *painter);

    void GroupMove(const QPointF &pos);

    auto PieceColor() const -> QColor;

    auto NoBrush() const -> QBrush;
};

#endif // VPGRAPHICSPIECE_H
