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

#include "vpuzzlemimedatapiece.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pMainGraphicsView, "p.mainGraphicsView")


//---------------------------------------------------------------------------------------------------------------------
VPuzzleMainGraphicsView::VPuzzleMainGraphicsView(VPuzzleLayout *layout, QWidget *parent) :
    QGraphicsView(parent),
    m_graphicsPieces(QList<VPuzzleGraphicsPiece*>())
{
    m_scene = new VPuzzleMainGraphicsScene(this);
    setScene(m_scene);

    m_graphicsLayout = new VPuzzleGraphicsLayout(layout);
    m_graphicsLayout->setPos(0,0);
    m_scene->addItem(m_graphicsLayout);

    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    setAcceptDrops(true);
}

//---------------------------------------------------------------------------------------------------------------------
VPuzzleMainGraphicsView::~VPuzzleMainGraphicsView()
{

}

//---------------------------------------------------------------------------------------------------------------------
void VPuzzleMainGraphicsView::RefreshLayout()
{
    // FIXME: Is that the way to go?

    m_graphicsLayout->update();

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
            QPointF scenePos = mapToScene(point);
            // todo take the position into account

            AddPiece(piece, scenePos);

        }
    }
}


//---------------------------------------------------------------------------------------------------------------------
void VPuzzleMainGraphicsView::AddPiece(VPuzzlePiece *piece, QPointF pos)
{
    VPuzzleGraphicsPiece *item = new VPuzzleGraphicsPiece(piece);
    m_scene->addItem(item);
    item->setSelected(true);
    item->setPos(pos);

    item->blockSignals(true);
    piece->SetPosition(pos);
    item->blockSignals(false);
}
