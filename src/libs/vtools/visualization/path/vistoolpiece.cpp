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
#include "../vpatterndb/vpiecepath.h"
#include "../vgeometry/vpointf.h"
#include "../vwidgets/scalesceneitems.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolPiece::VisToolPiece(const VContainer *data, QGraphicsItem *parent)
    : VisPath(data, parent)
{
    m_line1 = InitItem<VScaledLine>(Color(VColor::SupportColor), this);
    m_line2 = InitItem<VScaledLine>(Color(VColor::SupportColor), this);
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

        DrawPath(this, m_cachedMainPath, Color(VColor::MainColor), Qt::SolidLine, Qt::RoundCap);

        for (int i = 0; i < m_cachedNodes.size(); ++i)
        {
            VScaledEllipse *point = GetPoint(static_cast<quint32>(i), Color(VColor::SupportColor));
            DrawPoint(point, m_cachedNodes.at(i).toQPointF(), Color(VColor::SupportColor));
        }

        if (GetMode() == Mode::Creation)
        {
            for (int i = 0; i < m_cachedCurvesPath.size(); ++i)
            {
                VCurvePathItem *path = GetCurve(static_cast<quint32>(i), Color(VColor::SupportColor));
                DrawPath(path, m_cachedCurvesPath.at(i), Color(VColor::SupportColor));
            }

            DrawLine(m_line1, QLineF(ConstFirst(m_cachedMainPathPoints), ScenePos()), Color(VColor::SupportColor),
                     Qt::DashLine);

            if (m_cachedMainPathPoints.size() > 1)
            {
                DrawLine(m_line2, QLineF(ConstLast(m_cachedMainPathPoints), ScenePos()), Color(VColor::SupportColor),
                         Qt::DashLine);
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
auto VisToolPiece::GetPoint(quint32 i, const QColor &color) -> VScaledEllipse *
{
    return GetPointItem(m_points, i, color, this);
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolPiece::GetCurve(quint32 i, const QColor &color) -> VCurvePathItem *
{
    return GetCurveItem(m_curves, i, color, this);
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

    for(QGraphicsEllipseItem *item : qAsConst(m_points))
    {
        if (item)
        {
            item->setVisible(false);
        }
    }

    for(QGraphicsPathItem *item : qAsConst(m_curves))
    {
        if (item)
        {
            item->setVisible(false);
        }
    }
}
