/************************************************************************
 **
 **  @file   visflippingoperation.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 3, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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
#ifndef VISFLIPPINGOPERATION_H
#define VISFLIPPINGOPERATION_H

#include "visoperation.h"

class VisFlippingOperation : public VisOperation
{
    Q_OBJECT // NOLINT

public:
    explicit VisFlippingOperation(const VContainer *data, QGraphicsItem *parent = nullptr);
    ~VisFlippingOperation() override = default;

    auto type() const -> int override { return Type; }
    enum
    {
        Type = UserType + static_cast<int>(Vis::FlippingOperation)
    };

protected:
    void CreateFlippedObjects(
        int &iPoint, int &iCurve, quint32 originPointId, const QPointF &firstPoint, const QPointF &secondPoint);

    template<class Item>
    auto AddFlippedCurve(quint32 originPointId, const QPointF &firstPoint, const QPointF &secondPoint, quint32 id, int i)
        -> int;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VisFlippingOperation) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
inline auto VisFlippingOperation::AddFlippedCurve(
    quint32 originPointId, const QPointF &firstPoint, const QPointF &secondPoint, quint32 id, int i) -> int
{
    if (originPointId == NULL_ID)
    {
        return i;
    }

    const QSharedPointer<Item> curve = GetData()->template GeometricObject<Item>(id);

    ++i;
    VCurvePathItem *path = GetCurve(static_cast<quint32>(i), VColorRole::VisSupportColor);
    const Item flipped = curve->Flip(QLineF(firstPoint, secondPoint));
    DrawPath(path, flipped.GetPath(), flipped.DirectionArrows(), Qt::SolidLine, Qt::RoundCap);

    return i;
}

#endif // VISFLIPPINGOPERATION_H
