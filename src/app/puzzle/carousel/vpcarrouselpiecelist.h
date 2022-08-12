/************************************************************************
 **
 **  @file   vpcarrouselpiecelist.h
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

#ifndef VPCARROUSELPIECELIST_H
#define VPCARROUSELPIECELIST_H

#include <QListWidget>

#include "vpcarrousel.h"


class VPCarrouselPieceList : public QListWidget
{
    Q_OBJECT // NOLINT
public:
    explicit VPCarrouselPieceList(QWidget* parent);
    ~VPCarrouselPieceList() override = default;

    /**
     * @brief Refresh refreshes the items of the carrousel piece list
     */
    void Refresh();

    /**
     * @brief SetCurrentPieceList Sets the current piece list to the given piece list and redraw
     * the carrousel.
     */
    void SetCurrentPieceList(const QList<VPPiecePtr> &pieceList);

    /**
     * @brief SetCarrousel Sets the carrousel corresponding to the list
     * @param carrousel pointer to the carrousel
     */
    void SetCarrousel(VPCarrousel *carrousel);

public slots:
    /**
     * @brief on_SelectionChangedExternal when the selection was changed outside of the carrousel
     */
    void on_SelectionChangedExternal();

protected:
    void startDrag(Qt::DropActions supportedActions) override;
    void dragMoveEvent(QDragMoveEvent* e) override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    Q_DISABLE_COPY_MOVE(VPCarrouselPieceList) // NOLINT

    QList<VPPiecePtr> m_pieceList{};
    QPoint m_dragStart{};
    VPCarrousel *m_carrousel{nullptr};
};

#endif // VPCARROUSELPIECELIST_H
