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
#include "../layout/vpsheet.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pCarrouselPieceList, "p.carrouselPieceList")

//---------------------------------------------------------------------------------------------------------------------
VPCarrouselPieceList::VPCarrouselPieceList(QWidget* parent) :
    QListWidget(parent)
{
    setStyleSheet("QListWidget::item{border: 2px solid transparent; color: black;}  "
                  "QListWidget::item:selected {border: 2px solid rgb(255,160,160);}");
    setContextMenuPolicy(Qt::DefaultContextMenu);
    setSelectionMode(QAbstractItemView::MultiSelection);
    setViewMode(QListView::IconMode);
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

    if(not m_pieceList.isEmpty())
    {
        // create the corresponding carrousel pieces
        for (auto *piece : m_pieceList)
        {
            // update the label of the piece
             auto* carrouselpiece = new VPCarrouselPiece(piece, this);
             carrouselpiece->setSelected(piece->IsSelected());
        }
        sortItems();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::SetCurrentPieceList(const QList<VPPiece *> &pieceList)
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
        for(auto *item: selectedItems())
        {
            item->setSelected(false);
        }
    }

    QListWidget::mousePressEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::mouseMoveEvent(QMouseEvent *event)
{
    if (((event->buttons() & Qt::LeftButton) != 0U) &&
        ((event->pos() - m_dragStart).manhattanLength() >= QApplication::startDragDistance()) &&
        (selectedItems().count() > 0) &&
        (not m_pieceList.isEmpty() && m_pieceList.first()->Sheet() == nullptr)) // only if it's from unplaced pieces
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
    if(_item->type() == VPCarrouselPiece::Type)
    {
        auto *pieceItem = static_cast<VPCarrouselPiece *> (_item);

        // starts the dragging
        auto *drag = new QDrag(this);
        auto *mimeData = new VPMimeDataPiece();
        VPPiece* piece = pieceItem->GetPiece();
        mimeData->SetPiecePtr(piece);

        QPixmap pixmap = pieceItem->CreatePieceIcon(QSize(120, 120), true).pixmap(QSize(120, 120));

        drag->setDragCursor(VPMimeDataPiece::DragCursor(pixmap), Qt::MoveAction);
        drag->setMimeData(mimeData);
        if(drag->exec() == Qt::MoveAction)
        {
            m_carrousel->Refresh();
            piece->SetSelected(true);
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
    if(_item != nullptr && _item->type() == VPCarrouselPiece::Type)
    {
        auto *pieceItem = static_cast<VPCarrouselPiece *> (_item);

        QMenu menu;

        QAction *moveAction = menu.addAction(tr("Move to Sheet"));
        moveAction->setVisible(false);

        QAction *deleteAction = menu.addAction(tr("Delete"));
        deleteAction->setVisible(false);

        QAction *removeAction = menu.addAction(tr("Remove from Sheet"));
        removeAction->setVisible(false);

        if(not m_pieceList.isEmpty() && m_pieceList.first()->Sheet() == nullptr)
        {
            moveAction->setVisible(true);
            deleteAction->setVisible(true);
        }

        if(not m_pieceList.isEmpty() && m_pieceList.first()->Sheet() != nullptr)
        {
            removeAction->setVisible(true);
        }

        QAction *selectedAction = menu.exec(event->globalPos());

        VPPiece *piece = pieceItem->GetPiece();
        VPLayout *layout = piece->Layout();

        if (selectedAction == moveAction)
        {
            VPSheet *sheet = layout->GetFocusedSheet();
            piece->SetSheet(sheet);
            emit layout->PieceSheetChanged(piece);
        }
        else if (selectedAction == deleteAction)
        {
            VPSheet *sheet = layout->GetTrashSheet();
            piece->SetSheet(sheet);
            emit layout->PieceSheetChanged(piece);
        }
        else if (selectedAction == removeAction)
        {
            piece->SetSheet(nullptr);
            emit layout->PieceSheetChanged(piece);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::on_SelectionChangedExternal()
{
    blockSignals(true);
    for(int i = 0; i < count(); ++i)
    {
        QListWidgetItem* _item = item(i);
        if(_item->type() == VPCarrouselPiece::Type)
        {
            auto *itemPiece = static_cast<VPCarrouselPiece *> (_item);
            itemPiece->RefreshSelection();
        }
    }
    blockSignals(false);
}
