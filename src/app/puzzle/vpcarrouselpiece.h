/************************************************************************
 **
 **  @file   vpcarrouselpiece.h
 **  @author Ronan Le Tiec
 **  @date   25 4, 2020
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
#ifndef VPCARROUSELPIECE_H
#define VPCARROUSELPIECE_H

#include <QFrame>
#include <QLabel>
#include <QGraphicsView>
#include <QMouseEvent>

#include "vpuzzlepiece.h"
#include "vpcarrouselpiecepreview.h"


class VPCarrouselPieceList;

class VPCarrouselPiece : public QFrame
{
    Q_OBJECT
public:
    explicit VPCarrouselPiece(VPuzzlePiece *piece, VPCarrouselPieceList *carrouselLayer);
    ~VPCarrouselPiece();

    void Init();
    void Refresh();
    /**
    * @brief CleanPiecesPreview fitInView of the qGraphicsView of the pieces works properly
    * only when the piece is in place in the layer and we call it from the layer.
    */
    void CleanPreview();

    /**
     * @brief GetPiece Returns the corresponding layout piece
     * @return  the corresponding layout piece
     */
    VPuzzlePiece * GetPiece();

public slots:
    void on_PieceSelectionChanged();

protected:
    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
    /**
     * @brief on_ActionPieceMovedToLayer Slot called when the piece is moved via the
     * context menu to anoter layer
     */
    void on_ActionPieceMovedToLayer();

private:
    Q_DISABLE_COPY(VPCarrouselPiece)

    VPuzzlePiece *m_piece;

    VPCarrouselPieceList *m_carrouselPieceList;

    QLabel *m_label{nullptr};
    VPCarrouselPiecePreview *m_piecePreview{nullptr};

    QPoint m_dragStart;

private slots:

};

#endif // VPCARROUSELPIECE_H
