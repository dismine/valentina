/************************************************************************
 **
 **  @file   vistoolparallelcurve.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 11, 2025
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
#ifndef VISTOOLPARALLELCURVE_H
#define VISTOOLPARALLELCURVE_H

#include "vispath.h"

class VisToolParallelCurve final : public VisPath
{
    Q_OBJECT // NOLINT

public:
    explicit VisToolParallelCurve(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolParallelCurve() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    void SetCurveId(quint32 newCurveId);
    void SetOffsetWidth(const QString &expression);

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Vis::ToolParallelCurve)
    };

private:
    Q_DISABLE_COPY_MOVE(VisToolParallelCurve) // NOLINT

    VCurvePathItem *m_originalCurve{nullptr};

    qreal m_width{std::numeric_limits<qreal>::infinity()};
    quint32 m_curveId{NULL_ID};
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolParallelCurve::SetCurveId(quint32 newCurveId)
{
    m_curveId = newCurveId;
}

#endif // VISTOOLPARALLELCURVE_H
