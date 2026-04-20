/************************************************************************
 **
 **  @file   vistoolinsertnode.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 4, 2026
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
#include "vistoolinsertnode.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/scalesceneitems.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolInsertNode::VisToolInsertNode(const VContainer *data, QGraphicsItem *parent)
  : VisPath(data, parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolInsertNode::VisualMode(quint32 id)
{
    Q_UNUSED(id)

    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolInsertNode::RefreshGeometry()
{
    HideAllItems();

    if (m_nodes.isEmpty())
    {
        return;
    }

    int iPoint = -1;
    int iCurve = -1;

    for (auto id : std::as_const(m_nodes))
    {
        const QSharedPointer<VGObject> obj = GetData()->GetGObject(id);

        // This check helps to find missed objects in the switch
        Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 8, "Not all objects were handled.");

        switch (obj->getType())
        {
            case GOType::Point:
                CreateNodePoint(id, iPoint);
                break;
            case GOType::Arc:
            case GOType::EllipticalArc:
            case GOType::Spline:
            case GOType::SplinePath:
            case GOType::CubicBezier:
            case GOType::CubicBezierPath:
                CreateNodeCurve(id, iCurve);
                break;
            case GOType::Unknown:
            case GOType::PlaceLabel:
                Q_UNREACHABLE();
                break;
            default:
                break;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolInsertNode::SetNodes(const QVector<quint32> &nodes)
{
    m_nodes = nodes;
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolInsertNode::HideAllItems()
{
    for (QGraphicsEllipseItem *item : std::as_const(m_points))
    {
        if (item)
        {
            item->setVisible(false);
        }
    }

    for (QGraphicsPathItem *item : std::as_const(m_curves))
    {
        if (item)
        {
            item->setVisible(false);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolInsertNode::GetPoint(quint32 i, VColorRole role) -> VScaledEllipse *
{
    return GetPointItem(m_points, i, role, this);
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolInsertNode::GetCurve(quint32 i, VColorRole role) -> VCurvePathItem *
{
    if (!m_curves.isEmpty() && static_cast<quint32>(m_curves.size() - 1) >= i)
    {
        return m_curves.at(static_cast<int>(i));
    }

    auto *curve = InitItem<VCurvePathItem>(role, this);
    m_curves.append(curve);
    return curve;
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolInsertNode::CreateNodeCurve(quint32 id, int &i) -> QGraphicsPathItem *
{
    const QSharedPointer<VAbstractCurve> curve = GetData()->template GeometricObject<VAbstractCurve>(id);

    ++i;
    VCurvePathItem *path = GetCurve(static_cast<quint32>(i), VColorRole::VisSupportColor2);
    DrawPath(path, curve->GetPath(), curve->DirectionArrows(), Qt::SolidLine, Qt::RoundCap);

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolInsertNode::CreateNodePoint(quint32 id, int &i) -> VScaledEllipse *
{
    const QSharedPointer<VPointF> p = GetData()->GeometricObject<VPointF>(id);

    ++i;
    VScaledEllipse *point = GetPoint(static_cast<quint32>(i), VColorRole::VisSupportColor2);
    DrawPoint(point, static_cast<QPointF>(*p));

    return point;
}
