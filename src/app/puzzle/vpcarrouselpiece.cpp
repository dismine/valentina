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

#include <QLabel>
#include <QVBoxLayout>
#include <QGraphicsScene>
#include <QPainter>
#include <QDrag>
#include <QPainter>
#include <QApplication>
#include <QMenu>

#include "vpmimedatapiece.h"
#include "vpcarrouselpiecelist.h"
#include "vpcarrousel.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pCarrouselPiece, "p.carrouselPiece")


//---------------------------------------------------------------------------------------------------------------------
VPCarrouselPiece::VPCarrouselPiece(VPuzzlePiece *piece, VPCarrouselPieceList *carrouselPieceList) :
    m_piece(piece),
    m_carrouselPieceList(carrouselPieceList),
    m_dragStart(QPoint())
{
    Init();
}


//---------------------------------------------------------------------------------------------------------------------
VPCarrouselPiece::~VPCarrouselPiece()
{
    delete m_piecePreview;
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPiece::Init()
{
    // Define the structure
    setFixedSize(124,128);
    QVBoxLayout *pieceLayout = new QVBoxLayout();
    pieceLayout->setMargin(0);
    pieceLayout->setSpacing(0);
    setLayout(pieceLayout);

    setStyleSheet("background-color:white; border: 2px solid transparent;");

    // define the preview of the piece
    m_piecePreview = new VPCarrouselPiecePreview(this);

    // m_graphicsView = new VMainGraphicsView(this);
    // --> undefined reference to 'VMainGraphicsView::VMainGraphicView(QWidget*)'
    QGraphicsScene *graphicsScene = new QGraphicsScene(this);
    m_piecePreview->setScene(graphicsScene);
    m_piecePreview->setFixedSize(120,100);
    m_piecePreview->setStyleSheet("border: 4px solid transparent;");
    m_piecePreview->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    // define the label
    m_label = new QLabel();
    m_label->sizePolicy();
    m_label->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    m_label->setFixedSize(120,24);
    m_label->setStyleSheet("border: 0px;");
    m_label->setMouseTracking(false);

    pieceLayout->addWidget(m_piecePreview);
    pieceLayout->addWidget(m_label);


    // connect the signals
    connect(m_piece, &VPuzzlePiece::SelectionChanged, this, &VPCarrouselPiece::on_PieceSelectionChanged);

    // then refresh the data
    Refresh();
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPiece::CleanPreview()
{
    m_piecePreview->fitInView(m_piecePreview->scene()->sceneRect(), Qt::KeepAspectRatio);
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPiece::Refresh()
{
    // update the graphic view / the scene

    QVector<QPointF> points = m_piece->GetSeamLine();
    if(points.isEmpty())
    {
        points = m_piece->GetCuttingLine();
    }

    QPen pen(Qt::black, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    pen.setCosmetic(true);
    QBrush noBrush(Qt::NoBrush);

    QPainterPath path;
    path.moveTo(points.first());
    for (int i = 1; i < points.size(); ++i)
        path.lineTo(points.at(i));
    m_piecePreview->scene()->addPath(path, pen, noBrush);

    m_piecePreview->fitInView(m_piecePreview->scene()->sceneRect(), Qt::KeepAspectRatio);

    // update the label of the piece
    QFontMetrics metrix(m_label->font());
    int width = m_label->width() - 8;
    QString clippedText = metrix.elidedText(m_piece->GetName(), Qt::ElideRight, width);
    m_label->setText(clippedText);

    // set the tooltip
    setToolTip(m_piece->GetName());

    // set the selection state correctly.
    on_PieceSelectionChanged();
}

//---------------------------------------------------------------------------------------------------------------------
VPuzzlePiece * VPCarrouselPiece::GetPiece()
{
    return m_piece;
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPiece::on_PieceSelectionChanged()
{
    if(m_piece->GetIsSelected())
    {
        setStyleSheet("background-color:white; border: 2px solid red;");
    }
    else
    {
        setStyleSheet("background-color:white; border: 2px solid transparent;");
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPiece::mousePressEvent(QMouseEvent *event)
{
    qCDebug(pCarrouselPiece, "mouse pressed");


    if (event->button() == Qt::LeftButton)
    {
        if(!(event->modifiers() & Qt::ControlModifier))
        {
            m_carrouselPieceList->GetCarrousel()->ClearSelection();
            m_piece->SetIsSelected(true);
        }
        else
        {
            m_piece->SetIsSelected(!m_piece->GetIsSelected());
        }

        m_dragStart = event->pos();
    }
}


//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPiece::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
    {
        return;
    }

    if(m_piece->GetPieceList() != m_piece->GetPieceList()->GetLayout()->GetUnplacedPieceList())
    {
        return;
    }

    if((event->pos() - m_dragStart).manhattanLength() < QApplication::startDragDistance())
    {
        return;
    }

    // make sure the multiple selection is removed
    m_carrouselPieceList->GetCarrousel()->ClearSelection();
    m_piece->SetIsSelected(true);

    // starts the dragging
    QDrag *drag = new QDrag(this);
    VPMimeDataPiece *mimeData = new VPMimeDataPiece();
    mimeData->SetPiecePtr(m_piece);
    mimeData->setObjectName("piecePointer");

    // in case we would want to have the pieces original size:
    //drag->setHotSpot(QPoint(0,0));
    //QPixmap pixmap(m_piecePreview->sceneRect().size().toSize());

    QPixmap pixmap(112,92);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    m_piecePreview->scene()->render(&painter);

    drag->setPixmap(pixmap);
    drag->setMimeData(mimeData);
    drag->exec();
}




//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPiece::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu contextMenu;

    VPPieceList* unplacedPieces = m_piece->GetPieceList()->GetLayout()->GetUnplacedPieceList();
    QList<VPPieceList*> pieceLists = m_piece->GetPieceList()->GetLayout()->GetPiecesLists();

    // move to piece list actions  -- TODO : To be tested properly when we have several piece lists
    pieceLists.removeAll(m_piece->GetPieceList());
    if(pieceLists.count() > 0)
    {
        QMenu *moveMenu = contextMenu.addMenu(tr("Move to"));

        // TODO order in alphabetical order

        for (auto pieceList : pieceLists)
        {
            QAction* moveToPieceList = moveMenu->addAction(pieceList->GetName());
            QVariant data = QVariant::fromValue(pieceList);
            moveToPieceList->setData(data);

            connect(moveToPieceList, &QAction::triggered, this, &VPCarrouselPiece::on_ActionPieceMovedToPieceList);
        }
    }

    // remove from piece list action
    if(m_piece->GetPieceList() != unplacedPieces)
    {
        QAction *removeAction = contextMenu.addAction(tr("Remove from Layout"));
        QVariant data = QVariant::fromValue(m_piece->GetPieceList()->GetLayout()->GetUnplacedPieceList());
        removeAction->setData(data);
        connect(removeAction, &QAction::triggered, this, &VPCarrouselPiece::on_ActionPieceMovedToPieceList);
    }

    contextMenu.exec(event->globalPos());
}

//---------------------------------------------------------------------------------------------------------------------
void VPCarrouselPiece::on_ActionPieceMovedToPieceList()
{
    QAction *act = qobject_cast<QAction *>(sender());
    QVariant v = act->data();
    VPPieceList *pieceList = v.value<VPPieceList *>();
    if(pieceList != nullptr)
    {
        pieceList->GetLayout()->MovePieceToPieceList(m_piece, pieceList);
    }
}
