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

#ifndef VISTOOLMOVE_H
#define VISTOOLMOVE_H

#include <QColor>
#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QPointF>
#include <QString>
#include <QVector>
#include <QtGlobal>
#include <limits>

#include "../vmisc/def.h"
#include "visoperation.h"

class VisToolMove : public VisOperation
{
    Q_OBJECT // NOLINT

public:
    explicit VisToolMove(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolMove() override = default;

    void RefreshGeometry() override;

    auto Angle() const -> QString;
    void SetAngle(const QString &expression);

    auto RotationAngle() const -> QString;
    void SetRotationAngle(const QString &expression);

    auto Length() const -> QString;
    auto LengthValue() const -> qreal;
    void SetLength(const QString &expression);

    void SetRotationOriginPointId(quint32 value);

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Vis::ToolMove)
    };

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VisToolMove) // NOLINT
    qreal m_angle{0};
    qreal m_rotationAngle{INT_MIN};
    qreal m_length{std::numeric_limits<qreal>::infinity()};
    VScaledEllipse *m_pointOrigin{nullptr};
    VScaledEllipse *m_pointRotationOrigin{nullptr};
    VScaledEllipse *m_pointFinish{nullptr};
    VCurvePathItem *m_angleArc{nullptr};
    VScaledLine *m_rotationLine{nullptr};
    VScaledLine *m_xAxis{nullptr};
    quint32 m_rotationOriginId{NULL_ID};

    template <class Item> auto AddOriginCurve(quint32 id, int &i) -> QGraphicsPathItem *;

    template <class Item>
    auto AddMovedRotatedCurve(qreal angle, qreal length, quint32 id, int i, qreal rotationAngle,
                              const QPointF &rotationOrigin) -> int;

    static auto GetOriginPoint(const QVector<QGraphicsItem *> &objects) -> QPointF;

    auto CreateOriginObjects(int &iPoint, int &iCurve) -> QVector<QGraphicsItem *>;
    void CreateMovedRotatedObjects(int &iPoint, int &iCurve, qreal length, qreal angle, qreal rotationAngle,
                                   const QPointF &rotationOrigin);
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolMove::SetRotationOriginPointId(quint32 value)
{
    m_rotationOriginId = value;
}

#endif // VISTOOLMOVE_H
