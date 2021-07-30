/************************************************************************
 **
 **  @file   vpmaingraphicsview.h
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

#ifndef VPMAINGRAPHICSVIEW_H
#define VPMAINGRAPHICSVIEW_H

#include "vpgraphicssheet.h"
#include "vpgraphicspiece.h"
#include "vptilefactory.h"
#include "vpgraphicstilegrid.h"
#include "../vwidgets/vmaingraphicsview.h"

class VMainGraphicsScene;

class VPTileFactory;

class VPMainGraphicsView : public VMainGraphicsView
{
    Q_OBJECT
public:
    VPMainGraphicsView(VPLayout *layout, VPTileFactory *tileFactory, QWidget *parent);
    ~VPMainGraphicsView() = default;

    /**
     * @brief RefreshLayout Refreshes the rectangles for the layout border and the margin
     */
    void RefreshLayout();


    /**
     * @brief GetScene Returns the scene of the view
     * @return scene of the view
     */
    VMainGraphicsScene* GetScene();

    /**
     * @brief PrepareForExport prepares the graphic for an export (i.e hide margin etc)
     */
    void PrepareForExport();

    /**
     * @brief CleanAfterExport cleans the graphic for an export (i.e show margin etc)
     */
    void CleanAfterExport();

public slots:
    /**
     * @brief on_PieceSheetChanged The slot is called when the given piece was moved from the given piece list to
     * the other given piece list
     * @param piece the piece that was moved
     */
    void on_PieceSheetChanged(VPPiece *piece);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;

    void drawTilesLine();

private:
    Q_DISABLE_COPY(VPMainGraphicsView)

    VMainGraphicsScene *m_scene;

    VPGraphicsSheet *m_graphicsSheet{nullptr};

    VPGraphicsTileGrid *m_graphicsTileGrid{nullptr};

    VPLayout *m_layout;

    QList<VPGraphicsPiece*> m_graphicsPieces{};

    /**
     * variable to hold temporarly hte value of the show tiles
     */
    bool m_showTilesTmp{false};

    /**
     * variable to hold temporarly hte value of the show grid
     */
    bool m_showGridTmp{false};

};

#endif // VPMAINGRAPHICSVIEW_H
