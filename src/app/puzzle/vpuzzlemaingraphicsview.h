/************************************************************************
 **
 **  @file   vpuzzlemaingraphicsview.h
 **  @author Ronan Le Tiec
 **  @date   3 5, 2020
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

#ifndef VPUZZLEMAINGRAPHICSVIEW_H
#define VPUZZLEMAINGRAPHICSVIEW_H

#include "vpuzzlegraphicslayout.h"
#include "vpuzzlegraphicspiece.h"
#include "../vwidgets/vmaingraphicsview.h"

class VMainGraphicsScene;


class VPuzzleMainGraphicsView : public VMainGraphicsView
{
    Q_OBJECT
public:
    VPuzzleMainGraphicsView(VPuzzleLayout *layout, QWidget *parent);
    ~VPuzzleMainGraphicsView() = default;

    /**
     * @brief RefreshLayout Refreshes the rectangles for the layout border and the margin
     */
    void RefreshLayout();

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

private slots:
    /**
     * @brief on_PieceMovedToLayer The slot is called when the given piece was moved from the given layer to the other
     * given layer
     * @param piece the piece that was moved
     * @param layerBefore the layer before the move
     * @param layerAfter the layer after the move
     */
    void on_PieceMovedToLayer(VPuzzlePiece *piece, VPuzzleLayer *layerBefore, VPuzzleLayer *layerAfter);

    /**
     * @brief on_SceneSelectionChanged Slot is called when the scene selection has changed
     */
    void on_SceneSelectionChanged();

private:
    Q_DISABLE_COPY(VPuzzleMainGraphicsView)

    VMainGraphicsScene *m_scene{nullptr};

    VPuzzleGraphicsLayout *m_graphicsLayout{nullptr};
    VPuzzleLayout *m_layout{nullptr};

    QList<VPuzzleGraphicsPiece*> m_graphicsPieces{};

};

#endif // VPUZZLEMAINGRAPHICVIEW_H
