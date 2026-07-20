/************************************************************************
 **
 **  @file   tst_formulacache.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   5 6, 2026
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

#include "tst_formulacache.h"

#include "../ifc/exception/vexceptionbadid.h"
#include "../vpatterndb/calculator.h"
#include "../vpatterndb/variables/vincrement.h"
#include "../vpatterndb/variables/vlinelength.h"
#include "../vpatterndb/vcontainer.h"
#include "../vtools/tools/vabstracttool.h"

#include <QtTest>

//---------------------------------------------------------------------------------------------------------------------
TST_FormulaCache::TST_FormulaCache(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaCache::RefreshesValueOnReevaluation()
{
    VContainer data(nullptr, nullptr, VContainer::UniqueNamespace());

    auto *increment = new VIncrement(&data, QStringLiteral("myvar"));
    increment->SetFormula(5, QStringLiteral("5"), true);
    data.AddUniqueVariable(increment); // container takes ownership; increment stays a valid alias

    QString formula = QStringLiteral("myvar + 1");
    const quint32 toolId = 0;

    // First call compiles and caches the bytecode.
    QCOMPARE(VAbstractTool::CheckFormula(toolId, formula, &data), 6.0);

    // Subsequent calls hit the cache; they must reflect the current variable value (not a stale
    // cached one) and match a fresh, uncached Calculator evaluation.
    const QVector<qreal> values{10, 0, -3.5, 42};
    for (qreal const v : values)
    {
        increment->SetFormula(v, QString::number(v), true);

        Calculator cal;
        const qreal fresh = cal.EvalFormula(data.DataVariables(), formula);
        const qreal cached = VAbstractTool::CheckFormula(toolId, formula, &data);

        QCOMPARE(cached, v + 1);
        QCOMPARE(cached, fresh);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_FormulaCache::GetVariableWrongTypeThrows() const
{
    // Regression: a name present in the container but requested as the wrong type used to fail an
    // SCASSERT that is compiled away in release builds, so GetVariable returned a null QSharedPointer
    // that callers (e.g. DialogEditWrongFormula::ValChanged) dereferenced -> EXCEPTION_ACCESS_VIOLATION.
    // It must throw VExceptionBadId instead so the existing handling deals with the wrong-type case.
    VContainer data(nullptr, nullptr, VContainer::UniqueNamespace());

    auto *increment = new VIncrement(&data, QStringLiteral("myvar"));
    increment->SetFormula(5, QStringLiteral("5"), true);
    data.AddUniqueVariable(increment); // container takes ownership

    bool thrown = false;
    try
    {
        QSharedPointer<VLengthLine> const wrong = data.GetVariable<VLengthLine>(QStringLiteral("myvar"));
        Q_UNUSED(wrong)
    }
    catch (const VExceptionBadId &)
    {
        thrown = true;
    }

    QVERIFY2(thrown, "GetVariable with a wrong type must throw VExceptionBadId, not return a null pointer.");
}
