/************************************************************************
 **
 **  @file   vistoolellipticalarc.cpp
 **  @author Valentina Zhuravska <zhuravska19(at)gmail.com>
 **  @date   24 10, 2016
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
#include "vistoolellipticalarc.h"
#include <QPainterPath>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <new>

#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "../vwidgets/scalesceneitems.h"
#include "vispath.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolEllipticalArc::VisToolEllipticalArc(const VContainer *data, QGraphicsItem *parent)
    :VisPath(data, parent)
{
    m_arcCenter = InitPoint(Color(VColor::MainColor), this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArc::RefreshGeometry()
{
    if (m_centerId > NULL_ID)
    {
        const QSharedPointer<VPointF> first = GetData()->GeometricObject<VPointF>(m_centerId);
        DrawPoint(m_arcCenter, static_cast<QPointF>(*first), Color(VColor::SupportColor));

        if (not qFuzzyIsNull(m_radius1) && not qFuzzyIsNull(m_radius2) && m_f1 >= 0 && m_f2 >= 0 && m_rotationAngle >= 0)
        {
            VEllipticalArc elArc = VEllipticalArc(*first, m_radius1, m_radius2, m_f1, m_f2, m_rotationAngle);
            DrawPath(this, elArc.GetPath(), elArc.DirectionArrows(), Color(VColor::MainColor), LineStyle(),
                     Qt::RoundCap);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArc::VisualMode(quint32 id)
{
    m_centerId = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArc::SetRadius1(const QString &expression)
{
    m_radius1 = FindLengthFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArc::SetRadius2(const QString &expression)
{
    m_radius2 = FindLengthFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArc::SetF1(const QString &expression)
{
    m_f1 = FindValFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArc::SetF2(const QString &expression)
{
    m_f2 = FindValFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEllipticalArc::SetRotationAngle(const QString &expression)
{
    m_rotationAngle = FindValFromUser(expression, GetData()->DataVariables());
}
