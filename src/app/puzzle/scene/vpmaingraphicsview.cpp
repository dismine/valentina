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

#include "../scene/vpgraphicssheet.h"
#include "../scene/vpgraphicspiece.h"
#include "../vptilefactory.h"
#include "../scene/vpgraphicstilegrid.h"
#include "../carousel/vpmimedatapiece.h"
#include "../layout/vplayout.h"
#include "../layout/vpsheet.h"
#include "../layout/vppiece.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vptilefactory.h"
#include "vpgraphicspiececontrols.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pMainGraphicsView, "p.mainGraphicsView")

namespace
{
const QKeySequence restoreOriginShortcut = QKeySequence(Qt::ControlModifier + Qt::Key_Asterisk);
}


//---------------------------------------------------------------------------------------------------------------------
VPMainGraphicsView::VPMainGraphicsView(VPLayout *layout, VPTileFactory *tileFactory, QWidget *parent) :
    VMainGraphicsView(parent),
    m_scene(new VMainGraphicsScene(this)),
    m_layout(layout)
{
    SCASSERT(m_layout != nullptr)
    setScene(m_scene);

    m_graphicsSheet = new VPGraphicsSheet(m_layout);
    m_graphicsSheet->setPos(0, 0);
    m_scene->addItem(m_graphicsSheet);

    setAcceptDrops(true);

    m_graphicsTileGrid = new VPGraphicsTileGrid(m_layout, tileFactory);
    m_scene->addItem(m_graphicsTileGrid);

    m_rotationControls = new VPGraphicsPieceControls(m_layout);
    m_rotationControls->setVisible(false);
    m_scene->addItem(m_rotationControls);

    m_rotationOrigin = new VPGraphicsTransformationOrigin(m_layout);
    m_rotationOrigin->setVisible(false);
    m_scene->addItem(m_rotationOrigin);

    connect(m_rotationControls, &VPGraphicsPieceControls::ShowOrigin,
            m_rotationOrigin, &VPGraphicsTransformationOrigin::on_ShowOrigin);
    connect(m_rotationControls, &VPGraphicsPieceControls::TransformationOriginChanged,
            m_rotationOrigin, &VPGraphicsTransformationOrigin::SetTransformationOrigin);

    // add the connections
    connect(m_layout, &VPLayout::PieceSheetChanged, this, &VPMainGraphicsView::on_PieceSheetChanged);

    auto *restoreOrigin = new QAction(this);
    restoreOrigin->setShortcut(restoreOriginShortcut);
    connect(restoreOrigin, &QAction::triggered, this, &VPMainGraphicsView::RestoreOrigin);
    this->addAction(restoreOrigin);

    auto *rotateByPlus15 = new QAction(this);
    rotateByPlus15->setShortcut(QKeySequence(Qt::Key_BracketLeft));
    connect(rotateByPlus15, &QAction::triggered, this, &VPMainGraphicsView::RotatePiecesByPlus15);
    this->addAction(rotateByPlus15);

    auto *rotateByMinus15 = new QAction(this);
    rotateByMinus15->setShortcut(QKeySequence(Qt::Key_BracketRight));
    connect(rotateByMinus15, &QAction::triggered, this, &VPMainGraphicsView::RotatePiecesByMinus15);
    this->addAction(rotateByMinus15);

    auto *rotateByPlus90 = new QAction(this);
    rotateByPlus90->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_BracketLeft));
    connect(rotateByPlus90, &QAction::triggered, this, &VPMainGraphicsView::RotatePiecesByPlus90);
    this->addAction(rotateByPlus90);

    auto *rotateByMinus90 = new QAction(this);
    rotateByMinus90->setShortcut(QKeySequence(Qt::ControlModifier + Qt::Key_BracketRight));
    connect(rotateByMinus90, &QAction::triggered, this, &VPMainGraphicsView::RotatePiecesByMinus90);
    this->addAction(rotateByMinus90);

    auto *rotateByPlus1 = new QAction(this);
    rotateByPlus1->setShortcut(QKeySequence(Qt::AltModifier + Qt::Key_BracketLeft));
    connect(rotateByPlus1, &QAction::triggered, this, &VPMainGraphicsView::RotatePiecesByPlus1);
    this->addAction(rotateByPlus1);

    auto *rotateByMinus1 = new QAction(this);
    rotateByMinus1->setShortcut(QKeySequence(Qt::AltModifier + Qt::Key_BracketRight));
    connect(rotateByMinus1, &QAction::triggered, this, &VPMainGraphicsView::RotatePiecesByMinus1);
    this->addAction(rotateByMinus1);
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
            ConnectPiece(graphicsPiece);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VPMainGraphicsView::GetScene() -> VMainGraphicsScene*
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

            piece->ClearTransformations();
            piece->SetPosition(mapToScene(event->pos()));

            // change the piecelist of the piece
            piece->SetSheet(m_layout->GetFocusedSheet());

            auto *graphicsPiece = new VPGraphicsPiece(piece);
            m_graphicsPieces.append(graphicsPiece);

            scene()->addItem(graphicsPiece);

            ConnectPiece(graphicsPiece);

            event->acceptProposedAction();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete)
    {
        for(auto *graphicsPiece : m_graphicsPieces)
        {
            VPPiece *piece = graphicsPiece->GetPiece();

            if(piece->IsSelected())
            {
                piece->SetSelected(false);
                piece->SetSheet(nullptr);
                m_graphicsPieces.removeAll(graphicsPiece);
                delete graphicsPiece;
            }
        }
    }
    else if (event->key() == Qt::Key_Left)
    {
        if((event->modifiers() & Qt::ShiftModifier) != 0U)
        {
            TranslatePiecesOn(-10, 0);
        }
        else
        {
            TranslatePiecesOn(-1, 0);
        }
    }
    else if (event->key() == Qt::Key_Right)
    {
        if((event->modifiers() & Qt::ShiftModifier) != 0U)
        {
            TranslatePiecesOn(10, 0);
        }
        else
        {
            TranslatePiecesOn(1, 0);
        }
    }
    else if (event->key() == Qt::Key_Up)
    {
        if((event->modifiers() & Qt::ShiftModifier) != 0U)
        {
            TranslatePiecesOn(0, -10);
        }
        else
        {
            TranslatePiecesOn(0, 1);
        }
    }
    else if (event->key() == Qt::Key_Down)
    {
        if((event->modifiers() & Qt::ShiftModifier) != 0U)
        {
            TranslatePiecesOn(0, 10);
        }
        else
        {
            TranslatePiecesOn(0, 1);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::contextMenuEvent(QContextMenuEvent *event)
{
    QGraphicsItem *item = itemAt(event->pos());
    if (item != nullptr && item->type() == VPGraphicsPiece::Type)
    {
        VMainGraphicsView::contextMenuEvent(event);
        return;
    }

    QMenu menu;
    VPSheet *sheet = m_layout->GetFocusedSheet();

    QAction *restoreOriginAction = menu.addAction(tr("Restore transformation origin"));
    restoreOriginAction->setShortcut(restoreOriginShortcut);
    restoreOriginAction->setEnabled(sheet != nullptr && sheet->TransformationOrigin().custom);

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
    else if (selectedAction == restoreOriginAction)
    {
        RestoreOrigin();
    }

}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::RestoreOrigin() const
{
    VPSheet *sheet = m_layout->GetFocusedSheet();
    if (sheet != nullptr)
    {
        VPTransformationOrigon origin = sheet->TransformationOrigin();
        origin.custom = false;
        sheet->SetTransformationOrigin(origin);
        m_rotationControls->on_UpdateControls();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::RotatePiecesByPlus15() const
{
    RotatePiecesByAngle(15);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::RotatePiecesByMinus15() const
{
    RotatePiecesByAngle(-15);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::RotatePiecesByPlus90() const
{
    RotatePiecesByAngle(90);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::RotatePiecesByMinus90() const
{
    RotatePiecesByAngle(-90);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::RotatePiecesByPlus1() const
{
    RotatePiecesByAngle(1);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::RotatePiecesByMinus1() const
{
    RotatePiecesByAngle(-1);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::ConnectPiece(VPGraphicsPiece *piece)
{
    SCASSERT(piece != nullptr)

    connect(piece, &VPGraphicsPiece::PieceSelectionChanged,
            m_rotationControls, &VPGraphicsPieceControls::on_UpdateControls);
    connect(piece, &VPGraphicsPiece::PiecePositionChanged,
            m_rotationControls, &VPGraphicsPieceControls::on_UpdateControls);
    connect(m_rotationControls, &VPGraphicsPieceControls::Rotate, piece, &VPGraphicsPiece::on_Rotate);
    connect(piece, &VPGraphicsPiece::HideTransformationHandles,
            m_rotationControls, &VPGraphicsPieceControls::on_HideHandles);
    connect(piece, &VPGraphicsPiece::HideTransformationHandles,
            m_rotationOrigin, &VPGraphicsTransformationOrigin::on_HideHandles);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::RotatePiecesByAngle(qreal angle) const
{
    VPSheet *sheet = m_layout->GetFocusedSheet();
    if (sheet == nullptr)
    {
        return;
    }

    VPTransformationOrigon origin = sheet->TransformationOrigin();

    for(auto *graphicsPiece : m_graphicsPieces)
    {
        if (graphicsPiece->isSelected())
        {
            graphicsPiece->on_Rotate(origin.origin, angle);
            m_rotationControls->on_UpdateControls();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::TranslatePiecesOn(qreal dx, qreal dy) const
{
    for(auto *graphicsPiece : m_graphicsPieces)
    {
        if (graphicsPiece->isSelected())
        {
            graphicsPiece->TranslatePiece(dx, dy);
            m_rotationControls->on_UpdateControls();
        }
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

    if (piece->Sheet() == nullptr || piece->Sheet() == m_layout->GetTrashSheet() ||
            piece->Sheet() != m_layout->GetFocusedSheet()) // remove
    {
        if (_graphicsPiece != nullptr)
        {
            scene()->removeItem(_graphicsPiece);
            m_graphicsPieces.removeAll(_graphicsPiece);
            delete _graphicsPiece;
        }
    }
    else // add
    {
        if(_graphicsPiece == nullptr)
        {
            piece->ClearTransformations();
            _graphicsPiece = new VPGraphicsPiece(piece);
            m_graphicsPieces.append(_graphicsPiece);
            ConnectPiece(_graphicsPiece);
        }
        scene()->addItem(_graphicsPiece);
    }
}
