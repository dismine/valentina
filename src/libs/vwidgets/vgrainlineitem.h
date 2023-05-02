/************************************************************************
 **
 **  @file   vgrainlineitem.h
 **  @author Bojan Kverh
 **  @date   September 10, 2016
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

#ifndef VGRAINLINEITEM_H
#define VGRAINLINEITEM_H

#include "vpieceitem.h"
#include "../vpatterndb/floatItemData/floatitemdef.h"
#include "../vmisc/def.h"

class VPieceGrainline;

class VGrainlineItem final : public VPieceItem
{
    Q_OBJECT // NOLINT
public:
    explicit VGrainlineItem(QGraphicsItem* pParent = nullptr);
    ~VGrainlineItem() override = default;

    auto shape() const -> QPainterPath override;

    void paint(QPainter* pP, const QStyleOptionGraphicsItem* pOption, QWidget* pWidget) override;
    void UpdateGeometry(const QPointF& ptPos, qreal dRotation, qreal dLength, GrainlineArrowDirection eAT);

    auto type() const -> int override {return Type;}
    enum { Type = UserType + static_cast<int>(Vis::GrainlineItem)};

    auto Grainline() const -> VPieceGrainline;

signals:
    void SignalResized(qreal dLength);
    void SignalRotated(qreal dRot, const QPointF& ptNewPos);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* pME) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* pME) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* pME) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* pME) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* pME) override;
    void Update() override;
    void UpdateRectangle();

    auto GetAngle(const QPointF &pt) const -> double override;

    static auto Rotate(const QPointF& pt, const QPointF& ptCenter, qreal dAng) -> QPointF;
    auto GetInsideCorner(int i, qreal dDist) const -> QPointF;

private:
    Q_DISABLE_COPY_MOVE(VGrainlineItem) // NOLINT
    qreal                   m_dRotation{0};
    qreal                   m_dStartRotation{0};
    qreal                   m_dLength{0};
    QPolygonF               m_polyBound{};
    QPointF                 m_ptStartPos{};
    QPointF                 m_ptStartMove{};
    QPolygonF               m_polyResize{};
    qreal                   m_dStartLength{0};
    QPointF                 m_ptStart{};
    QPointF                 m_ptFinish{};
    QPointF                 m_ptSecondaryStart{};
    QPointF                 m_ptSecondaryFinish{};
    QPointF                 m_ptCenter{};
    qreal                   m_dAngle{0};
    GrainlineArrowDirection m_eArrowType{GrainlineArrowDirection::twoWaysUpDown};
    double                  m_penWidth{1};

    auto MainShape() const -> QPainterPath;

    void AllUserModifications(const QPointF &pos);
    void UserRotateAndMove();
    void UserMoveAndResize(const QPointF &pos);

    void UpdatePolyResize();
};

#endif // VGRAINLINEITEM_H
