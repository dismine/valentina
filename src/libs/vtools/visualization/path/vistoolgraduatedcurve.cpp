/************************************************************************
 **
 **  @file   vistoolgraduatedcurve.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 12, 2025
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
#include "vistoolgraduatedcurve.h"
#include "../../tools/toolsdef.h"
#include "../vgeometry/vsplinepath.h"
#include "../vmisc/vmodifierkey.h"
#include "../vpatterndb/variables/vincrement.h"
#include "../vpatterndb/vcontainer.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolGraduatedCurve::VisToolGraduatedCurve(const VContainer *data, QGraphicsItem *parent)
  : VisPath(data, parent)
{
    m_originalCurve = InitItem<VCurvePathItem>(VColorRole::VisSupportColor, this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolGraduatedCurve::RefreshGeometry()
{
    if (m_curveId == NULL_ID)
    {
        return;
    }

    const QSharedPointer<VAbstractCurve> curve = GetData()->GeometricObject<VAbstractCurve>(m_curveId);
    DrawPath(m_originalCurve, curve->GetPath(), curve->DirectionArrows(), LineStyle(), Qt::RoundCap);

    if (GetMode() == Mode::Show && !m_offsets.isEmpty())
    {
        VSplinePath splPath = curve->Outline(m_offsets);
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

        QVector<qreal> tmpOffsets = m_offsets;
        tmpOffsets.append(width);

        VSplinePath splPath = curve->Outline(tmpOffsets);
        splPath.SetApproximationScale(ApproximationScale());

        DrawPath(this, splPath.GetPath(), splPath.DirectionArrows(), LineStyle(), Qt::RoundCap);

        static const QString suffix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);

        auto PrepareOffsetValues = [](const QVector<qreal> &offsets) -> QString
        {
            QStringList values;

            for (const auto &offset : offsets)
            {
                values.append(LengthToUser(offset) + suffix);
            }

            QString str = values.join(';');

            if (values.size() > 1)
            {
                str.prepend('(');
                str.append(')');
            }

            return str;
        };

        SetToolTip(tr("<b>Graduated curve</b>: offset(s) = %1; "
                      "<b>Mouse click</b> - selecting next width, "
                      "<b>%3</b> - negative width, "
                      "<b>%4</b> - finish",
                      nullptr,
                      static_cast<int>(tmpOffsets.size()))
                       .arg(PrepareOffsetValues(tmpOffsets), VModifierKey::Shift(), VModifierKey::EnterKey()));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolGraduatedCurve::VisualMode(quint32 id)
{
    m_curveId = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolGraduatedCurve::SetOffsets(const QVector<VRawGraduatedCurveOffset> &offsets)
{
    m_offsets.clear();
    m_offsets.reserve(offsets.size());

    VContainer localData = *GetData();

    for (const auto &offset : offsets)
    {
        qreal const width = FindLengthFromUser(offset.formula, localData.DataVariables());
        m_offsets.append(width);

        auto *offsetVal = new VIncrement(&localData, offset.name);
        offsetVal->SetFormula(width, offset.formula, true);

        localData.AddVariable(offsetVal);
    }
}
