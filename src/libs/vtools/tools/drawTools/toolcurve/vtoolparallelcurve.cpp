/************************************************************************
 **
 **  @file   vtoolparallelcurve.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 11, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
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
#include "vtoolparallelcurve.h"

#include "../../../dialogs/tools/dialogparallelcurve.h"
#include "../../../visualization/path/vistoolparallelcurve.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vsplinepath.h"
#include "../vpatterndb/vformula.h"
#include "ifcdef.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

const QString VToolParallelCurve::ToolType = QStringLiteral("parallelCurve");

//---------------------------------------------------------------------------------------------------------------------
void VToolParallelCurve::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogParallelCurve> dialogTool = qobject_cast<DialogParallelCurve *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VSplinePath> splPath = VAbstractTool::data.GeometricObject<VSplinePath>(m_id);
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetOriginCurveId(OriginCurveId());
    dialogTool->SetFormulaWidth(m_formulaWidth);
    dialogTool->SetPenStyle(splPath->GetPenStyle());
    dialogTool->SetColor(splPath->GetColor());
    dialogTool->SetApproximationScale(splPath->GetApproximationScale());
    dialogTool->SetName(GetName());
    dialogTool->SetAliasSuffix(splPath->GetAliasSuffix());
    dialogTool->SetNotes(m_notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolParallelCurve::Create(const QPointer<DialogTool> &dialog,
                                VMainGraphicsScene *scene,
                                VAbstractPattern *doc,
                                VContainer *data) -> VToolParallelCurve *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogParallelCurve> dialogTool = qobject_cast<DialogParallelCurve *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolParallelCurveInitData initData;
    initData.originCurveId = dialogTool->GetOriginCurveId();
    initData.formulaWidth = dialogTool->GetFormulaWidth();
    initData.name = dialogTool->GetName();
    initData.color = dialogTool->GetColor();
    initData.penStyle = dialogTool->GetPenStyle();
    initData.approximationScale = dialogTool->GetApproximationScale();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();
    initData.aliasSuffix = dialogTool->GetAliasSuffix();

    VToolParallelCurve *curve = Create(initData);
    if (curve != nullptr)
    {
        curve->m_dialog = dialog;
    }
    return curve;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolParallelCurve::Create(VToolParallelCurveInitData &initData) -> VToolParallelCurve *
{
    qreal const calcWidth = VAbstractValApplication::VApp()->toPixel(
        CheckFormula(initData.id, initData.formulaWidth, initData.data));

    const QSharedPointer<VAbstractCurve> curve = initData.data->GeometricObject<VAbstractCurve>(initData.originCurveId);

    VSplinePath splPath = curve->Offset(calcWidth, initData.name);
    splPath.SetColor(initData.color);
    splPath.SetPenStyle(initData.penStyle);
    splPath.SetApproximationScale(initData.approximationScale);
    splPath.SetAliasSuffix(initData.aliasSuffix);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->AddGObject(new VSplinePath(splPath));
    }
    else
    {
        initData.data->UpdateGObject(initData.id, new VSplinePath(splPath));
    }

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->AddVertex(initData.id, VNodeType::TOOL, initData.doc->PatternBlockMapper()->GetActiveId());

    const auto varData = initData.data->DataDependencyVariables();
    initData.doc->FindFormulaDependencies(initData.formulaWidth, initData.id, varData);

    initData.data->AddSpline(initData.data->GeometricObject<VAbstractBezier>(initData.id), initData.id);

    patternGraph->AddEdge(initData.originCurveId, initData.id);

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::ParallelCurve, initData.doc);
        auto *path = new VToolParallelCurve(initData);
        initData.scene->addItem(path);
        InitSplinePathToolConnections(initData.scene, path);
        VAbstractPattern::AddTool(initData.id, path);
        return path;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolParallelCurve::GetFormulaWidth() const -> VFormula
{
    VFormula width(m_formulaWidth, getData());
    width.setToolId(m_id);
    width.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    width.Eval();
    return width;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolParallelCurve::SetFormulaWidth(const VFormula &value)
{
    if (!value.error())
    {
        m_formulaWidth = value.GetFormula(FormulaType::FromUser);

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolParallelCurve::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolParallelCurve>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolParallelCurve::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    Q_UNUSED(id)
    try
    {
        ContextMenu<DialogParallelCurve>(event);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolParallelCurve::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QPointer<DialogParallelCurve> const dialogTool = qobject_cast<DialogParallelCurve *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrCurve, dialogTool->GetOriginCurveId());
    doc->SetAttribute(domElement, AttrWidth, dialogTool->GetFormulaWidth());
    doc->SetAttribute(domElement, AttrName, dialogTool->GetName());
    doc->SetAttribute(domElement, AttrColor, dialogTool->GetColor());
    doc->SetAttribute(domElement, AttrPenStyle, dialogTool->GetPenStyle());
    doc->SetAttribute(domElement, AttrAScale, dialogTool->GetApproximationScale());
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrAlias,
                                         dialogTool->GetAliasSuffix(),
                                         [](const QString &suffix) noexcept -> bool { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement,
                                         AttrNotes,
                                         dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept -> bool { return notes.isEmpty(); });

    // We no longer need to handle suffix attribute here. The code can be removed.
    Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
    if (!dialogTool->GetName().isEmpty() && domElement.hasAttribute(AttrSuffix))
    {
        domElement.removeAttribute(AttrSuffix);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolParallelCurve::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolAbstractOffsetCurve::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrWidth, m_formulaWidth);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolParallelCurve::SetVisualization()
{
    if (not vis.isNull())
    {
        QSharedPointer<VAbstractCurve> const curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
        auto *visual = qobject_cast<VisToolParallelCurve *>(vis);
        SCASSERT(visual != nullptr)

        const bool osSeparator = VAbstractApplication::VApp()->Settings()->GetOsSeparator();
        const VTranslateVars *trVars = VAbstractApplication::VApp()->TrVars();

        visual->SetCurveId(OriginCurveId());
        visual->SetOffsetWidth(trVars->FormulaToUser(m_formulaWidth, osSeparator));
        visual->SetLineStyle(LineStyleToPenStyle(curve->GetPenStyle()));
        visual->SetApproximationScale(curve->GetApproximationScale());
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolParallelCurve::ReadToolAttributes(const QDomElement &domElement)
{
    VToolAbstractOffsetCurve::ReadToolAttributes(domElement);

    m_formulaWidth = VDomDocument::GetParametrString(domElement, AttrWidth, QChar('0'));
}

//---------------------------------------------------------------------------------------------------------------------
void VToolParallelCurve::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    ProcessOffsetCurveToolOptions(oldDomElement, newDomElement, GatherToolChanges());
}

//---------------------------------------------------------------------------------------------------------------------
VToolParallelCurve::VToolParallelCurve(const VToolParallelCurveInitData &initData, QGraphicsItem *parent)
  : VToolAbstractOffsetCurve(initData.doc,
                             initData.data,
                             initData.id,
                             initData.originCurveId,
                             initData.name,
                             initData.notes,
                             parent),
    m_formulaWidth(initData.formulaWidth)
{
    SetSceneType(SceneObject::SplinePath);

    this->setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus

    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolParallelCurve::GatherToolChanges() const -> VToolAbstractOffsetCurve::ToolChanges
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogParallelCurve> dialogTool = qobject_cast<DialogParallelCurve *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    return {.oldName = GetName(),
            .newName = dialogTool->GetName(),
            .oldAliasSuffix = GetAliasSuffix(),
            .newAliasSuffix = dialogTool->GetAliasSuffix()};
}
