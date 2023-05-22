/************************************************************************
 **
 **  @file   vtextgraphicsitem.h
 **  @author Bojan Kverh
 **  @date   June 16, 2016
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

#ifndef VTEXTGRAPHICSITEM_H
#define VTEXTGRAPHICSITEM_H

#include <QFont>
#include <QGraphicsObject>
#include <QList>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <QtGlobal>

#include "../vlayout/vtextmanager.h"
#include "vpieceitem.h"

/**
 * @brief The VTextGraphicsItem class. This class implements text graphics item,
 * which can be dragged around, resized and rotated within the parent item. The text font
 * size will be automatically updated, so that the entire text will fit into the item.
 */
class VTextGraphicsItem final : public VPieceItem
{
    Q_OBJECT // NOLINT

public:
    enum ItemType
    {
        PatternLabel,
        PieceLabel,
        Unknown
    };

    explicit VTextGraphicsItem(ItemType type, QGraphicsItem *pParent = nullptr);
    ~VTextGraphicsItem() override = default;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void Update() override;

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Vis::TextGraphicsItem)
    };

    void SetFont(const QFont &fnt);
    auto GetFontSize() const -> int;
    void SetSize(qreal fW, qreal fH);
    auto IsContained(QRectF rectBB, qreal dRot, qreal &dX, qreal &dY) const -> bool;
    void UpdateData(const QString &qsName, const VPieceLabelData &data, const VContainer *pattern);
    void UpdateData(VAbstractPattern *pDoc, const VContainer *pattern);
    auto GetTextLines() const -> vsizetype;
    void SetPieceName(const QString &name);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *pME) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *pME) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *pME) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent *pME) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *pHE) override;

    void UpdateBox();
    void CorrectLabel();

signals:
    void SignalResized(qreal iTW);
    void SignalRotated(qreal dAng);
    void SignalShrink();

private:
    Q_DISABLE_COPY_MOVE(VTextGraphicsItem) // NOLINT
    QPointF m_ptStartPos{};
    QPointF m_ptStart{};
    QSizeF m_szStart{};
    double m_dRotation{0};
    double m_dAngle{0};
    QRectF m_rectResize{};
    VTextManager m_tm{};
    QString m_pieceName{};
    ItemType m_itemType{Unknown};

    void AllUserModifications(const QPointF &pos);
    void UserRotateAndMove();
    void UserMoveAndResize(const QPointF &pos);

    void MoveLabel(QGraphicsSceneMouseEvent *pME);
    void ResizeLabel(QGraphicsSceneMouseEvent *pME);
    void RotateLabel(QGraphicsSceneMouseEvent *pME);

    void PaintLabel(QPainter *painter);
};

#endif // VTEXTGRAPHICSITEM_H
