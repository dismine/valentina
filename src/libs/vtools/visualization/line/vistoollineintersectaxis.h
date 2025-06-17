/************************************************************************
 **
 **  @file   vistoollineintersectaxis.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   19 10, 2014
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

#ifndef VISTOOLLINEINTERSECTAXIS_H
#define VISTOOLLINEINTERSECTAXIS_H


#include <QGraphicsItem>
#include <QLineF>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "visline.h"

class VisToolLineIntersectAxis : public VisLine
{
    Q_OBJECT // NOLINT
public:
    explicit VisToolLineIntersectAxis(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolLineIntersectAxis() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    auto Angle() const -> QString;
    void SetAngle(const QString &expression);
    void SetPoint1Id(quint32 value);
    void SetPoint2Id(quint32 value);
    void SetAxisPointId(quint32 value);

    auto type() const -> int override {return Type;}
    enum {Type = UserType + static_cast<int>(Vis::ToolLineIntersectAxis)};
private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VisToolLineIntersectAxis) // NOLINT
    quint32         m_point1Id{NULL_ID};
    quint32         m_point2Id{NULL_ID};
    quint32         m_axisPointId{NULL_ID};
    qreal           m_angle{-1};
    VScaledEllipse *m_point{nullptr};
    VScaledEllipse *m_lineP1{nullptr};
    VScaledEllipse *m_lineP2{nullptr};
    VScaledEllipse *m_basePoint{nullptr};
    VScaledLine    *m_baseLine{nullptr};
    VScaledLine    *m_axisLine{nullptr};
    VScaledLine    *m_lineIntersection{nullptr};

    void ShowIntersection(const QLineF &axis_line, const QLineF &base_line);
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolLineIntersectAxis::SetPoint1Id(quint32 value)
{
    m_point1Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolLineIntersectAxis::SetPoint2Id(quint32 value)
{
    m_point2Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolLineIntersectAxis::SetAxisPointId(quint32 value)
{
    m_axisPointId = value;
}

#endif // VISTOOLLINEINTERSECTAXIS_H
