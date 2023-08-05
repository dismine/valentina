/************************************************************************
 **
 **  @file   vistoolpointofintersectioncurves.cpp
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

#include "vistoolpointofintersectioncurves.h"

#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPointF>
#include <QSharedPointer>
#include <new>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersectioncurves.h"
#include "../../visualization/path/vispath.h"
#include "../vgeometry/vabstractcurve.h"
#include "../visualization.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/scalesceneitems.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolPointOfIntersectionCurves::VisToolPointOfIntersectionCurves(const VContainer *data, QGraphicsItem *parent)
  : VisPath(data, parent)
{
    SetColorRole(VColorRole::VisSupportColor);

    m_visCurve2 = InitItem<VCurvePathItem>(VColorRole::VisSupportColor, this);
    m_visCurve2->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
    m_point = InitPoint(VColorRole::VisMainColor, this);
    m_point->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
    m_point->setZValue(1);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfIntersectionCurves::RefreshGeometry()
{
    if (m_curve1Id > NULL_ID)
    {
        auto curve1 = GetData()->GeometricObject<VAbstractCurve>(m_curve1Id);
        DrawPath(this, curve1->GetPath(), curve1->DirectionArrows(), Qt::SolidLine, Qt::RoundCap);

        if (m_curve2Id > NULL_ID)
        {
            auto curve2 = GetData()->GeometricObject<VAbstractCurve>(m_curve2Id);
            DrawPath(m_visCurve2, curve2->GetPath(), curve2->DirectionArrows(), Qt::SolidLine, Qt::RoundCap);

            QPointF p;
            VToolPointOfIntersectionCurves::FindPoint(curve1->GetPoints(), curve2->GetPoints(), m_vCrossPoint,
                                                      m_hCrossPoint, &p);
            DrawPoint(m_point, p);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfIntersectionCurves::VisualMode(quint32 id)
{
    m_curve1Id = id;
    StartVisualMode();
}
