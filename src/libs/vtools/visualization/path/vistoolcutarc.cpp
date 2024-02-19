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
#include <new>

#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/varc.h"
#include "../visualization.h"
#include "../vmisc/theme/themeDef.h"
#include "../vmisc/vmodifierkey.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/scalesceneitems.h"
#include "vispath.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolCutArc::VisToolCutArc(const VContainer *data, QGraphicsItem *parent)
  : VisPath(data, parent)
{
    SetColorRole(VColorRole::VisSupportColor);

    m_arc1 = InitItem<VCurvePathItem>(VColorRole::VisSupportColor2, this);
    m_arc1->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
    m_arc2 = InitItem<VCurvePathItem>(VColorRole::VisSupportColor4, this);
    m_arc2->setFlag(QGraphicsItem::ItemStacksBehindParent, false);

    m_point = InitPoint(VColorRole::VisMainColor, this);
    m_point->setZValue(2);
    m_point->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCutArc::RefreshGeometry()
{
    if (m_arcId > NULL_ID)
    {
        const QSharedPointer<VArc> arc = GetData()->GeometricObject<VArc>(m_arcId);
        DrawPath(this, arc->GetPath(), arc->DirectionArrows(), LineStyle(), Qt::RoundCap);

        if (!qIsInf(m_length))
        {
            VArc ar1;
            VArc ar2;
            QPointF const p = arc->CutArc(m_length, ar1, ar2, QString());
            DrawPoint(m_point, p);

            DrawPath(m_arc1, ar1.GetPath(), ar1.DirectionArrows(), LineStyle(), Qt::RoundCap);
            DrawPath(m_arc2, ar2.GetPath(), ar2.DirectionArrows(), LineStyle(), Qt::RoundCap);
        }
        else if (GetMode() == Mode::Creation)
        {
            QPointF const p = arc->ClosestPoint(ScenePos());
            qreal length = arc->GetLengthByPoint(p);
            length = !arc->IsFlipped() ? qBound(0.0, length, arc->GetLength()) : qBound(arc->GetLength(), -length, 0.0);

            DrawPoint(m_point, p);

            const QString prefix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
            SetToolTip(tr("Length = %1%2; "
                          "<b>Mouse click</b> - finish selecting the length, "
                          "<b>%3</b> - skip")
                           .arg(LengthToUser(length), prefix, VModifierKey::EnterKey()));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCutArc::SetLength(const QString &expression)
{
    m_length = FindLengthFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCutArc::VisualMode(quint32 id)
{
    m_arcId = id;
    StartVisualMode();
}
