/************************************************************************
 **
 **  @file   tst_vabstractpattern.cpp
 **  @author Paco Arjonilla <pacoarjonilla(at)yahoo.es>
 **  @date   15 7, 2026
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2026 Valentina project
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

#include "tst_vabstractpattern.h"
#include "../ifc/xml/vabstractpattern.h"
#include "../ifc/xml/vpatterngraph.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vgeometry/vsplinepath.h"
#include "../vgeometry/vsplinepoint.h"
#include "../vmisc/vabstractvalapplication.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolspline.h"
#include "../vtools/tools/drawTools/toolcurve/vtoolsplinepath.h"

#include <QElapsedTimer>
#include <QThreadPool>
#include <QtTest>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
constexpr int tokenCount = 20000;
constexpr int referencesPerToken = 20;
constexpr quint32 formulaOwnerId = 1;
constexpr quint32 firstReferenceId = 1000;

//---------------------------------------------------------------------------------------------------------------------
// Minimal concrete document. The dependency-check machinery lives in VAbstractPattern itself.
class TestDoc : public VAbstractPattern
{
public:
    explicit TestDoc(QObject *parent = nullptr)
      : VAbstractPattern(parent)
    {
    }

    void CreateEmptyFile() override {}
    auto GenerateLabel(const LabelType &type, const QString &reservedName = QString()) const -> QString override
    {
        Q_UNUSED(type)
        Q_UNUSED(reservedName)
        return {};
    }
    void UpdateToolData(const quint32 &id, VContainer *data) override
    {
        Q_UNUSED(id)
        Q_UNUSED(data)
    }
    void LiteParseTree(const Document &parse) override { Q_UNUSED(parse) }

    using VAbstractPattern::CancelFormulaDependencyChecks;
};

//---------------------------------------------------------------------------------------------------------------------
auto BuildVariables() -> QHash<QString, QList<quint32>>
{
    QHash<QString, QList<quint32>> variables;
    variables.reserve(tokenCount);
    quint32 referenceId = firstReferenceId;
    for (int i = 0; i < tokenCount; ++i)
    {
        QList<quint32> references;
        references.reserve(referencesPerToken);
        for (int j = 0; j < referencesPerToken; ++j)
        {
            references.append(referenceId++);
        }
        variables.insert(u"v%1"_s.arg(i), references);
    }
    return variables;
}

//---------------------------------------------------------------------------------------------------------------------
auto BuildFormula() -> QString
{
    QStringList terms;
    terms.reserve(tokenCount);
    for (int i = 0; i < tokenCount; ++i)
    {
        terms.append(u"v%1"_s.arg(i));
    }
    return terms.join('+'_L1);
}

//---------------------------------------------------------------------------------------------------------------------
void AddVertices(VPatternGraph *graph)
{
    graph->AddVertex(formulaOwnerId, VNodeType::MODELING_OBJECT, 0);
    const auto lastReferenceId =
        firstReferenceId + static_cast<quint32>(tokenCount) * static_cast<quint32>(referencesPerToken);
    for (quint32 id = firstReferenceId; id < lastReferenceId; ++id)
    {
        graph->AddVertex(id, VNodeType::MODELING_OBJECT, 0);
    }
}

//---------------------------------------------------------------------------------------------------------------------
// Busy-wait until the worker starts adding edges so cancellation happens mid-run.
auto WaitForFirstEdge(const VPatternGraph *graph) -> bool
{
    QElapsedTimer timer;
    timer.start();
    while (graph->EdgeCount() == 0 && timer.elapsed() < 10000)
    {
    }
    return graph->EdgeCount() > 0;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
TST_VAbstractPattern::TST_VAbstractPattern(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
// A worker in the middle of a large dependency check must stop early on cancellation instead of
// finishing its full workload.
void TST_VAbstractPattern::CancelStopsRunningWorkers()
{
    TestDoc doc;
    VPatternGraph *graph = doc.PatternGraph();
    QVERIFY(graph != nullptr);

    AddVertices(graph);
    doc.FindFormulaDependencies(BuildFormula(), formulaOwnerId, BuildVariables());

    QVERIFY(WaitForFirstEdge(graph));
    doc.CancelFormulaDependencyChecks();
    QThreadPool::globalInstance()->waitForDone();

    const auto totalEdges = static_cast<std::size_t>(tokenCount) * static_cast<std::size_t>(referencesPerToken);
    QVERIFY2(graph->EdgeCount() < totalEdges / 2,
             qUtf8Printable(u"Worker was not cancelled: %1 of %2 edges were added"_s.arg(graph->EdgeCount())
                                .arg(totalEdges)));
}

//---------------------------------------------------------------------------------------------------------------------
// Clearing a document while a dependency check is running must not leak edges into the graph once the
// next document re-adds vertices with the same ids.
void TST_VAbstractPattern::ClearCancelsPendingWorkers()
{
    TestDoc doc;
    VPatternGraph *graph = doc.PatternGraph();
    QVERIFY(graph != nullptr);

    AddVertices(graph);
    doc.FindFormulaDependencies(BuildFormula(), formulaOwnerId, BuildVariables());

    QVERIFY(WaitForFirstEdge(graph));
    doc.Clear();
    AddVertices(graph);
    QThreadPool::globalInstance()->waitForDone();

    QCOMPARE(graph->EdgeCount(), static_cast<std::size_t>(0));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief MaxRecordedIdCountsOrphanedNodes a full parse must not reissue an id that only survives on an
 * orphaned node.
 *
 * When a node's source object is deleted, the node element itself is left behind in the document (its
 * idObject no longer resolves), and parsing it just skips recreating a live tool for it. MaxRecordedId() feeds
 * the id generator after a full-parse reset, so it must count that element's id too, or a freshly minted id
 * can collide with it -- exactly the crash this test is standing in for.
 */
void TST_VAbstractPattern::MaxRecordedIdCountsOrphanedNodes()
{
    TestDoc doc;
    QVERIFY(doc.setContent(QByteArray(R"(
        <pattern>
            <draw name="Block A">
                <calculation>
                    <point id="5" type="single"/>
                    <line id="10" firstPoint="5" secondPoint="5"/>
                </calculation>
                <modeling>
                    <spline id="161" idObject="78" inUse="false" type="modelingSpline"/>
                </modeling>
            </draw>
        </pattern>)")));

    doc.RefreshElementIdCache();

    QCOMPARE(doc.MaxRecordedId(), static_cast<quint32>(161));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPattern::MaxRecordedIdOnEmptyDocumentIsZero()
{
    TestDoc doc;
    QVERIFY(doc.setContent(QByteArray(R"(<pattern><draw name="Block A"/></pattern>)")));

    doc.RefreshElementIdCache();

    QCOMPARE(doc.MaxRecordedId(), static_cast<quint32>(NULL_ID));
}

//---------------------------------------------------------------------------------------------------------------------
// Increment rename and the "used" check go through ListExpressions(); buffer formulas must be part of it.
void TST_VAbstractPattern::ListExpressionsIncludesBufferFormulas()
{
    TestDoc doc;
    QVERIFY(doc.setContent(QByteArray(R"(
        <pattern>
            <draw name="Block A">
                <details>
                    <detail id="9" width="1" bufferVisible="#show" bufferWidth="#buffer"/>
                </details>
            </draw>
        </pattern>)")));

    QStringList formulas;
    for (const auto &field : doc.ListExpressions())
    {
        formulas.append(field.expression);
    }

    QVERIFY2(formulas.contains(u"#show"_s), qUtf8Printable(formulas.join(", "_L1)));
    QVERIFY2(formulas.contains(u"#buffer"_s), qUtf8Printable(formulas.join(", "_L1)));
}

namespace
{
//---------------------------------------------------------------------------------------------------------------------
auto ElementById(const QDomDocument &doc, int id) -> QDomElement
{
    for (QDomElement e = doc.documentElement().firstChildElement(); not e.isNull(); e = e.nextSiblingElement())
    {
        if (e.attribute(u"id"_s).toInt() == id)
        {
            return e;
        }
    }
    return {};
}

//---------------------------------------------------------------------------------------------------------------------
auto PointWithId(qreal x, qreal y, const QString &name, quint32 id) -> VPointF
{
    VPointF point(x, y, name, 0, 0);
    point.setId(id);
    return point;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
// A spline is in the old format until it has the length attributes. Older versions changed only the type when they
// converted a spline and saved such a file, so the type alone can't tell.
void TST_VAbstractPattern::IsOldFormatSplineDetection()
{
    TestDoc doc;
    QVERIFY(doc.setContent(QByteArray(R"(
        <root>
            <spline id="1" type="simple" angle1="1" angle2="2" kAsm1="1" kAsm2="1" kCurve="1" point1="1" point4="2"/>
            <spline id="2" type="simpleInteractive" angle1="1" angle2="2" kAsm1="1" kAsm2="1" kCurve="1" point1="1"
                    point4="2"/>
            <spline id="3" type="simpleInteractive" angle1="1" angle2="2" length1="3" length2="4" point1="1"
                    point4="2"/>
            <spline id="4" type="simpleInteractive" point1="1" point4="2"/>
        </root>)")));

    QVERIFY2(VAbstractPattern::IsOldFormatSpline(ElementById(doc, 1)), "old type");
    QVERIFY2(VAbstractPattern::IsOldFormatSpline(ElementById(doc, 2)), "new type, old attributes");
    QVERIFY2(not VAbstractPattern::IsOldFormatSpline(ElementById(doc, 3)), "new format");
    QVERIFY2(not VAbstractPattern::IsOldFormatSpline(ElementById(doc, 4)), "nothing of the old format");
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPattern::IsOldFormatSplinePathDetection()
{
    TestDoc doc;
    QVERIFY(doc.setContent(QByteArray(R"(
        <root>
            <spline id="1" type="path" kCurve="1">
                <pathPoint kAsm1="1" kAsm2="1" angle="10" pSpline="1"/>
                <pathPoint kAsm1="1" kAsm2="1" angle="20" pSpline="2"/>
                <pathPoint kAsm1="1" kAsm2="1" angle="30" pSpline="3"/>
            </spline>
            <spline id="2" type="pathInteractive" kCurve="1">
                <pathPoint kAsm1="1" kAsm2="1" angle="10" pSpline="1"/>
                <pathPoint kAsm1="1" kAsm2="1" angle="20" pSpline="2"/>
                <pathPoint kAsm1="1" kAsm2="1" angle="30" pSpline="3"/>
            </spline>
            <spline id="3" type="pathInteractive">
                <pathPoint length1="0" length2="1" angle1="10" angle2="190" pSpline="1"/>
                <pathPoint length1="1" length2="1" angle1="20" angle2="200" pSpline="2"/>
                <pathPoint length1="1" length2="0" angle1="30" angle2="210" pSpline="3"/>
            </spline>
        </root>)")));

    QVERIFY2(VAbstractPattern::IsOldFormatSplinePath(ElementById(doc, 1)), "old type");
    QVERIFY2(VAbstractPattern::IsOldFormatSplinePath(ElementById(doc, 2)), "new type, old attributes");
    QVERIFY2(not VAbstractPattern::IsOldFormatSplinePath(ElementById(doc, 3)), "new format");
}

//---------------------------------------------------------------------------------------------------------------------
// The converted spline must describe the same curve. With the old attributes dropped and no length attributes written
// the control points collapse into the end points, and the curve becomes a straight line.
void TST_VAbstractPattern::ConvertedSplineKeepsShape()
{
    const Unit unit = Unit::Cm;
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    TestDoc doc;
    QVERIFY(doc.setContent(QByteArray(R"(
        <root>
            <spline id="4" type="simple" angle1="229.381" angle2="41.6325" kAsm1="0.962941" kAsm2="1.00054" kCurve="1"
                    point1="3" point4="2"/>
        </root>)")));
    QDomElement element = ElementById(doc, 4);

    const VPointF p1 = PointWithId(ToPixel(30.926, unit), ToPixel(1.058, unit), u"A2"_s, 3);
    const VPointF p4 = PointWithId(ToPixel(18.03, unit), ToPixel(48.04, unit), u"A1"_s, 2);
    const VSpline spline(p1, p4, 229.381, 41.6325, 0.962941, 1.00054, 1.0);
    QVERIFY(QLineF(p1.toQPointF(), spline.GetP2().toQPointF()).length() > 1); // the curve is not a straight line

    VToolSpline::SetSplineAttributes(&doc, element, spline);

    QCOMPARE(element.attribute(u"type"_s), VToolSpline::ToolType);
    QCOMPARE(element.attribute(u"point1"_s).toUInt(), 3U);
    QCOMPARE(element.attribute(u"point4"_s).toUInt(), 2U);
    QVERIFY(not element.hasAttribute(u"kAsm1"_s));
    QVERIFY(not element.hasAttribute(u"kAsm2"_s));
    QVERIFY(not element.hasAttribute(u"kCurve"_s));
    QVERIFY(not VAbstractPattern::IsOldFormatSpline(element));

    // Rebuild the spline from what was written, the way the new format is read
    const VSpline loaded(p1,
                         p4,
                         element.attribute(u"angle1"_s).toDouble(),
                         element.attribute(u"angle1"_s),
                         element.attribute(u"angle2"_s).toDouble(),
                         element.attribute(u"angle2"_s),
                         ToPixel(element.attribute(u"length1"_s).toDouble(), unit),
                         element.attribute(u"length1"_s),
                         ToPixel(element.attribute(u"length2"_s).toDouble(), unit),
                         element.attribute(u"length2"_s));

    constexpr qreal tolerance = 0.01; // px, the lengths are written with six significant digits
    QVERIFY2(QLineF(spline.GetP2().toQPointF(), loaded.GetP2().toQPointF()).length() < tolerance,
             "first control point moved");
    QVERIFY2(QLineF(spline.GetP3().toQPointF(), loaded.GetP3().toQPointF()).length() < tolerance,
             "second control point moved");
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VAbstractPattern::ConvertedSplinePathGetsNewAttributes()
{
    const Unit unit = Unit::Cm;
    VAbstractValApplication::VApp()->SetPatternUnits(unit);

    TestDoc doc;
    QVERIFY(doc.setContent(QByteArray(R"(
        <root>
            <spline id="7" type="path" kCurve="1">
                <pathPoint kAsm1="1" kAsm2="1" angle="10" pSpline="1"/>
                <pathPoint kAsm1="1" kAsm2="1" angle="40" pSpline="2"/>
                <pathPoint kAsm1="1" kAsm2="1" angle="70" pSpline="3"/>
            </spline>
        </root>)")));
    QDomElement element = ElementById(doc, 7);

    const QVector<VFSplinePoint>
        points{VFSplinePoint(PointWithId(0, 0, u"A"_s, 1), 1, 190, 1, 10),
               VFSplinePoint(PointWithId(ToPixel(10, unit), ToPixel(10, unit), u"B"_s, 2), 1, 220, 1, 40),
               VFSplinePoint(PointWithId(ToPixel(20, unit), 0, u"C"_s, 3), 1, 250, 1, 70)};
    const VSplinePath path(points, 1.0);

    VToolSplinePath::SetSplinePathAttributes(&doc, element, path);

    QCOMPARE(element.attribute(u"type"_s), VToolSplinePath::ToolType);
    QVERIFY(not element.hasAttribute(u"kCurve"_s));
    QVERIFY(not VAbstractPattern::IsOldFormatSplinePath(element));

    const QDomNodeList children = element.childNodes();
    QCOMPARE(children.size(), 3); // the old points were replaced, not kept next to the new ones

    for (int i = 0; i < children.size(); ++i)
    {
        const QDomElement point = children.at(i).toElement();
        QCOMPARE(point.attribute(u"pSpline"_s).toUInt(), static_cast<uint>(i + 1));
        QVERIFY(not point.hasAttribute(u"kAsm1"_s));
        QVERIFY(not point.hasAttribute(u"kAsm2"_s));
        QVERIFY(not point.hasAttribute(u"angle"_s));
        for (const auto &name : {u"length1"_s, u"length2"_s, u"angle1"_s, u"angle2"_s})
        {
            QVERIFY2(point.hasAttribute(name), qUtf8Printable(u"point %1 lacks %2"_s.arg(i).arg(name)));
        }
    }

    // The handles that exist are not zero: a zero length is a straight line
    QVERIFY(children.at(0).toElement().attribute(u"length2"_s).toDouble() > 0);
    QVERIFY(children.at(1).toElement().attribute(u"length1"_s).toDouble() > 0);
    QVERIFY(children.at(1).toElement().attribute(u"length2"_s).toDouble() > 0);
    QVERIFY(children.at(2).toElement().attribute(u"length1"_s).toDouble() > 0);
}
