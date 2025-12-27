/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 9, 2016
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

#include "visoperation.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepath.h"

//---------------------------------------------------------------------------------------------------------------------
VisOperation::VisOperation(const VContainer *data, QGraphicsItem *parent)
  : VisLine(data, parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
VisOperation::~VisOperation()
{
    qDeleteAll(m_points);
    qDeleteAll(m_curves);
}

//---------------------------------------------------------------------------------------------------------------------
void VisOperation::SetObjects(const QVector<quint32> &objects)
{
    m_objects = objects;
}

//---------------------------------------------------------------------------------------------------------------------
void VisOperation::VisualMode(quint32 id)
{
    Q_UNUSED(id)
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
auto VisOperation::GetPoint(quint32 i, VColorRole role) -> VScaledEllipse *
{
    return GetPointItem(m_points, i, role, this);
}

//---------------------------------------------------------------------------------------------------------------------
auto VisOperation::GetCurve(quint32 i, VColorRole role) -> VCurvePathItem *
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

void VisOperation::RefreshFlippedObjects(quint32 originPointId, const QPointF &firstPoint, const QPointF &secondPoint)
{
    int iPoint = -1;
    int iCurve = -1;
    for (auto id : std::as_const(m_objects))
    {
        const QSharedPointer<VGObject> obj = GetData()->GetGObject(id);

        // This check helps to find missed objects in the switch
        Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 8, "Not all objects were handled.");

        QT_WARNING_PUSH
        QT_WARNING_DISABLE_GCC("-Wswitch-default")
        QT_WARNING_DISABLE_CLANG("-Wswitch-default")

        switch (obj->getType())
        {
            case GOType::Point:
            {
                const QSharedPointer<VPointF> p = GetData()->GeometricObject<VPointF>(id);

                ++iPoint;
                VScaledEllipse *point = GetPoint(static_cast<quint32>(iPoint), VColorRole::VisSupportColor2);
                DrawPoint(point, static_cast<QPointF>(*p));

                ++iPoint;
                point = GetPoint(static_cast<quint32>(iPoint), VColorRole::VisSupportColor);

                if (originPointId != NULL_ID)
                {
                    DrawPoint(point, static_cast<QPointF>(p->Flip(QLineF(firstPoint, secondPoint))));
                }
                break;
            }
            case GOType::Arc:
            {
                iCurve = AddFlippedCurve<VArc>(originPointId, firstPoint, secondPoint, id, iCurve);
                break;
            }
            case GOType::EllipticalArc:
            {
                iCurve = AddFlippedCurve<VEllipticalArc>(originPointId, firstPoint, secondPoint, id, iCurve);
                break;
            }
            case GOType::Spline:
            {
                iCurve = AddFlippedCurve<VSpline>(originPointId, firstPoint, secondPoint, id, iCurve);
                break;
            }
            case GOType::SplinePath:
            {
                iCurve = AddFlippedCurve<VSplinePath>(originPointId, firstPoint, secondPoint, id, iCurve);
                break;
            }
            case GOType::CubicBezier:
            {
                iCurve = AddFlippedCurve<VCubicBezier>(originPointId, firstPoint, secondPoint, id, iCurve);
                break;
            }
            case GOType::CubicBezierPath:
            {
                iCurve = AddFlippedCurve<VCubicBezierPath>(originPointId, firstPoint, secondPoint, id, iCurve);
                break;
            }
            case GOType::Unknown:
            case GOType::PlaceLabel:
                Q_UNREACHABLE();
                break;
        }

        QT_WARNING_POP
    }
}
