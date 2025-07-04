/************************************************************************
 **
 **  @file   vtoollineintersect.cpp
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

#include "vtoollineintersect.h"

#include <QLine>
#include <QLineF>
#include <QPointF>
#include <QSharedPointer>
#include <new>

#include "../../../../dialogs/tools/dialoglineintersect.h"
#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../visualization/line/vistoollineintersect.h"
#include "../../../../visualization/visualization.h"
#include "../../../vabstracttool.h"
#include "../../vdrawtool.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/compatibility.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vtoolsinglepoint.h"

using namespace Qt::Literals::StringLiterals;

template <class T> class QSharedPointer;

const QString VToolLineIntersect::ToolType = QStringLiteral("lineIntersect");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolLineIntersect constructor.
 * @param initData init data.
 * @param parent parent object.
 */
VToolLineIntersect::VToolLineIntersect(const VToolLineIntersectInitData &initData, QGraphicsItem *parent)
  : VToolSinglePoint(initData.doc, initData.data, initData.id, initData.notes, parent),
    p1Line1(initData.p1Line1Id),
    p2Line1(initData.p2Line1Id),
    p1Line2(initData.p1Line2Id),
    p2Line2(initData.p2Line2Id)
{
    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolLineIntersect::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogLineIntersect> dialogTool = qobject_cast<DialogLineIntersect *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->SetP1Line1(p1Line1);
    dialogTool->SetP2Line1(p2Line1);
    dialogTool->SetP1Line2(p1Line2);
    dialogTool->SetP2Line2(p2Line2);
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
auto VToolLineIntersect::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                                VContainer *data) -> VToolLineIntersect *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogLineIntersect> dialogTool = qobject_cast<DialogLineIntersect *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolLineIntersectInitData initData;
    initData.p1Line1Id = dialogTool->GetP1Line1();
    initData.p2Line1Id = dialogTool->GetP2Line1();
    initData.p1Line2Id = dialogTool->GetP1Line2();
    initData.p2Line2Id = dialogTool->GetP2Line2();
    initData.name = dialogTool->GetPointName();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();

    VToolLineIntersect *point = Create(initData);
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
auto VToolLineIntersect::Create(VToolLineIntersectInitData initData) -> VToolLineIntersect *
{
    const QSharedPointer<VPointF> p1Line1 = initData.data->GeometricObject<VPointF>(initData.p1Line1Id);
    const QSharedPointer<VPointF> p2Line1 = initData.data->GeometricObject<VPointF>(initData.p2Line1Id);
    const QSharedPointer<VPointF> p1Line2 = initData.data->GeometricObject<VPointF>(initData.p1Line2Id);
    const QSharedPointer<VPointF> p2Line2 = initData.data->GeometricObject<VPointF>(initData.p2Line2Id);

    QLineF const line1(static_cast<QPointF>(*p1Line1), static_cast<QPointF>(*p2Line1));
    QLineF const line2(static_cast<QPointF>(*p1Line2), static_cast<QPointF>(*p2Line2));
    QPointF fPoint;
    if (VGObject::LinesIntersect(line1, line2, &fPoint) == QLineF::NoIntersection)
    {
        const QString errorMsg = tr("Error calculating point '%1'. Lines (%2;%3) and (%4;%5) have no point of "
                                    "intersection")
                                     .arg(initData.name,
                                          p1Line1->name(),
                                          p2Line1->name(),
                                          p1Line2->name(),
                                          p2Line2->name());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VExceptionObjectError(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        fPoint = QPointF();
    }

    auto *p = new VPointF(fPoint, initData.name, initData.mx, initData.my);
    p->SetShowLabel(initData.showLabel);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->AddGObject(p);
        initData.data->AddLine(initData.p1Line1Id, initData.id);
        initData.data->AddLine(initData.id, initData.p2Line1Id);
        initData.data->AddLine(initData.p1Line2Id, initData.id);
        initData.data->AddLine(initData.id, initData.p2Line2Id);
    }
    else
    {
        initData.data->UpdateGObject(initData.id, p);
        initData.data->AddLine(initData.p1Line1Id, initData.id);
        initData.data->AddLine(initData.id, initData.p2Line1Id);
        initData.data->AddLine(initData.p1Line2Id, initData.id);
        initData.data->AddLine(initData.id, initData.p2Line2Id);
        if (initData.parse != Document::FullParse)
        {
            initData.doc->UpdateToolData(initData.id, initData.data);
        }
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::LineIntersect, initData.doc);
        auto *point = new VToolLineIntersect(initData);
        initData.scene->addItem(point);
        InitToolConnections(initData.scene, point);
        VAbstractPattern::AddTool(initData.id, point);
        initData.doc->IncrementReferens(p1Line1->getIdTool());
        initData.doc->IncrementReferens(p2Line1->getIdTool());
        initData.doc->IncrementReferens(p1Line2->getIdTool());
        initData.doc->IncrementReferens(p2Line2->getIdTool());
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolLineIntersect::Line1P1Name() const -> QString
{
    return VAbstractTool::data.GetGObject(p1Line1)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolLineIntersect::Line1P2Name() const -> QString
{
    return VAbstractTool::data.GetGObject(p2Line1)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolLineIntersect::Line2P1Name() const -> QString
{
    return VAbstractTool::data.GetGObject(p1Line2)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolLineIntersect::Line2P2Name() const -> QString
{
    return VAbstractTool::data.GetGObject(p2Line2)->name();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RemoveReferens decrement value of reference.
 */
void VToolLineIntersect::RemoveReferens()
{
    const auto p1L1 = VAbstractTool::data.GetGObject(p1Line1);
    const auto p2L1 = VAbstractTool::data.GetGObject(p2Line1);
    const auto p1L2 = VAbstractTool::data.GetGObject(p1Line2);
    const auto p2L2 = VAbstractTool::data.GetGObject(p2Line2);

    doc->DecrementReferens(p1L1->getIdTool());
    doc->DecrementReferens(p2L1->getIdTool());
    doc->DecrementReferens(p1L2->getIdTool());
    doc->DecrementReferens(p2L2->getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolLineIntersect::SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies,
                                    QList<quint32> &newDependencies)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogLineIntersect> dialogTool = qobject_cast<DialogLineIntersect *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    AddDependence(oldDependencies, p1Line1);
    AddDependence(oldDependencies, p2Line1);
    AddDependence(oldDependencies, p1Line2);
    AddDependence(oldDependencies, p2Line2);
    AddDependence(newDependencies, dialogTool->GetP1Line1());
    AddDependence(newDependencies, dialogTool->GetP2Line1());
    AddDependence(newDependencies, dialogTool->GetP1Line2());
    AddDependence(newDependencies, dialogTool->GetP2Line2());

    doc->SetAttribute(domElement, AttrName, dialogTool->GetPointName());
    doc->SetAttribute(domElement, AttrP1Line1, QString().setNum(dialogTool->GetP1Line1()));
    doc->SetAttribute(domElement, AttrP2Line1, QString().setNum(dialogTool->GetP2Line1()));
    doc->SetAttribute(domElement, AttrP1Line2, QString().setNum(dialogTool->GetP1Line2()));
    doc->SetAttribute(domElement, AttrP2Line2, QString().setNum(dialogTool->GetP2Line2()));
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrNotes, dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLineIntersect::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrP1Line1, p1Line1);
    doc->SetAttribute(tag, AttrP2Line1, p2Line1);
    doc->SetAttribute(tag, AttrP1Line2, p1Line2);
    doc->SetAttribute(tag, AttrP2Line2, p2Line2);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLineIntersect::ReadToolAttributes(const QDomElement &domElement)
{
    VToolSinglePoint::ReadToolAttributes(domElement);

    p1Line1 = VAbstractPattern::GetParametrUInt(domElement, AttrP1Line1, NULL_ID_STR);
    p2Line1 = VAbstractPattern::GetParametrUInt(domElement, AttrP2Line1, NULL_ID_STR);
    p1Line2 = VAbstractPattern::GetParametrUInt(domElement, AttrP1Line2, NULL_ID_STR);
    p2Line2 = VAbstractPattern::GetParametrUInt(domElement, AttrP2Line2, NULL_ID_STR);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLineIntersect::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolLineIntersect *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetLine1P1Id(p1Line1);
        visual->SetLine1P2Id(p2Line1);
        visual->SetLine2P1Id(p1Line2);
        visual->SetLine2P2Id(p2Line2);
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolLineIntersect::MakeToolTip() const -> QString
{
    const QSharedPointer<VPointF> p1L1 = VAbstractTool::data.GeometricObject<VPointF>(p1Line1);
    const QSharedPointer<VPointF> p2L1 = VAbstractTool::data.GeometricObject<VPointF>(p2Line1);
    const QSharedPointer<VPointF> p1L2 = VAbstractTool::data.GeometricObject<VPointF>(p1Line2);
    const QSharedPointer<VPointF> p2L2 = VAbstractTool::data.GeometricObject<VPointF>(p2Line2);
    const QSharedPointer<VPointF> current = VAbstractTool::data.GeometricObject<VPointF>(m_id);

    const QLineF p1L1ToCur(static_cast<QPointF>(*p1L1), static_cast<QPointF>(*current));
    const QLineF curToP2L1(static_cast<QPointF>(*current), static_cast<QPointF>(*p2L1));
    const QLineF p1L2ToCur(static_cast<QPointF>(*p1L2), static_cast<QPointF>(*current));
    const QLineF curToP2L2(static_cast<QPointF>(*current), static_cast<QPointF>(*p2L2));

    const QString toolTip = u"<table>"
                            u"<tr> <td><b>%10:</b> %11</td> </tr>"
                            u"<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                            u"<tr> <td><b>%4:</b> %5 %3</td> </tr>"
                            u"<tr> <td><b>%6:</b> %7 %3</td> </tr>"
                            u"<tr> <td><b>%8:</b> %9 %3</td> </tr>"
                            u"</table>"_s.arg(u"%1->%2"_s.arg(p1L1->name(), current->name()))
                                .arg(VAbstractValApplication::VApp()->fromPixel(p1L1ToCur.length()))
                                .arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true),
                                     u"%1->%2"_s.arg(current->name(), p2L1->name()))
                                .arg(VAbstractValApplication::VApp()->fromPixel(curToP2L1.length()))
                                .arg(u"%1->%2"_s.arg(p1L2->name(), current->name()))
                                .arg(VAbstractValApplication::VApp()->fromPixel(p1L2ToCur.length()))
                                .arg(u"%1->%2"_s.arg(current->name(), p2L2->name()))
                                .arg(VAbstractValApplication::VApp()->fromPixel(curToP2L2.length()))
                                .arg(tr("Label"), current->name());
    return toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLineIntersect::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolLineIntersect>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolLineIntersect::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogLineIntersect>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}
