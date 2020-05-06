/************************************************************************
 **
 **  @file   vpiececarrousel.h
 **  @author Ronan Le Tiec
 **  @date   13 04, 2020
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

#ifndef VPIECECARROUSEL_H
#define VPIECECARROUSEL_H

#include <QWidget>
#include <QComboBox>
#include <QScrollArea>
#include "vpuzzlelayout.h"
#include "vpuzzlepiece.h"
#include "vpiececarrousellayer.h"

class VPieceCarrousel : public QWidget
{
    Q_OBJECT
public:
    explicit VPieceCarrousel(VPuzzleLayout *layout, QWidget *parent = nullptr);
    virtual ~VPieceCarrousel();

    /**
     * @brief SetOrientation Sets the orientation to the given value and refreshes
     * the orientation of the carrousel.
     * @param orientation the orientation to set the carrousel to.
     */
    void SetOrientation(Qt::Orientation orientation);

    /**
     * @brief RefreshOrientation Refreshes the orientation of the carrousel with the
     * m_orientation value;
     */
    void RefreshOrientation();

    /**
     * @brief Inits the carroussel
     */
    void Init();

    /**
     * @brief Refresh Refreshes the content of the carrousel
     */
    void Refresh();

    /**
     * @brief Clear Clears the carrousel (removes everything)
     */
    void Clear();

    /**
     * @brief ClearSelection Clears the selection of the carrousel.
     */
    void ClearSelection();


private:
    Q_DISABLE_COPY(VPieceCarrousel)

    VPuzzleLayout *m_layout;

    QComboBox *m_comboBoxLayer;
    QScrollArea *m_scrollArea;
    QWidget *m_layersContainer;

    QList<VPieceCarrouselLayer*> m_carrouselLayers;

    Qt::Orientation m_orientation{Qt::Vertical};


private slots:
    void on_ActiveLayerChanged(int index);
};

#endif // VPIECECARROUSEL_H
