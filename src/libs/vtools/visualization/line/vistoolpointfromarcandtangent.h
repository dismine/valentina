/************************************************************************
 **
 **  @file   vistoolpointfromarcandtangent.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#ifndef VISTOOLPOINTFROMARCANDTANGENT_H
#define VISTOOLPOINTFROMARCANDTANGENT_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "visline.h"

class VArc;

class VisToolPointFromArcAndTangent : public VisLine
{
    Q_OBJECT // NOLINT
public:
    explicit VisToolPointFromArcAndTangent(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolPointFromArcAndTangent() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    void SetPointId(quint32 newPointId);
    void SetArcId(const quint32 &value);
    void SetCrossPoint(const CrossCirclesPoint &value);

    auto type() const -> int override {return Type;}
    enum {Type = UserType + static_cast<int>(Vis::ToolPointFromArcAndTangent)};

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VisToolPointFromArcAndTangent) // NOLINT
    quint32            m_arcId{NULL_ID};
    CrossCirclesPoint  m_crossPoint{CrossCirclesPoint::FirstPoint};
    VScaledEllipse    *m_point{nullptr};
    VScaledEllipse    *m_tangent{nullptr};
    VCurvePathItem    *m_arcPath{nullptr};
    quint32            m_pointId{NULL_ID};

    void FindRays(const QPointF &p, const VArc *arc);
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolPointFromArcAndTangent::SetPointId(quint32 newPointId)
{
    m_pointId = newPointId;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolPointFromArcAndTangent::SetArcId(const quint32 &value)
{
    m_arcId = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolPointFromArcAndTangent::SetCrossPoint(const CrossCirclesPoint &value)
{
    m_crossPoint = value;
}

#endif // VISTOOLPOINTFROMARCANDTANGENT_H
