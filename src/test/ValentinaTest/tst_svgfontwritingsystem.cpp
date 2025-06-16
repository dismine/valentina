/************************************************************************
 **
 **  @file   tst_svgfontwritingsystem.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 6, 2025
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2025 Valentina project
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
#include "tst_svgfontwritingsystem.h"
#include "../vmisc/svgfont/svgdef.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTest>

//---------------------------------------------------------------------------------------------------------------------
TST_SVGFontWritingSystem::TST_SVGFontWritingSystem(QObject *parent)
  : QObject{parent}
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_SVGFontWritingSystem::TestSimplifiedChineseWritingSystemValidCases_data()
{
    QFile file("://svg_font_writing_system/simplified_chinese_writing_system.json");
    QVERIFY(file.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();

    QTest::addColumn<QChar>("input");

    auto AddChar = [](char16_t cp)
    {
        QTest::newRow(qUtf8Printable(QString("0x%1").arg(static_cast<unsigned int>(cp), 4, 16, QLatin1Char('0'))))
            << QChar(cp);
    };

    const QJsonArray ranges = root["ranges"].toArray();
    for (const QJsonValue val : ranges)
    {
        const QJsonObject r = val.toObject();
        const char16_t from = r["from"].toString().toUShort(nullptr, 0);
        const char16_t to = r["to"].toString().toUShort(nullptr, 0);
        for (char16_t cp = from; cp <= to; ++cp)
        {
            AddChar(cp);
        }
    }

    const QJsonArray singles = root["singles"].toArray();
    for (const QJsonValue val : singles)
    {
        const char16_t cp = val.toString().toUShort(nullptr, 0);
        AddChar(cp);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_SVGFontWritingSystem::TestSimplifiedChineseWritingSystemValidCases()
{
    QFETCH(QChar, input);
    QVERIFY(SimplifiedChineseWritingSystem(input));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_SVGFontWritingSystem::TestSimplifiedChineseWritingSystemOutOfRangeCases()
{
    // Characters just outside the valid ranges
    QCOMPARE(SimplifiedChineseWritingSystem(QChar(0x1F)), false);
    QCOMPARE(SimplifiedChineseWritingSystem(QChar(0x60)), false);
    QCOMPARE(SimplifiedChineseWritingSystem(QChar(0x7A)), false);
    QCOMPARE(SimplifiedChineseWritingSystem(QChar(0x7E)), false);

    // Invalid code points
    QCOMPARE(SimplifiedChineseWritingSystem(QChar(0x00)), false);
    QCOMPARE(SimplifiedChineseWritingSystem(QChar(0x1F00)), false);
    QCOMPARE(SimplifiedChineseWritingSystem(QChar(0xFFFF)), false);
}

//---------------------------------------------------------------------------------------------------------------------
void TST_SVGFontWritingSystem::TestTraditionalChineseWritingSystemValidCases_data()
{
    QFile file("://svg_font_writing_system/traditional_chinese_writing_system.json");
    QVERIFY(file.open(QIODevice::ReadOnly));
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();

    QTest::addColumn<QChar>("input");

    auto AddChar = [](char16_t cp)
    {
        QTest::newRow(qUtf8Printable(QString("0x%1").arg(static_cast<unsigned int>(cp), 4, 16, QLatin1Char('0'))))
            << QChar(cp);
    };

    const QJsonArray ranges = root["ranges"].toArray();
    for (const QJsonValue val : ranges)
    {
        const QJsonObject r = val.toObject();
        const char16_t from = r["from"].toString().toUShort(nullptr, 0);
        const char16_t to = r["to"].toString().toUShort(nullptr, 0);
        for (char16_t cp = from; cp <= to; ++cp)
        {
            AddChar(cp);
        }
    }

    const QJsonArray singles = root["singles"].toArray();
    for (const QJsonValue val : singles)
    {
        const char16_t cp = val.toString().toUShort(nullptr, 0);
        AddChar(cp);
    }
}

//---------------------------------------------------------------------------------------------------------------------
void TST_SVGFontWritingSystem::TestTraditionalChineseWritingSystemValidCases()
{
    QFETCH(QChar, input);
    QVERIFY(TraditionalChineseWritingSystem(input));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_SVGFontWritingSystem::TestTraditionalChineseWritingSystemOutOfRangeCases()
{
    // Characters just outside the valid ranges
    QCOMPARE(TraditionalChineseWritingSystem(QChar(0x1F)), false);
    QCOMPARE(TraditionalChineseWritingSystem(QChar(0x60)), false);
    QCOMPARE(TraditionalChineseWritingSystem(QChar(0x7A)), false);
    QCOMPARE(TraditionalChineseWritingSystem(QChar(0x7E)), false);

    // Invalid code points
    QCOMPARE(TraditionalChineseWritingSystem(QChar(0x00)), false);
    QCOMPARE(TraditionalChineseWritingSystem(QChar(0x1F00)), false);
    QCOMPARE(TraditionalChineseWritingSystem(QChar(0xFFFF)), false);
}
