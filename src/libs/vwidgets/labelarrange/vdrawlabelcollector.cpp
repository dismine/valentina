/************************************************************************
 **
 **  @file   vdrawlabelcollector.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 5, 2026
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

#include "vdrawlabelcollector.h"

#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QList>
#include <QSizeF>
#include <QVariant>

#include "../global.h"
#include "../vgraphicssimpletextitem.h"

//---------------------------------------------------------------------------------------------------------------------
auto VDrawLabelCollector::Collect(const QGraphicsScene *scene)
    -> std::pair<QVector<LabelArrangeData>, QVector<LabelMoveRequest>>
{
    QVector<LabelArrangeData> arrangeData;
    QVector<LabelMoveRequest> requests;

    if (scene == nullptr)
    {
        return {arrangeData, requests};
    }

    // The engine works in m_realPos units (scale-independent, same as stored in
    // XML and consumed by undo commands). When scale > 1, sceneBoundingRect()
    // returns rawSize/scale because the item has setScale(1/scale) applied.
    // Multiply back so labelSize is in the same coordinate space as currentOffset.
    qreal viewScale = 1.0;
    if (!scene->views().isEmpty())
    {
        viewScale = scene->views().constFirst()->transform().m11();
    }

    // All geometry passed to the engine must be in m_realPos units:
    //   m_realPos  = pos() * scale   (scale-independent, stored in XML)
    //   labelSize  = rawSize * scale (already normalised above)
    //   anchor     = scenePos * scale
    //   anchorRadius = ScaledRadius(1.0)  – the visual dot radius is constant at
    //                  ~DefPointRadiusPixel() view-pixels regardless of zoom;
    //                  ScaledRadius(viewScale) would give the scene-coordinate
    //                  radius (smaller when zoomed in) which is the wrong unit.
    const qreal anchorRadius = ScaledRadius(1.0);

    const QList<QGraphicsItem *> allItems = scene->items();
    arrangeData.reserve(allItems.size());
    requests.reserve(allItems.size());

    for (QGraphicsItem *item : allItems)
    {
        if (item->type() != VGraphicsSimpleTextItem::Type)
        {
            continue;
        }

        if (!item->isVisible())
        {
            continue;
        }

        QGraphicsItem const *parent = item->parentItem();
        if (parent == nullptr)
        {
            continue;
        }

        const QVariant kindVar = parent->data(LabelKindRole);
        if (!kindVar.isValid())
        {
            continue;
        }

        const auto *textItem = static_cast<VGraphicsSimpleTextItem *>(item);

        const QSizeF rawSize = textItem->sceneBoundingRect().size();
        if (rawSize.isEmpty())
        {
            continue;
        }
        const QSizeF labelSize = viewScale > 1.0 ? rawSize * viewScale : rawSize;

        const QPointF anchor = viewScale > 1.0 ? parent->scenePos() * viewScale : parent->scenePos();

        const LabelArrangeData data{.anchor = anchor,
                                    .labelSize = labelSize,
                                    .currentOffset = textItem->GetRealPos(),
                                    .anchorRadius = anchorRadius};

        const LabelMoveRequest req{.kind = static_cast<LabelMoveRequest::Kind>(kindVar.toInt()),
                                   .toolId = parent->data(LabelToolIdRole).toUInt(),
                                   .pointId = parent->data(LabelPointIdRole).toUInt(),
                                   .extraIdx = parent->data(LabelExtraIdxRole).toInt()};

        arrangeData.append(data);
        requests.append(req);
    }

    arrangeData.squeeze();
    requests.squeeze();

    return {arrangeData, requests};
}
