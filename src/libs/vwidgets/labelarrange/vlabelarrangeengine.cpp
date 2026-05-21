/************************************************************************
 **
 **  @file   vlabelarrangeengine.cpp
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

#include "vlabelarrangeengine.h"

#include <QHash>
#include <QRectF>
#include <QSizeF>
#include <QtMath>

#include <algorithm>
#include <limits>
#include <numeric>

#include "../vmisc/defglobal.h"

namespace
{
constexpr int kPasses = 3;
constexpr vsizetype kAngles = 24;
constexpr vsizetype kDistances = 6;

// Minimum clearance gap (px) kept between any two label bounding boxes
constexpr qreal kLabelMargin = 25.0;

// Multiplier for the anchor-coverage penalty relative to one label area
constexpr qreal kAnchorPenaltyFactor = 10.0;

auto AnchorZone(const LabelArrangeData &L) -> QRectF
{
    const qreal r = L.anchorRadius;
    return {L.anchor.x() - r, L.anchor.y() - r, 2.0 * r, 2.0 * r};
}

// Lightweight spatial hash grid for O(k) proximity lookups instead of O(n).
// Items are stored in cells of size m_cellSize. The Nearby() query returns all
// items whose stored position falls within an expanded version of the query rect.
struct SpatialGrid
{
    qreal m_cellSize{1.0};
    QHash<qint64, QVector<int>> m_cells{};

    explicit SpatialGrid(qreal cellSize) : m_cellSize(qMax(cellSize, 1.0)) {}

    static auto PackCell(int col, int row) -> qint64
    {
        return (static_cast<qint64>(col) << 32) | static_cast<quint32>(row);
    }

    auto CellOf(QPointF p) const -> qint64
    {
        return PackCell(static_cast<int>(qFloor(p.x() / m_cellSize)),
                        static_cast<int>(qFloor(p.y() / m_cellSize)));
    }

    void Clear() { m_cells.clear(); }

    void Insert(int idx, QPointF pos) { m_cells[CellOf(pos)].append(idx); }

    void Remove(int idx, QPointF pos)
    {
        const qint64 key = CellOf(pos);
        auto it = m_cells.find(key);
        if (it != m_cells.end())
        {
            it->removeOne(idx);
            if (it->isEmpty())
            {
                m_cells.erase(it);
            }
        }
    }

    // Returns indices of all items in cells that overlap queryRect expanded by 'expansion'
    // on every side. Use expansion = max item reach to guarantee no false negatives.
    auto Nearby(const QRectF &queryRect, qreal expansion) const -> QVector<int>
    {
        const auto c0 = static_cast<int>(qFloor((queryRect.left() - expansion) / m_cellSize));
        const auto r0 = static_cast<int>(qFloor((queryRect.top() - expansion) / m_cellSize));
        const auto c1 = static_cast<int>(qFloor((queryRect.right() + expansion) / m_cellSize));
        const auto r1 = static_cast<int>(qFloor((queryRect.bottom() + expansion) / m_cellSize));

        QVector<int> result;
        for (int c = c0; c <= c1; ++c)
        {
            for (int r = r0; r <= r1; ++r)
            {
                const auto it = m_cells.constFind(PackCell(c, r));
                if (it != m_cells.constEnd())
                {
                    result.append(*it);
                }
            }
        }
        return result;
    }
};

// Bundles the two spatial grids and their lookup radii so they can be passed as
// a single parameter rather than four separate ones.
struct GridContext
{
    const SpatialGrid &anchorGrid;
    qreal maxAnchorRadius;
    SpatialGrid &rectGrid;
    qreal maxLabelEdge;
};

//---------------------------------------------------------------------------------------------------------------------
// Returns true when the label rect has no real problems: does not cover its own
// anchor, does not cover any other anchor, and has no true overlap with any other
// label. The margin gap is intentionally NOT checked here — it is only a soft
// preference used during candidate selection.
auto IsWellPlaced(const QRectF &rect, int selfIdx,
                  const QVector<QPointF> &offsets,
                  const QVector<LabelArrangeData> &labels,
                  const GridContext &grid) -> bool
{
    if (rect.intersects(AnchorZone(labels.at(selfIdx))))
    {
        return false;
    }

    // Check other anchor zones via anchor grid
    const QVector<int> nearbyAnchors = grid.anchorGrid.Nearby(rect, grid.maxAnchorRadius);
    if (std::any_of(nearbyAnchors.cbegin(), nearbyAnchors.cend(),
                    [labels, selfIdx, rect](int j) -> bool
                    {
                        return j != selfIdx && rect.intersects(AnchorZone(labels.at(j)));
                    }))
    {
        return false;
    }

    // Check other label rects via rect grid
    const QVector<int> nearby = grid.rectGrid.Nearby(rect, grid.maxLabelEdge);
    return std::none_of(nearby.cbegin(),
                        nearby.cend(),
                        [labels, offsets, selfIdx, rect](int j) -> bool
                        {
                            if (j == selfIdx)
                            {
                                return false;
                            }
                            const QRectF other{labels.at(j).anchor + offsets.at(j), labels.at(j).labelSize};
                            return rect.intersects(other);
                        });
}

//---------------------------------------------------------------------------------------------------------------------
auto ScoreCandidate(const QRectF &candidateRect,
                    const QPointF &anchor,
                    int selfIdx,
                    const QVector<QPointF> &offsets,
                    const QVector<LabelArrangeData> &labels,
                    const GridContext &grid) -> qreal
{
    qreal score = 0.0;

    // Heavy penalty when the candidate covers the anchor-point indicator
    const qreal selfRadius = labels.at(selfIdx).anchorRadius;
    if (const QRectF anchorZone{anchor.x() - selfRadius, anchor.y() - selfRadius, 2.0 * selfRadius, 2.0 * selfRadius};
        candidateRect.intersects(anchorZone))
    {
        score += candidateRect.width() * candidateRect.height() * kAnchorPenaltyFactor;
    }

    // Expanded rect used to enforce a minimum clearance gap between labels
    const QRectF paddedCandidate = candidateRect.adjusted(-kLabelMargin, -kLabelMargin,
                                                           kLabelMargin, kLabelMargin);

    // Penalty for covering another label's anchor point (anchor grid lookup)
    for (int const j : grid.anchorGrid.Nearby(candidateRect, grid.maxAnchorRadius))
    {
        if (j == selfIdx)
        {
            continue;
        }
        const QPointF &otherAnchor = labels.at(j).anchor;
        const qreal otherRadius = labels.at(j).anchorRadius;
        const QRectF otherAnchorZone{otherAnchor.x() - otherRadius, otherAnchor.y() - otherRadius,
                                     2.0 * otherRadius, 2.0 * otherRadius};
        if (candidateRect.intersects(otherAnchorZone))
        {
            score += candidateRect.width() * candidateRect.height() * kAnchorPenaltyFactor;
        }
    }

    // Overlap area with padded rect (rect grid lookup)
    for (int const j : grid.rectGrid.Nearby(paddedCandidate, grid.maxLabelEdge))
    {
        if (j == selfIdx)
        {
            continue;
        }
        const QRectF other{labels.at(j).anchor + offsets.at(j), labels.at(j).labelSize};
        const QRectF inter = paddedCandidate.intersected(other);
        if (!inter.isEmpty())
        {
            score += inter.width() * inter.height();
        }
    }

    return score;
}

//---------------------------------------------------------------------------------------------------------------------
auto PickBestCandidate(const QVector<QPointF> &activeCandidates,
                       const LabelArrangeData &L,
                       int idx,
                       const QVector<QPointF> &offsets,
                       const QVector<LabelArrangeData> &labels,
                       const GridContext &grid) -> QPointF
{
    qreal bestScore = std::numeric_limits<qreal>::max();
    QPointF bestOffset = offsets.at(idx);

    for (const QPointF &cand : std::as_const(activeCandidates))
    {
        const QRectF rect{L.anchor + cand, L.labelSize};
        const qreal sc = ScoreCandidate(rect, L.anchor, idx, offsets, labels, grid);
        if (sc < bestScore)
        {
            bestScore = sc;
            bestOffset = cand;
        }
    }

    return bestOffset;
}

//---------------------------------------------------------------------------------------------------------------------
void PlaceLabel(int idx,
                const QVector<LabelArrangeData> &labels,
                QVector<QPointF> &offsets,
                const QVector<QPointF> &baseCandidates,
                const GridContext &grid)
{
    const LabelArrangeData &L = labels.at(idx);

    QVector<QPointF> candidates = baseCandidates;
    candidates.append(offsets.at(idx));

    const QRectF selfAnchorZone = AnchorZone(L);
    QVector<QPointF> validCandidates;
    QVector<QPointF> fallbackCandidates;
    validCandidates.reserve(candidates.size());
    for (const QPointF &cand : std::as_const(candidates))
    {
        const QRectF rect{L.anchor + cand, L.labelSize};
        (rect.intersects(selfAnchorZone) ? fallbackCandidates : validCandidates).append(cand);
    }
    const QVector<QPointF> &activeCandidates = validCandidates.isEmpty() ? fallbackCandidates : validCandidates;

    const QPointF bestOffset = PickBestCandidate(activeCandidates, L, idx, offsets, labels, grid);

    const QPointF oldTopLeft = L.anchor + offsets.at(idx);
    offsets[idx] = bestOffset;
    grid.rectGrid.Remove(idx, oldTopLeft);
    grid.rectGrid.Insert(idx, L.anchor + bestOffset);
}

//---------------------------------------------------------------------------------------------------------------------
void RunArrangePass(int n,
                    const QVector<LabelArrangeData> &labels,
                    const QVector<int> &order,
                    QVector<QPointF> &offsets,
                    const QVector<QVector<QPointF>> &allCandidates,
                    const GridContext &grid)
{
    grid.rectGrid.Clear();
    for (int i = 0; i < n; ++i)
    {
        grid.rectGrid.Insert(i, labels.at(i).anchor + offsets.at(i));
    }

    for (int const idx : std::as_const(order))
    {
        const LabelArrangeData &L = labels.at(idx);
        if (const QRectF currentRect{L.anchor + offsets.at(idx), L.labelSize};
            IsWellPlaced(currentRect, idx, offsets, labels, grid))
        {
            continue;
        }

        PlaceLabel(idx, labels, offsets, allCandidates.at(idx), grid);
    }
}

} // namespace

//---------------------------------------------------------------------------------------------------------------------
auto VLabelArrangeEngine::GenerateCandidates(const QSizeF &size, qreal anchorRadius) -> QVector<QPointF>
{
    const qreal halfW = size.width() / 2.0;
    const qreal halfH = size.height() / 2.0;
    const qreal halfDiag = qSqrt(size.width() * size.width() + size.height() * size.height()) / 2.0;

    // Base distance: label centre placed so the nearest label edge just clears the anchor zone.
    // Multiplied by (1 + 0.5*d) for subsequent distances.
    const qreal baseR = halfDiag + anchorRadius;

    QVector<QPointF> candidates;
    candidates.reserve(kAngles * kDistances);

    for (vsizetype a = 0; a < kAngles; ++a)
    {
        const qreal angle = 2.0 * M_PI * static_cast<qreal>(a) / static_cast<qreal>(kAngles);
        const qreal cosA = qCos(angle);
        const qreal sinA = qSin(angle);

        for (vsizetype d = 0; d < kDistances; ++d)
        {
            const qreal r = baseR * (1.0 + 0.5 * static_cast<qreal>(d));

            // Place label centre at (anchor + r*direction), topLeft = centre - halfSize
            candidates.append(QPointF{cosA * r - halfW, sinA * r - halfH});
        }
    }
    return candidates;
}

//---------------------------------------------------------------------------------------------------------------------
auto VLabelArrangeEngine::Arrange(const QVector<LabelArrangeData> &labels) -> QVector<QPointF>
{
    const auto n = static_cast<int>(labels.size());
    if (n == 0)
    {
        return {};
    }

    // Start from current positions
    QVector<QPointF> offsets(n);
    for (int i = 0; i < n; ++i)
    {
        offsets[i] = labels.at(i).currentOffset;
    }

    // Process largest labels first – they are hardest to place
    QVector<int> order(n);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(),
              order.end(),
              [&](int a, int b) -> bool
              {
                  const auto &sa = labels.at(a).labelSize;
                  const auto &sb = labels.at(b).labelSize;
                  return sa.width() * sa.height() > sb.width() * sb.height();
              });

    // Precompute grid parameters from the label set
    qreal maxAnchorRadius = 0.0;
    qreal maxLabelEdge = 0.0;
    for (const auto &L : labels)
    {
        maxAnchorRadius = qMax(maxAnchorRadius, L.anchorRadius);
        maxLabelEdge = qMax(maxLabelEdge, qMax(L.labelSize.width(), L.labelSize.height()));
    }
    // Cell size covers the max label edge + margin + anchor zone, ensuring any two interacting
    // items are always in the same cell or adjacent cells.
    const qreal cellSize = maxLabelEdge + 2.0 * kLabelMargin + maxAnchorRadius;

    // Static anchor grid: keyed by anchor position, never updated.
    SpatialGrid anchorGrid(cellSize);
    for (int i = 0; i < n; ++i)
    {
        anchorGrid.Insert(i, labels.at(i).anchor);
    }

    // Dynamic rect grid: keyed by current label rect top-left (anchor + offset).
    // Rebuilt at the start of each pass and updated after each label is moved.
    SpatialGrid rectGrid(cellSize);

    // Candidates are geometry-only and identical across passes; compute once here
    // where GenerateCandidates is accessible as a private member.
    QVector<QVector<QPointF>> allCandidates(n);
    for (int i = 0; i < n; ++i)
    {
        allCandidates[i] = GenerateCandidates(labels.at(i).labelSize, labels.at(i).anchorRadius);
    }

    GridContext grid{anchorGrid, maxAnchorRadius, rectGrid, maxLabelEdge};
    for (int pass = 0; pass < kPasses; ++pass)
    {
        RunArrangePass(n, labels, order, offsets, allCandidates, grid);
    }

    return offsets;
}
