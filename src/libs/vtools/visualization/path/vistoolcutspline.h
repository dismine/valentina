/************************************************************************
 **
 **  @file   vistoolcutspline.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   6 9, 2014
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

#ifndef VISTOOLCUTSPLINE_H
#define VISTOOLCUTSPLINE_H

#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>
#include <limits>

#include "../vmisc/def.h"
#include "vispath.h"

class VisToolCutSpline final : public VisPath
{
    Q_OBJECT // NOLINT

public:
    explicit VisToolCutSpline(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolCutSpline() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    void SetSplineId(quint32 id);
    void SetLength(const QString &expression);

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Vis::ToolCutSpline)
    };

private:
    Q_DISABLE_COPY_MOVE(VisToolCutSpline) // NOLINT
    VScaledEllipse *m_point{nullptr};
    VCurvePathItem *m_spl1{nullptr};
    VCurvePathItem *m_spl2{nullptr};
    qreal m_length{std::numeric_limits<qreal>::infinity()};
    quint32 m_splineId{NULL_ID};
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolCutSpline::SetSplineId(quint32 id)
{
    m_splineId = id;
}

#endif // VISTOOLCUTSPLINE_H
