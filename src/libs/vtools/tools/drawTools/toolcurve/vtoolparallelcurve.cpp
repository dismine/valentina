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
    dialogTool->SetOriginCurveId(m_originCurveId);
    dialogTool->SetFormulaWidth(m_formulaWidth);
    dialogTool->SetPenStyle(splPath->GetPenStyle());
    dialogTool->SetColor(splPath->GetColor());
    dialogTool->SetApproximationScale(splPath->GetApproximationScale());
    dialogTool->SetSuffix(m_suffix);
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
    initData.suffix = dialogTool->GetSuffix();
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

    VSplinePath splPath = curve->Offset(calcWidth, initData.suffix);
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
        initData.doc->IncrementReferens(curve->getIdTool());
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
auto VToolParallelCurve::GetApproximationScale() const -> qreal
{
    QSharedPointer<VAbstractCurve> const curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    SCASSERT(curve.isNull() == false)

    return curve->GetApproximationScale();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolParallelCurve::SetApproximationScale(qreal value)
{
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    QSharedPointer<VAbstractCurve> const curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    curve->SetApproximationScale(value);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolParallelCurve::GetSuffix() const -> QString
{
    return m_suffix;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolParallelCurve::SetSuffix(QString suffix)
{
    m_suffix = suffix;
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolParallelCurve::CurveName() const -> QString
{
    return VAbstractTool::data.GetGObject(m_originCurveId)->ObjectName();
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
void VToolParallelCurve::RemoveReferens()
{
    const auto curve = VAbstractTool::data.GetGObject(m_originCurveId);
    doc->DecrementReferens(curve->getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolParallelCurve::SaveDialog(QDomElement &domElement,
                                    QList<quint32> &oldDependencies,
                                    QList<quint32> &newDependencies)
{
    SCASSERT(not m_dialog.isNull())
    QPointer<DialogParallelCurve> const dialogTool = qobject_cast<DialogParallelCurve *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    AddDependence(oldDependencies, m_originCurveId);
    AddDependence(newDependencies, dialogTool->GetOriginCurveId());

    doc->SetAttribute(domElement, AttrCurve, dialogTool->GetOriginCurveId());
    doc->SetAttribute(domElement, AttrWidth, dialogTool->GetFormulaWidth());
    doc->SetAttribute(domElement, AttrSuffix, dialogTool->GetSuffix());
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
}

//---------------------------------------------------------------------------------------------------------------------
void VToolParallelCurve::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VAbstractSpline::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrCurve, m_originCurveId);
    doc->SetAttribute(tag, AttrWidth, m_formulaWidth);
    doc->SetAttribute(tag, AttrSuffix, m_suffix);
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

        visual->SetCurveId(m_originCurveId);
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
    VAbstractSpline::ReadToolAttributes(domElement);

    m_originCurveId = VDomDocument::GetParametrUInt(domElement, AttrCurve, NULL_ID_STR);
    m_suffix = VDomDocument::GetParametrString(domElement, AttrSuffix);
    m_formulaWidth = VDomDocument::GetParametrString(domElement, AttrWidth, QChar('0'));
}

//---------------------------------------------------------------------------------------------------------------------
VToolParallelCurve::VToolParallelCurve(const VToolParallelCurveInitData &initData, QGraphicsItem *parent)
  : VAbstractSpline(initData.doc, initData.data, initData.id, initData.notes, parent),
    m_formulaWidth(initData.formulaWidth),
    m_originCurveId(initData.originCurveId),
    m_suffix(initData.suffix)
{
    SetSceneType(SceneObject::SplinePath);

    this->setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus

    ToolCreation(initData.typeCreation);
}
