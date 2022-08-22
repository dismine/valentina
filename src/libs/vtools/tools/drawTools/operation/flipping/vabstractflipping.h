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

#ifndef VABSTRACTFLIPPING_H
#define VABSTRACTFLIPPING_H

#include <QtGlobal>

#include "../vabstractoperation.h"

class VAbstractFlipping : public VAbstractOperation
{
    Q_OBJECT // NOLINT
public:
    virtual ~VAbstractFlipping() = default;
protected:
    explicit VAbstractFlipping(const VAbstractOperationInitData &initData, QGraphicsItem *parent = nullptr);

    static void CreateDestination(VAbstractOperationInitData &initData, const QPointF &fPoint, const QPointF &sPoint);

    static DestinationItem CreatePoint(quint32 idTool, const SourceItem &sItem, const QPointF &firstPoint,
                                       const QPointF &secondPoint, const QString &suffix, VContainer *data);

    template <class Item>
    static DestinationItem CreateItem(quint32 idTool, const SourceItem &sItem, const QPointF &firstPoint,
                                      const QPointF &secondPoint, const QString &suffix, VContainer *data);
    template <class Item>
    static DestinationItem CreateArc(quint32 idTool, const SourceItem &sItem, const QPointF &firstPoint,
                                     const QPointF &secondPoint, const QString &suffix, VContainer *data);
    template <class Item>
    static DestinationItem CreateCurve(quint32 idTool, const SourceItem &sItem, const QPointF &firstPoint,
                                       const QPointF &secondPoint, const QString &suffix, VContainer *data);
    template <class Item>
    static DestinationItem CreateCurveWithSegments(quint32 idTool, const SourceItem &sItem, const QPointF &firstPoint,
                                                   const QPointF &secondPoint, const QString &suffix, VContainer *data);

    static void UpdatePoint(quint32 idTool, const SourceItem &sItem, const QPointF &firstPoint,
                            const QPointF &secondPoint, const QString &suffix, VContainer *data,
                            const DestinationItem &item);
    template <class Item>
    static void UpdateItem(quint32 idTool, const SourceItem &sItem, const QPointF &firstPoint,
                           const QPointF &secondPoint, const QString &suffix, VContainer *data, quint32 id);
    template <class Item>
    static void UpdateArc(quint32 idTool, const SourceItem &sItem, const QPointF &firstPoint,
                          const QPointF &secondPoint, const QString &suffix, VContainer *data, quint32 id);
    template <class Item>
    static void UpdateCurve(quint32 idTool, const SourceItem &sItem, const QPointF &firstPoint,
                            const QPointF &secondPoint, const QString &suffix, VContainer *data, quint32 id);
    template <class Item>
    static void UpdateCurveWithSegments(quint32 idTool, const SourceItem &sItem, const QPointF &firstPoint,
                                        const QPointF &secondPoint, const QString &suffix, VContainer *data,
                                        quint32 id);
private:
    Q_DISABLE_COPY_MOVE(VAbstractFlipping) // NOLINT
};

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
DestinationItem VAbstractFlipping::CreateItem(quint32 idTool, const SourceItem &sItem, const QPointF &firstPoint,
                                                const QPointF &secondPoint, const QString &suffix, VContainer *data)
{
    const QSharedPointer<Item> i = data->GeometricObject<Item>(sItem.id);
    Item rotated = i->Flip(QLineF(firstPoint, secondPoint), suffix);
    rotated.setIdObject(idTool);

    if (not sItem.alias.isEmpty())
    {
        rotated.SetAliasSuffix(sItem.alias);
    }

    if (sItem.penStyle != TypeLineDefault)
    {
        rotated.SetPenStyle(sItem.penStyle);
    }

    if (sItem.color != ColorDefault)
    {
        rotated.SetColor(sItem.color);
    }

    DestinationItem item;
    item.id = data->AddGObject(new Item(rotated));
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
DestinationItem VAbstractFlipping::CreateCurve(quint32 idTool, const SourceItem &sItem, const QPointF &firstPoint,
                                                 const QPointF &secondPoint, const QString &suffix, VContainer *data)
{
    const DestinationItem item = CreateItem<Item>(idTool, sItem, firstPoint, secondPoint, suffix, data);
    data->AddSpline(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
DestinationItem VAbstractFlipping::CreateCurveWithSegments(quint32 idTool, const SourceItem &sItem,
                                                           const QPointF &firstPoint, const QPointF &secondPoint,
                                                           const QString &suffix, VContainer *data)
{
    const DestinationItem item = CreateItem<Item>(idTool, sItem, firstPoint, secondPoint, suffix, data);
    data->AddCurveWithSegments(data->GeometricObject<Item>(item.id), item.id);
    return item;
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VAbstractFlipping::UpdateItem(quint32 idTool, const SourceItem &sItem, const QPointF &firstPoint,
                                   const QPointF &secondPoint, const QString &suffix, VContainer *data, quint32 id)
{
    const QSharedPointer<Item> i = data->GeometricObject<Item>(sItem.id);
    Item rotated = i->Flip(QLineF(firstPoint, secondPoint), suffix);
    rotated.setIdObject(idTool);

    if (not sItem.alias.isEmpty())
    {
        rotated.SetAliasSuffix(sItem.alias);
    }

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
template <class Item>
void VAbstractFlipping::UpdateCurve(quint32 idTool, const SourceItem &sItem, const QPointF &firstPoint,
                                    const QPointF &secondPoint, const QString &suffix, VContainer *data, quint32 id)
{
    UpdateItem<Item>(idTool, sItem, firstPoint, secondPoint, suffix, data, id);
    data->AddSpline(data->GeometricObject<Item>(id), id);
}

//---------------------------------------------------------------------------------------------------------------------
template <class Item>
void VAbstractFlipping::UpdateCurveWithSegments(quint32 idTool, const SourceItem &sItem, const QPointF &firstPoint,
                                                const QPointF &secondPoint, const QString &suffix, VContainer *data,
                                                quint32 id)
{
    UpdateItem<Item>(idTool, sItem, firstPoint, secondPoint, suffix, data, id);
    data->AddCurveWithSegments(data->GeometricObject<Item>(id), id);
}

#endif // VABSTRACTFLIPPING_H
