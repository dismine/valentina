/************************************************************************
 **
 **  @file   zvaluemovebackgroundimage.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 1, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Valentina project
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
#include "zvaluemovebackgroundimage.h"
#include "../ifc/xml/vbackgroundpatternimage.h"

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto CorrectedZValues(const QList<QVector<QUuid>> &order) -> QHash<QUuid, qreal>
{
    QHash<QUuid, qreal> correctedZValues;

    for (int i = 0; i < order.size(); ++i)
    {
        const QVector<QUuid> &level = order.at(i);
        for (const auto &imgId : level)
        {
            correctedZValues.insert(imgId, i);
        }
    }

    return correctedZValues;
}
}  // namespace

//---------------------------------------------------------------------------------------------------------------------
ZValueMoveBackgroundImage::ZValueMoveBackgroundImage(QUuid id,
                                                     ZValueMoveType move,
                                                     VAbstractPattern *doc,
                                                     QUndoCommand *parent)
  : VUndoCommand(doc, parent),
    m_id(id),
    m_move(move)
{
    setText(tr("z value move a background image"));

    QVector<VBackgroundPatternImage> const images = doc->GetBackgroundImages();

    for (const auto &image: images)
    {
        m_oldValues.insert(image.Id(), image.ZValue());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void ZValueMoveBackgroundImage::undo()
{
    QVector<VBackgroundPatternImage> images = Doc()->GetBackgroundImages();

    for (auto &image: images)
    {
        image.SetZValue(m_oldValues.value(image.Id(), image.ZValue()));
    }

    Doc()->SaveBackgroundImages(images);
    emit Doc()->BackgroundImagesZValueChanged();
}

//---------------------------------------------------------------------------------------------------------------------
void ZValueMoveBackgroundImage::redo()
{
    QVector<VBackgroundPatternImage> images = Doc()->GetBackgroundImages();

    auto Levels = [this](const QVector<VBackgroundPatternImage> &images, bool skip)
    {
        QMap<qreal, QVector<QUuid>> levels;

        for (const auto &image: images)
        {
            if (skip && image.Id() == m_id)
            {
                continue;
            }

            if (levels.contains(image.ZValue()))
            {
                QVector<QUuid> level_images = levels.value(image.ZValue());
                level_images.append(image.Id());
                levels[image.ZValue()] = level_images;
            }
            else
            {
                levels[image.ZValue()] = {image.Id()};
            }
        }

        return levels.values();
    };

    QList<QVector<QUuid>> order;

    if (m_move == ZValueMoveType::Top)
    {
        order = Levels(images, true);
        order.prepend({m_id});
    }
    else if (m_move == ZValueMoveType::Up)
    {
        for (auto &image: images)
        {
            if (image.Id() != m_id)
            {
                image.SetZValue(image.ZValue() + 1);
            }
        }

        order = Levels(images, false);
    }
    else if (m_move == ZValueMoveType::Down)
    {
        for (auto &image: images)
        {
            if (image.Id() != m_id)
            {
                image.SetZValue(image.ZValue() - 1);
            }
        }

        order = Levels(images, false);
    }
    else if (m_move == ZValueMoveType::Bottom)
    {
        order = Levels(images, true);
        order.append({m_id});
    }

    QHash<QUuid, qreal> const correctedZValues = CorrectedZValues(order);
    for (auto &image: images)
    {
        image.SetZValue(correctedZValues.value(image.Id(), image.ZValue()));
    }

    Doc()->SaveBackgroundImages(images);
    emit Doc()->BackgroundImagesZValueChanged();
}
