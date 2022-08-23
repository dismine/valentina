/************************************************************************
 **
 **  @file   vistoolpointofintersectionarcs.cpp
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

#include "vistoolpointofintersectionarcs.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPen>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <new>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersectionarcs.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/varc.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "visualization/line/visline.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolPointOfIntersectionArcs::VisToolPointOfIntersectionArcs(const VContainer *data, QGraphicsItem *parent)
    : VisLine(data, parent)
{
    this->setPen(QPen(Qt::NoPen)); // don't use parent this time

    m_arc1Path = InitItem<VCurvePathItem>(Qt::darkGreen, this);
    m_arc1Path->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
    m_arc2Path = InitItem<VCurvePathItem>(Qt::darkRed, this);
    m_arc2Path->setFlag(QGraphicsItem::ItemStacksBehindParent, false);

    m_point = InitPoint(Color(VColor::MainColor), this);
    m_point->setZValue(2);
    m_point->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfIntersectionArcs::RefreshGeometry()
{
    if (m_arc1Id > NULL_ID)
    {
        const QSharedPointer<VArc> arc1 = GetData()->GeometricObject<VArc>(m_arc1Id);
        DrawPath(m_arc1Path, arc1->GetPath(), arc1->DirectionArrows(), Qt::darkGreen, Qt::SolidLine, Qt::RoundCap);

        if (m_arc2Id > NULL_ID)
        {
            const QSharedPointer<VArc> arc2 = GetData()->GeometricObject<VArc>(m_arc2Id);
            DrawPath(m_arc2Path, arc2->GetPath(), arc2->DirectionArrows(), Qt::darkRed, Qt::SolidLine, Qt::RoundCap);

            QPointF fPoint;
            VToolPointOfIntersectionArcs::FindPoint(arc1.data(), arc2.data(), m_crossPoint, &fPoint);
            DrawPoint(m_point, fPoint, Color(VColor::MainColor));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPointOfIntersectionArcs::VisualMode(quint32 id)
{
    m_arc1Id = id;
    StartVisualMode();
}
