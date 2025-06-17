/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 6, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#ifndef SCALESCENEITEMS_H
#define SCALESCENEITEMS_H

#include <QGraphicsLineItem>

#include "../vmisc/def.h"
#include "../vmisc/theme/themeDef.h"

class VScaledLine : public QGraphicsLineItem
{
public:
    explicit VScaledLine(VColorRole role, QGraphicsItem *parent = nullptr);
    VScaledLine(const QLineF &line, VColorRole role, QGraphicsItem *parent = nullptr);
    ~VScaledLine() override = default;

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Vis::ScaledLine)
    };

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    auto IsBoldLine() const -> bool;
    void SetBoldLine(bool bold);

    auto GetColorRole() const -> VColorRole;
    void SetColorRole(VColorRole role);

private:
    Q_DISABLE_COPY_MOVE(VScaledLine) // NOLINT

    bool m_isBoldLine{true};
    VColorRole m_role;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VScaledLine::GetColorRole() const -> VColorRole
{
    return m_role;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VScaledLine::SetColorRole(VColorRole role)
{
    m_role = role;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VScaledLine::IsBoldLine() const -> bool
{
    return m_isBoldLine;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VScaledLine::SetBoldLine(bool bold)
{
    m_isBoldLine = bold;
}

class VScaledEllipse : public QGraphicsEllipseItem
{
public:
    explicit VScaledEllipse(VColorRole role, QGraphicsItem *parent = nullptr);
    ~VScaledEllipse() override = default;

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Vis::ScaledEllipse)
    };

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    auto PointMode() const -> bool;
    void SetPointMode(bool newPointMode);

private:
    Q_DISABLE_COPY_MOVE(VScaledEllipse) // NOLINT

    bool m_pointMode{true};
    VColorRole m_role;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VScaledEllipse::PointMode() const -> bool
{
    return m_pointMode;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VScaledEllipse::SetPointMode(bool newPointMode)
{
    m_pointMode = newPointMode;
}

#endif // SCALESCENEITEMS_H
