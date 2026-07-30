/************************************************************************
 **
 **  @file   tst_vcommonsettings.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   28 7, 2026
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

#include "tst_vcommonsettings.h"

#include "../vmisc/vcommonsettings.h"

#include <QStringList>
#include <QTemporaryDir>
#include <QVariant>
#include <QtTest>

namespace
{
//---------------------------------------------------------------------------------------------------------------------
// A value the size of the ones that made a reporter's settings file 138 MB.
auto OversizedString() -> QString
{
    return {VCommonSettings::oversizedValueThreshold + 1, QChar('x')};
}

//---------------------------------------------------------------------------------------------------------------------
// The same, but stored so that QSettings hands it back as a QStringList rather than a QString.
auto OversizedList() -> QStringList
{
    QStringList list;
    const QString item(100, QChar('y'));
    while (list.size() * (item.size() + 2) <= VCommonSettings::oversizedValueThreshold)
    {
        list.append(item);
    }
    return list;
}
} // namespace

//---------------------------------------------------------------------------------------------------------------------
TST_VCommonSettings::TST_VCommonSettings(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VCommonSettings::ValueSize_data()
{
    QTest::addColumn<QVariant>("value");
    QTest::addColumn<qsizetype>("expected");

    QTest::newRow("empty string") << QVariant(QString()) << static_cast<qsizetype>(0);
    QTest::newRow("plain string") << QVariant(QStringLiteral("abcde")) << static_cast<qsizetype>(5);
    QTest::newRow("number") << QVariant(12345) << static_cast<qsizetype>(5);

    // The regression that matters. A value holding a comma comes back from QSettings as a QStringList, and toString()
    // reports one of those as an empty string. Measuring only the string form made the largest value in a corrupt file
    // - a 29 million character materials list - score zero, so the guard walked straight past it.
    QTest::newRow("string list") << QVariant(QStringList({QStringLiteral("ab"), QStringLiteral("cde")}))
                                 << static_cast<qsizetype>(9);
    QTest::newRow("single item list") << QVariant(QStringList({QStringLiteral("abcd")})) << static_cast<qsizetype>(6);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VCommonSettings::ValueSize()
{
    QFETCH(QVariant, value);
    QFETCH(qsizetype, expected);

    QCOMPARE(VCommonSettings::ValueSize(value), expected);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VCommonSettings::RemoveOversizedValues()
{
    QTemporaryDir dir;
    QVERIFY2(dir.isValid(), qUtf8Printable(dir.errorString()));

    const QString fileName = dir.filePath(QStringLiteral("Test.ini"));
    const QStringList oversizedList = OversizedList();

    {
        VCommonSettings settings(fileName, QSettings::IniFormat);
        settings.setValue(QStringLiteral("paths/labels"), OversizedString());
        settings.setValue(QStringLiteral("pattern/knownMaterials"), oversizedList);
        settings.setValue(QStringLiteral("configuration/locale"), QStringLiteral("uk_UA"));
        settings.setValue(QStringLiteral("General/recentFileList"),
                          QStringList({QStringLiteral("/tmp/a.val"), QStringLiteral("/tmp/b.val")}));
        settings.sync();
        QCOMPARE(settings.status(), QSettings::NoError);
    }

    {
        VCommonSettings settings(fileName, QSettings::IniFormat);
        const QStringList removed = settings.RemoveOversizedValues();

        QCOMPARE(removed.size(), 2);
        QVERIFY2(removed.contains(QStringLiteral("paths/labels")), "The oversized string was not dropped.");
        QVERIFY2(removed.contains(QStringLiteral("pattern/knownMaterials")),
                 "The oversized list was not dropped. toString() reports a QStringList as empty, so measuring only the "
                 "string form makes the guard blind to exactly the largest value in a corrupt file.");
        settings.sync();
    }

    // The repair has to survive the round trip to disk, otherwise the file is still oversized on the next launch.
    {
        VCommonSettings settings(fileName, QSettings::IniFormat);
        QVERIFY(not settings.contains(QStringLiteral("paths/labels")));
        QVERIFY(not settings.contains(QStringLiteral("pattern/knownMaterials")));

        // Everything healthy is kept: dropping a corrupt path must not cost the user the rest of their preferences.
        QCOMPARE(settings.value(QStringLiteral("configuration/locale")).toString(), QStringLiteral("uk_UA"));
        QCOMPARE(settings.value(QStringLiteral("General/recentFileList")).toStringList().size(), 2);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_VCommonSettings::KeepsHealthySettingsUntouched()
{
    QTemporaryDir dir;
    QVERIFY2(dir.isValid(), qUtf8Printable(dir.errorString()));

    const QString fileName = dir.filePath(QStringLiteral("Test.ini"));

    {
        VCommonSettings settings(fileName, QSettings::IniFormat);
        settings.setValue(QStringLiteral("paths/labels"), QStringLiteral("C:/Users/1th/valentina/Шаблони мітки"));
        settings.setValue(QStringLiteral("pattern/knownMaterials"),
                          QStringList({QStringLiteral("Оксфорд"), QStringLiteral("фліс")}));
        settings.setValue(QStringLiteral("pattern/lineWidth"), 1.2);
        settings.sync();
    }

    {
        VCommonSettings settings(fileName, QSettings::IniFormat);
        QVERIFY2(settings.RemoveOversizedValues().isEmpty(), "A healthy settings file must come through untouched.");
    }

    {
        VCommonSettings settings(fileName, QSettings::IniFormat);
        QCOMPARE(settings.value(QStringLiteral("paths/labels")).toString(),
                 QStringLiteral("C:/Users/1th/valentina/Шаблони мітки"));
        QCOMPARE(settings.value(QStringLiteral("pattern/knownMaterials")).toStringList().size(), 2);
        QCOMPARE(settings.value(QStringLiteral("pattern/lineWidth")).toDouble(), 1.2);
    }
}
