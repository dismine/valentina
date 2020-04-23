/************************************************************************
 **
 **  @file   vpuzzlelayout.h
 **  @author Ronan Le Tiec
 **  @date   13 4, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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
#ifndef VPUZZLELAYOUT_H
#define VPUZZLELAYOUT_H

#include <QSizeF>
#include <QMarginsF>
#include <QList>

#include "def.h"

class VPuzzleLayer;

// is this the right place for the definition?
enum class FollowGrainline : qint8 { No = 0, Follow90 = 1, Follow180 = 2};

class VPuzzleLayout
{

public:
    VPuzzleLayout();
    virtual ~VPuzzleLayout();

    VPuzzleLayer* GetUnplacedPiecesLayer();

    VPuzzleLayer* AddLayer();
    VPuzzleLayer* AddLayer(VPuzzleLayer *layer);
    QList<VPuzzleLayer *> GetLayers();

    void SetUnit(Unit unit);
    Unit getUnit() const;

    /**
     * @brief SetLayoutSize sets the size of the layout, the values have to be in Unit::Px
     * @param width
     * @param height
     */
    void SetLayoutSize(qreal width, qreal height);

    /**
     * @brief SetLayoutSize sets the size of the layout, the values have to be in the layout's unit
     * @param width
     * @param height
     */
    void SetLayoutSizeConverted(qreal width, qreal height);

    /**
     * @brief SetLayoutSize sets the size of the layout, the values have to be in Unit::Px
     * @param size
     */
    void SetLayoutSize(const QSizeF &size);
    /**
     * @brief SetLayoutSizeConverted sets the size of the layout, the values have to be in the layout's unit
     * @param size
     */
    void SetLayoutSizeConverted(const QSizeF &size);

    /**
     * @brief GetLayoutSize Returns the size in Unit::Px
     * @return
     */
    QSizeF GetLayoutSize() const;

    /**
     * @brief GetLayoutSizeConverted Returns the size in the layout's unit
     * @return
     */
    QSizeF GetLayoutSizeConverted() const;

    /**
     * @brief SetLayoutMargins, set the margins of the layout, the values have to be in Unit::Px
     * @param left in Unit::Px
     * @param top in Unit::Px
     * @param right in Unit::Px
     * @param bottom in Unit::Px
     */
    void SetLayoutMargins(qreal left, qreal top, qreal right, qreal bottom);

    /**
     * @brief SetLayoutMargins, set the margins of the layout, the values have to be in the unit of the layout
     * @param left in Unit::Px
     * @param top in Unit::Px
     * @param right in Unit::Px
     * @param bottom in Unit::Px
     */
    void SetLayoutMarginsConverted(qreal left, qreal top, qreal right, qreal bottom);

    /**
     * @brief SetLayoutMargins set the margins of the layout, the values have to be in Unit::Px
     * @param margins
     */
    void SetLayoutMargins(const QMarginsF &margins);

    /**
     * @brief SetLayoutMargins set the margins of the layout, the values have to be in the unit of the layout
     * @param margins
     */
    void SetLayoutMarginsConverted(const QMarginsF &margins);

    /**
     * @brief GetLayoutMargins Returns the size in Unit::Px
     * @return
     */
    QMarginsF GetLayoutMargins() const;

    /**
     * @brief GetLayoutMarginsConverted Returns the margins in the layout's unit
     * @return
     */
    QMarginsF GetLayoutMarginsConverted() const;

    void SetFollowGrainline(FollowGrainline state);
    FollowGrainline SetFollowGrainline();

    /**
     * @brief SetPiecesGap sets the pieces gap to the given value, the unit has to be in Unit::Px
     * @param value
     */
    void SetPiecesGap(qreal value);

    /**
     * @brief SetPiecesGapConverted sets the pieces gap to the given value, the unit has to be in the layout's unit
     * @param value
     */
    void SetPiecesGapConverted(qreal value);

    /**
     * @brief GetPiecesGap returns the pieces gap in Unit::Px
     * @return
     */
    qreal GetPiecesGap() const;

    /**
     * @brief GetPiecesGapConverted returns the pieces gap in the layout's unit
     * @return
     */
    qreal GetPiecesGapConverted() const;

    void SetWarningSuperpositionOfPieces(bool state);
    bool GetWarningSuperpositionOfPieces() const;

    void SetWarningPiecesOutOfBound(bool state);
    bool GetWarningPiecesOutOfBound() const;

    void SetStickyEdges(bool state);
    bool GetStickyEdges() const;

private:
    Q_DISABLE_COPY(VPuzzleLayout)
    VPuzzleLayer *m_unplacedPiecesLayer;
    QList<VPuzzleLayer *> m_layers{};

    // format
    Unit m_unit{Unit::Cm};
    /**
     * @brief m_size the Size in Unit::Px
     */
    QSizeF m_size{};

    // margins
    /**
     * @brief m_margins the margins in Unit::Px
     */
    QMarginsF m_margins{};

    // control
    FollowGrainline m_followGrainLine{FollowGrainline::No};

    /**
     * @brief m_piecesGap the pieces gap in Unit::Px
     */
    qreal m_piecesGap{0};
    bool m_warningSuperpositionOfPieces{false};
    bool m_warningPiecesOutOfBound{false};
    bool m_stickyEdges{false};

};

#endif // VPUZZLELAYOUT_H
