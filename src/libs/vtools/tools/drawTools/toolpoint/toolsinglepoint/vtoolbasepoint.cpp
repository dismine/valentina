/************************************************************************
 **
 **  @file   vtoolsinglepoint.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   November 15, 2013
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2013-2015 Valentina project
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

#include "vtoolbasepoint.h"

#include <QApplication>
#include <QEvent>
#include <QFlags>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QList>
#include <QMessageBox>
#include <QPen>
#include <QPointF>
#include <QPolygonF>
#include <QRectF>
#include <QSharedPointer>

#include "../../../../dialogs/tools/dialogsinglepoint.h"
#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../undocommands/addpatternpiece.h"
#include "../../../../undocommands/deletepatternpiece.h"
#include "../../../../undocommands/movespoint.h"
#include "../../../vabstracttool.h"
#include "../../../vdatatool.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/exception/vexception.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vgraphicssimpletextitem.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "def.h"
#include "vtoolsinglepoint.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

const QString VToolBasePoint::ToolType = QStringLiteral("single");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolBasePoint constructor.
 * @param initData init data.
 * @param parent parent object.
 */
VToolBasePoint::VToolBasePoint(const VToolBasePointInitData &initData, QGraphicsItem *parent)
  : VToolSinglePoint(initData.doc, initData.data, initData.id, initData.notes, parent),
    m_indexPatternBlock(initData.doc->PatternBlockMapper()->GetActiveId())
{
    SetColorRole(VColorRole::BasePointColor);
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    m_namePoint->setBrush(Qt::black);
    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolBasePoint::GatherToolChanges() const -> VAbstractPoint::ToolChanges
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogSinglePoint> dialogTool = qobject_cast<DialogSinglePoint *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    return {.pointId = m_id, .oldLabel = name(), .newLabel = dialogTool->GetPointName()};
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolBasePoint::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogSinglePoint> dialogTool = qobject_cast<DialogSinglePoint *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetData(p->name(), static_cast<QPointF>(*p));
    dialogTool->SetNotes(m_notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolBasePoint::Create(VToolBasePointInitData initData) -> VToolBasePoint *
{
    auto *point = new VPointF(initData.x, initData.y, initData.name, initData.mx, initData.my);
    point->SetShowLabel(initData.showLabel);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->AddGObject(point);
    }
    else
    {
        initData.data->UpdateGObject(initData.id, point);
    }

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->AddVertex(initData.id, VNodeType::TOOL, initData.doc->PatternBlockMapper()->GetActiveId());

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::BasePoint, initData.doc);
        auto *spoint = new VToolBasePoint(initData);
        initData.scene->addItem(spoint);
        InitToolConnections(initData.scene, spoint);
        VAbstractPattern::AddTool(initData.id, spoint);
        return spoint;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::ShowVisualization(bool show)
{
    Q_UNUSED(show) // don't have any visualization for base point yet
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolBasePoint::IsRemovable() const -> RemoveStatus
{
    if (doc->CountPatternBlockTags() <= 1)
    {
        return RemoveStatus::Locked; // One pattern block
    }

    if (!doc->IsPatternGraphComplete())
    {
        return RemoveStatus::Pending; // Data not ready yet
    }

    VPatternGraph const *patternGraph = doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    auto Filter = [this](const auto &node) -> auto
    {
        return node.indexPatternBlock != m_indexPatternBlock && node.type != VNodeType::MODELING_TOOL
               && node.type != VNodeType::MODELING_OBJECT;
    };

    auto const dependecies = patternGraph->TryGetDependentNodes(m_id, 1000, Filter);

    if (!dependecies)
    {
        return RemoveStatus::Pending; // Lock timeout
    }

    return dependecies->isEmpty() ? RemoveStatus::Removable : RemoveStatus::Blocked;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddToFile add tag with informations about tool into file.
 */
void VToolBasePoint::AddToFile()
{
    VPatternBlockMapper *blocks = doc->PatternBlockMapper();
    const QString blockName = blocks->FindName(m_indexPatternBlock);
    Q_ASSERT_X(not blockName.isEmpty(), Q_FUNC_INFO, "name pattern block name is empty");

    QDomElement sPoint = doc->createElement(getTagName());

    // Create SPoint tag
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOptions(sPoint, obj);

    // Create pattern piece structure
    QDomElement patternPiece = doc->createElement(VAbstractPattern::TagDraw);
    doc->SetAttribute(patternPiece, AttrName, blockName);

    if (blocks->GetElementById(m_indexPatternBlock).isNull())
    {
        blocks->SetElementById(m_indexPatternBlock, patternPiece);
    }

    QDomElement calcElement = doc->createElement(VAbstractPattern::TagCalculation);
    calcElement.appendChild(sPoint);

    patternPiece.appendChild(calcElement);
    patternPiece.appendChild(doc->createElement(VAbstractPattern::TagModeling));
    patternPiece.appendChild(doc->createElement(VAbstractPattern::TagDetails));

    auto *addPP = new AddPatternPiece(patternPiece, doc, m_indexPatternBlock);
    connect(addPP, &AddPatternPiece::ClearScene, doc, &VAbstractPattern::ClearScene);
    connect(addPP, &AddPatternPiece::NeedFullParsing, doc, &VAbstractPattern::NeedFullParsing);
    VAbstractApplication::VApp()->getUndoStack()->push(addPP);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief itemChange handle tool change.
 * @param change change.
 * @param value value.
 * @return value.
 */
auto VToolBasePoint::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) -> QVariant
{
    if (change == ItemPositionChange && scene())
    {
        // Each time we move something we call recalculation scene rect. In some cases this can cause moving
        // objects positions. And this cause infinite redrawing. That's why we wait the finish of saving the last move.
        static bool changeFinished = true;
        if (changeFinished)
        {
            changeFinished = false;
            // value - this is new position.
            QPointF const newPos = value.toPointF();

            auto *moveSP = new MoveSPoint(doc, newPos.x(), newPos.y(), m_id, this->scene());
            connect(moveSP, &MoveSPoint::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
            VAbstractApplication::VApp()->getUndoStack()->push(moveSP);
            const QList<QGraphicsView *> viewList = scene()->views();
            if (not viewList.isEmpty())
            {
                if (auto *view = qobject_cast<VMainGraphicsView *>(viewList.at(0)))
                {
                    view->EnsureItemVisibleWithDelay(this, VMainGraphicsView::scrollDelay);
                }
            }
            changeFinished = true;
        }
    }
    return VToolSinglePoint::itemChange(change, value);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolBasePoint::GetBasePointPos() const -> QPointF
{
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    QPointF pos(VAbstractValApplication::VApp()->fromPixel(p->x()), VAbstractValApplication::VApp()->fromPixel(p->y()));
    return pos;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::SetBasePointPos(const QPointF &pos)
{
    QSharedPointer<VPointF> const p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    p->setX(VAbstractValApplication::VApp()->toPixel(pos.x()));
    p->setY(VAbstractValApplication::VApp()->toPixel(pos.y()));

    QSharedPointer<VGObject> obj = qSharedPointerCast<VGObject>(p);

    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::DeleteToolWithConfirm(bool ask)
{
    if (IsRemovable() != RemoveStatus::Removable)
    {
        return;
    }

    qCDebug(vTool, "Deleting base point.");
    emit VAbstractValApplication::VApp()->getSceneView()->itemClicked(nullptr);
    if (ask)
    {
        qCDebug(vTool, "Asking.");
        if (ConfirmDeletion() == QMessageBox::No)
        {
            qCDebug(vTool, "User said no.");
            return;
        }
    }

    qCDebug(vTool, "Begin deleting.");
    auto *deletePP = new DeletePatternPiece(doc, m_indexPatternBlock);
    connect(deletePP, &DeletePatternPiece::NeedFullParsing, doc, &VAbstractPattern::NeedFullParsing);
    VAbstractApplication::VApp()->getUndoStack()->push(deletePP);

    // Throw exception, this will help prevent case when we forget to immediately quit function.
    throw VExceptionToolWasDeleted(QStringLiteral("Tool was used after deleting."));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolBasePoint::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogSinglePoint> dialogTool = qobject_cast<DialogSinglePoint *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    const QPointF p = dialogTool->GetPoint();
    const QString name = dialogTool->GetPointName();
    doc->SetAttribute(domElement, AttrName, name);
    doc->SetAttribute(domElement, AttrX, QString().setNum(VAbstractValApplication::VApp()->fromPixel(p.x())));
    doc->SetAttribute(domElement, AttrY, QString().setNum(VAbstractValApplication::VApp()->fromPixel(p.y())));
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrNotes,
                                         dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept -> bool { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    VToolSinglePoint::hoverEnterEvent(event);

    if (flags() & QGraphicsItem::ItemIsMovable)
    {
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (flags() & QGraphicsItem::ItemIsMovable && event->button() == Qt::LeftButton &&
        event->type() != QEvent::GraphicsSceneMouseDoubleClick)
    {
        SetItemOverrideCursor(this, cursorArrowCloseHand, 1, 1);
        event->accept();
    }
    VToolSinglePoint::mousePressEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (flags() & QGraphicsItem::ItemIsMovable && event->button() == Qt::LeftButton &&
        event->type() != QEvent::GraphicsSceneMouseDoubleClick)
    {
        SetItemOverrideCursor(this, cursorArrowOpenHand, 1, 1);
    }
    VToolSinglePoint::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    QSharedPointer<VPointF> const point = qSharedPointerDynamicCast<VPointF>(obj);
    SCASSERT(point.isNull() == false)

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrX, VAbstractValApplication::VApp()->fromPixel(point->x()));
    doc->SetAttribute(tag, AttrY, VAbstractValApplication::VApp()->fromPixel(point->y()));
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolBasePoint::MakeToolTip() const -> QString
{
    const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(m_id);

    const QString toolTip = u"<table>"
                            u"<tr> <td><b>%1:</b> %2</td> </tr>"
                            u"</table>"_s.arg(tr("Label"), point->name());
    return toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    ProcessPointToolOptions(oldDomElement, newDomElement, GatherToolChanges());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    qCDebug(vTool, "Context menu base point");
#ifndef QT_NO_CURSOR
    QGuiApplication::restoreOverrideCursor();
    qCDebug(vTool, "Restored overriden cursor");
#endif

    try
    {
        ContextMenu<DialogSinglePoint>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        qCDebug(vTool, "Tool was deleted. Immediately leave method.");
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
    qCDebug(vTool, "Context menu closed. Tool was not deleted.");
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FullUpdateFromFile update tool data form file.
 */
void VToolBasePoint::FullUpdateFromFile()
{
    ReadAttributes();
    RefreshPointGeometry(*VAbstractTool::data.GeometricObject<VPointF>(m_id));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolBasePoint::EnableToolMove(bool move)
{
    this->setFlag(QGraphicsItem::ItemIsMovable, move);
    VToolSinglePoint::EnableToolMove(move);
}
