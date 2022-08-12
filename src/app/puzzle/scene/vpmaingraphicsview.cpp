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

#include "../carousel/vpmimedatapiece.h"
#include "../layout/vplayout.h"
#include "../layout/vpsheet.h"
#include "../layout/vppiece.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "undocommands/vpundopiecezvaluemove.h"
#include "vpgraphicspiece.h"
#include "vpgraphicspiececontrols.h"

#include "../undocommands/vpundopiecemove.h"
#include "../undocommands/vpundopiecerotate.h"
#include "../undocommands/vpundooriginmove.h"
#include "../undocommands/vpundomovepieceonsheet.h"
#include "../undocommands/vpundoremovesheet.h"

#include <QLoggingCategory>

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wmissing-prototypes")
QT_WARNING_DISABLE_INTEL(1418)

Q_LOGGING_CATEGORY(pMainGraphicsView, "p.mainGraphicsView") // NOLINT

QT_WARNING_POP

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wenum-enum-conversion")

Q_GLOBAL_STATIC_WITH_ARGS(const QKeySequence, restoreOriginShortcut, // NOLINT
                          (QKeySequence(Qt::ControlModifier + Qt::Key_Asterisk)))

QT_WARNING_POP
}


//---------------------------------------------------------------------------------------------------------------------
VPMainGraphicsView::VPMainGraphicsView(const VPLayoutPtr &layout, QWidget *parent) :
    VMainGraphicsView(parent),
    m_layout(layout)
{
    SCASSERT(not layout.isNull())

    SwitchScene(layout->GetFocusedSheet());

    setAcceptDrops(true);

    // add the connections
    connect(layout.data(), &VPLayout::PieceSheetChanged, this, &VPMainGraphicsView::on_PieceSheetChanged);
    connect(layout.data(), &VPLayout::ActiveSheetChanged, this, &VPMainGraphicsView::on_ActiveSheetChanged);

    auto *restoreOrigin = new QAction(this);
    restoreOrigin->setShortcut(*restoreOriginShortcut);
    connect(restoreOrigin, &QAction::triggered, this, &VPMainGraphicsView::RestoreOrigin);
    this->addAction(restoreOrigin);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::RefreshLayout()
{
    VPLayoutPtr layout = m_layout.toStrongRef();
    if (not layout.isNull())
    {
        VPSheetPtr sheet = layout->GetFocusedSheet();
        if (not sheet.isNull())
        {
            sheet->SceneData()->RefreshLayout();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::RefreshPieces()
{
    VPLayoutPtr layout = m_layout.toStrongRef();
    if (not layout.isNull())
    {
        VPSheetPtr sheet = layout->GetFocusedSheet();
        if (not sheet.isNull())
        {
            sheet->SceneData()->RefreshPieces();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *mime = event->mimeData();

    if(mime->hasFormat(VPMimeDataPiece::mineFormatPiecePtr))
    {
        VPLayoutPtr layout = m_layout.toStrongRef();
        if (layout.isNull())
        {
            return;
        }

        const auto *mimePiece = qobject_cast<const VPMimeDataPiece *> (mime);
        if (mimePiece != nullptr && mimePiece->LayoutUuid() == layout->Uuid())
        {
            qCDebug(pMainGraphicsView(), "drag enter");
            event->acceptProposedAction();
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::dragMoveEvent(QDragMoveEvent *event)
{
    const QMimeData *mime = event->mimeData();

    if(mime->hasFormat(VPMimeDataPiece::mineFormatPiecePtr))
    {
        VPLayoutPtr layout = m_layout.toStrongRef();
        if (layout.isNull())
        {
            return;
        }

        const auto *mimePiece = qobject_cast<const VPMimeDataPiece *> (mime);
        if (mimePiece != nullptr && mimePiece->LayoutUuid() == layout->Uuid())
        {
            event->acceptProposedAction();
        }
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
        VPLayoutPtr layout = m_layout.toStrongRef();
        if (layout.isNull())
        {
            return;
        }

        const auto *mimePiece = qobject_cast<const VPMimeDataPiece *> (mime);

        if (mimePiece == nullptr || mimePiece->LayoutUuid() != layout->Uuid())
        {
            return;
        }

        VPPiecePtr piece = mimePiece->GetPiecePtr();
        if(not piece.isNull())
        {
            qCDebug(pMainGraphicsView(), "element dropped, %s", qUtf8Printable(piece->GetName()));
            event->acceptProposedAction();

            piece->ClearTransformations();
            piece->SetPosition(mapToScene(event->pos()));
            piece->SetZValue(1.0);

            auto *command = new VPUndoMovePieceOnSheet(layout->GetFocusedSheet(), piece);
            layout->UndoStack()->push(command);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::keyPressEvent(QKeyEvent *event)
{
    const bool shiftModifier = (event->modifiers() & Qt::ShiftModifier) != 0U;
    const bool controlModifier = (event->modifiers() & Qt::ControlModifier) != 0U;
    const bool altModifier = (event->modifiers() & Qt::AltModifier) != 0U;

    switch(event->key())
    {
        case Qt::Key_Backspace:
        case Qt::Key_Delete:
            RemovePiece();
            break;
        case Qt::Key_Left:
            shiftModifier ? TranslatePiecesOn(-10, 0) : TranslatePiecesOn(-1, 0);
            break;
        case Qt::Key_Right:
            shiftModifier ? TranslatePiecesOn(10, 0) : TranslatePiecesOn(1, 0);
            break;
        case Qt::Key_Up:
            shiftModifier ? TranslatePiecesOn(0, -10) : TranslatePiecesOn(0, -1);
            break;
        case Qt::Key_Down:
            shiftModifier ? TranslatePiecesOn(0, 10) : TranslatePiecesOn(0, 1);
            break;
        case Qt::Key_BracketLeft:
            if(controlModifier)
            {
                RotatePiecesByAngle(90);
            }
            else if(altModifier)
            {
                RotatePiecesByAngle(1);
            }
            else
            {
                RotatePiecesByAngle(15);
            }
            break;
        case Qt::Key_BracketRight:
            if(controlModifier)
            {
                RotatePiecesByAngle(-90);
            }
            else if(altModifier)
            {
                RotatePiecesByAngle(-1);
            }
            else
            {
                RotatePiecesByAngle(-15);
            }
            break;
        case Qt::Key_Home:
            ZValueMove(static_cast<int>(ML::ZValueMove::Top));
            break;
        case Qt::Key_PageUp:
            ZValueMove(static_cast<int>(ML::ZValueMove::Up));
            break;
        case Qt::Key_PageDown:
            ZValueMove(static_cast<int>(ML::ZValueMove::Down));
            break;
        case Qt::Key_End:
            ZValueMove(static_cast<int>(ML::ZValueMove::Bottom));
            break;
        default:
            break;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::keyReleaseEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_Left:
        case Qt::Key_Right:
        case Qt::Key_Up:
        case Qt::Key_Down:
        case Qt::Key_BracketLeft:
        case Qt::Key_BracketRight:
            MovePiece(event);
            break;
        default:
            break;
    }

    if (event->key() == Qt::Key_BracketLeft || event->key() == Qt::Key_BracketRight)
    {
        if (not event->isAutoRepeat())
        {
            VPLayoutPtr layout = m_layout.toStrongRef();
            if (layout.isNull())
            {
                return;
            }

            VPSheetPtr sheet = layout->GetFocusedSheet();
            if (not sheet.isNull())
            {
                sheet->SceneData()->RotationControls()->SetIgnorePieceTransformation(false);
                sheet->SceneData()->RotationControls()->on_UpdateControls();
                sheet->SceneData()->RotationControls()->on_HideHandles(false);
            }

            m_rotationSum = 0;
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
    restoreOriginAction->setShortcut(*restoreOriginShortcut);
    restoreOriginAction->setEnabled(not sheet.isNull() && sheet->TransformationOrigin().custom);

    QAction *removeSheetAction = menu.addAction(QIcon::fromTheme(QStringLiteral("edit-delete")), tr("Remove sheet"));
    removeSheetAction->setEnabled(not sheet.isNull() && layout->GetSheets().size() > 1);

    QAction *selectedAction = menu.exec(event->globalPos());
    if (selectedAction == removeSheetAction)
    {
        layout->UndoStack()->push(new VPUndoRemoveSheet(sheet));
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
            QList<VPPiecePtr> selectedPieces = sheet->GetSelectedPieces();
            for (const auto& piece : selectedPieces)
            {
                if (piece->IsSelected())
                {
                    boundingRect = boundingRect.united(piece->MappedDetailBoundingRect());
                }
            }
            origin.origin = boundingRect.center();

            auto *command = new VPUndoOriginMove(sheet, origin);
            layout->UndoStack()->push(command);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::on_SceneMouseMove(const QPointF &scenePos)
{
    emit mouseMove(scenePos);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::RotatePiecesByAngle(qreal angle)
{
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

    sheet->SceneData()->RotationControls()->on_HideHandles(true);
    sheet->SceneData()->RotationControls()->SetIgnorePieceTransformation(true);

    VPTransformationOrigon origin = sheet->TransformationOrigin();

    auto PreparePieces = [this]()
    {
        QList<VPPiecePtr> pieces;
        VPLayoutPtr layout = m_layout.toStrongRef();
        if (not layout.isNull())
        {
            VPSheetPtr sheet = layout->GetFocusedSheet();
            if (not sheet.isNull())
            {
                pieces = sheet->GetSelectedPieces();
            }
        }

        return pieces;
    };

    if (layout->LayoutSettings().GetFollowGrainline() && not origin.custom)
    {
        if (m_rotationSum > 90 || m_rotationSum < -90)
        {
            m_rotationSum = angle;
        }
        else
        {
            m_rotationSum += angle;
        }
    }
    else
    {
        m_rotationSum = angle;
    }

    QList<VPPiecePtr> pieces = PreparePieces();

    if (pieces.size() == 1)
    {
        auto *command = new VPUndoPieceRotate(ConstFirst(pieces), origin, angle, m_rotationSum, m_allowChangeMerge);
        layout->UndoStack()->push(command);
    }
    else if (pieces.size() > 1)
    {
        auto *command = new VPUndoPiecesRotate(pieces, origin, angle, m_rotationSum, m_allowChangeMerge);
        layout->UndoStack()->push(command);
    }

    m_allowChangeMerge = true;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::TranslatePiecesOn(qreal dx, qreal dy)
{
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

    const QList<VPGraphicsPiece *> &graphicsPieces = sheet->SceneData()->GraphicsPieces();

    if (graphicsPieces.isEmpty())
    {
        return;
    }

    VPPiecePtr piece = ConstFirst(graphicsPieces)->GetPiece();
    if (piece.isNull())
    {
        return;
    }

    auto PreparePieces = [this]()
    {
        QList<VPPiecePtr> pieces;
        VPLayoutPtr layout = m_layout.toStrongRef();
        if (not layout.isNull())
        {
            VPSheetPtr sheet = layout->GetFocusedSheet();
            if (not sheet.isNull())
            {
                pieces = sheet->GetSelectedPieces();
            }
        }

        return pieces;
    };

    QList<VPPiecePtr> pieces = PreparePieces();
    if (pieces.size() == 1)
    {
        const VPPiecePtr &p = ConstFirst(pieces);
        auto *command = new VPUndoPieceMove(p, dx, dy, m_allowChangeMerge);
        layout->UndoStack()->push(command);

        if (layout->LayoutSettings().GetStickyEdges())
        {
            QVector<QPointF> path;
            if (not p.isNull() && p->StickyPosition(m_stickyTranslateX, m_stickyTranslateY))
            {
                path = p->GetMappedExternalContourPoints();
                QTransform m;
                m.translate(m_stickyTranslateX, m_stickyTranslateY);
                path = m.map(path);
                m_hasStickyPosition = true;
            }
            else
            {
                m_hasStickyPosition = false;
            }

            VPGraphicsPiece *gPiece = sheet->SceneData()->ScenePiece(p);
            if (gPiece != nullptr)
            {
                gPiece->SetStickyPoints(path);
            }
        }
    }
    else if (pieces.size() > 1)
    {
        auto *command = new VPUndoPiecesMove(pieces, dx, dy, m_allowChangeMerge);
        layout->UndoStack()->push(command);
    }

    m_allowChangeMerge = true;
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::SwitchScene(const VPSheetPtr &sheet)
{
    if (not sheet.isNull())
    {
       VMainGraphicsScene *scene = sheet->SceneData()->Scene();
       setScene(scene);
       connect(scene, &VMainGraphicsScene::mouseMove, this, &VPMainGraphicsView::on_SceneMouseMove,
               Qt::UniqueConnection);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::ClearSelection()
{
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

    QList<VPPiecePtr> pieces = sheet->GetSelectedPieces();
    for (const auto& piece : pieces)
    {
        piece->SetSelected(false);
        emit layout->PieceSelectionChanged(piece);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::ZValueMove(int move)
{
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

    QList<VPPiecePtr> selectedPieces = sheet->GetSelectedPieces();
    if (selectedPieces.isEmpty())
    {
        return;
    }

    QList<VPPiecePtr> allPieces = sheet->GetPieces();
    if (allPieces.isEmpty() || (allPieces.size() == selectedPieces.size()))
    {
        return;
    }

    auto zMove = static_cast<ML::ZValueMove>(move);

    if (selectedPieces.size() == 1)
    {
        layout->UndoStack()->push(new VPUndoPieceZValueMove(ConstFirst(selectedPieces), zMove));
    }
    else if (selectedPieces.size() > 1)
    {
        layout->UndoStack()->push(new VPUndoPiecesZValueMove(allPieces, zMove));
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::RemovePiece() const
{
    VPLayoutPtr layout = m_layout.toStrongRef();
    if (layout.isNull())
    {
        return;
    }

    VPSheetPtr sheet = layout->GetFocusedSheet();
    if (sheet.isNull())
    {
        return ;
    }

    const QList<VPGraphicsPiece *> &graphicsPieces = sheet->SceneData()->GraphicsPieces();
    for(auto *graphicsPiece : graphicsPieces)
    {
        VPPiecePtr piece = graphicsPiece->GetPiece();

        if(not piece.isNull() && piece->IsSelected())
        {
            piece->SetSelected(false);

            VPLayoutPtr layout = m_layout.toStrongRef();
            if (not layout.isNull())
            {
                emit layout->PieceSelectionChanged(piece);

                auto *command = new VPUndoMovePieceOnSheet(VPSheetPtr(), piece);
                layout->UndoStack()->push(command);
            }
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::MovePiece(QKeyEvent *event)
{
    if (not event->isAutoRepeat())
    {
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

        const QList<VPGraphicsPiece *> &graphicsPieces = sheet->SceneData()->GraphicsPieces();
        if (m_hasStickyPosition && not graphicsPieces.isEmpty())
        {
            if (layout->LayoutSettings().GetStickyEdges())
            {
                auto PreparePieces = [layout]()
                {
                    QList<VPPiecePtr> pieces;
                    VPSheetPtr sheet = layout->GetFocusedSheet();
                    if (not sheet.isNull())
                    {
                        pieces = sheet->GetSelectedPieces();
                    }

                    return pieces;
                };

                QList<VPPiecePtr> pieces = PreparePieces();
                if (pieces.size() == 1)
                {
                    const VPPiecePtr &p = ConstFirst(pieces);

                    auto *command = new VPUndoPieceMove(p, m_stickyTranslateX, m_stickyTranslateY,
                                                        m_allowChangeMerge);
                    layout->UndoStack()->push(command);

                    VPGraphicsPiece * gPiece = sheet->SceneData()->ScenePiece(p);
                    if (gPiece != nullptr)
                    {
                        gPiece->SetStickyPoints(QVector<QPointF>());
                    }
                }
            }
        }

        m_allowChangeMerge = false;
        m_hasStickyPosition = false;
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::on_PieceSheetChanged(const VPPiecePtr &piece)
{
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

    VPGraphicsPiece *graphicsPiece = sheet->SceneData()->ScenePiece(piece);

    if (piece != nullptr && (piece->Sheet().isNull() || piece->Sheet() == layout->GetTrashSheet() ||
            piece->Sheet() != layout->GetFocusedSheet())) // remove
    {
        if (graphicsPiece != nullptr)
        {
            sheet->SceneData()->Scene()->removeItem(graphicsPiece);
            sheet->SceneData()->RemovePiece(graphicsPiece);
            delete graphicsPiece;
        }
    }
    else // add
    {
        if(graphicsPiece == nullptr)
        {
            graphicsPiece = new VPGraphicsPiece(piece);
            sheet->SceneData()->AddPiece(graphicsPiece);
        }
        sheet->SceneData()->Scene()->addItem(graphicsPiece);
    }

    sheet->SceneData()->RotationControls()->on_UpdateControls();

    VMainGraphicsView::NewSceneRect(scene(), this);
}

//---------------------------------------------------------------------------------------------------------------------
void VPMainGraphicsView::on_ActiveSheetChanged(const VPSheetPtr &focusedSheet)
{
    if (not focusedSheet.isNull())
    {
        ClearSelection();
        SwitchScene(focusedSheet);
        RefreshLayout();
        RefreshPieces();
    }
}
