/************************************************************************
 **
 **  @file   vistoolcutarc.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 8, 2014
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

#ifndef VISTOOLCUTARC_H
#define VISTOOLCUTARC_H

#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>
#include <limits>

#include "../vmisc/def.h"
#include "vispath.h"

class VisToolCutArc final : public VisPath
{
    Q_OBJECT // NOLINT

public:
    explicit VisToolCutArc(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolCutArc() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    void SetArcId(quint32 newArcId);
    void SetLength(const QString &expression);

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Vis::ToolCutArc)
    };

private:
    Q_DISABLE_COPY_MOVE(VisToolCutArc) // NOLINT
    VScaledEllipse *m_point{nullptr};
    VCurvePathItem *m_arc1{nullptr};
    VCurvePathItem *m_arc2{nullptr};
    qreal m_length{std::numeric_limits<qreal>::infinity()};
    quint32 m_arcId{NULL_ID};
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolCutArc::SetArcId(quint32 newArcId)
{
    m_arcId = newArcId;
}

#endif // VISTOOLCUTARC_H
