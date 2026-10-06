/************************************************************************
 **
 **  @file   tst_valentinacommandline.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   4 10, 2015
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2015 Valentina project
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

#include "tst_valentinacommandline.h"
#include "../vmisc/vsysexits.h"

#include <QGlobalStatic>
#include <QtTest>

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
#include "../vmisc/compatibility.h"
#endif

using namespace Qt::Literals::StringLiterals;

namespace
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunused-member-function")

Q_GLOBAL_STATIC_WITH_ARGS(const QString, tmpTestFolder, ("tst_valentina_tmp"_L1)) // NOLINT
// NOLINTNEXTLINE
Q_GLOBAL_STATIC_WITH_ARGS(const QString, tmpTestCollectionFolder, ("tst_valentina_collection_tmp"_L1))

QT_WARNING_POP
} // namespace

TST_ValentinaCommandLine::TST_ValentinaCommandLine(QObject *parent)
  : AbstractTest(parent)
{
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_ValentinaCommandLine::initTestCase()
{
    { // Test files
        QDir tmpDir(*tmpTestFolder);
        if (not tmpDir.removeRecursively())
        {
            QFAIL("Fail to remove test temp directory.");
        }

        if (not CopyRecursively(QCoreApplication::applicationDirPath() + QDir::separator() + "tst_valentina"_L1,
                                QCoreApplication::applicationDirPath() + QDir::separator() + *tmpTestFolder))
        {
            QFAIL("Fail to prepare test files for testing.");
        }
    }

    { // Collection
        QDir tmpDir(*tmpTestCollectionFolder);
        if (not tmpDir.removeRecursively())
        {
            QFAIL("Fail to remove collection temp directory.");
        }

        if (not CopyRecursively(QCoreApplication::applicationDirPath() + QDir::separator()
                                    + "tst_valentina_collection"_L1,
                                QCoreApplication::applicationDirPath() + QDir::separator() + *tmpTestCollectionFolder))
        {
            QFAIL("Fail to prepare collection files for testing.");
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_ValentinaCommandLine::OpenPatterns_data() const
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("arguments");
    QTest::addColumn<int>("exitCode");

    // The file doesn't exist!
    QTest::newRow("Send wrong path to a file") << "wrongPath.val"
                                               << "--test" << V_EX_NOINPUT;

    QTest::newRow("Measurement independent empty file") << "empty.val"
                                                        << "--test" << V_EX_OK;

    QTest::newRow("File with invalid object type") << "wrong_obj_type.val"
                                                   << "--test" << V_EX_NOINPUT;

    QTest::newRow("Empty text VAL file") << "txt.val"
                                         << "--test" << V_EX_NOINPUT;

    QTest::newRow("Pattern with a warning") << "test_pedantic.val"
                                            << "--test;;--pedantic" << V_EX_DATAERR;

    QTest::newRow("Pattern with a buffer") << "buffer.val"
                                           << "--test;;--pedantic" << V_EX_OK;

    QTest::newRow("Pattern with a zero width buffer") << "buffer_zero_width.val"
                                                      << "--test;;--pedantic" << V_EX_DATAERR;

    QTest::newRow("Pattern with offset lines") << "offset_lines.val"
                                               << "--test;;--pedantic" << V_EX_OK;

    QTest::newRow("Pattern with a too wide offset line") << "offset_lines_too_wide.val"
                                                         << "--test;;--pedantic" << V_EX_DATAERR;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_ValentinaCommandLine::OpenPatterns()
{
    QFETCH(QString, file);
    QFETCH(QString, arguments);
    QFETCH(int, exitCode);

    QString error;
    const QString tmp = QCoreApplication::applicationDirPath() + QDir::separator() + *tmpTestFolder;
    const int exit = Run(exitCode,
                         ValentinaPath(),
                         QStringList() << arguments.split(";;") << tmp + QDir::separator() + file,
                         error);

    QVERIFY2(exit == exitCode, qUtf8Printable(error.right(350)));
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_ValentinaCommandLine::ExportMode_data() const
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("arguments");
    QTest::addColumn<int>("exitCode");

    const QString tmp = QCoreApplication::applicationDirPath() + QDir::separator() + *tmpTestFolder;

    QTest::newRow("Issue #372") << "issue_372.val" << u"-p;;0;;-d;;%1;;-b;;output;;--coefficient;;1"_s.arg(tmp)
                                << V_EX_OK;

    QTest::newRow("Buffer piece export") << "buffer.val"
                                         << u"-d;;%1;;-b;;buffer;;-f;;0;;--exportOnlyDetails;;--pedantic"_s.arg(tmp)
                                         << V_EX_OK;

    QTest::newRow("Buffer with zero width, pedantic")
        << "buffer_zero_width.val" << u"-d;;%1;;-b;;buffer_zero;;-f;;0;;--exportOnlyDetails;;--pedantic"_s.arg(tmp)
        << V_EX_DATAERR;
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_ValentinaCommandLine::ExportMode()
{
    QFETCH(QString, file);
    QFETCH(QString, arguments);
    QFETCH(int, exitCode);

    QString error;
    const QString tmp = QCoreApplication::applicationDirPath() + QDir::separator() + *tmpTestFolder;
    const auto arg = QStringList() << tmp + QDir::separator() + file << arguments.split(";;");
    const int exit = Run(exitCode, ValentinaPath(), arg, error);

    QVERIFY2(exit == exitCode, qUtf8Printable(error.right(350)));
}

//---------------------------------------------------------------------------------------------------------------------
// Offset lines reach DXF ASTM as internal lines: layer 8, quality validation on layer 85, NM for the not mirrored one.
void TST_ValentinaCommandLine::ExportOffsetLinesASTM()
{
    const QString tmp = QCoreApplication::applicationDirPath() + QDir::separator() + *tmpTestFolder;
    const QStringList arg{tmp + QDir::separator() + "offset_lines.val"_L1,
                          "-d"_L1,
                          tmp,
                          "-b"_L1,
                          "offset_lines"_L1,
                          "-f"_L1,
                          "25"_L1, // DXF_ASTM
                          "--exportOnlyDetails"_L1,
                          "--pedantic"_L1};
    QString error;
    const int exit = Run(V_EX_OK, ValentinaPath(), arg, error);
    QVERIFY2(exit == V_EX_OK, qUtf8Printable(error.right(350)));

    const QStringList files = QDir(tmp).entryList({"offset_lines*.dxf"_L1}, QDir::Files);
    QVERIFY2(not files.isEmpty(), "No DXF produced");

    QFile file(tmp + QDir::separator() + files.constFirst());
    QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));
    QStringList lines;
    const QStringList rawLines = QString::fromLatin1(file.readAll()).split('\n');
    lines.reserve(rawLines.size());
    for (const QString &line : rawLines)
    {
        lines.append(line.trimmed());
    }

    int layer85 = 0;
    for (int i = 0; i + 1 < lines.size(); ++i)
    {
        if (lines.at(i) == "8"_L1 && lines.at(i + 1) == "85"_L1)
        {
            ++layer85;
        }
    }
    QVERIFY2(layer85 >= 2, qUtf8Printable(u"layer 85 entities: %1"_s.arg(layer85))); // one per offset line
    QVERIFY(lines.contains("NM"_L1));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_ValentinaCommandLine::ExportSplineKeepsCurve_data() const
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("base");

    // A simple spline in a pattern of the old format.
    QTest::newRow("Old format spline") << "spline_old_format.val"
                                       << "spline_old";

    // What older versions wrote after opening such a pattern: the new spline type, but the old attributes. The curve must
    // not turn into a straight line.
    QTest::newRow("Spline saved with the old attributes") << "spline_damaged_format.val"
                                                          << "spline_damaged";
}

//---------------------------------------------------------------------------------------------------------------------
// The pieces of the pattern are bounded by a curve. A curve is exported as many points, a straight line as one segment,
// so the longest path of the exported SVG tells which one we got: about 50 commands for the curve, 10 for the line.
void TST_ValentinaCommandLine::ExportSplineKeepsCurve()
{
    QFETCH(QString, file);
    QFETCH(QString, base);

    const QString tmp = QCoreApplication::applicationDirPath() + QDir::separator() + *tmpTestFolder;
    const QStringList arg{tmp + QDir::separator() + file,
                          "-d"_L1,
                          tmp,
                          "-b"_L1,
                          base,
                          "-f"_L1,
                          "0"_L1, // SVG
                          "--exportOnlyDetails"_L1,
                          "--pedantic"_L1};
    QString error;
    const int exit = Run(V_EX_OK, ValentinaPath(), arg, error);
    QVERIFY2(exit == V_EX_OK, qUtf8Printable(error.right(350)));

    const QStringList files = QDir(tmp).entryList({base + "*.svg"_L1}, QDir::Files);
    QVERIFY2(not files.isEmpty(), "No SVG produced");

    QFile svg(tmp + QDir::separator() + files.constFirst());
    QVERIFY(svg.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString content = QString::fromUtf8(svg.readAll());

    static const QRegularExpression pathData(R"re(<path[^>]*\sd="([^"]+)")re"_L1);
        static const QRegularExpression command(u"[MLCQZ]"_s);
    qsizetype longest = 0;
    for (auto it = pathData.globalMatch(content); it.hasNext();)
    {
        longest = qMax(longest, it.next().captured(1).count(command));
    }

    constexpr qsizetype curveCommands = 30;
    QVERIFY2(longest >= curveCommands,
             qUtf8Printable(u"The longest path has %1 commands, the curve is lost"_s.arg(longest)));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_ValentinaCommandLine::TestMode_data() const
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("arguments");
    QTest::addColumn<int>("exitCode");

    const QString tmp = QCoreApplication::applicationDirPath() + QDir::separator() + *tmpTestFolder;

    QTest::newRow("Issue #256. Correct path.") << "issue_256.val" << u"--test"_s << V_EX_OK;

    QTest::newRow("Issue #256. Wrong path.") << "issue_256_wrong_path.vit" << u"--test"_s << V_EX_NOINPUT;

    QTest::newRow("Issue #256. Correct individual measurements.")
        << "issue_256.val" << u"--test;;-m;;%1"_s.arg(tmp + QDir::separator() + "issue_256_correct.vit"_L1) << V_EX_OK;

    QTest::newRow("Issue #256. Wrong individual measurements.")
        << "issue_256.val" << u"--test;;-m;;%1"_s.arg(tmp + QDir::separator() + "issue_256_wrong.vit"_L1)
        << V_EX_NOINPUT;

    QTest::newRow("Issue #256. Correct multisize measurements.")
        << "issue_256.val" << u"--test;;-m;;%1"_s.arg(tmp + QDir::separator() + "issue_256_correct.vst"_L1) << V_EX_OK;

    QTest::newRow("Issue #256. Wrong multisize measurements.")
        << "issue_256.val" << u"--test;;-m;;%1"_s.arg(tmp + QDir::separator() + "issue_256_wrong.vst"_L1)
        << V_EX_NOINPUT;

    QTest::newRow("Wrong formula.") << "wrong_formula.val" << u"--test"_s << V_EX_DATAERR;

    QTest::newRow("Legacy cutArc point without name1/name2 attributes.")
        << "legacy_cutarc_name.val" << u"--test"_s << V_EX_OK;

    // A measurement file with only 2 dimensions (size + height). Requesting a dimension the file
    // doesn't have used to crash (null QPointer<QComboBox> dereference in SetDimensionC) instead
    // of failing gracefully.
    QTest::newRow("Dimension letter unsupported by the measurement file.")
        << "issue_256.val"
        << u"--test;;-m;;%1;;--dimensionC;;62"_s.arg(tmp + QDir::separator() + "issue_256_correct.vst"_L1)
        << V_EX_DATAERR;
}

//---------------------------------------------------------------------------------------------------------------------
void TST_ValentinaCommandLine::TestMode()
{
    QFETCH(QString, file);
    QFETCH(QString, arguments);
    QFETCH(int, exitCode);

    QString error;
    const QString tmp = QCoreApplication::applicationDirPath() + QDir::separator() + *tmpTestFolder;
    const auto arg = QStringList() << tmp + QDir::separator() + file << arguments.split(";;"_L1);
    const int exit = Run(exitCode, ValentinaPath(), arg, error);

    QVERIFY2(exit == exitCode, qUtf8Printable(error.right(350)));
}

//---------------------------------------------------------------------------------------------------------------------
void TST_ValentinaCommandLine::TestOpenCollection_data() const
{
    QTest::addColumn<QString>("file");
    QTest::addColumn<QString>("arguments");
    QTest::addColumn<int>("exitCode");

    const QString tmp = QCoreApplication::applicationDirPath() + QDir::separator() + *tmpTestCollectionFolder;
    const QString testGOST = u"--test;;-m;;%1"_s.arg(tmp + QDir::separator() + "GOST_man_ru.vst"_L1);
    const auto keyTest = QStringLiteral("--test");

    QTest::newRow("bra") << "bra.val" << keyTest << V_EX_OK;
#ifdef Q_OS_WIN
    Q_UNUSED(testGOST)
#else
    QTest::newRow("jacketM1_52-176") << "jacketM1_52-176.val" << testGOST << V_EX_OK;
    QTest::newRow("jacketM2_40-146") << "jacketM2_40-146.val" << testGOST << V_EX_OK;
    QTest::newRow("jacketM3_40-146") << "jacketM3_40-146.val" << testGOST << V_EX_OK;
    QTest::newRow("jacketM4_40-146") << "jacketM4_40-146.val" << testGOST << V_EX_OK;
    QTest::newRow("jacketM5_30-110") << "jacketM5_30-110.val" << testGOST << V_EX_OK;
    QTest::newRow("jacketM6_30-110") << "jacketM6_30-110.val" << testGOST << V_EX_OK;
    QTest::newRow("pantsM1_52-176") << "pantsM1_52-176.val" << testGOST << V_EX_OK;
    QTest::newRow("pantsM2_40-146") << "pantsM2_40-146.val" << testGOST << V_EX_OK;
    QTest::newRow("pantsM7") << "pantsM7.val" << testGOST << V_EX_OK;
#endif

    QTest::newRow("TShirt_test") << "TShirt_test.val" << keyTest << V_EX_OK;
    QTest::newRow("TestDart") << "TestDart.val" << keyTest << V_EX_OK;
    QTest::newRow("MaleShirt") << "MaleShirt.val" << keyTest << V_EX_OK;
    QTest::newRow("Trousers") << "Trousers.val" << keyTest << V_EX_OK;
    QTest::newRow("Basic block women") << "Basic_block_women-2016.val" << keyTest << V_EX_OK;
    QTest::newRow("Gent Jacket with tummy") << "Gent_Jacket_with_tummy.val" << keyTest << V_EX_OK;
    QTest::newRow("Steampunk_trousers") << "Steampunk_trousers.val" << keyTest << V_EX_OK;
#ifndef Q_OS_WIN
    QTest::newRow("pattern_blusa") << "pattern_blusa.val" << keyTest << V_EX_OK;
    QTest::newRow("PajamaTopWrap2") << "PajamaTopWrap2.val" << keyTest << V_EX_OK;
    QTest::newRow("Keiko_skirt") << "Keiko_skirt.val" << keyTest << V_EX_OK;
    QTest::newRow("pantalon_base_Eli") << "pantalon_base_Eli.val" << keyTest << V_EX_OK;
    QTest::newRow("modell_2") << "modell_2.val" << keyTest << V_EX_OK;
    QTest::newRow("IMK_Zhaketa") << "IMK_Zhaketa_poluprilegayuschego_silueta.val" << keyTest << V_EX_OK;
    QTest::newRow("Moulage_0.5_armhole_neckline") << "Moulage_0.5_armhole_neckline.val" << keyTest << V_EX_OK;
    QTest::newRow("0.7_Armhole_adjustment_0.10") << "0.7_Armhole_adjustment_0.10.val" << keyTest << V_EX_OK;
#endif
    // We have a problem with encoding in Windows when we try to open some files in terminal
}

//---------------------------------------------------------------------------------------------------------------------
void TST_ValentinaCommandLine::TestOpenCollection()
{
    QFETCH(QString, file);
    QFETCH(QString, arguments);
    QFETCH(int, exitCode);

    QString error;
    const QString tmp = QCoreApplication::applicationDirPath() + QDir::separator() + *tmpTestCollectionFolder;
    const auto arg = QStringList() << tmp + QDir::separator() + file << arguments.split(";;");
    const int exit = Run(exitCode, ValentinaPath(), arg, error);

    QVERIFY2(exit == exitCode, qUtf8Printable(error.right(350)));
}

//---------------------------------------------------------------------------------------------------------------------
// cppcheck-suppress unusedFunction
void TST_ValentinaCommandLine::cleanupTestCase()
{
    {
        QDir tmpDir(*tmpTestFolder);
        if (not tmpDir.removeRecursively())
        {
            qWarning("Fail to remove test temp directory.");
        }
    }

    {
        QDir tmpDir(*tmpTestCollectionFolder);
        if (not tmpDir.removeRecursively())
        {
            qWarning("Fail to remove collection temp directory.");
        }
    }
}
