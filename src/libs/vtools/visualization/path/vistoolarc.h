/************************************************************************
 **
 **  @file   vistoolarc.h
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

#ifndef VISTOOLARC_H
#define VISTOOLARC_H

#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "vispath.h"

class VisToolArc final : public VisPath
{
    Q_OBJECT // NOLINT

public:
    explicit VisToolArc(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolArc() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    void SetCenterId(quint32 newCenterId);
    void SetRadius(const QString &expression);
    void SetF1(const QString &expression);
    void SetF2(const QString &expression);

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Vis::ToolArc)
    };

    auto StickyEnd(qreal angle) const -> qreal;

private:
    Q_DISABLE_COPY_MOVE(VisToolArc) // NOLINT
    VScaledEllipse *m_arcCenter{nullptr};
    VScaledEllipse *m_f1Point{nullptr};
    qreal m_radius{0};
    qreal m_f1{-1};
    qreal m_f2{-1};
    quint32 m_centerId{NULL_ID};
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolArc::SetCenterId(quint32 newCenterId)
{
    m_centerId = newCenterId;
}

#endif // VISTOOLARC_H
