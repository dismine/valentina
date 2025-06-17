/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   31 1, 2017
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

#ifndef VISTOOLSPECIALPOINT_H
#define VISTOOLSPECIALPOINT_H

#include "visline.h"

class VSimplePoint;

class VisToolSpecialPoint : public VisLine
{
    Q_OBJECT // NOLINT
public:
    explicit VisToolSpecialPoint(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolSpecialPoint() override;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    void SetPointId(quint32 newPointId);

    auto type() const -> int override {return Type;}
    enum {Type = UserType + static_cast<int>(Vis::ToolSpecialPoint)};
private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VisToolSpecialPoint) // NOLINT
    QPointer<VSimplePoint> m_point{};
    quint32                m_pointId{NULL_ID};
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolSpecialPoint::SetPointId(quint32 newPointId)
{
    m_pointId = newPointId;
}

#endif // VISTOOLSPECIALPOINT_H
