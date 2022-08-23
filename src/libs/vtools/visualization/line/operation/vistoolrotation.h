/************************************************************************
 **
 **  @file   vistoolrotation.h
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

#ifndef VISTOOLROTATION_H
#define VISTOOLROTATION_H

#include <qcompilerdetection.h>
#include <QColor>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVector>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "visoperation.h"

class VisToolRotation : public VisOperation
{
    Q_OBJECT // NOLINT
public:
    explicit VisToolRotation(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolRotation() override = default;

    void RefreshGeometry() override;

    void SetOriginPointId(quint32 value);

    auto Angle() const -> QString;
    void SetAngle(const QString &expression);

    auto type() const -> int override {return Type;}
    enum {Type = UserType + static_cast<int>(Vis::ToolRotation)};
private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VisToolRotation) // NOLINT
    qreal           m_angle{INT_MIN};
    VScaledEllipse *m_point{nullptr};
    VCurvePathItem *m_angleArc{nullptr};
    VScaledLine    *m_xAxis{nullptr};
    quint32         m_originPointId{NULL_ID};

    template <class Item>
    auto AddCurve(qreal angle, const QPointF &origin, quint32 id, int i) -> int;
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolRotation::SetOriginPointId(quint32 value)
{
    m_originPointId = value;
}

#endif // VISTOOLROTATION_H
