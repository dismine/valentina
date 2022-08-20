/************************************************************************
 **
 **  @file   vistoolcutarc.cpp
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

#include "vistoolcutarc.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <new>

#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/varc.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "vispath.h"
#include "../vwidgets/scalesceneitems.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolCutArc::VisToolCutArc(const VContainer *data, QGraphicsItem *parent)
    :VisPath(data, parent)
{
    m_arc1 = InitItem<VCurvePathItem>(Qt::darkGreen, this);
    m_arc1->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
    m_arc2 = InitItem<VCurvePathItem>(Qt::darkRed, this);
    m_arc2->setFlag(QGraphicsItem::ItemStacksBehindParent, false);

    m_point = InitPoint(mainColor, this);
    m_point->setZValue(2);
    m_point->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCutArc::RefreshGeometry()
{
    if (object1Id > NULL_ID)
    {
        const QSharedPointer<VArc> arc = Visualization::data->GeometricObject<VArc>(object1Id);
        DrawPath(this, arc->GetPath(), arc->DirectionArrows(), supportColor, lineStyle, Qt::RoundCap);

        if (not qFuzzyIsNull(m_length))
        {
            VArc ar1;
            VArc ar2;
            QPointF p = arc->CutArc(m_length, ar1, ar2, QString());
            DrawPoint(m_point, p, mainColor);

            DrawPath(m_arc1, ar1.GetPath(), ar1.DirectionArrows(), Qt::darkGreen, lineStyle, Qt::RoundCap);
            DrawPath(m_arc2, ar2.GetPath(), ar2.DirectionArrows(), Qt::darkRed, lineStyle, Qt::RoundCap);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCutArc::setLength(const QString &expression)
{
    m_length = FindLengthFromUser(expression, Visualization::data->DataVariables());
}
