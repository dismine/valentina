/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 5, 2017
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

#ifndef VCURVEPATHITEM_H
#define VCURVEPATHITEM_H

#include <QGraphicsPathItem>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "../vmisc/theme/themeDef.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")
QT_WARNING_DISABLE_GCC("-Wsuggest-final-methods")

class VCurvePathItem : public QGraphicsPathItem
{
public:
    explicit VCurvePathItem(VColorRole role, QGraphicsItem *parent = nullptr);
    ~VCurvePathItem() override = default;

    auto shape() const -> QPainterPath override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Vis::CurvePathItem)
    };

    void SetDirectionArrows(const QVector<QPair<QLineF, QLineF>> &arrows);
    void SetPoints(const QVector<QPointF> &points);
    void SetWidth(qreal width);

protected:
    virtual void ScalePenWidth();

    auto GetColorRole() const -> VColorRole;
    void SetColorRole(VColorRole role);

private:
    Q_DISABLE_COPY_MOVE(VCurvePathItem) // NOLINT

    QVector<QPair<QLineF, QLineF>> m_directionArrows{};
    QVector<QPointF> m_points{};
    qreal m_defaultWidth;
    VColorRole m_role;
};

//---------------------------------------------------------------------------------------------------------------------
inline void VCurvePathItem::SetColorRole(VColorRole role)
{
    m_role = role;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VCurvePathItem::GetColorRole() const -> VColorRole
{
    return m_role;
}

QT_WARNING_POP

#endif // VCURVEPATHITEM_H
