/************************************************************************
 **
 **  @file   vlabelarrangeengine.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 5, 2026
 **
 **  @brief  Pure-geometry label placement engine.
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

#ifndef VLABELARRANGEENGINE_H
#define VLABELARRANGEENGINE_H

#include <QVector>

#include "labelarrangetypes.h"

class QRectF;
class QSizeF;

/**
 * @brief Greedy candidate-based label placement engine.
 *
 * Operates purely on geometry (QRectF / QPointF / QSizeF). Has no dependency on
 * Qt scene or undo-command infrastructure. Safe to call from a worker thread.
 *
 * Algorithm (3 passes):
 *   For each label (largest-first order), try 144 candidate offsets
 *   (24 angles × 6 radial distances, starting just beyond the anchor zone)
 *   plus the current offset.  Candidates are hard-partitioned into two
 *   groups: those that clear the self-anchor (preferred) and those that
 *   cover it (last resort).  Among preferred candidates the one minimising
 *   total overlap area plus heavy penalties for covering any anchor point
 *   is chosen.  Labels already satisfying all hard constraints are skipped
 *   entirely.  Repeat for kPasses iterations.
 *
 *   Proximity lookups use two internal spatial hash grids (O(n·k) instead of
 *   O(n²)), making the algorithm practical for patterns with 1000+ labels.
 */
class VLabelArrangeEngine
{
public:
    VLabelArrangeEngine() = delete;

    /**
     * @brief Arrange Compute new (mx, my) offsets for all labels.
     * @param labels  Input geometry; order is preserved in the output.
     * @return New pixel offsets in the same order as \p labels.
     */
    static auto Arrange(const QVector<LabelArrangeData> &labels) -> QVector<QPointF>;

private:
    static auto GenerateCandidates(const QSizeF &size, qreal anchorRadius) -> QVector<QPointF>;
};

#endif // VLABELARRANGEENGINE_H
