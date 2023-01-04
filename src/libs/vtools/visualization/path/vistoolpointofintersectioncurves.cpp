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
#include <Qt>
#include <new>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersectioncurves.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/scalesceneitems.h"
#include "../visualization.h"
#include "../../visualization/path/vispath.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolPointOfIntersectionCurves::VisToolPointOfIntersectionCurves(const VContainer *data, QGraphicsItem *parent)
    :VisPath(data, parent)
{
    m_visCurve2 = InitItem<VCurvePathItem>(Color(VColor::SupportColor), this);
    m_point = InitPoint(Color(VColor::MainColor), this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfIntersectionCurves::RefreshGeometry()
{
    if (m_curve1Id > NULL_ID)
    {
        auto curve1 = GetData()->GeometricObject<VAbstractCurve>(m_curve1Id);
        DrawPath(this, curve1->GetPath(), curve1->DirectionArrows(), Color(VColor::SupportColor), Qt::SolidLine,
                 Qt::RoundCap);

        if (m_curve2Id > NULL_ID)
        {
            auto curve2 = GetData()->GeometricObject<VAbstractCurve>(m_curve2Id);
            DrawPath(m_visCurve2, curve2->GetPath(), curve2->DirectionArrows(), Color(VColor::SupportColor),
                     Qt::SolidLine, Qt::RoundCap);

            QPointF p;
            VToolPointOfIntersectionCurves::FindPoint(curve1->GetPoints(), curve2->GetPoints(), m_vCrossPoint,
                                                      m_hCrossPoint, &p);
            DrawPoint(m_point, p, Color(VColor::MainColor));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfIntersectionCurves::VisualMode(quint32 id)
{
    m_curve1Id = id;
    StartVisualMode();
}
