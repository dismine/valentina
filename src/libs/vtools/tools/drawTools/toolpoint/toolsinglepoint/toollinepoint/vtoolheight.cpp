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
#include <new>

#include "../../../../../dialogs/tools/dialogheight.h"
#include "../../../../../dialogs/tools/dialogtool.h"
#include "../../../../../visualization/line/vistoolheight.h"
#include "../../../../../visualization/visualization.h"
#include "../../../../vabstracttool.h"
#include "../../../vdrawtool.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
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
        initData.data->AddLine(initData.basePointId, initData.id);
        initData.data->AddLine(initData.p1LineId, initData.id);
        initData.data->AddLine(initData.p2LineId, initData.id);
    }
    else
    {
        initData.data->UpdateGObject(initData.id, p);
        initData.data->AddLine(initData.basePointId, initData.id);
        initData.data->AddLine(initData.p1LineId, initData.id);
        initData.data->AddLine(initData.p2LineId, initData.id);
        if (initData.parse != Document::FullParse)
        {
            initData.doc->UpdateToolData(initData.id, initData.data);
        }
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::Height, initData.doc);
        auto *point = new VToolHeight(initData);
        initData.scene->addItem(point);
        InitToolConnections(initData.scene, point);
        VAbstractPattern::AddTool(initData.id, point);
        initData.doc->IncrementReferens(basePoint->getIdTool());
        initData.doc->IncrementReferens(p1Line->getIdTool());
        initData.doc->IncrementReferens(p2Line->getIdTool());
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
void VToolHeight::SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies, QList<quint32> &newDependencies)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogHeight> dialogTool = qobject_cast<DialogHeight *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    AddDependence(oldDependencies, basePointId);
    AddDependence(oldDependencies, p1LineId);
    AddDependence(oldDependencies, p2LineId);
    AddDependence(newDependencies, dialogTool->GetBasePointId());
    AddDependence(newDependencies, dialogTool->GetP1LineId());
    AddDependence(newDependencies, dialogTool->GetP2LineId());

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
