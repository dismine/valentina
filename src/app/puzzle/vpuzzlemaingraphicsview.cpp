/************************************************************************
 **
 **  @file   vpuzzlemaingraphicsview.cpp
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

#include "vpuzzlemaingraphicsview.h"

#include <QDragEnterEvent>
#include <QMimeData>
#include <QKeyEvent>

#include "vpuzzlemimedatapiece.h"
#include "vppiecelist.h"
#include "../vwidgets/vmaingraphicsscene.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pMainGraphicsView, "p.mainGraphicsView")


//---------------------------------------------------------------------------------------------------------------------
VPuzzleMainGraphicsView::VPuzzleMainGraphicsView(VPuzzleLayout *layout, QWidget *parent) :
    VMainGraphicsView(parent),
    m_layout(layout)
{
    m_scene = new VMainGraphicsScene(this);
    setScene(m_scene);

    m_graphicsSheet = new VPGraphicsSheet(layout);
    m_graphicsSheet->setPos(0, 0);
    m_scene->addItem(m_graphicsSheet);

    setAcceptDrops(true);

    // add the connections
    connect(m_layout, &VPuzzleLayout::PieceMovedToPieceList, this, &VPuzzleMainGraphicsView::on_PieceMovedToPieceList);
    connect(m_scene, &VMainGraphicsScene::selectionChanged, this,
            &VPuzzleMainGraphicsView::on_SceneSelectionChanged);
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleMainGraphicsView::RefreshLayout()
{
    // FIXME: Is that the way to go?

    m_graphicsSheet->update();

    m_scene->update();
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleMainGraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *mime = event->mimeData();

    if(mime->objectName() == "piecePointer")
    {
        qCDebug(pMainGraphicsView(), "drag enter");
        event->acceptProposedAction();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleMainGraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    const QMimeData *mime = event->mimeData();

    if(mime->objectName() == "piecePointer")
    {
        event->acceptProposedAction();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleMainGraphicsView::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}



//---------------------------------------------------------------------------------------------------------------------
void VPuzzleMainGraphicsView::dropEvent(QDropEvent *event)
{
    const QMimeData *mime = event->mimeData();

    qCDebug(pMainGraphicsView(), "drop enter , %s", qUtf8Printable(mime->objectName()));

    if(mime->objectName() == "piecePointer")
    {
        const VPuzzleMimeDataPiece *mimePiece = qobject_cast<const VPuzzleMimeDataPiece *> (mime);

        VPuzzlePiece *piece = mimePiece->GetPiecePtr();
        if(piece != nullptr)
        {
            qCDebug(pMainGraphicsView(), "element dropped, %s", qUtf8Printable(piece->GetName()));
            event->acceptProposedAction();

            QPoint point = event->pos();
            piece->SetPosition(mapToScene(point));

            // change the piecelist of the piece
            VPPieceList *focusedPieceList = m_layout->GetFocusedPieceList();
            if(focusedPieceList != nullptr)
            {
                m_layout->MovePieceToPieceList(piece, focusedPieceList);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleMainGraphicsView::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete)
    {
        QList<VPGraphicsPiece*> tmpGraphicsPieces = m_graphicsPieces;

        for(auto graphicsPiece : tmpGraphicsPieces)
        {
            VPuzzlePiece *piece = graphicsPiece->GetPiece();

            if(piece->GetIsSelected())
            {
                piece->SetIsSelected(false);
                m_layout->MovePieceToPieceList(piece, m_layout->GetUnplacedPieceList());
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleMainGraphicsView::on_PieceMovedToPieceList(VPuzzlePiece *piece, VPPieceList *pieceListBefore, VPPieceList *pieceListAfter)
{
    Q_UNUSED(pieceListBefore)

    VPGraphicsPiece *_graphicsPiece = nullptr;
    for(auto graphicPiece : m_graphicsPieces)
    {
        if(graphicPiece->GetPiece() == piece)
        {
            _graphicsPiece = graphicPiece;
        }
    }

    if(pieceListAfter == m_layout->GetUnplacedPieceList() && _graphicsPiece != nullptr)
    {
        scene()->removeItem(_graphicsPiece);
        m_graphicsPieces.removeAll(_graphicsPiece);
    }
    else if(pieceListAfter != m_layout->GetUnplacedPieceList())
    {
        if(_graphicsPiece == nullptr)
        {
            _graphicsPiece = new VPGraphicsPiece(piece);
            m_graphicsPieces.append(_graphicsPiece);
        }

        scene()->addItem(_graphicsPiece);
        // not very clean to directly call slots
        _graphicsPiece->on_PieceSelectionChanged();
        _graphicsPiece->on_PiecePositionChanged();
        _graphicsPiece->on_PieceRotationChanged();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleMainGraphicsView::on_SceneSelectionChanged()
{
    // most of the selection behaviour taks place automatically
    // but we need to make sure that the unplaced pieces are unselected when the scene selection has changed
    // because as they are not part of the scene, they are not updated

    m_layout->GetUnplacedPieceList()->ClearSelection();
}
