/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 6, 2017
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

#ifndef VSCENEPOINT_H
#define VSCENEPOINT_H

#include <QGraphicsEllipseItem>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "../vmisc/theme/themeDef.h"

class VGraphicsSimpleTextItem;
class VPointF;
class VScaledLine;

class VScenePoint : public QGraphicsEllipseItem
{
public:
    explicit VScenePoint(VColorRole role, QGraphicsItem *parent = nullptr);
    ~VScenePoint() override = default;

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Vis::ScenePoint)
    };

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    virtual void RefreshPointGeometry(const VPointF &point);
    auto boundingRect() const -> QRectF override;

    void RefreshLine();

    auto GetColorRole() const -> VColorRole;
    void SetColorRole(VColorRole role);

    auto GetLabelLineColorRole() const -> VColorRole;
    void SetLabelLineColorRole(VColorRole role);

    auto GetLabelTextColorRole() const -> VColorRole;
    void SetLabelTextColorRole(VColorRole role);

    auto GetLabelTextHoverColorRole() const -> VColorRole;
    void SetLabelTextHoverColorRole(VColorRole role);

protected:
    /** @brief namePoint point label. */
    VGraphicsSimpleTextItem *m_namePoint; // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)

    /** @brief lineName line what we see if label moved too away from point. */
    VScaledLine *m_lineName; // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)

    bool m_onlyPoint{false}; // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)
    bool m_isHovered{false}; // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)
    bool m_showLabel{true};  // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)

    bool m_selectedFromChild{false}; // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)

    void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

    void SetOnlyPoint(bool value);
    auto IsOnlyPoint() const -> bool;

private:
    Q_DISABLE_COPY_MOVE(VScenePoint) // NOLINT

    void ScaleMainPenWidth(qreal scale);

    VColorRole m_role;
};

//---------------------------------------------------------------------------------------------------------------------
inline auto VScenePoint::GetColorRole() const -> VColorRole
{
    return m_role;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VScenePoint::SetColorRole(VColorRole role)
{
    m_role = role;
}

#endif // VSCENEPOINT_H
