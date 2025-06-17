/************************************************************************
 **
 **  @file   vtoolpointofintersectioncurves.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   22 1, 2016
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

#include "vtoolpointofintersectioncurves.h"

#include <QLineF>
#include <QSharedPointer>
#include <new>

#include "../../../../dialogs/tools/dialogpointofintersectioncurves.h"
#include "../../../../dialogs/tools/dialogtool.h"
#include "../../../../visualization/path/vistoolpointofintersectioncurves.h"
#include "../../../vabstracttool.h"
#include "../../vdrawtool.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/exception/vexceptionobjecterror.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vgobject.h"
#include "../vgeometry/vpointf.h"
#include "../vpatterndb/vcontainer.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vtoolsinglepoint.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

template <class T> class QSharedPointer;

const QString VToolPointOfIntersectionCurves::ToolType = QStringLiteral("pointOfIntersectionCurves");

//---------------------------------------------------------------------------------------------------------------------
VToolPointOfIntersectionCurves::VToolPointOfIntersectionCurves(const VToolPointOfIntersectionCurvesInitData &initData,
                                                               QGraphicsItem *parent)
  : VToolSinglePoint(initData.doc, initData.data, initData.id, initData.notes, parent),
    firstCurveId(initData.firstCurveId),
    secondCurveId(initData.secondCurveId),
    vCrossPoint(initData.vCrossPoint),
    hCrossPoint(initData.hCrossPoint),
    m_curve1Segments(initData.curve1Segments),
    m_curve2Segments(initData.curve2Segments),
    m_curve1AliasSuffix1(initData.curve1AliasSuffix1),
    m_curve1AliasSuffix2(initData.curve1AliasSuffix2),
    m_curve2AliasSuffix1(initData.curve2AliasSuffix1),
    m_curve2AliasSuffix2(initData.curve2AliasSuffix2)
{
    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    auto *dialogTool = qobject_cast<DialogPointOfIntersectionCurves *>(m_dialog);
    SCASSERT(dialogTool != nullptr)
    auto p = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->SetFirstCurveId(firstCurveId);
    dialogTool->SetSecondCurveId(secondCurveId);
    dialogTool->SetVCrossPoint(vCrossPoint);
    dialogTool->SetHCrossPoint(hCrossPoint);
    dialogTool->SetPointName(p->name());
    dialogTool->SetNotes(m_notes);
    dialogTool->SetCurve1AliasSuffix1(m_curve1AliasSuffix1);
    dialogTool->SetCurve1AliasSuffix2(m_curve1AliasSuffix2);
    dialogTool->SetCurve2AliasSuffix1(m_curve2AliasSuffix1);
    dialogTool->SetCurve2AliasSuffix2(m_curve2AliasSuffix2);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene,
                                            VAbstractPattern *doc, VContainer *data) -> VToolPointOfIntersectionCurves *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogPointOfIntersectionCurves> dialogTool =
        qobject_cast<DialogPointOfIntersectionCurves *>(dialog);
    SCASSERT(not dialogTool.isNull())

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_GCC("-Wnoexcept")

    VToolPointOfIntersectionCurvesInitData initData;

    QT_WARNING_POP

    initData.firstCurveId = dialogTool->GetFirstCurveId();
    initData.secondCurveId = dialogTool->GetSecondCurveId();
    initData.vCrossPoint = dialogTool->GetVCrossPoint();
    initData.hCrossPoint = dialogTool->GetHCrossPoint();
    initData.name = dialogTool->GetPointName();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();
    initData.curve1AliasSuffix1 = dialogTool->GetCurve1AliasSuffix1();
    initData.curve1AliasSuffix2 = dialogTool->GetCurve1AliasSuffix2();
    initData.curve2AliasSuffix1 = dialogTool->GetCurve2AliasSuffix1();
    initData.curve2AliasSuffix2 = dialogTool->GetCurve2AliasSuffix2();

    VToolPointOfIntersectionCurves *point = Create(initData);
    if (point != nullptr)
    {
        point->m_dialog = dialog;
    }
    return point;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::Create(VToolPointOfIntersectionCurvesInitData initData)
    -> VToolPointOfIntersectionCurves *
{
    auto curve1 = initData.data->GeometricObject<VAbstractCurve>(initData.firstCurveId);
    auto curve2 = initData.data->GeometricObject<VAbstractCurve>(initData.secondCurveId);

    QPointF fPoint;
    const bool success = VToolPointOfIntersectionCurves::FindPoint(curve1->GetPoints(), curve2->GetPoints(),
                                                                   initData.vCrossPoint, initData.hCrossPoint, &fPoint);

    if (not success)
    {
        const QString errorMsg = tr("Error calculating point '%1'. Curves '%2' and '%3' have no point of intersection")
                                     .arg(initData.name, curve1->name(), curve2->name());
        VAbstractApplication::VApp()->IsPedantic()
            ? throw VExceptionObjectError(errorMsg)
            : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
    }

    const qreal segLength1 = curve1->GetLengthByPoint(fPoint);
    const qreal segLength2 = curve2->GetLengthByPoint(fPoint);

    auto *p = new VPointF(fPoint, initData.name, initData.mx, initData.my);
    p->SetShowLabel(initData.showLabel);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->AddGObject(p);

        initData.curve1Segments =
            VToolSinglePoint::InitSegments(curve1->getType(), segLength1, p, initData.firstCurveId, initData.data,
                                           initData.curve1AliasSuffix1, initData.curve1AliasSuffix2);
        initData.curve2Segments =
            VToolSinglePoint::InitSegments(curve2->getType(), segLength2, p, initData.secondCurveId, initData.data,
                                           initData.curve2AliasSuffix1, initData.curve2AliasSuffix2);
    }
    else
    {
        initData.data->UpdateGObject(initData.id, p);

        initData.curve1Segments =
            VToolSinglePoint::InitSegments(curve1->getType(), segLength1, p, initData.firstCurveId, initData.data,
                                           initData.curve1AliasSuffix1, initData.curve1AliasSuffix2);
        initData.curve2Segments =
            VToolSinglePoint::InitSegments(curve2->getType(), segLength2, p, initData.secondCurveId, initData.data,
                                           initData.curve2AliasSuffix1, initData.curve2AliasSuffix2);

        if (initData.parse != Document::FullParse)
        {
            initData.doc->UpdateToolData(initData.id, initData.data);
        }
    }

    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::PointOfIntersectionCurves, initData.doc);
        auto *point = new VToolPointOfIntersectionCurves(initData);
        initData.scene->addItem(point);
        InitToolConnections(initData.scene, point);
        VAbstractPattern::AddTool(initData.id, point);
        initData.doc->IncrementReferens(curve1->getIdTool());
        initData.doc->IncrementReferens(curve2->getIdTool());
        return point;
    }

    auto *tool = qobject_cast<VToolPointOfIntersectionCurves *>(VAbstractPattern::getTool(initData.id));
    tool->SetCurve1Segments(initData.curve1Segments);
    tool->SetCurve2Segments(initData.curve2Segments);

    return nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::FindPoint(const QVector<QPointF> &curve1Points,
                                               const QVector<QPointF> &curve2Points, VCrossCurvesPoint vCrossPoint,
                                               HCrossCurvesPoint hCrossPoint, QPointF *intersectionPoint) -> bool
{
    SCASSERT(intersectionPoint != nullptr)

    if (curve1Points.isEmpty() || curve2Points.isEmpty())
    {
        return false;
    }

    QVector<QPointF> intersections;
    for (auto i = 0; i < curve1Points.count() - 1; ++i)
    {
        intersections << VAbstractCurve::CurveIntersectLine(curve2Points,
                                                            QLineF(curve1Points.at(i), curve1Points.at(i + 1)));
    }

    if (intersections.isEmpty())
    {
        return false;
    }

    if (intersections.size() == 1)
    {
        *intersectionPoint = intersections.at(0);
        return true;
    }

    QVector<QPointF> vIntersections;
    if (vCrossPoint == VCrossCurvesPoint::HighestPoint)
    {
        qreal minY = intersections.at(0).y();
        vIntersections.append(intersections.at(0));

        for (auto i = 1; i < intersections.count(); ++i)
        {
            const QPointF p = intersections.at(i);
            if (p.y() < minY)
            {
                minY = p.y();
                vIntersections.clear();
                vIntersections.append(p);
            }
            else if (VFuzzyComparePossibleNulls(p.y(), minY))
            {
                vIntersections.append(p);
            }
        }
    }
    else
    {
        qreal maxY = intersections.at(0).y();
        vIntersections.append(intersections.at(0));

        for (auto i = 1; i < intersections.count(); ++i)
        {
            const QPointF p = intersections.at(i);
            if (p.y() > maxY)
            {
                maxY = p.y();
                vIntersections.clear();
                vIntersections.append(p);
            }
            else if (VFuzzyComparePossibleNulls(p.y(), maxY))
            {
                vIntersections.append(p);
            }
        }
    }

    if (vIntersections.isEmpty())
    {
        return false;
    }

    if (vIntersections.size() == 1)
    {
        *intersectionPoint = vIntersections.at(0);
        return true;
    }

    QPointF crossPoint = vIntersections.at(0);

    if (hCrossPoint == HCrossCurvesPoint::RightmostPoint)
    {
        qreal maxX = vIntersections.at(0).x();

        for (auto i = 1; i < vIntersections.count(); ++i)
        {
            const QPointF p = vIntersections.at(i);
            if (p.x() > maxX)
            {
                maxX = p.x();
                crossPoint = p;
            }
        }
    }
    else
    {
        qreal minX = vIntersections.at(0).x();

        for (auto i = 1; i < vIntersections.count(); ++i)
        {
            const QPointF p = vIntersections.at(i);
            if (p.x() < minX)
            {
                minX = p.x();
                crossPoint = p;
            }
        }
    }

    *intersectionPoint = crossPoint;
    return true;
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::FirstCurveName() const -> QString
{
    return VAbstractTool::data.GetGObject(firstCurveId)->ObjectName();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::SecondCurveName() const -> QString
{
    return VAbstractTool::data.GetGObject(secondCurveId)->ObjectName();
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::GetVCrossPoint() const -> VCrossCurvesPoint
{
    return vCrossPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetVCrossPoint(VCrossCurvesPoint value)
{
    vCrossPoint = value;

    auto obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::GetHCrossPoint() const -> HCrossCurvesPoint
{
    return hCrossPoint;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetHCrossPoint(HCrossCurvesPoint value)
{
    hCrossPoint = value;

    auto obj = VAbstractTool::data.GetGObject(m_id);
    SaveOption(obj);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolPointOfIntersectionCurves>(show);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogPointOfIntersectionCurves>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::RemoveReferens()
{
    const auto curve1 = VAbstractTool::data.GetGObject(firstCurveId);
    const auto curve2 = VAbstractTool::data.GetGObject(secondCurveId);

    doc->DecrementReferens(curve1->getIdTool());
    doc->DecrementReferens(curve2->getIdTool());
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies,
                                                QList<quint32> &newDependencies)
{
    SCASSERT(not m_dialog.isNull())
    auto *dialogTool = qobject_cast<DialogPointOfIntersectionCurves *>(m_dialog);
    SCASSERT(dialogTool != nullptr)

    AddDependence(oldDependencies, firstCurveId);
    AddDependence(oldDependencies, secondCurveId);
    AddDependence(newDependencies, dialogTool->GetFirstCurveId());
    AddDependence(newDependencies, dialogTool->GetSecondCurveId());

    doc->SetAttribute(domElement, AttrName, dialogTool->GetPointName());
    doc->SetAttribute(domElement, AttrCurve1, QString().setNum(dialogTool->GetFirstCurveId()));
    doc->SetAttribute(domElement, AttrCurve2, QString().setNum(dialogTool->GetSecondCurveId()));
    doc->SetAttribute(domElement, AttrVCrossPoint, QString().setNum(static_cast<int>(dialogTool->GetVCrossPoint())));
    doc->SetAttribute(domElement, AttrHCrossPoint, QString().setNum(static_cast<int>(dialogTool->GetHCrossPoint())));
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrCurve1Alias1, dialogTool->GetCurve1AliasSuffix1(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrCurve1Alias2, dialogTool->GetCurve1AliasSuffix2(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrCurve2Alias1, dialogTool->GetCurve2AliasSuffix1(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrCurve2Alias2, dialogTool->GetCurve2AliasSuffix2(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrNotes, dialogTool->GetNotes(),
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolSinglePoint::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrCurve1, firstCurveId);
    doc->SetAttribute(tag, AttrCurve2, secondCurveId);
    doc->SetAttribute(tag, AttrVCrossPoint, static_cast<int>(vCrossPoint));
    doc->SetAttribute(tag, AttrHCrossPoint, static_cast<int>(hCrossPoint));
    doc->SetAttributeOrRemoveIf<QString>(tag, AttrCurve1Alias1, m_curve1AliasSuffix1,
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(tag, AttrCurve1Alias2, m_curve1AliasSuffix2,
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(tag, AttrCurve2Alias1, m_curve2AliasSuffix1,
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(tag, AttrCurve2Alias2, m_curve2AliasSuffix2,
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::ReadToolAttributes(const QDomElement &domElement)
{
    VToolSinglePoint::ReadToolAttributes(domElement);

    firstCurveId = VAbstractPattern::GetParametrUInt(domElement, AttrCurve1, NULL_ID_STR);
    secondCurveId = VAbstractPattern::GetParametrUInt(domElement, AttrCurve2, NULL_ID_STR);
    vCrossPoint =
        static_cast<VCrossCurvesPoint>(VAbstractPattern::GetParametrUInt(domElement, AttrVCrossPoint, QChar('1')));
    hCrossPoint =
        static_cast<HCrossCurvesPoint>(VAbstractPattern::GetParametrUInt(domElement, AttrHCrossPoint, QChar('1')));
    m_curve1AliasSuffix1 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurve1Alias1);
    m_curve1AliasSuffix2 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurve1Alias2);
    m_curve2AliasSuffix1 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurve2Alias1);
    m_curve2AliasSuffix2 = VAbstractPattern::GetParametrEmptyString(domElement, AttrCurve2Alias2);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolPointOfIntersectionCurves *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetCurve1Id(firstCurveId);
        visual->SetCurve2Id(secondCurveId);
        visual->SetVCrossPoint(vCrossPoint);
        visual->SetHCrossPoint(hCrossPoint);
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolPointOfIntersectionCurves::MakeToolTip() const -> QString
{
    const QSharedPointer<VPointF> p = VAbstractTool::data.GeometricObject<VPointF>(m_id);

    const QString toolTip = u"<table>"
                            u"<tr> <td><b>%1:</b> %2</td> </tr>"
                            u"<tr> <td><b>%3:</b> %4</td> </tr>"
                            u"<tr> <td><b>%5:</b> %6</td> </tr>"
                            u"<tr> <td><b>%7:</b> %8</td> </tr>"
                            u"<tr> <td><b>%9:</b> %10</td> </tr>"
                            u"</table>"_s
                                .arg(tr("Label"), p->name(),                            /* 1, 2 */
                                     tr("Curve 1 segment 1"), m_curve1Segments.first,   /* 3, 4 */
                                     tr("Curve 1 segment 2"), m_curve1Segments.second)  /* 5, 6 */
                                .arg(tr("Curve 2 segment 1"), m_curve2Segments.first,   /* 7, 8 */
                                     tr("Curve 2 segment 2"), m_curve2Segments.second); /* 9, 10 */

    return toolTip;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetCurve1Segments(const QPair<QString, QString> &segments)
{
    m_curve1Segments = segments;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolPointOfIntersectionCurves::SetCurve2Segments(const QPair<QString, QString> &segments)
{
    m_curve2Segments = segments;
}
