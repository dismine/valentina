/************************************************************************
 **
 **  @file   vgraphicsfillitem.h
 **  @author Bojan Kverh
 **  @date   October 16, 2016
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

#ifndef VGRAPHICSFILLITEM_H
#define VGRAPHICSFILLITEM_H

#include <QGraphicsPathItem>
#include <QPainter>

#include "../vmisc/def.h"

class VGraphicsFillItem : public QGraphicsPathItem
{
public:
    /**
     * @brief VGraphicsFillItem Constructor
     */
    explicit VGraphicsFillItem(QGraphicsItem *parent = nullptr);
    explicit VGraphicsFillItem(const QPainterPath &path, QGraphicsItem *parent = nullptr);
    ~VGraphicsFillItem() override = default;

    /**
     * @brief paint Paints the item, filling the inside surface
     * @param painter pointer to the painter object
     * @param option unused
     * @param widget unused
     */
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Layout::GrainlineItem)
    };

    auto GetWidth() const -> qreal;
    void SetWidth(const qreal &value);

    auto CustomPen() const -> bool;
    void SetCustomPen(bool newCustomPen);

private:
    Q_DISABLE_COPY_MOVE(VGraphicsFillItem) // NOLINT
    qreal width{1};
    bool m_customPen{false};
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VGraphicsFillItem::GetWidth() const -> qreal
{
    return width;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VGraphicsFillItem::SetWidth(const qreal &value)
{
    width = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VGraphicsFillItem::CustomPen() const -> bool
{
    return m_customPen;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VGraphicsFillItem::SetCustomPen(bool newCustomPen)
{
    m_customPen = newCustomPen;
}

class VGraphicsFoldLineItem : public VGraphicsFillItem
{
public:
    explicit VGraphicsFoldLineItem(QGraphicsItem *parent = nullptr);
    explicit VGraphicsFoldLineItem(const QPainterPath &path, QGraphicsItem *parent = nullptr);
    ~VGraphicsFoldLineItem() override = default;

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Layout::FoldLineItem)
    };

private:
    Q_DISABLE_COPY_MOVE(VGraphicsFoldLineItem) // NOLINT
};

#endif // VGRAPHICSFILLITEM_H
