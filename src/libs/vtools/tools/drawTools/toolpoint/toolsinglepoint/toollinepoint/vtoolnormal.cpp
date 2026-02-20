/************************************************************************
 **
 **  @file   vtoolnormal.cpp
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

#include "vtoolnormal.h"

#include <QLineF>
#include <QSharedPointer>

#include "../../../../../dialogs/tools/dialognormal.h"
#include "../../../../../dialogs/tools/dialogtool.h"
#include "../../../../../visualization/line/vistoolnormal.h"
#include "../../../../../visualization/visualization.h"
#include "../../../../vabstracttool.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/exception/vexception.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vtoollinepoint.h"
#include "xml/vpatterngraph.h"

template <class T> class QSharedPointer;

const QString VToolNormal::ToolType = QStringLiteral("normal");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolNormal constructor.
 * @param initData init data.
 * @param parent parent object.
 */
VToolNormal::VToolNormal(const VToolNormalInitData &initData, QGraphicsItem *parent)
    :VToolLinePoint(initData.doc, initData.data, initData.id, initData.typeLine, initData.lineColor, initData.formula,
                    initData.firstPointId, initData.angle, initData.notes, parent),
    secondPointId(initData.secondPointId)
{

    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolNormal::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogNormal> dialogTool = qobject_cast<DialogNormal *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetTypeLine(m_lineType);
    dialogTool->SetLineColor(lineColor);
    dialogTool->SetFormula(formulaLength);
    dialogTool->SetAngle(angle);
    dialogTool->SetFirstPointId(basePointId);
    dialogTool->SetSecondPointId(secondPointId);
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
 */
auto VToolNormal::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                         VContainer *data) -> VToolNormal *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogNormal> dialogTool = qobject_cast<DialogNormal *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolNormalInitData initData;
    initData.formula = dialogTool->GetFormula();
    initData.firstPointId = dialogTool->GetFirstPointId();
    initData.secondPointId = dialogTool->GetSecondPointId();
    initData.angle = dialogTool->GetAngle();
    initData.typeLine = dialogTool->GetTypeLine();
    initData.lineColor = dialogTool->GetLineColor();
    initData.name = dialogTool->GetPointName();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();

    VToolNormal *point = Create(initData);
    if (point != nullptr)
    {
        point->m_dialog = dialog;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool.
 * @param initData init data.
 */
auto VToolNormal::Create(VToolNormalInitData &initData) -> VToolNormal *
{
    const QSharedPointer<VPointF> firstPoint = initData.data->GeometricObject<VPointF>(initData.firstPointId);
    const QSharedPointer<VPointF> secondPoint = initData.data->GeometricObject<VPointF>(initData.secondPointId);

    const qreal result = CheckFormula(initData.id, initData.formula, initData.data);

    QPointF const fPoint = VToolNormal::FindPoint(static_cast<QPointF>(*firstPoint), static_cast<QPointF>(*secondPoint),
                                                  VAbstractValApplication::VApp()->toPixel(result), initData.angle);

    auto *p = new VPointF(fPoint, initData.name, initData.mx, initData.my);
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

    const auto varData = initData.data->DataDependencyVariables();
    initData.doc->FindFormulaDependencies(initData.formula, initData.id, varData);

    initData.data->AddLine(initData.firstPointId, initData.id);

    patternGraph->AddEdge(initData.firstPointId, initData.id);
    patternGraph->AddEdge(initData.secondPointId, initData.id);

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::Normal, initData.doc);
        auto *point = new VToolNormal(initData);
        initData.scene->addItem(point);
        InitToolConnections(initData.scene, point);
        VAbstractPattern::AddTool(initData.id, point);
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FindPoint return normal point.
 * @param firstPoint first line point.
 * @param secondPoint second line point.
 * @param length normal length.
 * @param angle additional angle.
 * @return normal point.
 */
auto VToolNormal::FindPoint(const QPointF &firstPoint, const QPointF &secondPoint, qreal length, qreal angle) -> QPointF
{
    QLineF const line(firstPoint, secondPoint);
    QLineF normal = line.normalVector();
    normal.setAngle(normal.angle()+angle);
    normal.setLength(length);
    return normal.p2();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolNormal::SecondPointName() const -> QString
{
    return VAbstractTool::data.GetGObject(secondPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolNormal::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogNormal> dialogTool = qobject_cast<DialogNormal *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrName, dialogTool->GetPointName());
    doc->SetAttribute(domElement, AttrTypeLine, dialogTool->GetTypeLine());
    doc->SetAttribute(domElement, AttrLineColor, dialogTool->GetLineColor());
    doc->SetAttribute(domElement, AttrLength, dialogTool->GetFormula());
    doc->SetAttribute(domElement, AttrAngle, QString().setNum(dialogTool->GetAngle()));
    doc->SetAttribute(domElement, AttrFirstPoint, QString().setNum(dialogTool->GetFirstPointId()));
    doc->SetAttribute(domElement, AttrSecondPoint, QString().setNum(dialogTool->GetSecondPointId()));
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrNotes,
                                         dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept -> bool { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolNormal::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolLinePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrLength, formulaLength);
    doc->SetAttribute(tag, AttrAngle, angle);
    doc->SetAttribute(tag, AttrFirstPoint, basePointId);
    doc->SetAttribute(tag, AttrSecondPoint, secondPointId);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolNormal::ReadToolAttributes(const QDomElement &domElement)
{
    VToolLinePoint::ReadToolAttributes(domElement);

    m_lineType = VDomDocument::GetParametrString(domElement, AttrTypeLine, TypeLineLine);
    lineColor = VDomDocument::GetParametrString(domElement, AttrLineColor, ColorBlack);
    formulaLength = VDomDocument::GetParametrString(domElement, AttrLength, QString());
    basePointId = VAbstractPattern::GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
    secondPointId = VAbstractPattern::GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);
    angle = VDomDocument::GetParametrDouble(domElement, AttrAngle, QChar('0'));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolNormal::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolNormal *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetPoint1Id(basePointId);
        visual->SetPoint2Id(secondPointId);
        visual->SetLength(VAbstractApplication::VApp()->TrVars()
                          ->FormulaToUser(formulaLength, VAbstractApplication::VApp()->Settings()->GetOsSeparator()));
        visual->SetAngle(angle);
        visual->SetLineStyle(LineStyleToPenStyle(m_lineType));
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolNormal::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogNormal> dialogTool = qobject_cast<DialogNormal *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    ProcessLinePointToolOptions(oldDomElement,
                                newDomElement,
                                dialogTool->GetPointName(),
                                VAbstractTool::data.GetGObject(dialogTool->GetFirstPointId())->name());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolNormal::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolNormal>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolNormal::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogNormal>(event, id);
    }
    catch(const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return;//Leave this method immediately!!!
    }
}
