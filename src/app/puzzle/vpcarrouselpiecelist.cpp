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

#include "vpcarrousel.h"
#include "vpcarrouselpiece.h"
#include "../vmisc/backport/qoverload.h"
#include "vpmimedatapiece.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pCarrouselPieceList, "p.carrouselPieceList")

//---------------------------------------------------------------------------------------------------------------------
VPCarrouselPieceList::VPCarrouselPieceList(QWidget* parent) :
    QListWidget(parent)
{
//    Init();

    setStyleSheet("QListWidget::item{background-color:transparent; border: 2px solid transparent; color: black;}  QListWidget::item:selected {background-color:transparent; border: 2px solid red; color: black; selection-background-color: white;}");
    setContextMenuPolicy(Qt::CustomContextMenu);
    setSelectionMode(QAbstractItemView::MultiSelection);
    setViewMode(QListView::IconMode);

    connect(this, &VPCarrouselPieceList::itemSelectionChanged, this, &VPCarrouselPieceList::on_SelectionChangedInternal);
}

//---------------------------------------------------------------------------------------------------------------------
VPCarrouselPieceList::~VPCarrouselPieceList()
{

}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::Init()
{
//    // add the connections
//    connect(m_pieceList, &VPPieceList::PieceAdded, this, &VPCarrouselPieceList::on_PieceAdded);
//    connect(m_pieceList, &VPPieceList::PieceRemoved, this, &VPCarrouselPieceList::on_PieceRemoved);
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::Refresh()
{
    clear();

    if(m_pieceList != nullptr)
    {
        // Updates the carrousel pieces from the pieces list
        QList<VPPiece*> pieces = m_pieceList->GetPieces();

        // sort the pieces in alphabetical order
        std::sort(pieces.begin(), pieces.end(),
              [](const VPPiece* a, const VPPiece* b) -> bool { return a->GetName() < b->GetName();});

        // create the corresponding carrousel pieces
        for (auto piece : pieces)
        {
            // update the label of the piece
             VPCarrouselPiece* carrouselpiece = new VPCarrouselPiece(piece,this);
             carrouselpiece->setSelected(piece->GetIsSelected());
             connect(piece, &VPPiece::SelectionChanged, this, &VPCarrouselPieceList::on_SelectionChangedExternal);
        }
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
    qCDebug(pCarrouselPieceList, "mouse pressed");

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
        qCDebug(pCarrouselPieceList, "mouse moved");

    if ((event->buttons() & Qt::LeftButton) &&
        ((event->pos() - m_dragStart).manhattanLength() >= QApplication::startDragDistance()) &&
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

    qCDebug(pCarrouselPieceList, "start drag");

    QListWidgetItem* item = currentItem();
    if(item->type() == 1001)
    {
        VPCarrouselPiece *pieceItem = static_cast<VPCarrouselPiece *> (item);

        // starts the dragging
        QDrag *drag = new QDrag(this);
        VPMimeDataPiece *mimeData = new VPMimeDataPiece();
        mimeData->SetPiecePtr(pieceItem->GetPiece()); //TODO
        mimeData->setObjectName("piecePointer");

        QPixmap pixmap = pieceItem->GetPiece()->PieceIcon(QSize(120,120)).pixmap(QSize(120,120));

        drag->setPixmap(pixmap);
        drag->setMimeData(mimeData);
        if(drag->exec() == Qt::MoveAction)
        {
            delete takeItem(row(item));
            clearSelection();
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
void VPCarrouselPieceList::on_PieceAdded(VPPiece* piece)
{
    Q_UNUSED(piece)

    // TODO/ FIXME: see if we find a solution more efficient refreshing the complete layout everytime.

    Refresh();
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::on_PieceRemoved(VPPiece* piece)
{
    // TODO
    Q_UNUSED(piece)
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::on_SelectionChangedInternal()
{
    for(int i = 0; i < count(); ++i)
    {
        QListWidgetItem* _item = item(i);
        if(_item->type() == 1001)
        {
            VPCarrouselPiece *itemPiece = static_cast<VPCarrouselPiece *> (_item);
            blockSignals(true);
            itemPiece->GetPiece()->SetIsSelected(itemPiece->isSelected());
            blockSignals(false);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::on_SelectionChangedExternal()
{
    for(int i = 0; i < count(); ++i)
    {
        QListWidgetItem* _item = item(i);
        if(_item->type() == 1001)
        {
            VPCarrouselPiece *itemPiece = static_cast<VPCarrouselPiece *> (_item);
            blockSignals(true);
            itemPiece->RefreshSelection();
            blockSignals(false);
        }
    }
}
