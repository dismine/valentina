/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 9, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#ifndef VISOPERATION_H
#define VISOPERATION_H

#include <QtGlobal>

#include "../visline.h"
#include "../vpatterndb/vcontainer.h"

class VisOperation : public VisLine
{
    Q_OBJECT // NOLINT
public:
    explicit VisOperation(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisOperation() override;

    void SetObjects(const QVector<quint32> &objects);
    void VisualMode(quint32 id = NULL_ID) override;

    auto type() const -> int override {return Type;}
    enum {Type = UserType + static_cast<int>(Vis::ToolRotation)};

protected:
    auto GetPoint(quint32 i, const QColor &color) -> VScaledEllipse *;
    auto GetCurve(quint32 i, const QColor &color) -> VCurvePathItem *;

    template <class Item>
    auto AddFlippedCurve(quint32 originPointId, const QPointF &firstPoint, const QPointF &secondPoint, quint32 id,
                         int i) -> int;

    void RefreshFlippedObjects(quint32 originPointId, const QPointF &firstPoint, const QPointF &secondPoint);

    auto Objects() const -> const QVector<quint32> &;
private:
    Q_DISABLE_COPY_MOVE(VisOperation) // NOLINT

    QVector<quint32> m_objects{};

    QVector<VScaledEllipse *> m_points{};
    QVector<VCurvePathItem *> m_curves{};
};

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
auto VisOperation::AddFlippedCurve(quint32 originPointId, const QPointF &firstPoint, const QPointF &secondPoint,
                                   quint32 id, int i) -> int
{
    const QSharedPointer<Item> curve = GetData()->template GeometricObject<Item>(id);

    ++i;
    VCurvePathItem *path = GetCurve(static_cast<quint32>(i), Color(VColor::SupportColor2));
    DrawPath(path, curve->GetPath(), curve->DirectionArrows(), Color(VColor::SupportColor2), Qt::SolidLine,
             Qt::RoundCap);

    ++i;
    path = GetCurve(static_cast<quint32>(i), Color(VColor::SupportColor));
    if (originPointId != NULL_ID)
    {
        const Item flipped = curve->Flip(QLineF(firstPoint, secondPoint));
        DrawPath(path, flipped.GetPath(), flipped.DirectionArrows(), Color(VColor::SupportColor), Qt::SolidLine,
                 Qt::RoundCap);
    }

    return i;
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VisOperation::Objects() const -> const QVector<quint32> &
{
    return m_objects;
}

#endif // VISOPERATION_H
