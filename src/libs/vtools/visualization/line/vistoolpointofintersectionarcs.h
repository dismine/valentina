/************************************************************************
 **
 **  @file   vistoolpointofintersectionarcs.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 5, 2015
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

#ifndef VISTOOLPOINTOFINTERSECTIONARCS_H
#define VISTOOLPOINTOFINTERSECTIONARCS_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../ifc/xml/vabstractpattern.h"
#include "../vmisc/def.h"
#include "visline.h"

class VisToolPointOfIntersectionArcs : public VisLine
{
    Q_OBJECT // NOLINT
public:
    explicit VisToolPointOfIntersectionArcs(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolPointOfIntersectionArcs() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    void SetArc1Id(quint32 value);
    void SetArc2Id(quint32 value);
    void SetCrossPoint(CrossCirclesPoint value);

    auto type() const -> int override {return Type;}
    enum {Type = UserType + static_cast<int>(Vis::ToolPointOfIntersectionArcs)};
private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VisToolPointOfIntersectionArcs) // NOLINT
    quint32            m_arc1Id{NULL_ID};
    quint32            m_arc2Id{NULL_ID};
    CrossCirclesPoint  m_crossPoint{CrossCirclesPoint::FirstPoint};
    VScaledEllipse    *m_point{nullptr};
    VCurvePathItem    *m_arc1Path{nullptr};
    VCurvePathItem    *m_arc2Path{nullptr};
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolPointOfIntersectionArcs::SetArc1Id(quint32 value)
{
    m_arc1Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolPointOfIntersectionArcs::SetArc2Id(quint32 value)
{
    m_arc2Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolPointOfIntersectionArcs::SetCrossPoint(CrossCirclesPoint value)
{
    m_crossPoint = value;
}

#endif // VISTOOLPOINTOFINTERSECTIONARCS_H
