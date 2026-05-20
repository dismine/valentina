/************************************************************************
 **
 **  @file   vdrawlabelcollector.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 5, 2026
 **
 **  @brief  Collects all stampable label items from the draw scene.
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

#ifndef VDRAWLABELCOLLECTOR_H
#define VDRAWLABELCOLLECTOR_H

#include <utility>
#include <QVector>

#include "labelarrangetypes.h"

class QGraphicsScene;

/**
 * @brief Walks a QGraphicsScene and extracts geometry + move-request data for
 *        every label item that a tool has stamped with LabelDataRole keys.
 *
 * The two output vectors are parallel: element i in the arrange-data vector
 * corresponds to element i in the request vector.
 *
 * Only items whose parent has a valid LabelKindRole entry are collected;
 * labels without a stamp (e.g., detail-piece text) are silently skipped.
 */
class VDrawLabelCollector
{
public:
    VDrawLabelCollector() = delete;

    static auto Collect(const QGraphicsScene *scene) -> std::pair<QVector<LabelArrangeData>, QVector<LabelMoveRequest>>;
};

#endif // VDRAWLABELCOLLECTOR_H
