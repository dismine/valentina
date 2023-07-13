/************************************************************************
 **
 **  @file   vistooltruedarts.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   23 6, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#ifndef VISTOOLTRUEDARTS_H
#define VISTOOLTRUEDARTS_H


#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "visline.h"

class VisToolTrueDarts :public VisLine
{
    Q_OBJECT // NOLINT
public:
    explicit VisToolTrueDarts(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolTrueDarts() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    void SetBaseLineP1Id(quint32 value);
    void SetBaseLineP2Id(quint32 value);
    void SetD1PointId(quint32 value);
    void SetD2PointId(quint32 value);
    void SetD3PointId(quint32 value);

    auto type() const -> int override {return Type;}
    enum {Type = UserType + static_cast<int>(Vis::ToolTrueDarts)};
private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VisToolTrueDarts) // NOLINT
    quint32 m_baseLineP1Id{NULL_ID};
    quint32 m_baseLineP2Id{NULL_ID};
    quint32 m_dartP1Id{NULL_ID};
    quint32 m_dartP2Id{NULL_ID};
    quint32 m_dartP3Id{NULL_ID};

    VScaledEllipse *m_point1{nullptr};
    VScaledEllipse *m_point2{nullptr};
    VScaledEllipse *m_baseLineP1{nullptr};
    VScaledEllipse *m_baseLineP2{nullptr};
    VScaledEllipse *m_dartP1{nullptr};
    VScaledEllipse *m_dartP2{nullptr};
    VScaledEllipse *m_dartP3{nullptr};

    VScaledLine *m_lineblP1P1{nullptr};
    VScaledLine *m_lineblP2P2{nullptr};
    VScaledLine *m_p1d2{nullptr};
    VScaledLine *m_d2p2{nullptr};
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolTrueDarts::SetBaseLineP1Id(quint32 value)
{
    m_baseLineP1Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolTrueDarts::SetBaseLineP2Id(quint32 value)
{
    m_baseLineP2Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolTrueDarts::SetD1PointId(quint32 value)
{
    m_dartP1Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolTrueDarts::SetD2PointId(quint32 value)
{
    m_dartP2Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolTrueDarts::SetD3PointId(quint32 value)
{
    m_dartP3Id = value;
}

#endif // VISTOOLTRUEDARTS_H
