/************************************************************************
 **
 **  @file   vpcarrousel.h
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

#ifndef VPCARROUSEL_H
#define VPCARROUSEL_H

#include <QWidget>
#include <QComboBox>
#include <QScrollArea>
#include "vplayout.h"
#include "vppiece.h"

namespace Ui
{
class VPCarrousel;
}

struct VPCarrouselSheet
{
    bool unplaced{true};
    QString name{};
    VPPieceList* pieces{nullptr};
};

class VPCarrousel : public QWidget
{
    Q_OBJECT
public:
    explicit VPCarrousel(VPLayout *layout, QWidget *parent = nullptr);
    virtual ~VPCarrousel() = default;

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
     * @brief Refresh Refreshes the content of the carrousel
     */
    void Refresh();

    void RefreshSheetNames();

    /**
     * @brief Clear Clears the carrousel (removes everything)
     */
    void Clear();

    /**
     * @brief ClearSelection Clears the selection of the carrousel.
     */
    void ClearSelection();

    /**
     * @brief ClearSelectionExceptForCurrentPieceList Clears the selection of all pieces of
     * the layout except for the one in the current piece list
     */
    void ClearSelectionExceptForCurrentPieceList();

protected:
    virtual void changeEvent(QEvent* event) override;

private slots:

    /**
     * @brief on_ActivePieceListChanged Called when the active piece list is changed
     * @param index piece index
     */
    void on_ActivePieceListChanged(int index);

private:
    Q_DISABLE_COPY(VPCarrousel)
    Ui::VPCarrousel *ui;

    VPLayout *m_layout{nullptr};

    QList<VPCarrouselSheet> m_pieceLists{};

    Qt::Orientation m_orientation{Qt::Vertical};

    static QString GetSheetName(const VPCarrouselSheet &sheet);
};

#endif // VPCARROUSEL_H
