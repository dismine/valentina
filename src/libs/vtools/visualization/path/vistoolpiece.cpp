/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   5 11, 2016
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

#include "vistoolpiece.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vpiecepath.h"
#include "../vwidgets/scalesceneitems.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolPiece::VisToolPiece(const VContainer *data, QGraphicsItem *parent)
  : VisPath(data, parent)
{
    m_line1 = InitItem<VScaledLine>(VColorRole::VisSupportColor, this);
    m_line2 = InitItem<VScaledLine>(VColorRole::VisSupportColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPiece::RefreshGeometry()
{
    HideAllItems();

    if (m_piece.GetPath().CountNodes() > 0)
    {
        if (not m_pieceCached)
        {
            m_cachedNodes = m_piece.MainPathNodePoints(GetData());
            if (GetMode() == Mode::Creation)
            {
                m_cachedCurvesPath = m_piece.CurvesPainterPath(GetData());
                CastTo(m_piece.MainPathPoints(GetData()), m_cachedMainPathPoints);
                m_cachedMainPath = VPiece::MainPathPath(m_cachedMainPathPoints);
            }
            else
            {
                m_cachedMainPath = m_piece.MainPathPath(GetData());
            }
            m_pieceCached = true;
        }

        DrawPath(this, m_cachedMainPath, Qt::SolidLine, Qt::RoundCap);

        for (int i = 0; i < m_cachedNodes.size(); ++i)
        {
            VScaledEllipse *point = GetPoint(static_cast<quint32>(i), VColorRole::VisSupportColor);
            DrawPoint(point, m_cachedNodes.at(i).toQPointF());
        }

        if (GetMode() == Mode::Creation)
        {
            for (int i = 0; i < m_cachedCurvesPath.size(); ++i)
            {
                VCurvePathItem *path = GetCurve(static_cast<quint32>(i), VColorRole::VisSupportColor);
                DrawPath(path, m_cachedCurvesPath.at(i));
            }

            DrawLine(m_line1, QLineF(m_cachedMainPathPoints.constFirst(), ScenePos()), Qt::DashLine);

            if (m_cachedMainPathPoints.size() > 1)
            {
                DrawLine(m_line2, QLineF(m_cachedMainPathPoints.constLast(), ScenePos()), Qt::DashLine);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPiece::VisualMode(quint32 id)
{
    Q_UNUSED(id)
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPiece::SetPiece(const VPiece &piece)
{
    m_piece = piece;
    m_pieceCached = false;
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolPiece::GetPoint(quint32 i, VColorRole role) -> VScaledEllipse *
{
    return GetPointItem(m_points, i, role, this);
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolPiece::GetCurve(quint32 i, VColorRole role) -> VCurvePathItem *
{
    return GetCurveItem(m_curves, i, role, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolPiece::HideAllItems()
{
    if (m_line1)
    {
        m_line1->setVisible(false);
    }

    if (m_line2)
    {
        m_line2->setVisible(false);
    }

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
