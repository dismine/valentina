/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   1 10, 2016
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

#include "vistoolmove.h"

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

#include "../ifc/xml/vdomdocument.h"
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
#include "../vgeometry/varc.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "visoperation.h"
#include "../vwidgets/global.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolMove::VisToolMove(const VContainer *data, QGraphicsItem *parent)
    : VisOperation(data, parent)
{
    m_pointOrigin = InitPoint(Color(VColor::SupportColor2), this);
    m_pointRotationOrigin = InitPoint(Color(VColor::SupportColor2), this);
    m_pointFinish = InitPoint(Color(VColor::SupportColor), this);
    m_angleArc = InitItem<VCurvePathItem>(Color(VColor::SupportColor3), this);
    m_rotationLine = InitItem<VScaledLine>(Color(VColor::SupportColor3), this);
    m_xAxis = InitItem<VScaledLine>(Color(VColor::SupportColor3), this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolMove::RefreshGeometry()
{
    if (Objects().isEmpty())
    {
        return;
    }

    int iPoint = -1;
    int iCurve = -1;

    const QVector<QGraphicsItem *> originObjects = CreateOriginObjects(iPoint, iCurve);

    QPointF origin = GetOriginPoint(originObjects);
    DrawPoint(m_pointOrigin, origin, Color(VColor::SupportColor2));

    qreal tempAngle = 0;
    qreal tempLength = 0;
    qreal tempRoationAngle = 0;

    QLineF line;
    if (qFuzzyIsNull(m_length))
    {
        if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
        {
            line = QLineF(origin, ScenePos());
            line.setAngle(CorrectAngle(line.angle()));
        }
        else
        {
            line = QLineF(origin, ScenePos());
        }

        tempAngle = line.angle();
        tempLength = line.length();
    }
    else
    {
        line = VGObject::BuildLine(origin, m_length, m_angle);
        tempAngle = m_angle;
        tempLength = m_length;

        if (m_rotationOriginId != NULL_ID)
        {
            origin = GetData()->GeometricObject<VPointF>(m_rotationOriginId)->toQPointF();
            DrawPoint(m_pointRotationOrigin, origin, Color(VColor::SupportColor2));
        }
        else
        {
            origin = line.p2();
            m_pointRotationOrigin->setVisible(false);
        }

        QLineF rLine;
        if (VFuzzyComparePossibleNulls(m_rotationAngle, INT_MIN))
        {
            rLine = QLineF(origin, ScenePos());

            if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
            {
                rLine.setAngle(CorrectAngle(rLine.angle()));
            }

            qreal cursorLength = rLine.length();
            rLine.setP2(Ray(origin, rLine.angle()));
            //Radius of point circle, but little bigger. Need handle with hover sizes.
            qreal minL = ScaledRadius(SceneScale(VAbstractValApplication::VApp()->getCurrentScene()))*1.5;
            if (cursorLength > minL)
            {
               tempRoationAngle = rLine.angle();
            }
            else
            {
                rLine.setAngle(0);
            }
        }
        else
        {
            rLine = QLineF(origin, Ray(origin, m_rotationAngle));
            tempRoationAngle = m_rotationAngle;
        }

        DrawLine(m_rotationLine, rLine, Color(VColor::SupportColor3), Qt::DashLine);
        DrawLine(m_xAxis, QLineF(origin, Ray(origin, 0)), Color(VColor::SupportColor3), Qt::DashLine);

        VArc arc(VPointF(origin),
                 ScaledRadius(SceneScale(VAbstractValApplication::VApp()->getCurrentScene()))*2, 0, tempRoationAngle);
        DrawPath(m_angleArc, arc.GetPath(), Color(VColor::SupportColor3), Qt::SolidLine, Qt::RoundCap);
    }
    DrawLine(this, line, Color(VColor::SupportColor2), Qt::DashLine);
    DrawPoint(m_pointFinish, line.p2(), Color(VColor::SupportColor));

    static const QString prefix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
    if (qFuzzyIsNull(m_length))
    {
        SetToolTip(tr("Length = %1%2, angle = %3°, <b>%4</b> - sticking angle, "
                      "<b>Mouse click</b> - finish selecting a position")
                       .arg(LengthToUser(tempLength), prefix, AngleToUser(tempAngle), VModifierKey::Shift()));
    }
    else
    {
        SetToolTip(tr("Length = %1%2, angle = %3°, rotation angle = %4°, <b>%5</b> - sticking angle, "
                      "<b>%6</b> - change rotation origin point, <b>Mouse click</b> - finish creating")
                       .arg(LengthToUser(tempLength), prefix, AngleToUser(tempAngle), AngleToUser(tempRoationAngle),
                            VModifierKey::Shift(), VModifierKey::Control()));
    }

    CreateMovedRotatedObjects(iPoint, iCurve, tempLength, tempAngle, tempRoationAngle, origin);
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolMove::Angle() const -> QString
{
    return QString::number(line().angle());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolMove::SetAngle(const QString &expression)
{
    m_angle = FindValFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolMove::RotationAngle() const -> QString
{
    return QString::number(m_rotationLine->line().angle());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolMove::SetRotationAngle(const QString &expression)
{
    m_rotationAngle = FindValFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolMove::Length() const -> QString
{
    return QString::number(LengthValue());
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolMove::LengthValue() const -> qreal
{
    return VAbstractValApplication::VApp()->fromPixel(line().length());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolMove::SetLength(const QString &expression)
{
    m_length = FindLengthFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
auto VisToolMove::AddOriginCurve(quint32 id, int &i) -> QGraphicsPathItem *
{
    const QSharedPointer<Item> curve = GetData()->template GeometricObject<Item>(id);

    ++i;
    VCurvePathItem *path = GetCurve(static_cast<quint32>(i), Color(VColor::SupportColor2));
    DrawPath(path, curve->GetPath(), curve->DirectionArrows(), Color(VColor::SupportColor2), Qt::SolidLine,
             Qt::RoundCap);

    return path;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
auto VisToolMove::AddMovedRotatedCurve(qreal angle, qreal length, quint32 id, int i, qreal rotationAngle,
                                       const QPointF &rotationOrigin) -> int
{
    const QSharedPointer<Item> curve = GetData()->template GeometricObject<Item>(id);

    ++i;
    VCurvePathItem *path = GetCurve(static_cast<quint32>(i), Color(VColor::SupportColor));
    const Item moved = curve->Move(length, angle).Rotate(rotationOrigin, rotationAngle);
    DrawPath(path, moved.GetPath(), moved.DirectionArrows(), Color(VColor::SupportColor), Qt::SolidLine, Qt::RoundCap);

    return i;
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolMove::GetOriginPoint(const QVector<QGraphicsItem *> &objects) -> QPointF
{
    QRectF boundingRect;
    for (auto *object : qAsConst(objects))
    {
        if (object)
        {
            QRectF childrenRect = object->childrenBoundingRect();
            //map to scene coordinate.
            childrenRect.translate(object->scenePos());

            QRectF itemRect = object->boundingRect();
            //map to scene coordinate.
            itemRect.translate(object->scenePos());

            boundingRect = boundingRect.united(itemRect);
            boundingRect = boundingRect.united(childrenRect);
        }
    }

    return boundingRect.center();
}

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")

auto VisToolMove::CreateOriginObjects(int &iPoint, int &iCurve) -> QVector<QGraphicsItem *>
{
    QVector<QGraphicsItem *> originObjects;
    originObjects.reserve(Objects().size());

    for (auto id : Objects())
    {
        const QSharedPointer<VGObject> obj = GetData()->GetGObject(id);

        // This check helps to find missed objects in the switch
        Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 8, "Not all objects were handled.");

        switch(static_cast<GOType>(obj->getType()))
        {
            case GOType::Point:
            {
                const QSharedPointer<VPointF> p = GetData()->GeometricObject<VPointF>(id);

                ++iPoint;
                VScaledEllipse *point = GetPoint(static_cast<quint32>(iPoint), Color(VColor::SupportColor2));
                DrawPoint(point, static_cast<QPointF>(*p), Color(VColor::SupportColor2));
                originObjects.append(point);

                break;
            }
            case GOType::Arc:
                originObjects.append(AddOriginCurve<VArc>(id, iCurve));
                break;
            case GOType::EllipticalArc:
                originObjects.append(AddOriginCurve<VEllipticalArc>(id, iCurve));
                break;
            case GOType::Spline:
                originObjects.append(AddOriginCurve<VSpline>(id, iCurve));
                break;
            case GOType::SplinePath:
                originObjects.append(AddOriginCurve<VSplinePath>(id, iCurve));
                break;
            case GOType::CubicBezier:
                originObjects.append(AddOriginCurve<VCubicBezier>(id, iCurve));
                break;
            case GOType::CubicBezierPath:
                originObjects.append(AddOriginCurve<VCubicBezierPath>(id, iCurve));
                break;
            case GOType::Unknown:
            case GOType::PlaceLabel:
                Q_UNREACHABLE();
                break;
        }
    }

    return originObjects;
}

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wswitch-default")

void VisToolMove::CreateMovedRotatedObjects(int &iPoint, int &iCurve, qreal length, qreal angle, qreal rotationAngle,
                                            const QPointF &rotationOrigin)
{
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
                VScaledEllipse *point = GetPoint(static_cast<quint32>(iPoint), Color(VColor::SupportColor));
                DrawPoint(point, static_cast<QPointF>(p->Move(length, angle).Rotate(rotationOrigin, rotationAngle)),
                          Color(VColor::SupportColor));
                break;
            }
            case GOType::Arc:
                iCurve = AddMovedRotatedCurve<VArc>(angle, length, id, iCurve, rotationAngle, rotationOrigin);
                break;
            case GOType::EllipticalArc:
                iCurve = AddMovedRotatedCurve<VEllipticalArc>(angle, length, id, iCurve, rotationAngle, rotationOrigin);
                break;
            case GOType::Spline:
                iCurve = AddMovedRotatedCurve<VSpline>(angle, length, id, iCurve, rotationAngle, rotationOrigin);
                break;
            case GOType::SplinePath:
                iCurve = AddMovedRotatedCurve<VSplinePath>(angle, length, id, iCurve, rotationAngle, rotationOrigin);
                break;
            case GOType::CubicBezier:
                iCurve = AddMovedRotatedCurve<VCubicBezier>(angle, length, id, iCurve, rotationAngle, rotationOrigin);
                break;
            case GOType::CubicBezierPath:
                iCurve = AddMovedRotatedCurve<VCubicBezierPath>(angle, length, id, iCurve, rotationAngle,
                                                                rotationOrigin);
                break;
            case GOType::Unknown:
            case GOType::PlaceLabel:
                Q_UNREACHABLE();
                break;
        }
    }
}

QT_WARNING_POP
