/************************************************************************
 **
 **  @file   vtoolgraduatedcurve.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   3 12, 2025
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
#include "vtoolgraduatedcurve.h"

#include "../../../dialogs/tools/dialoggraduatedcurve.h"
#include "../../../visualization/path/vistoolgraduatedcurve.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vpatternblockmapper.h"
#include "../ifc/xml/vpatternconverter.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vsplinepath.h"
#include "../vmisc/compatibility.h"
#include "../vpatterndb/variables/vincrement.h"

using namespace Qt::Literals::StringLiterals;

const QString VToolGraduatedCurve::ToolType = QStringLiteral("graduatedCurve");

//---------------------------------------------------------------------------------------------------------------------
void VToolGraduatedCurve::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogGraduatedCurve> dialogTool = qobject_cast<DialogGraduatedCurve *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VSplinePath> splPath = VAbstractTool::data.GeometricObject<VSplinePath>(m_id);
    dialogTool->CheckDependencyTreeComplete();
    dialogTool->SetOriginCurveId(OriginCurveId());
    dialogTool->SetOffsets(m_offsets);
    dialogTool->SetPenStyle(splPath->GetPenStyle());
    dialogTool->SetColor(splPath->GetColor());
    dialogTool->SetApproximationScale(splPath->GetApproximationScale());
    dialogTool->SetName(GetName());
    dialogTool->SetAliasSuffix(splPath->GetAliasSuffix());
    dialogTool->SetNotes(m_notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolGraduatedCurve::Create(const QPointer<DialogTool> &dialog,
                                 VMainGraphicsScene *scene,
                                 VAbstractPattern *doc,
                                 VContainer *data) -> VToolGraduatedCurve *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogGraduatedCurve> dialogTool = qobject_cast<DialogGraduatedCurve *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolGraduatedCurveInitData initData;
    initData.originCurveId = dialogTool->GetOriginCurveId();
    initData.offsets = dialogTool->GetOffsets();
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

    VToolGraduatedCurve *curve = Create(initData);
    if (curve != nullptr)
    {
        curve->m_dialog = dialog;
    }
    return curve;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolGraduatedCurve::Create(VToolGraduatedCurveInitData &initData) -> VToolGraduatedCurve *
{
    QVector<qreal> widths;
    widths.reserve(initData.offsets.size());
    {
        VContainer localData = *initData.data;

        for (auto &offset : initData.offsets)
        {
            qreal const calcWidth = VAbstractValApplication::VApp()->toPixel(
                CheckFormula(initData.id, offset.formula, &localData));

            auto *offsetVal = new VIncrement(&localData, offset.name);
            offsetVal->SetFormula(calcWidth, offset.formula, true);
            offsetVal->SetDescription(offset.description.isEmpty() ? tr("Offset %1").arg(offset.name)
                                                                   : offset.description);

            localData.AddVariable(offsetVal);

            widths.append(calcWidth);
        }
    }

    const QSharedPointer<VAbstractCurve> curve = initData.data->GeometricObject<VAbstractCurve>(initData.originCurveId);

    VSplinePath splPath = curve->Outline(widths, initData.name);
    splPath.SetColor(initData.color);
    splPath.SetPenStyle(initData.penStyle);
    splPath.SetApproximationScale(initData.approximationScale);
    splPath.SetAliasSuffix(initData.aliasSuffix);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->AddGObject(new VSplinePath(splPath));
        initData.data->AddSpline(initData.data->GeometricObject<VAbstractBezier>(initData.id), initData.id);
    }
    else
    {
        initData.data->UpdateGObject(initData.id, new VSplinePath(splPath));
    }

    VPatternGraph *patternGraph = initData.doc->PatternGraph();
    SCASSERT(patternGraph != nullptr)

    patternGraph->AddVertex(initData.id, VNodeType::TOOL, initData.doc->PatternBlockMapper()->GetActiveId());

    const auto varData = initData.data->DataDependencyVariables();
    for (const auto &offset : std::as_const(initData.offsets))
    {
        initData.doc->FindFormulaDependencies(offset.formula, initData.id, varData);
    }

    initData.data->AddSpline(initData.data->GeometricObject<VAbstractBezier>(initData.id), initData.id);

    patternGraph->AddEdge(initData.originCurveId, initData.id);

    if (initData.typeCreation != Source::FromGui && initData.parse != Document::FullParse)
    {
        initData.doc->UpdateToolData(initData.id, initData.data);
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::GraduatedCurve, initData.doc);
        auto *path = new VToolGraduatedCurve(initData);
        initData.scene->addItem(path);
        InitSplinePathToolConnections(initData.scene, path);
        VAbstractPattern::AddTool(initData.id, path);
        return path;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolGraduatedCurve::GetGraduatedOffsets() const -> QVector<VGraduatedCurveOffset>
{
    QVector<VGraduatedCurveOffset> widths;
    widths.reserve(m_offsets.size());

    VContainer localData = VAbstractTool::data;

    for (const auto &offset : m_offsets)
    {
        VFormula width(offset.formula, &localData);
        width.setToolId(m_id);
        width.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
        width.Eval();

        auto *offsetVal = new VIncrement(&localData, offset.name);
        offsetVal->SetFormula(width.getDoubleValue(), offset.formula, width.error());

        localData.AddVariable(offsetVal);

        widths.append({.name = offset.name, .offset = width, .description = offset.description});
    }

    return widths;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolGraduatedCurve::SetGraduatedOffsets(const QVector<VGraduatedCurveOffset> &offsets)
{
    m_offsets.clear();
    m_offsets.reserve(offsets.size());

    for (const auto &offset : offsets)
    {
        QString formula = "0"_L1;
        if (!offset.offset.error())
        {
            formula = offset.offset.GetFormula(FormulaType::FromUser);
        }

        m_offsets.append({.name = offset.name, .formula = formula, .description = offset.description});
    }

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolGraduatedCurve::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolGraduatedCurve>(show);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolGraduatedCurve::ExtractOffsetData(const QDomElement &domElement) -> QVector<VRawGraduatedCurveOffset>
{
    QVector<VRawGraduatedCurveOffset> offsets;
    const QDomNodeList nodeList = domElement.childNodes();
    offsets.reserve(nodeList.size());

    QDOM_LOOP(nodeList, i)
    {
        if (const QDomElement element = QDOM_ELEMENT(nodeList, i).toElement();
            not element.isNull() && element.tagName() == VAbstractPattern::TagOffset)
        {
            offsets.append({.name = VDomDocument::GetParametrString(element, AttrName),
                            .formula = VDomDocument::GetParametrString(element, AttrWidth, QChar('0')),
                            .description = VDomDocument::GetParametrEmptyString(element, AttrDescription)});
        }
    }

    return offsets;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolGraduatedCurve::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    Q_UNUSED(id)
    try
    {
        ContextMenu<DialogGraduatedCurve>(event);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolGraduatedCurve::SaveDialog(QDomElement &domElement)
{
    SCASSERT(not m_dialog.isNull())
    QPointer<DialogGraduatedCurve> const dialogTool = qobject_cast<DialogGraduatedCurve *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    doc->SetAttribute(domElement, AttrCurve, dialogTool->GetOriginCurveId());
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

    QVector<VRawGraduatedCurveOffset> const offsets = dialogTool->GetOffsets();
    UpdateOffsets(domElement, offsets);

    // We no longer need to handle suffix attribute here. The code can be removed.
    Q_STATIC_ASSERT(VPatternConverter::PatternMinVer < FormatVersion(1, 1, 1));
    if (!dialogTool->GetName().isEmpty() && domElement.hasAttribute(AttrSuffix))
    {
        domElement.removeAttribute(AttrSuffix);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolGraduatedCurve::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolAbstractOffsetCurve::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);

    UpdateOffsets(tag, m_offsets);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolGraduatedCurve::SetVisualization()
{
    if (not vis.isNull())
    {
        QSharedPointer<VAbstractCurve> const curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
        auto *visual = qobject_cast<VisToolGraduatedCurve *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetCurveId(OriginCurveId());

        const bool osSeparator = VAbstractApplication::VApp()->Settings()->GetOsSeparator();
        const VTranslateVars *trVars = VAbstractApplication::VApp()->TrVars();

        QVector<VRawGraduatedCurveOffset> toUserOffsets;
        toUserOffsets.reserve(m_offsets.size());
        for (const auto &offset : std::as_const(m_offsets))
        {
            toUserOffsets.append({.name = offset.name, .formula = trVars->FormulaToUser(offset.formula, osSeparator)});
        }
        visual->SetOffsets(toUserOffsets);

        visual->SetLineStyle(LineStyleToPenStyle(curve->GetPenStyle()));
        visual->SetApproximationScale(curve->GetApproximationScale());
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolGraduatedCurve::ReadToolAttributes(const QDomElement &domElement)
{
    VToolAbstractOffsetCurve::ReadToolAttributes(domElement);

    m_offsets = ExtractOffsetData(domElement);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolGraduatedCurve::ApplyToolOptions(const QDomElement &oldDomElement, const QDomElement &newDomElement)
{
    ProcessOffsetCurveToolOptions(oldDomElement, newDomElement, GatherToolChanges());
}

//---------------------------------------------------------------------------------------------------------------------
VToolGraduatedCurve::VToolGraduatedCurve(const VToolGraduatedCurveInitData &initData, QGraphicsItem *parent)
  : VToolAbstractOffsetCurve(initData.doc,
                             initData.data,
                             initData.id,
                             initData.originCurveId,
                             initData.name,
                             initData.notes,
                             parent),
    m_offsets(initData.offsets)
{
    SetSceneType(SceneObject::SplinePath);

    this->setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus

    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolGraduatedCurve::UpdateOffsets(QDomElement &tag, const QVector<VRawGraduatedCurveOffset> &offsets)
{
    VDomDocument::RemoveAllChildren(tag);
    for (const auto &offset : offsets)
    {
        QDomElement offsetTag = doc->createElement(AttrOffset);

        doc->SetAttribute(offsetTag, AttrName, offset.name);
        doc->SetAttribute(offsetTag, AttrWidth, offset.formula);
        doc->SetAttribute(offsetTag, AttrDescription, offset.description);

        tag.appendChild(offsetTag);
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolGraduatedCurve::GatherToolChanges() const -> VToolAbstractOffsetCurve::ToolChanges
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogGraduatedCurve> dialogTool = qobject_cast<DialogGraduatedCurve *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    return {.oldName = GetName(),
            .newName = dialogTool->GetName(),
            .oldAliasSuffix = GetAliasSuffix(),
            .newAliasSuffix = dialogTool->GetAliasSuffix()};
}
