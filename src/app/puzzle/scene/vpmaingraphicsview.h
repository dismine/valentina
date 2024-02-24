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

#include "../vwidgets/vmaingraphicsview.h"
#include "../layout/layoutdef.h"

class VMainGraphicsScene;
class VPGraphicsPieceControls;
class VPGraphicsTransformationOrigin;
class VPTileFactory;
class VPGraphicsPiece;
class VPLayout;
class VPGraphicsTileGrid;
class VPGraphicsSheet;
class VPPiece;

class VPMainGraphicsView : public VMainGraphicsView
{
    Q_OBJECT // NOLINT
public:
    VPMainGraphicsView(const VPLayoutPtr &layout, QWidget *parent);
    ~VPMainGraphicsView() override = default;

    /**
     * @brief RefreshLayout Refreshes the rectangles for the layout border and the margin
     */
    void RefreshLayout() const;

public slots:
    /**
     * @brief on_PieceSheetChanged The slot is called when the given piece was moved from the given piece list to
     * the other given piece list
     * @param piece the piece that was moved
     */
    void on_PieceSheetChanged(const VPPiecePtr &piece);

    void on_ActiveSheetChanged(const VPSheetPtr &focusedSheet);

    void RefreshPieces() const;

signals:
    /**
     * @brief mouseMove send new mouse position.
     * @param scenePos new mouse position.
     */
    void mouseMove(const QPointF &scenePos);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

    void contextMenuEvent(QContextMenuEvent *event) override;

    void drawTilesLine();

private slots:
    void RestoreOrigin() const;
    void on_SceneMouseMove(const QPointF &scenePos);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VPMainGraphicsView) // NOLINT

    VPLayoutWeakPtr m_layout;

    bool m_allowChangeMerge{false};

    qreal m_rotationSum{0};

    bool m_hasStickyPosition{false};
    qreal m_stickyTranslateX{0};
    qreal m_stickyTranslateY{0};

    void RotatePiecesByAngle(qreal angle);
    void TranslatePiecesOn(qreal dx, qreal dy);

    void SwitchScene(const VPSheetPtr &sheet);
    void ClearSelection() const;

    void ZValueMove(int move) const;

    void RemovePiece() const;
    void MovePiece(QKeyEvent *event);
};

#endif // VPMAINGRAPHICSVIEW_H
