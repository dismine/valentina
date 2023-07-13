/************************************************************************
 **
 **  @file   vistooltriangle.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 8, 2014
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

#ifndef VISTOOLTRIANGLE_H
#define VISTOOLTRIANGLE_H


#include <QColor>
#include <QGraphicsItem>
#include <QLineF>
#include <QMetaObject>
#include <QObject>
#include <QString>
#include <Qt>
#include <QtGlobal>

#include "../vmisc/def.h"
#include "visline.h"

class VisToolTriangle : public VisLine
{
    Q_OBJECT // NOLINT
public:
    explicit VisToolTriangle(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisToolTriangle() override = default;

    void RefreshGeometry() override;
    void VisualMode(quint32 id) override;

    void SetObject1Id(quint32 value);
    void SetObject2Id(quint32 value);
    void SetHypotenuseP1Id(quint32 value);
    void SetHypotenuseP2Id(quint32 value);

    auto type() const -> int override {return Type;}
    enum {Type = UserType + static_cast<int>(Vis::ToolTriangle)};
private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VisToolTriangle) // NOLINT
    quint32         m_object1Id{NULL_ID};//axis first point
    quint32         m_object2Id{NULL_ID};//axis second point
    quint32         m_hypotenuseP1Id{NULL_ID};
    quint32         m_hypotenuseP2Id{NULL_ID};
    VScaledEllipse *m_point{nullptr};
    VScaledEllipse *m_axisP1{nullptr};
    VScaledEllipse *m_axisP2{nullptr};
    VCurvePathItem *m_axis{nullptr};
    VScaledEllipse *m_hypotenuseP1{nullptr};
    VScaledEllipse *m_hypotenuseP2{nullptr};
    VScaledLine    *m_foot1{nullptr};
    VScaledLine    *m_foot2{nullptr};

    static void DrawAimedAxis(VCurvePathItem *item, const QLineF &line, const QColor &color,
                              Qt::PenStyle style = Qt::SolidLine);
    static void DrawArrow(const QLineF &axis, QPainterPath &path, const qreal &arrow_size);
};

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolTriangle::SetObject1Id(quint32 value)
{
    m_object1Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolTriangle::SetObject2Id(quint32 value)
{
    m_object2Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolTriangle::SetHypotenuseP1Id(quint32 value)
{
    m_hypotenuseP1Id = value;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VisToolTriangle::SetHypotenuseP2Id(quint32 value)
{
    m_hypotenuseP2Id = value;
}

#endif // VISTOOLTRIANGLE_H
