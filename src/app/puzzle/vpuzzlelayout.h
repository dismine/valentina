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
#include "vpuzzlelayer.h"
#include "def.h"

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
    Unit getUnit();

    void SetLayoutSize(qreal width, qreal height);
    void SetLayoutSize(QSizeF size);
    QSizeF GetLayoutSize();

    void SetLayoutMargins(qreal left, qreal top, qreal right, qreal bottom);
    void SetLayoutMargins(QMarginsF margins);
    QMarginsF GetLayoutMargins();

    void SetFollowGrainline(FollowGrainline state);
    FollowGrainline SetFollowGrainline();

    void SetPiecesGap(qreal value);
    qreal GetPiecesGap();

    void SetWarningSuperpositionOfPieces(bool state);
    bool GetWarningSuperpositionOfPieces();

    void SetWarningPiecesOutOfBound(bool state);
    bool GetWarningPiecesOutOfBound();

    void SetStickyEdges(bool state);
    bool GetStickyEdges();

private:
    Q_DISABLE_COPY(VPuzzleLayout)
    VPuzzleLayer *m_unplacedPiecesLayer;
    QList<VPuzzleLayer *> m_layers;

    // format
    Unit m_unit;
    QSizeF m_size;

    // margins
    QMarginsF m_margins;

    // control
    FollowGrainline m_followGrainLine;
    qreal m_piecesGap;
    bool m_warningSuperpositionOfPieces;
    bool m_warningPiecesOutOfBound;
    bool m_stickyEdges;

};

#endif // VPUZZLELAYOUT_H
