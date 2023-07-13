/************************************************************************
 **
 **  @file   vistoolcubicbezier.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 3, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#ifndef VISTOOLCUBICBEZIER_H
#define VISTOOLCUBICBEZIER_H


#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "vispath.h"

class VisToolCubicBezier : public VisPath
{
    Q_OBJECT // NOLINT
public:
    explicit VisToolCubicBezier(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolCubicBezier() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    void SetPoint1Id(quint32 value);
    void SetPoint2Id(quint32 value);
    void SetPoint3Id(quint32 value);
    void SetPoint4Id(quint32 value);

    auto type() const -> int override {return Type;}
    enum {Type = UserType + static_cast<int>(Vis::ToolCubicBezier)};
private:
    Q_DISABLE_COPY_MOVE(VisToolCubicBezier) // NOLINT
    quint32         m_point1Id{NULL_ID};
    quint32         m_point2Id{NULL_ID};
    quint32         m_point3Id{NULL_ID};
    quint32         m_point4Id{NULL_ID};
    VScaledEllipse *m_point1{nullptr};
    VScaledEllipse *m_point2{nullptr};
    VScaledEllipse *m_point3{nullptr};
    VScaledEllipse *m_point4{nullptr};
    VScaledLine    *m_helpLine1{nullptr};
    VScaledLine    *m_helpLine2{nullptr};
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolCubicBezier::SetPoint1Id(quint32 value)
{
    m_point1Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolCubicBezier::SetPoint2Id(quint32 value)
{
    m_point2Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolCubicBezier::SetPoint3Id(quint32 value)
{
    m_point3Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolCubicBezier::SetPoint4Id(quint32 value)
{
    m_point4Id = value;
}
#endif // VISTOOLCUBICBEZIER_H
