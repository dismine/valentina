/************************************************************************
 **
 **  @file   tst_vcontainer.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   27 8, 2026
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

#include "tst_vcontainer.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/typedef.h"
#include "../vpatterndb/vcontainer.h"

#include <QtTest>

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
TST_VContainer::TST_VContainer(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VContainer::DuplicateNameIsNotRejected()
{
    const Unit unit = Unit::Cm;
    VContainer data(nullptr, &unit, VContainer::UniqueNamespace());

    const QString name = u"A78_A80__m1"_s;

    QVERIFY(data.IsUnique(name));

    const quint32 id1 = data.AddGObject(new VPointF(0, 0, name));
    QVERIFY(id1 != NULL_ID);
    QVERIFY(!data.IsUnique(name));

    // A second, independently created object claiming the already-registered name must not be
    // rejected today -- reproducing what happens when an undo/redo cycle restores a tool whose
    // name was, in the meantime, generated again for an unrelated tool.
    const quint32 id2 = data.AddGObject(new VPointF(10, 10, name));
    QVERIFY(id2 != NULL_ID);
    QVERIFY(id1 != id2);

    QCOMPARE(data.GetGObject(id1)->name(), name);
    QCOMPARE(data.GetGObject(id2)->name(), name);
}

namespace
{
QtMessageHandler previousMessageHandler = nullptr;
bool sawDuplicateNameWarning = false;

void CaptureDuplicateNameWarning(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (type == QtWarningMsg && msg.contains(u"sharing the name"_s))
    {
        sawDuplicateNameWarning = true;
    }

    if (previousMessageHandler != nullptr)
    {
        previousMessageHandler(type, context, msg);
    }
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
void TST_VContainer::UpdatingObjectWithUnchangedNameDoesNotWarn()
{
    const Unit unit = Unit::Cm;
    VContainer data(nullptr, &unit, VContainer::UniqueNamespace());

    const QString name = u"A78_A80__m1"_s;
    const quint32 id = data.AddGObject(new VPointF(0, 0, name));
    QVERIFY(id != NULL_ID);

    sawDuplicateNameWarning = false;
    previousMessageHandler = qInstallMessageHandler(CaptureDuplicateNameWarning);
    data.UpdateGObject(id, new VPointF(1, 1, name));
    qInstallMessageHandler(previousMessageHandler);

    QVERIFY2(!sawDuplicateNameWarning, "Recomputing an object with its own existing name must not warn");
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VContainer::ModelingMirrorSharingCalculationNameDoesNotWarn()
{
    const Unit unit = Unit::Cm;
    VContainer data(nullptr, &unit, VContainer::UniqueNamespace());

    const QString name = u"A4"_s;
    const quint32 calcId = data.AddGObject(new VPointF(0, 0, name));
    QVERIFY(calcId != NULL_ID);

    // Mirrors what VNodePoint::Create() does for a piece detail node: copy the source point,
    // point it back at the source via idObject, mark it Draw::Modeling, register under a new id.
    auto *modelingPoint = new VPointF(0, 0, name);
    modelingPoint->setIdObject(calcId);
    modelingPoint->setMode(Draw::Modeling);

    sawDuplicateNameWarning = false;
    previousMessageHandler = qInstallMessageHandler(CaptureDuplicateNameWarning);
    data.UpdateGObject(data.getNextId(), modelingPoint);
    qInstallMessageHandler(previousMessageHandler);

    QVERIFY2(!sawDuplicateNameWarning,
             "A Draw::Modeling object sharing its source Draw::Calculation object's name must not warn");
}
