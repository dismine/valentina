/************************************************************************
 **
 **  @file   vistoolshoulderpoint.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   25 7, 2014
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

#ifndef VISTOOLSHOULDERPOINT_H
#define VISTOOLSHOULDERPOINT_H


#include <QGraphicsItem>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "visline.h"

class VisToolShoulderPoint : public VisLine
{
    Q_OBJECT // NOLINT
public:
    explicit VisToolShoulderPoint(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolShoulderPoint() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    void SetPoint3Id(quint32 newPoint3Id);
    void SetLineP1Id(quint32 value);
    void SetLineP2Id(quint32 value);
    void SetLength(const QString &expression);

    auto type() const -> int override {return Type;}
    enum {Type = UserType + static_cast<int>(Vis::ToolShoulderPoint)};
private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VisToolShoulderPoint) // NOLINT
    quint32         m_lineP1Id{NULL_ID};
    quint32         m_lineP2Id{NULL_ID};
    VScaledEllipse *m_point{nullptr};
    VScaledEllipse *m_line1P1{nullptr};
    VScaledEllipse *m_line1P2{nullptr};
    VScaledLine    *m_line1{nullptr};
    VScaledEllipse *m_line2P2{nullptr};
    VScaledLine    *m_line2{nullptr};
    VScaledLine    *m_line3{nullptr};
    qreal           m_length{0};
    quint32         m_point3Id{NULL_ID};
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolShoulderPoint::SetPoint3Id(quint32 newPoint3Id)
{
    m_point3Id = newPoint3Id;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolShoulderPoint::SetLineP1Id(quint32 value)
{
    m_lineP1Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolShoulderPoint::SetLineP2Id(quint32 value)
{
    m_lineP2Id = value;
}

#endif // VISTOOLSHOULDERPOINT_H
