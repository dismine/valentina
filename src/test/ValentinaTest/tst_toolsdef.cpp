/************************************************************************
 **
 **  @file   tst_toolsdef.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 8, 2026
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

#include "tst_toolsdef.h"
#include "../vgeometry/vpointf.h"
#include "../vgeometry/vspline.h"
#include "../vpatterndb/vcontainer.h"
#include "../vtools/tools/toolsdef.h"

#include <QtTest>

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
TST_ToolsDef::TST_ToolsDef(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_ToolsDef::GetDefSourceNameSkipsPrefixedCollision()
{
    const Unit unit = Unit::Cm;
    VContainer data(nullptr, &unit, VContainer::UniqueNamespace());

    // Curve whose default source name we are about to generate. Its headless name is derived
    // from its endpoints, "A78_A80" here.
    const VPointF p1(0, 0, u"A78"_s);
    const VPointF p4(10, 10, u"A80"_s);
    const VSpline original(p1, QPointF(2, 2), QPointF(8, 8), p4);
    const quint32 originalId = data.AddGObject(new VSpline(original));
    data.AddSpline(data.GeometricObject<VSpline>(originalId), originalId);

    // Simulates a curve already produced by an earlier operation and registered under the full
    // (type-head-prefixed) name that the buggy code path used to ignore.
    QVERIFY(data.AddGObject(new VPointF(1, 1, u"Spl_A78_A80__m1"_s)) != NULL_ID);

    const QString suggested = GetDefSourceName(originalId, &data, u"m"_s, {});

    QVERIFY2(suggested != u"A78_A80__m1"_s,
             "Default name collides with an already registered curve once the type head is added");
    QCOMPARE(suggested, u"A78_A80__m2"_s);
}
