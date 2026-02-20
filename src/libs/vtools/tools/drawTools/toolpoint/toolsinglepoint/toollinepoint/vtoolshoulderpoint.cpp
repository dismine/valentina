/************************************************************************
 **
 **  @file   vtoolshoulderpoint.cpp
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

#include "vtoolshoulderpoint.h"

#include <QLineF>
#include <QMessageLogger>
#include <QSharedPointer>
#include <QUndoStack>
#include <QtDebug>

#include "../../../../../dialogs/tools/dialogshoulderpoint.h"
#include "../../../../../dialogs/tools/dialogtool.h"
#include "../../../../../undocommands/renameobject.h"
#include "../../../../../undocommands/savetooloptions.h"
#include "../../../../../visualization/line/vistoolshoulderpoint.h"
#include "../../../../../visualization/visualization.h"
#include "../../../../vabstracttool.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/exception/vexception.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vtoollinepoint.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

template <class T> class QSharedPointer;

const QString VToolShoulderPoint::ToolType = QStringLiteral("shoulder");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolShoulderPoint constructor.
 * @param initData init data.
 * @param parent parent object.
 */
VToolShoulderPoint::VToolShoulderPoint(const VToolShoulderPointInitData &initData, QGraphicsItem *parent)
  : VToolLinePoint(initData.doc, initData.data, initData.id, initData.typeLine, initData.lineColor, initData.formula,
                   initData.p1Line, 0, initData.notes, parent),
    p2Line(initData.p2Line),
    pShoulder(initData.pShoulder)
{
    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolShoulderPoint::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogShoulderPoint> dialogTool = qobject_cast<DialogShoulderPoint *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetTypeLine(m_lineType);
    dialogTool->SetLineColor(lineColor);
    dialogTool->SetFormula(formulaLength);
    dialogTool->SetP1Line(basePointId);
    dialogTool->SetP2Line(p2Line);
    dialogTool->SetP3(pShoulder);
    dialogTool->SetPointName(p->name());
    dialogTool->SetNotes(m_notes);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FindPoint find point.
 * @param p1Line first line point.
 * @param p2Line second line point.
 * @param pShoulder shoulder point.
 * @param length length form shoulder point to our.
 * @return point.
 */
auto VToolShoulderPoint::FindPoint(const QPointF &p1Line, const QPointF &p2Line, const QPointF &pShoulder, qreal length)
    -> QPointF
{
    QPointF shoulderPoint = p2Line; // Position if result was not found
    if (length <= 0)
    {
        return shoulderPoint;
    }

    auto line = QLineF(p1Line, p2Line);
    const qreal baseLength = line.length();
    const int baseAngle = qRound(line.angle());
    line.setLength(length * 2);

    QPointF p1;
    QPointF p2;

    const qint32 res = VGObject::LineIntersectCircle(pShoulder, length, line, p1, p2);

    if (res == 1 || res == 2)
    {
        const auto line1 = QLineF(p1Line, p1);
        const auto line2 = QLineF(p1Line, p2);
        if (line1.length() > baseLength && baseAngle == qRound(line1.angle()))
        {
            shoulderPoint = p1;
        }
        else if (res == 2 && line2.length() > baseLength && baseAngle == qRound(line2.angle()))
        {
            shoulderPoint = p2;
        }
    }

    return shoulderPoint;
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
auto VToolShoulderPoint::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                                VContainer *data) -> VToolShoulderPoint *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogShoulderPoint> dialogTool = qobject_cast<DialogShoulderPoint *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolShoulderPointInitData initData;
    initData.formula = dialogTool->GetFormula();
    initData.p1Line = dialogTool->GetP1Line();
    initData.p2Line = dialogTool->GetP2Line();
    initData.pShoulder = dialogTool->GetP3();
    initData.typeLine = dialogTool->GetTypeLine();
    initData.lineColor = dialogTool->GetLineColor();
    initData.name = dialogTool->GetPointName();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();

    VToolShoulderPoint *point = Create(initData);
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
 * @return the created tool
 */
auto VToolShoulderPoint::Create(VToolShoulderPointInitData &initData) -> VToolShoulderPoint *
{
    const QSharedPointer<VPointF> firstPoint = initData.data->GeometricObject<VPointF>(initData.p1Line);
    const QSharedPointer<VPointF> secondPoint = initData.data->GeometricObject<VPointF>(initData.p2Line);
    const QSharedPointer<VPointF> shoulderPoint = initData.data->GeometricObject<VPointF>(initData.pShoulder);

    const qreal result = CheckFormula(initData.id, initData.formula, initData.data);

    QPointF const fPoint = VToolShoulderPoint::FindPoint(
        static_cast<QPointF>(*firstPoint), static_cast<QPointF>(*secondPoint), static_cast<QPointF>(*shoulderPoint),
        VAbstractValApplication::VApp()->toPixel(result));

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

    initData.data->AddLine(initData.p1Line, initData.id);
    initData.data->AddLine(initData.p2Line, initData.id);

    patternGraph->AddEdge(initData.p1Line, initData.id);
    patternGraph->AddEdge(initData.p2Line, initData.id);
    patternGraph->AddEdge(initData.pShoulder, initData.id);

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::ShoulderPoint, initData.doc);
        auto *point = new VToolShoulderPoint(initData);
        initData.scene->addItem(point);
        InitToolConnections(initData.scene, point);
        VAbstractPattern::AddTool(initData.id, point);
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolShoulderPoint::SecondPointName() const -> QString
{
    return VAbstractTool::data.GetGObject(p2Line)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolShoulderPoint::ShoulderPointName() const -> QString
{
    return VAbstractTool::data.GetGObject(pShoulder)->name();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolShoulderPoint::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogShoulderPoint> dialogTool = qobject_cast<DialogShoulderPoint *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrName, dialogTool->GetPointName());
    doc->SetAttribute(domElement, AttrTypeLine, dialogTool->GetTypeLine());
    doc->SetAttribute(domElement, AttrLineColor, dialogTool->GetLineColor());
    doc->SetAttribute(domElement, AttrLength, dialogTool->GetFormula());
    doc->SetAttribute(domElement, AttrP1Line, QString().setNum(dialogTool->GetP1Line()));
    doc->SetAttribute(domElement, AttrP2Line, QString().setNum(dialogTool->GetP2Line()));
    doc->SetAttribute(domElement, AttrPShoulder, QString().setNum(dialogTool->GetP3()));

    const QString notes = dialogTool->GetNotes();
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrNotes,
                                         notes,
                                         [](const QString &notes) noexcept -> bool { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolShoulderPoint::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolLinePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrLength, formulaLength);
    doc->SetAttribute(tag, AttrP1Line, basePointId);
    doc->SetAttribute(tag, AttrP2Line, p2Line);
    doc->SetAttribute(tag, AttrPShoulder, pShoulder);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolShoulderPoint::ReadToolAttributes(const QDomElement &domElement)
{
    VToolLinePoint::ReadToolAttributes(domElement);

    m_lineType = VDomDocument::GetParametrString(domElement, AttrTypeLine, TypeLineLine);
    lineColor = VDomDocument::GetParametrString(domElement, AttrLineColor, ColorBlack);
    formulaLength = VDomDocument::GetParametrString(domElement, AttrLength, QString());
    basePointId = VAbstractPattern::GetParametrUInt(domElement, AttrP1Line, NULL_ID_STR);
    p2Line = VAbstractPattern::GetParametrUInt(domElement, AttrP2Line, NULL_ID_STR);
    pShoulder = VAbstractPattern::GetParametrUInt(domElement, AttrPShoulder, NULL_ID_STR);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolShoulderPoint::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolShoulderPoint *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetLineP1Id(pShoulder);
        visual->SetLineP1Id(basePointId);
        visual->SetLineP2Id(p2Line);
        visual->SetLength(VAbstractApplication::VApp()->TrVars()->FormulaToUser(
            formulaLength, VAbstractApplication::VApp()->Settings()->GetOsSeparator()));
        visual->SetLineStyle(LineStyleToPenStyle(m_lineType));
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolShoulderPoint::MakeToolTip() const -> QString
{
    const QSharedPointer<VPointF> first = VAbstractTool::data.GeometricObject<VPointF>(basePointId);
    const QSharedPointer<VPointF> second = VAbstractTool::data.GeometricObject<VPointF>(p2Line);
    const QSharedPointer<VPointF> current = VAbstractTool::data.GeometricObject<VPointF>(m_id);

    const QLineF firstToCur(static_cast<QPointF>(*first), static_cast<QPointF>(*current));
    const QLineF secondToCur(static_cast<QPointF>(*second), static_cast<QPointF>(*current));

    const QString toolTip = u"<table>"
                            u"<tr> <td><b>%8:</b> %9</td> </tr>"
                            u"<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                            u"<tr> <td><b>%4:</b> %5°</td> </tr>"
                            u"<tr> <td><b>%6:</b> %7 %3</td> </tr>"
                            u"</table>"_s.arg(tr("Length"))
                                .arg(VAbstractValApplication::VApp()->fromPixel(firstToCur.length()))
                                .arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true), tr("Angle"))
                                .arg(firstToCur.angle())
                                .arg(u"%1->%2"_s.arg(second->name(), current->name()))
                                .arg(VAbstractValApplication::VApp()->fromPixel(secondToCur.length()))
                                .arg(tr("Label"), current->name());
    return toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolShoulderPoint::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogShoulderPoint> dialogTool = qobject_cast<DialogShoulderPoint *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    const QString oldLabel = VAbstractTool::data.GetGObject(m_id)->name();
    const QString newLabel = dialogTool->GetPointName();

    const QString newBasePointLabel = VAbstractTool::data.GetGObject(dialogTool->GetP1Line())->name();
    const QString oldBasePointLabel = BasePointName();

    const QString newSecondPointLabel = VAbstractTool::data.GetGObject(dialogTool->GetP2Line())->name();
    const QString oldSecondPointLabel = SecondPointName();

    if (oldBasePointLabel == newBasePointLabel && oldSecondPointLabel == newSecondPointLabel && oldLabel == newLabel)
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
        if (oldLabel == newLabel && oldSecondPointLabel == newSecondPointLabel)
        {
            connect(renamePair, &RenamePair::NeedLiteParsing, doc, &VAbstractPattern::LiteParseTree);
        }
    }

    if (oldSecondPointLabel != newSecondPointLabel)
    {
        auto *renamePair = RenamePair::CreateForLine(std::make_pair(oldSecondPointLabel, oldLabel),
                                                     std::make_pair(newSecondPointLabel, oldLabel),
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
void VToolShoulderPoint::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogShoulderPoint>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolShoulderPoint::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolShoulderPoint>(show);
}
