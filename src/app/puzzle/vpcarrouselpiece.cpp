/************************************************************************
 **
 **  @file   vpcarrouselpiece.cpp
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

#include "vpcarrouselpiece.h"

#include <QApplication>
#include <QMenu>

#include "vpmimedatapiece.h"
#include "vpcarrouselpiecelist.h"
#include "vpcarrousel.h"
#include "vpsheet.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pCarrouselPiece, "p.carrouselPiece")


//---------------------------------------------------------------------------------------------------------------------
VPCarrouselPiece::VPCarrouselPiece(VPPiece *piece, QListWidget* parent) :
    QListWidgetItem(parent,1001),
    m_piece(piece)
{
    int width = 120 - 8;
    QFontMetrics metrix = QFontMetrics(QFont());
    QString clippedText = metrix.elidedText(piece->GetName(), Qt::ElideRight, width);
    setIcon(m_piece->PieceIcon(QSize(120, 120)));
    setText(clippedText);
}


//---------------------------------------------------------------------------------------------------------------------
VPCarrouselPiece::~VPCarrouselPiece()
{

}

//---------------------------------------------------------------------------------------------------------------------
VPPiece * VPCarrouselPiece::GetPiece()
{
    return m_piece;
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPiece::RefreshSelection()
{
    setSelected(m_piece->GetIsSelected());
}

////---------------------------------------------------------------------------------------------------------------------
//void VPCarrouselPiece::contextMenuEvent(QContextMenuEvent *event)
//{
//    QMenu contextMenu;

//    VPPieceList* unplacedPieces = m_piece->GetPieceList()->GetLayout()->GetUnplacedPieceList();
//    QList<VPSheet*> sheets = m_piece->GetPieceList()->GetLayout()->GetSheets();
//    QList<VPPieceList*> pieceLists = QList<VPPieceList*>();
//    for (auto sheet : sheets)
//    {
//        pieceLists.append(sheet->GetPieceList());
//    }

//    // move to piece list actions  -- TODO : To be tested properly when we have several piece lists
//    pieceLists.removeAll(m_piece->GetPieceList());
//    if(pieceLists.count() > 0)
//    {
//        QMenu *moveMenu = contextMenu.addMenu(tr("Move to"));

//        // TODO order in alphabetical order

//        for (auto pieceList : pieceLists)
//        {
//            QAction* moveToPieceList = moveMenu->addAction(pieceList->GetName());
//            QVariant data = QVariant::fromValue(pieceList);
//            moveToPieceList->setData(data);

//            connect(moveToPieceList, &QAction::triggered, this, &VPCarrouselPiece::on_ActionPieceMovedToPieceList);
//        }
//    }

//    // remove from piece list action
//    if(m_piece->GetPieceList() != unplacedPieces)
//    {
//        QAction *removeAction = contextMenu.addAction(tr("Remove from Sheet"));
//        QVariant data = QVariant::fromValue(m_piece->GetPieceList()->GetLayout()->GetUnplacedPieceList());
//        removeAction->setData(data);
//        connect(removeAction, &QAction::triggered, this, &VPCarrouselPiece::on_ActionPieceMovedToPieceList);
//    }

//    contextMenu.exec(event->globalPos());
//}

////---------------------------------------------------------------------------------------------------------------------
//void VPCarrouselPiece::on_ActionPieceMovedToPieceList()
//{
//    QAction *act = qobject_cast<QAction *>(sender());
//    QVariant v = act->data();
//    VPPieceList *pieceList = v.value<VPPieceList *>();
//    if(pieceList != nullptr)
//    {
//        pieceList->GetLayout()->MovePieceToPieceList(m_piece, pieceList);
//    }
//}
