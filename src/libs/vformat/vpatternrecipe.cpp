/************************************************************************
 **
 **  @file   vpatternrecipe.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   8 7, 2019
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2019 Valentina project
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

#include "vpatternrecipe.h"
#include "../ifc/exception/vexceptioninvalidhistory.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../vgeometry/vcubicbezier.h"
#include "../vgeometry/vcubicbezierpath.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vsplinepath.h"
#include "../vmisc/projectversion.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/variables/vincrement.h"
#include "../vpatterndb/variables/vmeasurement.h"
#include "../vpatterndb/vcontainer.h"

#include "../vtools/tools/drawTools/operation/flipping/vtoolflippingbyaxis.h"
#include "../vtools/tools/drawTools/operation/flipping/vtoolflippingbyline.h"
#include "../vtools/tools/drawTools/operation/vtoolmove.h"
#include "../vtools/tools/drawTools/operation/vtoolrotation.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolarc.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolarcwithlength.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolcubicbezier.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolcubicbezierpath.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolellipticalarc.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolspline.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolsplinepath.h"
#include "../vtools/tools/drawTools/toolpoint/tooldoublepoint/vtooltruedarts.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutarc.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutspline.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toolcut/vtoolcutsplinepath.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolalongline.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolbisector.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolcurveintersectaxis.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolendline.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolheight.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoollineintersectaxis.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolnormal.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/toollinepoint/vtoolshoulderpoint.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolbasepoint.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoollineintersect.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointfromarcandtangent.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointfromcircleandtangent.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofcontact.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersection.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersectionarcs.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersectioncircles.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtoolpointofintersectioncurves.h"
#include "../vtools/tools/drawTools/toolpoint/toolsinglepoint/vtooltriangle.h"
#include "../vtools/tools/drawTools/vtoolline.h"

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
#define TagStep QStringLiteral("step")

#define AttrLabel QStringLiteral("label")
#define AttrLengthValue QStringLiteral("lengthValue")
#define AttrAngleValue QStringLiteral("angleValue")
#define AttrAngle1Value QStringLiteral("angle1Value")
#define AttrAngle2Value QStringLiteral("angle2Value")
#define AttrLength1Value QStringLiteral("length1Value")
#define AttrLength2Value QStringLiteral("length2Value")
#define AttrRadiusValue QStringLiteral("radiusValue")
#define AttrC1RadiusValue QStringLiteral("c1RadiusValue")
#define AttrC2RadiusValue QStringLiteral("c2RadiusValue")
#define AttrCRadiusValue QStringLiteral("cRadiusValue")
#define AttrRadius1Value QStringLiteral("radius1Value")
#define AttrRadius2Value QStringLiteral("radius2Value")
#define AttrRotationAngleValue QStringLiteral("rotationAngleValue")

//---------------------------------------------------------------------------------------------------------------------
inline auto FileComment() -> QString
{
    return QStringLiteral("Recipe created with Valentina v%1 (https://smart-pattern.com.ua/).").arg(AppVersionStr());
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> auto GetPatternTool(quint32 id) -> T *
{
    T *tool = qobject_cast<T *>(VAbstractPattern::getTool(id));
    if (not tool)
    {
        throw VExceptionInvalidHistory(QObject::tr("Cannot cast tool with id '%1'.").arg(id));
    }
    return tool;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
VPatternRecipe::VPatternRecipe(VAbstractPattern *pattern, QObject *parent)
  : VDomDocument(parent),
    m_pattern(pattern)
{
    SCASSERT(pattern != nullptr)

    QDomElement recipeElement = createElement(QStringLiteral("recipe"));
    recipeElement.appendChild(createComment(FileComment()));
    SetAttribute(recipeElement, QStringLiteral("version"), QStringLiteral("1.3.1"));

    recipeElement.appendChild(Prerequisite());
    recipeElement.appendChild(Content());

    appendChild(recipeElement);
    insertBefore(
        createProcessingInstruction(QStringLiteral("xml"), QStringLiteral("version=\"1.0\" encoding=\"UTF-8\"")),
        firstChild());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::Prerequisite() -> QDomElement
{
    /*
     <prerequisite>
        <measurements>
            <m description="" full_name="Обхват талии" name="@От" value="65"/>
        </measurements>
        <increments>
            <increment description="" formula="height/2-15" name="#L_C"/>
        </increments>
        <previewCalculations>
            <increment description="" formula="height/2-15" name="#L_C"/>
        </previewCalculations>
    </prerequisite>
     */
    QDomElement prerequisiteElement = createElement(QStringLiteral("prerequisite"));

    prerequisiteElement.appendChild(CreateElementWithText(QStringLiteral("valentina"), AppVersionStr()));
    prerequisiteElement.appendChild(
        CreateElementWithText(QStringLiteral("unit"), UnitsToStr(VAbstractValApplication::VApp()->patternUnits())));
    prerequisiteElement.appendChild(CreateElementWithText(QStringLiteral("author"), m_pattern->GetCompanyName()));
    prerequisiteElement.appendChild(CreateElementWithText(QStringLiteral("pattenName"), m_pattern->GetPatternName()));
    prerequisiteElement.appendChild(CreateElementWithText(QStringLiteral("description"), m_pattern->GetDescription()));
    prerequisiteElement.appendChild(CreateElementWithText(QStringLiteral("notes"), m_pattern->GetNotes()));
    prerequisiteElement.appendChild(Measurements());
    prerequisiteElement.appendChild(Increments());
    prerequisiteElement.appendChild(PreviewCalculations());

    return prerequisiteElement;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::Measurements() -> QDomElement
{
    QDomElement measurements = createElement(QStringLiteral("measurements"));

    VContainer data = m_pattern->GetCompleteData();
    QList<QSharedPointer<VMeasurement>> patternMeasurements = data.DataMeasurementsWithSeparators().values();

    // Resore order
    std::sort(patternMeasurements.begin(), patternMeasurements.end(),
              [](const QSharedPointer<VMeasurement> &a, const QSharedPointer<VMeasurement> &b)
              { return a->Index() < b->Index(); });

    for (auto &m : patternMeasurements)
    {
        measurements.appendChild(Measurement(m));
    }

    return measurements;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::Measurement(const QSharedPointer<VMeasurement> &m) -> QDomElement
{
    /*
     * <measurements>
     *  <m description="" full_name="Обхват талии" name="@От" value="65"/>
     * </measurements>
     */
    QDomElement measurement = createElement(QStringLiteral("measurement"));

    SetAttribute(measurement, QStringLiteral("name"), m->GetName());
    SetAttribute(measurement, QStringLiteral("description"), m->GetDescription());

    if (m->GetType() != VarType::MeasurementSeparator)
    {
        SetAttribute(measurement, QStringLiteral("fullName"), m->GetGuiText());

        QString formula = m->GetFormula();
        if (not formula.isEmpty())
        {
            SetAttribute(measurement, QStringLiteral("formula"), m->GetFormula());
        }
        else
        {
            SetAttribute(measurement, QStringLiteral("formula"), *m->GetValue());
        }
        SetAttribute(measurement, QStringLiteral("value"), *m->GetValue());
    }
    else
    {
        SetAttribute(measurement, QStringLiteral("separator"), true);
    }

    return measurement;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::Increments() -> QDomElement
{
    QDomElement increments = createElement(QStringLiteral("increments"));

    VContainer data = m_pattern->GetCompleteData();
    QList<QSharedPointer<VIncrement>> patternIncrements = data.DataIncrementsWithSeparators().values();

    // Resore order
    std::sort(patternIncrements.begin(), patternIncrements.end(),
              [](const QSharedPointer<VIncrement> &a, const QSharedPointer<VIncrement> &b)
              { return a->GetIndex() < b->GetIndex(); });

    for (auto &incr : patternIncrements)
    {
        if (not incr->IsPreviewCalculation())
        {
            increments.appendChild(Increment(incr));
        }
    }

    return increments;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::PreviewCalculations() -> QDomElement
{
    QDomElement previewCalculations = createElement(QStringLiteral("previewCalculations"));

    VContainer data = m_pattern->GetCompleteData();
    QList<QSharedPointer<VIncrement>> patternIncrements = data.DataIncrementsWithSeparators().values();

    // Resore order
    std::sort(patternIncrements.begin(), patternIncrements.end(),
              [](const QSharedPointer<VIncrement> &a, const QSharedPointer<VIncrement> &b)
              { return a->GetIndex() < b->GetIndex(); });

    for (auto &incr : patternIncrements)
    {
        if (incr->IsPreviewCalculation())
        {
            previewCalculations.appendChild(Increment(incr));
        }
    }

    return previewCalculations;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::Increment(const QSharedPointer<VIncrement> &incr) -> QDomElement
{
    QDomElement measurement = createElement(QStringLiteral("increment"));

    SetAttribute(measurement, QStringLiteral("description"), incr->GetDescription());
    SetAttribute(measurement, QStringLiteral("name"), incr->GetName());

    if (incr->GetType() != VarType::IncrementSeparator)
    {
        SetAttribute(measurement, QStringLiteral("formula"), incr->GetFormula()); // TODO: localize
        SetAttribute(measurement, QStringLiteral("value"), *incr->GetValue());
    }
    else
    {
        SetAttribute(measurement, QStringLiteral("separator"), true);
    }

    return measurement;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::Content() -> QDomElement
{
    QDomElement content = createElement(QStringLiteral("content"));

    const QDomNodeList draws = m_pattern->documentElement().elementsByTagName(VAbstractPattern::TagDraw);
    for (int i = 0; i < draws.size(); ++i)
    {
        QDomElement draw = draws.at(i).toElement();
        if (draw.isNull())
        {
            throw VExceptionInvalidHistory(tr("Invalid tag %1").arg(VAbstractPattern::TagDraw));
        }

        content.appendChild(Draft(draw));
    }

    content.appendChild(FinalMeasurements());

    return content;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::Draft(const QDomElement &draft) -> QDomElement
{
    QDomElement recipeDraft = createElement(QStringLiteral("draft"));

    const QString draftName = draft.attribute(QStringLiteral("name"));
    SetAttribute(recipeDraft, QStringLiteral("name"), draftName);

    VContainer data = m_pattern->GetCompletePPData(draftName);

    QVector<VToolRecord> *history = m_pattern->getHistory();
    for (auto &record : *history)
    {
        if (record.getNameDraw() == draftName)
        {
            QDomElement step = Step(record, data);
            if (not step.isNull())
            {
                recipeDraft.appendChild(step);
            }
        }
    }

    return recipeDraft;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::Step(const VToolRecord &tool, const VContainer &data) -> QDomElement
{
    // This check helps to find missed tools in the switch
    Q_STATIC_ASSERT_X(static_cast<int>(Tool::LAST_ONE_DO_NOT_USE) == 59, "Not all tools were used in history.");

    const QDomElement domElem = m_pattern->elementById(tool.getId());
    if (not domElem.isElement() && tool.IsMandatory())
    {
        throw VExceptionInvalidHistory(tr("Can't find element by id '%1'").arg(tool.getId()));
    }
    try
    {
        QT_WARNING_PUSH
        QT_WARNING_DISABLE_GCC("-Wswitch-default")
        switch (tool.getTypeTool())
        {
            case Tool::Arrow:
            case Tool::SinglePoint:
            case Tool::DoublePoint:
            case Tool::LinePoint:
            case Tool::AbstractSpline:
            case Tool::Cut:
            case Tool::Midpoint:         // Same as Tool::AlongLine, but tool will never has such type
            case Tool::ArcIntersectAxis: // Same as Tool::CurveIntersectAxis, but tool will never has such type
            case Tool::BackgroundImage:
            case Tool::BackgroundImageControls:
            case Tool::BackgroundPixmapImage:
            case Tool::BackgroundSVGImage:
            case Tool::LAST_ONE_DO_NOT_USE:
                Q_UNREACHABLE(); //-V501
                break;
            case Tool::BasePoint:
                return BasePoint(tool);
            case Tool::EndLine:
                return EndLine(tool);
            case Tool::Line:
                return Line(tool);
            case Tool::AlongLine:
                return AlongLine(tool);
            case Tool::ShoulderPoint:
                return ShoulderPoint(tool);
            case Tool::Normal:
                return Normal(tool);
            case Tool::Bisector:
                return Bisector(tool);
            case Tool::LineIntersect:
                return LineIntersect(tool);
            case Tool::Spline:
                return Spline(tool);
            case Tool::CubicBezier:
                return CubicBezier(tool);
            case Tool::Arc:
                return Arc(tool);
            case Tool::ArcWithLength:
                return ArcWithLength(tool);
            case Tool::SplinePath:
                return SplinePath(tool);
            case Tool::CubicBezierPath:
                return CubicBezierPath(tool);
            case Tool::PointOfContact:
                return PointOfContact(tool);
            case Tool::Height:
                return Height(tool);
            case Tool::Triangle:
                return Triangle(tool);
            case Tool::PointOfIntersection:
                return PointOfIntersection(tool);
            case Tool::CutArc:
                return CutArc(tool);
            case Tool::CutSpline:
                return CutSpline(tool);
            case Tool::CutSplinePath:
                return CutSplinePath(tool);
            case Tool::LineIntersectAxis:
                return LineIntersectAxis(tool);
            case Tool::CurveIntersectAxis:
                return CurveIntersectAxis(tool);
            case Tool::PointOfIntersectionArcs:
                return PointOfIntersectionArcs(tool);
            case Tool::PointOfIntersectionCircles:
                return PointOfIntersectionCircles(tool);
            case Tool::PointOfIntersectionCurves:
                return PointOfIntersectionCurves(tool);
            case Tool::PointFromCircleAndTangent:
                return PointFromCircleAndTangent(tool);
            case Tool::PointFromArcAndTangent:
                return PointFromArcAndTangent(tool);
            case Tool::TrueDarts:
                return TrueDarts(tool);
            case Tool::EllipticalArc:
                return EllipticalArc(tool);
            case Tool::Rotation:
                return Rotation(tool, data);
            case Tool::FlippingByLine:
                return FlippingByLine(tool, data);
            case Tool::FlippingByAxis:
                return FlippingByAxis(tool, data);
            case Tool::Move:
                return Move(tool, data);
            // Because "history" not only show history of pattern, but help restore current data for each pattern's
            // piece, we need add record about details and nodes, but don't show them.
            case Tool::Piece:
            case Tool::UnionDetails:
            case Tool::NodeArc:
            case Tool::NodeElArc:
            case Tool::NodePoint:
            case Tool::NodeSpline:
            case Tool::NodeSplinePath:
            case Tool::Group:
            case Tool::PiecePath:
            case Tool::Pin:
            case Tool::PlaceLabel:
            case Tool::InsertNode:
            case Tool::DuplicateDetail:
                return QDomElement();
        }
        QT_WARNING_POP
    }
    catch (const VExceptionBadId &e)
    {
        throw VExceptionInvalidHistory(e.ErrorMessage());
    }

    throw VExceptionInvalidHistory(tr("Can't create history record for the tool."));
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::FinalMeasurements() -> QDomElement
{
    QDomElement recipeFinalMeasurements = createElement(QStringLiteral("finalMeasurements"));

    const QVector<VFinalMeasurement> measurements = m_pattern->GetFinalMeasurements();
    VContainer data = m_pattern->GetCompleteData();

    for (const auto &m : measurements)
    {
        recipeFinalMeasurements.appendChild(FinalMeasurement(m, data));
    }

    return recipeFinalMeasurements;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::FinalMeasurement(const VFinalMeasurement &fm, const VContainer &data) -> QDomElement
{
    QDomElement recipeFinalMeasurement = createElement(QStringLiteral("finalMeasurement"));

    SetAttribute(recipeFinalMeasurement, QStringLiteral("description"), fm.description);
    SetAttribute(recipeFinalMeasurement, QStringLiteral("name"), fm.name);
    SetAttribute(recipeFinalMeasurement, QStringLiteral("formula"), fm.formula); // TODO: localize

    QScopedPointer<Calculator> cal(new Calculator());
    try
    {
        const qreal result = cal->EvalFormula(data.DataVariables(), fm.formula);
        if (qIsInf(result) || qIsNaN(result))
        {
            const QString errorMsg = u"%1\n\n%1"_s.arg(tr("Reading final measurements error."),
                                                       tr("Value for final measurtement '%1' is infinite or NaN. "
                                                          "Please, check your calculations.")
                                                           .arg(fm.name));
            VAbstractApplication::VApp()->IsPedantic()
                ? throw VException(errorMsg)
                : qWarning() << VAbstractValApplication::warningMessageSignature + errorMsg;
        }

        SetAttribute(recipeFinalMeasurement, QStringLiteral("value"), result);
    }
    catch (const qmu::QmuParserError &e)
    {
        throw VExceptionInvalidHistory(
            tr("Unable to create record for final measurement '%1'. Error: %2").arg(fm.name).arg(e.GetMsg()));
    }

    return recipeFinalMeasurement;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::BasePoint(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolBasePoint>(record.getId());

    QDomElement step = createElement(TagStep);
    ToolAttributes(step, tool);
    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::EndLine(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolEndLine>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    SetAttribute(step, AttrBasePoint, tool->BasePointName());
    Formula(step, tool->GetFormulaLength(), AttrLength, AttrLengthValue);
    Formula(step, tool->GetFormulaAngle(), AttrAngle, AttrAngleValue);
    LineAttributes(step, tool);

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::Line(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolLine>(record.getId());

    QDomElement step = createElement(TagStep);

    SetAttribute(step, AttrType, QStringLiteral("segment"));
    SetAttribute(step, AttrFirstPoint, tool->FirstPointName());
    SetAttribute(step, AttrSecondPoint, tool->SecondPointName());
    LineAttributes(step, tool);

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::AlongLine(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolAlongLine>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    SetAttribute(step, AttrBasePoint, tool->BasePointName());
    SetAttribute(step, AttrSecondPoint, tool->SecondPointName());
    Formula(step, tool->GetFormulaLength(), AttrLength, AttrLengthValue);
    LineAttributes(step, tool);

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::ShoulderPoint(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolShoulderPoint>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    SetAttribute(step, AttrP1Line, tool->BasePointName());
    SetAttribute(step, AttrP2Line, tool->SecondPointName());
    SetAttribute(step, AttrPShoulder, tool->ShoulderPointName());
    Formula(step, tool->GetFormulaLength(), AttrLength, AttrLengthValue);
    LineAttributes(step, tool);

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::Normal(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolNormal>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    SetAttribute(step, AttrFirstPoint, tool->BasePointName());
    SetAttribute(step, AttrSecondPoint, tool->SecondPointName());
    Formula(step, tool->GetFormulaLength(), AttrLength, AttrLengthValue);
    SetAttribute(step, AttrAngle, tool->GetAngle());
    LineAttributes(step, tool);

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::Bisector(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolBisector>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    SetAttribute(step, AttrFirstPoint, tool->FirstPointName());
    SetAttribute(step, AttrSecondPoint, tool->BasePointName());
    SetAttribute(step, AttrThirdPoint, tool->ThirdPointName());
    Formula(step, tool->GetFormulaLength(), AttrLength, AttrLengthValue);
    LineAttributes(step, tool);

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::LineIntersect(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolLineIntersect>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);

    SetAttribute(step, AttrP1Line1, tool->Line1P1Name());
    SetAttribute(step, AttrP2Line1, tool->Line1P2Name());
    SetAttribute(step, AttrP1Line2, tool->Line2P1Name());
    SetAttribute(step, AttrP2Line2, tool->Line2P2Name());

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::Spline(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolSpline>(record.getId());
    VSpline spl = tool->getSpline();

    QDomElement step = createElement(TagStep);

    SetAttribute(step, AttrType, QStringLiteral("spline"));
    SetAttribute(step, AttrLabel, tool->name());

    SetAttribute(step, AttrPoint1, spl.GetP1().name());
    SetAttribute(step, AttrPoint4, spl.GetP4().name());

    SetAttribute(step, AttrAngle1, spl.GetStartAngleFormula());
    SetAttribute(step, AttrAngle1Value, spl.GetStartAngle());

    SetAttribute(step, AttrAngle2, spl.GetEndAngleFormula());
    SetAttribute(step, AttrAngle2Value, spl.GetEndAngle());

    SetAttribute(step, AttrLength1, spl.GetC1LengthFormula());
    SetAttribute(step, AttrLength1Value, VAbstractValApplication::VApp()->fromPixel(spl.GetC1Length()));

    SetAttribute(step, AttrLength2, spl.GetC2LengthFormula());
    SetAttribute(step, AttrLength2Value, VAbstractValApplication::VApp()->fromPixel(spl.GetC2Length()));

    CurveAttributes(step, tool);

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::CubicBezier(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolCubicBezier>(record.getId());
    VCubicBezier spl = tool->getSpline();

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    SetAttribute(step, AttrPoint1, spl.GetP1().name());
    SetAttribute(step, AttrPoint2, spl.GetP2().name());
    SetAttribute(step, AttrPoint3, spl.GetP3().name());
    SetAttribute(step, AttrPoint4, spl.GetP4().name());

    CurveAttributes(step, tool);

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::Arc(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolArc>(record.getId());

    QDomElement step = createElement(TagStep);

    SetAttribute(step, AttrType, QStringLiteral("arc"));
    SetAttribute(step, AttrLabel, tool->name());
    SetAttribute(step, AttrCenter, tool->CenterPointName());
    Formula(step, tool->GetFormulaRadius(), AttrRadius, AttrRadiusValue);
    Formula(step, tool->GetFormulaF1(), AttrAngle1, AttrAngle1Value);
    Formula(step, tool->GetFormulaF2(), AttrAngle2, AttrAngle2Value);

    CurveAttributes(step, tool);

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::ArcWithLength(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolArcWithLength>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    SetAttribute(step, AttrCenter, tool->CenterPointName());
    Formula(step, tool->GetFormulaRadius(), AttrRadius, AttrRadiusValue);
    Formula(step, tool->GetFormulaF1(), AttrAngle1, AttrAngle1Value);
    Formula(step, tool->GetFormulaLength(), AttrLength, AttrLengthValue);

    CurveAttributes(step, tool);

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::SplinePath(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolSplinePath>(record.getId());
    VSplinePath spl = tool->getSplinePath();

    QDomElement step = createElement(TagStep);

    SetAttribute(step, AttrType, QStringLiteral("splinePath"));
    SetAttribute(step, AttrLabel, tool->name());

    QDomElement nodes = createElement(QStringLiteral("nodes"));
    QVector<VSplinePoint> path = spl.GetSplinePath();

    if (path.isEmpty())
    {
        throw VExceptionInvalidHistory(QObject::tr("Empty list of nodes for tool with id '%1'.").arg(record.getId()));
    }

    for (auto &pathNode : path)
    {
        QDomElement node = createElement(QStringLiteral("node"));

        SetAttribute(node, AttrPSpline, pathNode.P().name());

        SetAttribute(node, AttrAngle1, pathNode.Angle1Formula());
        SetAttribute(node, AttrAngle1Value, pathNode.Angle1());

        SetAttribute(node, AttrAngle2, pathNode.Angle2Formula());
        SetAttribute(node, AttrAngle2Value, pathNode.Angle2());

        SetAttribute(node, AttrLength1, pathNode.Length1Formula());
        SetAttribute(node, AttrLength1Value, VAbstractValApplication::VApp()->fromPixel(pathNode.Length1()));

        SetAttribute(node, AttrLength2, pathNode.Length2Formula());
        SetAttribute(node, AttrLength2Value, VAbstractValApplication::VApp()->fromPixel(pathNode.Length2()));

        nodes.appendChild(node);
    }

    step.appendChild(nodes);

    CurveAttributes(step, tool);

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::CubicBezierPath(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolCubicBezierPath>(record.getId());
    VCubicBezierPath spl = tool->getSplinePath();

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);

    QDomElement nodes = createElement(QStringLiteral("nodes"));
    QVector<VSplinePoint> path = spl.GetSplinePath();

    if (path.isEmpty())
    {
        throw VExceptionInvalidHistory(QObject::tr("Empty list of nodes for tool with id '%1'.").arg(record.getId()));
    }

    for (auto &pathNode : path)
    {
        QDomElement node = createElement(QStringLiteral("node"));

        SetAttribute(node, AttrPSpline, pathNode.P().name());

        nodes.appendChild(node);
    }

    step.appendChild(nodes);

    CurveAttributes(step, tool);

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::PointOfContact(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolPointOfContact>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    SetAttribute(step, AttrCenter, tool->ArcCenterPointName());
    SetAttribute(step, AttrFirstPoint, tool->FirstPointName());
    SetAttribute(step, AttrSecondPoint, tool->SecondPointName());
    Formula(step, tool->getArcRadius(), AttrRadius, AttrRadiusValue);

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::Height(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolHeight>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    SetAttribute(step, AttrBasePoint, tool->BasePointName());
    SetAttribute(step, AttrP1Line, tool->FirstLinePointName());
    SetAttribute(step, AttrP2Line, tool->SecondLinePointName());
    LineAttributes(step, tool);

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::Triangle(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolTriangle>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    SetAttribute(step, AttrAxisP1, tool->AxisP1Name());
    SetAttribute(step, AttrAxisP2, tool->AxisP2Name());
    SetAttribute(step, AttrFirstPoint, tool->FirstPointName());
    SetAttribute(step, AttrSecondPoint, tool->SecondPointName());

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::PointOfIntersection(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolPointOfIntersection>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    SetAttribute(step, AttrFirstPoint, tool->FirstPointName());
    SetAttribute(step, AttrSecondPoint, tool->SecondPointName());

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::CutArc(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolCutArc>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    Formula(step, tool->GetFormulaLength(), AttrLength, AttrLengthValue);
    SetAttribute(step, AttrArc, tool->CurveName());

    CutCurveAttributes(step, tool);

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::CutSpline(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolCutSpline>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    Formula(step, tool->GetFormulaLength(), AttrLength, AttrLengthValue);
    SetAttribute(step, VToolCutSpline::AttrSpline, tool->CurveName());

    CutCurveAttributes(step, tool);

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::CutSplinePath(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolCutSplinePath>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    Formula(step, tool->GetFormulaLength(), AttrLength, AttrLengthValue);
    SetAttribute(step, VToolCutSplinePath::AttrSplinePath, tool->CurveName());

    CutCurveAttributes(step, tool);

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::LineIntersectAxis(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolLineIntersectAxis>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    SetAttribute(step, AttrBasePoint, tool->BasePointName());
    SetAttribute(step, AttrP1Line, tool->FirstLinePoint());
    SetAttribute(step, AttrP2Line, tool->SecondLinePoint());
    Formula(step, tool->GetFormulaAngle(), AttrAngle, AttrAngleValue);
    LineAttributes(step, tool);

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::CurveIntersectAxis(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolCurveIntersectAxis>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    SetAttribute(step, AttrBasePoint, tool->BasePointName());
    SetAttribute(step, AttrCurve, tool->CurveName());
    Formula(step, tool->GetFormulaAngle(), AttrAngle, AttrAngleValue);
    LineAttributes(step, tool);

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::PointOfIntersectionArcs(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolPointOfIntersectionArcs>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    SetAttribute(step, AttrFirstArc, tool->FirstArcName());
    SetAttribute(step, AttrSecondArc, tool->SecondArcName());
    SetAttribute(step, AttrCrossPoint, static_cast<int>(tool->GetCrossCirclesPoint()));

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::PointOfIntersectionCircles(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolPointOfIntersectionCircles>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    SetAttribute(step, AttrC1Center, tool->FirstCircleCenterPointName());
    SetAttribute(step, AttrC2Center, tool->SecondCircleCenterPointName());
    Formula(step, tool->GetFirstCircleRadius(), AttrC1Radius, AttrC1RadiusValue);
    Formula(step, tool->GetSecondCircleRadius(), AttrC2Radius, AttrC2RadiusValue);
    SetAttribute(step, AttrCrossPoint, static_cast<int>(tool->GetCrossCirclesPoint()));

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::PointOfIntersectionCurves(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolPointOfIntersectionCurves>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    SetAttribute(step, AttrCurve1, tool->FirstCurveName());
    SetAttribute(step, AttrCurve2, tool->SecondCurveName());
    SetAttribute(step, AttrVCrossPoint, static_cast<int>(tool->GetVCrossPoint()));
    SetAttribute(step, AttrHCrossPoint, static_cast<int>(tool->GetHCrossPoint()));

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::PointFromCircleAndTangent(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolPointFromCircleAndTangent>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    SetAttribute(step, AttrCCenter, tool->CircleCenterPointName());
    SetAttribute(step, AttrTangent, tool->TangentPointName());
    Formula(step, tool->GetCircleRadius(), AttrCRadius, AttrCRadiusValue);
    SetAttribute(step, AttrCrossPoint, static_cast<int>(tool->GetCrossCirclesPoint()));

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::PointFromArcAndTangent(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolPointFromArcAndTangent>(record.getId());

    QDomElement step = createElement(TagStep);

    ToolAttributes(step, tool);
    SetAttribute(step, AttrArc, tool->ArcName());
    SetAttribute(step, AttrTangent, tool->TangentPointName());
    SetAttribute(step, AttrCrossPoint, static_cast<int>(tool->GetCrossCirclesPoint()));

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::TrueDarts(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolTrueDarts>(record.getId());

    QDomElement step = createElement(TagStep);

    SetAttribute(step, AttrType, VToolTrueDarts::ToolType);
    SetAttribute(step, AttrPoint1, tool->nameP1());
    SetAttribute(step, AttrPoint2, tool->nameP2());
    SetAttribute(step, AttrBaseLineP1, tool->BaseLineP1Name());
    SetAttribute(step, AttrBaseLineP2, tool->BaseLineP2Name());
    SetAttribute(step, AttrDartP1, tool->DartP1Name());
    SetAttribute(step, AttrDartP2, tool->DartP2Name());
    SetAttribute(step, AttrDartP3, tool->DartP3Name());

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::EllipticalArc(const VToolRecord &record) -> QDomElement
{
    auto *tool = GetPatternTool<VToolEllipticalArc>(record.getId());

    QDomElement step = createElement(TagStep);

    SetAttribute(step, AttrType, QStringLiteral("ellipticalArc"));
    SetAttribute(step, AttrLabel, tool->name());

    SetAttribute(step, AttrCenter, tool->CenterPointName());
    Formula(step, tool->GetFormulaRadius1(), AttrRadius1, AttrRadius1Value);
    Formula(step, tool->GetFormulaRadius2(), AttrRadius2, AttrRadius2Value);
    Formula(step, tool->GetFormulaF1(), AttrAngle1, AttrAngle1Value);
    Formula(step, tool->GetFormulaF2(), AttrAngle2, AttrAngle2Value);
    Formula(step, tool->GetFormulaRotationAngle(), AttrRotationAngle, AttrRotationAngleValue);

    CurveAttributes(step, tool);

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::Rotation(const VToolRecord &record, const VContainer &data) -> QDomElement
{
    auto *tool = GetPatternTool<VToolRotation>(record.getId());

    QDomElement step = createElement(TagStep);

    SetAttribute(step, AttrType, VToolRotation::ToolType);
    SetAttribute(step, AttrCenter, tool->OriginPointName());
    Formula(step, tool->GetFormulaAngle(), AttrAngle, AttrAngleValue);
    SetAttribute(step, AttrSuffix, tool->Suffix());

    step.appendChild(GroupOperationSource(tool, record.getId(), data));

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::FlippingByLine(const VToolRecord &record, const VContainer &data) -> QDomElement
{
    auto *tool = GetPatternTool<VToolFlippingByLine>(record.getId());

    QDomElement step = createElement(TagStep);

    SetAttribute(step, AttrType, VToolFlippingByLine::ToolType);
    SetAttribute(step, AttrP1Line, tool->FirstLinePointName());
    SetAttribute(step, AttrP2Line, tool->SecondLinePointName());
    SetAttribute(step, AttrSuffix, tool->Suffix());

    step.appendChild(GroupOperationSource(tool, record.getId(), data));

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::FlippingByAxis(const VToolRecord &record, const VContainer &data) -> QDomElement
{
    auto *tool = GetPatternTool<VToolFlippingByAxis>(record.getId());

    QDomElement step = createElement(TagStep);

    SetAttribute(step, AttrType, VToolFlippingByAxis::ToolType);
    SetAttribute(step, AttrCenter, tool->OriginPointName());
    SetAttribute(step, AttrAxisType, static_cast<int>(tool->GetAxisType()));
    SetAttribute(step, AttrSuffix, tool->Suffix());

    step.appendChild(GroupOperationSource(tool, record.getId(), data));

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::Move(const VToolRecord &record, const VContainer &data) -> QDomElement
{
    auto *tool = GetPatternTool<VToolMove>(record.getId());

    QDomElement step = createElement(TagStep);

    SetAttribute(step, AttrType, VToolMove::ToolType);
    Formula(step, tool->GetFormulaAngle(), AttrAngle, AttrAngleValue);
    Formula(step, tool->GetFormulaRotationAngle(), AttrRotationAngle, AttrRotationAngleValue);
    Formula(step, tool->GetFormulaLength(), AttrLength, AttrLengthValue);
    SetAttribute(step, AttrCenter, tool->OriginPointName());
    SetAttribute(step, AttrSuffix, tool->Suffix());

    step.appendChild(GroupOperationSource(tool, record.getId(), data));

    return step;
}

//---------------------------------------------------------------------------------------------------------------------
inline void VPatternRecipe::Formula(QDomElement &step, const VFormula &formula, const QString &formulaStr,
                                    const QString &formulaValue)
{
    if (formula.error())
    {
        throw VExceptionInvalidHistory(QObject::tr("Invalid formula '%1' for tool with id '%2'. %3.")
                                           .arg(formula.GetFormula(FormulaType::ToSystem))
                                           .arg(formula.getToolId())
                                           .arg(formula.Reason()));
    }

    SetAttribute(step, formulaStr, formula.GetFormula(FormulaType::ToSystem));
    SetAttribute(step, formulaValue, formula.getDoubleValue());
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline void VPatternRecipe::LineAttributes(QDomElement &step, T *tool)
{
    SetAttribute(step, AttrLineColor, tool->GetLineColor());
    SetAttribute(step, AttrTypeLine, tool->getLineType());
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> void VPatternRecipe::CurveAttributes(QDomElement &step, T *tool)
{
    SetAttribute(step, AttrLineColor, tool->GetLineColor());
    SetAttribute(step, AttrPenStyle, tool->GetPenStyle());
    SetAttribute(step, AttrAScale, tool->GetApproximationScale());
    SetAttribute(step, AttrDuplicate, tool->GetDuplicate());
    SetAttributeOrRemoveIf<QString>(step, AttrAlias, tool->GetAliasSuffix(),
                                    [](const QString &suffix) noexcept { return suffix.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> void VPatternRecipe::CutCurveAttributes(QDomElement &step, T *tool)
{
    SetAttributeOrRemoveIf<QString>(step, AttrAlias1, tool->GetAliasSuffix1(),
                                    [](const QString &suffix) noexcept { return suffix.isEmpty(); });
    SetAttributeOrRemoveIf<QString>(step, AttrAlias2, tool->GetAliasSuffix2(),
                                    [](const QString &suffix) noexcept { return suffix.isEmpty(); });
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline void VPatternRecipe::ToolAttributes(QDomElement &step, T *tool)
{
    SetAttribute(step, AttrType, T::ToolType);
    SetAttribute(step, AttrLabel, tool->name());
    SetAttribute(step, AttrNotes, tool->GetNotes());
}

//---------------------------------------------------------------------------------------------------------------------
auto VPatternRecipe::GroupOperationSource(VAbstractOperation *tool, quint32 id, const VContainer &data) -> QDomElement
{
    SCASSERT(tool)

    QDomElement nodes = createElement(QStringLiteral("nodes"));
    QVector<SourceItem> items = tool->SourceItems();

    if (items.isEmpty())
    {
        throw VExceptionInvalidHistory(QObject::tr("Empty list of nodes for tool with id '%1'.").arg(id));
    }

    for (auto &item : items)
    {
        QDomElement node = createElement(QStringLiteral("node"));

        QSharedPointer<VGObject> obj;

        try
        {
            obj = data.GetGObject(item.id);
        }
        catch (const VExceptionBadId &e)
        {
            qCritical() << e.ErrorMessage() << Q_FUNC_INFO;
            continue;
        }

        SetAttribute(node, AttrItem, obj->ObjectName());
        SetAttributeOrRemoveIf<QString>(node, AttrAlias, item.alias,
                                        [](const QString &alias) noexcept { return alias.isEmpty(); });

        if (obj->getType() != GOType::Point)
        {
            if (item.penStyle != TypeLineDefault)
            {
                SetAttribute(node, AttrPenStyle, item.penStyle);
            }

            if (item.color != ColorDefault)
            {
                SetAttribute(node, AttrColor, item.color);
            }
        }

        QT_WARNING_PUSH
        QT_WARNING_DISABLE_GCC("-Wswitch-default")
        switch (static_cast<GOType>(obj->getType()))
        {
            case GOType::Point:
                SetAttribute(node, AttrType, QStringLiteral("point"));
                break;
            case GOType::Arc:
                SetAttribute(node, AttrType, QStringLiteral("arc"));
                break;
            case GOType::EllipticalArc:
                SetAttribute(node, AttrType, QStringLiteral("elArc"));
                break;
            case GOType::Spline:
            case GOType::CubicBezier:
                SetAttribute(node, AttrType, QStringLiteral("spline"));
                break;
            case GOType::SplinePath:
            case GOType::CubicBezierPath:
                SetAttribute(node, AttrType, QStringLiteral("splinePath"));
                break;
            case GOType::Unknown:
            case GOType::PlaceLabel:
                Q_UNREACHABLE();
                break;
        }
        QT_WARNING_POP

        nodes.appendChild(node);
    }

    return nodes;
}
