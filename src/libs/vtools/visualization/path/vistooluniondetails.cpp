/************************************************************************
 **
 **  @file   vistooluniondetails.cpp
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

#include "vistooluniondetails.h"

#include <QPointF>

#include "../ifc/exception/vexceptionbadid.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiece.h"
#include "../vpatterndb/vpiecepath.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vwidgets/scalesceneitems.h"
#include "../vwidgets/vcurvepathitem.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolUnionDetails::VisToolUnionDetails(const VContainer *data, QGraphicsItem *parent)
  : VisPath(data, parent)
{
    m_piece2 = InitItem<VCurvePathItem>(VColorRole::VisSupportColor, this);
    m_point1 = InitPoint(VColorRole::VisSupportColor, this);
    m_point2 = InitPoint(VColorRole::VisSupportColor, this);
    m_point3 = InitPoint(VColorRole::VisSupportColor, this);
    m_point4 = InitPoint(VColorRole::VisSupportColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolUnionDetails::RefreshGeometry()
{
    m_piece2->setVisible(false);
    m_point1->setVisible(false);
    m_point2->setVisible(false);
    m_point3->setVisible(false);
    m_point4->setVisible(false);

    try
    {
        if (m_d1Id != NULL_ID)
        {
            const VPiece d1 = GetData()->GetPiece(m_d1Id);
            const QPointF d1Offset(d1.GetMx(), d1.GetMy());

            DrawPath(this, d1.MainPathPath(GetData()).translated(d1Offset), Qt::SolidLine, Qt::RoundCap);

            const VPiecePath d1Path = d1.GetPath();

            if (m_indexD1 < 0)
            {
                // d1 edge not confirmed yet — show p1 if selected (p1 is a point on d1)
                if (m_p1Id != NULL_ID)
                {
                    DrawPoint(m_point1,
                              static_cast<QPointF>(*GetData()->GeometricObject<VPointF>(m_p1Id)) + d1Offset);
                }
            }
            else
            {
                // d1 edge confirmed — show both edge endpoints at their visual positions
                VPieceNode det1p1;
                VPieceNode det1p2;
                d1Path.NodeOnEdge(static_cast<quint32>(m_indexD1), det1p1, det1p2);
                DrawPoint(m_point1,
                          static_cast<QPointF>(*GetData()->GeometricObject<VPointF>(det1p1.GetId())) + d1Offset);
                DrawPoint(m_point2,
                          static_cast<QPointF>(*GetData()->GeometricObject<VPointF>(det1p2.GetId())) + d1Offset);
            }
        }

        if (m_d2Id != NULL_ID)
        {
            const VPiece d2 = GetData()->GetPiece(m_d2Id);
            const QPointF d2Offset(d2.GetMx(), d2.GetMy());

            DrawPath(m_piece2, d2.MainPathPath(GetData()).translated(d2Offset), Qt::SolidLine, Qt::RoundCap);
            m_piece2->setVisible(true);

            if (m_indexD2 >= 0)
            {
                // d2 edge confirmed — show both edge endpoints
                VPieceNode det2p1;
                VPieceNode det2p2;
                d2.GetPath().NodeOnEdge(static_cast<quint32>(m_indexD2), det2p1, det2p2);
                DrawPoint(m_point3,
                          static_cast<QPointF>(*GetData()->GeometricObject<VPointF>(det2p1.GetId())) + d2Offset);
                DrawPoint(m_point4,
                          static_cast<QPointF>(*GetData()->GeometricObject<VPointF>(det2p2.GetId())) + d2Offset);
            }
            else if (m_indexD1 >= 0 && m_p1Id != NULL_ID)
            {
                // d2 edge not yet confirmed — show first selected point on d2
                DrawPoint(m_point3,
                          static_cast<QPointF>(*GetData()->GeometricObject<VPointF>(m_p1Id)) + d2Offset);
            }
        }
    }
    catch (const VExceptionBadId &)
    {
        // Silently ignore bad IDs during interactive selection
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolUnionDetails::VisualMode(quint32 id)
{
    Q_UNUSED(id)
    StartVisualMode();
}
