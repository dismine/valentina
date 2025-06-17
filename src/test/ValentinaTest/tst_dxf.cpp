/************************************************************************
 **
 **  @file   tst_dxf.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   20 2, 2025
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
#include "tst_dxf.h"
#include "../vdxf/libdxfrw/intern/drw_textcodec.h"
#include <QTest>

//---------------------------------------------------------------------------------------------------------------------
TST_DXF::TST_DXF(QObject *parent)
  : QObject(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
void TST_DXF::TestDecode_data()
{
    QTest::addColumn<QString>("input");              // Input string
    QTest::addColumn<int>("expectedCode");           // Expected code point
    QTest::addColumn<unsigned int>("expectedBytes"); // Expected byte length

    QTest::newRow("2-byte ©") << QString("\xC2\xA9") << 0x00A9 << 2u;           // © (U+00A9)
    QTest::newRow("2-byte ¢") << QString("\xC2\xA2") << 0x00A2 << 2u;           // ¢ (U+00A2)
    QTest::newRow("3-byte €") << QString("\xE2\x82\xAC") << 0x20AC << 3u;       // € (U+20AC)
    QTest::newRow("3-byte ḟ") << QString("\xE1\xB8\x9F") << 0x1E1F << 3u;       // ḟ (U+1E1F)
    QTest::newRow("4-byte 😀") << QString("\xF0\x9F\x98\x80") << 0x1F600 << 4u; // 😀 (U+1F600)
    QTest::newRow("4-byte 🎈") << QString("\xF0\x9F\x8E\x88") << 0x1F388 << 4u; // 🎈 (U+1F388)

    // Edge case for the longest valid 4-byte sequence (U+10FFFF)
    QTest::newRow("longest 4-byte U+10FFFF") << QString("\xF4\x8F\xBF\xBF") << 0x10FFFF << 4u; // U+10FFFF
}

//---------------------------------------------------------------------------------------------------------------------
void TST_DXF::TestDecode()
{
    QFETCH(QString, input);
    QFETCH(int, expectedCode);
    QFETCH(unsigned int, expectedBytes);

    // Convert QString to std::string for compatibility with the decodeNum function
    const std::string strInput = input.toStdString();
    unsigned int bytesProcessed = 0;
    const int result = DRW_Converter::decodeNum(strInput, &bytesProcessed);
    QCOMPARE(result, expectedCode);
    QCOMPARE(bytesProcessed, expectedBytes);
}
