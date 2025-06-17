/************************************************************************
 **
 **  @file   vistoolpointofintersectioncurves.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   24 1, 2016
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

#ifndef VISTOOLPOINTOFINTERSECTIONCURVES_H
#define VISTOOLPOINTOFINTERSECTIONCURVES_H


#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "vispath.h"

class VisToolPointOfIntersectionCurves : public VisPath
{
    Q_OBJECT // NOLINT
public:
    explicit VisToolPointOfIntersectionCurves(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolPointOfIntersectionCurves() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    void SetCurve1Id(quint32 value);
    void SetCurve2Id(quint32 value);
    void SetVCrossPoint(VCrossCurvesPoint value);
    void SetHCrossPoint(HCrossCurvesPoint value);

    auto type() const -> int override {return Type;}
    enum {Type = UserType + static_cast<int>(Vis::ToolPointOfIntersectionCurves)};
private:
    Q_DISABLE_COPY_MOVE(VisToolPointOfIntersectionCurves) // NOLINT
    quint32            m_curve1Id{NULL_ID};
    quint32            m_curve2Id{NULL_ID};
    VCrossCurvesPoint  m_vCrossPoint{VCrossCurvesPoint::HighestPoint};
    HCrossCurvesPoint  m_hCrossPoint{HCrossCurvesPoint::LeftmostPoint};
    VScaledEllipse    *m_point{nullptr};
    VCurvePathItem    *m_visCurve2{nullptr};
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolPointOfIntersectionCurves::SetCurve1Id(quint32 value)
{
    m_curve1Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolPointOfIntersectionCurves::SetCurve2Id(quint32 value)
{
    m_curve2Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolPointOfIntersectionCurves::SetVCrossPoint(VCrossCurvesPoint value)
{
    m_vCrossPoint = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolPointOfIntersectionCurves::SetHCrossPoint(HCrossCurvesPoint value)
{
    m_hCrossPoint = value;
}

#endif // VISTOOLPOINTOFINTERSECTIONCURVES_H
