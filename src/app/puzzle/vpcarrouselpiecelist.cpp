/************************************************************************
 **
 **  @file   vpcarrouselpiecelist.cpp
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
#include "vpcarrouselpiecelist.h"

#include <QDrag>
#include <QDragMoveEvent>
#include <QPainter>
#include <QApplication>
#include <QMenu>

#include "vpcarrousel.h"
#include "vpcarrouselpiece.h"
#include "../vmisc/backport/qoverload.h"
#include "vpmimedatapiece.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pCarrouselPieceList, "p.carrouselPieceList")

//---------------------------------------------------------------------------------------------------------------------
VPCarrouselPieceList::VPCarrouselPieceList(QWidget* parent) :
    QListWidget(parent),
    m_dragStart(QPoint())
{
    setStyleSheet("QListWidget::item{border: 2px solid transparent; color: black;}  QListWidget::item:selected {border: 2px solid red;}");
    setContextMenuPolicy(Qt::DefaultContextMenu);
    setSelectionMode(QAbstractItemView::MultiSelection);
    setViewMode(QListView::IconMode);

    connect(this, &VPCarrouselPieceList::itemSelectionChanged, this, &VPCarrouselPieceList::on_SelectionChangedInternal);
}

//---------------------------------------------------------------------------------------------------------------------
VPCarrouselPieceList::~VPCarrouselPieceList()
{

}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::SetCarrousel(VPCarrousel *carrousel)
{
    m_carrousel = carrousel;
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::Refresh()
{
    clear();

    if(m_pieceList != nullptr)
    {
        m_pieceList->disconnect(this);

        // Updates the carrousel pieces from the pieces list
        QList<VPPiece*> pieces = m_pieceList->GetPieces();

        // create the corresponding carrousel pieces
        for (auto piece : pieces)
        {
            // update the label of the piece
             VPCarrouselPiece* carrouselpiece = new VPCarrouselPiece(piece,this);
             carrouselpiece->setSelected(piece->GetIsSelected());
             connect(piece, &VPPiece::SelectionChanged, this, &VPCarrouselPieceList::on_SelectionChangedExternal);
        }
        sortItems();

        connect(m_pieceList, &VPPieceList::PieceAdded, this, &VPCarrouselPieceList::on_PieceAdded);
        connect(m_pieceList, &VPPieceList::PieceRemoved, this, &VPCarrouselPieceList::on_PieceRemoved);
    }
}


//---------------------------------------------------------------------------------------------------------------------
VPPieceList* VPCarrouselPieceList::GetCurrentPieceList()
{
    return m_pieceList;
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::SetCurrentPieceList(VPPieceList* pieceList)
{
    m_pieceList = pieceList;

    Refresh();
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragStart = event->pos();
    }

    if (!(event->modifiers() & Qt::ControlModifier))
    {
        // clearSelection doesn't work properly here so we go through the elements.
        for(auto item: selectedItems())
        {
            item->setSelected(false);
        }
    }

    QListWidget::mousePressEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::mouseMoveEvent(QMouseEvent *event)
{
    if ((event->buttons() & Qt::LeftButton) &&
        ((event->pos() - m_dragStart).manhattanLength() >= QApplication::startDragDistance()) &&
        (selectedItems().count() > 0) &&
        (m_pieceList->GetSheet() == nullptr)) // only if it's from unplaced pieces
    {
        startDrag(Qt::MoveAction);
    }
    else
    {
        QListWidget::mouseMoveEvent(event);
    }
}


//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::startDrag(Qt::DropActions supportedActions)
{
    Q_UNUSED(supportedActions)

    QListWidgetItem* _item = currentItem();
    if(_item->type() == 1001)
    {
        VPCarrouselPiece *pieceItem = static_cast<VPCarrouselPiece *> (_item);

        // starts the dragging
        QDrag *drag = new QDrag(this);
        VPMimeDataPiece *mimeData = new VPMimeDataPiece();
        VPPiece* piece = pieceItem->GetPiece();
        mimeData->SetPiecePtr(piece);
        mimeData->setObjectName("piecePointer");

        QPixmap pixmap = pieceItem->CreatePieceIcon(QSize(120,120)).pixmap(QSize(120,120));

        drag->setPixmap(pixmap);
        drag->setMimeData(mimeData);
        if(drag->exec() == Qt::MoveAction)
        {
            delete takeItem(row(_item));
            clearSelection();
            piece->SetIsSelected(true);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::dragMoveEvent(QDragMoveEvent* e)
{
        qCDebug(pCarrouselPieceList, "drag move");
        e->acceptProposedAction();
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::contextMenuEvent(QContextMenuEvent *event)
{
    QListWidgetItem* _item = currentItem();
    if(_item != nullptr)
    {
        if(_item->type() == 1001)
        {
            VPCarrouselPiece *pieceItem = static_cast<VPCarrouselPiece *> (_item);

            QMenu contextMenu;

            if(m_pieceList->GetSheet() == nullptr)
            {
                VPPieceList* sheetPieces = pieceItem->GetPiece()->GetPieceList()->GetLayout()->GetFocusedSheet()->GetPieceList();
                QAction *moveAction = contextMenu.addAction(tr("Move to Sheet"));
                QVariant moveData = QVariant::fromValue(sheetPieces);
                moveAction->setData(moveData);

                VPPieceList* trashPieceList = pieceItem->GetPiece()->GetPieceList()->GetLayout()->GetTrashPieceList();
                QAction *deleteAction = contextMenu.addAction(tr("Delete"));
                QVariant deleteData = QVariant::fromValue(trashPieceList);
                deleteAction->setData(deleteData);

                connect(moveAction, &QAction::triggered, this, &VPCarrouselPieceList::on_ActionPieceMovedToPieceList);
                connect(deleteAction, &QAction::triggered, this, &VPCarrouselPieceList::on_ActionPieceMovedToPieceList);
            }

            // remove from piece list action
            if(m_pieceList->GetSheet() != nullptr)
            {
                VPPieceList* unplacedPieces = pieceItem->GetPiece()->GetPieceList()->GetLayout()->GetUnplacedPieceList();
                QAction *removeAction = contextMenu.addAction(tr("Remove from Sheet"));
                QVariant data = QVariant::fromValue(unplacedPieces);
                removeAction->setData(data);
                connect(removeAction, &QAction::triggered, this, &VPCarrouselPieceList::on_ActionPieceMovedToPieceList);
            }

            contextMenu.exec(event->globalPos());
        }
    }
}


//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::on_ActionPieceMovedToPieceList()
{
    QListWidgetItem* _item = currentItem();
    if(_item->type() == 1001)
    {
        VPCarrouselPiece *pieceItem = static_cast<VPCarrouselPiece *> (_item);
        QAction *act = qobject_cast<QAction *>(sender());
        QVariant v = act->data();
        VPPieceList *pieceList = v.value<VPPieceList *>();
        if(pieceList != nullptr)
        {
            pieceList->GetLayout()->MovePieceToPieceList(pieceItem->GetPiece(), pieceList);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::on_PieceAdded(VPPiece* piece)
{
    if(piece->GetPieceList() == m_pieceList)
    {
        // update the label of the piece
         VPCarrouselPiece* carrouselpiece = new VPCarrouselPiece(piece,this);
         carrouselpiece->setSelected(piece->GetIsSelected());
         connect(piece, &VPPiece::SelectionChanged, this, &VPCarrouselPieceList::on_SelectionChangedExternal);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::on_PieceRemoved(VPPiece* piece)
{
    for(int i = 0; i < count(); ++i)
    {
        QListWidgetItem* _item = item(i);
        if(_item->type() == 1001)
        {
            VPCarrouselPiece *itemPiece = static_cast<VPCarrouselPiece *> (_item);

            if(piece == itemPiece->GetPiece())
            {
                delete takeItem(row(_item));

                return;
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::on_SelectionChangedInternal()
{
    blockSignals(true);

    for(int i = 0; i < count(); ++i)
    {
        QListWidgetItem* _item = item(i);
        if(_item->type() == 1001)
        {
            VPCarrouselPiece *itemPiece = static_cast<VPCarrouselPiece *> (_item);
            itemPiece->GetPiece()->SetIsSelected(itemPiece->isSelected());
        }
    }
    m_carrousel->ClearSelectionExceptForCurrentPieceList();
    // TODO FIXME:  when selecting pieces on the sheet, and then selecting a unplaced piece in the piece carrousel
    // the selection is cleared in the sheet (good !) but the cliked item in unplaced pieces in not selected (bad!)

    blockSignals(false);
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::on_SelectionChangedExternal()
{
    blockSignals(true);
    for(int i = 0; i < count(); ++i)
    {
        QListWidgetItem* _item = item(i);
        if(_item->type() == 1001)
        {
            VPCarrouselPiece *itemPiece = static_cast<VPCarrouselPiece *> (_item);
            itemPiece->RefreshSelection();
        }
    }
    blockSignals(false);
}
