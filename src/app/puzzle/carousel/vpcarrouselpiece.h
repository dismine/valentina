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

#include <QListWidgetItem>
#include <QMouseEvent>

#include "../layout/layoutdef.h"

class QColor;
class QLabel;

/// spec 21.3: a collapsible header before the pieces of one group, with the group's tint as a badge
class VPCarrouselGroupHeader : public QListWidgetItem
{
public:
    enum
    {
        Type = UserType + 2
    };

    VPCarrouselGroupHeader(const QString &group, const QColor &color, bool collapsed, QListWidget *parent);
    ~VPCarrouselGroupHeader() override = default;

    auto Group() const -> QString;
    void SetCollapsed(bool collapsed);
    auto operator<(const QListWidgetItem &other) const -> bool override;

private:
    Q_DISABLE_COPY_MOVE(VPCarrouselGroupHeader) // NOLINT
    QString m_group;
    QLabel *m_name{nullptr};
};

class VPCarrouselPiece : public QListWidgetItem
{
public:
    enum
    {
        Type = UserType + 1
    };

    explicit VPCarrouselPiece(const VPPiecePtr &piece, QListWidget *parent);
    ~VPCarrouselPiece() override = default;

    /**
     * @brief GetPiece Returns the corresponding layout piece
     * @return  the corresponding layout piece
     */
    auto GetPiece() const -> VPPiecePtr;

    // spec 21.3: the list is sorted by group (natural order), then by name
    auto operator<(const QListWidgetItem &other) const -> bool override;

    /**
     * @brief RefreshSelection refreshes the selection of the piece according to the selection information of m_piece
     */
    void RefreshSelection();

    void RefreshPieceIcon();

    /**
     * @brief CreatePieceIcon Creates an icon of the piece of given size
     * @param size of the icon
     * @return the created icon
     */
    auto CreatePieceIcon(const QSize &size, bool isDragIcon = false) const -> QIcon;

private:
    // cppcheck-suppress unknownMacro
    Q_DISABLE_COPY_MOVE(VPCarrouselPiece) // NOLINT
    VPPieceWeakPtr m_piece;
};

#endif // VPCARROUSELPIECE_H
