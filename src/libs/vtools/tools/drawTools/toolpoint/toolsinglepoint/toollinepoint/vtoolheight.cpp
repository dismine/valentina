/************************************************************************
 **
 **  @file   vtoolheight.cpp
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

#include "vtoolheight.h"

#include <QSharedPointer>
#include <QUndoStack>

#include "../../../../../dialogs/tools/dialogheight.h"
#include "../../../../../dialogs/tools/dialogtool.h"
#include "../../../../../undocommands/renameobject.h"
#include "../../../../../undocommands/savetooloptions.h"
#include "../../../../../visualization/line/vistoolheight.h"
#include "../../../../../visualization/visualization.h"
#include "../../../../vabstracttool.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/exception/vexception.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vtoollinepoint.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

template <class T> class QSharedPointer;

const QString VToolHeight::ToolType = QStringLiteral("height");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolHeight constructor.
 * @param initData init data.
 * @param parent parent object.
 */
VToolHeight::VToolHeight(const VToolHeightInitData &initData, QGraphicsItem *parent)
  : VToolLinePoint(initData.doc, initData.data, initData.id, initData.typeLine, initData.lineColor, QString(),
                   initData.basePointId, 0, initData.notes, parent),
    p1LineId(initData.p1LineId),
    p2LineId(initData.p2LineId)
{
    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolHeight::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogHeight> dialogTool = qobject_cast<DialogHeight *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetTypeLine(m_lineType);
    dialogTool->SetLineColor(lineColor);
    dialogTool->SetBasePointId(basePointId);
    dialogTool->SetP1LineId(p1LineId);
    dialogTool->SetP2LineId(p2LineId);
    dialogTool->SetPointName(p->name());
    dialogTool->SetNotes(m_notes);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool from GUI.
 * @param dialog dialog.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 * @return the created tool
 */
auto VToolHeight::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                         VContainer *data) -> VToolHeight *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogHeight> dialogTool = qobject_cast<DialogHeight *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolHeightInitData initData;
    initData.basePointId = dialogTool->GetBasePointId();
    initData.p1LineId = dialogTool->GetP1LineId();
    initData.p2LineId = dialogTool->GetP2LineId();
    initData.typeLine = dialogTool->GetTypeLine();
    initData.lineColor = dialogTool->GetLineColor();
    initData.name = dialogTool->GetPointName();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();

    VToolHeight *point = Create(initData);
    if (point != nullptr)
    {
        point->m_dialog = dialog;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool
 * @param initData init data.
 */
auto VToolHeight::Create(VToolHeightInitData initData) -> VToolHeight *
{
    const QSharedPointer<VPointF> basePoint = initData.data->GeometricObject<VPointF>(initData.basePointId);
    const QSharedPointer<VPointF> p1Line = initData.data->GeometricObject<VPointF>(initData.p1LineId);
    const QSharedPointer<VPointF> p2Line = initData.data->GeometricObject<VPointF>(initData.p2LineId);

    QPointF const pHeight = FindPoint(QLineF(static_cast<QPointF>(*p1Line), static_cast<QPointF>(*p2Line)),
                                      static_cast<QPointF>(*basePoint));

    auto *p = new VPointF(pHeight, initData.name, initData.mx, initData.my);
    p->SetShowLabel(initData.showLabel);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->AddGObject(p);
    }
    else
    {
        initData.data->UpdateGObject(initData.id, p);
    }

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->AddVertex(initData.id, VNodeType::TOOL, initData.doc->PatternBlockMapper()->GetActiveId());

    initData.data->AddLine(initData.basePointId, initData.id);
    initData.data->AddLine(initData.p1LineId, initData.id);
    initData.data->AddLine(initData.id, initData.p2LineId);

    patternGraph->AddEdge(initData.basePointId, initData.id);
    patternGraph->AddEdge(initData.p1LineId, initData.id);
    patternGraph->AddEdge(initData.p2LineId, initData.id);

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::Height, initData.doc);
        auto *point = new VToolHeight(initData);
        initData.scene->addItem(point);
        InitToolConnections(initData.scene, point);
        VAbstractPattern::AddTool(initData.id, point);
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FindPoint find projection base point onto line.
 * @param line line
 * @param point base point.
 * @return point onto line.
 */
auto VToolHeight::FindPoint(const QLineF &line, const QPointF &point) -> QPointF
{
    return VGObject::ClosestPoint(line, point);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolHeight::FirstLinePointName() const -> QString
{
    return VAbstractTool::data.GetGObject(p1LineId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolHeight::SecondLinePointName() const -> QString
{
    return VAbstractTool::data.GetGObject(p2LineId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolHeight::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogHeight> dialogTool = qobject_cast<DialogHeight *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrName, dialogTool->GetPointName());
    doc->SetAttribute(domElement, AttrTypeLine, dialogTool->GetTypeLine());
    doc->SetAttribute(domElement, AttrLineColor, dialogTool->GetLineColor());
    doc->SetAttribute(domElement, AttrBasePoint, QString().setNum(dialogTool->GetBasePointId()));
    doc->SetAttribute(domElement, AttrP1Line, QString().setNum(dialogTool->GetP1LineId()));
    doc->SetAttribute(domElement, AttrP2Line, QString().setNum(dialogTool->GetP2LineId()));

    const QString notes = dialogTool->GetNotes();
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrNotes, notes,
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolHeight::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolLinePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrBasePoint, basePointId);
    doc->SetAttribute(tag, AttrP1Line, p1LineId);
    doc->SetAttribute(tag, AttrP2Line, p2LineId);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolHeight::ReadToolAttributes(const QDomElement &domElement)
{
    VToolLinePoint::ReadToolAttributes(domElement);

    m_lineType = VDomDocument::GetParametrString(domElement, AttrTypeLine, TypeLineLine);
    lineColor = VDomDocument::GetParametrString(domElement, AttrLineColor, ColorBlack);
    basePointId = VAbstractPattern::GetParametrUInt(domElement, AttrBasePoint, NULL_ID_STR);
    p1LineId = VAbstractPattern::GetParametrUInt(domElement, AttrP1Line, NULL_ID_STR);
    p2LineId = VAbstractPattern::GetParametrUInt(domElement, AttrP2Line, NULL_ID_STR);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolHeight::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolHeight *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetBasePointId(basePointId);
        visual->SetLineP1Id(p1LineId);
        visual->SetLineP2Id(p2LineId);
        visual->SetLineStyle(LineStyleToPenStyle(m_lineType));
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolHeight::MakeToolTip() const -> QString
{
    const QSharedPointer<VPointF> basePoint = VAbstractTool::data.GeometricObject<VPointF>(basePointId);
    const QSharedPointer<VPointF> p1Line = VAbstractTool::data.GeometricObject<VPointF>(p1LineId);
    const QSharedPointer<VPointF> p2Line = VAbstractTool::data.GeometricObject<VPointF>(p2LineId);
    const QSharedPointer<VPointF> current = VAbstractTool::data.GeometricObject<VPointF>(m_id);

    const QLineF curLine(static_cast<QPointF>(*basePoint), static_cast<QPointF>(*current));
    const QLineF p1ToCur(static_cast<QPointF>(*p1Line), static_cast<QPointF>(*current));
    const QLineF p2ToCur(static_cast<QPointF>(*p2Line), static_cast<QPointF>(*current));

    const QString toolTip = u"<table>"
                            u"<tr> <td><b>%10:</b> %11</td> </tr>"
                            u"<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                            u"<tr> <td><b>%4:</b> %5°</td> </tr>"
                            u"<tr> <td><b>%6:</b> %7 %3</td> </tr>"
                            u"<tr> <td><b>%8:</b> %9 %3</td> </tr>"
                            u"</table>"_s.arg(tr("Length"))
                                .arg(VAbstractValApplication::VApp()->fromPixel(curLine.length()))
                                .arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true), tr("Angle"))
                                .arg(curLine.angle())
                                .arg(u"%1->%2"_s.arg(p1Line->name(), current->name()))
                                .arg(VAbstractValApplication::VApp()->fromPixel(p1ToCur.length()))
                                .arg(u"%1->%2"_s.arg(p2Line->name(), current->name()))
                                .arg(VAbstractValApplication::VApp()->fromPixel(p2ToCur.length()))
                                .arg(tr("Label"), current->name());
    return toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolHeight::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogHeight> dialogTool = qobject_cast<DialogHeight *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    const QString oldLabel = VAbstractTool::data.GetGObject(m_id)->name();
    const QString newLabel = dialogTool->GetPointName();

    const QString newBasePointLabel = VAbstractTool::data.GetGObject(dialogTool->GetBasePointId())->name();
    const QString oldBasePointLabel = BasePointName();

    const QString newFirstPointLabel = VAbstractTool::data.GetGObject(dialogTool->GetP1LineId())->name();
    const QString oldFirstPointLabel = FirstLinePointName();

    const QString newSecondPointLabel = VAbstractTool::data.GetGObject(dialogTool->GetP2LineId())->name();
    const QString oldSecondPointLabel = SecondLinePointName();

    if (oldBasePointLabel == newBasePointLabel && oldFirstPointLabel == newFirstPointLabel
        && oldSecondPointLabel == newSecondPointLabel && oldLabel == newLabel)
    {
        VToolLinePoint::ApplyToolOptions(oldDomElement, newDomElement);
        return;
    }

    QUndoStack *undoStack = VAbstractApplication::VApp()->getUndoStack();
    auto *newGroup = new QUndoCommand(); // an empty command
    newGroup->setText(tr("save tool options"));

    auto *saveOptions = new SaveToolOptions(oldDomElement, newDomElement, doc, m_id, newGroup);
    saveOptions->SetInGroup(true);
    connect(saveOptions, &SaveToolOptions::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);

    if (oldBasePointLabel != newBasePointLabel)
    {
        auto *renamePair = RenamePair::CreateForLine(std::make_pair(oldBasePointLabel, oldLabel),
                                                     std::make_pair(newBasePointLabel, oldLabel),
                                                     doc,
                                                     m_id,
                                                     newGroup);
        if (oldLabel == newLabel && oldFirstPointLabel == newFirstPointLabel
            && oldSecondPointLabel == newSecondPointLabel)
        {
            connect(renamePair, &RenamePair::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    if (oldFirstPointLabel != newFirstPointLabel)
    {
        auto *renamePair = RenamePair::CreateForLine(std::make_pair(oldFirstPointLabel, oldLabel),
                                                     std::make_pair(newFirstPointLabel, oldLabel),
                                                     doc,
                                                     m_id,
                                                     newGroup);
        if (oldLabel == newLabel && oldSecondPointLabel == newSecondPointLabel)
        {
            connect(renamePair, &RenamePair::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    if (oldSecondPointLabel != newSecondPointLabel)
    {
        auto *renamePair = RenamePair::CreateForLine(std::make_pair(oldLabel, oldSecondPointLabel),
                                                     std::make_pair(oldLabel, newSecondPointLabel),
                                                     doc,
                                                     m_id,
                                                     newGroup);
        if (oldLabel == newLabel)
        {
            connect(renamePair, &RenamePair::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    if (oldLabel != newLabel)
    {
        auto *renameLabel = new RenameLabel(oldLabel, newLabel, doc, m_id, newGroup);
        connect(renameLabel, &RenameLabel::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
    }

    undoStack->push(newGroup);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolHeight::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolHeight>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolHeight::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogHeight>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}
