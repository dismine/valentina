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
#include "../ifc/exception/vexceptionbadid.h"
#include "../vgeometry/vplacelabelitem.h"
#include "../vgeometry/vpointf.h"
#include "../vmisc/typedef.h"
#include "../vpatterndb/variables/vincrement.h"
#include "../vpatterndb/vcontainer.h"

#include <QtTest>

using namespace Qt::Literals::StringLiterals;

//---------------------------------------------------------------------------------------------------------------------
TST_VContainer::TST_VContainer(QObject *parent)
  : QObject(parent)
{
}

namespace
{
auto MakeIncrement(VContainer *data, const QString &name, qreal value) -> VIncrement *
{
    auto *incr = new VIncrement(data, name);
    incr->SetFormula(value, QString::number(value), true);
    return incr;
}

// Benchmark-only variant. VIncrement(VContainer *, ...) deep-copies the whole container and then
// runs ClearVariables() over every variable in it, so building one is O(variables) all by itself --
// a cost that has nothing to do with the container's copy-on-write behaviour, and that real draw
// tools never pay (they add VLengthLine / VLineAngle / VArcRadius, not VIncrement). Using it inside
// a timed loop hides the very thing these benchmarks exist to measure.
auto MakeBenchVariable(const QString &name, qreal value) -> VIncrement *
{
    auto *incr = new VIncrement();
    incr->SetName(name);
    incr->SetFormula(value, QString::number(value), true);
    return incr;
}

void BenchmarkSizes()
{
    QTest::addColumn<int>("preload");
    QTest::newRow("n=0") << 0;
    QTest::newRow("n=500") << 500;
    QTest::newRow("n=2000") << 2000;
    QTest::newRow("n=8000") << 8000;
}

// Fills the container with `preload` variables and `preload` points.
auto Preload(VContainer *data, int preload) -> QVector<quint32>
{
    QVector<quint32> ids;
    ids.reserve(preload);
    for (int i = 0; i < preload; ++i)
    {
        data->AddVariable(MakeBenchVariable(u"preload_%1"_s.arg(i), i + 1));
        ids.append(data->AddGObject(new VPointF(i, i, u"P_preload_%1"_s.arg(i))));
    }
    return ids;
}
} // namespace

namespace
{
QtMessageHandler &PreviousMessageHandler()
{
    static QtMessageHandler previousMessageHandler = nullptr;
    return previousMessageHandler;
}

bool &SawDuplicateNameWarning()
{
    static bool sawDuplicateNameWarning = false;
    return sawDuplicateNameWarning;
}

void CaptureDuplicateNameWarning(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    if (type == QtWarningMsg && msg.contains(u"sharing the name"_s))
    {
        SawDuplicateNameWarning() = true;
    }

    if (PreviousMessageHandler() != nullptr)
    {
        PreviousMessageHandler()(type, context, msg);
    }
}
} // namespace

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
    SawDuplicateNameWarning() = false;
    PreviousMessageHandler() = qInstallMessageHandler(CaptureDuplicateNameWarning);
    const quint32 id2 = data.AddGObject(new VPointF(10, 10, name));
    qInstallMessageHandler(PreviousMessageHandler());

    QVERIFY(id2 != NULL_ID);
    QVERIFY(id1 != id2);
    QVERIFY2(SawDuplicateNameWarning(), "Two unrelated Draw::Calculation objects sharing a name must warn");

    QCOMPARE(data.GetGObject(id1)->name(), name);
    QCOMPARE(data.GetGObject(id2)->name(), name);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VContainer::UpdatingObjectWithUnchangedNameDoesNotWarn()
{
    const Unit unit = Unit::Cm;
    VContainer data(nullptr, &unit, VContainer::UniqueNamespace());

    const QString name = u"A78_A80__m1"_s;
    const quint32 id = data.AddGObject(new VPointF(0, 0, name));
    QVERIFY(id != NULL_ID);

    SawDuplicateNameWarning() = false;
    PreviousMessageHandler() = qInstallMessageHandler(CaptureDuplicateNameWarning);
    data.UpdateGObject(id, new VPointF(1, 1, name));
    qInstallMessageHandler(PreviousMessageHandler());

    QVERIFY2(!SawDuplicateNameWarning(), "Recomputing an object with its own existing name must not warn");
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

    SawDuplicateNameWarning() = false;
    PreviousMessageHandler() = qInstallMessageHandler(CaptureDuplicateNameWarning);
    data.UpdateGObject(data.getNextId(), modelingPoint);
    qInstallMessageHandler(PreviousMessageHandler());

    QVERIFY2(!SawDuplicateNameWarning(),
             "A Draw::Modeling object sharing its source Draw::Calculation object's name must not warn");
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VContainer::ModelingMirrorOfModelingMirrorDoesNotWarn()
{
    const Unit unit = Unit::Cm;
    VContainer data(nullptr, &unit, VContainer::UniqueNamespace());

    const QString name = u"B"_s;
    const quint32 calcId = data.AddGObject(new VPointF(0, 0, name));
    QVERIFY(calcId != NULL_ID);

    auto *sourceNode = new VPointF(0, 0, name);
    sourceNode->setIdObject(calcId);
    sourceNode->setMode(Draw::Modeling);
    const quint32 sourceNodeId = data.AddGObject(sourceNode);
    QVERIFY(sourceNodeId != NULL_ID);

    // Mirrors what VToolUnionDetails::AddNodePoint() does: the exposed piece node mirrors an
    // internal helper object (itself a mirror of the calc source), not the calc source directly --
    // a two-hop idObject chain.
    auto *helper = new VPointF(0, 0, name);
    helper->setIdObject(sourceNodeId);
    helper->setMode(Draw::Modeling);
    const quint32 helperId = data.AddGObject(helper);
    QVERIFY(helperId != NULL_ID);

    auto *unitedNode = new VPointF(0, 0, name);
    unitedNode->setIdObject(helperId);
    unitedNode->setMode(Draw::Modeling);

    SawDuplicateNameWarning() = false;
    PreviousMessageHandler() = qInstallMessageHandler(CaptureDuplicateNameWarning);
    data.UpdateGObject(data.getNextId(), unitedNode);
    qInstallMessageHandler(PreviousMessageHandler());

    QVERIFY2(!SawDuplicateNameWarning(),
             "A Draw::Modeling object mirroring another Draw::Modeling mirror of the same "
             "Draw::Calculation source must not warn");
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VContainer::PlaceLabelSharingCenterPointNameDoesNotWarn()
{
    const Unit unit = Unit::Cm;
    VContainer data(nullptr, &unit, VContainer::UniqueNamespace());

    const QString name = u"A170"_s;
    const quint32 calcId = data.AddGObject(new VPointF(0, 0, name));
    QVERIFY(calcId != NULL_ID);

    // Mirrors VToolPin::Create(): a piece pin is a Draw::Modeling copy of the calc point.
    auto *pin = new VPointF(0, 0, name);
    pin->setIdObject(calcId);
    pin->setMode(Draw::Modeling);
    const quint32 pinId = data.AddGObject(pin);
    QVERIFY(pinId != NULL_ID);

    // Mirrors VToolPlaceLabel::Create(): the label takes the center point's name but never calls
    // setIdObject() -- the link is SetCenterPoint(). Two labels may share one center point.
    auto MakeLabel = [&name](quint32 centerPoint)
    {
        auto *label = new VPlaceLabelItem();
        label->setName(name);
        label->SetCenterPoint(centerPoint);
        return label;
    };

    SawDuplicateNameWarning() = false;
    PreviousMessageHandler() = qInstallMessageHandler(CaptureDuplicateNameWarning);
    data.UpdateGObject(data.getNextId(), MakeLabel(pinId));
    data.UpdateGObject(data.getNextId(), MakeLabel(calcId));
    qInstallMessageHandler(PreviousMessageHandler());

    QVERIFY2(!SawDuplicateNameWarning(),
             "A place label sharing its center point's name must not warn, however many labels "
             "hang off that point");
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VContainer::UnionDetailsScaffoldCopyDoesNotWarn()
{
    const Unit unit = Unit::Cm;
    VContainer data(nullptr, &unit, VContainer::UniqueNamespace());

    const QString name = u"A170"_s;
    const quint32 calcId = data.AddGObject(new VPointF(0, 0, name));
    QVERIFY(calcId != NULL_ID);
    Q_UNUSED(calcId)

    // Mirrors the first object every VToolUnionDetails::AddNode*()/AddPin() builds: a Draw::Modeling copy
    // of the source carrying its name, registered before anything links back to it -- no idObject at all.
    auto *scaffold = new VPointF(0, 0, name);
    scaffold->setMode(Draw::Modeling);

    SawDuplicateNameWarning() = false;
    PreviousMessageHandler() = qInstallMessageHandler(CaptureDuplicateNameWarning);
    data.AddGObject(scaffold);
    qInstallMessageHandler(PreviousMessageHandler());

    QVERIFY2(!SawDuplicateNameWarning(),
             "An intermediate Draw::Modeling copy with no back-link to its source must not warn");
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VContainer::OldToolSnapshotUnaffectedByLaterStructuralChange()
{
    const Unit unit = Unit::Cm;
    VContainer data(nullptr, &unit, VContainer::UniqueNamespace());
    data.AddVariable(MakeIncrement(&data, u"first"_s, 1));

    const VContainer snapshot = data; // mirrors VDataTool::VDataTool's data(*data)

    // A later tool adds a structurally new variable to the *live* container.
    data.AddVariable(MakeIncrement(&data, u"second"_s, 2));

    // The old snapshot must not see it.
    bool exceptionThrown = false;
    try
    {
        snapshot.GetVariable<VIncrement>(u"second"_s);
    }
    catch (const VExceptionBadId &)
    {
        exceptionThrown = true;
    }
    QVERIFY(exceptionThrown);

    // The live container must.
    QCOMPARE(*data.GetVariable<VIncrement>(u"second"_s)->GetValue(), 2.0);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VContainer::OldToolSnapshotSeesValueUpdateToExistingVariable()
{
    // Pins the aliasing subtlety: FullLiteParse relies on old snapshots observing value
    // updates to already-existing variables, even though they must NOT see new ones.
    const Unit unit = Unit::Cm;
    VContainer data(nullptr, &unit, VContainer::UniqueNamespace());
    data.AddVariable(MakeIncrement(&data, u"first"_s, 1));

    const VContainer snapshot = data;

    data.AddVariable(MakeIncrement(&data, u"first"_s, 42)); // existing-name branch: mutates in place

    QCOMPARE(*snapshot.GetVariable<VIncrement>(u"first"_s)->GetValue(), 42.0);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VContainer::BenchmarkToolParseShape_data()
{
    BenchmarkSizes();
}

// The cost this plan exists to remove: parsing one more tool into an already-large container.
void TST_VContainer::BenchmarkToolParseShape()
{
    QFETCH(int, preload);

    const Unit unit = Unit::Cm;
    VContainer data(nullptr, &unit, VContainer::UniqueNamespace());
    Preload(&data, preload);

    // A fixed iteration count instead of QBENCHMARK's adaptive repeat: the adaptive
    // version would grow the container by a different amount for each row and for each
    // implementation, blurring the `preload` axis this benchmark is measuring along.
    constexpr int iterations = 200;

    QBENCHMARK_ONCE
    {
        for (int i = 0; i < iterations; ++i)
        {
            // Exactly what parsing one draw tool does: VDataTool takes a by-value snapshot
            // of the container, then the tool writes its own object and variable into the
            // live one -- which is what forces the copy-on-write detach.
            const VContainer snapshot = data;
            data.AddGObject(new VPointF(i, i, u"P_bench_%1"_s.arg(i)));
            data.AddVariable(MakeBenchVariable(u"bench_%1"_s.arg(i), i + 1));
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VContainer::BenchmarkVariableLookup_data()
{
    BenchmarkSizes();
}

// The cost this plan risks adding: HAMT lookup is slower per call than QHash lookup, and
// formula evaluation is lookup-count-dominated. This is the regression guard.
void TST_VContainer::BenchmarkVariableLookup()
{
    QFETCH(int, preload);

    if (preload == 0)
    {
        QSKIP("Nothing to look up.");
    }

    const Unit unit = Unit::Cm;
    VContainer data(nullptr, &unit, VContainer::UniqueNamespace());
    Preload(&data, preload);

    QStringList names;
    names.reserve(preload);
    for (int i = 0; i < preload; ++i)
    {
        names.append(u"preload_%1"_s.arg(i));
    }

    qreal sum = 0;
    QBENCHMARK
    {
        for (const auto &name : names)
        {
            sum += *data.GetVariable<VIncrement>(name)->GetValue();
        }
    }
    QVERIFY(sum > 0); // keeps the loop observable so it cannot be optimised away
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VContainer::BenchmarkObjectLookup_data()
{
    BenchmarkSizes();
}

void TST_VContainer::BenchmarkObjectLookup()
{
    QFETCH(int, preload);

    if (preload == 0)
    {
        QSKIP("Nothing to look up.");
    }

    const Unit unit = Unit::Cm;
    VContainer data(nullptr, &unit, VContainer::UniqueNamespace());
    const QVector<quint32> ids = Preload(&data, preload);

    qreal sum = 0;
    QBENCHMARK
    {
        for (auto id : ids)
        {
            sum += data.GeometricObject<VPointF>(id)->x();
        }
    }
    QVERIFY(sum >= 0);
}
