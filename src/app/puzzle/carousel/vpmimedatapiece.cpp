/************************************************************************
 **
 **  @file   vpmimedatapiece.cpp
 **  @author Ronan Le Tiec
 **  @date   4 5, 2020
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

#include "vpmimedatapiece.h"

#include <QPainter>
#include <QPixmap>

#include "../layout/vppiece.h"

const QString VPMimeDataPiece::mineFormatPiecePtr =
    QStringLiteral("application/vnd.puzzle.piece.ptr"); // NOLINT(cert-err58-cpp)

//---------------------------------------------------------------------------------------------------------------------
VPMimeDataPiece::VPMimeDataPiece(const QUuid &layoutUuid)
  : m_layoutUuid(layoutUuid)
{
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMimeDataPiece::formats() const -> QStringList
{
    return {mineFormatPiecePtr};
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMimeDataPiece::GetPiecePtr() const -> VPPiecePtr
{
    return m_piece;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMimeDataPiece::SetPiecePtr(const VPPiecePtr &piece)
{
    m_piece = piece;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMimeDataPiece::DragCursor(const QPixmap &piecePixmap) -> QPixmap
{
    QPixmap dragCursor(piecePixmap.width() * 2, piecePixmap.height() * 2);
    dragCursor.fill(Qt::transparent);
    QPainter painter(&dragCursor);
    painter.drawPixmap(dragCursor.width() / 2, dragCursor.height() / 2, piecePixmap);
    QPixmap cursor(QStringLiteral("://puzzlecursor/collect.png"));
    painter.drawPixmap(dragCursor.width() / 2 - cursor.width() / 2, dragCursor.height() / 2 - cursor.height() / 2,
                       cursor);
    painter.end();
    return dragCursor;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMimeDataPiece::LayoutUuid() const -> const QUuid &
{
    return m_layoutUuid;
}
