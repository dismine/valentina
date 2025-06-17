/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   31 1, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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

#include "vistoolspecialpoint.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vsimplepoint.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolSpecialPoint::VisToolSpecialPoint(const VContainer *data, QGraphicsItem *parent)
  : VisLine(data, parent)
{
    setZValue(2); // Show on top real tool
    setPen(QPen(Qt::NoPen));

    m_point = new VSimplePoint(NULL_ID, VColorRole::VisMainColor);
    m_point->SetPointHighlight(true);
    m_point->setParentItem(this);
    m_point->SetVisualizationMode(true);
    m_point->setVisible(false);
}

//---------------------------------------------------------------------------------------------------------------------
VisToolSpecialPoint::~VisToolSpecialPoint()
{
    delete m_point.data();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolSpecialPoint::RefreshGeometry()
{
    if (m_pointId > NULL_ID)
    {
        // Keep first, you can hide only objects which have shape
        m_point->RefreshPointGeometry(*GetData()->GeometricObject<VPointF>(m_pointId));
        m_point->SetOnlyPoint(GetMode() == Mode::Creation);
        m_point->setVisible(true);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolSpecialPoint::VisualMode(quint32 id)
{
    m_pointId = id;
    StartVisualMode();
}
