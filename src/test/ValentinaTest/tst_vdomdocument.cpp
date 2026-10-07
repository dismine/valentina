/************************************************************************
 **
 **  @file   tst_vdomdocument.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   29 6, 2026
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

#include "tst_vdomdocument.h"

#include "../ifc/exception/vexceptionwrongid.h"
#include "../ifc/xml/vdomdocument.h"
#include "../ifc/xml/vpatternconverter.h"

#include <QFile>
#include <QTemporaryDir>
#include <QTest>

//---------------------------------------------------------------------------------------------------------------------
TST_VDomDocument::TST_VDomDocument(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
// Saving an empty document must fail and must NOT overwrite an existing good file on disk. QSaveFile is atomic but
// does not protect against committing empty content - this guard does. See empty .val corruption reports.
void TST_VDomDocument::RefuseEmptyDocumentSave() const
{
    QTemporaryDir dir;
    QVERIFY2(dir.isValid(), "Failed to create temporary directory.");

    const QString path = dir.filePath(QStringLiteral("pattern.val"));
    const auto sentinel = QByteArrayLiteral("<pattern>good</pattern>");

    {
        QFile file(path);
        QVERIFY2(file.open(QIODevice::WriteOnly), "Failed to write sentinel file.");
        file.write(sentinel);
    }

    VDomDocument doc; // empty: no document element
    QString error;
    QVERIFY2(not doc.SaveDocument(path, error), "Saving an empty document unexpectedly succeeded.");
    QVERIFY2(not error.isEmpty(), "Failed save did not report an error.");

    QFile file(path);
    QVERIFY2(file.open(QIODevice::ReadOnly), "Failed to reopen sentinel file.");
    QCOMPARE(file.readAll(), sentinel); // existing good file must be untouched
}

//---------------------------------------------------------------------------------------------------------------------
// ValidateXMLData must reject content that does not conform to the schema, so a damaged document can never be
// committed over a good file.
void TST_VDomDocument::RejectInvalidDataAgainstSchema() const
{
    const auto invalid = QByteArrayLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<notapattern/>\n");
    QString error;
    QVERIFY2(not VDomDocument::ValidateXMLData(VPatternConverter::CurrentSchema, invalid, QStringLiteral("test"), error),
             "Invalid XML unexpectedly passed schema validation.");
    QVERIFY2(not error.isEmpty(), "Failed validation did not report an error.");
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VDomDocument::TestUniqueId_data() const
{
    QTest::addColumn<QByteArray>("xml");
    QTest::addColumn<bool>("unique");

    QTest::newRow("empty document") << QByteArrayLiteral("<pattern/>") << true;

    QTest::newRow("unique ids") << QByteArrayLiteral("<pattern><draw><calculation>"
                                                     "<point id=\"1\"/><point id=\"2\"/><point id=\"3\"/>"
                                                     "</calculation></draw></pattern>")
                                << true;

    QTest::newRow("duplicate siblings") << QByteArrayLiteral("<pattern><draw><calculation>"
                                                             "<point id=\"1\"/><point id=\"1\"/>"
                                                             "</calculation></draw></pattern>")
                                        << false;

    // The duplicate sits at a different depth than its twin, so a traversal that skips or reorders nested elements
    // will silently miss it.
    QTest::newRow("duplicate across depths") << QByteArrayLiteral("<pattern><draw><calculation>"
                                                                  "<point id=\"1\"/>"
                                                                  "<operation id=\"2\"><source><point id=\"1\"/>"
                                                                  "</source></operation>"
                                                                  "</calculation></draw></pattern>")
                                             << false;

    // Non-element nodes interleaved with elements must not terminate the walk early.
    QTest::newRow("duplicate after comment and text") << QByteArrayLiteral("<pattern><draw><calculation>"
                                                                           "<point id=\"1\"/><!-- c -->text"
                                                                           "<point id=\"1\"/>"
                                                                           "</calculation></draw></pattern>")
                                                      << false;
}

//---------------------------------------------------------------------------------------------------------------------
// Each id in a pattern file must be unique; VPattern relies on it to resolve tools by id. TestUniqueId is the guard,
// so it must catch duplicates wherever they sit in the tree, and never cry wolf on a clean document.
void TST_VDomDocument::TestUniqueId() const
{
    QFETCH(QByteArray, xml);
    QFETCH(bool, unique);

    VDomDocument doc;
    QVERIFY2(doc.setContent(xml), "Failed to parse test document.");

    if (unique)
    {
        try
        {
            doc.TestUniqueId();
        }
        catch (const VExceptionWrongId &e)
        {
            QFAIL(qUtf8Printable(QStringLiteral("Unique ids rejected: %1").arg(e.ErrorMessage())));
        }
    }
    else
    {
        bool thrown = false;
        try
        {
            doc.TestUniqueId();
        }
        catch (const VExceptionWrongId &)
        {
            thrown = true;
        }
        QVERIFY2(thrown, "Expected VExceptionWrongId to be thrown for duplicate ids.");
    }
}

//---------------------------------------------------------------------------------------------------------------------
// Every saved pattern opens with a comment and indentation text before the first element, so the id walk has to step
// over non-element siblings instead of stopping at them. Nothing else catches this: the id cache is only an
// optimization, and FindElementById falls back to elementsByTagName() when it misses, so a walk that quietly collects
// nothing still returns the right answer by the slow path. Look up without a tag name - that path uses the walk itself.
void TST_VDomDocument::FindElementByIdStepsOverNonElementNodes()
{
    VDomDocument doc;
    QVERIFY2(doc.setContent(QByteArrayLiteral("<pattern>\n    <!--Pattern created with Valentina-->\n    <draw>"
                                              "<calculation><point id=\"7\"/></calculation></draw>\n</pattern>")),
             "Failed to parse test document.");

    const QDomElement e = doc.FindElementById(7);
    QVERIFY2(not e.isNull(), "Element sitting after comment and text nodes was not found.");
    QCOMPARE(e.tagName(), QStringLiteral("point"));
}

//---------------------------------------------------------------------------------------------------------------------
// Before 1.1.0 a Height point H with second line point P registered Line_P_H and AngleLine_P_H. Now it registers
// Line_H_P and AngleLine_H_P, so the converter must rewrite old formulas. The reversed angle has to stay in [0; 360),
// otherwise an angle >= 180 becomes off by 360. P can be produced by a chain of operations (name = source + suffix).
void TST_VDomDocument::ConvertHeightLineNamesToV1_1_0() const
{
    QTemporaryDir dir;
    QVERIFY2(dir.isValid(), "Failed to create temporary directory.");

    const QString path = dir.filePath(QStringLiteral("pattern.val"));
    {
        QFile file(path);
        QVERIFY2(file.open(QIODevice::WriteOnly), "Failed to write test pattern.");
        file.write(QByteArrayLiteral(
            R"(<?xml version="1.0" encoding="UTF-8"?>
<pattern>
    <version>0.9.8</version>
    <unit>cm</unit>
    <description/>
    <notes/>
    <measurements/>
    <increments>
        <increment description="" formula="Line_B_H*2" name="#inc"/>
    </increments>
    <previewCalculations/>
    <draw name="Piece1">
        <calculation>
            <point id="1" mx="0" my="0" name="A" type="single" x="0" y="0"/>
            <point id="2" mx="0" my="0" name="B" type="single" x="0" y="10"/>
            <point id="3" mx="0" my="0" name="C" type="single" x="5" y="5"/>
            <point basePoint="3" id="4" lineColor="black" mx="0" my="0" name="H" p1Line="1" p2Line="2" type="height" typeLine="hair"/>
            <operation angle="0" id="5" length="1" suffix="a" type="moving">
                <source><item idObject="2"/></source>
                <destination><item idObject="6" mx="0" my="0"/></destination>
            </operation>
            <operation angle="0" id="7" length="1" suffix="b" type="moving">
                <source><item idObject="6"/></source>
                <destination><item idObject="8" mx="0" my="0"/></destination>
            </operation>
            <point basePoint="3" id="9" lineColor="black" mx="0" my="0" name="H2" p1Line="1" p2Line="8" type="height" typeLine="hair"/>
            <point angle="AngleLine_B_H+AngleLine_A_H" basePoint="1" id="10" length="Line_B_H+Line_Bab_H2" lineColor="black" mx="0" my="0" name="D" type="endLine" typeLine="hair"/>
        </calculation>
        <modeling/>
        <details/>
        <groups/>
    </draw>
</pattern>
)"));
    }

    VPatternConverter converter(path);
    VDomDocument doc;
    doc.setXMLContent(converter.Convert());

    const QDomElement point = doc.FindElementById(10);
    QCOMPARE(point.attribute(QStringLiteral("angle")), QStringLiteral("fmod(AngleLine_H_B+180;360)+AngleLine_A_H"));
    QCOMPARE(point.attribute(QStringLiteral("length")), QStringLiteral("Line_H_B+Line_H2_Bab"));

    const QDomElement increment = doc.elementsByTagName(QStringLiteral("increment")).at(0).toElement();
    QCOMPARE(increment.attribute(QStringLiteral("formula")), QStringLiteral("Line_H_B*2"));
}
