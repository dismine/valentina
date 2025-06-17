/************************************************************************
 **
 **  @file   vistoollineintersect.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   14 8, 2014
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

#ifndef VISTOOLLINEINTERSECT_H
#define VISTOOLLINEINTERSECT_H


#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "visline.h"

class VisToolLineIntersect :public VisLine
{
    Q_OBJECT // NOLINT
public:
    explicit VisToolLineIntersect(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolLineIntersect() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    void SetLine1P1Id(quint32 value);
    void SetLine1P2Id(quint32 value);
    void SetLine2P1Id(quint32 value);
    void SetLine2P2Id(quint32 value);

    auto type() const -> int override {return Type;}
    enum {Type = UserType + static_cast<int>(Vis::ToolLineIntersect)};
private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VisToolLineIntersect) // NOLINT
    quint32         m_line1P1Id{NULL_ID};
    quint32         m_line1P2Id{NULL_ID};
    quint32         m_line2P1Id{NULL_ID};
    quint32         m_line2P2Id{NULL_ID};
    VScaledEllipse *m_point{nullptr};
    VScaledEllipse *m_line1P1{nullptr};
    VScaledEllipse *m_line1P2{nullptr};
    VScaledLine    *m_line1{nullptr};
    VScaledEllipse *m_line2P1{nullptr};
    VScaledEllipse *m_line2P2{nullptr};
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolLineIntersect::SetLine1P1Id(quint32 value)
{
    m_line1P1Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolLineIntersect::SetLine1P2Id(quint32 value)
{
    m_line1P2Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolLineIntersect::SetLine2P1Id(quint32 value)
{
    m_line2P1Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolLineIntersect::SetLine2P2Id(quint32 value)
{
    m_line2P2Id = value;
}

#endif // VISTOOLLINEINTERSECT_H
