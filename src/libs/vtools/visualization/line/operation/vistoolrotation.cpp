/************************************************************************
 **
 **  @file   vistoolrotation.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   11 4, 2016
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

#include "vistoolrotation.h"

#include <climits>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGuiApplication>
#include <QLineF>
#include <QPainterPath>
#include <QSharedPointer>
#include <Qt>
#include <QtAlgorithms>
#include <new>

#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/vmodifierkey.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vgeometrydef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepath.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/global.h"
#include "visoperation.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolRotation::VisToolRotation(const VContainer *data, QGraphicsItem *parent)
    : VisOperation(data, parent)
{
    m_point = InitPoint(Color(VColor::SupportColor2), this);
    m_angleArc = InitItem<VCurvePathItem>(Color(VColor::SupportColor2), this);
    m_xAxis = InitItem<VScaledLine>(Color(VColor::SupportColor2), this);
}

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")

void VisToolRotation::RefreshGeometry()
{
    if (Objects().isEmpty())
    {
        return;
    }

    QSharedPointer<VPointF> origin = QSharedPointer<VPointF>(new VPointF());

    qreal tempAngle = 0;

    if (m_originPointId != NULL_ID)
    {
        origin = GetData()->GeometricObject<VPointF>(m_originPointId);
        DrawPoint(m_point, static_cast<QPointF>(*origin), Color(VColor::SupportColor2));

        QLineF rLine;
        if (VFuzzyComparePossibleNulls(m_angle, INT_MIN))
        {
            rLine = QLineF(static_cast<QPointF>(*origin), ScenePos());

            if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
            {
                rLine.setAngle(CorrectAngle(rLine.angle()));
            }

            rLine.setP2(Ray(static_cast<QPointF>(*origin), rLine.angle()));
            tempAngle = rLine.angle();
        }
        else
        {
            rLine = QLineF(static_cast<QPointF>(*origin), Ray(static_cast<QPointF>(*origin), m_angle));
            tempAngle = m_angle;
        }

        DrawLine(this, rLine, Color(VColor::SupportColor2), Qt::DashLine);
        DrawLine(m_xAxis, QLineF(static_cast<QPointF>(*origin), Ray(static_cast<QPointF>(*origin), 0)),
                 Color(VColor::SupportColor2), Qt::DashLine);

        VArc arc(*origin, ScaledRadius(SceneScale(VAbstractValApplication::VApp()->getCurrentScene()))*2, 0, tempAngle);
        DrawPath(m_angleArc, arc.GetPath(), Color(VColor::SupportColor2), Qt::SolidLine, Qt::RoundCap);

        SetToolTip(tr("Rotating angle = %1°, <b>%2</b> - sticking angle, "
                      "<b>Mouse click</b> - finish creation")
                       .arg(AngleToUser(tempAngle), VModifierKey::Shift()));
    }

    int iPoint = -1;
    int iCurve = -1;
    for (auto id : Objects())
    {
        const QSharedPointer<VGObject> obj = GetData()->GetGObject(id);

        // This check helps to find missed objects in the switch
        Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 8, "Not all objects was handled.");

        switch(static_cast<GOType>(obj->getType()))
        {
            case GOType::Point:
            {
                const QSharedPointer<VPointF> p = GetData()->GeometricObject<VPointF>(id);

                ++iPoint;
                VScaledEllipse *point = GetPoint(static_cast<quint32>(iPoint), Color(VColor::SupportColor2));
                DrawPoint(point, static_cast<QPointF>(*p), Color(VColor::SupportColor2));

                ++iPoint;
                point = GetPoint(static_cast<quint32>(iPoint), Color(VColor::SupportColor));

                if (m_originPointId != NULL_ID)
                {
                    DrawPoint(point, static_cast<QPointF>(p->Rotate(static_cast<QPointF>(*origin), tempAngle)),
                              Color(VColor::SupportColor));
                }
                break;
            }
            case GOType::Arc:
            {
                iCurve = AddCurve<VArc>(tempAngle, static_cast<QPointF>(*origin), id, iCurve);
                break;
            }
            case GOType::EllipticalArc:
            {
                iCurve = AddCurve<VEllipticalArc>(tempAngle, static_cast<QPointF>(*origin), id, iCurve);
                break;
            }
            case GOType::Spline:
            {
                iCurve = AddCurve<VSpline>(tempAngle, static_cast<QPointF>(*origin), id, iCurve);
                break;
            }
            case GOType::SplinePath:
            {
                iCurve = AddCurve<VSplinePath>(tempAngle, static_cast<QPointF>(*origin), id, iCurve);
                break;
            }
            case GOType::CubicBezier:
            {
                iCurve = AddCurve<VCubicBezier>(tempAngle, static_cast<QPointF>(*origin), id, iCurve);
                break;
            }
            case GOType::CubicBezierPath:
            {
                iCurve = AddCurve<VCubicBezierPath>(tempAngle, static_cast<QPointF>(*origin), id, iCurve);
                break;
            }
            case GOType::Unknown:
            case GOType::PlaceLabel:
                Q_UNREACHABLE();
                break;
        }
    }
}

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
auto VisToolRotation::Angle() const -> QString
{
    return QString::number(line().angle());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolRotation::SetAngle(const QString &expression)
{
    m_angle = FindValFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
auto VisToolRotation::AddCurve(qreal angle, const QPointF &origin, quint32 id, int i) -> int
{
    const QSharedPointer<Item> curve = GetData()->template GeometricObject<Item>(id);

    ++i;
    VCurvePathItem *path = GetCurve(static_cast<quint32>(i), Color(VColor::SupportColor2));
    DrawPath(path, curve->GetPath(), curve->DirectionArrows(), Color(VColor::SupportColor2), Qt::SolidLine,
             Qt::RoundCap);

    ++i;
    path = GetCurve(static_cast<quint32>(i), Color(VColor::SupportColor));
    if (m_originPointId != NULL_ID)
    {
        const Item rotated = curve->Rotate(origin, angle);
        DrawPath(path, rotated.GetPath(), rotated.DirectionArrows(), Color(VColor::SupportColor), Qt::SolidLine,
                 Qt::RoundCap);
    }

    return i;
}
