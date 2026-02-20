/************************************************************************
 **
 **  @file   vtoolline.cpp
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

#include "vtoolline.h"

#include <QKeyEvent>
#include <QLineF>
#include <QMessageLogger>
#include <QPen>
#include <QPointF>
#include <QSharedPointer>
#include <QUndoStack>
#include <QtDebug>

#include "../../dialogs/tools/dialogline.h"
#include "../../dialogs/tools/dialogtool.h"
#include "../../undocommands/renameobject.h"
#include "../../undocommands/savetooloptions.h"
#include "../../visualization/line/vistoolline.h"
#include "../../visualization/visualization.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vabstracttool.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/exception/vexception.h"
#include "../vmisc/theme/vscenestylesheet.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/global.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vdrawtool.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

template <class T> class QSharedPointer;

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolLine constructor.
 * @param initData init data.
 * @param parent parent object.
 */
VToolLine::VToolLine(const VToolLineInitData &initData, QGraphicsItem *parent)
  : VDrawTool(initData.doc, initData.data, initData.id, initData.notes),
    VScaledLine(VColorRole::CustomColor, parent),
    firstPoint(initData.firstPoint),
    secondPoint(initData.secondPoint),
    lineColor(initData.lineColor),
    m_acceptHoverEvents(true)
{
    SetBoldLine(false);
    this->m_lineType = initData.typeLine;
    // Line
    RefreshGeometry();
    this->setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus
    this->setAcceptHoverEvents(m_acceptHoverEvents);

    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolLine::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogLine> dialogTool = qobject_cast<DialogLine *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetFirstPoint(firstPoint);
    dialogTool->SetSecondPoint(secondPoint);
    dialogTool->SetTypeLine(m_lineType);
    dialogTool->SetLineColor(lineColor);
    dialogTool->SetNotes(m_notes);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool form GUI.
 * @param dialog dialog.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 */
auto VToolLine::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                       VContainer *data) -> VToolLine *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogLine> dialogTool = qobject_cast<DialogLine *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolLineInitData initData;
    initData.firstPoint = dialogTool->GetFirstPoint();
    initData.secondPoint = dialogTool->GetSecondPoint();
    initData.typeLine = dialogTool->GetTypeLine();
    initData.lineColor = dialogTool->GetLineColor();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();

    VToolLine *line = Create(initData);
    if (line != nullptr)
    {
        line->m_dialog = dialog;
    }
    return line;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool.
 * @param initData init data.
 */
auto VToolLine::Create(VToolLineInitData initData) -> VToolLine *
{
    SCASSERT(initData.scene != nullptr)
    SCASSERT(initData.doc != nullptr)
    SCASSERT(initData.data != nullptr)
    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->getNextId();
    }
    else
    {
        initData.data->UpdateId(initData.id);
    }

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->AddVertex(initData.id, VNodeType::TOOL, initData.doc->PatternBlockMapper()->GetActiveId());

    initData.data->AddLine(initData.firstPoint, initData.secondPoint, initData.id);

    patternGraph->AddEdge(initData.firstPoint, initData.id);
    patternGraph->AddEdge(initData.secondPoint, initData.id);

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::Line, initData.doc);
        auto *line = new VToolLine(initData);
        initData.scene->addItem(line);
        InitDrawToolConnections(initData.scene, line);
        connect(initData.scene, &VMainGraphicsScene::EnableLineItemSelection, line, &VToolLine::AllowSelecting);
        connect(initData.scene, &VMainGraphicsScene::EnableLineItemHover, line, &VToolLine::AllowHover);
        VAbstractPattern::AddTool(initData.id, line);
        return line;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolLine::getTagName() const -> QString
{
    return VDomDocument::TagLine;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Don't set pen width. Parent should take care of it.
    QPen lPen = pen();
    lPen.setColor(VSceneStylesheet::CorrectToolColor(this, VSceneStylesheet::CorrectToolColorForDarkTheme(lineColor)));
    lPen.setStyle(LineStyleToPenStyle(m_lineType));
    setPen(lPen);

    PaintWithFixItemHighlightSelected<VScaledLine>(this, painter, option, widget);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolLine::FirstPointName() const -> QString
{
    return VAbstractTool::data.GetGObject(firstPoint)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolLine::SecondPointName() const -> QString
{
    return VAbstractTool::data.GetGObject(secondPoint)->name();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FullUpdateFromFile update tool data form file.
 */
void VToolLine::FullUpdateFromFile()
{
    ReadAttributes();
    RefreshGeometry();
    SetVisualization();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::Enable()
{
    setEnabled(m_indexActivePatternBlock == doc->PatternBlockMapper()->GetActiveId());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::AllowHover(bool enabled)
{
    // Manually handle hover events. Need for setting cursor for not selectable paths.
    m_acceptHoverEvents = enabled;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::AllowSelecting(bool enabled)
{
    setFlag(QGraphicsItem::ItemIsSelectable, enabled);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    Q_UNUSED(id)
    try
    {
        ContextMenu<DialogLine>(event);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief contextMenuEvent handle context menu events.
 * @param event context menu event.
 */
void VToolLine::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    ShowContextMenu(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief AddToFile add tag with informations about tool into file.
 */
void VToolLine::AddToFile()
{
    QDomElement domElement = doc->createElement(getTagName());
    auto obj = QSharedPointer<VGObject>();
    SaveOptions(domElement, obj);
    AddToCalculation(domElement);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief hoverEnterEvent handle hover enter events.
 * @param event hover enter event.
 */
void VToolLine::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_acceptHoverEvents)
    {
        SetBoldLine(true);
        setToolTip(MakeToolTip());
        VScaledLine::hoverEnterEvent(event);
    }
    else
    {
        setCursor(VAbstractValApplication::VApp()->getSceneView()->viewport()->cursor());
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief hoverLeaveEvent handle hover leave events.
 * @param event hover leave event.
 */
void VToolLine::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    if (m_acceptHoverEvents && vis.isNull())
    {
        SetBoldLine(false);
        VScaledLine::hoverLeaveEvent(event);
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief itemChange handle item change.
 * @param change change.
 * @param value value.
 * @return value.
 */
auto VToolLine::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value) -> QVariant
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        emit ChangedToolSelection(value.toBool(), m_id, m_id);
    }

    return QGraphicsItem::itemChange(change, value);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief keyReleaseEvent handle key realse events.
 * @param event key realse event.
 */
void VToolLine::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key())
    {
        case Qt::Key_Delete:
            try
            {
                DeleteToolWithConfirm();
            }
            catch (const VExceptionToolWasDeleted &e)
            {
                Q_UNUSED(e)
                return; // Leave this method immediately!!!
            }
            break;
        default:
            break;
    }
    QGraphicsItem::keyReleaseEvent(event);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolLine::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogLine> dialogTool = qobject_cast<DialogLine *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrFirstPoint, QString().setNum(dialogTool->GetFirstPoint()));
    doc->SetAttribute(domElement, AttrSecondPoint, QString().setNum(dialogTool->GetSecondPoint()));
    doc->SetAttribute(domElement, AttrTypeLine, dialogTool->GetTypeLine());
    doc->SetAttribute(domElement, AttrLineColor, dialogTool->GetLineColor());
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrNotes,
                                         dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept -> bool { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VDrawTool::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrFirstPoint, firstPoint);
    doc->SetAttribute(tag, AttrSecondPoint, secondPoint);
    doc->SetAttribute(tag, AttrTypeLine, m_lineType);
    doc->SetAttribute(tag, AttrLineColor, lineColor);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::ReadToolAttributes(const QDomElement &domElement)
{
    VDrawTool::ReadToolAttributes(domElement);

    firstPoint = VAbstractPattern::GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
    secondPoint = VAbstractPattern::GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);
    m_lineType = VDomDocument::GetParametrString(domElement, AttrTypeLine, TypeLineLine);
    lineColor = VDomDocument::GetParametrString(domElement, AttrLineColor, ColorBlack);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolLine *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetPoint1Id(firstPoint);
        visual->SetPoint2Id(secondPoint);
        visual->SetLineStyle(LineStyleToPenStyle(m_lineType));
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolLine::MakeToolTip() const -> QString
{
    const QSharedPointer<VPointF> first = VAbstractTool::data.GeometricObject<VPointF>(firstPoint);
    const QSharedPointer<VPointF> second = VAbstractTool::data.GeometricObject<VPointF>(secondPoint);

    const QLineF line(static_cast<QPointF>(*first), static_cast<QPointF>(*second));

    const QString toolTip = u"<table>"
                            u"<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                            u"<tr> <td><b>%4:</b> %5°</td> </tr>"
                            u"</table>"_s.arg(tr("Length"))
                                .arg(VAbstractValApplication::VApp()->fromPixel(line.length()))
                                .arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true), tr("Angle"))
                                .arg(line.angle());
    return toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogLine> dialogTool = qobject_cast<DialogLine *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    const QString oldFirstPointLabel = FirstPointName();
    const QString newFirstPointLabel = VAbstractTool::data.GetGObject(dialogTool->GetFirstPoint())->name();

    const QString oldSecondPointLabel = SecondPointName();
    const QString newSecondPointLabel = VAbstractTool::data.GetGObject(dialogTool->GetFirstPoint())->name();

    if (oldFirstPointLabel == newFirstPointLabel && oldSecondPointLabel == newSecondPointLabel)
    {
        VDrawTool::ApplyToolOptions(oldDomElement, newDomElement);
        return;
    }

    QUndoStack *undoStack = VAbstractApplication::VApp()->getUndoStack();
    auto *newGroup = new QUndoCommand(); // an empty command
    newGroup->setText(tr("save tool options"));

    auto *saveOptions = new SaveToolOptions(oldDomElement, newDomElement, doc, m_id, newGroup);
    saveOptions->SetInGroup(true);
    connect(saveOptions, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);

    ObjectPair_t newPair;
    if (oldFirstPointLabel != newFirstPointLabel && oldSecondPointLabel != newSecondPointLabel)
    {
        newPair = std::make_pair(newFirstPointLabel, newSecondPointLabel);
    }
    else if (oldFirstPointLabel != newFirstPointLabel)
    {
        newPair = std::make_pair(newFirstPointLabel, oldSecondPointLabel);
    }
    else if (oldSecondPointLabel != newSecondPointLabel)
    {
        newPair = std::make_pair(oldFirstPointLabel, newSecondPointLabel);
    }

    auto *renamePair = RenamePair::CreateForLine(std::make_pair(oldFirstPointLabel, oldSecondPointLabel),
                                                 newPair,
                                                 doc,
                                                 m_id,
                                                 newGroup);
    connect(renamePair, &RenamePair::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);

    undoStack->push(newGroup);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::ShowVisualization(bool show)
{
    if (show)
    {
        if (vis.isNull())
        {
            AddVisualization<VisToolLine>();
            SetVisualization();
        }
        else
        {
            if (auto *visual = qobject_cast<VisToolLine *>(vis))
            {
                visual->show();
            }
        }
    }
    else
    {
        delete vis.data();
        hoverLeaveEvent(nullptr);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::SetLineType(const QString &value)
{
    m_lineType = value;

    QSharedPointer<VGObject> obj; // We don't have object for line in data container. Just will send an empty object.
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolLine::GetLineColor() const -> QString
{
    return lineColor;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::SetLineColor(const QString &value)
{
    lineColor = value;

    QSharedPointer<VGObject> obj; // We don't have object for line in data container. Just will send an empty object.
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::SetNotes(const QString &notes)
{
    m_notes = notes;

    QSharedPointer<VGObject> obj; // We don't have object for line in data container. Just will send an empty object.
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLine::GroupVisibility(quint32 object, bool visible)
{
    Q_UNUSED(object)
    setVisible(visible);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RefreshGeometry refresh item on scene.
 */
void VToolLine::RefreshGeometry()
{
    const QSharedPointer<VPointF> first = VAbstractTool::data.GeometricObject<VPointF>(firstPoint);
    const QSharedPointer<VPointF> second = VAbstractTool::data.GeometricObject<VPointF>(secondPoint);
    this->setLine(QLineF(static_cast<QPointF>(*first), static_cast<QPointF>(*second)));
}
