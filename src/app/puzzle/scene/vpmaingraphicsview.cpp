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
#include <QUndoStack>

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
#include "../undocommands/vpundopiecemove.h"
#include "../undocommands/vpundopiecerotate.h"
#include "../undocommands/vpundooriginmove.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(pMainGraphicsView, "p.mainGraphicsView")

namespace
{
const QKeySequence restoreOriginShortcut = QKeySequence(Qt::ControlModifier + Qt::Key_Asterisk);
}


//---------------------------------------------------------------------------------------------------------------------
VPMainGraphicsView::VPMainGraphicsView(const VPLayoutPtr &layout, VPTileFactory *tileFactory, QWidget *parent) :
    VMainGraphicsView(parent),
    m_scene(new VMainGraphicsScene(this)),
    m_layout(layout)
{
    SCASSERT(not m_layout.isNull())
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
    connect(layout.get(), &VPLayout::PieceSheetChanged, this, &VPMainGraphicsView::on_PieceSheetChanged);

    auto *restoreOrigin = new QAction(this);
    restoreOrigin->setShortcut(restoreOriginShortcut);
    connect(restoreOrigin, &QAction::triggered, this, &VPMainGraphicsView::RestoreOrigin);
    this->addAction(restoreOrigin);
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

    VPLayoutPtr layout = m_layout.toStrongRef();
    if (not layout.isNull())
    {
        VPSheetPtr sheet = layout->GetFocusedSheet();
        if (not sheet.isNull())
        {
            QList<VPPiecePtr> pieces = sheet->GetPieces();
            m_graphicsPieces.reserve(pieces.size());

            for (const auto &piece : pieces)
            {
                if (not piece.isNull())
                {
                    auto *graphicsPiece = new VPGraphicsPiece(piece);
                    m_graphicsPieces.append(graphicsPiece);
                    scene()->addItem(graphicsPiece);
                    ConnectPiece(graphicsPiece);
                }
            }
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

    VPLayoutPtr layout = m_layout.toStrongRef();
    if (not layout.isNull())
    {
        m_showGridTmp = layout->GetFocusedSheet()->GetLayout()->LayoutSettings().GetShowGrid();
        layout->GetFocusedSheet()->GetLayout()->LayoutSettings().SetShowGrid(false);

        m_showTilesTmp = layout->LayoutSettings().GetShowTiles();
        layout->LayoutSettings().SetShowTiles(false);
    }

    RefreshLayout();
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::CleanAfterExport()
{
    m_graphicsSheet->SetShowBorder(true);
    m_graphicsSheet->SetShowMargin(true);

    VPLayoutPtr layout = m_layout.toStrongRef();
    if (not layout.isNull())
    {
        layout->GetFocusedSheet()->GetLayout()->LayoutSettings().SetShowGrid(m_showGridTmp);

        layout->LayoutSettings().SetShowTiles(m_showTilesTmp);
    }

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

        VPPiecePtr piece = mimePiece->GetPiecePtr();
        if(not piece.isNull())
        {
            VPLayoutPtr layout = m_layout.toStrongRef();
            if (layout.isNull())
            {
                return;
            }

            qCDebug(pMainGraphicsView(), "element dropped, %s", qUtf8Printable(piece->GetName()));
            event->acceptProposedAction();

            piece->ClearTransformations();
            piece->SetPosition(mapToScene(event->pos()));

            // change the piecelist of the piece
            piece->SetSheet(layout->GetFocusedSheet());

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
            VPPiecePtr piece = graphicsPiece->GetPiece();

            if(not piece.isNull() && piece->IsSelected())
            {
                piece->SetSelected(false);
                piece->SetSheet(VPSheetPtr());
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
    else if (event->key() == Qt::Key_BracketLeft)
    {
        if((event->modifiers() & Qt::ControlModifier) != 0U)
        {
            RotatePiecesByAngle(90);
        }
        else if((event->modifiers() & Qt::AltModifier) != 0U)
        {
            RotatePiecesByAngle(1);
        }
        else
        {
            RotatePiecesByAngle(15);
        }
    }
    else if (event->key() == Qt::Key_BracketRight)
    {
        if((event->modifiers() & Qt::ControlModifier) != 0U)
        {
            RotatePiecesByAngle(-90);
        }
        else if((event->modifiers() & Qt::AltModifier) != 0U)
        {
            RotatePiecesByAngle(-1);
        }
        else
        {
            RotatePiecesByAngle(-15);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Left ||
            event->key() == Qt::Key_Right ||
            event->key() == Qt::Key_Up ||
            event->key() == Qt::Key_Down ||
            event->key() == Qt::Key_BracketLeft ||
            event->key() == Qt::Key_BracketRight)
    {
        if (not event->isAutoRepeat())
        {
            m_allowChangeMerge = false;
        }
    }

    if (event->key() == Qt::Key_BracketLeft || event->key() == Qt::Key_BracketRight)
    {
        if (not event->isAutoRepeat())
        {
            m_rotationControls->SetIgnorePieceTransformation(false);
            m_rotationControls->on_UpdateControls();
            m_rotationControls->on_HideHandles(false);
        }
    }
    VMainGraphicsView::keyReleaseEvent(event);
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

    VPLayoutPtr layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return;
    }

    VPSheetPtr sheet = layout->GetFocusedSheet();

    QMenu menu;

    QAction *restoreOriginAction = menu.addAction(tr("Restore transformation origin"));
    restoreOriginAction->setShortcut(restoreOriginShortcut);
    restoreOriginAction->setEnabled(not sheet.isNull() && sheet->TransformationOrigin().custom);

    QAction *removeSheetAction = menu.addAction(QIcon::fromTheme(QStringLiteral("edit-delete")), tr("Remove sheet"));
    removeSheetAction->setEnabled(not sheet.isNull() && layout->GetSheets().size() > 1);

    QAction *selectedAction = menu.exec(event->globalPos());
    if (selectedAction == removeSheetAction)
    {
        if (sheet != nullptr)
        {
            sheet->SetVisible(false);

            QList<VPPiecePtr> pieces = sheet->GetPieces();
            for (const auto &piece : pieces)
            {
                if (not piece.isNull())
                {
                    piece->SetSheet(nullptr);
                }
            }
        }

        layout->SetFocusedSheet(VPSheetPtr());
        emit on_SheetRemoved();
    }
    else if (selectedAction == restoreOriginAction)
    {
        RestoreOrigin();
    }

}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::RestoreOrigin() const
{
    VPLayoutPtr layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return;
    }

    VPSheetPtr sheet = layout->GetFocusedSheet();
    if (not sheet.isNull())
    {
        VPTransformationOrigon origin = sheet->TransformationOrigin();
        if (origin.custom)
        { // ignore if not custom. Prevent double call
            origin.custom = false;

            QRectF boundingRect;
            for (auto *graphicsPiece : m_graphicsPieces)
            {
                if (graphicsPiece->isSelected())
                {
                    boundingRect = boundingRect.united(graphicsPiece->sceneBoundingRect());
                }
            }
            origin.origin = boundingRect.center();

            auto *command = new VPUndoOriginMove(sheet, origin);
            layout->UndoStack()->push(command);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::ConnectPiece(VPGraphicsPiece *piece)
{
    SCASSERT(piece != nullptr)

    VPLayoutPtr layout = m_layout.toStrongRef();

    connect(layout.get(), &VPLayout::PieceTransformationChanged, piece,
            &VPGraphicsPiece::on_RefreshPiece);
    connect(piece, &VPGraphicsPiece::PieceSelectionChanged,
            m_rotationControls, &VPGraphicsPieceControls::on_UpdateControls);
    connect(piece, &VPGraphicsPiece::PieceTransformationChanged,
            m_rotationControls, &VPGraphicsPieceControls::on_UpdateControls);
    connect(piece, &VPGraphicsPiece::HideTransformationHandles,
            m_rotationControls, &VPGraphicsPieceControls::on_HideHandles);
    connect(piece, &VPGraphicsPiece::HideTransformationHandles,
            m_rotationOrigin, &VPGraphicsTransformationOrigin::on_HideHandles);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::RotatePiecesByAngle(qreal angle)
{
    m_rotationControls->on_HideHandles(true);
    m_rotationControls->SetIgnorePieceTransformation(true);

    VPLayoutPtr layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return;
    }

    VPSheetPtr sheet = layout->GetFocusedSheet();
    if (sheet.isNull())
    {
        return;
    }

    VPTransformationOrigon origin = sheet->TransformationOrigin();

    auto PreparePieces = [this]()
    {
        QVector<VPPiecePtr> pieces;
        for (auto *item : m_graphicsPieces)
        {
            if (item->isSelected())
            {
                pieces.append(item->GetPiece());
            }
        }

        return pieces;
    };

    QVector<VPPiecePtr> pieces = PreparePieces();

    if (pieces.size() == 1)
    {
        auto *command = new VPUndoPieceRotate(pieces.first(), origin.origin, angle, m_allowChangeMerge);
        layout->UndoStack()->push(command);
    }
    else if (pieces.size() > 1)
    {
        auto *command = new VPUndoPiecesRotate(pieces, origin.origin, angle, m_allowChangeMerge);
        layout->UndoStack()->push(command);
    }

    m_allowChangeMerge = true;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::TranslatePiecesOn(qreal dx, qreal dy)
{
    if (m_graphicsPieces.isEmpty())
    {
        return;
    }

    VPPiecePtr piece = m_graphicsPieces.first()->GetPiece();
    if (piece.isNull())
    {
        return;
    }

    VPLayoutPtr layout = piece->Layout();
    if (layout.isNull())
    {
        return;
    }

    auto PreparePieces = [this]()
    {
        QVector<VPPiecePtr> pieces;
        for (auto *graphicsPiece : m_graphicsPieces)
        {
            if (graphicsPiece->isSelected())
            {
                pieces.append(graphicsPiece->GetPiece());
            }
        }

        return pieces;
    };

    QVector<VPPiecePtr> pieces = PreparePieces();
    if (pieces.size() == 1)
    {
        auto *command = new VPUndoPieceMove(pieces.first(), dx, dy, m_allowChangeMerge);
        layout->UndoStack()->push(command);
    }
    else if (pieces.size() > 1)
    {
        auto *command = new VPUndoPiecesMove(pieces, dx, dy, m_allowChangeMerge);
        layout->UndoStack()->push(command);
    }

    m_allowChangeMerge = true;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::on_PieceSheetChanged(const VPPiecePtr &piece)
{
    VPGraphicsPiece *_graphicsPiece = nullptr;
    for(auto *graphicPiece : m_graphicsPieces)
    {
        if(graphicPiece->GetPiece() == piece)
        {
            _graphicsPiece = graphicPiece;
        }
    }

    VPLayoutPtr layout = piece->Layout();
    if (layout.isNull())
    {
        return;
    }

    if (piece->Sheet().isNull() || piece->Sheet() == layout->GetTrashSheet() ||
            piece->Sheet() != layout->GetFocusedSheet()) // remove
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
