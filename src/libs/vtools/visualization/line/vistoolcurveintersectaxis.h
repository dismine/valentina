/************************************************************************
 **
 **  @file   vistoolcurveintersectaxis.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   21 10, 2014
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

#ifndef VISTOOLCURVEINTERSECTAXIS_H
#define VISTOOLCURVEINTERSECTAXIS_H

#include <qcompilerdetection.h>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "visline.h"

class VisToolCurveIntersectAxis final : public VisLine
{
    Q_OBJECT // NOLINT
public:
    explicit VisToolCurveIntersectAxis(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolCurveIntersectAxis() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    auto Angle() const -> QString;
    void SetAngle(const QString &expression);

    void SetCurveId(quint32 newCurveId);
    void setAxisPointId(const quint32 &value);

    auto type() const -> int override {return Type;}
    enum {Type = UserType + static_cast<int>(Vis::ToolLineIntersectAxis)};

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VisToolCurveIntersectAxis) // NOLINT
    quint32         m_axisPointId{NULL_ID};
    qreal           m_angle{-1};
    VScaledEllipse *m_point{nullptr};
    VScaledEllipse *m_basePoint{nullptr};
    VScaledLine    *m_baseLine{nullptr};
    VScaledLine    *m_axisLine{nullptr};
    VCurvePathItem *m_visCurve{nullptr};
    quint32         m_curveId{NULL_ID};
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolCurveIntersectAxis::SetCurveId(quint32 newCurveId)
{
    m_curveId = newCurveId;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolCurveIntersectAxis::setAxisPointId(const quint32 &value)
{
    m_axisPointId = value;
}

#endif // VISTOOLCURVEINTERSECTAXIS_H
