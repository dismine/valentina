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

#include <QElapsedTimer>
#include <QThreadPool>
#include <QtTest>

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
