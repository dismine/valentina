/************************************************************************
 **
 **  @file   vtoolpointofcontact.cpp
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

#include "vtoolpointofcontact.h"

#include <QLineF>
#include <QMessageLogger>
#include <QSharedPointer>
#include <QtDebug>
#include <new>

#include "../../../../dialogs/tools/dialogpointofcontact.h"
#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../visualization/line/vistoolpointofcontact.h"
#include "../../../../visualization/visualization.h"
#include "../../../vabstracttool.h"
#include "../../vdrawtool.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/ifcdef.h"
#include "../ifc/xml/vdomdocument.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vformula.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vtoolsinglepoint.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

template <class T> class QSharedPointer;

const QString VToolPointOfContact::ToolType = QStringLiteral("pointOfContact");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolPointOfContact constructor.
 * @param initData init data.
 * @param parent parent object.
 */
VToolPointOfContact::VToolPointOfContact(const VToolPointOfContactInitData &initData, QGraphicsItem *parent)
  : VToolSinglePoint(initData.doc, initData.data, initData.id, initData.notes, parent),
    arcRadius(initData.radius),
    center(initData.center),
    firstPointId(initData.firstPointId),
    secondPointId(initData.secondPointId)
{
    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolPointOfContact::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogPointOfContact> dialogTool = qobject_cast<DialogPointOfContact *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->SetRadius(arcRadius);
    dialogTool->SetCenter(center);
    dialogTool->SetFirstPoint(firstPointId);
    dialogTool->SetSecondPoint(secondPointId);
    dialogTool->SetPointName(p->name());
    dialogTool->SetNotes(m_notes);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief FindPoint return point intersection line and arc.
 * @param radius string with formula arc radius.
 * @param center center arc point.
 * @param firstPoint first line point.
 * @param secondPoint second line point.
 * @param intersectionPoint point of intersection.
 * @return true if intersection exists.
 */
auto VToolPointOfContact::FindPoint(qreal radius, const QPointF &center, const QPointF &firstPoint,
                                    const QPointF &secondPoint, QPointF *intersectionPoint) -> bool
{
    SCASSERT(intersectionPoint != nullptr)

    QPointF p1, p2;
    qint32 const res = VGObject::LineIntersectCircle(center, qAbs(radius), QLineF(firstPoint, secondPoint), p1, p2);
    switch (res)
    {
        case 0:
            return false;
        case 1:
            *intersectionPoint = p1;
            return true;
        case 2:
        {
            const bool flagP1 = VGObject::IsPointOnLineSegment(p1, firstPoint, secondPoint);
            const bool flagP2 = VGObject::IsPointOnLineSegment(p2, firstPoint, secondPoint);
            if ((flagP1 && flagP2) || (!flagP1 && !flagP2) /*In case we have something wrong*/)
            {
                // We don't have options for choosing correct point. Use closest to segment first point.
                if (QLineF(firstPoint, p1).length() <= QLineF(firstPoint, p2).length())
                {
                    *intersectionPoint = p1;
                    return true;
                }

                *intersectionPoint = p2;
                return true;
            }

            // In this case we have one real and one theoretical intersection.
            if (flagP1)
            {
                *intersectionPoint = p1;
                return true;
            }

            *intersectionPoint = p2;
            return true;
        }
        default:
            qDebug() << "Unxpected value" << res;
            break;
    }
    return false;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool from GUI.
 * @param dialog dialog.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 */
auto VToolPointOfContact::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                                 VContainer *data) -> VToolPointOfContact *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogPointOfContact> dialogTool = qobject_cast<DialogPointOfContact *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolPointOfContactInitData initData;
    initData.radius = dialogTool->GetRadius();
    initData.center = dialogTool->GetCenter();
    initData.firstPointId = dialogTool->GetFirstPoint();
    initData.secondPointId = dialogTool->GetSecondPoint();
    initData.name = dialogTool->GetPointName();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();

    VToolPointOfContact *point = Create(initData);
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
auto VToolPointOfContact::Create(VToolPointOfContactInitData &initData) -> VToolPointOfContact *
{
    const QSharedPointer<VPointF> centerP = initData.data->GeometricObject<VPointF>(initData.center);
    const QSharedPointer<VPointF> firstP = initData.data->GeometricObject<VPointF>(initData.firstPointId);
    const QSharedPointer<VPointF> secondP = initData.data->GeometricObject<VPointF>(initData.secondPointId);

    const qreal result = CheckFormula(initData.id, initData.radius, initData.data);

    QPointF fPoint;
    const bool success =
        VToolPointOfContact::FindPoint(VAbstractValApplication::VApp()->toPixel(result), static_cast<QPointF>(*centerP),
                                       static_cast<QPointF>(*firstP), static_cast<QPointF>(*secondP), &fPoint);

    if (not success)
    {
        const QString errorMsg = tr("Error calculating point '%1'. Circle with center '%2' and radius '%3' doesn't "
                                    "have intersection with line (%4;%5)")
                                     .arg(initData.name, centerP->name())
                                     .arg(result)
                                     .arg(firstP->name(), secondP->name());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VExceptionObjectError(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
    }

    auto *p = new VPointF(fPoint, initData.name, initData.mx, initData.my);
    p->SetShowLabel(initData.showLabel);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->AddGObject(p);
        initData.data->AddLine(initData.firstPointId, initData.id);
        initData.data->AddLine(initData.secondPointId, initData.id);
        initData.data->AddLine(initData.center, initData.id);
    }
    else
    {
        initData.data->UpdateGObject(initData.id, p);
        initData.data->AddLine(initData.firstPointId, initData.id);
        initData.data->AddLine(initData.secondPointId, initData.id);
        initData.data->AddLine(initData.center, initData.id);
        if (initData.parse != Document::FullParse)
        {
            initData.doc->UpdateToolData(initData.id, initData.data);
        }
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::PointOfContact, initData.doc);
        auto *point = new VToolPointOfContact(initData);
        initData.scene->addItem(point);
        InitToolConnections(initData.scene, point);
        VAbstractPattern::AddTool(initData.id, point);
        initData.doc->IncrementReferens(centerP->getIdTool());
        initData.doc->IncrementReferens(firstP->getIdTool());
        initData.doc->IncrementReferens(secondP->getIdTool());
        return point;
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfContact::ArcCenterPointName() const -> QString
{
    return VAbstractTool::data.GetGObject(center)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfContact::FirstPointName() const -> QString
{
    return VAbstractTool::data.GetGObject(firstPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfContact::SecondPointName() const -> QString
{
    return VAbstractTool::data.GetGObject(secondPointId)->name();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief RemoveReferens decrement value of reference.
 */
void VToolPointOfContact::RemoveReferens()
{
    const auto c = VAbstractTool::data.GetGObject(center);
    const auto firstPoint = VAbstractTool::data.GetGObject(firstPointId);
    const auto secondPoint = VAbstractTool::data.GetGObject(secondPointId);

    doc->DecrementReferens(c->getIdTool());
    doc->DecrementReferens(firstPoint->getIdTool());
    doc->DecrementReferens(secondPoint->getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolPointOfContact::SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies,
                                     QList<quint32> &newDependencies)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogPointOfContact> dialogTool = qobject_cast<DialogPointOfContact *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    AddDependence(oldDependencies, center);
    AddDependence(oldDependencies, firstPointId);
    AddDependence(oldDependencies, secondPointId);
    AddDependence(newDependencies, dialogTool->GetCenter());
    AddDependence(newDependencies, dialogTool->GetFirstPoint());
    AddDependence(newDependencies, dialogTool->GetSecondPoint());

    doc->SetAttribute(domElement, AttrName, dialogTool->GetPointName());
    doc->SetAttribute(domElement, AttrRadius, dialogTool->GetRadius());
    doc->SetAttribute(domElement, AttrCenter, QString().setNum(dialogTool->GetCenter()));
    doc->SetAttribute(domElement, AttrFirstPoint, QString().setNum(dialogTool->GetFirstPoint()));
    doc->SetAttribute(domElement, AttrSecondPoint, QString().setNum(dialogTool->GetSecondPoint()));
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrNotes, dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfContact::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrRadius, arcRadius);
    doc->SetAttribute(tag, AttrCenter, center);
    doc->SetAttribute(tag, AttrFirstPoint, firstPointId);
    doc->SetAttribute(tag, AttrSecondPoint, secondPointId);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfContact::ReadToolAttributes(const QDomElement &domElement)
{
    VToolSinglePoint::ReadToolAttributes(domElement);

    arcRadius = VDomDocument::GetParametrString(domElement, AttrRadius, QString());
    center = VAbstractPattern::GetParametrUInt(domElement, AttrCenter, NULL_ID_STR);
    firstPointId = VAbstractPattern::GetParametrUInt(domElement, AttrFirstPoint, NULL_ID_STR);
    secondPointId = VAbstractPattern::GetParametrUInt(domElement, AttrSecondPoint, NULL_ID_STR);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfContact::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolPointOfContact *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetLineP1Id(firstPointId);
        visual->SetLineP2Id(secondPointId);
        visual->SetRadiusId(center);
        visual->SetRadius(VAbstractApplication::VApp()->TrVars()->FormulaToUser(
            arcRadius, VAbstractApplication::VApp()->Settings()->GetOsSeparator()));
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfContact::MakeToolTip() const -> QString
{
    const QSharedPointer<VPointF> p1 = VAbstractTool::data.GeometricObject<VPointF>(firstPointId);
    const QSharedPointer<VPointF> p2 = VAbstractTool::data.GeometricObject<VPointF>(secondPointId);
    const QSharedPointer<VPointF> centerP = VAbstractTool::data.GeometricObject<VPointF>(center);
    const QSharedPointer<VPointF> current = VAbstractTool::data.GeometricObject<VPointF>(m_id);

    const QLineF p1ToCur(static_cast<QPointF>(*p1), static_cast<QPointF>(*current));
    const QLineF p2ToCur(static_cast<QPointF>(*p2), static_cast<QPointF>(*current));
    const QLineF centerToCur(static_cast<QPointF>(*centerP), static_cast<QPointF>(*current));

    const QString toolTip = u"<table>"
                            u"<tr> <td><b>%10:</b> %11</td> </tr>"
                            u"<tr> <td><b>%1:</b> %2 %3</td> </tr>"
                            u"<tr> <td><b>%4:</b> %5 %3</td> </tr>"
                            u"<tr> <td><b>%6:</b> %7 %3</td> </tr>"
                            u"<tr> <td><b>%8:</b> %9°</td> </tr>"
                            u"</table>"_s.arg(u"%1->%2"_s.arg(p1->name(), current->name()))
                                .arg(VAbstractValApplication::VApp()->fromPixel(p1ToCur.length()))
                                .arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true),
                                     u"%1->%2"_s.arg(p2->name(), current->name()))
                                .arg(VAbstractValApplication::VApp()->fromPixel(p2ToCur.length()))
                                .arg(u"%1 %2->%3"_s.arg(tr("Length"), centerP->name(), current->name()))
                                .arg(VAbstractValApplication::VApp()->fromPixel(centerToCur.length()))
                                .arg(u"%1 %2->%3"_s.arg(tr("Angle"), centerP->name(), current->name()))
                                .arg(centerToCur.angle())
                                .arg(tr("Label"), current->name());
    return toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfContact::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolPointOfContact>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfContact::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogPointOfContact>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfContact::getArcRadius() const -> VFormula
{
    VFormula radius(arcRadius, this->getData());
    radius.setToolId(m_id);
    radius.setPostfix(UnitsToStr(VAbstractValApplication::VApp()->patternUnits()));
    radius.Eval();

    return radius;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfContact::setArcRadius(const VFormula &value)
{
    if (value.error() == false)
    {
        arcRadius = value.GetFormula(FormulaType::FromUser);

        QSharedPointer<VGObject> obj = VAbstractTool::data.GetGObject(m_id);
        SaveOption(obj);
    }
}
