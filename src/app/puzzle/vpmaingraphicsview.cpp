/************************************************************************
 **
 **  @file   vpmaingraphicsview.cpp
 **  @author Ronan Le Tiec
 **  @date   3 5, 2020
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

#include "vpmaingraphicsview.h"

#include <QDragEnterEvent>
#include <QMimeData>
#include <QKeyEvent>
#include <QMenu>

#include "vpmimedatapiece.h"
#include "vplayout.h"
#include "vpsheet.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vptilefactory.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pMainGraphicsView, "p.mainGraphicsView")


//---------------------------------------------------------------------------------------------------------------------
VPMainGraphicsView::VPMainGraphicsView(VPLayout *layout, VPTileFactory *tileFactory, QWidget *parent) :
    VMainGraphicsView(parent),
    m_scene(new VMainGraphicsScene(this)),
    m_layout(layout)
{
    SCASSERT(m_layout != nullptr)
    setScene(m_scene);

    m_graphicsSheet = new VPGraphicsSheet(layout->GetFocusedSheet());
    m_graphicsSheet->setPos(0, 0);
    m_scene->addItem(m_graphicsSheet);

    setAcceptDrops(true);

    m_graphicsTileGrid = new VPGraphicsTileGrid(layout, tileFactory);
    m_scene->addItem(m_graphicsTileGrid);

    // add the connections
    connect(m_layout, &VPLayout::PieceSheetChanged, this, &VPMainGraphicsView::on_PieceSheetChanged);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::RefreshLayout()
{
    // FIXME: Is that the way to go?

    m_graphicsSheet->update();

    m_graphicsTileGrid->update();

    m_scene->update();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::RefreshPieces()
{
    qDeleteAll(m_graphicsPieces);
    m_graphicsPieces.clear();

    VPSheet *sheet = m_layout->GetFocusedSheet();
    if (sheet != nullptr)
    {
        QList<VPPiece *> pieces = sheet->GetPieces();
        m_graphicsPieces.reserve(pieces.size());

        for (auto *piece : pieces)
        {
            auto *graphicsPiece = new VPGraphicsPiece(piece);
            m_graphicsPieces.append(graphicsPiece);

            scene()->addItem(graphicsPiece);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
VMainGraphicsScene* VPMainGraphicsView::GetScene()
{
    return m_scene;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::PrepareForExport()
{
    m_graphicsSheet->SetShowBorder(false);
    m_graphicsSheet->SetShowMargin(false);

    m_showGridTmp = m_layout->GetFocusedSheet()->GetLayout()->LayoutSettings().GetShowGrid();
    m_layout->GetFocusedSheet()->GetLayout()->LayoutSettings().SetShowGrid(false);

    m_showTilesTmp = m_layout->LayoutSettings().GetShowTiles();
    m_layout->LayoutSettings().SetShowTiles(false);

    RefreshLayout();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::CleanAfterExport()
{
    m_graphicsSheet->SetShowBorder(true);
    m_graphicsSheet->SetShowMargin(true);

    m_layout->GetFocusedSheet()->GetLayout()->LayoutSettings().SetShowGrid(m_showGridTmp);

    m_layout->LayoutSettings().SetShowTiles(m_showTilesTmp);

    RefreshLayout();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *mime = event->mimeData();

    if(mime->hasFormat(VPMimeDataPiece::mineFormatPiecePtr))
    {
        qCDebug(pMainGraphicsView(), "drag enter");
        event->acceptProposedAction();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    const QMimeData *mime = event->mimeData();

    if(mime->hasFormat(VPMimeDataPiece::mineFormatPiecePtr))
    {
        event->acceptProposedAction();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::dropEvent(QDropEvent *event)
{
    const QMimeData *mime = event->mimeData();

    qCDebug(pMainGraphicsView(), "drop enter , %s", qUtf8Printable(mime->objectName()));

    if(mime->hasFormat(VPMimeDataPiece::mineFormatPiecePtr))
    {
        const auto *mimePiece = qobject_cast<const VPMimeDataPiece *> (mime);

        VPPiece *piece = mimePiece->GetPiecePtr();
        if(piece != nullptr)
        {
            qCDebug(pMainGraphicsView(), "element dropped, %s", qUtf8Printable(piece->GetName()));
            event->acceptProposedAction();

            QPoint point = event->pos();
            piece->SetPosition(mapToScene(point));
            piece->SetRotation(0);

            // change the piecelist of the piece
            piece->SetSheet(m_layout->GetFocusedSheet());

            auto *graphicsPiece = new VPGraphicsPiece(piece);
            m_graphicsPieces.append(graphicsPiece);

            scene()->addItem(graphicsPiece);

            event->acceptProposedAction();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete)
    {
        QList<VPGraphicsPiece*> tmpGraphicsPieces = m_graphicsPieces;

        for(auto *graphicsPiece : tmpGraphicsPieces)
        {
            VPPiece *piece = graphicsPiece->GetPiece();

            if(piece->GetIsSelected())
            {
                piece->SetIsSelected(false);
                piece->SetSheet(nullptr);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu;

    VPSheet *sheet = m_layout->GetFocusedSheet();
    QAction *removeSheetAction = menu.addAction(QIcon::fromTheme(QStringLiteral("edit-delete")), tr("Remove sheet"));
    removeSheetAction->setEnabled(sheet != nullptr && m_layout->GetSheets().size() > 1);

    QAction *selectedAction = menu.exec(event->globalPos());
    if (selectedAction == removeSheetAction)
    {
        if (sheet != nullptr)
        {
            sheet->SetVisible(false);

            QList<VPPiece *> pieces = sheet->GetPieces();
            for (auto *piece : pieces)
            {
                piece->SetSheet(nullptr);
            }
        }

        m_layout->SetFocusedSheet(nullptr);
        emit on_SheetRemoved();
        RefreshPieces();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::on_PieceSheetChanged(VPPiece *piece)
{
    VPGraphicsPiece *_graphicsPiece = nullptr;
    for(auto *graphicPiece : m_graphicsPieces)
    {
        if(graphicPiece->GetPiece() == piece)
        {
            _graphicsPiece = graphicPiece;
        }
    }

    if (piece->Sheet() == nullptr || piece->Sheet() == m_layout->GetTrashSheet()) // remove
    {
        if (_graphicsPiece != nullptr)
        {
            scene()->removeItem(_graphicsPiece);
            m_graphicsPieces.removeAll(_graphicsPiece);
        }
    }
    else // add
    {
        if(_graphicsPiece == nullptr)
        {
            _graphicsPiece = new VPGraphicsPiece(piece);
            m_graphicsPieces.append(_graphicsPiece);
        }
        scene()->addItem(_graphicsPiece);
    }
}
