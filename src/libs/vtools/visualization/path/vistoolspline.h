/************************************************************************
 **
 **  @file   vistoolspline.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   18 8, 2014
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

#ifndef VISTOOLSPLINE_H
#define VISTOOLSPLINE_H

#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "vispath.h"

class VControlPointSpline;

class VisToolSpline : public VisPath // clazy:exclude=ctor-missing-parent-argument
{
    Q_OBJECT // NOLINT

public:
    explicit VisToolSpline(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolSpline() override;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    void SetPoint1Id(quint32 value);
    void SetPoint4Id(quint32 value);
    void SetAngle1(qreal value);
    void SetAngle2(qreal value);
    void SetKAsm1(qreal value);
    void SetKAsm2(qreal value);
    void SetKCurve(qreal value);

    auto GetP2() const -> QPointF;
    auto GetP3() const -> QPointF;

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Vis::ToolSpline)
    };
public slots:
    void MouseLeftPressed();
    void MouseLeftReleased();

private:
    Q_DISABLE_COPY_MOVE(VisToolSpline) // NOLINT
    quint32 m_point1Id{NULL_ID};
    quint32 m_point4Id{NULL_ID};
    VScaledEllipse *m_point1{nullptr};
    VScaledEllipse *m_point4{nullptr};
    qreal m_angle1;
    qreal m_angle2;
    qreal m_kAsm1{1};
    qreal m_kAsm2{1};
    qreal m_kCurve{1};

    bool m_isLeftMousePressed{false};
    bool m_p2Selected{false};
    bool m_p3Selected{false};

    QPointF m_p2{};
    QPointF m_p3{};

    QVector<VControlPointSpline *> m_controlPoints{};

    void DragFirstControlPoint(const QPointF &point);
    void DragLastControlPoint(const QPointF &point);
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolSpline::SetPoint1Id(quint32 value)
{
    m_point1Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolSpline::SetPoint4Id(quint32 value)
{
    m_point4Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolSpline::SetAngle1(qreal value)
{
    m_angle1 = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolSpline::SetAngle2(qreal value)
{
    m_angle2 = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolSpline::SetKAsm1(qreal value)
{
    m_kAsm1 = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolSpline::SetKAsm2(qreal value)
{
    m_kAsm2 = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolSpline::SetKCurve(qreal value)
{
    m_kCurve = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VisToolSpline::GetP2() const -> QPointF
{
    return m_p2;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VisToolSpline::GetP3() const -> QPointF
{
    return m_p3;
}

#endif // VISTOOLSPLINE_H
