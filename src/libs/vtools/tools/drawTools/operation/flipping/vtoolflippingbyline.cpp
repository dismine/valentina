/************************************************************************
 **
 **  @file
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 9, 2016
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2016 Valentina project
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

#include "vtoolflippingbyline.h"

#include <QColor>
#include <QDomNode>
#include <QDomNodeList>
#include <QMapIterator>
#include <QPoint>
#include <QSharedPointer>
#include <QUndoStack>
#include <climits>
#include <qiterator.h>

#include "../../../../dialogs/tools/dialogflippingbyline.h"
#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../visualization/line/operation/vistoolflippingbyline.h"
#include "../../../../visualization/visualization.h"
#include "../../../vabstracttool.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/exception/vexception.h"
#include "../vmisc/vabstractapplication.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vformula.h"
#include "../vwidgets/vabstractsimple.h"
#include "../vwidgets/vmaingraphicsscene.h"

template <class T> class QSharedPointer;

const QString VToolFlippingByLine::ToolType = QStringLiteral("flippingByLine");

//---------------------------------------------------------------------------------------------------------------------
void VToolFlippingByLine::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogFlippingByLine> dialogTool = qobject_cast<DialogFlippingByLine *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetFirstLinePointId(m_firstLinePointId);
    dialogTool->SetSecondLinePointId(m_secondLinePointId);
    dialogTool->SetNotes(m_notes);
    dialogTool->SetSourceObjects(source);

    SetDialogVisibilityGroupData(dialogTool);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolFlippingByLine::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                                 VContainer *data) -> VToolFlippingByLine *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogFlippingByLine> dialogTool = qobject_cast<DialogFlippingByLine *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolFlippingByLineInitData initData;
    initData.firstLinePointId = dialogTool->GetFirstLinePointId();
    initData.secondLinePointId = dialogTool->GetSecondLinePointId();
    initData.source = dialogTool->GetSourceObjects();
    initData.hasLinkedVisibilityGroup = dialogTool->HasLinkedVisibilityGroup();
    initData.visibilityGroupName = dialogTool->GetVisibilityGroupName();
    initData.visibilityGroupTags = dialogTool->GetVisibilityGroupTags();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();

    VToolFlippingByLine *operation = Create(initData);
    if (operation != nullptr)
    {
        operation->m_dialog = dialog;
    }
    return operation;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolFlippingByLine::Create(VToolFlippingByLineInitData initData) -> VToolFlippingByLine *
{
    const auto firstPoint = *initData.data->GeometricObject<VPointF>(initData.firstLinePointId);
    const auto fPoint = static_cast<QPointF>(firstPoint);

    const auto secondPoint = *initData.data->GeometricObject<VPointF>(initData.secondLinePointId);
    const auto sPoint = static_cast<QPointF>(secondPoint);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.destination.clear(); // Try to avoid mistake, value must be empty

        initData.id = initData.data->getNextId(); // Just reserve id for tool
    }
    else
    {
        PrepareNames(initData);
    }

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->AddVertex(initData.id, VNodeType::TOOL, initData.doc->PatternBlockMapper()->GetActiveId());

    CreateDestination(initData, fPoint, sPoint);

    patternGraph->AddEdge(initData.firstLinePointId, initData.id);
    patternGraph->AddEdge(initData.secondLinePointId, initData.id);

    for (const auto &object : std::as_const(initData.source))
    {
        patternGraph->AddEdge(object.id, initData.id);
    }

    for (const auto &object : std::as_const(initData.destination))
    {
        patternGraph->AddVertex(object.id, VNodeType::OBJECT, initData.doc->PatternBlockMapper()->GetActiveId());
        patternGraph->AddEdge(initData.id, object.id);
    }

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        if (initData.typeCreation == Source::FromGui && initData.hasLinkedVisibilityGroup)
        {
            VAbstractApplication::VApp()->getUndoStack()->beginMacro(tr("flipping by line"));
        }

        VAbstractTool::AddRecord(initData.id, Tool::FlippingByLine, initData.doc);
        auto *tool = new VToolFlippingByLine(initData);
        initData.scene->addItem(tool);
        InitOperationToolConnections(initData.scene, tool);
        VAbstractPattern::AddTool(initData.id, tool);

        if (initData.typeCreation == Source::FromGui && initData.hasLinkedVisibilityGroup)
        {
            VAbstractOperation::CreateVisibilityGroup(initData);
            VAbstractApplication::VApp()->getUndoStack()->endMacro();
        }

        return tool;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolFlippingByLine::FirstLinePointName() const -> QString
{
    return VAbstractTool::data.GetGObject(m_firstLinePointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolFlippingByLine::SecondLinePointName() const -> QString
{
    return VAbstractTool::data.GetGObject(m_secondLinePointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolFlippingByLine::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolFlippingByLine>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolFlippingByLine::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogFlippingByLine>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolFlippingByLine::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolFlippingByLine *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetObjects(SourceToObjects(source));
        visual->SetFirstLinePointId(m_firstLinePointId);
        visual->SetSecondLinePointId(m_secondLinePointId);
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolFlippingByLine::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogFlippingByLine> dialogTool = qobject_cast<DialogFlippingByLine *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrP1Line, QString().setNum(dialogTool->GetFirstLinePointId()));
    doc->SetAttribute(domElement, AttrP2Line, QString().setNum(dialogTool->GetSecondLinePointId()));
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrNotes,
                                         dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept -> bool { return notes.isEmpty(); });

    // We no longer need to handle suffix attribute here. The code can be removed.
    Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
    if (domElement.hasAttribute(AttrSuffix))
    {
        domElement.removeAttribute(AttrSuffix);
    }

    source = dialogTool->GetSourceObjects();
    SaveSourceDestination(domElement);

    // Save visibility data for later use
    SaveVisibilityGroupData(dialogTool);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolFlippingByLine::ReadToolAttributes(const QDomElement &domElement)
{
    VAbstractFlipping::ReadToolAttributes(domElement);

    m_firstLinePointId = VAbstractPattern::GetParametrUInt(domElement, AttrP1Line, NULL_ID_STR);
    m_secondLinePointId = VAbstractPattern::GetParametrUInt(domElement, AttrP2Line, NULL_ID_STR);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolFlippingByLine::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VAbstractFlipping::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrP1Line, QString().setNum(m_firstLinePointId));
    doc->SetAttribute(tag, AttrP2Line, QString().setNum(m_secondLinePointId));
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolFlippingByLine::MakeToolTip() const -> QString
{
    return QStringLiteral("<tr> <td><b>%1:</b> %2</td> </tr>"
                          "<tr> <td><b>%3:</b> %4</td> </tr>"
                          "%5")
        .arg(tr("First line point"),    // 1
             FirstLinePointName(),      // 2
             tr("Second line point"),   // 3
             SecondLinePointName(),     // 4
             VisibilityGroupToolTip()); // 5
}

//---------------------------------------------------------------------------------------------------------------------
void VToolFlippingByLine::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogFlippingByLine> dialogTool = qobject_cast<DialogFlippingByLine *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    ProcessOperationToolOptions(oldDomElement, newDomElement, dialogTool->GetSourceObjects());
}

//---------------------------------------------------------------------------------------------------------------------
VToolFlippingByLine::VToolFlippingByLine(const VToolFlippingByLineInitData &initData, QGraphicsItem *parent)
  : VAbstractFlipping(initData, parent),
    m_firstLinePointId(initData.firstLinePointId),
    m_secondLinePointId(initData.secondLinePointId)
{
    InitOperatedObjects();
    ToolCreation(initData.typeCreation);
}
