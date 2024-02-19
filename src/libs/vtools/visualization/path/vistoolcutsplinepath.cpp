/************************************************************************
 **
 **  @file   vistoolcutsplinepath.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   7 9, 2014
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

#include "vistoolcutsplinepath.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QPointF>
#include <QSharedPointer>

#include "../../tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutsplinepath.h"
#include "../vgeometry/vabstractcubicbezierpath.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vsplinepath.h"
#include "../visualization.h"
#include "../vmisc/vmodifierkey.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/scalesceneitems.h"
#include "vispath.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolCutSplinePath::VisToolCutSplinePath(const VContainer *data, QGraphicsItem *parent)
  : VisPath(data, parent)
{
    SetColorRole(VColorRole::VisSupportColor);

    m_splPath1 = InitItem<VCurvePathItem>(VColorRole::VisSupportColor2, this);
    m_splPath1->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
    m_splPath2 = InitItem<VCurvePathItem>(VColorRole::VisSupportColor4, this);
    m_splPath2->setFlag(QGraphicsItem::ItemStacksBehindParent, false);

    m_point = InitPoint(VColorRole::VisMainColor, this);
    m_point->setZValue(2);
    m_point->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCutSplinePath::RefreshGeometry()
{
    if (m_splinePathId > NULL_ID)
    {
        const auto splPath = GetData()->GeometricObject<VAbstractCubicBezierPath>(m_splinePathId);
        DrawPath(this, splPath->GetPath(), splPath->DirectionArrows(), LineStyle(), Qt::RoundCap);

        if (!qIsInf(m_length))
        {
            VSplinePath *spPath1 = nullptr;
            VSplinePath *spPath2 = nullptr;
            VPointF *p = VToolCutSplinePath::CutSplinePath(m_length, splPath, QChar('X'), &spPath1, &spPath2);
            SCASSERT(p != nullptr)
            SCASSERT(spPath1 != nullptr)
            SCASSERT(spPath2 != nullptr)

            DrawPoint(m_point, static_cast<QPointF>(*p));
            delete p;

            DrawPath(m_splPath1, spPath1->GetPath(), spPath1->DirectionArrows(), LineStyle(), Qt::RoundCap);
            DrawPath(m_splPath2, spPath2->GetPath(), spPath2->DirectionArrows(), LineStyle(), Qt::RoundCap);

            delete spPath1;
            delete spPath2;
        }
        else if (GetMode() == Mode::Creation)
        {
            QPointF const p = splPath->ClosestPoint(ScenePos());
            qreal const length = splPath->GetLengthByPoint(p);

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
void VisToolCutSplinePath::VisualMode(quint32 id)
{
    m_splinePathId = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCutSplinePath::SetLength(const QString &expression)
{
    m_length = FindLengthFromUser(expression, GetData()->DataVariables());
}
