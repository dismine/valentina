/************************************************************************
 **
 **  @file   vbackgroundimageitem.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 1, 2022
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2022 Valentina project
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

#include "vbackgroundimageitem.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../../undocommands/image/movebackgroundimage.h"
#include "../../undocommands/image/holdbackgroundimage.h"
#include "../../undocommands/image/rotatebackgroundimage.h"
#include "../../undocommands/image/scalebackgroundimage.h"
#include "../../undocommands/image/renamebackgroundimage.h"
#include "../../undocommands/image/hidebackgroundimage.h"
#include "../../undocommands/image/resetbackgroundimage.h"
#include "../../undocommands/image/opaquebackgroundimage.h"
#include "../../undocommands/image/zvaluemovebackgroundimage.h"
#include "../toolsdef.h"

#include <QUndoStack>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include <QKeyEvent>

//---------------------------------------------------------------------------------------------------------------------
VBackgroundImageItem::VBackgroundImageItem(const VBackgroundPatternImage &image, VAbstractPattern *doc,
                                           QGraphicsItem *parent)
    : QGraphicsObject{parent},
      m_image(image),
      m_doc(doc)
{
    SCASSERT(doc != nullptr)

    setAcceptHoverEvents(true);

    connect(doc, &VAbstractPattern::BackgroundImageTransformationChanged, this,
            &VBackgroundImageItem::ImageTransformationChanged);
    connect(doc, &VAbstractPattern::BackgroundImageHoldChanged, this, &VBackgroundImageItem::HoldChanged);
    connect(doc, &VAbstractPattern::BackgroundImageVisibilityChanged, this, &VBackgroundImageItem::VisibilityChanged);
    connect(doc, &VAbstractPattern::BackgroundImageNameChanged, this, &VBackgroundImageItem::NameChanged);
    connect(doc, &VAbstractPattern::BackgroundImagesHoldChanged, this, &VBackgroundImageItem::UpdateHoldState);
    connect(doc, &VAbstractPattern::BackgroundImagesVisibilityChanged, this,
            &VBackgroundImageItem::UpdateVisibilityState);
    connect(doc, &VAbstractPattern::BackgroundImagesZValueChanged, this, &VBackgroundImageItem::ZValueChanged);
    connect(doc, &VAbstractPattern::BackgroundImagePositionChanged, this, &VBackgroundImageItem::PositionChanged);
    connect(doc, &VAbstractPattern::BackgroundImageOpacityChanged, this, &VBackgroundImageItem::OpacityChanged);

    InitImage();
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageItem::Image() const -> const VBackgroundPatternImage &
{
    return m_image;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::SetImage(const VBackgroundPatternImage &newImage)
{
    prepareGeometryChange();
    m_image = newImage;
    InitImage();
    m_stale = true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageItem::pen() -> QPen
{
    return {QBrush(), 1.0};
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageItem::name() const -> QString
{
    return m_image.Name();
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::setName(const QString &newName)
{
    VAbstractApplication::VApp()->getUndoStack()->push(new RenameBackgroundImage(m_image.Id(), newName, m_doc));
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageItem::IsHold() const -> bool
{
    return m_image.Hold();
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::SetHold(bool hold)
{
    VAbstractApplication::VApp()->getUndoStack()->push(new HoldBackgroundImage(m_image.Id(), hold, m_doc));
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageItem::IsVisible() const -> bool
{
    return m_image.Visible();
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::SetVisible(bool visible)
{
    VAbstractApplication::VApp()->getUndoStack()->push(new HideBackgroundImage(m_image.Id(), not visible, m_doc));
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageItem::GetOpacity() const -> qreal
{
    return m_image.Opacity();
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::SetOpacity(qreal opacity)
{
    VAbstractApplication::VApp()->getUndoStack()->push(new OpaqueBackgroundImage(m_image.Id(), opacity, m_doc));
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (m_showHover)
    {
        painter->save();

        QBrush brush(QColor(177, 216, 250, 25));
        painter->setBrush(brush);

        painter->drawRect(boundingRect());

        painter->restore();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::PositionChanged(QUuid id)
{
    if (m_image.Id() != id)
    {
        return;
    }

    QTransform oldMatrix = m_image.Matrix();
    m_image = m_doc->GetBackgroundImage(id);
    QTransform newMatrix = m_image.Matrix();

    if (not VFuzzyComparePossibleNulls(oldMatrix.m31(), newMatrix.m31()) ||
            not VFuzzyComparePossibleNulls(oldMatrix.m32(), newMatrix.m32()))
    {
        prepareGeometryChange();
        update();
    }

    emit UpdateControls();
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::ImageTransformationChanged(QUuid id)
{
    if (m_image.Id() != id)
    {
        return;
    }

    prepareGeometryChange();
    m_image = m_doc->GetBackgroundImage(id);

    emit UpdateControls();
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::HoldChanged(QUuid id)
{
    if (m_image.Id() != id)
    {
        return;
    }

    UpdateHoldState();
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::OpacityChanged(const QUuid &id)
{
    if (m_image.Id() != id)
    {
        return;
    }

    m_image = m_doc->GetBackgroundImage(m_image.Id());
    update();
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::VisibilityChanged(QUuid id)
{
    if (m_image.Id() != id)
    {
        return;
    }

    UpdateVisibilityState();
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::NameChanged(QUuid id)
{
    if (m_image.Id() != id)
    {
        return;
    }

    m_image = m_doc->GetBackgroundImage(id);
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::EnableSelection(bool enable)
{
    m_selectable = enable;
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, m_selectable && not m_image.Hold());
    setFlag(QGraphicsItem::ItemIsFocusable, m_selectable && not m_image.Hold());

    if (not m_selectable)
    {
        emit ActivateControls(QUuid());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::DeleteFromMenu()
{
    DeleteToolWithConfirm();
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageItem::itemChange(GraphicsItemChange change, const QVariant &value) -> QVariant
{
    if (change == ItemPositionChange && (scene() != nullptr))
    {
        // Each time we move something we call recalculation scene rect. In some cases this can cause moving
        // objects positions. And this cause infinite redrawing. That's why we wait the finish of saving the last move.
        static bool changeFinished = true;
        if (changeFinished)
        {
            changeFinished = false;

            // value - this is new position.
            const QPointF newPos = value.toPointF();
            const QPointF diff = newPos - m_lastMoveDistance;

            auto *command = new MoveBackgroundImage(m_image.Id(), diff.x(), diff.y(), m_doc, m_allowChangeMerge);
            VAbstractApplication::VApp()->getUndoStack()->push(command);

            const QList<QGraphicsView *> viewList = scene()->views();
            if (not viewList.isEmpty())
            {
                if (auto *view = qobject_cast<VMainGraphicsView *>(viewList.at(0)))
                {
                    view->EnsureItemVisibleWithDelay(this, VMainGraphicsView::scrollDelay);
                }
            }

            changeFinished = true;
            m_lastMoveDistance = newPos;
        }
        return pos();
    }

    return value;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (not m_selectable)
    {
        event->ignore();
        return;
    }

    if (not Image().Hold())
    {
        if (flags() & QGraphicsItem::ItemIsMovable)
        {
            if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
            {
                SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
            }
        }

        if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
        {
            m_lastMoveDistance = QPointF();
            emit Selected(m_image.Id());
            event->accept();
        }
        else
        {
            QGraphicsObject::mousePressEvent(event);
        }
    }
    else
    {
        if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick)
        {
            emit ActivateControls(m_image.Id());
        }
        QGraphicsObject::mousePressEvent(event);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsObject::mouseMoveEvent(event);
    m_allowChangeMerge = true;
    m_wasMoved = true;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && event->type() != QEvent::GraphicsSceneMouseDoubleClick &&
            (flags() & QGraphicsItem::ItemIsMovable))
    {
        m_lastMoveDistance = QPointF();
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
        m_allowChangeMerge = false;
        if (not m_wasMoved && m_selectable)
        {
            emit ActivateControls(m_image.Id());
        }
        m_wasMoved = false;
    }

    QGraphicsObject::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_selectable && flags() & QGraphicsItem::ItemIsMovable)
    {
        m_showHover = true;
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
    }
    else
    {
        setCursor(VAbstractValApplication::VApp()->getSceneView()->viewport()->cursor());
    }
    QGraphicsObject::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_selectable && flags() & QGraphicsItem::ItemIsMovable)
    {
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
    }
    else
    {
        setCursor(VAbstractValApplication::VApp()->getSceneView()->viewport()->cursor());
    }
    QGraphicsObject::hoverMoveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_showHover = false;
    setCursor(VAbstractValApplication::VApp()->getSceneView()->viewport()->cursor());
    QGraphicsObject::hoverLeaveEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (not m_selectable)
    {
        return;
    }

    QMenu menu;

    QAction *holdOption = menu.addAction(tr("Hold"));
    holdOption->setCheckable(true);
    holdOption->setChecked(m_image.Hold());

    QAction *actionVisible = menu.addAction(tr("Visible"));
    actionVisible->setCheckable(true);
    actionVisible->setChecked(m_image.Visible());

 #if defined(Q_OS_MAC)
    const QString actionShowTitle = tr("Show in Finder");
#else
    const QString actionShowTitle = tr("Show in Explorer");
#endif
    QAction *actionShow = menu.addAction(QIcon::fromTheme(QStringLiteral("system-search")), actionShowTitle);
    actionShow->setVisible(false);
    actionShow->setEnabled(QFileInfo::exists(m_image.FilePath()));

    QAction *actionSaveAs = menu.addAction(QIcon::fromTheme(QStringLiteral("document-save-as")), tr("Save as …"));
    actionSaveAs->setVisible(false);

    if (not m_image.FilePath().isEmpty())
    {
        actionShow->setVisible(true);
    }
    else if (not m_image.ContentData().isEmpty())
    {
        actionSaveAs->setVisible(true);
    }

    QAction *actionReset = menu.addAction(tr("Reset transformation"));
    actionReset->setEnabled(not m_image.Hold());

    QAction *actionRemove = menu.addAction(QIcon::fromTheme(QStringLiteral("edit-delete")), tr("Delete"));

    QAction *selectedAction = menu.exec(event->screenPos());
    if (selectedAction == holdOption)
    {
        SetHold(selectedAction->isChecked());
    }
    else if (selectedAction == actionVisible)
    {
        SetVisible(selectedAction->isChecked());
    }
    else if (selectedAction == actionShow)
    {
        emit ShowImageInExplorer(m_image.Id());
    }
    else if (selectedAction == actionSaveAs)
    {
        emit SaveImage(m_image.Id());
    }
    else if (selectedAction == actionReset)
    {
        VAbstractApplication::VApp()->getUndoStack()->push(new ResetBackgroundImage(m_image.Id(), m_doc));
    }
    else if (selectedAction == actionRemove)
    {
        DeleteFromMenu();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::keyPressEvent(QKeyEvent *event)
{
    const int move = (event->modifiers() & Qt::ShiftModifier) ? 10 : 1;

    int angle = 15;
    if(event->modifiers() & Qt::ControlModifier)
    {
        angle = 90;
    }
    else if(event->modifiers() & Qt::AltModifier)
    {
        angle = 1;
    }

    switch(event->key())
    {
        case Qt::Key_Left:
            TranslateImageOn(-move, 0);
            event->accept();
            return;
        case Qt::Key_Right:
            TranslateImageOn(move, 0);
            event->accept();
            return;
        case Qt::Key_Up:
            TranslateImageOn(0, -move);
            event->accept();
            return;
        case Qt::Key_Down:
            TranslateImageOn(0, move);
            event->accept();
            return;
        case Qt::Key_BracketLeft:
            RotateImageByAngle(angle);
            event->accept();
            return;
        case Qt::Key_BracketRight:
            RotateImageByAngle(-angle);
            event->accept();
            return;
        case Qt::Key_Period:
        case Qt::Key_Greater:
            (event->modifiers() & Qt::AltModifier) ? ScaleImageByFactor(2) : ScaleImageByAdjustSize(2);
            return;
        case Qt::Key_Comma:
        case Qt::Key_Less:
            (event->modifiers() & Qt::AltModifier) ? ScaleImageByFactor(0.5) : ScaleImageByAdjustSize(-2);
            return;
        case Qt::Key_Home:
            MoveImageZValue(static_cast<int>(ZValueMoveType::Top));
            return;
        case Qt::Key_PageUp:
            MoveImageZValue(static_cast<int>(ZValueMoveType::Up));
            return;
        case Qt::Key_PageDown:
            MoveImageZValue(static_cast<int>(ZValueMoveType::Down));
            return;
        case Qt::Key_End:
            MoveImageZValue(static_cast<int>(ZValueMoveType::Bottom));
            return;
        default:
            break;
    }

    QGraphicsObject::keyPressEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::keyReleaseEvent(QKeyEvent *event)
{
    if (not Image().Hold())
    {
        if (event->key() == Qt::Key_Delete)
        {
            if (ConfirmDeletion() == QMessageBox::Yes)
            {
                DeleteToolWithConfirm();
                event->accept();
                return;
            }
        }
        else if (event->key() == Qt::Key_Left ||
                 event->key() == Qt::Key_Right ||
                 event->key() == Qt::Key_Up ||
                 event->key() == Qt::Key_Down ||
                 event->key() == Qt::Key_BracketLeft ||
                 event->key() == Qt::Key_BracketRight ||
                 event->key() == Qt::Key_Period ||
                 event->key() == Qt::Key_Greater ||
                 event->key() == Qt::Key_Comma ||
                 event->key() == Qt::Key_Less)
        {
            if (not event->isAutoRepeat())
            {
                m_allowChangeMerge = false;
            }
            event->accept();
            return;
        }
    }
    QGraphicsObject::keyReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
auto VBackgroundImageItem::Stale() const -> bool
{
    return m_stale;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::MakeFresh() const
{
    m_stale = false;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::DeleteToolWithConfirm(bool ask)
{
    if (ask)
    {
        if (ConfirmDeletion() == QMessageBox::No)
        {
            return;
        }
    }

    emit ActivateControls(QUuid());
    emit DeleteImage(m_image.Id());
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::UpdateHoldState()
{
    m_image = m_doc->GetBackgroundImage(m_image.Id());
    setFlag(QGraphicsItem::ItemIsMovable, not m_image.Hold());
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, not m_image.Hold());
    setFlag(QGraphicsItem::ItemIsFocusable, not m_image.Hold());// For keyboard input focus
    emit UpdateControls();
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::UpdateVisibilityState()
{
    m_image = m_doc->GetBackgroundImage(m_image.Id());

    setVisible(m_image.Visible());

    if (not m_image.Visible())
    {
        emit ActivateControls(QUuid());
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::ZValueChanged()
{
    m_image = m_doc->GetBackgroundImage(m_image.Id());

    SetupZValue();
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::InitImage()
{
    SetupZValue();

    setFlag(QGraphicsItem::ItemIsMovable, not m_image.Hold());
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, not m_image.Hold());
    setFlag(QGraphicsItem::ItemIsFocusable, not m_image.Hold());// For keyboard input focus

    setVisible(m_image.Visible());
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::TranslateImageOn(qreal dx, qreal dy)
{
    auto *command = new MoveBackgroundImage(m_image.Id(), dx, dy, m_doc, m_allowChangeMerge);
    VAbstractApplication::VApp()->getUndoStack()->push(command);

    UpdateSceneRect();

    m_allowChangeMerge = true;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::RotateImageByAngle(qreal angle)
{
    QTransform imageMatrix = m_image.Matrix();

    QPointF originPos = m_image.BoundingRect().center();

    QTransform m;
    m.translate(originPos.x(), originPos.y());
    m.rotate(-angle);
    m.translate(-originPos.x(), -originPos.y());
    imageMatrix *= m;

    auto *command = new RotateBackgroundImage(m_image.Id(), imageMatrix, m_doc, m_allowChangeMerge);
    VAbstractApplication::VApp()->getUndoStack()->push(command);

    UpdateSceneRect();

    m_allowChangeMerge = true;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::ScaleImageByAdjustSize(qreal value)
{
    QRectF rect = m_image.BoundingRect();
    QRectF adjusted = rect;
    adjusted.adjust(-value, -value, value, value);

    qreal factor = adjusted.width() / rect.width();
    ScaleImageByFactor(factor);
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::ScaleImageByFactor(qreal factor)
{
    QTransform imageMatrix = m_image.Matrix();
    QPointF originPos = m_image.BoundingRect().center();

    QTransform m;
    m.translate(originPos.x(), originPos.y());
    m.scale(factor, factor);
    m.translate(-originPos.x(), -originPos.y());
    imageMatrix *= m;

    auto *command = new ScaleBackgroundImage(m_image.Id(), imageMatrix, m_doc, m_allowChangeMerge);
    VAbstractApplication::VApp()->getUndoStack()->push(command);

    UpdateSceneRect();

    m_allowChangeMerge = true;
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::MoveImageZValue(int move)
{
    auto zMove = static_cast<ZValueMoveType>(move);
    VAbstractApplication::VApp()->getUndoStack()->push(new ZValueMoveBackgroundImage(m_image.Id(), zMove, m_doc));
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::UpdateSceneRect()
{
    const QList<QGraphicsView *> viewList = scene()->views();
    if (not viewList.isEmpty())
    {
        if (auto *view = qobject_cast<VMainGraphicsView *>(viewList.at(0)))
        {
            setFlag(QGraphicsItem::ItemSendsGeometryChanges, false);
            VMainGraphicsView::NewSceneRect(scene(), view);
            setFlag(QGraphicsItem::ItemSendsGeometryChanges, not m_image.Hold());
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VBackgroundImageItem::SetupZValue()
{
    if (qFuzzyIsNull(m_image.ZValue()))
    {
        setZValue(-1);
    }
    else if (m_image.ZValue() > 0)
    {
        setZValue(-1 * m_image.ZValue() - 1);
    }
    else
    {
        setZValue(m_image.ZValue() - 1);
    }
}
