/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 9, 2016
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

#include "vabstractflipping.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/varc.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../vgeometry/vellipticalarc.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepath.h"

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto CreatePoint(quint32 idTool,
                 const SourceItem &sItem,
                 const QPointF &firstPoint,
                 const QPointF &secondPoint,
                 VContainer *data) -> DestinationItem
{
    const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(sItem.id);
    VPointF rotated = point->Flip(QLineF(firstPoint, secondPoint), sItem.name);
    rotated.setIdObject(idTool);

    return {.id = data->AddGObject(new VPointF(rotated)),
            .mx = rotated.mx(),
            .my = rotated.my(),
            .showLabel = rotated.IsShowLabel()};
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
auto CreateItem(
    quint32 idTool, const SourceItem &sItem, const QPointF &firstPoint, const QPointF &secondPoint, VContainer *data)
    -> DestinationItem
{
    const QSharedPointer<Item> i = data->GeometricObject<Item>(sItem.id);
    Item rotated = i->Flip(QLineF(firstPoint, secondPoint), sItem.name);
    rotated.setIdObject(idTool);

    if (sItem.penStyle != TypeLineDefault)
    {
        rotated.SetPenStyle(sItem.penStyle);
    }

    if (sItem.color != ColorDefault)
    {
        rotated.SetColor(sItem.color);
    }

    return {.id = data->AddGObject(new Item(rotated))};
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
auto CreateCurve(
    quint32 idTool, const SourceItem &sItem, const QPointF &firstPoint, const QPointF &secondPoint, VContainer *data)
    -> DestinationItem
{
    const DestinationItem item = CreateItem<Item>(idTool, sItem, firstPoint, secondPoint, data);
    data->AddSpline(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
auto CreateCurveWithSegments(
    quint32 idTool, const SourceItem &sItem, const QPointF &firstPoint, const QPointF &secondPoint, VContainer *data)
    -> DestinationItem
{
    const DestinationItem item = CreateItem<Item>(idTool, sItem, firstPoint, secondPoint, data);
    data->AddCurveWithSegments(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
auto CreateArc(
    quint32 idTool, const SourceItem &sItem, const QPointF &firstPoint, const QPointF &secondPoint, VContainer *data)
    -> DestinationItem
{
    const DestinationItem item = CreateItem<Item>(idTool, sItem, firstPoint, secondPoint, data);
    data->AddArc(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
auto CreateDestinationObject(
    quint32 id, const SourceItem &object, GOType type, const QPointF &fPoint, const QPointF &sPoint, VContainer *data)
    -> DestinationItem
{
    // This check helps to find missed objects in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 8, "Not all objects were handled.");

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wswitch-default")
    QT_WARNING_DISABLE_CLANG("-Wswitch-default")

    switch (type)
    {
        case GOType::Point:
            return CreatePoint(id, object, fPoint, sPoint, data);
        case GOType::Arc:
            return CreateArc<VArc>(id, object, fPoint, sPoint, data);
        case GOType::EllipticalArc:
            return CreateArc<VEllipticalArc>(id, object, fPoint, sPoint, data);
        case GOType::Spline:
            return CreateCurve<VSpline>(id, object, fPoint, sPoint, data);
        case GOType::SplinePath:
            return CreateCurveWithSegments<VSplinePath>(id, object, fPoint, sPoint, data);
        case GOType::CubicBezier:
            return CreateCurve<VCubicBezier>(id, object, fPoint, sPoint, data);
        case GOType::CubicBezierPath:
            return CreateCurveWithSegments<VCubicBezierPath>(id, object, fPoint, sPoint, data);
        case GOType::Unknown:
        case GOType::PlaceLabel:
            Q_UNREACHABLE();
            break;
    }

    QT_WARNING_POP
    Q_UNREACHABLE();
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
void CreateDestinationObjects(VAbstractOperationInitData &initData, const QPointF &fPoint, const QPointF &sPoint)
{
    for (const auto &object : std::as_const(initData.source))
    {
        const QSharedPointer<VGObject> obj = initData.data->GetGObject(object.id);
        const DestinationItem item
            = CreateDestinationObject(initData.id, object, obj->getType(), fPoint, sPoint, initData.data);
        initData.destination.append(item);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void UpdatePoint(quint32 idTool,
                 const SourceItem &sItem,
                 const QPointF &firstPoint,
                 const QPointF &secondPoint,
                 VContainer *data,
                 const DestinationItem &item)
{
    const QSharedPointer<VPointF> point = data->GeometricObject<VPointF>(sItem.id);
    VPointF rotated = point->Flip(QLineF(firstPoint, secondPoint), sItem.name);
    rotated.setIdObject(idTool);
    rotated.setMx(item.mx);
    rotated.setMy(item.my);
    rotated.SetShowLabel(item.showLabel);

    data->UpdateGObject(item.id, new VPointF(rotated));
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
void UpdateItem(quint32 idTool,
                const SourceItem &sItem,
                const QPointF &firstPoint,
                const QPointF &secondPoint,
                VContainer *data,
                quint32 id)
{
    const QSharedPointer<Item> i = data->GeometricObject<Item>(sItem.id);
    Item rotated = i->Flip(QLineF(firstPoint, secondPoint), sItem.name);
    rotated.setIdObject(idTool);

    if (sItem.penStyle != TypeLineDefault)
    {
        rotated.SetPenStyle(sItem.penStyle);
    }

    if (sItem.color != ColorDefault)
    {
        rotated.SetColor(sItem.color);
    }

    data->UpdateGObject(id, new Item(rotated));
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
void UpdateCurve(quint32 idTool,
                 const SourceItem &sItem,
                 const QPointF &firstPoint,
                 const QPointF &secondPoint,
                 VContainer *data,
                 quint32 id)
{
    UpdateItem<Item>(idTool, sItem, firstPoint, secondPoint, data, id);
    data->AddSpline(data->GeometricObject<Item>(id), id);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
void UpdateCurveWithSegments(quint32 idTool,
                             const SourceItem &sItem,
                             const QPointF &firstPoint,
                             const QPointF &secondPoint,
                             VContainer *data,
                             quint32 id)
{
    UpdateItem<Item>(idTool, sItem, firstPoint, secondPoint, data, id);
    data->AddCurveWithSegments(data->GeometricObject<Item>(id), id);
}

//---------------------------------------------------------------------------------------------------------------------
template<class Item>
void UpdateArc(quint32 idTool,
               const SourceItem &sItem,
               const QPointF &firstPoint,
               const QPointF &secondPoint,
               VContainer *data,
               quint32 id)
{
    UpdateItem<Item>(idTool, sItem, firstPoint, secondPoint, data, id);
    data->AddArc(data->GeometricObject<Item>(id), id);
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateDestinationObject(quint32 id,
                             const SourceItem &object,
                             GOType type,
                             const QPointF &fPoint,
                             const QPointF &sPoint,
                             VContainer *data,
                             const DestinationItem &destination)
{
    // This check helps to find missed objects in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(GOType::Unknown) == 8, "Not all objects were handled.");

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wswitch-default")
    QT_WARNING_DISABLE_CLANG("-Wswitch-default")

    switch (type)
    {
        case GOType::Point:
            UpdatePoint(id, object, fPoint, sPoint, data, destination);
            break;
        case GOType::Arc:
            UpdateArc<VArc>(id, object, fPoint, sPoint, data, destination.id);
            break;
        case GOType::EllipticalArc:
            UpdateArc<VEllipticalArc>(id, object, fPoint, sPoint, data, destination.id);
            break;
        case GOType::Spline:
            UpdateCurve<VSpline>(id, object, fPoint, sPoint, data, destination.id);
            break;
        case GOType::SplinePath:
            UpdateCurveWithSegments<VSplinePath>(id, object, fPoint, sPoint, data, destination.id);
            break;
        case GOType::CubicBezier:
            UpdateCurve<VCubicBezier>(id, object, fPoint, sPoint, data, destination.id);
            break;
        case GOType::CubicBezierPath:
            UpdateCurveWithSegments<VCubicBezierPath>(id, object, fPoint, sPoint, data, destination.id);
            break;
        case GOType::Unknown:
        case GOType::PlaceLabel:
            Q_UNREACHABLE();
            break;
    }

    QT_WARNING_POP
}

//---------------------------------------------------------------------------------------------------------------------
void UpdateDestinationObjects(VAbstractOperationInitData &initData, const QPointF &fPoint, const QPointF &sPoint)
{
    for (int i = 0; i < initData.source.size(); ++i)
    {
        const SourceItem object = initData.source.at(i);
        const QSharedPointer<VGObject> obj = initData.data->GetGObject(object.id);
        UpdateDestinationObject(initData.id,
                                object,
                                obj->getType(),
                                fPoint,
                                sPoint,
                                initData.data,
                                initData.destination.at(i));
    }
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VAbstractFlipping::VAbstractFlipping(const VAbstractOperationInitData &initData, QGraphicsItem *parent)
  : VAbstractOperation(initData, parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void VAbstractFlipping::CreateDestination(VAbstractOperationInitData &initData, const QPointF &fPoint,
                                          const QPointF &sPoint)
{
    if (initData.typeCreation == Source::FromGui)
    {
        CreateDestinationObjects(initData, fPoint, sPoint);
    }
    else
    {
        UpdateDestinationObjects(initData, fPoint, sPoint);
    }
}
