/************************************************************************
 **
 **  @file   vistoolellipticalarcwithlength.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 10, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
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
#ifndef VISTOOLELLIPTICALARCWITHLENGTH_H
#define VISTOOLELLIPTICALARCWITHLENGTH_H

#include "../vmisc/def.h"
#include "vispath.h"

class VPointF;
class VEllipticalArc;

class VisToolEllipticalArcWithLength final : public VisPath
{
    Q_OBJECT // NOLINT

public:
    explicit VisToolEllipticalArcWithLength(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolEllipticalArcWithLength() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    void SetCenterId(quint32 newCenterId);
    void SetRadius1(const QString &expression);
    void SetRadius2(const QString &expression);
    void SetF1(const QString &expression);
    void SetLength(const QString &expression);
    void SetRotationAngle(const QString &expression);

    auto StartingRotationAngle() const -> qreal;

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Vis::ToolEllipticalArcWithLength)
    };

private:
    Q_DISABLE_COPY_MOVE(VisToolEllipticalArcWithLength) // NOLINT

    VScaledEllipse *m_arcCenter{nullptr};
    VScaledLine *m_radius1Line{nullptr};
    VScaledLine *m_radius2Line{nullptr};
    VScaledEllipse *m_f1Point{nullptr};
    qreal m_radius1{0};
    qreal m_radius2{0};
    qreal m_f1{-1};
    qreal m_length{0};
    qreal m_startingRotationAngle{INT_MAX};
    qreal m_rotationAngle{INT_MAX};
    quint32 m_centerId{NULL_ID};

    void DrawRadius1Line(const QPointF &center, qreal radius, qreal rotationAngle = 0);
    void DrawRadius2Line(const QPointF &center, qreal radius, qreal rotationAngle = 0);
    auto DrawElArc(const VPointF &center, qreal radius1, qreal radius2, qreal f1, qreal f2, qreal rotationAngle = 0)
        -> VEllipticalArc;
    auto DrawElArc(qreal length, const VPointF &center, qreal radius1, qreal radius2, qreal f1, qreal rotationAngle = 0)
        -> VEllipticalArc;
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolEllipticalArcWithLength::SetCenterId(quint32 newCenterId)
{
    m_centerId = newCenterId;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VisToolEllipticalArcWithLength::StartingRotationAngle() const -> qreal
{
    return m_startingRotationAngle;
}

#endif // VISTOOLELLIPTICALARCWITHLENGTH_H
