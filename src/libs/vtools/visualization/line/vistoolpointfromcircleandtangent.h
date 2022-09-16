/************************************************************************
 **
 **  @file   vistoolpointfromcircleandtangent.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   5 6, 2015
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

#ifndef VISTOOLPOINTFROMCIRCLEANDTANGENT_H
#define VISTOOLPOINTFROMCIRCLEANDTANGENT_H

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

class VisToolPointFromCircleAndTangent : public VisLine
{
    Q_OBJECT // NOLINT
public:
    explicit VisToolPointFromCircleAndTangent(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolPointFromCircleAndTangent() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    void SetPointId(quint32 value);
    void SetCenterId(quint32 value);
    void SetCRadius(const QString &value);
    void SetCrossPoint(CrossCirclesPoint value);

    auto type() const -> int override {return Type;}
    enum {Type = UserType + static_cast<int>(Vis::ToolPointFromCircleAndTangent)};
private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VisToolPointFromCircleAndTangent) // NOLINT
    quint32               m_centerId{NULL_ID};
    qreal                 m_cRadius{0};
    CrossCirclesPoint     m_crossPoint{CrossCirclesPoint::FirstPoint};
    VScaledEllipse       *m_point{nullptr};
    VScaledEllipse       *m_tangent{nullptr};
    VScaledEllipse       *m_cCenter{nullptr};
    VScaledEllipse       *m_cPath{nullptr};
    VScaledLine          *m_tangent2{nullptr};
    quint32               m_pointId{NULL_ID};

    void FindRays(const QPointF &p, const QPointF &center, qreal radius);
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolPointFromCircleAndTangent::SetPointId(quint32 value)
{
    m_pointId = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolPointFromCircleAndTangent::SetCenterId(quint32 value)
{
    m_centerId = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolPointFromCircleAndTangent::SetCrossPoint(CrossCirclesPoint value)
{
    m_crossPoint = value;
}

#endif // VISTOOLPOINTFROMCIRCLEANDTANGENT_H
