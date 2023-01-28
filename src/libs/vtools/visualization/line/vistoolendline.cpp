/************************************************************************
 **
 **  @file   vistoolendline.cpp
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

#include "vistoolendline.h"

#include <QColor>
#include <QGuiApplication>
#include <QLineF>
#include <QPointF>
#include <QSharedPointer>
#include <Qt>
#include <new>

#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vmisc/vmodifierkey.h"
#include "../vpatterndb/vcontainer.h"
#include "../visualization.h"
#include "visline.h"
#include "../vpatterndb/vtranslatevars.h"

//---------------------------------------------------------------------------------------------------------------------
VisToolEndLine::VisToolEndLine(const VContainer *data, QGraphicsItem *parent)
    : VisLine(data, parent)
{
    SetMainColor(Qt::red);

    m_point = InitPoint(Color(VColor::MainColor), this);
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEndLine::RefreshGeometry()
{
    const QSharedPointer<VPointF> first = GetData()->GeometricObject<VPointF>(m_basePointId);
    QLineF line;
    if (qFuzzyIsNull(m_length))
    {
        if (GetMode() == Mode::Creation)
        {
            if (QGuiApplication::keyboardModifiers() == Qt::ShiftModifier)
            {
                line = QLineF(static_cast<QPointF>(*first), ScenePos());
                line.setAngle(CorrectAngle(line.angle()));
            }
            else
            {
                line = QLineF(static_cast<QPointF>(*first), ScenePos());
            }
        }
        else
        {
            DrawPoint(m_point, static_cast<QPointF>(*first), Color(VColor::MainColor));
        }
    }
    else
    {
        line = VGObject::BuildLine(static_cast<QPointF>(*first), m_length, m_angle);
        DrawPoint(m_point, line.p2(), Color(VColor::MainColor));
    }
    DrawLine(this, line, Color(VColor::MainColor), LineStyle());
    static const QString prefix = UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true);
    SetToolTip(tr("<b>Point at distance and angle</b>: angle = %1°, length = %2%3; "
                  "<b>%4</b> - sticking angle, <b>%5</b> - finish creation")
                   .arg(AngleToUser(this->line().angle()), LengthToUser(this->line().length()), prefix,
                        VModifierKey::Shift(), VModifierKey::EnterKey()));
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEndLine::VisualMode(quint32 id)
{
    m_basePointId = id;
    StartVisualMode();
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolEndLine::Angle() const -> QString
{
    return QString::number(this->line().angle());
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEndLine::SetAngle(const QString &expression)
{
    m_angle = FindValFromUser(expression, GetData()->DataVariables());
}

//---------------------------------------------------------------------------------------------------------------------
auto VisToolEndLine::Length() const -> QString
{
    return QString::number(VAbstractValApplication::VApp()->fromPixel(this->line().length()));
}

//---------------------------------------------------------------------------------------------------------------------
void VisToolEndLine::SetLength(const QString &expression)
{
    m_length = FindLengthFromUser(expression, GetData()->DataVariables());
}
