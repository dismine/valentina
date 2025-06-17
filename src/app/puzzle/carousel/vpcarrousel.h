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

#include <QComboBox>
#include <QScrollArea>
#include <QUuid>
#include <QWidget>
#include <memory>

#include "../layout/layoutdef.h"

namespace Ui
{
class VPCarrousel;
}

struct VPCarrouselSheet
{
    bool unplaced{true};
    bool active{false};
    QString name{};
    QList<VPPiecePtr> pieces{};
    QUuid sheetUuid{};
};

class VPCarrousel : public QWidget
{
    Q_OBJECT // NOLINT

public:
    explicit VPCarrousel(const VPLayoutPtr &layout, QWidget *parent = nullptr);
    ~VPCarrousel() override;

    /**
     * @brief SetOrientation Sets the orientation to the given value and refreshes
     * the orientation of the carrousel.
     * @param orientation the orientation to set the carrousel to.
     */
    void SetOrientation(Qt::Orientation orientation);

    void RefreshPieceMiniature();

    /**
     * @brief RefreshOrientation Refreshes the orientation of the carrousel with the
     * m_orientation value;
     */
    void RefreshOrientation();

    void RefreshSheetNames();

    /**
     * @brief Clear Clears the carrousel (removes everything)
     */
    void Clear();
    auto Layout() const -> VPLayoutWeakPtr;

public slots:
    /**
     * @brief Refresh Refreshes the content of the carrousel
     */
    void Refresh();
    void on_ActiveSheetChanged(const VPSheetPtr &sheet);

protected:
    void changeEvent(QEvent *event) override;

private slots:

    /**
     * @brief on_ActivePieceListChanged Called when the active piece list is changed
     * @param index piece index
     */
    void on_ActivePieceListChanged(int index);

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VPCarrousel) // NOLINT
    std::unique_ptr<Ui::VPCarrousel> ui;

    VPLayoutWeakPtr m_layout{};

    QList<VPCarrouselSheet> m_pieceLists{};

    Qt::Orientation m_orientation{Qt::Vertical};

    bool m_ignoreActiveSheetChange{false};

    static auto GetSheetName(const VPCarrouselSheet &sheet) -> QString;
};

#endif // VPCARROUSEL_H
