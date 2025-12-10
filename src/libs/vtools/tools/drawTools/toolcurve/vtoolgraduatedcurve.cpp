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
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vsplinepath.h"
#include "../vpatterndb/variables/vincrement.h"
#include "ifcdef.h"
#include <utility>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

const QString VToolGraduatedCurve::ToolType = QStringLiteral("graduatedCurve");

//---------------------------------------------------------------------------------------------------------------------
void VToolGraduatedCurve::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogGraduatedCurve> dialogTool = qobject_cast<DialogGraduatedCurve *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VSplinePath> splPath = VAbstractTool::data.GeometricObject<VSplinePath>(m_id);
    dialogTool->SetOriginCurveId(m_originCurveId);
    dialogTool->SetOffsets(m_offsets);
    dialogTool->SetPenStyle(splPath->GetPenStyle());
    dialogTool->SetColor(splPath->GetColor());
    dialogTool->SetApproximationScale(splPath->GetApproximationScale());
    dialogTool->SetSuffix(m_suffix);
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

    VSplinePath splPath = curve->Outline(widths, initData.suffix);
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
        initData.data->AddSpline(initData.data->GeometricObject<VAbstractBezier>(initData.id), initData.id);
        if (initData.parse != Document::FullParse)
        {
            initData.doc->UpdateToolData(initData.id, initData.data);
        }
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::GraduatedCurve, initData.doc);
        auto *path = new VToolGraduatedCurve(initData);
        initData.scene->addItem(path);
        InitSplinePathToolConnections(initData.scene, path);
        VAbstractPattern::AddTool(initData.id, path);
        initData.doc->IncrementReferens(curve->getIdTool());
        return path;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolGraduatedCurve::GetApproximationScale() const -> qreal
{
    QSharedPointer<VAbstractCurve> const curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    SCASSERT(curve.isNull() == false)

    return curve->GetApproximationScale();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolGraduatedCurve::SetApproximationScale(qreal value)
{
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    QSharedPointer<VAbstractCurve> const curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(m_id);
    curve->SetApproximationScale(value);
    SaveOption(obj);
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

        VGraduatedCurveOffset offsetData;
        offsetData.name = offset.name;
        offsetData.offset = width;
        offsetData.description = offset.description;

        widths.append(offsetData);
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

        VRawGraduatedCurveOffset offsetData;
        offsetData.name = offset.name;
        offsetData.formula = formula;
        offsetData.description = offset.description;

        m_offsets.append(offsetData);
    }

    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolGraduatedCurve::GetSuffix() const -> QString
{
    return m_suffix;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolGraduatedCurve::SetSuffix(QString suffix)
{
    m_suffix = suffix;
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolGraduatedCurve::CurveName() const -> QString
{
    return VAbstractTool::data.GetGObject(m_originCurveId)->ObjectName();
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
    const qint32 num = nodeList.size();
    offsets.reserve(num);

    for (qint32 i = 0; i < num; ++i)
    {
        if (const QDomElement element = nodeList.at(i).toElement();
            not element.isNull() && element.tagName() == VAbstractPattern::TagOffset)
        {
            VRawGraduatedCurveOffset offsetData;
            offsetData.name = VDomDocument::GetParametrString(element, AttrName);
            offsetData.description = VDomDocument::GetParametrEmptyString(element, AttrDescription);
            offsetData.formula = VDomDocument::GetParametrString(element, AttrWidth, QChar('0'));
            offsets.append(offsetData);
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
void VToolGraduatedCurve::RemoveReferens()
{
    const auto curve = VAbstractTool::data.GetGObject(m_originCurveId);
    doc->DecrementReferens(curve->getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolGraduatedCurve::SaveDialog(QDomElement &domElement,
                                     QList<quint32> &oldDependencies,
                                     QList<quint32> &newDependencies)
{
    SCASSERT(not m_dialog.isNull())
    QPointer<DialogGraduatedCurve> const dialogTool = qobject_cast<DialogGraduatedCurve *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    AddDependence(oldDependencies, m_originCurveId);
    AddDependence(newDependencies, dialogTool->GetOriginCurveId());

    doc->SetAttribute(domElement, AttrCurve, dialogTool->GetOriginCurveId());
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

    QVector<VRawGraduatedCurveOffset> const offsets = dialogTool->GetOffsets();
    UpdateOffsets(domElement, offsets);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolGraduatedCurve::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VAbstractSpline::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrCurve, m_originCurveId);
    doc->SetAttribute(tag, AttrSuffix, m_suffix);

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

        visual->SetCurveId(m_originCurveId);

        const bool osSeparator = VAbstractApplication::VApp()->Settings()->GetOsSeparator();
        const VTranslateVars *trVars = VAbstractApplication::VApp()->TrVars();

        QVector<VRawGraduatedCurveOffset> toUserOffsets;
        toUserOffsets.reserve(m_offsets.size());
        for (const auto &offset : qAsConst(m_offsets))
        {
            VRawGraduatedCurveOffset offsetData;
            offsetData.name = offset.name;
            offsetData.formula = trVars->FormulaToUser(offset.formula, osSeparator);

            toUserOffsets.append(offsetData);
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
    VAbstractSpline::ReadToolAttributes(domElement);

    m_originCurveId = VDomDocument::GetParametrUInt(domElement, AttrCurve, NULL_ID_STR);
    m_suffix = VDomDocument::GetParametrString(domElement, AttrSuffix);
    m_offsets = ExtractOffsetData(domElement);
}

//---------------------------------------------------------------------------------------------------------------------
VToolGraduatedCurve::VToolGraduatedCurve(const VToolGraduatedCurveInitData &initData, QGraphicsItem *parent)
  : VAbstractSpline(initData.doc, initData.data, initData.id, initData.notes, parent),
    m_offsets(initData.offsets),
    m_originCurveId(initData.originCurveId),
    m_suffix(initData.suffix)
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
