/************************************************************************
 **
 **  @file   vispath.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 8, 2014
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#include "vispath.h"

#include <QPen>

#include "../visualization.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vsimplepoint.h"

//---------------------------------------------------------------------------------------------------------------------
VisPath::VisPath(const VContainer *data, QGraphicsItem *parent)
  : Visualization(data),
    VCurvePathItem(VColorRole::VisMainColor, parent)
{
    setZValue(1); // Show on top real tool
    VisPath::InitPen();
}

//---------------------------------------------------------------------------------------------------------------------
void VisPath::InitPen()
{
    QPen visPen = pen();
    visPen.setStyle(LineStyle());
    setPen(visPen);
}

//---------------------------------------------------------------------------------------------------------------------
void VisPath::AddOnScene()
{
    AddItem(this);
}

//---------------------------------------------------------------------------------------------------------------------
auto VisPath::GetPoint(QVector<VSimplePoint *> &points, quint32 i, VColorRole role) -> VSimplePoint *
{
    if (not points.isEmpty() && static_cast<quint32>(points.size() - 1) >= i)
    {
        return points.at(static_cast<int>(i));
    }

    auto *point = new VSimplePoint(NULL_ID, role);
    point->SetPointHighlight(true);
    point->setParentItem(this);
    point->SetVisualizationMode(true);
    points.append(point);

    return point;
}

//---------------------------------------------------------------------------------------------------------------------
void VisPath::SetApproximationScale(qreal approximationScale)
{
    m_approximationScale = approximationScale;
}
