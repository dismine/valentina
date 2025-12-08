/************************************************************************
 **
 **  @file   vistoolgraduatedcurve.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 12, 2025
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
#ifndef VISTOOLGRADUATEDCURVE_H
#define VISTOOLGRADUATEDCURVE_H

#include "vispath.h"

struct VRawGraduatedCurveOffset;

class VisToolGraduatedCurve final : public VisPath
{
    Q_OBJECT // NOLINT

public:
    explicit VisToolGraduatedCurve(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolGraduatedCurve() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    void SetCurveId(quint32 newCurveId);
    void SetOffsets(const QVector<VRawGraduatedCurveOffset> &offsets);

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Vis::ToolParallelCurve)
    };

private:
    Q_DISABLE_COPY_MOVE(VisToolGraduatedCurve) // NOLINT

    VCurvePathItem *m_originalCurve{nullptr};

    QVector<qreal> m_offsets{};
    quint32 m_curveId{NULL_ID};
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolGraduatedCurve::SetCurveId(quint32 newCurveId)
{
    m_curveId = newCurveId;
}

#endif // VISTOOLGRADUATEDCURVE_H
