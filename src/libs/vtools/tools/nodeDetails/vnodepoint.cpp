/************************************************************************
 **
 **  @file   vnodepoint.cpp
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

#include "vnodepoint.h"

#include <QBrush>
#include <QDomElement>
#include <QFlags>
#include <QGraphicsLineItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QIcon>
#include <QMenu>
#include <QPen>
#include <QPoint>
#include <QRectF>
#include <QSharedPointer>
#include <new>

#include "../../../vgeometry/vpointf.h"
#include "../../../vwidgets/vgraphicssimpletextitem.h"
#include "../../undocommands/label/movelabel.h"
#include "../../undocommands/label/showlabel.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vdomdocument.h"
#include "../vabstracttool.h"
#include "../vdatatool.h"
#include "../vmisc/theme/themeDef.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vpiecenode.h"
#include "../vtoolseamallowance.h"
#include "../vwidgets/global.h"
#include "../vwidgets/scalesceneitems.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "../vwidgets/vmaingraphicsview.h"
#include "vabstractnode.h"

const QString VNodePoint::ToolType = QStringLiteral("modeling");

namespace
{
enum class ContextMenuOption : int
{
    NoSelection,
    ShowLabel,
    NonePassmark,
    Exclude,
    ByLength,
    ByPointsIntersection,
    ByFirstEdgeSymmetry,
    BySecondEdgeSymmetry,
    ByFirstEdgeRightAngle,
    BySecondEdgeRightAngle,
    Straightforward,
    Bisector,
    Intersection,
    IntersectionOnlyLeft,
    IntersectionOnlyRight,
    Intersection2,
    Intersection2OnlyLeft,
    Intersection2OnlyRight,
    OneLine,
    TwoLines,
    ThreeLines,
    TMark,
    ExternalVMark,
    InternalVMark,
    UMark,
    BoxMark,
    CheckMark,
    Option,
    InLayout,
    ForbidFlipping,
    ForceFlipping,
    ResetLabelTemplate,
    Remove,
    TurnPoint,
    LAST_ONE_DO_NOT_USE
};
} // namespace

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VNodePoint constructor.
 * @param initData init data.
 * @param parent parent object.
 */
VNodePoint::VNodePoint(const VAbstractNodeInitData &initData, QObject *qoParent, QGraphicsItem *parent)
  : VAbstractNode(initData.doc, initData.data, initData.id, initData.idObject, initData.drawName, initData.idTool,
                  qoParent),
    VScenePoint(VColorRole::PiecePointColor, parent)
{
    m_namePoint->SetTextColor(VColorRole::PieceNodeLabelColor);
    m_namePoint->SetTextHoverColor(VColorRole::PieceNodeLabelHoverColor);
    m_namePoint->SetShowParentTooltip(false);
    connect(m_namePoint, &VGraphicsSimpleTextItem::PointChoosed, this, &VNodePoint::PointChoosed);
    connect(m_namePoint, &VGraphicsSimpleTextItem::NameChangePosition, this, &VNodePoint::NameChangePosition);
    connect(m_namePoint, &VGraphicsSimpleTextItem::ShowContextMenu, this,
            [this](QGraphicsSceneContextMenuEvent *event) { contextMenuEvent(event); });
    m_lineName->SetColorRole(VColorRole::PieceNodeLabelLineColor);
    RefreshPointGeometry(*VAbstractTool::data.GeometricObject<VPointF>(initData.id));
    ToolCreation(initData.typeCreation);
    setCursor(Qt::ArrowCursor);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool.
 * @param initData init data.
 */
void VNodePoint::Create(const VAbstractNodeInitData &initData)
{
    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::NodePoint, initData.doc);
        // TODO Need create garbage collector and remove all nodes, what we don't use.
        // Better check garbage before each saving file. Check only modeling tags.
        auto *point = new VNodePoint(initData);

        connect(initData.scene, &VMainGraphicsScene::EnableToolMove, point, &VNodePoint::EnableToolMove);
        connect(initData.scene, &VMainGraphicsScene::EnablePointItemHover, point, &VNodePoint::AllowHover);
        connect(initData.scene, &VMainGraphicsScene::EnablePointItemSelection, point, &VNodePoint::AllowSelecting);
        connect(initData.scene, &VMainGraphicsScene::EnableLabelItemHover, point, &VNodePoint::AllowLabelHover);
        connect(initData.scene, &VMainGraphicsScene::EnableLabelItemSelection, point, &VNodePoint::AllowLabelSelecting);
        VAbstractPattern::AddTool(initData.id, point);
        if (initData.idTool != NULL_ID)
        {
            // Some nodes we don't show on scene. Tool that create this nodes must free memory.
            VDataTool *tool = VAbstractPattern::getTool(initData.idTool);
            SCASSERT(tool != nullptr)
            point->setParent(tool); // Adopted by a tool
        }
        else
        {
            // Try to prevent memory leak
            initData.scene->addItem(point); // First adopted by scene
            point->hide();                  // If no one will use node, it will stay hidden
            point->SetParentType(ParentType::Scene);
        }
    }
    else
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VNodePoint::getTagName() const -> QString
{
    return VAbstractPattern::TagPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::ChangeLabelPosition(quint32 id, const QPointF &pos)
{
    if (id == m_id)
    {
        QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(id);
        point->setMx(pos.x());
        point->setMy(pos.y());
        m_namePoint->SetRealPos(pos);
        RefreshLine();
        if (QGraphicsScene *sc = scene())
        {
            VMainGraphicsView::NewSceneRect(sc, VAbstractValApplication::VApp()->getSceneView(), this);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::SetLabelVisible(quint32 id, bool visible)
{
    if (m_id == id)
    {
        const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(id);
        point->SetShowLabel(visible);
        RefreshPointGeometry(*point);
        if (QGraphicsScene *sc = scene())
        {
            VMainGraphicsView::NewSceneRect(sc, VAbstractValApplication::VApp()->getSceneView(), this);
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::PointChoosed()
{
    emit ChoosedTool(m_id, SceneObject::Point);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FullUpdateFromFile update tool data form file.
 */
void VNodePoint::FullUpdateFromFile()
{
    try
    {
        RefreshPointGeometry(*VAbstractTool::data.GeometricObject<VPointF>(m_id));
    }
    catch (const VExceptionBadId &)
    {
        // ignore
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddToFile add tag with informations about tool into file.
 */
void VNodePoint::AddToFile()
{
    const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    QDomElement domElement = doc->createElement(getTagName());

    doc->SetAttribute(domElement, VDomDocument::AttrId, m_id);
    doc->SetAttribute(domElement, AttrType, ToolType);
    doc->SetAttribute(domElement, AttrIdObject, idNode);
    doc->SetAttribute(domElement, AttrMx, VAbstractValApplication::VApp()->fromPixel(point->mx()));
    doc->SetAttribute(domElement, AttrMy, VAbstractValApplication::VApp()->fromPixel(point->my()));
    doc->SetAttribute<bool>(domElement, AttrShowLabel, point->IsShowLabel());
    if (idTool != NULL_ID)
    {
        doc->SetAttribute(domElement, AttrIdTool, idTool);
    }

    AddToModeling(domElement);
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Special for not selectable item first need to call standard mousePressEvent then accept event
    VScenePoint::mousePressEvent(event);

    // Somehow clicking on notselectable object do not clean previous selections.
    if (not(flags() & ItemIsSelectable) && scene())
    {
        scene()->clearSelection();
    }

    event->accept(); // Special for not selectable item first need to call standard mousePressEvent then accept event
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief mouseReleaseEvent handle mouse release events.
 * @param event mouse release event.
 */
void VNodePoint::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (IsSelectedByReleaseEvent(this, event))
    {
        emit ChoosedTool(m_id, SceneObject::Point);
    }
    VScenePoint::mouseReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief NameChangePosition label change position.
 * @param pos new position.
 */
void VNodePoint::NameChangePosition(const QPointF &pos)
{
    VAbstractApplication::VApp()->getUndoStack()->push(new MoveLabel(doc, pos - this->pos(), m_id));
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::ShowNode()
{
    if (parentType != ParentType::Scene && not m_exluded)
    {
        show();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::HideNode()
{
    hide();
}

//---------------------------------------------------------------------------------------------------------------------
auto VNodePoint::InitContextMenu(QMenu *menu, vidtype pieceId, quint32 referens) -> QHash<int, QAction *>
{
    SCASSERT(menu != nullptr)

    QHash<int, QAction *> contextMenu;

    QAction *actionShowLabel = menu->addAction(QCoreApplication::translate("VNodePoint", "Show label"));
    actionShowLabel->setCheckable(true);
    actionShowLabel->setChecked(VAbstractTool::data.GeometricObject<VPointF>(m_id)->IsShowLabel());
    contextMenu.insert(static_cast<int>(ContextMenuOption::ShowLabel), actionShowLabel);

    InitPassmarkMenu(menu, pieceId, contextMenu);

    contextMenu.insert(static_cast<int>(ContextMenuOption::Exclude),
                       menu->addAction(QCoreApplication::translate("VNodePoint", "Exclude")));

    InitAngleTypeMenu(menu, pieceId, contextMenu);
    InitPassmarkAngleTypeMenu(menu, pieceId, contextMenu);

    auto *separatorAct = new QAction(this);
    separatorAct->setSeparator(true);
    menu->addAction(separatorAct);

    contextMenu.insert(static_cast<int>(ContextMenuOption::Option),
                       menu->addAction(QIcon::fromTheme(QStringLiteral("preferences-other")),
                                       QCoreApplication::translate("VNodePoint", "Options")));

    const VPiece detail = VAbstractTool::data.GetPiece(pieceId);

    QAction *inLayoutOption = menu->addAction(QCoreApplication::translate("VNodePoint", "In layout"));
    inLayoutOption->setCheckable(true);
    inLayoutOption->setChecked(detail.IsInLayout());
    contextMenu.insert(static_cast<int>(ContextMenuOption::InLayout), inLayoutOption);

    QAction *forbidFlippingOption = menu->addAction(QCoreApplication::translate("VNodePoint", "Forbid flipping"));
    forbidFlippingOption->setCheckable(true);
    forbidFlippingOption->setChecked(detail.IsForbidFlipping());
    contextMenu.insert(static_cast<int>(ContextMenuOption::ForbidFlipping), forbidFlippingOption);

    QAction *forceFlippingOption = menu->addAction(QCoreApplication::translate("VNodePoint", "Force flipping"));
    forceFlippingOption->setCheckable(true);
    forceFlippingOption->setChecked(detail.IsForceFlipping());
    contextMenu.insert(static_cast<int>(ContextMenuOption::ForceFlipping), forceFlippingOption);

    QAction *reseteLabelTemplateOption =
        menu->addAction(QCoreApplication::translate("VNodePoint", "Reset piece label template"));
    reseteLabelTemplateOption->setEnabled(not doc->GetDefaultPieceLabelPath().isEmpty());
    contextMenu.insert(static_cast<int>(ContextMenuOption::ResetLabelTemplate), reseteLabelTemplateOption);

    QAction *actionRemove = menu->addAction(QIcon::fromTheme(QStringLiteral("edit-delete")),
                                            QCoreApplication::translate("VNodePoint", "Delete"));
    referens > 1 ? actionRemove->setEnabled(false) : actionRemove->setEnabled(true);
    contextMenu.insert(static_cast<int>(ContextMenuOption::Remove), actionRemove);

    return contextMenu;
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::InitPassmarkMenu(QMenu *menu, vidtype pieceId, QHash<int, QAction *> &contextMenu)
{
    SCASSERT(menu != nullptr)

    const VPiece detail = VAbstractTool::data.GetPiece(pieceId);
    const VPiecePath &path = detail.GetPath();
    const int nodeIndex = path.indexOfNode(m_id);
    if (nodeIndex == -1)
    {
        return;
    }

    const VPieceNode &node = path.at(nodeIndex);

    QMenu *passmarkSubmenu = menu->addMenu(tr("Passmark"));

    QAction *actionNonePassmark = passmarkSubmenu->addAction(tr("None"));
    actionNonePassmark->setCheckable(true);
    actionNonePassmark->setChecked(!node.IsPassmark());
    contextMenu.insert(static_cast<int>(ContextMenuOption::NonePassmark), actionNonePassmark);

    Q_STATIC_ASSERT_X(static_cast<int>(PassmarkLineType::LAST_ONE_DO_NOT_USE) == 9, "Not all types were handled.");

    auto InitPassmarkLineTypeAction = [passmarkSubmenu, node](const QString &name, PassmarkLineType lineType)
    {
        QAction *action = passmarkSubmenu->addAction(name);
        action->setCheckable(true);
        action->setChecked(node.IsPassmark() && lineType == node.GetPassmarkLineType());
        return action;
    };

    contextMenu.insert(static_cast<int>(ContextMenuOption::OneLine),
                       InitPassmarkLineTypeAction(tr("One line"), PassmarkLineType::OneLine));
    contextMenu.insert(static_cast<int>(ContextMenuOption::TwoLines),
                       InitPassmarkLineTypeAction(tr("Two lines"), PassmarkLineType::TwoLines));
    contextMenu.insert(static_cast<int>(ContextMenuOption::ThreeLines),
                       InitPassmarkLineTypeAction(tr("Three lines"), PassmarkLineType::ThreeLines));
    contextMenu.insert(static_cast<int>(ContextMenuOption::TMark),
                       InitPassmarkLineTypeAction(tr("T mark"), PassmarkLineType::TMark));
    contextMenu.insert(static_cast<int>(ContextMenuOption::ExternalVMark),
                       InitPassmarkLineTypeAction(tr("External V mark"), PassmarkLineType::ExternalVMark));
    contextMenu.insert(static_cast<int>(ContextMenuOption::InternalVMark),
                       InitPassmarkLineTypeAction(tr("Internal V mark"), PassmarkLineType::InternalVMark));
    contextMenu.insert(static_cast<int>(ContextMenuOption::UMark),
                       InitPassmarkLineTypeAction(tr("U mark"), PassmarkLineType::UMark));
    contextMenu.insert(static_cast<int>(ContextMenuOption::BoxMark),
                       InitPassmarkLineTypeAction(tr("Box mark"), PassmarkLineType::BoxMark));
    contextMenu.insert(static_cast<int>(ContextMenuOption::CheckMark),
                       InitPassmarkLineTypeAction(tr("Check mark"), PassmarkLineType::CheckMark));

    QAction *actionTurnPoint = menu->addAction(tr("Turn point"));
    actionTurnPoint->setCheckable(true);
    actionTurnPoint->setChecked(node.IsTurnPoint());
    contextMenu.insert(static_cast<int>(ContextMenuOption::TurnPoint), actionTurnPoint);
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::InitAngleTypeMenu(QMenu *menu, vidtype pieceId, QHash<int, QAction *> &contextMenu)
{
    QMenu *angleTypeMenu = menu->addMenu(tr("Seam allowance angle"));
    PieceNodeAngle curType = PieceNodeAngle::ByLength;

    const VPiece detail = VAbstractTool::data.GetPiece(pieceId);
    const VPiecePath &path = detail.GetPath();
    const int nodeIndex = path.indexOfNode(m_id);
    if (nodeIndex != -1)
    {
        curType = path.at(nodeIndex).GetAngleType();

        angleTypeMenu->setEnabled(detail.IsSeamAllowance() && not detail.IsSeamAllowanceBuiltIn());
    }
    else
    {
        angleTypeMenu->setVisible(false);
    }

    auto InitAngleAction = [angleTypeMenu, curType](const QString &name, PieceNodeAngle checkType)
    {
        QAction *action = angleTypeMenu->addAction(name);
        action->setCheckable(true);
        action->setChecked(curType == checkType);
        return action;
    };

    Q_STATIC_ASSERT_X(static_cast<int>(PieceNodeAngle::LAST_ONE_DO_NOT_USE) == 7, "Not all types were handled.");
    contextMenu.insert(static_cast<int>(ContextMenuOption::ByLength),
                       InitAngleAction(tr("by length"), PieceNodeAngle::ByLength));
    contextMenu.insert(static_cast<int>(ContextMenuOption::ByPointsIntersection),
                       InitAngleAction(tr("by points intersetions"), PieceNodeAngle::ByPointsIntersection));
    contextMenu.insert(static_cast<int>(ContextMenuOption::ByFirstEdgeSymmetry),
                       InitAngleAction(tr("by first edge symmetry"), PieceNodeAngle::ByFirstEdgeSymmetry));
    contextMenu.insert(static_cast<int>(ContextMenuOption::BySecondEdgeSymmetry),
                       InitAngleAction(tr("by second edge symmetry"), PieceNodeAngle::BySecondEdgeSymmetry));
    contextMenu.insert(static_cast<int>(ContextMenuOption::ByFirstEdgeRightAngle),
                       InitAngleAction(tr("by first edge right angle"), PieceNodeAngle::ByFirstEdgeRightAngle));
    contextMenu.insert(static_cast<int>(ContextMenuOption::BySecondEdgeRightAngle),
                       InitAngleAction(tr("by second edge right angle"), PieceNodeAngle::BySecondEdgeRightAngle));
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::InitPassmarkAngleTypeMenu(QMenu *menu, vidtype pieceId, QHash<int, QAction *> &contextMenu)
{
    const VPiece detail = VAbstractTool::data.GetPiece(pieceId);
    const VPiecePath &path = detail.GetPath();
    const int nodeIndex = path.indexOfNode(m_id);
    if (nodeIndex == -1)
    {
        return;
    }

    const VPieceNode &node = path.at(nodeIndex);

    QMenu *passmarkAngleMenu = menu->addMenu(tr("Passmark angle"));

    auto InitPassmarkAngleAction = [passmarkAngleMenu, node](const QString &name, PassmarkAngleType checkType)
    {
        QAction *action = passmarkAngleMenu->addAction(name);
        action->setCheckable(true);
        action->setChecked(node.IsPassmark() && node.GetPassmarkAngleType() == checkType);
        action->setEnabled(node.IsPassmark());
        return action;
    };

    Q_STATIC_ASSERT_X(static_cast<int>(PassmarkAngleType::LAST_ONE_DO_NOT_USE) == 8, "Not all types were handled.");
    contextMenu.insert(static_cast<int>(ContextMenuOption::Straightforward),
                       InitPassmarkAngleAction(tr("Straightforward"), PassmarkAngleType::Straightforward));
    contextMenu.insert(static_cast<int>(ContextMenuOption::Bisector),
                       InitPassmarkAngleAction(tr("Bisector"), PassmarkAngleType::Bisector));
    contextMenu.insert(static_cast<int>(ContextMenuOption::Intersection),
                       InitPassmarkAngleAction(tr("Intersection"), PassmarkAngleType::Intersection));
    contextMenu.insert(
        static_cast<int>(ContextMenuOption::IntersectionOnlyLeft),
        InitPassmarkAngleAction(tr("Intersection (only left)"), PassmarkAngleType::IntersectionOnlyLeft));
    contextMenu.insert(
        static_cast<int>(ContextMenuOption::IntersectionOnlyRight),
        InitPassmarkAngleAction(tr("Intersection (only right)"), PassmarkAngleType::IntersectionOnlyRight));
    contextMenu.insert(static_cast<int>(ContextMenuOption::Intersection2),
                       InitPassmarkAngleAction(tr("Intersection 2"), PassmarkAngleType::Intersection2));
    contextMenu.insert(
        static_cast<int>(ContextMenuOption::Intersection2OnlyLeft),
        InitPassmarkAngleAction(tr("Intersection 2 (only left)"), PassmarkAngleType::Intersection2OnlyLeft));
    contextMenu.insert(
        static_cast<int>(ContextMenuOption::Intersection2OnlyRight),
        InitPassmarkAngleAction(tr("Intersection 2 (only right)"), PassmarkAngleType::Intersection2OnlyRight));
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    if (m_suppressContextMenu)
    {
        return;
    }

    auto *piece = qgraphicsitem_cast<VToolSeamAllowance *>(parentItem());
    if (piece == nullptr)
    {
        return;
    }

    QMenu menu;
    QHash<int, QAction *> contextMenu = InitContextMenu(&menu, piece->getId(), piece->referens());

    PieceNodeAngle angleCurType = PieceNodeAngle::ByLength;
    PassmarkAngleType passmarkAngleCurType = PassmarkAngleType::Straightforward;
    PassmarkLineType passmarkLineCurType = PassmarkLineType::OneLine;

    const VPiece detail = VAbstractTool::data.GetPiece(piece->getId());
    const VPiecePath &path = detail.GetPath();
    const int nodeIndex = path.indexOfNode(m_id);
    if (nodeIndex != -1)
    {
        const VPieceNode &node = path.at(nodeIndex);
        angleCurType = node.GetAngleType();
        passmarkAngleCurType = node.GetPassmarkAngleType();
        passmarkLineCurType = node.GetPassmarkLineType();
    }

    auto SelectSeamAllowanceAngle = [angleCurType, this](PieceNodeAngle type)
    {
        if (angleCurType != type)
        {
            emit ToggleSeamAllowanceAngleType(m_id, type);
        }
    };

    auto SelectPassmarkAngle = [passmarkAngleCurType, this](PassmarkAngleType type)
    {
        if (passmarkAngleCurType != type)
        {
            emit TogglePassmarkAngleType(m_id, type);
        }
    };

    auto SelectPassmarkLine = [passmarkLineCurType, this](PassmarkLineType type)
    {
        emit TogglePassmark(m_id, true);

        if (passmarkLineCurType != type)
        {
            emit TogglePassmarkLineType(m_id, type);
        }
    };

    QAction *selectedAction = menu.exec(event->screenPos());
    ContextMenuOption selectedOption = static_cast<ContextMenuOption>(
        contextMenu.key(selectedAction, static_cast<int>(ContextMenuOption::NoSelection)));

    Q_STATIC_ASSERT_X(static_cast<int>(ContextMenuOption::LAST_ONE_DO_NOT_USE) == 34, "Not all options were handled.");

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wswitch-default")
    switch (selectedOption)
    {
        case ContextMenuOption::LAST_ONE_DO_NOT_USE:
            Q_UNREACHABLE();
            break;
        case ContextMenuOption::NoSelection:
            return;
        case ContextMenuOption::Option:
            emit ShowOptions();
            break;
        case ContextMenuOption::InLayout:
            emit ToggleInLayout(selectedAction->isChecked());
            break;
        case ContextMenuOption::ForbidFlipping:
            emit ToggleForbidFlipping(selectedAction->isChecked());
            break;
        case ContextMenuOption::ForceFlipping:
            emit ToggleForceFlipping(selectedAction->isChecked());
            break;
        case ContextMenuOption::ResetLabelTemplate:
            emit ResetPieceLabelTemplate();
            break;
        case ContextMenuOption::Remove:
            try
            {
                emit Delete();
            }
            catch (const VExceptionToolWasDeleted &e)
            {
                Q_UNUSED(e);
                return; // Leave this method immediately!!!
            }
            return; // Leave this method immediately after call!!!
        case ContextMenuOption::ShowLabel:
            VAbstractApplication::VApp()->getUndoStack()->push(new ShowLabel(doc, m_id, selectedAction->isChecked()));
            break;
        case ContextMenuOption::Exclude:
            emit ToggleExcludeState(m_id);
            break;
        case ContextMenuOption::TurnPoint:
            emit ToggleTurnPointState(m_id);
            break;
        case ContextMenuOption::ByLength:
            SelectSeamAllowanceAngle(PieceNodeAngle::ByLength);
            break;
        case ContextMenuOption::ByPointsIntersection:
            SelectSeamAllowanceAngle(PieceNodeAngle::ByPointsIntersection);
            break;
        case ContextMenuOption::ByFirstEdgeSymmetry:
            SelectSeamAllowanceAngle(PieceNodeAngle::ByFirstEdgeSymmetry);
            break;
        case ContextMenuOption::BySecondEdgeSymmetry:
            SelectSeamAllowanceAngle(PieceNodeAngle::BySecondEdgeSymmetry);
            break;
        case ContextMenuOption::ByFirstEdgeRightAngle:
            SelectSeamAllowanceAngle(PieceNodeAngle::ByFirstEdgeRightAngle);
            break;
        case ContextMenuOption::BySecondEdgeRightAngle:
            SelectSeamAllowanceAngle(PieceNodeAngle::BySecondEdgeRightAngle);
            break;
        case ContextMenuOption::NonePassmark:
            emit TogglePassmark(m_id, false);
            break;
        case ContextMenuOption::Straightforward:
            SelectPassmarkAngle(PassmarkAngleType::Straightforward);
            break;
        case ContextMenuOption::Bisector:
            SelectPassmarkAngle(PassmarkAngleType::Bisector);
            break;
        case ContextMenuOption::Intersection:
            SelectPassmarkAngle(PassmarkAngleType::Intersection);
            break;
        case ContextMenuOption::IntersectionOnlyLeft:
            SelectPassmarkAngle(PassmarkAngleType::IntersectionOnlyLeft);
            break;
        case ContextMenuOption::IntersectionOnlyRight:
            SelectPassmarkAngle(PassmarkAngleType::IntersectionOnlyRight);
            break;
        case ContextMenuOption::Intersection2:
            SelectPassmarkAngle(PassmarkAngleType::Intersection2);
            break;
        case ContextMenuOption::Intersection2OnlyLeft:
            SelectPassmarkAngle(PassmarkAngleType::Intersection2OnlyLeft);
            break;
        case ContextMenuOption::Intersection2OnlyRight:
            SelectPassmarkAngle(PassmarkAngleType::Intersection2OnlyRight);
            break;
        case ContextMenuOption::OneLine:
            SelectPassmarkLine(PassmarkLineType::OneLine);
            break;
        case ContextMenuOption::TwoLines:
            SelectPassmarkLine(PassmarkLineType::TwoLines);
            break;
        case ContextMenuOption::ThreeLines:
            SelectPassmarkLine(PassmarkLineType::ThreeLines);
            break;
        case ContextMenuOption::TMark:
            SelectPassmarkLine(PassmarkLineType::TMark);
            break;
        case ContextMenuOption::ExternalVMark:
            SelectPassmarkLine(PassmarkLineType::ExternalVMark);
            break;
        case ContextMenuOption::InternalVMark:
            SelectPassmarkLine(PassmarkLineType::InternalVMark);
            break;
        case ContextMenuOption::UMark:
            SelectPassmarkLine(PassmarkLineType::UMark);
            break;
        case ContextMenuOption::BoxMark:
            SelectPassmarkLine(PassmarkLineType::BoxMark);
            break;
        case ContextMenuOption::CheckMark:
            SelectPassmarkLine(PassmarkLineType::CheckMark);
            break;
    };
    QT_WARNING_POP
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    setToolTip(VAbstractTool::data.GetGObject(m_id)->name());
    VScenePoint::hoverEnterEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::EnableToolMove(bool move)
{
    m_namePoint->setFlag(QGraphicsItem::ItemIsMovable, move);
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::AllowHover(bool enabled)
{
    setAcceptHoverEvents(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::AllowSelecting(bool enabled)
{
    setFlag(QGraphicsItem::ItemIsSelectable, enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::AllowLabelHover(bool enabled)
{
    m_namePoint->setAcceptHoverEvents(enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VNodePoint::AllowLabelSelecting(bool enabled)
{
    m_namePoint->setFlag(QGraphicsItem::ItemIsSelectable, enabled);
}
