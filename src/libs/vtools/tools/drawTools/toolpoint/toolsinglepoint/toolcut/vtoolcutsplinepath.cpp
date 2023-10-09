/************************************************************************
 **
 **  @file   vtoolcutsplinepath.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   15 12, 2013
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

#include "vtoolcutsplinepath.h"

#include <QPointF>
#include <QSharedPointer>
#include <QVector>
#include <new>

#include "../../../../../dialogs/tools/dialogcutsplinepath.h"
#include "../../../../../dialogs/tools/dialogtool.h"
#include "../../../../../visualization/path/vistoolcutsplinepath.h"
#include "../../../../../visualization/visualization.h"
#include "../../../../vabstracttool.h"
#include "../../../vdrawtool.h"
#include "../ifc/exception/vexception.h"
#include "../ifc/ifcdef.h"
#include "../vgeometry/vabstractcubicbezierpath.h"
#include "../vgeometry/vabstractcurve.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepath.h"
#include "../vgeometry/vsplinepoint.h"
#include "../vmisc/vabstractapplication.h"
#include "../vmisc/vcommonsettings.h"
#include "../vpatterndb/variables/vcurvelength.h"
#include "../vpatterndb/vcontainer.h"
#include "../vpatterndb/vtranslatevars.h"
#include "../vwidgets/vmaingraphicsscene.h"
#include "vtoolcut.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

template <class T> class QSharedPointer;

const QString VToolCutSplinePath::ToolType = QStringLiteral("cutSplinePath");
const QString VToolCutSplinePath::AttrSplinePath = QStringLiteral("splinePath");

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief VToolCutSplinePath constructor.
 * @param initData init data.
 * @param parent parent object.
 */
VToolCutSplinePath::VToolCutSplinePath(const VToolCutInitData &initData, QGraphicsItem *parent)
  : VToolCut(initData, parent)
{
    ToolCreation(initData.typeCreation);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief setDialog set dialog when user want change tool option.
 */
void VToolCutSplinePath::SetDialog()
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogCutSplinePath> dialogTool = qobject_cast<DialogCutSplinePath *>(m_dialog);
    SCASSERT(not dialogTool.isNull())
    const QSharedPointer<VPointF> point = VAbstractTool::data.GeometricObject<VPointF>(m_id);
    dialogTool->SetFormula(formula);
    dialogTool->setSplinePathId(baseCurveId);
    dialogTool->SetPointName(point->name());
    dialogTool->SetNotes(m_notes);
    dialogTool->SetAliasSuffix1(m_aliasSuffix1);
    dialogTool->SetAliasSuffix2(m_aliasSuffix2);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Create help create tool form GUI.
 * @param dialog dialog.
 * @param scene pointer to scene.
 * @param doc dom document container.
 * @param data container with variables.
 */
auto VToolCutSplinePath::Create(const QPointer<DialogTool> &dialog, VMainGraphicsScene *scene, VAbstractPattern *doc,
                                VContainer *data) -> VToolCutSplinePath *
{
    SCASSERT(not dialog.isNull())
    const QPointer<DialogCutSplinePath> dialogTool = qobject_cast<DialogCutSplinePath *>(dialog);
    SCASSERT(not dialogTool.isNull())

    VToolCutInitData initData;
    initData.formula = dialogTool->GetFormula();
    initData.baseCurveId = dialogTool->getSplinePathId();
    initData.name = dialogTool->GetPointName();
    initData.scene = scene;
    initData.doc = doc;
    initData.data = data;
    initData.parse = Document::FullParse;
    initData.typeCreation = Source::FromGui;
    initData.notes = dialogTool->GetNotes();
    initData.aliasSuffix1 = dialogTool->GetAliasSuffix1();
    initData.aliasSuffix2 = dialogTool->GetAliasSuffix2();

    VToolCutSplinePath *point = Create(initData);
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
auto VToolCutSplinePath::Create(VToolCutInitData &initData) -> VToolCutSplinePath *
{
    const auto splPath = initData.data->GeometricObject<VAbstractCubicBezierPath>(initData.baseCurveId);
    SCASSERT(splPath != nullptr)

    // Declare special variable "CurrentLength"
    VCurveLength *length =
        new VCurveLength(initData.baseCurveId, initData.baseCurveId, splPath.data(), *initData.data->GetPatternUnit());
    length->SetName(currentLength);
    initData.data->AddVariable(length);

    const qreal result = CheckFormula(initData.id, initData.formula, initData.data);

    VSplinePath *splPath1 = nullptr;
    VSplinePath *splPath2 = nullptr;
    VPointF *p = VToolCutSplinePath::CutSplinePath(VAbstractValApplication::VApp()->toPixel(result), splPath,
                                                   initData.name, &splPath1, &splPath2);

    SCASSERT(splPath1 != nullptr)
    SCASSERT(splPath2 != nullptr)
    SCASSERT(p != nullptr)

    p->setMx(initData.mx);
    p->setMy(initData.my);
    p->SetShowLabel(initData.showLabel);

    splPath1->SetAliasSuffix(initData.aliasSuffix1);
    splPath2->SetAliasSuffix(initData.aliasSuffix2);

    if (initData.typeCreation == Source::FromGui)
    {
        initData.id = initData.data->AddGObject(p);

        auto path1 = QSharedPointer<VAbstractBezier>(splPath1);
        initData.data->AddSpline(path1, NULL_ID, initData.id);
        initData.data->RegisterUniqueName(path1);

        auto path2 = QSharedPointer<VAbstractBezier>(splPath2);
        initData.data->AddSpline(path2, NULL_ID, initData.id);
        initData.data->RegisterUniqueName(path2);
    }
    else
    {
        initData.data->UpdateGObject(initData.id, p);

        auto path1 = QSharedPointer<VAbstractBezier>(splPath1);
        initData.data->AddSpline(path1, NULL_ID, initData.id);
        initData.data->RegisterUniqueName(path1);

        auto path2 = QSharedPointer<VAbstractBezier>(splPath2);
        initData.data->AddSpline(path2, NULL_ID, initData.id);
        initData.data->RegisterUniqueName(path2);

        if (initData.parse != Document::FullParse)
        {
            initData.doc->UpdateToolData(initData.id, initData.data);
        }
    }

    VToolCutSplinePath *tool = nullptr;
    if (initData.parse == Document::FullParse)
    {
        VAbstractTool::AddRecord(initData.id, Tool::CutSplinePath, initData.doc);
        tool = new VToolCutSplinePath(initData);
        initData.scene->addItem(tool);
        InitToolConnections(initData.scene, tool);
        VAbstractPattern::AddTool(initData.id, tool);
        initData.doc->IncrementReferens(splPath->getIdTool());
    }
    // Very important to delete it. Only this tool need this special variable.
    initData.data->RemoveVariable(currentLength);
    return tool;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutSplinePath::ShowVisualization(bool show)
{
    ShowToolVisualization<VisToolCutSplinePath>(show);
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCutSplinePath::CutSplinePath(qreal length, const QSharedPointer<VAbstractCubicBezierPath> &splPath,
                                       const QString &pName, VSplinePath **splPath1, VSplinePath **splPath2)
    -> VPointF *
{
    SCASSERT(splPath != nullptr)

    QPointF spl1p2, spl1p3, spl2p2, spl2p3;
    qint32 p1 = 0, p2 = 0;

    const QPointF point = splPath->CutSplinePath(length, p1, p2, spl1p2, spl1p3, spl2p2, spl2p3, pName);
    VPointF *p = new VPointF(point);
    p->setName(pName);

    const QVector<VSplinePoint> points = splPath->GetSplinePath();

    const VSplinePoint splP1 = points.at(p1);
    const VSplinePoint splP2 = points.at(p2);

    VSpline spl1 = VSpline(splP1.P(), spl1p2, spl1p3, *p);
    spl1.SetApproximationScale(splPath->GetApproximationScale());

    VSpline spl2 = VSpline(*p, spl2p2, spl2p3, splP2.P());
    spl2.SetApproximationScale(splPath->GetApproximationScale());

    *splPath1 = new VSplinePath();
    (*splPath1)->SetApproximationScale(splPath->GetApproximationScale());
    *splPath2 = new VSplinePath();
    (*splPath2)->SetApproximationScale(splPath->GetApproximationScale());

    for (qint32 i = 0; i < points.size(); i++)
    {
        if (i <= p1 && i < p2)
        {
            if (i == p1)
            {
                const qreal angle1 = spl1.GetStartAngle() + 180;
                const QString angle1F = QString::number(angle1);

                (*splPath1)->append(VSplinePoint(splP1.P(), angle1, angle1F, spl1.GetStartAngle(),
                                                 spl1.GetStartAngleFormula(), splP1.Length1(), splP1.Length1Formula(),
                                                 spl1.GetC1Length(), spl1.GetC1LengthFormula()));

                const qreal angle2 = spl1.GetEndAngle() + 180;
                const QString angle2F = QString::number(angle2);

                const auto cutPoint =
                    VSplinePoint(*p, spl1.GetEndAngle(), spl1.GetEndAngleFormula(), angle2, angle2F, spl1.GetC2Length(),
                                 spl1.GetC2LengthFormula(), spl2.GetC1Length(), spl2.GetC1LengthFormula());
                (*splPath1)->append(cutPoint);
                continue;
            }
            (*splPath1)->append(points.at(i));
        }
        else
        {
            if (i == p2)
            {
                const qreal angle1 = spl2.GetStartAngle() + 180;
                const QString angle1F = QString::number(angle1);

                const auto cutPoint = VSplinePoint(
                    *p, angle1, angle1F, spl2.GetStartAngle(), spl2.GetStartAngleFormula(), spl1.GetC2Length(),
                    spl1.GetC2LengthFormula(), spl2.GetC1Length(), spl2.GetC1LengthFormula());

                (*splPath2)->append(cutPoint);

                const qreal angle2 = spl2.GetEndAngle() + 180;
                const QString angle2F = QString::number(angle2);

                (*splPath2)->append(VSplinePoint(splP2.P(), spl2.GetEndAngle(), spl2.GetEndAngleFormula(), angle2,
                                                 angle2F, spl2.GetC2Length(), spl2.GetC2LengthFormula(),
                                                 splP2.Length2(), splP2.Length2Formula()));
                continue;
            }
            (*splPath2)->append(points.at(i));
        }
    }

    return p;
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutSplinePath::ShowContextMenu(QGraphicsSceneContextMenuEvent *event, quint32 id)
{
    try
    {
        ContextMenu<DialogCutSplinePath>(event, id);
    }
    catch (const VExceptionToolWasDeleted &e)
    {
        Q_UNUSED(e)
        return; // Leave this method immediately!!!
    }
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief SaveDialog save options into file after change in dialog.
 */
void VToolCutSplinePath::SaveDialog(QDomElement &domElement, QList<quint32> &oldDependencies,
                                    QList<quint32> &newDependencies)
{
    SCASSERT(not m_dialog.isNull())
    const QPointer<DialogCutSplinePath> dialogTool = qobject_cast<DialogCutSplinePath *>(m_dialog);
    SCASSERT(not dialogTool.isNull())

    AddDependence(oldDependencies, baseCurveId);
    AddDependence(newDependencies, dialogTool->getSplinePathId());

    doc->SetAttribute(domElement, AttrName, dialogTool->GetPointName());
    doc->SetAttribute(domElement, AttrLength, dialogTool->GetFormula());
    doc->SetAttribute(domElement, AttrSplinePath, QString().setNum(dialogTool->getSplinePathId()));
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrAlias1, dialogTool->GetAliasSuffix1(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrAlias2, dialogTool->GetAliasSuffix2(),
                                         [](const QString &suffix) noexcept { return suffix.isEmpty(); });

    const QString notes = dialogTool->GetNotes();
    doc->SetAttributeOrRemoveIf<QString>(domElement, AttrNotes, notes,
                                         [](const QString &notes) noexcept { return notes.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutSplinePath::SaveOptions(QDomElement &tag, QSharedPointer<VGObject> &obj)
{
    VToolCut::SaveOptions(tag, obj);

    doc->SetAttribute(tag, AttrType, ToolType);
    doc->SetAttribute(tag, AttrLength, formula);
    doc->SetAttribute(tag, AttrSplinePath, baseCurveId);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutSplinePath::ReadToolAttributes(const QDomElement &domElement)
{
    VToolCut::ReadToolAttributes(domElement);

    formula = doc->GetParametrString(domElement, AttrLength, QString());
    baseCurveId = doc->GetParametrUInt(domElement, AttrSplinePath, NULL_ID_STR);
}

//---------------------------------------------------------------------------------------------------------------------
void VToolCutSplinePath::SetVisualization()
{
    if (not vis.isNull())
    {
        auto *visual = qobject_cast<VisToolCutSplinePath *>(vis);
        SCASSERT(visual != nullptr)

        visual->SetSplinePathId(baseCurveId);
        visual->SetLength(VAbstractApplication::VApp()->TrVars()->FormulaToUser(
            formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator()));

        const QSharedPointer<VAbstractCurve> curve = VAbstractTool::data.GeometricObject<VAbstractCurve>(baseCurveId);
        visual->SetLineStyle(LineStyleToPenStyle(curve->GetPenStyle()));
        visual->SetMode(Mode::Show);
        visual->RefreshGeometry();
    }
}

//---------------------------------------------------------------------------------------------------------------------
auto VToolCutSplinePath::MakeToolTip() const -> QString
{
    const auto splPath = VAbstractTool::data.GeometricObject<VAbstractCubicBezierPath>(baseCurveId);

    const QString expression = VAbstractApplication::VApp()->TrVars()->FormulaToUser(
        formula, VAbstractApplication::VApp()->Settings()->GetOsSeparator());
    const qreal length = Visualization::FindValFromUser(expression, VAbstractTool::data.DataVariables());

    VSplinePath *splPath1 = nullptr;
    VSplinePath *splPath2 = nullptr;
    VPointF *p = VToolCutSplinePath::CutSplinePath(VAbstractValApplication::VApp()->toPixel(length), splPath,
                                                   QChar('X'), &splPath1, &splPath2);
    delete p; // Don't need this point

    splPath1->SetAliasSuffix(m_aliasSuffix1);
    splPath2->SetAliasSuffix(m_aliasSuffix2);

    const QString curveStr = QCoreApplication::translate("VToolCutSplinePath", "Curve");
    const QString lengthStr = QCoreApplication::translate("VToolCutSplinePath", "length");

    const QString toolTip =
        u"<table>"
        u"<tr> <td><b>%6:</b> %7</td> </tr>"
        u"<tr> <td><b>%1:</b> %2 %3</td> </tr>"
        u"<tr> <td><b>%8:</b> %9</td> </tr>"
        u"<tr> <td><b>%4:</b> %5 %3</td> </tr>"
        u"</table>"_s.arg(curveStr + "1 "_L1 + lengthStr)
            .arg(VAbstractValApplication::VApp()->fromPixel(splPath1->GetLength()))
            .arg(UnitsToStr(VAbstractValApplication::VApp()->patternUnits(), true), curveStr + "2 "_L1 + lengthStr)
            .arg(VAbstractValApplication::VApp()->fromPixel(splPath2->GetLength()))
            .arg(curveStr + " 1"_L1 + QCoreApplication::translate("VToolCutSplinePath", "label"),
                 splPath1->ObjectName(),
                 curveStr + " 2"_L1 + QCoreApplication::translate("VToolCutSplinePath", "label"),
                 splPath2->ObjectName());

    delete splPath1;
    delete splPath2;

    return toolTip;
}
