/************************************************************************
 **
 **  @file   vtoolcubicbezier.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   10 3, 2016
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

#include "vtoolcubicbezier.h"

#include <QDomElement>
#include <QPen>
#include <QSharedPointer>
#include <Qt>
#include <new>

#include "../../../dialogs/tools/dialogcubicbezier.h"
#include "../../../dialogs/tools/dialogtool.h"
#include "../../../visualization/path/vistoolcubicbezier.h"
#include "../../../visualization/visualization.h"
#include "../../vabstracttool.h"
#include "../ifc/exception/vexception.h"
#include "../vdrawtool.h"
#include "../vgeometry/../ifc/ifcdef.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vabstractspline.h"

const QString VToolCubicBezier::ToolType = QStringLiteral("cubicBezier");

//---------------------------------------------------------------------------------------------------------------------
VToolCubicBezier::VToolCubicBezier(const VToolCubicBezierInitData &initData, QGraphicsItem *parent)
  : VAbstractSpline(initData.doc, initData.data, initData.id, initData.notes, parent)
{
    SetSceneType(SceneObject::Spline);

    this->setFlag(QGraphicsItem::ItemIsFocusable, true); // For keyboard input focus

    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    auto *dialogTool = qobject_cast<DialogCubicBezier *>(m_dialog);
    SCASSERT(dialogTool != nullptr)
    const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    dialogTool->SetSpline(*spl);
    dialogTool->SetNotes(m_notes);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCubicBezier::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                              VContainer *data) -> VToolCubicBezier *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogCubicBezier> dialogTool = qobject_cast<DialogCubicBezier *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolCubicBezierInitData initData;
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.spline = new VCubicBezier(dialogTool->GetSpline());
    initData.notes = dialogTool->GetNotes();

    auto *spl = Create(initData);

    if (spl != nullptr)
    {
        spl->m_dialog = dialog;
    }
    return spl;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCubicBezier::Create(VToolCubicBezierInitData initData) -> VToolCubicBezier *
{
    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->AddGObject(initData.spline);
        initData.data->AddSpline(initData.data->GeometricObject<VAbstractBezier>(initData.id), initData.id);
    }
    else
    {
        initData.data->UpdateGObject(initData.id, initData.spline);
        initData.data->AddSpline(initData.data->GeometricObject<VAbstractBezier>(initData.id), initData.id);
        if (initData.parse != Document::FullParse)
        {
            initData.doc->UpdateToolData(initData.id, initData.data);
        }
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::CubicBezier, initData.doc);
        auto *_spl = new VToolCubicBezier(initData);
        initData.scene->addItem(_spl);
        InitSplineToolConnections(initData.scene, _spl);
        VAbstractPattern::AddTool(initData.id, _spl);
        initData.doc->IncrementReferens(initData.spline->GetP1().getIdTool());
        initData.doc->IncrementReferens(initData.spline->GetP1().getIdTool());
        initData.doc->IncrementReferens(initData.spline->GetP1().getIdTool());
        initData.doc->IncrementReferens(initData.spline->GetP4().getIdTool());
        return _spl;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCubicBezier::FirstPointName() const -> QString
{
    auto spline = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    return spline->GetP1().name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCubicBezier::SecondPointName() const -> QString
{
    auto spline = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    return spline->GetP2().name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCubicBezier::ThirdPointName() const -> QString
{
    auto spline = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    return spline->GetP3().name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCubicBezier::ForthPointName() const -> QString
{
    auto spline = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    return spline->GetP4().name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCubicBezier::getSpline() const -> VCubicBezier
{
    auto spline = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    return *spline.data();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::setSpline(const VCubicBezier &spl)
{
    QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
    QSharedPointer<VCubicBezier> spline = qSharedPointerDynamicCast<VCubicBezier>(obj);
    *spline.data() = spl;
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolCubicBezier>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    Q_UNUSED(id)
    try
    {
        ContextMenu<DialogCubicBezier>(event);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::RemoveReferens()
{
    const auto spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    doc->DecrementReferens(spl->GetP1().getIdTool());
    doc->DecrementReferens(spl->GetP2().getIdTool());
    doc->DecrementReferens(spl->GetP3().getIdTool());
    doc->DecrementReferens(spl->GetP4().getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies,
                                  QList<quint32> &newDependencies)
{
    SCASSERT(not m_dialog.isNull())
    auto *dialogTool = qobject_cast<DialogCubicBezier *>(m_dialog);
    SCASSERT(dialogTool != nullptr)

    const auto oldSpl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    AddDependence(oldDependencies, oldSpl->GetP1().id());
    AddDependence(oldDependencies, oldSpl->GetP2().id());
    AddDependence(oldDependencies, oldSpl->GetP3().id());
    AddDependence(oldDependencies, oldSpl->GetP4().id());

    const VCubicBezier spl = dialogTool->GetSpline();
    AddDependence(newDependencies, spl.GetP1().id());
    AddDependence(newDependencies, spl.GetP2().id());
    AddDependence(newDependencies, spl.GetP3().id());
    AddDependence(newDependencies, spl.GetP4().id());
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrNotes, dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });

    SetSplineAttributes(domElement, spl);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VAbstractSpline::SaveOptions(tag, obj);

    auto spl = qSharedPointerDynamicCast<VCubicBezier>(obj);
    SCASSERT(spl.isNull() == false)
    SetSplineAttributes(tag, *spl);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolCubicBezier *>(vis);
        SCASSERT(visual != nullptr)

        const QSharedPointer<VCubicBezier> spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
        visual->SetPoint1Id(spl->GetP1().id());
        visual->SetPoint2Id(spl->GetP2().id());
        visual->SetPoint3Id(spl->GetP3().id());
        visual->SetPoint4Id(spl->GetP4().id());
        visual->SetLineStyle(LineStyleToPenStyle(spl->GetPenStyle()));
        visual->SetApproximationScale(spl->GetApproximationScale());
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::RefreshGeometry()
{
    const QSharedPointer<VCubicBezier> spl = VAbstractTool::data.GeometricObject<VCubicBezier>(m_id);
    this->setPath(spl->GetPath());

    SetVisualization();
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCubicBezier::SetSplineAttributes(QDomElement &domElement, const VCubicBezier &spl)
{
    SCASSERT(doc != nullptr)

    doc->SetAttribute(domElement, AttrType, ToolType);
    doc->SetAttribute(domElement, AttrPoint1, spl.GetP1().id());
    doc->SetAttribute(domElement, AttrPoint2, spl.GetP2().id());
    doc->SetAttribute(domElement, AttrPoint3, spl.GetP3().id());
    doc->SetAttribute(domElement, AttrPoint4, spl.GetP4().id());
    doc->SetAttribute(domElement, AttrColor, spl.GetColor());
    doc->SetAttribute(domElement, AttrPenStyle, spl.GetPenStyle());
    doc->SetAttribute(domElement, AttrAScale, spl.GetApproximationScale());
    doc->SetAttributeOrRemoveIf<quint32>(domElement, AttrDuplicate, spl.GetDuplicate(),
                                         [](quint32 duplicate) noexcept { return duplicate == 0; });
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrAlias, spl.GetAliasSuffix(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
}
