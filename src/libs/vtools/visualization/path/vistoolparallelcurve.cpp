/************************************************************************
 **
 **  @file   vistoolparallelcurve.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 11, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
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
#include "vistoolparallelcurve.h"
#include "../vgeometry/vsplinepath.h"
#include "../vmisc/vmodifierkey.h"
#include "../vpatterndb/vcontainer.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolParallelCurve::VisToolParallelCurve(const VContainer *data, QGraphicsItem *parent)
  : VisPath(data, parent)
{
    m_originalCurve = InitItem<VCurvePathItem>(VColorRole::VisSupportColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolParallelCurve::RefreshGeometry()
{
    if (m_curveId == NULL_ID)
    {
        return;
    }

    const QSharedPointer<VAbstractCurve> curve = GetData()->GeometricObject<VAbstractCurve>(m_curveId);
    DrawPath(m_originalCurve, curve->GetPath(), curve->DirectionArrows(), LineStyle(), Qt::RoundCap);

    if (!qIsInf(m_width))
    {
        VSplinePath splPath = curve->Offset(m_width);
        splPath.SetApproximationScale(ApproximationScale());

        DrawPath(this, splPath.GetPath(), splPath.DirectionArrows(), LineStyle(), Qt::RoundCap);
    }
    else if (GetMode() == Mode::Creation)
    {
        QPointF const p = curve->ClosestPoint(ScenePos());

        qreal width = QLineF(p, ScenePos()).length();
        if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
        {
            width = -width;
        }

        VSplinePath splPath = curve->Offset(width);
        splPath.SetApproximationScale(ApproximationScale());

        DrawPath(this, splPath.GetPath(), splPath.DirectionArrows(), LineStyle(), Qt::RoundCap);

        static const QString suffix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);

        SetToolTip(tr("<b>Parallel curve</b>: width = %1%2; "
                      "<b>Mouse click</b> - finish selecting the width, "
                      "<b>%3</b> - negative width, "
                      "<b>%4</b> - skip")
                       .arg(LengthToUser(width), suffix, VModifierKey::Shift(), VModifierKey::EnterKey()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolParallelCurve::VisualMode(quint32 id)
{
    m_curveId = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolParallelCurve::SetOffsetWidth(const QString &expression)
{
    m_width = FindLengthFromUser(expression, GetData()->DataVariables());
}
