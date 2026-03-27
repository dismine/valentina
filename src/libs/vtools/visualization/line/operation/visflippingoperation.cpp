/************************************************************************
 **
 **  @file   visflippingoperation.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 3, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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
#include "visflippingoperation.h"

#include "../vgeometry/varc.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepath.h"

//---------------------------------------------------------------------------------------------------------------------
VisFlippingOperation::VisFlippingOperation(const VContainer *data, QGraphicsItem *parent)
  : VisOperation(data, parent)
{
    SetColorRole(VColorRole::VisSupportColor2);
}

//---------------------------------------------------------------------------------------------------------------------
void VisFlippingOperation::CreateFlippedObjects(
    int &iPoint, int &iCurve, quint32 originPointId, const QPointF &firstPoint, const QPointF &secondPoint)
{
    for (auto id : Objects())
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
                VScaledEllipse *point = GetPoint(static_cast<quint32>(iPoint), VColorRole::VisSupportColor);

                if (originPointId != NULL_ID)
                {
                    DrawPoint(point, static_cast<QPointF>(p->Flip(QLineF(firstPoint, secondPoint))));
                }
                break;
            }
            case GOType::Arc:
                iCurve = AddFlippedCurve<VArc>(originPointId, firstPoint, secondPoint, id, iCurve);
                break;
            case GOType::EllipticalArc:
                iCurve = AddFlippedCurve<VEllipticalArc>(originPointId, firstPoint, secondPoint, id, iCurve);
                break;
            case GOType::Spline:
                iCurve = AddFlippedCurve<VSpline>(originPointId, firstPoint, secondPoint, id, iCurve);
                break;
            case GOType::SplinePath:
                iCurve = AddFlippedCurve<VSplinePath>(originPointId, firstPoint, secondPoint, id, iCurve);
                break;
            case GOType::CubicBezier:
                iCurve = AddFlippedCurve<VCubicBezier>(originPointId, firstPoint, secondPoint, id, iCurve);
                break;
            case GOType::CubicBezierPath:
                iCurve = AddFlippedCurve<VCubicBezierPath>(originPointId, firstPoint, secondPoint, id, iCurve);
                break;
            case GOType::Unknown:
            case GOType::PlaceLabel:
                Q_UNREACHABLE();
                break;
        }

        QT_WARNING_POP
    }
}
