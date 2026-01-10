/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 11, 2016
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

#include "vistoolpiecepath.h"
#include "../vgeometry/vpointf.h"
#include "../vlayout/vlayoutpoint.h"
#include "../vwidgets/scalesceneitems.h"
#include "../vwidgets/vsimplepoint.h"

#include <QGraphicsSceneMouseEvent>

//---------------------------------------------------------------------------------------------------------------------
VisToolPiecePath::VisToolPiecePath(const VContainer *data, QGraphicsItem *parent)
  : VisPath(data, parent)
{
    m_line = InitItem<VScaledLine>(VColorRole::VisSupportColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPiecePath::RefreshGeometry()
{
    HideAllItems();

    if (m_path.CountNodes() > 0)
    {
        DrawPath(this, m_path.PainterPath(GetData(), m_cuttingPath), m_path.GetPenType(), Qt::RoundCap);

        const QVector<VPointF> nodes = m_path.PathNodePoints(GetData());

        for (int i = 0; i < nodes.size(); ++i)
        {
            VSimplePoint *point = GetPoint(static_cast<quint32>(i), VColorRole::VisSupportColor);
            point->RefreshPointGeometry(nodes.at(i)); // Keep first, you can hide only objects those have shape
            point->SetOnlyPoint(GetMode() == Mode::Creation);
            point->setVisible(true);
        }

        if (GetMode() == Mode::Creation)
        {
            const QVector<VLayoutPoint> points = m_path.PathPoints(GetData());
            if (not points.empty())
            {
                DrawLine(m_line, QLineF(points.constLast(), ScenePos()), Qt::DashLine);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPiecePath::VisualMode(quint32 id)
{
    Q_UNUSED(id)
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPiecePath::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->ignore();
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolPiecePath::GetPoint(quint32 i, VColorRole role) -> VSimplePoint *
{
    return VisPath::GetPoint(m_points, i, role);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPiecePath::HideAllItems()
{
    if (m_line)
    {
        m_line->setVisible(false);
    }

    for (auto *item : std::as_const(m_points))
    {
        if (item)
        {
            item->setVisible(false);
        }
    }
}
