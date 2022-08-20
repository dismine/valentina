/************************************************************************
 **
 **  @file   vistoolcutspline.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 9, 2014
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

#include "vistoolcutspline.h"

#include <QGraphicsEllipseItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <new>

#include "../vgeometry/vabstractcubicbezier.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "vispath.h"
#include "../vwidgets/scalesceneitems.h"
#include "../vmisc/vmodifierkey.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolCutSpline::VisToolCutSpline(const VContainer *data, QGraphicsItem *parent)
    :VisPath(data, parent)
{
    m_spl1 = InitItem<VCurvePathItem>(Qt::darkGreen, this);
    m_spl1->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
    m_spl2 = InitItem<VCurvePathItem>(Qt::darkRed, this);
    m_spl2->setFlag(QGraphicsItem::ItemStacksBehindParent, false);

    m_point = InitPoint(mainColor, this);
    m_point->setZValue(2);
    m_point->setFlag(QGraphicsItem::ItemStacksBehindParent, false);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCutSpline::RefreshGeometry()
{
    if (object1Id > NULL_ID)
    {
        const auto spl = Visualization::data->GeometricObject<VAbstractCubicBezier>(object1Id);
        DrawPath(this, spl->GetPath(), spl->DirectionArrows(), supportColor, lineStyle, Qt::RoundCap);

        if (not qFuzzyIsNull(m_length))
        {
            QPointF spl1p2;
            QPointF spl1p3;
            QPointF spl2p2;
            QPointF spl2p3;
            const QPointF p = spl->CutSpline(m_length, spl1p2, spl1p3, spl2p2, spl2p3, QString());

            VSpline sp1 = VSpline(spl->GetP1(), spl1p2, spl1p3, VPointF(p));
            sp1.SetApproximationScale(spl->GetApproximationScale());
            VSpline sp2 = VSpline(VPointF(p), spl2p2, spl2p3, spl->GetP4());
            sp2.SetApproximationScale(spl->GetApproximationScale());

            DrawPoint(m_point, p, mainColor);

            DrawPath(m_spl1, sp1.GetPath(), sp1.DirectionArrows(), Qt::darkGreen, lineStyle, Qt::RoundCap);
            DrawPath(m_spl2, sp2.GetPath(), sp2.DirectionArrows(), Qt::darkRed, lineStyle, Qt::RoundCap);
        }
        else if (mode == Mode::Creation)
        {
            QPointF p = spl->ClosestPoint(Visualization::scenePos);
            qreal length = spl->GetLengthByPoint(p);

            DrawPoint(m_point, p, mainColor);

            const QString prefix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
            Visualization::toolTip = tr("Length = %1%2; "
                                        "<b>Mouse click</b> - finish selecting the length, "
                                        "<b>%3</b> - skip")
                                         .arg(NumberToUser(length), prefix, VModifierKey::EnterKey());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolCutSpline::setLength(const QString &expression)
{
    m_length = FindLengthFromUser(expression, Visualization::data->DataVariables());
}
