/************************************************************************
 **
 **  @file   vistooluniondetails.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 5, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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

#ifndef VISTOOLUNIONDETAILS_H
#define VISTOOLUNIONDETAILS_H

#include <QMetaObject>
#include <QObject>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "vispath.h"

class VCurvePathItem;
class VScaledEllipse;

class VisToolUnionDetails : public VisPath
{
    Q_OBJECT // NOLINT

public:
    explicit VisToolUnionDetails(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolUnionDetails() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    void SetD1Id(quint32 id);
    void SetD2Id(quint32 id);
    void SetIndexD1(vsizetype index);
    void SetIndexD2(vsizetype index);
    void SetP1Id(quint32 id);

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Vis::ToolUnionDetails)
    };

private:
    Q_DISABLE_COPY_MOVE(VisToolUnionDetails) // NOLINT

    quint32 m_d1Id{NULL_ID};
    quint32 m_d2Id{NULL_ID};
    quint32 m_p1Id{NULL_ID};
    vsizetype m_indexD1{-1};
    vsizetype m_indexD2{-1};
    VCurvePathItem *m_piece2{nullptr};
    VScaledEllipse *m_point1{nullptr};
    VScaledEllipse *m_point2{nullptr};
    VScaledEllipse *m_point3{nullptr};
    VScaledEllipse *m_point4{nullptr};
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolUnionDetails::SetD1Id(quint32 id)
{
    m_d1Id = id;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolUnionDetails::SetD2Id(quint32 id)
{
    m_d2Id = id;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolUnionDetails::SetIndexD1(vsizetype index)
{
    m_indexD1 = index;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolUnionDetails::SetIndexD2(vsizetype index)
{
    m_indexD2 = index;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolUnionDetails::SetP1Id(quint32 id)
{
    m_p1Id = id;
}

#endif // VISTOOLUNIONDETAILS_H
