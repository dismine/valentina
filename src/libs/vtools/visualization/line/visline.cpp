/************************************************************************
 **
 **  @file   visline.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 7, 2014
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

#include "visline.h"

#include <QGraphicsScene>
#include <QGuiApplication>
#include <QPen>
#include <QRectF>
#include <QtMath>

#include "../vgeometry/vgobject.h"
#include "../vmisc/theme/themeDef.h"
#include "../vpatterndb/vcontainer.h"

//---------------------------------------------------------------------------------------------------------------------
VisLine::VisLine(const VContainer *data, QGraphicsItem *parent)
  : Visualization(data),
    VScaledLine(VColorRole::VisMainColor, parent)
{
    this->setZValue(1); // Show on top of a real tool
    VisLine::InitPen();
}

//---------------------------------------------------------------------------------------------------------------------
auto VisLine::CorrectAngle(const qreal &angle) -> qreal
{
    qreal ang = angle;
    if (angle > 360)
    {
        ang = angle - 360.0 * qFloor(angle / 360);
    }

    switch (qFloor((qAbs(ang) + 22.5) / 45))
    {
        case 1: // <67.5
            return 45;
        case 2: // <112.5
            return 90;
        case 3: // <157.5
            return 135;
        case 4: // <202.5
            return 180;
        case 5: // <247.5
            return 225;
        case 6: // < 292.5
            return 270;
        case 7: // <337.5
            return 315;
        case 0:  // <22.5
        default: // <360
            return 0;
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VisLine::Ray(const QPointF &firstPoint, const qreal &angle) const -> QPointF
{
    if (this->scene() == nullptr)
    {
        auto line = QLineF(firstPoint, ScenePos());
        line.setAngle(angle);
        return line.p2(); // We can't find ray because item doesn't have scene. We will return cursor position on scene.
    }

    QRectF scRect = this->scene()->sceneRect();

    // Limit size of the scene rect. Axis that has same size as scene rect cause scene size growth.
    auto line1 = QLineF(scRect.topLeft(), scRect.bottomRight());
    line1.setLength(2);

    auto line2 = QLineF(scRect.bottomRight(), scRect.topLeft());
    line2.setLength(2);

    scRect = QRectF(line1.p2(), line2.p2());

    if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
    {
        return VGObject::BuildRay(firstPoint, CorrectAngle(angle), scRect);
    }

    return VGObject::BuildRay(firstPoint, angle, scRect);
}

//---------------------------------------------------------------------------------------------------------------------
auto VisLine::Ray(const QPointF &firstPoint) const -> QPointF
{
    auto const line = QLineF(firstPoint, ScenePos());
    return Ray(firstPoint, line.angle());
}

//---------------------------------------------------------------------------------------------------------------------
auto VisLine::Axis(const QPointF &p, const qreal &angle) const -> QLineF
{
    return {Ray(p, angle + 180), Ray(p, angle)};
}

//---------------------------------------------------------------------------------------------------------------------
auto VisLine::Axis(const QPointF &p1, const QPointF &p2) const -> QLineF
{
    QLineF const line(p1, p2);
    return Axis(p1, line.angle());
}

//---------------------------------------------------------------------------------------------------------------------
void VisLine::InitPen()
{
    QPen visPen = pen();
    visPen.setStyle(LineStyle());
    setPen(visPen);
}

//---------------------------------------------------------------------------------------------------------------------
void VisLine::AddOnScene()
{
    AddItem(this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisLine::DrawRay(VScaledLine *lineItem, const QPointF &p, const QPointF &pTangent, Qt::PenStyle style)
{
    SCASSERT(lineItem != nullptr)

    const qreal angle = QLineF(p, pTangent).angle();
    const QPointF endRay = Ray(p, angle);
    const QLineF tangent = VGObject::BuildLine(p, QLineF(p, endRay).length(), angle);
    DrawLine(lineItem, tangent, style);
}
