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

#include <QApplication>
#include <QDrag>
#include <QDragMoveEvent>
#include <QLoggingCategory>
#include <QMenu>
#include <QPainter>

#include "../layout/vplayout.h"
#include "../layout/vppiece.h"
#include "../layout/vpsheet.h"
#include "../undocommands/vpundomovepieceonsheet.h"
#include "../vmisc/theme/vtheme.h"
#include "vpcarrousel.h"
#include "vpcarrouselpiece.h"
#include "vpmimedatapiece.h"

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(pCarrouselPieceList, "p.carrouselPieceList") // NOLINT

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
VPCarrouselPieceList::VPCarrouselPieceList(QWidget *parent)
  : QListWidget(parent)
{
    InitStyleSheet();
    setContextMenuPolicy(Qt::DefaultContextMenu);
    setSelectionMode(QAbstractItemView::MultiSelection);
    setViewMode(QListView::IconMode);

    // Because we cannot control icon color with stylesheet we must wait until scene style update. It happens after
    // the palette change signal.
    connect(VTheme::Instance(), &VTheme::ThemeSettingsChanged, this,
            [this]()
            {
                for (int i = 0; i < count(); ++i)
                {
                    if (auto *pieceItem = dynamic_cast<VPCarrouselPiece *>(item(i)))
                    {
                        pieceItem->RefreshPieceIcon();
                    }
                }

                InitStyleSheet();
            });
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

    if (m_pieceList.isEmpty())
    {
        return;
    }

    // create the corresponding carrousel pieces
    for (const auto &piece : std::as_const(m_pieceList)) // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)
    {
        if (not piece.isNull())
        {
            // update the label of the piece
            auto *carrouselpiece = new VPCarrouselPiece(piece, this);
            carrouselpiece->setSelected(piece->IsSelected());
        }
    }
    sortItems();
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::SetCurrentPieceList(const QList<VPPiecePtr> &pieceList)
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
        const QList<QListWidgetItem *> items = selectedItems();
        for (auto *item : items)
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
        (!selectedItems().isEmpty()) &&
        (!m_pieceList.isEmpty() && m_pieceList.constFirst()->Sheet() == nullptr)) // only if it's from unplaced pieces
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

    QListWidgetItem *_item = currentItem();
    if (_item->type() == VPCarrouselPiece::Type)
    {
        auto *pieceItem = dynamic_cast<VPCarrouselPiece *>(_item);
        SCASSERT(pieceItem != nullptr)

        if (m_carrousel == nullptr)
        {
            return;
        }

        VPLayoutPtr const layout = m_carrousel->Layout().toStrongRef();
        if (layout.isNull())
        {
            return;
        }

        // starts the dragging
        auto *drag = new QDrag(this);
        auto *mimeData = new VPMimeDataPiece(layout->Uuid());
        VPPiecePtr const piece = pieceItem->GetPiece();
        mimeData->SetPiecePtr(piece);

        QIcon const pieceIcon = pieceItem->CreatePieceIcon(QSize(120, 120), true);
        QPixmap pixmap;
        if (!pieceIcon.isNull())
        {
            pixmap = pieceIcon.pixmap(QSize(120, 120));
        }

        drag->setDragCursor(VPMimeDataPiece::DragCursor(pixmap), Qt::MoveAction);
        drag->setMimeData(mimeData);
        if (drag->exec() == Qt::MoveAction)
        {
            m_carrousel->Refresh();
            piece->SetSelected(false);

            VPLayoutPtr const pieceLayout = piece->Layout();
            if (not pieceLayout.isNull())
            {
                emit pieceLayout->PieceSelectionChanged(piece);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::dragMoveEvent(QDragMoveEvent *e)
{
    qCDebug(pCarrouselPieceList, "drag move");
    e->acceptProposedAction();
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::contextMenuEvent(QContextMenuEvent *event)
{
    QListWidgetItem *_item = currentItem();
    if (_item == nullptr || _item->type() != VPCarrouselPiece::Type)
    {
        return;
    }

    auto *pieceItem = dynamic_cast<VPCarrouselPiece *>(_item);
    SCASSERT(pieceItem != nullptr)

    VPPiecePtr const piece = pieceItem->GetPiece();
    VPLayoutPtr const layout = piece->Layout();

    if (piece.isNull() || layout.isNull())
    {
        return;
    }

    QMenu menu;

    QVector<QAction *> moveToActions;

    if (not piece->Sheet().isNull())
    {
        QList<VPSheetPtr> sheets = layout->GetSheets();
        sheets.removeAll(piece->Sheet());

        if (not sheets.isEmpty())
        {
            QMenu *moveMenu = menu.addMenu(tr("Move to"));

            for (const auto &sheet : sheets)
            {
                if (not sheet.isNull())
                {
                    QAction *moveToSheet = moveMenu->addAction(sheet->GetName());
                    moveToSheet->setData(QVariant::fromValue(sheet));
                    moveToActions.append(moveToSheet);
                }
            }
        }
    }

    QAction *moveAction = menu.addAction(tr("Move to Sheet"));
    moveAction->setVisible(false);

    QAction *deleteAction = menu.addAction(tr("Delete"));
    deleteAction->setVisible(false);

    QAction *removeAction = menu.addAction(tr("Remove from Sheet"));
    removeAction->setVisible(false);

    if (not m_pieceList.isEmpty() && m_pieceList.constFirst()->Sheet() == nullptr)
    {
        moveAction->setVisible(true);
        deleteAction->setVisible(true);
    }

    if (not m_pieceList.isEmpty() && m_pieceList.constFirst()->Sheet() != nullptr)
    {
        removeAction->setVisible(true);
    }

    QAction *selectedAction = menu.exec(event->globalPos());

    if (selectedAction == moveAction)
    {
        VPSheetPtr const sheet = layout->GetFocusedSheet();
        if (not sheet.isNull())
        {
            piece->ClearTransformations();
            QRectF const rect = sheet->GetMarginsRect();
            piece->SetPosition(QPointF(rect.topLeft().x() + 1, rect.topLeft().y() + 1));
            piece->SetZValue(1.0);
            auto *command = new VPUndoMovePieceOnSheet(layout->GetFocusedSheet(), piece);
            layout->UndoStack()->push(command);
        }
    }
    else if (selectedAction == deleteAction)
    {
        auto *command = new VPUndoMovePieceOnSheet(layout->GetTrashSheet(), piece);
        layout->UndoStack()->push(command);
    }
    else if (selectedAction == removeAction)
    {
        auto *command = new VPUndoMovePieceOnSheet(VPSheetPtr(), piece);
        layout->UndoStack()->push(command);
    }
    else if (moveToActions.contains(selectedAction))
    {
        auto *command = new VPUndoMovePieceOnSheet(qvariant_cast<VPSheetPtr>(selectedAction->data()), piece);
        layout->UndoStack()->push(command);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::InitStyleSheet()
{
    if (VTheme::ColorSheme() == VColorSheme::Dark)
    {
        setStyleSheet("QListWidget::item{border: 2px solid transparent;}"
                      "QListWidget::item:selected {border: 2px solid rgb(255,160,160);}");
    }
    else
    {
        setStyleSheet("QListWidget::item{border: 2px solid transparent; color: black;}  "
                      "QListWidget::item:selected {border: 2px solid rgb(255,160,160);}");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPieceList::on_SelectionChangedExternal()
{
    const QSignalBlocker blocker(this);
    for (int i = 0; i < count(); ++i)
    {
        QListWidgetItem *_item = item(i);
        if (_item->type() == VPCarrouselPiece::Type)
        {
            auto *itemPiece = dynamic_cast<VPCarrouselPiece *>(_item);
            SCASSERT(itemPiece != nullptr)
            itemPiece->RefreshSelection();
        }
    }
}
