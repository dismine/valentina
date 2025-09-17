/************************************************************************
 **
 **  @file   tst_vtextstream.cpp
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   12 9, 2025
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
#include "tst_vtextstream.h"
#include <qtestcase.h>

#include <QtTest/QtTest>

#ifdef WITH_TEXTCODEC
#include "../vmisc/codecs/qtextcodec.h"
#endif

#include "../vmisc/vtextstream.h"
#include <qobject.h>
#include <qstringliteral.h>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <qtypes.h>
#endif

TST_VTextStream::TST_VTextStream(QObject *parent)
  : QObject{parent},
    tempDir(QDir::tempPath() + QLatin1String("/tst_vtextstream.XXXXXX")),
    m_rfc3261FilePath(QFINDTESTDATA("vtextstream/rfc3261.txt")),
    m_shiftJisFilePath(QFINDTESTDATA("vtextstream/shift-jis.txt"))
{
}

void TST_VTextStream::initTestCase()
{
    QVERIFY2(tempDir.isValid(), qPrintable(tempDir.errorString()));
    QVERIFY(!m_rfc3261FilePath.isEmpty());
    QVERIFY(!m_shiftJisFilePath.isEmpty());

    testFileName = tempDir.path() + QLatin1String("/testfile");

    // chdir into the testdata dir and refer to our helper apps with relative paths
    QString const testdata_dir = QFileInfo(QStringLiteral(STDIN_PROCESS_BUILDDIR)).absolutePath();
    QVERIFY2(QDir::setCurrent(testdata_dir), qPrintable("Could not chdir to " + testdata_dir));
}

// Testing get/set functions
void TST_VTextStream::getSetCheck()
{
    // Initialize codecs
    VTextStream obj1;
#ifdef WITH_TEXTCODEC
    // QTextCodec * VTextStream::codec()
    // void VTextStream::setCodec(QTextCodec *)
    QTextCodec *var1 = QTextCodec::codecForName("en");
    obj1.setCodec(var1);
    QCOMPARE(var1, obj1.codec());
    obj1.setCodec(static_cast<QTextCodec *>(nullptr));
    QCOMPARE(static_cast<QTextCodec *>(nullptr), obj1.codec());

    // bool VTextStream::autoDetectUnicode()
    // void VTextStream::setAutoDetectUnicode(bool)
    obj1.setAutoDetectUnicode(false);
    QCOMPARE(false, obj1.autoDetectUnicode());
    obj1.setAutoDetectUnicode(true);
    QCOMPARE(true, obj1.autoDetectUnicode());

    // bool VTextStream::generateByteOrderMark()
    // void VTextStream::setGenerateByteOrderMark(bool)
    obj1.setGenerateByteOrderMark(false);
    QCOMPARE(false, obj1.generateByteOrderMark());
    obj1.setGenerateByteOrderMark(true);
    QCOMPARE(true, obj1.generateByteOrderMark());
#endif

    // QIODevice * VTextStream::device()
    // void VTextStream::setDevice(QIODevice *)
    auto *var4 = new QFile;
    obj1.setDevice(var4);
    QCOMPARE(static_cast<QIODevice *>(var4), obj1.device());
    obj1.setDevice(static_cast<QIODevice *>(nullptr));
    QCOMPARE(static_cast<QIODevice *>(nullptr), obj1.device());
    delete var4;

    // Status VTextStream::status()
    // void VTextStream::setStatus(Status)
    obj1.setStatus(VTextStream::Status(VTextStream::Ok));
    QCOMPARE(VTextStream::Status(VTextStream::Ok), obj1.status());
    obj1.setStatus(VTextStream::Status(VTextStream::ReadPastEnd));
    QCOMPARE(VTextStream::Status(VTextStream::ReadPastEnd), obj1.status());
    obj1.resetStatus();
    obj1.setStatus(VTextStream::Status(VTextStream::ReadCorruptData));
    QCOMPARE(VTextStream::Status(VTextStream::ReadCorruptData), obj1.status());

    // FieldAlignment VTextStream::fieldAlignment()
    // void VTextStream::setFieldAlignment(FieldAlignment)
    obj1.setFieldAlignment(VTextStream::FieldAlignment(VTextStream::AlignLeft));
    QCOMPARE(VTextStream::FieldAlignment(VTextStream::AlignLeft), obj1.fieldAlignment());
    obj1.setFieldAlignment(VTextStream::FieldAlignment(VTextStream::AlignRight));
    QCOMPARE(VTextStream::FieldAlignment(VTextStream::AlignRight), obj1.fieldAlignment());
    obj1.setFieldAlignment(VTextStream::FieldAlignment(VTextStream::AlignCenter));
    QCOMPARE(VTextStream::FieldAlignment(VTextStream::AlignCenter), obj1.fieldAlignment());
    obj1.setFieldAlignment(VTextStream::FieldAlignment(VTextStream::AlignAccountingStyle));
    QCOMPARE(VTextStream::FieldAlignment(VTextStream::AlignAccountingStyle), obj1.fieldAlignment());

    // QChar VTextStream::padChar()
    // void VTextStream::setPadChar(QChar)
    QChar var7 = 'Q';
    obj1.setPadChar(var7);
    QCOMPARE(var7, obj1.padChar());
    obj1.setPadChar(QChar());
    QCOMPARE(QChar(), obj1.padChar());

    // int VTextStream::fieldWidth()
    // void VTextStream::setFieldWidth(int)
    obj1.setFieldWidth(0);
    QCOMPARE(0, obj1.fieldWidth());
    obj1.setFieldWidth(INT_MIN);
    QCOMPARE(INT_MIN, obj1.fieldWidth());
    obj1.setFieldWidth(INT_MAX);
    QCOMPARE(INT_MAX, obj1.fieldWidth());

    // NumberFlags VTextStream::numberFlags()
    // void VTextStream::setNumberFlags(NumberFlags)
    obj1.setNumberFlags(VTextStream::NumberFlags(VTextStream::ShowBase));
    QCOMPARE(VTextStream::NumberFlags(VTextStream::ShowBase), obj1.numberFlags());
    obj1.setNumberFlags(VTextStream::NumberFlags(VTextStream::ForcePoint));
    QCOMPARE(VTextStream::NumberFlags(VTextStream::ForcePoint), obj1.numberFlags());
    obj1.setNumberFlags(VTextStream::NumberFlags(VTextStream::ForceSign));
    QCOMPARE(VTextStream::NumberFlags(VTextStream::ForceSign), obj1.numberFlags());
    obj1.setNumberFlags(VTextStream::NumberFlags(VTextStream::UppercaseBase));
    QCOMPARE(VTextStream::NumberFlags(VTextStream::UppercaseBase), obj1.numberFlags());
    obj1.setNumberFlags(VTextStream::NumberFlags(VTextStream::UppercaseDigits));
    QCOMPARE(VTextStream::NumberFlags(VTextStream::UppercaseDigits), obj1.numberFlags());

    // int VTextStream::integerBase()
    // void VTextStream::setIntegerBase(int)
    obj1.setIntegerBase(0);
    QCOMPARE(0, obj1.integerBase());
    obj1.setIntegerBase(INT_MIN);
    QCOMPARE(INT_MIN, obj1.integerBase());
    obj1.setIntegerBase(INT_MAX);
    QCOMPARE(INT_MAX, obj1.integerBase());

    // RealNumberNotation VTextStream::realNumberNotation()
    // void VTextStream::setRealNumberNotation(RealNumberNotation)
    obj1.setRealNumberNotation(VTextStream::RealNumberNotation(VTextStream::SmartNotation));
    QCOMPARE(VTextStream::RealNumberNotation(VTextStream::SmartNotation), obj1.realNumberNotation());
    obj1.setRealNumberNotation(VTextStream::RealNumberNotation(VTextStream::FixedNotation));
    QCOMPARE(VTextStream::RealNumberNotation(VTextStream::FixedNotation), obj1.realNumberNotation());
    obj1.setRealNumberNotation(VTextStream::RealNumberNotation(VTextStream::ScientificNotation));
    QCOMPARE(VTextStream::RealNumberNotation(VTextStream::ScientificNotation), obj1.realNumberNotation());

    // int VTextStream::realNumberPrecision()
    // void VTextStream::setRealNumberPrecision(int)
    obj1.setRealNumberPrecision(0);
    QCOMPARE(0, obj1.realNumberPrecision());
    obj1.setRealNumberPrecision(INT_MIN);
    QCOMPARE(6, obj1.realNumberPrecision()); // Setting a negative precision reverts it to the default value (6).
    obj1.setRealNumberPrecision(INT_MAX);
    QCOMPARE(INT_MAX, obj1.realNumberPrecision());
}

void TST_VTextStream::cleanup()
{
    QCoreApplication::instance()->processEvents();
}

// ------------------------------------------------------------------------------
void TST_VTextStream::construction()
{
    VTextStream stream;
#ifdef WITH_TEXTCODEC
    QCOMPARE(stream.codec(), QTextCodec::codecForLocale());
#endif
    QCOMPARE(stream.device(), static_cast<QIODevice *>(0));
    QCOMPARE(stream.string(), static_cast<QString *>(0));

    QTest::ignoreMessage(QtWarningMsg, "VTextStream: No device");
    QVERIFY(stream.atEnd());

    QTest::ignoreMessage(QtWarningMsg, "VTextStream: No device");
    QCOMPARE(stream.readAll(), QString());
}

void TST_VTextStream::generateLineData(bool for_QString)
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QStringList>("lines");

    // latin-1
    QTest::newRow("emptyer") << QByteArray() << QStringList();
    QTest::newRow("lf") << QByteArray("\n") << (QStringList() << "");
    QTest::newRow("crlf") << QByteArray("\r\n") << (QStringList() << "");
    QTest::newRow("oneline/nothing") << QByteArray("ole") << (QStringList() << "ole");
    QTest::newRow("oneline/lf") << QByteArray("ole\n") << (QStringList() << "ole");
    QTest::newRow("oneline/crlf") << QByteArray("ole\r\n") << (QStringList() << "ole");
    QTest::newRow("twolines/lf/lf") << QByteArray("ole\ndole\n") << (QStringList() << "ole" << "dole");
    QTest::newRow("twolines/crlf/crlf") << QByteArray("ole\r\ndole\r\n") << (QStringList() << "ole" << "dole");
    QTest::newRow("twolines/lf/crlf") << QByteArray("ole\ndole\r\n") << (QStringList() << "ole" << "dole");
    QTest::newRow("twolines/lf/nothing") << QByteArray("ole\ndole") << (QStringList() << "ole" << "dole");
    QTest::newRow("twolines/crlf/nothing") << QByteArray("ole\r\ndole") << (QStringList() << "ole" << "dole");
    QTest::newRow("threelines/lf/lf/lf") << QByteArray("ole\ndole\ndoffen\n")
                                         << (QStringList() << "ole" << "dole" << "doffen");
    QTest::newRow("threelines/crlf/crlf/crlf")
        << QByteArray("ole\r\ndole\r\ndoffen\r\n") << (QStringList() << "ole" << "dole" << "doffen");
    QTest::newRow("threelines/crlf/crlf/nothing")
        << QByteArray("ole\r\ndole\r\ndoffen") << (QStringList() << "ole" << "dole" << "doffen");

    if (!for_QString)
    {
#ifdef WITH_TEXTCODEC
        // utf-8
        QTest::newRow("utf8/twolines") << QByteArray("\xef\xbb\xbf"
                                                     "\x66\x67\x65\x0a"
                                                     "\x66\x67\x65\x0a",
                                                     11)
                                       << (QStringList() << "fge" << "fge");

        // utf-16
        // one line
        QTest::newRow("utf16-BE/nothing") << QByteArray("\xfe\xff"
                                                        "\x00\xe5\x00\x67\x00\x65",
                                                        8)
                                          << (QStringList() << QLatin1String("\345ge"));
        QTest::newRow("utf16-LE/nothing") << QByteArray("\xff\xfe"
                                                        "\xe5\x00\x67\x00\x65\x00",
                                                        8)
                                          << (QStringList() << QLatin1String("\345ge"));
        QTest::newRow("utf16-BE/lf") << QByteArray("\xfe\xff"
                                                   "\x00\xe5\x00\x67\x00\x65\x00\x0a",
                                                   10)
                                     << (QStringList() << QLatin1String("\345ge"));
        QTest::newRow("utf16-LE/lf") << QByteArray("\xff\xfe"
                                                   "\xe5\x00\x67\x00\x65\x00\x0a\x00",
                                                   10)
                                     << (QStringList() << QLatin1String("\345ge"));

        // two lines
        QTest::newRow("utf16-BE/twolines") << QByteArray("\xfe\xff"
                                                         "\x00\xe5\x00\x67\x00\x65\x00\x0a"
                                                         "\x00\xe5\x00\x67\x00\x65\x00\x0a",
                                                         18)
                                           << (QStringList() << QLatin1String("\345ge") << QLatin1String("\345ge"));
        QTest::newRow("utf16-LE/twolines") << QByteArray("\xff\xfe"
                                                         "\xe5\x00\x67\x00\x65\x00\x0a\x00"
                                                         "\xe5\x00\x67\x00\x65\x00\x0a\x00",
                                                         18)
                                           << (QStringList() << QLatin1String("\345ge") << QLatin1String("\345ge"));

        // three lines
        QTest::newRow("utf16-BE/threelines")
            << QByteArray("\xfe\xff"
                          "\x00\xe5\x00\x67\x00\x65\x00\x0a"
                          "\x00\xe5\x00\x67\x00\x65\x00\x0a"
                          "\x00\xe5\x00\x67\x00\x65\x00\x0a",
                          26)
            << (QStringList() << QLatin1String("\345ge") << QLatin1String("\345ge") << QLatin1String("\345ge"));
        QTest::newRow("utf16-LE/threelines")
            << QByteArray("\xff\xfe"
                          "\xe5\x00\x67\x00\x65\x00\x0a\x00"
                          "\xe5\x00\x67\x00\x65\x00\x0a\x00"
                          "\xe5\x00\x67\x00\x65\x00\x0a\x00",
                          26)
            << (QStringList() << QLatin1String("\345ge") << QLatin1String("\345ge") << QLatin1String("\345ge"));

        // utf-32
        QTest::newRow("utf32-BE/twolines")
            << QByteArray("\x00\x00\xfe\xff"
                          "\x00\x00\x00\xe5\x00\x00\x00\x67\x00\x00\x00\x65\x00\x00\x00\x0a"
                          "\x00\x00\x00\xe5\x00\x00\x00\x67\x00\x00\x00\x65\x00\x00\x00\x0a",
                          36)
            << (QStringList() << QLatin1String("\345ge") << QLatin1String("\345ge"));
        QTest::newRow("utf32-LE/twolines")
            << QByteArray("\xff\xfe\x00\x00"
                          "\xe5\x00\x00\x00\x67\x00\x00\x00\x65\x00\x00\x00\x0a\x00\x00\x00"
                          "\xe5\x00\x00\x00\x67\x00\x00\x00\x65\x00\x00\x00\x0a\x00\x00\x00",
                          36)
            << (QStringList() << QLatin1String("\345ge") << QLatin1String("\345ge"));
#endif
    }

    // partials
    QTest::newRow("cr") << QByteArray("\r") << (QStringList() << "");
    QTest::newRow("oneline/cr") << QByteArray("ole\r") << (QStringList() << "ole");
    if (!for_QString)
    {
#ifdef WITH_TEXTCODEC
        QTest::newRow("utf16-BE/cr") << QByteArray("\xfe\xff\x00\xe5\x00\x67\x00\x65\x00\x0d", 10)
                                     << (QStringList() << QLatin1String("\345ge"));
#endif
    }
}

// ------------------------------------------------------------------------------
void TST_VTextStream::readLineFromDevice_data()
{
    generateLineData(false);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::readLineFromDevice()
{
    QFETCH(QByteArray, data);
    QFETCH(QStringList, lines);

    QFile::remove(testFileName);
    QFile file(testFileName);
    QVERIFY(file.open(QFile::ReadWrite));
    QCOMPARE(file.write(data), qlonglong(data.size()));
    QVERIFY(file.flush());
    file.seek(0);

    VTextStream stream(&file);
    QStringList list;
    while (!stream.atEnd())
        list << stream.readLine();

    QCOMPARE(list, lines);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::readLineMaxlen_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QStringList>("lines");

    QTest::newRow("Hey") << QString("Hey") << (QStringList() << QString("Hey") << QString(""));
    QTest::newRow("Hey\\n") << QString("Hey\n") << (QStringList() << QString("Hey") << QString(""));
    QTest::newRow("HelloWorld") << QString("HelloWorld") << (QStringList() << QString("Hello") << QString("World"));
    QTest::newRow("Helo\\nWorlds") << QString("Helo\nWorlds") << (QStringList() << QString("Helo") << QString("World"));
    QTest::newRow("AAAAA etc.") << QString(16385, QLatin1Char('A'))
                                << (QStringList() << QString("AAAAA") << QString("AAAAA"));
#ifdef WITH_TEXTCODEC
    QTest::newRow("multibyte string") << QString::fromUtf8(
        "\341\233\222\341\233\226\341\232\251\341\232\271\341\232\242\341\233\232\341\232\240\n")
                                      << (QStringList() << QString::fromUtf8(
                                              "\341\233\222\341\233\226\341\232\251\341\232\271\341\232\242")
                                                        << QString::fromUtf8("\341\233\232\341\232\240"));
#endif
}

// ------------------------------------------------------------------------------
void TST_VTextStream::readLineMaxlen()
{
    QFETCH(QString, input);
    QFETCH(QStringList, lines);
    for (int i = 0; i < 2; ++i)
    {
        bool useDevice = (i == 1);
        VTextStream stream;
        QFile::remove("testfile");
        QFile file("testfile");
        if (useDevice)
        {
            (void) file.open(QIODevice::ReadWrite);
            file.write(input.toUtf8());
            file.seek(0);
            stream.setDevice(&file);
#ifdef WITH_TEXTCODEC
            stream.setCodec("utf-8");
#endif
        }
        else
        {
            stream.setString(&input);
        }

        QStringList list;
        list << stream.readLine(5);
        list << stream.readLine(5);

        QCOMPARE(list, lines);
    }
}

// ------------------------------------------------------------------------------
void TST_VTextStream::readLinesFromBufferCRCR()
{
    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);
    QByteArray data("0123456789\r\r\n");

    for (int i = 0; i < 10000; ++i)
        buffer.write(data);

    buffer.close();
    if (buffer.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        VTextStream stream(&buffer);
        while (!stream.atEnd())
        {
            QCOMPARE(stream.readLine(), QString("0123456789"));
        }
    }
}

class ErrorDevice : public QIODevice
{
protected:
    qint64 readData(char *data, qint64 maxlen) override
    {
        Q_UNUSED(data)
        Q_UNUSED(maxlen)
        return -1;
    }

    qint64 writeData(const char *data, qint64 len) override
    {
        Q_UNUSED(data)
        Q_UNUSED(len)
        return -1;
    }
};

void TST_VTextStream::readLineInto()
{
    QByteArray data = "1\n2\n3";

    VTextStream ts(&data);
    QString line;

    ts.readLineInto(&line);
    QCOMPARE(line, QStringLiteral("1"));

    ts.readLineInto(nullptr, 0); // read the second line, but don't store it

    ts.readLineInto(&line);
    QCOMPARE(line, QStringLiteral("3"));

    QVERIFY(!ts.readLineInto(&line));
    QVERIFY(line.isEmpty());

    QFile file(m_rfc3261FilePath);
    QVERIFY(file.open(QFile::ReadOnly));

    ts.setDevice(&file);
    line.reserve(1);
    qsizetype maxLineCapacity = line.capacity();

    while (ts.readLineInto(&line))
    {
        QVERIFY(line.capacity() >= maxLineCapacity);
        maxLineCapacity = line.capacity();
    }

    line = "Test string";
    ErrorDevice errorDevice;
    QVERIFY(errorDevice.open(QIODevice::ReadOnly));
    ts.setDevice(&errorDevice);

    QVERIFY(!ts.readLineInto(&line));
    QVERIFY(line.isEmpty());
}

// ------------------------------------------------------------------------------
void TST_VTextStream::readLineFromString_data()
{
    generateLineData(true);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::readLineFromString()
{
    QFETCH(QByteArray, data);
    QFETCH(QStringList, lines);

    QString dataString = data;

    VTextStream stream(&dataString, QIODevice::ReadOnly);
    QStringList list;
    while (!stream.atEnd())
        list << stream.readLine();

    QCOMPARE(list, lines);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::readLineFromStringThenChangeString()
{
    QString first = "First string";
    QString second = "Second string";

    VTextStream stream(&first, QIODevice::ReadOnly);
    QString result = stream.readLine();
    QCOMPARE(first, result);

    stream.setString(&second, QIODevice::ReadOnly);
    result = stream.readLine();
    QCOMPARE(second, result);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::setDevice()
{
    // Check that the read buffer is reset after setting a new device
    QByteArray data1("Hello World");
    QByteArray data2("How are you");

    QBuffer bufferOld(&data1);
    bufferOld.open(QIODevice::ReadOnly);

    QBuffer bufferNew(&data2);
    bufferNew.open(QIODevice::ReadOnly);

    QString text;
    VTextStream stream(&bufferOld);
    stream >> text;
    QCOMPARE(text, QString("Hello"));

    stream.setDevice(&bufferNew);
    stream >> text;
    QCOMPARE(text, QString("How"));
}

// ------------------------------------------------------------------------------
void TST_VTextStream::readLineFromTextDevice_data()
{
    generateLineData(false);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::readLineFromTextDevice()
{
    QFETCH(QByteArray, data);
    QFETCH(QStringList, lines);

    for (int i = 0; i < 8; ++i)
    {
        QBuffer buffer(&data);
        if (i < 4)
            QVERIFY(buffer.open(QIODevice::ReadOnly | QIODevice::Text));
        else
            QVERIFY(buffer.open(QIODevice::ReadOnly));

        VTextStream stream(&buffer);
        QStringList list;
        while (!stream.atEnd())
        {
            stream.pos(); // <- triggers side effects
            QString line;

            if (i & 1)
            {
                QChar c;
                while (!stream.atEnd())
                {
                    stream >> c;
                    if (stream.status() == VTextStream::Ok)
                    {
                        if (c != QLatin1Char('\n') && c != QLatin1Char('\r'))
                            line += c;
                        if (c == QLatin1Char('\n'))
                            break;
                    }
                }
            }
            else
            {
                line = stream.readLine();
            }

            if ((i & 3) == 3 && !QString(QTest::currentDataTag()).contains("utf16"))
                stream.seek(stream.pos());
            list << line;
        }
        QCOMPARE(list, lines);
    }
}

// ------------------------------------------------------------------------------
void TST_VTextStream::generateAllData(bool for_QString)
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<QString>("output");

    // latin-1
    QTest::newRow("empty") << QByteArray() << QString();
    QTest::newRow("latin1-a") << QByteArray("a") << QString("a");
    QTest::newRow("latin1-a\\r") << QByteArray("a\r") << QString("a\r");
    QTest::newRow("latin1-a\\r\\n") << QByteArray("a\r\n") << QString("a\r\n");
    QTest::newRow("latin1-a\\n") << QByteArray("a\n") << QString("a\n");

    // utf-16
    if (!for_QString)
    {
#ifdef WITH_TEXTCODEC
        // one line
        QTest::newRow("utf16-BE/nothing") << QByteArray("\xfe\xff"
                                                        "\x00\xe5\x00\x67\x00\x65",
                                                        8)
                                          << QString::fromLatin1("\345ge");
        QTest::newRow("utf16-LE/nothing") << QByteArray("\xff\xfe"
                                                        "\xe5\x00\x67\x00\x65\x00",
                                                        8)
                                          << QString::fromLatin1("\345ge");
        QTest::newRow("utf16-BE/lf") << QByteArray("\xfe\xff"
                                                   "\x00\xe5\x00\x67\x00\x65\x00\x0a",
                                                   10)
                                     << QString::fromLatin1("\345ge\n");
        QTest::newRow("utf16-LE/lf") << QByteArray("\xff\xfe"
                                                   "\xe5\x00\x67\x00\x65\x00\x0a\x00",
                                                   10)
                                     << QString::fromLatin1("\345ge\n");
        QTest::newRow("utf16-BE/crlf") << QByteArray("\xfe\xff"
                                                     "\x00\xe5\x00\x67\x00\x65\x00\x0d\x00\x0a",
                                                     12)
                                       << QString::fromLatin1("\345ge\r\n");
        QTest::newRow("utf16-LE/crlf") << QByteArray("\xff\xfe"
                                                     "\xe5\x00\x67\x00\x65\x00\x0d\x00\x0a\x00",
                                                     12)
                                       << QString::fromLatin1("\345ge\r\n");

        // two lines
        QTest::newRow("utf16-BE/twolines") << QByteArray("\xfe\xff"
                                                         "\x00\xe5\x00\x67\x00\x65\x00\x0a"
                                                         "\x00\xe5\x00\x67\x00\x65\x00\x0a",
                                                         18)
                                           << QString::fromLatin1("\345ge\n\345ge\n");
        QTest::newRow("utf16-LE/twolines") << QByteArray("\xff\xfe"
                                                         "\xe5\x00\x67\x00\x65\x00\x0a\x00"
                                                         "\xe5\x00\x67\x00\x65\x00\x0a\x00",
                                                         18)
                                           << QString::fromLatin1("\345ge\n\345ge\n");

        // three lines
        QTest::newRow("utf16-BE/threelines") << QByteArray("\xfe\xff"
                                                           "\x00\xe5\x00\x67\x00\x65\x00\x0a"
                                                           "\x00\xe5\x00\x67\x00\x65\x00\x0a"
                                                           "\x00\xe5\x00\x67\x00\x65\x00\x0a",
                                                           26)
                                             << QString::fromLatin1("\345ge\n\345ge\n\345ge\n");
        QTest::newRow("utf16-LE/threelines") << QByteArray("\xff\xfe"
                                                           "\xe5\x00\x67\x00\x65\x00\x0a\x00"
                                                           "\xe5\x00\x67\x00\x65\x00\x0a\x00"
                                                           "\xe5\x00\x67\x00\x65\x00\x0a\x00",
                                                           26)
                                             << QString::fromLatin1("\345ge\n\345ge\n\345ge\n");
#endif
    }
}

// ------------------------------------------------------------------------------
void TST_VTextStream::readLineUntilNull()
{
    QFile file(m_rfc3261FilePath);
    QVERIFY(file.open(QFile::ReadOnly));

    VTextStream stream(&file);
    for (int i = 0; i < 15066; ++i)
    {
        QString line = stream.readLine();
        QVERIFY(!line.isNull());
        QVERIFY(!line.isNull());
    }
    QVERIFY(!stream.readLine().isNull());
    QVERIFY(stream.readLine().isNull());
}

// ------------------------------------------------------------------------------
void TST_VTextStream::readAllFromDevice_data()
{
    generateAllData(false);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::readAllFromDevice()
{
    QFETCH(QByteArray, input);
    QFETCH(QString, output);

    QBuffer buffer(&input);
    buffer.open(QBuffer::ReadOnly);

    VTextStream stream(&buffer);
    QCOMPARE(stream.readAll(), output);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::readAllFromString_data()
{
    generateAllData(true);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::readAllFromString()
{
    QFETCH(QByteArray, input);
    QFETCH(QString, output);

    QString str = input;

    VTextStream stream(&str);
    QCOMPARE(stream.readAll(), output);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::skipWhiteSpace_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<QChar>("output");

    // latin1
    QTest::newRow("empty") << QByteArray() << QChar('\0');
    QTest::newRow(" one") << QByteArray(" one") << QChar('o');
    QTest::newRow("\\none") << QByteArray("\none") << QChar('o');
    QTest::newRow("\\n one") << QByteArray("\n one") << QChar('o');
    QTest::newRow(" \\r\\n one") << QByteArray(" \r\n one") << QChar('o');

#ifdef WITH_TEXTCODEC
    // utf-16
    QTest::newRow("utf16-BE (empty)") << QByteArray("\xfe\xff", 2) << QChar('\0');
    QTest::newRow("utf16-BE ( one)") << QByteArray("\xfe\xff\x00 \x00o\x00n\x00e", 10) << QChar('o');
    QTest::newRow("utf16-BE (\\none)") << QByteArray("\xfe\xff\x00\n\x00o\x00n\x00e", 10) << QChar('o');
    QTest::newRow("utf16-BE (\\n one)") << QByteArray("\xfe\xff\x00\n\x00 \x00o\x00n\x00e", 12) << QChar('o');
    QTest::newRow("utf16-BE ( \\r\\n one)")
        << QByteArray("\xfe\xff\x00 \x00\r\x00\n\x00 \x00o\x00n\x00e", 16) << QChar('o');

    QTest::newRow("utf16-LE (empty)") << QByteArray("\xff\xfe", 2) << QChar('\0');
    QTest::newRow("utf16-LE ( one)") << QByteArray("\xff\xfe \x00o\x00n\x00e\x00", 10) << QChar('o');
    QTest::newRow("utf16-LE (\\none)") << QByteArray("\xff\xfe\n\x00o\x00n\x00e\x00", 10) << QChar('o');
    QTest::newRow("utf16-LE (\\n one)") << QByteArray("\xff\xfe\n\x00 \x00o\x00n\x00e\x00", 12) << QChar('o');
    QTest::newRow("utf16-LE ( \\r\\n one)")
        << QByteArray("\xff\xfe \x00\r\x00\n\x00 \x00o\x00n\x00e\x00", 16) << QChar('o');
#endif
}

// ------------------------------------------------------------------------------
void TST_VTextStream::skipWhiteSpace()
{
    QFETCH(QByteArray, input);
    QFETCH(QChar, output);

    QBuffer buffer(&input);
    buffer.open(QBuffer::ReadOnly);

    VTextStream stream(&buffer);
    stream.skipWhiteSpace();

    QChar tmp;
    stream >> tmp;

    QCOMPARE(tmp, output);

    QString str = input;
    VTextStream stream2(&input);
    stream2.skipWhiteSpace();

    stream2 >> tmp;

    QCOMPARE(tmp, output);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::lineCount_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<int>("lineCount");

    QTest::newRow("empty") << QByteArray() << 0;
    QTest::newRow("oneline") << QByteArray("a\n") << 1;
    QTest::newRow("twolines") << QByteArray("a\nb\n") << 2;
    QTest::newRow("oneemptyline") << QByteArray("\n") << 1;
    QTest::newRow("twoemptylines") << QByteArray("\n\n") << 2;
    QTest::newRow("buffersize-1 line") << QByteArray(16382, '\n') << 16382;
    QTest::newRow("buffersize line") << QByteArray(16383, '\n') << 16383;
    QTest::newRow("buffersize+1 line") << QByteArray(16384, '\n') << 16384;
    QTest::newRow("buffersize+2 line") << QByteArray(16385, '\n') << 16385;

    QFile file(m_rfc3261FilePath);
    (void) file.open(QFile::ReadOnly);
    QTest::newRow("rfc3261") << file.readAll() << 15067;
}

// ------------------------------------------------------------------------------
void TST_VTextStream::lineCount()
{
    QFETCH(QByteArray, data);
    QFETCH(int, lineCount);

    QFile out("out.txt");
    (void) out.open(QFile::WriteOnly);

    VTextStream lineReader(data);
    int lines = 0;
    while (!lineReader.atEnd())
    {
        QString line = lineReader.readLine();
        out.write(line.toLatin1() + "\n");
        ++lines;
    }

    out.close();
    QCOMPARE(lines, lineCount);
}

// ------------------------------------------------------------------------------
struct CompareIndicesForArray
{
    qint64 *array;
    CompareIndicesForArray(qint64 *array)
      : array(array)
    {
    }
    bool operator()(const qint64 i1, const qint64 i2) { return array[i1] < array[i2]; }
};

void TST_VTextStream::performance()
{
    // Phase #1 - test speed of reading a huge text file with QFile.
    QElapsedTimer stopWatch;

    const int N = 3;
    const char *readMethods[N] = {"QFile::readLine()", "VTextStream::readLine()", "VTextStream::readLine(QString *)"};
    qint64 elapsed[N] = {0, 0, 0};

    stopWatch.restart();
    int nlines1 = 0;
    QFile file(m_rfc3261FilePath);
    QVERIFY(file.open(QFile::ReadOnly));

    while (!file.atEnd())
    {
        ++nlines1;
        file.readLine();
    }

    elapsed[0] = stopWatch.elapsed();
    stopWatch.restart();

    int nlines2 = 0;
    QFile file2(m_rfc3261FilePath);
    QVERIFY(file2.open(QFile::ReadOnly));

    VTextStream stream(&file2);
    while (!stream.atEnd())
    {
        ++nlines2;
        stream.readLine();
    }

    elapsed[1] = stopWatch.elapsed();
    stopWatch.restart();

    int nlines3 = 0;
    QFile file3(m_rfc3261FilePath);
    QVERIFY(file3.open(QFile::ReadOnly));

    VTextStream stream2(&file3);
    QString line;
    while (stream2.readLineInto(&line))
        ++nlines3;

    elapsed[2] = stopWatch.elapsed();

    QCOMPARE(nlines1, nlines2);
    QCOMPARE(nlines2, nlines3);

    for (int i = 0; i < N; i++)
    {
        qDebug("%s used %.3f seconds to read the file", readMethods[i], static_cast<double>(elapsed[i]) / 1000.0);
    }

    int idx[N] = {0, 1, 2};
    std::sort(idx, idx + N, CompareIndicesForArray(elapsed));

    for (int i = 0; i < N - 1; i++)
    {
        int i1 = idx[i];
        int i2 = idx[i + 1];
        qDebug("Reading by %s is %.2fx faster than by %s",
               readMethods[i1],
               double(elapsed[i2]) / double(elapsed[i1]),
               readMethods[i2]);
    }
}

// ------------------------------------------------------------------------------
void TST_VTextStream::hexTest_data()
{
    QTest::addColumn<qlonglong>("number");
    QTest::addColumn<QByteArray>("data");

    QTest::newRow("0") << Q_INT64_C(0) << QByteArray("0x0");
    QTest::newRow("1") << Q_INT64_C(1) << QByteArray("0x1");
    QTest::newRow("2") << Q_INT64_C(2) << QByteArray("0x2");
    QTest::newRow("3") << Q_INT64_C(3) << QByteArray("0x3");
    QTest::newRow("4") << Q_INT64_C(4) << QByteArray("0x4");
    QTest::newRow("5") << Q_INT64_C(5) << QByteArray("0x5");
    QTest::newRow("6") << Q_INT64_C(6) << QByteArray("0x6");
    QTest::newRow("7") << Q_INT64_C(7) << QByteArray("0x7");
    QTest::newRow("8") << Q_INT64_C(8) << QByteArray("0x8");
    QTest::newRow("9") << Q_INT64_C(9) << QByteArray("0x9");
    QTest::newRow("a") << Q_INT64_C(0xa) << QByteArray("0xa");
    QTest::newRow("b") << Q_INT64_C(0xb) << QByteArray("0xb");
    QTest::newRow("c") << Q_INT64_C(0xc) << QByteArray("0xc");
    QTest::newRow("d") << Q_INT64_C(0xd) << QByteArray("0xd");
    QTest::newRow("e") << Q_INT64_C(0xe) << QByteArray("0xe");
    QTest::newRow("f") << Q_INT64_C(0xf) << QByteArray("0xf");
    QTest::newRow("-1") << Q_INT64_C(-1) << QByteArray("-0x1");
    QTest::newRow("0xffffffff") << Q_INT64_C(0xffffffff) << QByteArray("0xffffffff");
    QTest::newRow("0xfffffffffffffffe") << Q_INT64_C(0xfffffffffffffffe) << QByteArray("-0x2");
    QTest::newRow("0xffffffffffffffff") << Q_INT64_C(0xffffffffffffffff) << QByteArray("-0x1");
    QTest::newRow("0x7fffffffffffffff") << Q_INT64_C(0x7fffffffffffffff) << QByteArray("0x7fffffffffffffff");
}

// ------------------------------------------------------------------------------
void TST_VTextStream::hexTest()
{
    QFETCH(qlonglong, number);
    QFETCH(QByteArray, data);

    QByteArray array;
    VTextStream stream(&array);

    stream << Val::showbase << Val::hex << number;
    stream.flush();
    QCOMPARE(array, data);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::binTest_data()
{
    QTest::addColumn<int>("number");
    QTest::addColumn<QByteArray>("data");

    QTest::newRow("0") << 0 << QByteArray("0b0");
    QTest::newRow("1") << 1 << QByteArray("0b1");
    QTest::newRow("2") << 2 << QByteArray("0b10");
    QTest::newRow("5") << 5 << QByteArray("0b101");
    QTest::newRow("-1") << -1 << QByteArray("-0b1");
    QTest::newRow("11111111") << 0xff << QByteArray("0b11111111");
    QTest::newRow("1111111111111111") << 0xffff << QByteArray("0b1111111111111111");
    QTest::newRow("1111111011111110") << 0xfefe << QByteArray("0b1111111011111110");
}

// ------------------------------------------------------------------------------
void TST_VTextStream::binTest()
{
    QFETCH(int, number);
    QFETCH(QByteArray, data);

    QByteArray array;
    VTextStream stream(&array);

    stream << Val::showbase << Val::bin << number;
    stream.flush();
    QCOMPARE(array.constData(), data.constData());
}

// ------------------------------------------------------------------------------
void TST_VTextStream::octTest_data()
{
    QTest::addColumn<int>("number");
    QTest::addColumn<QByteArray>("data");

    QTest::newRow("0") << 0 << QByteArray("00");
}

// ------------------------------------------------------------------------------
void TST_VTextStream::octTest()
{
    QFETCH(int, number);
    QFETCH(QByteArray, data);

    QByteArray array;
    VTextStream stream(&array);

    stream << Val::showbase << Val::oct << number;
    stream.flush();
    QCOMPARE(array, data);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::zeroTermination()
{
    VTextStream stream;
    char c = '@';

    QTest::ignoreMessage(QtWarningMsg, "VTextStream: No device");
    stream >> c;
    QCOMPARE(c, '\0');

    c = '@';

    QTest::ignoreMessage(QtWarningMsg, "VTextStream: No device");
    stream >> &c;
    QCOMPARE(c, '\0');
}

// ------------------------------------------------------------------------------
void TST_VTextStream::ws_manipulator()
{
    {
        QString string = "a b c d";
        VTextStream stream(&string);

        char a, b, c, d;
        stream >> a >> b >> c >> d;
        QCOMPARE(a, 'a');
        QCOMPARE(b, ' ');
        QCOMPARE(c, 'b');
        QCOMPARE(d, ' ');
    }
    {
        QString string = "a b c d";
        VTextStream stream(&string);

        char a, b, c, d;
        stream >> a >> Val::ws >> b >> Val::ws >> c >> Val::ws >> d;
        QCOMPARE(a, 'a');
        QCOMPARE(b, 'b');
        QCOMPARE(c, 'c');
        QCOMPARE(d, 'd');
    }
}

// ------------------------------------------------------------------------------
void TST_VTextStream::readNewlines_data()
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<QString>("output");

    QTest::newRow("empty") << QByteArray() << QString();
    QTest::newRow("\\r\\n") << QByteArray("\r\n") << QString("\n");
    QTest::newRow("\\r\\r\\n") << QByteArray("\r\r\n") << QString("\n");
    QTest::newRow("\\r\\n\\r\\n") << QByteArray("\r\n\r\n") << QString("\n\n");
    QTest::newRow("\\n") << QByteArray("\n") << QString("\n");
    QTest::newRow("\\n\\n") << QByteArray("\n\n") << QString("\n\n");
}

// ------------------------------------------------------------------------------
void TST_VTextStream::readNewlines()
{
    QFETCH(QByteArray, input);
    QFETCH(QString, output);

    QBuffer buffer(&input);
    buffer.open(QBuffer::ReadOnly | QBuffer::Text);
    VTextStream stream(&buffer);
    QCOMPARE(stream.readAll(), output);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::seek()
{
    QFile file(m_rfc3261FilePath);
    QVERIFY(file.open(QFile::ReadOnly));

    VTextStream stream(&file);
    QString tmp;
    stream >> tmp;
    QCOMPARE(tmp, QString::fromLatin1("Network"));

    // VTextStream::seek(0) should both clear its internal read/write buffers
    // and seek the device.
    for (int i = 0; i < 4; ++i)
    {
        stream.seek(12 + i);
        stream >> tmp;
        QCOMPARE(tmp, QString("Network").mid(i));
    }
    for (int i = 0; i < 4; ++i)
    {
        stream.seek(16 - i);
        stream >> tmp;
        QCOMPARE(tmp, QString("Network").mid(4 - i));
    }
    stream.seek(139181);
    stream >> tmp;
    QCOMPARE(tmp, QString("information"));
    stream.seek(388683);
    stream >> tmp;
    QCOMPARE(tmp, QString("telephone"));

    // Also test this with a string
    QString words = QLatin1String("thisisa");
    VTextStream stream2(&words, QIODevice::ReadOnly);
    stream2 >> tmp;
    QCOMPARE(tmp, QString::fromLatin1("thisisa"));

    for (int i = 0; i < 4; ++i)
    {
        stream2.seek(i);
        stream2 >> tmp;
        QCOMPARE(tmp, QString("thisisa").mid(i));
    }
    for (int i = 0; i < 4; ++i)
    {
        stream2.seek(4 - i);
        stream2 >> tmp;
        QCOMPARE(tmp, QString("thisisa").mid(4 - i));
    }
}

// ------------------------------------------------------------------------------
void TST_VTextStream::pos()
{
    {
        // Strings
        QString str("this is a test");
        VTextStream stream(&str, QIODevice::ReadWrite);

        QCOMPARE(stream.pos(), qint64(0));
        for (int i = 0; i <= str.size(); ++i)
        {
            QVERIFY(stream.seek(i));
            QCOMPARE(stream.pos(), qint64(i));
        }
        for (qsizetype j = str.size(); j >= 0; --j)
        {
            QVERIFY(stream.seek(j));
            QCOMPARE(stream.pos(), qint64(j));
        }

        QVERIFY(stream.seek(0));

        QChar ch;
        stream >> ch;
        QCOMPARE(ch, QChar('t'));

        QCOMPARE(stream.pos(), qint64(1));
        QVERIFY(stream.seek(1));
        QCOMPARE(stream.pos(), qint64(1));
        QVERIFY(stream.seek(0));

        QString strtmp;
        stream >> strtmp;
        QCOMPARE(strtmp, QString("this"));

        QCOMPARE(stream.pos(), qint64(4));
        stream.seek(0);
        stream.seek(4);

        stream >> ch;
        QCOMPARE(ch, QChar(' '));
        QCOMPARE(stream.pos(), qint64(5));

        stream.seek(10);
        stream >> strtmp;
        QCOMPARE(strtmp, QString("test"));
        QCOMPARE(stream.pos(), qint64(14));
    }
    {
        // Latin1 device
        QFile file(m_rfc3261FilePath);
        QVERIFY(file.open(QIODevice::ReadOnly));

        VTextStream stream(&file);

        QCOMPARE(stream.pos(), qint64(0));

        for (int i = 0; i <= file.size(); i += 7)
        {
            QVERIFY(stream.seek(i));
            QCOMPARE(stream.pos(), qint64(i));
        }
        for (qint64 j = file.size(); j >= 0; j -= 7)
        {
            QVERIFY(stream.seek(j));
            QCOMPARE(stream.pos(), qint64(j));
        }

        stream.seek(0);

        QString strtmp;
        stream >> strtmp;
        QCOMPARE(strtmp, QString("Network"));
        QCOMPARE(stream.pos(), qint64(19));

        stream.seek(2598);
        QCOMPARE(stream.pos(), qint64(2598));
        stream >> strtmp;
        QCOMPARE(stream.pos(), qint64(2607));
        QCOMPARE(strtmp, QString("locations"));
    }
#if defined(WITH_ICU_CODECS) || (defined(WITH_BIG_CODECS) && !defined(Q_OS_INTEGRITY))
    {
        // Shift-JIS device
        for (int i = 0; i < 2; ++i)
        {
            QFile file(m_shiftJisFilePath);
            if (i == 0)
                QVERIFY(file.open(QIODevice::ReadOnly));
            else
                QVERIFY(file.open(QIODevice::ReadOnly | QIODevice::Text));

            VTextStream stream(&file);

            stream.setCodec("Shift-JIS");
            QVERIFY(stream.codec());

            QCOMPARE(stream.pos(), qint64(0));
            for (int i = 0; i <= file.size(); i += 7)
            {
                QVERIFY(stream.seek(i));
                QCOMPARE(stream.pos(), qint64(i));
            }
            for (qsizetype j = file.size(); j >= 0; j -= 7)
            {
                QVERIFY(stream.seek(j));
                QCOMPARE(stream.pos(), qint64(j));
            }

            uchar *ptr = file.map(0, file.size());
            QVERIFY(ptr);

            QByteArray const data(reinterpret_cast<const char *>(ptr), file.size());

            // --- AUnicode ---
            {
                const QByteArray marker = stream.codec()->fromUnicode(QStringLiteral("AUnicode"));
                const qint64 offset = data.indexOf(marker);
                QVERIFY(offset >= 0);

                QVERIFY(stream.seek(offset));
                QCOMPARE(stream.pos(), offset);

                QString strtmp;
                stream >> strtmp;
                QCOMPARE(strtmp, QStringLiteral("AUnicode"));
                QCOMPARE(stream.pos(), offset + marker.size());
            }

            // --- Shift-JIS ---
            {
                const QByteArray marker = stream.codec()->fromUnicode(QStringLiteral("Shift-JIS"));
                qint64 offset = data.lastIndexOf(marker);
                QVERIFY(offset >= 0);

                QVERIFY(stream.seek(offset));
                QCOMPARE(stream.pos(), offset);

                QString strtmp;
                stream >> strtmp;
                QCOMPARE(strtmp, QStringLiteral("Shift-JIS"));
                offset += marker.size();
                QCOMPARE(stream.pos(), offset);

                stream >> strtmp;
                const QString str = QString::fromUtf8("\343\201\247\346\233\270\343\201\213\343\202\214\343\201\237");
                QCOMPARE(strtmp, str);
                offset += stream.codec()->fromUnicode(str).size() + 1;
                QCOMPARE(stream.pos(), offset);

                stream >> strtmp;
                QCOMPARE(strtmp, QStringLiteral("POD"));
                offset += stream.codec()->fromUnicode(QStringLiteral("POD")).size() + 1;
                QCOMPARE(stream.pos(), offset);
            }
        }
    }
#endif
}

// ------------------------------------------------------------------------------
void TST_VTextStream::pos2()
{
    QByteArray data("abcdef\r\nghijkl\r\n");
    QBuffer buffer(&data);
    QVERIFY(buffer.open(QIODevice::ReadOnly | QIODevice::Text));

    VTextStream stream(&buffer);

    QChar ch;

    QCOMPARE(stream.pos(), qint64(0));
    stream >> ch;
    QCOMPARE(ch, QChar('a'));
    QCOMPARE(stream.pos(), qint64(1));

    QString str;
    stream >> str;
    QCOMPARE(str, QString("bcdef"));
    QCOMPARE(stream.pos(), qint64(6));

    stream >> str;
    QCOMPARE(str, QString("ghijkl"));
    QCOMPARE(stream.pos(), qint64(14));

    // Seek back and try again
    stream.seek(1);
    QCOMPARE(stream.pos(), qint64(1));
    stream >> str;
    QCOMPARE(str, QString("bcdef"));
    QCOMPARE(stream.pos(), qint64(6));

    stream.seek(6);
    stream >> str;
    QCOMPARE(str, QString("ghijkl"));
    QCOMPARE(stream.pos(), qint64(14));
}

// ------------------------------------------------------------------------------
void TST_VTextStream::pos3LargeFile()
{
    {
        QFile file(testFileName);
        (void) file.open(QIODevice::WriteOnly | QIODevice::Text);
        VTextStream out(&file);
        // NOTE: The unusual spacing is to ensure non-1-character whitespace.
        QString lineString = QStringLiteral(" 0  1  2\t3  4\t \t5  6  7  8   9 \n");
        // Approximate 50kb text file
        const int NbLines = (50 * 1024) / static_cast<int>(lineString.length()) + 1;
        for (int line = 0; line < NbLines; ++line)
            out << lineString;
        // File is automatically flushed and closed on destruction.
    }
    QFile file(testFileName);
    (void) file.open(QIODevice::ReadOnly | QIODevice::Text);
    VTextStream in(&file);
    const int testValues[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int value;
    while (true)
    {
        in.pos();
        for (int i = 0; i < 10; ++i)
        {
            in >> value;
            if (in.status() != VTextStream::Ok)
            {
                // End case, i == 0 && eof reached.
                QCOMPARE(i, 0);
                QCOMPARE(in.status(), VTextStream::ReadPastEnd);
                return;
            }
            QCOMPARE(value, testValues[i]);
        }
    }
}

// ------------------------------------------------------------------------------
void TST_VTextStream::readStdin()
{
    QProcess stdinProcess;
    stdinProcess.start(QStringLiteral(STDIN_PROCESS_BUILDDIR));
    stdinProcess.setReadChannel(QProcess::StandardError);

    VTextStream stream(&stdinProcess);
    stream << "1" << Val::endl;
    stream << "2" << Val::endl;
    stream << "3" << Val::endl;

    stdinProcess.closeWriteChannel();

    QVERIFY(stdinProcess.waitForFinished(5000));

    int a, b, c;
    stream >> a >> b >> c;
    QCOMPARE(a, 1);
    QCOMPARE(b, 2);
    QCOMPARE(c, 3);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::readAllFromStdin()
{
    QProcess stdinProcess;
    stdinProcess.start(QStringLiteral(READ_ALL_STDIN_PROCESS_BUILDDIR), {}, QIODevice::ReadWrite | QIODevice::Text);
    stdinProcess.setReadChannel(QProcess::StandardError);

    VTextStream stream(&stdinProcess);
#ifdef WITH_TEXTCODEC
    stream.setCodec("ISO-8859-1");
#endif
    stream << "hello world" << Val::flush;

    stdinProcess.closeWriteChannel();

    QVERIFY(stdinProcess.waitForFinished(5000));
    QCOMPARE(stream.readAll(), QString::fromLatin1("hello world\n"));
}

// ------------------------------------------------------------------------------
void TST_VTextStream::readLineFromStdin()
{
    QProcess stdinProcess;
    stdinProcess.start(QStringLiteral(READ_LINE_STDIN_PROCESS_BUILDDIR), {}, QIODevice::ReadWrite | QIODevice::Text);
    stdinProcess.setReadChannel(QProcess::StandardError);

    stdinProcess.write("abc\n");
    QVERIFY(stdinProcess.waitForReadyRead(5000));
    QCOMPARE(stdinProcess.readAll().data(), QByteArray("abc").data());

    stdinProcess.write("def\n");
    QVERIFY(stdinProcess.waitForReadyRead(5000));
    QCOMPARE(stdinProcess.readAll(), QByteArray("def"));

    stdinProcess.closeWriteChannel();

    QVERIFY(stdinProcess.waitForFinished(5000));
}

// ------------------------------------------------------------------------------
void TST_VTextStream::read()
{
    {
        QFile::remove("testfile");
        QFile file("testfile");
        (void) file.open(QFile::WriteOnly);
        file.write("4.15 abc ole");
        file.close();

        QVERIFY(file.open(QFile::ReadOnly));
        VTextStream stream(&file);
        QCOMPARE(stream.read(0), QString(""));
        QCOMPARE(stream.read(4), QString("4.15"));
        QCOMPARE(stream.read(4), QString(" abc"));
        stream.seek(1);
        QCOMPARE(stream.read(4), QString(".15 "));
        stream.seek(1);
        QCOMPARE(stream.read(4), QString(".15 "));
        stream.seek(2);
        QCOMPARE(stream.read(4), QString("15 a"));
        // ### add tests for reading \r\n etc..
    }

    {
        // File larger than VTextStream_BUFFERSIZE
        QFile::remove("testfile");
        QFile file("testfile");
        (void) file.open(QFile::WriteOnly);
        for (int i = 0; i < 16384 / 8; ++i)
            file.write("01234567");
        file.write("0");
        file.close();

        QVERIFY(file.open(QFile::ReadOnly));
        VTextStream stream(&file);
        QCOMPARE(stream.read(10), QString("0123456701"));
        QCOMPARE(stream.read(10), QString("2345670123"));
        QCOMPARE(stream.readAll().size(), 16385 - 20);
    }
}

// ------------------------------------------------------------------------------
void TST_VTextStream::qbool()
{
    QString s;
    VTextStream stream(&s);
    stream << s.contains(QString("hei"));
    QCOMPARE(s, QString("0"));
}

// ------------------------------------------------------------------------------
void TST_VTextStream::forcePoint()
{
    QString str;
    VTextStream stream(&str);
    stream << Val::fixed << Val::forcepoint << 1.0 << ' ' << 1 << ' ' << 0 << ' ' << -1.0 << ' ' << -1;
    QCOMPARE(str, QString("1.000000 1 0 -1.000000 -1"));

    str.clear();
    stream.seek(0);
    stream << Val::scientific << Val::forcepoint << 1.0 << ' ' << 1 << ' ' << 0 << ' ' << -1.0 << ' ' << -1;
    QCOMPARE(str, QString("1.000000e+00 1 0 -1.000000e+00 -1"));

    str.clear();
    stream.seek(0);
    stream.setRealNumberNotation(VTextStream::SmartNotation);
    stream << Val::forcepoint << 1.0 << ' ' << 1 << ' ' << 0 << ' ' << -1.0 << ' ' << -1;
    QCOMPARE(str, QString("1.00000 1 0 -1.00000 -1"));
}

// ------------------------------------------------------------------------------
void TST_VTextStream::forceSign()
{
    QString str;
    VTextStream stream(&str);
    stream << Val::forcesign << 1.2 << ' ' << -1.2 << ' ' << 0;
    QCOMPARE(str, QString("+1.2 -1.2 +0"));
}

// ------------------------------------------------------------------------------
void TST_VTextStream::read0d0d0a()
{
    QFile file("task113817.txt");
    (void) file.open(QIODevice::ReadOnly | QIODevice::Text);

    VTextStream stream(&file);
    while (!stream.atEnd())
        stream.readLine();
}

// ------------------------------------------------------------------------------

Q_DECLARE_METATYPE(VTextStreamFunction)

// Also tests that we can have namespaces that conflict with our VTextStream constants.
namespace ws
{
static VTextStream &noop(VTextStream &s)
{
    return s;
}
} // namespace ws

void TST_VTextStream::numeralCase_data()
{
    VTextStreamFunction noop_ = ws::noop;
    VTextStreamFunction bin = Val::bin;
    VTextStreamFunction oct = Val::oct;
    VTextStreamFunction hex = Val::hex;
    VTextStreamFunction base = Val::showbase;
    VTextStreamFunction ucb = Val::uppercasebase;
    VTextStreamFunction lcb = Val::lowercasebase;
    VTextStreamFunction ucd = Val::uppercasedigits;
    VTextStreamFunction lcd = Val::lowercasedigits;

    QTest::addColumn<VTextStreamFunction>("func1");
    QTest::addColumn<VTextStreamFunction>("func2");
    QTest::addColumn<VTextStreamFunction>("func3");
    QTest::addColumn<VTextStreamFunction>("func4");
    QTest::addColumn<int>("value");
    QTest::addColumn<QString>("expected");
    QTest::newRow("dec 1") << noop_ << noop_ << noop_ << noop_ << 31 << "31";
    QTest::newRow("dec 2") << noop_ << base << noop_ << noop_ << 31 << "31";

    QTest::newRow("hex 1") << hex << noop_ << noop_ << noop_ << 31 << "1f";
    QTest::newRow("hex 2") << hex << noop_ << noop_ << lcd << 31 << "1f";
    QTest::newRow("hex 3") << hex << noop_ << ucb << noop_ << 31 << "1f";
    QTest::newRow("hex 4") << hex << noop_ << noop_ << ucd << 31 << "1F";
    QTest::newRow("hex 5") << hex << noop_ << lcb << ucd << 31 << "1F";
    QTest::newRow("hex 6") << hex << noop_ << ucb << ucd << 31 << "1F";
    QTest::newRow("hex 7") << hex << base << noop_ << noop_ << 31 << "0x1f";
    QTest::newRow("hex 8") << hex << base << lcb << lcd << 31 << "0x1f";
    QTest::newRow("hex 9") << hex << base << ucb << noop_ << 31 << "0X1f";
    QTest::newRow("hex 10") << hex << base << ucb << lcd << 31 << "0X1f";
    QTest::newRow("hex 11") << hex << base << noop_ << ucd << 31 << "0x1F";
    QTest::newRow("hex 12") << hex << base << lcb << ucd << 31 << "0x1F";
    QTest::newRow("hex 13") << hex << base << ucb << ucd << 31 << "0X1F";

    QTest::newRow("bin 1") << bin << noop_ << noop_ << noop_ << 31 << "11111";
    QTest::newRow("bin 2") << bin << base << noop_ << noop_ << 31 << "0b11111";
    QTest::newRow("bin 3") << bin << base << lcb << noop_ << 31 << "0b11111";
    QTest::newRow("bin 4") << bin << base << ucb << noop_ << 31 << "0B11111";
    QTest::newRow("bin 5") << bin << base << noop_ << ucd << 31 << "0b11111";
    QTest::newRow("bin 6") << bin << base << lcb << ucd << 31 << "0b11111";
    QTest::newRow("bin 7") << bin << base << ucb << ucd << 31 << "0B11111";

    QTest::newRow("oct 1") << oct << noop_ << noop_ << noop_ << 31 << "37";
    QTest::newRow("oct 2") << oct << base << noop_ << noop_ << 31 << "037";
}

void TST_VTextStream::numeralCase()
{
    QFETCH(VTextStreamFunction, func1);
    QFETCH(VTextStreamFunction, func2);
    QFETCH(VTextStreamFunction, func3);
    QFETCH(VTextStreamFunction, func4);
    QFETCH(int, value);
    QFETCH(QString, expected);

    QString str;
    VTextStream stream(&str);
    stream << func1 << func2 << func3 << func4 << value;
    QCOMPARE(str, expected);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::nanInf()
{
    // Cannot use test data in this function, as comparing nans and infs isn't
    // well defined.
    QString str("nan NAN nAn +nan +NAN +nAn -nan -NAN -nAn"
                " inf INF iNf +inf +INF +iNf -inf -INF -iNf");

    VTextStream stream(&str);

    double tmpD = 0;
    stream >> tmpD;
    QVERIFY(qIsNaN(tmpD));
    tmpD = 0;
    stream >> tmpD;
    QVERIFY(qIsNaN(tmpD));
    tmpD = 0;
    stream >> tmpD;
    QVERIFY(qIsNaN(tmpD));
    tmpD = 0;
    stream >> tmpD;
    QVERIFY(qIsNaN(tmpD));
    tmpD = 0;
    stream >> tmpD;
    QVERIFY(qIsNaN(tmpD));
    tmpD = 0;
    stream >> tmpD;
    QVERIFY(qIsNaN(tmpD));
    tmpD = 0;
    stream >> tmpD;
    QVERIFY(qIsNaN(tmpD));
    tmpD = 0;
    stream >> tmpD;
    QVERIFY(qIsNaN(tmpD));
    tmpD = 0;
    stream >> tmpD;
    QVERIFY(qIsNaN(tmpD));
    tmpD = 0;
    stream >> tmpD;
    QVERIFY(qIsInf(tmpD));
    QVERIFY(tmpD > 0);
    tmpD = 0;
    stream >> tmpD;
    QVERIFY(qIsInf(tmpD));
    QVERIFY(tmpD > 0);
    tmpD = 0;
    stream >> tmpD;
    QVERIFY(qIsInf(tmpD));
    QVERIFY(tmpD > 0);
    tmpD = 0;
    stream >> tmpD;
    QVERIFY(qIsInf(tmpD));
    QVERIFY(tmpD > 0);
    tmpD = 0;
    stream >> tmpD;
    QVERIFY(qIsInf(tmpD));
    QVERIFY(tmpD > 0);
    tmpD = 0;
    stream >> tmpD;
    QVERIFY(qIsInf(tmpD));
    QVERIFY(tmpD > 0);
    tmpD = 0;
    stream >> tmpD;
    QVERIFY(qIsInf(tmpD));
    QVERIFY(tmpD < 0);
    tmpD = 0;
    stream >> tmpD;
    QVERIFY(qIsInf(tmpD));
    QVERIFY(tmpD < 0);
    tmpD = 0;
    stream >> tmpD;
    QVERIFY(qIsInf(tmpD));
    QVERIFY(tmpD < 0);
    tmpD = 0;

    stream.seek(0);

    float tmpF = 0;
    stream >> tmpF;
    QVERIFY(qIsNaN(tmpF));
    tmpD = 0;
    stream >> tmpF;
    QVERIFY(qIsNaN(tmpF));
    tmpD = 0;
    stream >> tmpF;
    QVERIFY(qIsNaN(tmpF));
    tmpD = 0;
    stream >> tmpF;
    QVERIFY(qIsNaN(tmpF));
    tmpD = 0;
    stream >> tmpF;
    QVERIFY(qIsNaN(tmpF));
    tmpD = 0;
    stream >> tmpF;
    QVERIFY(qIsNaN(tmpF));
    tmpD = 0;
    stream >> tmpF;
    QVERIFY(qIsNaN(tmpF));
    tmpD = 0;
    stream >> tmpF;
    QVERIFY(qIsNaN(tmpF));
    tmpD = 0;
    stream >> tmpF;
    QVERIFY(qIsNaN(tmpF));
    tmpD = 0;
    stream >> tmpF;
    QVERIFY(qIsInf(tmpF));
    QVERIFY(tmpF > 0);
    tmpD = 0;
    stream >> tmpF;
    QVERIFY(qIsInf(tmpF));
    QVERIFY(tmpF > 0);
    tmpD = 0;
    stream >> tmpF;
    QVERIFY(qIsInf(tmpF));
    QVERIFY(tmpF > 0);
    tmpD = 0;
    stream >> tmpF;
    QVERIFY(qIsInf(tmpF));
    QVERIFY(tmpF > 0);
    tmpD = 0;
    stream >> tmpF;
    QVERIFY(qIsInf(tmpF));
    QVERIFY(tmpF > 0);
    tmpD = 0;
    stream >> tmpF;
    QVERIFY(qIsInf(tmpF));
    QVERIFY(tmpF > 0);
    tmpD = 0;
    stream >> tmpF;
    QVERIFY(qIsInf(tmpF));
    QVERIFY(tmpF < 0);
    tmpD = 0;
    stream >> tmpF;
    QVERIFY(qIsInf(tmpF));
    QVERIFY(tmpF < 0);
    tmpD = 0;
    stream >> tmpF;
    QVERIFY(qIsInf(tmpF));
    QVERIFY(tmpF < 0);

    QString s;
    VTextStream out(&s);
    out << qInf() << ' ' << -qInf() << ' ' << qQNaN() << Val::uppercasedigits << ' ' << qInf() << ' ' << -qInf() << ' '
        << qQNaN() << Val::flush;

    QCOMPARE(s, QString("inf -inf nan INF -INF NAN"));
}

// ------------------------------------------------------------------------------
void TST_VTextStream::utf8IncompleteAtBufferBoundary_data()
{
    QTest::addColumn<bool>("useLocale");

    QTest::newRow("utf8") << false;

    // is this locale UTF-8?
    if (QString(QChar::ReplacementCharacter).toLocal8Bit() == "\xef\xbf\xbd")
        QTest::newRow("locale") << true;
}

void TST_VTextStream::utf8IncompleteAtBufferBoundary()
{
    QFile::remove(testFileName);
    QFile data(testFileName);

#ifndef WITH_TEXTCODEC
    QSKIP("QTextCodec disabled");
#else

    QTextCodec *utf8Codec = QTextCodec::codecForMib(106);
    QString lineContents = QString::fromUtf8("\342\200\223" // U+2013 EN DASH
                                             "\342\200\223"
                                             "\342\200\223"
                                             "\342\200\223"
                                             "\342\200\223"
                                             "\342\200\223");

    (void) data.open(QFile::WriteOnly | QFile::Truncate);
    {
        VTextStream out(&data);
        out.setCodec(utf8Codec);
        out.setFieldWidth(3);

        for (int i = 0; i < 1000; ++i)
        {
            out << i << lineContents << Val::endl;
        }
    }
    data.close();

    (void) data.open(QFile::ReadOnly);
    VTextStream in(&data);

    QFETCH(bool, useLocale);
    if (!useLocale)
        in.setCodec(utf8Codec); // QUtf8Codec
    else
        in.setCodec(QTextCodec::codecForLocale());

    int i = 0;
    do
    {
        QString line = in.readLine().trimmed();
        ++i;
        QVERIFY2(line.endsWith(lineContents), QString("Line %1: %2").arg(i).arg(line).toLocal8Bit());
    } while (!in.atEnd());
#endif
}

// ------------------------------------------------------------------------------

// Make sure we don't write a BOM after seek()ing

void TST_VTextStream::writeSeekWriteNoBOM()
{
    //First with the default codec (normally either latin-1 or UTF-8)

    QBuffer out;
    out.open(QIODevice::WriteOnly);
    VTextStream stream(&out);

    int number = 0;
    QString sizeStr = QLatin1String("Size=") + QString::number(number).rightJustified(10, QLatin1Char('0'));
    stream << sizeStr << Val::endl;
    stream << "Version=" << QString::number(14) << Val::endl;
    stream << "blah blah blah" << Val::endl;
    stream.flush();

    QCOMPARE(out.buffer().constData(), "Size=0000000000\nVersion=14\nblah blah blah\n");

    // Now overwrite the size header item
    number = 42;
    stream.seek(0);
    sizeStr = QLatin1String("Size=") + QString::number(number).rightJustified(10, QLatin1Char('0'));
    stream << sizeStr << Val::endl;
    stream.flush();

    // Check buffer is still OK
    QCOMPARE(out.buffer().constData(), "Size=0000000042\nVersion=14\nblah blah blah\n");

#ifdef WITH_TEXTCODEC
    //Then UTF-16

    QBuffer out16;
    out16.open(QIODevice::WriteOnly);
    VTextStream stream16(&out16);
    stream16.setCodec("UTF-16");

    stream16 << "one" << "two" << QLatin1String("three");
    stream16.flush();

    // save that output
    QByteArray first = out16.buffer();

    stream16.seek(0);
    stream16 << "one";
    stream16.flush();

    QCOMPARE(out16.buffer(), first);
#endif
}

// ------------------------------------------------------------------------------
void TST_VTextStream::generateOperatorCharData(bool for_QString)
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<QChar>("qchar_output");
    QTest::addColumn<char>("char_output");
    QTest::addColumn<QByteArray>("write_output");

    QTest::newRow("empty") << QByteArray() << QChar('\0') << '\0' << QByteArray("\0", 1);
    QTest::newRow("a") << QByteArray("a") << QChar('a') << 'a' << QByteArray("a");
    QTest::newRow("\\na") << QByteArray("\na") << QChar('\n') << '\n' << QByteArray("\n");
    QTest::newRow("\\0") << QByteArray("\0") << QChar('\0') << '\0' << QByteArray("\0", 1);
    QTest::newRow("\\xff") << QByteArray("\xff") << QChar('\xff') << '\xff' << QByteArray("\xff");
    QTest::newRow("\\xfe") << QByteArray("\xfe") << QChar('\xfe') << '\xfe' << QByteArray("\xfe");

    if (!for_QString)
    {
#ifdef WITH_TEXTCODEC
        QTest::newRow("utf16-BE (empty)") << QByteArray("\xff\xfe", 2) << QChar('\0') << '\0' << QByteArray("\0", 1);
        QTest::newRow("utf16-BE (a)") << QByteArray("\xff\xfe\x61\x00", 4) << QChar('a') << 'a' << QByteArray("a");
        QTest::newRow("utf16-LE (empty)") << QByteArray("\xfe\xff", 2) << QChar('\0') << '\0' << QByteArray("\0", 1);
        QTest::newRow("utf16-LE (a)") << QByteArray("\xfe\xff\x00\x61", 4) << QChar('a') << 'a' << QByteArray("a");
#endif
    }
}

// ------------------------------------------------------------------------------
void TST_VTextStream::QChar_operators_FromDevice_data()
{
    generateOperatorCharData(false);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::QChar_operators_FromDevice()
{
    QFETCH(QByteArray, input);
    QFETCH(QChar, qchar_output);
    QFETCH(QByteArray, write_output);

    QBuffer buf(&input);
    buf.open(QBuffer::ReadOnly);
    VTextStream stream(&buf);
#ifdef WITH_TEXTCODEC
    stream.setCodec(QTextCodec::codecForName("ISO-8859-1"));
#endif
    QChar tmp;
    stream >> tmp;
    QCOMPARE(tmp, qchar_output);

    QBuffer writeBuf;
    writeBuf.open(QBuffer::WriteOnly);

    VTextStream writeStream(&writeBuf);
#ifdef WITH_TEXTCODEC
    writeStream.setCodec(QTextCodec::codecForName("ISO-8859-1"));
#endif
    writeStream << qchar_output;
    writeStream.flush();

    QCOMPARE(writeBuf.buffer().size(), write_output.size());
    QCOMPARE(writeBuf.buffer().constData(), write_output.constData());
}

// ------------------------------------------------------------------------------
void TST_VTextStream::char_operators_FromDevice_data()
{
    generateOperatorCharData(false);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::char_operators_FromDevice()
{
    QFETCH(QByteArray, input);
    QFETCH(char, char_output);
    QFETCH(QByteArray, write_output);

    QBuffer buf(&input);
    buf.open(QBuffer::ReadOnly);
    VTextStream stream(&buf);
#ifdef WITH_TEXTCODEC
    stream.setCodec(QTextCodec::codecForName("ISO-8859-1"));
#endif
    char tmp;
    stream >> tmp;
    QCOMPARE(tmp, char_output);

    QBuffer writeBuf;
    writeBuf.open(QBuffer::WriteOnly);

    VTextStream writeStream(&writeBuf);
#ifdef WITH_TEXTCODEC
    writeStream.setCodec(QTextCodec::codecForName("ISO-8859-1"));
#endif
    writeStream << char_output;
    writeStream.flush();

    QCOMPARE(writeBuf.buffer().size(), write_output.size());
    QCOMPARE(writeBuf.buffer().constData(), write_output.constData());
}

// ------------------------------------------------------------------------------
void TST_VTextStream::generateNaturalNumbersData(bool for_QString)
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<qulonglong>("output");

    QTest::newRow("empty") << QByteArray() << qulonglong(0);
    QTest::newRow("a") << QByteArray("a") << qulonglong(0);
    QTest::newRow(" ") << QByteArray(" ") << qulonglong(0);
    QTest::newRow("0") << QByteArray("0") << qulonglong(0);
    QTest::newRow("1") << QByteArray("1") << qulonglong(1);
    QTest::newRow("12") << QByteArray("12") << qulonglong(12);
    QTest::newRow("-12") << QByteArray("-12") << qulonglong(-12);
    QTest::newRow("-0") << QByteArray("-0") << qulonglong(0);
    QTest::newRow(" 1") << QByteArray(" 1") << qulonglong(1);
    QTest::newRow(" \\r\\n\\r\\n123") << QByteArray(" \r\n\r\n123") << qulonglong(123);

    // bit boundary tests
    QTest::newRow("127") << QByteArray("127") << qulonglong(127);
    QTest::newRow("128") << QByteArray("128") << qulonglong(128);
    QTest::newRow("129") << QByteArray("129") << qulonglong(129);
    QTest::newRow("-127") << QByteArray("-127") << qulonglong(-127);
    QTest::newRow("-128") << QByteArray("-128") << qulonglong(-128);
    QTest::newRow("-129") << QByteArray("-129") << qulonglong(-129);
    QTest::newRow("32767") << QByteArray("32767") << qulonglong(32767);
    QTest::newRow("32768") << QByteArray("32768") << qulonglong(32768);
    QTest::newRow("32769") << QByteArray("32769") << qulonglong(32769);
    QTest::newRow("-32767") << QByteArray("-32767") << qulonglong(-32767);
    QTest::newRow("-32768") << QByteArray("-32768") << qulonglong(-32768);
    QTest::newRow("-32769") << QByteArray("-32769") << qulonglong(-32769);
    QTest::newRow("65537") << QByteArray("65537") << qulonglong(65537);
    QTest::newRow("65536") << QByteArray("65536") << qulonglong(65536);
    QTest::newRow("65535") << QByteArray("65535") << qulonglong(65535);
    QTest::newRow("-65537") << QByteArray("-65537") << qulonglong(-65537);
    QTest::newRow("-65536") << QByteArray("-65536") << qulonglong(-65536);
    QTest::newRow("-65535") << QByteArray("-65535") << qulonglong(-65535);
    QTest::newRow("2147483646") << QByteArray("2147483646") << qulonglong(2147483646);
    QTest::newRow("2147483647") << QByteArray("2147483647") << qulonglong(2147483647);
    QTest::newRow("2147483648") << QByteArray("2147483648") << Q_UINT64_C(2147483648);
    QTest::newRow("-2147483646") << QByteArray("-2147483646") << qulonglong(-2147483646);
    QTest::newRow("-2147483647") << QByteArray("-2147483647") << qulonglong(-2147483647);
    QTest::newRow("-2147483648") << QByteArray("-2147483648") << quint64(-2147483648LL);
    QTest::newRow("4294967296") << QByteArray("4294967296") << Q_UINT64_C(4294967296);
    QTest::newRow("4294967297") << QByteArray("4294967297") << Q_UINT64_C(4294967297);
    QTest::newRow("4294967298") << QByteArray("4294967298") << Q_UINT64_C(4294967298);
    QTest::newRow("-4294967296") << QByteArray("-4294967296") << quint64(-4294967296);
    QTest::newRow("-4294967297") << QByteArray("-4294967297") << quint64(-4294967297);
    QTest::newRow("-4294967298") << QByteArray("-4294967298") << quint64(-4294967298);
    QTest::newRow("9223372036854775807") << QByteArray("9223372036854775807") << Q_UINT64_C(9223372036854775807);
    QTest::newRow("9223372036854775808") << QByteArray("9223372036854775808") << Q_UINT64_C(9223372036854775808);
    QTest::newRow("9223372036854775809") << QByteArray("9223372036854775809") << Q_UINT64_C(9223372036854775809);
    QTest::newRow("18446744073709551615") << QByteArray("18446744073709551615") << Q_UINT64_C(18446744073709551615);
    QTest::newRow("18446744073709551616") << QByteArray("18446744073709551616") << Q_UINT64_C(0);
    QTest::newRow("18446744073709551617") << QByteArray("18446744073709551617") << Q_UINT64_C(1);
    // 18446744073709551617 bytes should be enough for anyone.... ;-)

    // hex tests
    QTest::newRow("0x0") << QByteArray("0x0") << qulonglong(0);
    QTest::newRow("0x") << QByteArray("0x") << qulonglong(0);
    QTest::newRow("0x1") << QByteArray("0x1") << qulonglong(1);
    QTest::newRow("0xf") << QByteArray("0xf") << qulonglong(15);
    QTest::newRow("0xdeadbeef") << QByteArray("0xdeadbeef") << Q_UINT64_C(3735928559);
    QTest::newRow("0XDEADBEEF") << QByteArray("0XDEADBEEF") << Q_UINT64_C(3735928559);
    QTest::newRow("0xdeadbeefZzzzz") << QByteArray("0xdeadbeefZzzzz") << Q_UINT64_C(3735928559);
    QTest::newRow("  0xdeadbeefZzzzz") << QByteArray("  0xdeadbeefZzzzz") << Q_UINT64_C(3735928559);

    // oct tests
    QTest::newRow("00") << QByteArray("00") << qulonglong(0);
    QTest::newRow("0141") << QByteArray("0141") << qulonglong(97);
    QTest::newRow("01419999") << QByteArray("01419999") << qulonglong(97);
    QTest::newRow("  01419999") << QByteArray("  01419999") << qulonglong(97);

    // bin tests
    QTest::newRow("0b0") << QByteArray("0b0") << qulonglong(0);
    QTest::newRow("0b1") << QByteArray("0b1") << qulonglong(1);
    QTest::newRow("0b10") << QByteArray("0b10") << qulonglong(2);
    QTest::newRow("0B10") << QByteArray("0B10") << qulonglong(2);
    QTest::newRow("0b101010") << QByteArray("0b101010") << qulonglong(42);
    QTest::newRow("0b1010102345") << QByteArray("0b1010102345") << qulonglong(42);
    QTest::newRow("  0b1010102345") << QByteArray("  0b1010102345") << qulonglong(42);

    // utf-16 tests
    if (!for_QString)
    {
#ifdef WITH_TEXTCODEC
        QTest::newRow("utf16-BE (empty)") << QByteArray("\xfe\xff", 2) << qulonglong(0);
        QTest::newRow("utf16-BE (0xdeadbeef)")
            << QByteArray("\xfe\xff"
                          "\x00\x30\x00\x78\x00\x64\x00\x65\x00\x61\x00\x64\x00\x62\x00\x65\x00\x65\x00\x66",
                          22)
            << Q_UINT64_C(3735928559);
        QTest::newRow("utf16-LE (empty)") << QByteArray("\xff\xfe", 2) << Q_UINT64_C(0);
        QTest::newRow("utf16-LE (0xdeadbeef)")
            << QByteArray("\xff\xfe"
                          "\x30\x00\x78\x00\x64\x00\x65\x00\x61\x00\x64\x00\x62\x00\x65\x00\x65\x00\x66\x00",
                          22)
            << Q_UINT64_C(3735928559);
#endif
    }
}

// ------------------------------------------------------------------------------
#define IMPLEMENT_STREAM_RIGHT_INT_OPERATOR_TEST(texttype, type) \
    void TST_VTextStream::texttype##_read_operator_FromDevice_data() \
    { \
        generateNaturalNumbersData(false); \
    } \
    void TST_VTextStream::texttype##_read_operator_FromDevice() \
    { \
        QFETCH(QByteArray, input); \
        QFETCH(qulonglong, output); \
        type sh; \
        VTextStream stream(&input); \
        stream >> sh; \
        QCOMPARE(sh, static_cast<type>(output)); \
    }
IMPLEMENT_STREAM_RIGHT_INT_OPERATOR_TEST(signedShort, signed short)
IMPLEMENT_STREAM_RIGHT_INT_OPERATOR_TEST(unsignedShort, unsigned short)
IMPLEMENT_STREAM_RIGHT_INT_OPERATOR_TEST(signedInt, signed int)
IMPLEMENT_STREAM_RIGHT_INT_OPERATOR_TEST(unsignedInt, unsigned int)
IMPLEMENT_STREAM_RIGHT_INT_OPERATOR_TEST(qlonglong, qlonglong)
IMPLEMENT_STREAM_RIGHT_INT_OPERATOR_TEST(qulonglong, qulonglong)

// ------------------------------------------------------------------------------
void TST_VTextStream::generateRealNumbersData(bool for_QString)
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<double>("output");

    QTest::newRow("empty") << QByteArray() << 0.0;
    QTest::newRow("a") << QByteArray("a") << 0.0;
    QTest::newRow("1.0") << QByteArray("1.0") << 1.0;
    QTest::newRow(" 1") << QByteArray(" 1") << 1.0;
    QTest::newRow(" \\r\\n1.2") << QByteArray(" \r\n1.2") << 1.2;
    QTest::newRow("3.14") << QByteArray("3.14") << 3.14;
    QTest::newRow("-3.14") << QByteArray("-3.14") << -3.14;
    QTest::newRow(" -3.14") << QByteArray(" -3.14") << -3.14;
    QTest::newRow("314e-02") << QByteArray("314e-02") << 3.14;
    QTest::newRow("314E-02") << QByteArray("314E-02") << 3.14;
    QTest::newRow("314e+02") << QByteArray("314e+02") << 31400.;
    QTest::newRow("314E+02") << QByteArray("314E+02") << 31400.;

    // ### add numbers with exponents

    if (!for_QString)
    {
        QTest::newRow("utf16-BE (empty)") << QByteArray("\xff\xfe", 2) << 0.0;
        QTest::newRow("utf16-LE (empty)") << QByteArray("\xfe\xff", 2) << 0.0;
    }
}

// ------------------------------------------------------------------------------
#define IMPLEMENT_STREAM_RIGHT_REAL_OPERATOR_TEST(texttype, type) \
    void TST_VTextStream::texttype##_read_operator_FromDevice_data() \
    { \
        generateRealNumbersData(false); \
    } \
    void TST_VTextStream::texttype##_read_operator_FromDevice() \
    { \
        QFETCH(QByteArray, input); \
        QFETCH(double, output); \
        type sh; \
        VTextStream stream(&input); \
        stream >> sh; \
        QCOMPARE(sh, static_cast<type>(output)); \
    }
IMPLEMENT_STREAM_RIGHT_REAL_OPERATOR_TEST(float, float)
IMPLEMENT_STREAM_RIGHT_REAL_OPERATOR_TEST(double, double)

// ------------------------------------------------------------------------------
void TST_VTextStream::generateStringData(bool for_QString)
{
    QTest::addColumn<QByteArray>("input");
    QTest::addColumn<QByteArray>("array_output");
    QTest::addColumn<QString>("string_output");

    QTest::newRow("empty") << QByteArray() << QByteArray() << QString();
    QTest::newRow("a") << QByteArray("a") << QByteArray("a") << QString("a");
    QTest::newRow("a b") << QByteArray("a b") << QByteArray("a") << QString("a");
    QTest::newRow(" a b") << QByteArray(" a b") << QByteArray("a") << QString("a");
    QTest::newRow("a1") << QByteArray("a1") << QByteArray("a1") << QString("a1");
    QTest::newRow("a1 b1") << QByteArray("a1 b1") << QByteArray("a1") << QString("a1");
    QTest::newRow(" a1 b1") << QByteArray(" a1 b1") << QByteArray("a1") << QString("a1");
    QTest::newRow("\\n\\n\\nole i dole\\n") << QByteArray("\n\n\nole i dole\n") << QByteArray("ole") << QString("ole");

    if (!for_QString)
    {
#ifdef WITH_TEXTCODEC
        QTest::newRow("utf16-BE (empty)") << QByteArray("\xff\xfe", 2) << QByteArray() << QString();
        QTest::newRow("utf16-BE (corrupt)")
            << QByteArray("\xff", 1) << QByteArray("\xff") << QString::fromLatin1("\xff");
        QTest::newRow("utf16-LE (empty)") << QByteArray("\xfe\xff", 2) << QByteArray() << QString();
        QTest::newRow("utf16-LE (corrupt)")
            << QByteArray("\xfe", 1) << QByteArray("\xfe") << QString::fromLatin1("\xfe");
#endif
    }
}

// ------------------------------------------------------------------------------
void TST_VTextStream::charPtr_read_operator_FromDevice_data()
{
    generateStringData(false);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::charPtr_read_operator_FromDevice()
{
    QFETCH(QByteArray, input);
    QFETCH(QByteArray, array_output);

    QBuffer buffer(&input);
    buffer.open(QBuffer::ReadOnly);
    VTextStream stream(&buffer);
#ifdef WITH_TEXTCODEC
    stream.setCodec(QTextCodec::codecForName("ISO-8859-1"));
    stream.setAutoDetectUnicode(true);
#endif

    char buf[1024];
    stream >> buf;

    QCOMPARE(reinterpret_cast<const char *>(buf), array_output.constData());
}

// ------------------------------------------------------------------------------
void TST_VTextStream::stringRef_read_operator_FromDevice_data()
{
    generateStringData(false);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::stringRef_read_operator_FromDevice()
{
    QFETCH(QByteArray, input);
    QFETCH(QString, string_output);

    QBuffer buffer(&input);
    buffer.open(QBuffer::ReadOnly);
    VTextStream stream(&buffer);
#ifdef WITH_TEXTCODEC
    stream.setCodec(QTextCodec::codecForName("ISO-8859-1"));
    stream.setAutoDetectUnicode(true);
#endif

    QString tmp;
    stream >> tmp;

    QCOMPARE(tmp, string_output);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::byteArray_read_operator_FromDevice_data()
{
    generateStringData(false);
}

// ------------------------------------------------------------------------------
void TST_VTextStream::byteArray_read_operator_FromDevice()
{
    QFETCH(QByteArray, input);
    QFETCH(QByteArray, array_output);

    QBuffer buffer(&input);
    buffer.open(QBuffer::ReadOnly);
    VTextStream stream(&buffer);
#ifdef WITH_TEXTCODEC
    stream.setCodec(QTextCodec::codecForName("ISO-8859-1"));
    stream.setAutoDetectUnicode(true);
#endif

    QByteArray array;
    stream >> array;

    QCOMPARE(array, array_output);
}

// ------------------------------------------------------------------------------
#define IMPLEMENT_STREAM_LEFT_INT_OPERATOR_TEST(texttype, type) \
    void TST_VTextStream::texttype##_write_operator_ToDevice() \
    { \
        QFETCH(qulonglong, number); \
        QFETCH(QByteArray, data); \
        QFETCH(QByteArray, dataWithSeparators); \
\
        QBuffer buffer; \
        buffer.open(QBuffer::WriteOnly); \
        VTextStream stream(&buffer); \
        stream.setLocale(QLocale::c()); \
        stream << static_cast<type>(number); \
        stream.flush(); \
        QCOMPARE(buffer.data().constData(), data.constData()); \
\
        QLocale locale("en-US"); \
        buffer.reset(); \
        buffer.buffer().clear(); \
        stream.setLocale(locale); \
        stream << static_cast<type>(number); \
        stream.flush(); \
        QCOMPARE(buffer.data(), dataWithSeparators); \
\
        locale.setNumberOptions(QLocale::OmitGroupSeparator); \
        buffer.reset(); \
        buffer.buffer().clear(); \
        stream.setLocale(locale); \
        stream << static_cast<type>(number); \
        stream.flush(); \
        QCOMPARE(buffer.data().constData(), data.constData()); \
\
        locale = QLocale("de-DE"); \
        buffer.reset(); \
        buffer.buffer().clear(); \
        stream.setLocale(locale); \
        stream << static_cast<type>(number); \
        stream.flush(); \
        QCOMPARE(buffer.data(), dataWithSeparators.replace(',', '.')); \
    }

// ------------------------------------------------------------------------------
void TST_VTextStream::signedShort_write_operator_ToDevice_data()
{
    QTest::addColumn<qulonglong>("number");
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("dataWithSeparators");

    QTest::newRow("0") << Q_UINT64_C(0) << QByteArray("0") << QByteArray("0");
    QTest::newRow("1") << Q_UINT64_C(1) << QByteArray("1") << QByteArray("1");
    QTest::newRow("-1") << quint64(-1) << QByteArray("-1") << QByteArray("-1");
    QTest::newRow("32767") << Q_UINT64_C(32767) << QByteArray("32767") << QByteArray("32,767");
    QTest::newRow("32768") << Q_UINT64_C(32768) << QByteArray("-32768") << QByteArray("-32,768");
    QTest::newRow("32769") << Q_UINT64_C(32769) << QByteArray("-32767") << QByteArray("-32,767");
    QTest::newRow("65535") << Q_UINT64_C(65535) << QByteArray("-1") << QByteArray("-1");
    QTest::newRow("65536") << Q_UINT64_C(65536) << QByteArray("0") << QByteArray("0");
    QTest::newRow("65537") << Q_UINT64_C(65537) << QByteArray("1") << QByteArray("1");
}
IMPLEMENT_STREAM_LEFT_INT_OPERATOR_TEST(signedShort, signed short)

// ------------------------------------------------------------------------------
void TST_VTextStream::unsignedShort_write_operator_ToDevice_data()
{
    QTest::addColumn<qulonglong>("number");
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("dataWithSeparators");

    QTest::newRow("0") << Q_UINT64_C(0) << QByteArray("0") << QByteArray("0");
    QTest::newRow("1") << Q_UINT64_C(1) << QByteArray("1") << QByteArray("1");
    QTest::newRow("-1") << quint64(-1) << QByteArray("65535") << QByteArray("65,535");
    QTest::newRow("32767") << Q_UINT64_C(32767) << QByteArray("32767") << QByteArray("32,767");
    QTest::newRow("32768") << Q_UINT64_C(32768) << QByteArray("32768") << QByteArray("32,768");
    QTest::newRow("32769") << Q_UINT64_C(32769) << QByteArray("32769") << QByteArray("32,769");
    QTest::newRow("65535") << Q_UINT64_C(65535) << QByteArray("65535") << QByteArray("65,535");
    QTest::newRow("65536") << Q_UINT64_C(65536) << QByteArray("0") << QByteArray("0");
    QTest::newRow("65537") << Q_UINT64_C(65537) << QByteArray("1") << QByteArray("1");
}
IMPLEMENT_STREAM_LEFT_INT_OPERATOR_TEST(unsignedShort, unsigned short)

// ------------------------------------------------------------------------------
void TST_VTextStream::signedInt_write_operator_ToDevice_data()
{
    QTest::addColumn<qulonglong>("number");
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("dataWithSeparators");

    QTest::newRow("0") << Q_UINT64_C(0) << QByteArray("0") << QByteArray("0");
    QTest::newRow("1") << Q_UINT64_C(1) << QByteArray("1") << QByteArray("1");
    QTest::newRow("-1") << quint64(-1) << QByteArray("-1") << QByteArray("-1");
    QTest::newRow("32767") << Q_UINT64_C(32767) << QByteArray("32767") << QByteArray("32,767");
    QTest::newRow("32768") << Q_UINT64_C(32768) << QByteArray("32768") << QByteArray("32,768");
    QTest::newRow("32769") << Q_UINT64_C(32769) << QByteArray("32769") << QByteArray("32,769");
    QTest::newRow("65535") << Q_UINT64_C(65535) << QByteArray("65535") << QByteArray("65,535");
    QTest::newRow("65536") << Q_UINT64_C(65536) << QByteArray("65536") << QByteArray("65,536");
    QTest::newRow("65537") << Q_UINT64_C(65537) << QByteArray("65537") << QByteArray("65,537");
    QTest::newRow("2147483647") << Q_UINT64_C(2147483647) << QByteArray("2147483647") << QByteArray("2,147,483,647");
    QTest::newRow("2147483648") << Q_UINT64_C(2147483648) << QByteArray("-2147483648") << QByteArray("-2,147,483,648");
    QTest::newRow("2147483649") << Q_UINT64_C(2147483649) << QByteArray("-2147483647") << QByteArray("-2,147,483,647");
    QTest::newRow("4294967295") << Q_UINT64_C(4294967295) << QByteArray("-1") << QByteArray("-1");
    QTest::newRow("4294967296") << Q_UINT64_C(4294967296) << QByteArray("0") << QByteArray("0");
    QTest::newRow("4294967297") << Q_UINT64_C(4294967297) << QByteArray("1") << QByteArray("1");
}
IMPLEMENT_STREAM_LEFT_INT_OPERATOR_TEST(signedInt, signed int)

// ------------------------------------------------------------------------------
void TST_VTextStream::unsignedInt_write_operator_ToDevice_data()
{
    QTest::addColumn<qulonglong>("number");
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("dataWithSeparators");

    QTest::newRow("0") << Q_UINT64_C(0) << QByteArray("0") << QByteArray("0");
    QTest::newRow("1") << Q_UINT64_C(1) << QByteArray("1") << QByteArray("1");
    QTest::newRow("-1") << quint64(-1) << QByteArray("4294967295") << QByteArray("4,294,967,295");
    QTest::newRow("32767") << Q_UINT64_C(32767) << QByteArray("32767") << QByteArray("32,767");
    QTest::newRow("32768") << Q_UINT64_C(32768) << QByteArray("32768") << QByteArray("32,768");
    QTest::newRow("32769") << Q_UINT64_C(32769) << QByteArray("32769") << QByteArray("32,769");
    QTest::newRow("65535") << Q_UINT64_C(65535) << QByteArray("65535") << QByteArray("65,535");
    QTest::newRow("65536") << Q_UINT64_C(65536) << QByteArray("65536") << QByteArray("65,536");
    QTest::newRow("65537") << Q_UINT64_C(65537) << QByteArray("65537") << QByteArray("65,537");
    QTest::newRow("2147483647") << Q_UINT64_C(2147483647) << QByteArray("2147483647") << QByteArray("2,147,483,647");
    QTest::newRow("2147483648") << Q_UINT64_C(2147483648) << QByteArray("2147483648") << QByteArray("2,147,483,648");
    QTest::newRow("2147483649") << Q_UINT64_C(2147483649) << QByteArray("2147483649") << QByteArray("2,147,483,649");
    QTest::newRow("4294967295") << Q_UINT64_C(4294967295) << QByteArray("4294967295") << QByteArray("4,294,967,295");
    QTest::newRow("4294967296") << Q_UINT64_C(4294967296) << QByteArray("0") << QByteArray("0");
    QTest::newRow("4294967297") << Q_UINT64_C(4294967297) << QByteArray("1") << QByteArray("1");
}
IMPLEMENT_STREAM_LEFT_INT_OPERATOR_TEST(unsignedInt, unsigned int)

// ------------------------------------------------------------------------------
void TST_VTextStream::qlonglong_write_operator_ToDevice_data()
{
    QTest::addColumn<qulonglong>("number");
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("dataWithSeparators");

    QTest::newRow("0") << Q_UINT64_C(0) << QByteArray("0") << QByteArray("0");
    QTest::newRow("1") << Q_UINT64_C(1) << QByteArray("1") << QByteArray("1");
    QTest::newRow("-1") << quint64(-1) << QByteArray("-1") << QByteArray("-1");
    QTest::newRow("32767") << Q_UINT64_C(32767) << QByteArray("32767") << QByteArray("32,767");
    QTest::newRow("32768") << Q_UINT64_C(32768) << QByteArray("32768") << QByteArray("32,768");
    QTest::newRow("32769") << Q_UINT64_C(32769) << QByteArray("32769") << QByteArray("32,769");
    QTest::newRow("65535") << Q_UINT64_C(65535) << QByteArray("65535") << QByteArray("65,535");
    QTest::newRow("65536") << Q_UINT64_C(65536) << QByteArray("65536") << QByteArray("65,536");
    QTest::newRow("65537") << Q_UINT64_C(65537) << QByteArray("65537") << QByteArray("65,537");
    QTest::newRow("2147483647") << Q_UINT64_C(2147483647) << QByteArray("2147483647") << QByteArray("2,147,483,647");
    QTest::newRow("2147483648") << Q_UINT64_C(2147483648) << QByteArray("2147483648") << QByteArray("2,147,483,648");
    QTest::newRow("2147483649") << Q_UINT64_C(2147483649) << QByteArray("2147483649") << QByteArray("2,147,483,649");
    QTest::newRow("4294967295") << Q_UINT64_C(4294967295) << QByteArray("4294967295") << QByteArray("4,294,967,295");
    QTest::newRow("4294967296") << Q_UINT64_C(4294967296) << QByteArray("4294967296") << QByteArray("4,294,967,296");
    QTest::newRow("4294967297") << Q_UINT64_C(4294967297) << QByteArray("4294967297") << QByteArray("4,294,967,297");
    QTest::newRow("9223372036854775807") << Q_UINT64_C(9223372036854775807) << QByteArray("9223372036854775807")
                                         << QByteArray("9,223,372,036,854,775,807");
    QTest::newRow("9223372036854775808") << Q_UINT64_C(9223372036854775808) << QByteArray("-9223372036854775808")
                                         << QByteArray("-9,223,372,036,854,775,808");
    QTest::newRow("9223372036854775809") << Q_UINT64_C(9223372036854775809) << QByteArray("-9223372036854775807")
                                         << QByteArray("-9,223,372,036,854,775,807");
    QTest::newRow("18446744073709551615") << Q_UINT64_C(18446744073709551615) << QByteArray("-1") << QByteArray("-1");
}
IMPLEMENT_STREAM_LEFT_INT_OPERATOR_TEST(qlonglong, qlonglong)

// ------------------------------------------------------------------------------
void TST_VTextStream::qulonglong_write_operator_ToDevice_data()
{
    QTest::addColumn<qulonglong>("number");
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("dataWithSeparators");

    QTest::newRow("0") << Q_UINT64_C(0) << QByteArray("0") << QByteArray("0");
    QTest::newRow("1") << Q_UINT64_C(1) << QByteArray("1") << QByteArray("1");
    QTest::newRow("-1") << static_cast<quint64>(-1) << QByteArray("18446744073709551615")
                        << QByteArray("18,446,744,073,709,551,615");
    QTest::newRow("32767") << Q_UINT64_C(32767) << QByteArray("32767") << QByteArray("32,767");
    QTest::newRow("32768") << Q_UINT64_C(32768) << QByteArray("32768") << QByteArray("32,768");
    QTest::newRow("32769") << Q_UINT64_C(32769) << QByteArray("32769") << QByteArray("32,769");
    QTest::newRow("65535") << Q_UINT64_C(65535) << QByteArray("65535") << QByteArray("65,535");
    QTest::newRow("65536") << Q_UINT64_C(65536) << QByteArray("65536") << QByteArray("65,536");
    QTest::newRow("65537") << Q_UINT64_C(65537) << QByteArray("65537") << QByteArray("65,537");
    QTest::newRow("2147483647") << Q_UINT64_C(2147483647) << QByteArray("2147483647") << QByteArray("2,147,483,647");
    QTest::newRow("2147483648") << Q_UINT64_C(2147483648) << QByteArray("2147483648") << QByteArray("2,147,483,648");
    QTest::newRow("2147483649") << Q_UINT64_C(2147483649) << QByteArray("2147483649") << QByteArray("2,147,483,649");
    QTest::newRow("4294967295") << Q_UINT64_C(4294967295) << QByteArray("4294967295") << QByteArray("4,294,967,295");
    QTest::newRow("4294967296") << Q_UINT64_C(4294967296) << QByteArray("4294967296") << QByteArray("4,294,967,296");
    QTest::newRow("4294967297") << Q_UINT64_C(4294967297) << QByteArray("4294967297") << QByteArray("4,294,967,297");
    QTest::newRow("9223372036854775807") << Q_UINT64_C(9223372036854775807) << QByteArray("9223372036854775807")
                                         << QByteArray("9,223,372,036,854,775,807");
    QTest::newRow("9223372036854775808") << Q_UINT64_C(9223372036854775808) << QByteArray("9223372036854775808")
                                         << QByteArray("9,223,372,036,854,775,808");
    QTest::newRow("9223372036854775809") << Q_UINT64_C(9223372036854775809) << QByteArray("9223372036854775809")
                                         << QByteArray("9,223,372,036,854,775,809");
    QTest::newRow("18446744073709551615") << Q_UINT64_C(18446744073709551615) << QByteArray("18446744073709551615")
                                          << QByteArray("18,446,744,073,709,551,615");
}
IMPLEMENT_STREAM_LEFT_INT_OPERATOR_TEST(qulonglong, qulonglong)

// ------------------------------------------------------------------------------
void TST_VTextStream::generateRealNumbersDataWrite()
{
    QTest::addColumn<double>("number");
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<QByteArray>("dataWithSeparators");

    QTest::newRow("0") << 0.0 << QByteArray("0") << QByteArray("0");
    QTest::newRow("3.14") << 3.14 << QByteArray("3.14") << QByteArray("3.14");
    QTest::newRow("-3.14") << -3.14 << QByteArray("-3.14") << QByteArray("-3.14");
    QTest::newRow("1.2e+10") << 1.2e+10 << QByteArray("1.2e+10") << QByteArray("1.2e+10");
    QTest::newRow("-1.2e+10") << -1.2e+10 << QByteArray("-1.2e+10") << QByteArray("-1.2e+10");
    QTest::newRow("12345") << 12345. << QByteArray("12345") << QByteArray("12,345");
}

// ------------------------------------------------------------------------------
#define IMPLEMENT_STREAM_LEFT_REAL_OPERATOR_TEST(texttype, type) \
    void TST_VTextStream::texttype##_write_operator_ToDevice_data() \
    { \
        generateRealNumbersDataWrite(); \
    } \
    void TST_VTextStream::texttype##_write_operator_ToDevice() \
    { \
        QFETCH(double, number); \
        QFETCH(QByteArray, data); \
        QFETCH(QByteArray, dataWithSeparators); \
\
        QBuffer buffer; \
        buffer.open(QBuffer::WriteOnly); \
        VTextStream stream(&buffer); \
        stream.setLocale(QLocale::c()); \
        float f = static_cast<float>(number); \
        stream << f; \
        stream.flush(); \
        QCOMPARE(buffer.data().constData(), data.constData()); \
\
        buffer.reset(); \
        stream.setLocale(QLocale("en-US")); \
        stream << f; \
        stream.flush(); \
        QCOMPARE(buffer.data(), dataWithSeparators); \
    }
IMPLEMENT_STREAM_LEFT_REAL_OPERATOR_TEST(float, float)
IMPLEMENT_STREAM_LEFT_REAL_OPERATOR_TEST(double, float)

// ------------------------------------------------------------------------------
void TST_VTextStream::string_write_operator_ToDevice_data()
{
    QTest::addColumn<QByteArray>("bytedata");
    QTest::addColumn<QString>("stringdata");
    QTest::addColumn<QByteArray>("result");

    QTest::newRow("empty") << QByteArray("", 1) << QString(1, '\0') << QByteArray("", 1);
    QTest::newRow("a") << QByteArray("a") << QString("a") << QByteArray("a");
    QTest::newRow("a cow jumped over the moon")
        << QByteArray("a cow jumped over the moon") << QString("a cow jumped over the moon")
        << QByteArray("a cow jumped over the moon");

    // ### get the utf16-be test on its legs.
    /*
    QTest::newRow("utf16-BE (a cow jumped over the moon)")
        << QByteArray("\xff\xfe\x00\x61\x00\x20\x00\x63\x00\x6f\x00\x77\x00\x20\x00\x6a\x00\x75\x00\x6d\x00\x70\x00\x65\x00\x64\x00\x20\x00\x6f\x00\x76\x00\x65\x00\x72\x00\x20\x00\x74\x00\x68\x00\x65\x00\x20\x00\x6d\x00\x6f\x00\x6f\x00\x6e\x00\x0a", 56)
        << QString("a cow jumped over the moon")
        << QByteArray("a cow jumped over the moon");
    */
}

// ------------------------------------------------------------------------------
void TST_VTextStream::string_write_operator_ToDevice()
{
    QFETCH(QByteArray, bytedata);
    QFETCH(QString, stringdata);
    QFETCH(QByteArray, result);

    {
        // char*
        QBuffer buf;
        buf.open(QBuffer::WriteOnly);
        VTextStream stream(&buf);
#ifdef WITH_TEXTCODEC
        stream.setCodec(QTextCodec::codecForName("ISO-8859-1"));
        stream.setAutoDetectUnicode(true);
#endif

        stream << bytedata.constData();
        stream.flush();
        QCOMPARE(buf.buffer().constData(), result.constData());
    }
    {
        // QByteArray
        QBuffer buf;
        buf.open(QBuffer::WriteOnly);
        VTextStream stream(&buf);
#ifdef WITH_TEXTCODEC
        stream.setCodec(QTextCodec::codecForName("ISO-8859-1"));
        stream.setAutoDetectUnicode(true);
#endif

        stream << bytedata;
        stream.flush();
        QCOMPARE(buf.buffer().constData(), result.constData());
    }
    {
        // QString
        QBuffer buf;
        buf.open(QBuffer::WriteOnly);
        VTextStream stream(&buf);
#ifdef WITH_TEXTCODEC
        stream.setCodec(QTextCodec::codecForName("ISO-8859-1"));
        stream.setAutoDetectUnicode(true);
#endif

        stream << stringdata;
        stream.flush();
        QCOMPARE(buf.buffer().constData(), result.constData());
    }
}

void TST_VTextStream::latin1String_write_operator_ToDevice()
{
    QBuffer buf;
    buf.open(QBuffer::WriteOnly);
    VTextStream stream(&buf);
#ifdef WITH_TEXTCODEC
    stream.setCodec(QTextCodec::codecForName("ISO-8859-1"));
    stream.setAutoDetectUnicode(true);
#endif

    stream << QLatin1String("No explicit length");
    stream << QLatin1String("Explicit length - ignore this part", 15);
    stream.flush();
    QCOMPARE(buf.buffer().constData(), "No explicit lengthExplicit length");
}

void TST_VTextStream::stringview_write_operator_ToDevice()
{
    QBuffer buf;
    buf.open(QBuffer::WriteOnly);
    VTextStream stream(&buf);
    const QStringView expected = u"expectedStringView";
    stream << expected;
    stream.flush();
    QCOMPARE(buf.buffer().constData(), "expectedStringView");
}

// ------------------------------------------------------------------------------
void TST_VTextStream::useCase1()
{
    QFile::remove("testfile");
    QFile file("testfile");
    QVERIFY(file.open(QFile::ReadWrite));

    {
        VTextStream stream(&file);
#ifdef WITH_TEXTCODEC
        stream.setCodec(QTextCodec::codecForName("ISO-8859-1"));
        stream.setAutoDetectUnicode(true);
#endif

        stream << 4.15 << ' ' << QByteArray("abc") << ' ' << QString("ole");
    }

    file.seek(0);
    QCOMPARE(file.readAll(), QByteArray("4.15 abc ole"));
    file.seek(0);

    {
        double d;
        QByteArray a;
        QString s;
        VTextStream stream(&file);
#ifdef WITH_TEXTCODEC
        stream.setCodec(QTextCodec::codecForName("ISO-8859-1"));
        stream.setAutoDetectUnicode(true);
#endif

        stream >> d;
        stream >> a;
        stream >> s;

        QCOMPARE(d, 4.15);
        QCOMPARE(a, QByteArray("abc"));
        QCOMPARE(s, QString("ole"));
    }
}

// ------------------------------------------------------------------------------
void TST_VTextStream::useCase2()
{
    QFile::remove("testfile");
    QFile file("testfile");
    QVERIFY(file.open(QFile::ReadWrite));

    VTextStream stream(&file);
#ifdef WITH_TEXTCODEC
    stream.setCodec(QTextCodec::codecForName("ISO-8859-1"));
    stream.setAutoDetectUnicode(true);
#endif

    stream << 4.15 << ' ' << QByteArray("abc") << ' ' << QString("ole");

    file.close();
    QVERIFY(file.open(QFile::ReadWrite));

    QCOMPARE(file.readAll(), QByteArray("4.15 abc ole"));

    file.close();
    QVERIFY(file.open(QFile::ReadWrite));

    double d;
    QByteArray a;
    QString s;
    VTextStream stream2(&file);
#ifdef WITH_TEXTCODEC
    stream2.setCodec(QTextCodec::codecForName("ISO-8859-1"));
    stream2.setAutoDetectUnicode(true);
#endif

    stream2 >> d;
    stream2 >> a;
    stream2 >> s;

    QCOMPARE(d, 4.15);
    QCOMPARE(a, QByteArray("abc"));
    QCOMPARE(s, QString("ole"));
}

// ------------------------------------------------------------------------------
void TST_VTextStream::manipulators_data()
{
    QTest::addColumn<int>("base");
    QTest::addColumn<int>("alignFlag");
    QTest::addColumn<int>("numberFlag");
    QTest::addColumn<int>("width");
    QTest::addColumn<double>("realNumber");
    QTest::addColumn<int>("intNumber");
    QTest::addColumn<QString>("textData");
    QTest::addColumn<QByteArray>("result");

    QTest::newRow("no flags") << 10 << 0 << 0 << 0 << 5.0 << 5 << QString("five") << QByteArray("55five");
    QTest::newRow("rightadjust") << 10 << int(VTextStream::AlignRight) << 0 << 10 << 5.0 << 5 << QString("five")
                                 << QByteArray("         5         5      five");
    QTest::newRow("leftadjust") << 10 << int(VTextStream::AlignLeft) << 0 << 10 << 5.0 << 5 << QString("five")
                                << QByteArray("5         5         five      ");
    QTest::newRow("showpos") << 10 << int(VTextStream::AlignRight) << int(VTextStream::ForceSign) << 10 << 5.0 << 5
                             << QString("five") << QByteArray("        +5        +5      five");
    QTest::newRow("showpos2") << 10 << int(VTextStream::AlignRight) << int(VTextStream::ForceSign) << 5 << 3.14 << -5
                              << QString("five") << QByteArray("+3.14   -5 five");
    QTest::newRow("hex") << 16 << int(VTextStream::AlignRight) << int(VTextStream::ShowBase) << 5 << 3.14 << -5
                         << QString("five") << QByteArray(" 3.14 -0x5 five");
    QTest::newRow("hex") << 16 << int(VTextStream::AlignRight)
                         << int(VTextStream::ShowBase | VTextStream::UppercaseBase) << 5 << 3.14 << -5
                         << QString("five") << QByteArray(" 3.14 -0X5 five");
}

// ------------------------------------------------------------------------------
void TST_VTextStream::manipulators()
{
    QFETCH(int, base);
    QFETCH(int, alignFlag);
    QFETCH(int, numberFlag);
    QFETCH(int, width);
    QFETCH(double, realNumber);
    QFETCH(int, intNumber);
    QFETCH(QString, textData);
    QFETCH(QByteArray, result);

    QBuffer buffer;
    buffer.open(QBuffer::WriteOnly);

    VTextStream stream(&buffer);
#ifdef WITH_TEXTCODEC
    stream.setCodec(QTextCodec::codecForName("ISO-8859-1"));
    stream.setAutoDetectUnicode(true);
#endif

    stream.setIntegerBase(base);
    stream.setFieldAlignment(VTextStream::FieldAlignment(alignFlag));
    stream.setNumberFlags(VTextStream::NumberFlag(numberFlag));
    stream.setFieldWidth(width);
    stream << realNumber;
    stream << intNumber;
    stream << textData;
    stream.flush();

    QCOMPARE(buffer.data(), result);
}

void TST_VTextStream::generateBOM()
{
#ifdef WITH_TEXTCODEC
    QFile::remove("bom.txt");
    {
        QFile file("bom.txt");
        QVERIFY(file.open(QFile::ReadWrite | QFile::Truncate));

        VTextStream stream(&file);
        stream.setCodec(QTextCodec::codecForName("UTF-16LE"));
        stream << "Hello" << Val::endl;

        file.close();
        QVERIFY(file.open(QFile::ReadOnly));
        QCOMPARE(file.readAll(), QByteArray("\x48\x00\x65\00\x6c\00\x6c\00\x6f\x00\x0a\x00", 12));
    }

    QFile::remove("bom.txt");
    {
        QFile file("bom.txt");
        QVERIFY(file.open(QFile::ReadWrite | QFile::Truncate));

        VTextStream stream(&file);
        stream.setCodec(QTextCodec::codecForName("UTF-16LE"));
        stream << Val::bom << "Hello" << Val::endl;

        file.close();
        QVERIFY(file.open(QFile::ReadOnly));
        QCOMPARE(file.readAll(), QByteArray("\xff\xfe\x48\x00\x65\00\x6c\00\x6c\00\x6f\x00\x0a\x00", 14));
    }
#else
    QSKIP("QTextCodec disabled");
#endif
}

void TST_VTextStream::readBomSeekBackReadBomAgain()
{
#ifdef WITH_TEXTCODEC
    QFile::remove("utf8bom");
    QFile file("utf8bom");
    QVERIFY(file.open(QFile::ReadWrite));
    file.write("\xef\xbb\xbf"
               "Andreas");
    file.seek(0);
    QCOMPARE(file.pos(), qint64(0));

    VTextStream stream(&file);
    stream.setCodec("UTF-8");
    QString Andreas;
    stream >> Andreas;
    QCOMPARE(Andreas, QString("Andreas"));
    stream.seek(0);
    stream >> Andreas;
    QCOMPARE(Andreas, QString("Andreas"));
#else
    QSKIP("QTextCodec disabled");
#endif
}

// ------------------------------------------------------------------------------
void TST_VTextStream::status_real_read_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<double>("expected_f");
    QTest::addColumn<QString>("expected_w");
    QTest::addColumn<QList<int>>("results");

    QTest::newRow("1.23 abc   ") << QString("1.23 abc   ") << 1.23 << QString("abc")
                                 << (QList<int>()
                                     << static_cast<int>(VTextStream::Ok)
                                     << static_cast<int>(VTextStream::ReadCorruptData)
                                     << static_cast<int>(VTextStream::Ok) << static_cast<int>(VTextStream::Ok)
                                     << static_cast<int>(VTextStream::ReadPastEnd));
}

void TST_VTextStream::status_real_read()
{
    QFETCH(QString, input);
    QFETCH(double, expected_f);
    QFETCH(QString, expected_w);
    QFETCH(QList<int>, results);

    VTextStream s(&input);
    double f = 0.0;
    QString w;
    s >> f;
    QCOMPARE(static_cast<int>(s.status()), results.at(0));
    QCOMPARE(f, expected_f);
    s >> f;
    QCOMPARE(static_cast<int>(s.status()), results.at(1));
    s.resetStatus();
    QCOMPARE(static_cast<int>(s.status()), results.at(2));
    s >> w;
    QCOMPARE(static_cast<int>(s.status()), results.at(3));
    QCOMPARE(w, expected_w);
    s >> f;
    QCOMPARE(static_cast<int>(s.status()), results.at(4));
}

void TST_VTextStream::status_integer_read()
{
    VTextStream s("123 abc   ");
    int i;
    QString w;
    s >> i;
    QCOMPARE(s.status(), VTextStream::Ok);
    s >> i;
    QCOMPARE(s.status(), VTextStream::ReadCorruptData);
    s.resetStatus();
    QCOMPARE(s.status(), VTextStream::Ok);
    s >> w;
    QCOMPARE(s.status(), VTextStream::Ok);
    QCOMPARE(w, QString("abc"));
    s >> i;
    QCOMPARE(s.status(), VTextStream::ReadPastEnd);
}

void TST_VTextStream::status_word_read()
{
    VTextStream s("abc ");
    QString w;
    s >> w;
    QCOMPARE(s.status(), VTextStream::Ok);
    s >> w;
    QCOMPARE(s.status(), VTextStream::ReadPastEnd);
}

class FakeBuffer : public QBuffer
{
protected:
    qint64 writeData(const char *c, qint64 i) { return m_lock ? 0 : QBuffer::writeData(c, i); }

public:
    FakeBuffer(bool locked = false)
      : m_lock(locked)
    {
    }
    void setLocked(bool locked) { m_lock = locked; }

private:
    bool m_lock;
};

void TST_VTextStream::status_write_error()
{
    FakeBuffer fb(false);
    QVERIFY(fb.open(QBuffer::ReadWrite));
    VTextStream fs(&fb);
#ifdef WITH_TEXTCODEC
    fs.setCodec(QTextCodec::codecForName("latin1"));
#endif
    /* first write some initial content */
    fs << "hello";
    fs.flush();
    QCOMPARE(fs.status(), VTextStream::Ok);
    QCOMPARE(fb.data(), QByteArray("hello"));
    /* then test that writing can cause an error */
    fb.setLocked(true);
    fs << "error";
    fs.flush();
    QCOMPARE(fs.status(), VTextStream::WriteFailed);
    QCOMPARE(fb.data(), QByteArray("hello"));
    /* finally test that writing after an error doesn't change the stream any more */
    fb.setLocked(false);
    fs << "can't do that";
    fs.flush();
    QCOMPARE(fs.status(), VTextStream::WriteFailed);
    QCOMPARE(fb.data(), QByteArray("hello"));
}

void TST_VTextStream::alignAccountingStyle()
{
    {
        QString result;
        VTextStream out(&result);
        out.setFieldAlignment(VTextStream::AlignAccountingStyle);
        out.setFieldWidth(4);
        out.setPadChar('0');
        out << -1;
        QCOMPARE(result, QLatin1String("-001"));
    }

    {
        QString result;
        VTextStream out(&result);
        out.setFieldAlignment(VTextStream::AlignAccountingStyle);
        out.setFieldWidth(4);
        out.setPadChar('0');
        out << "-1";
        QCOMPARE(result, QLatin1String("00-1"));
    }

    {
        QString result;
        VTextStream out(&result);
        out.setFieldAlignment(VTextStream::AlignAccountingStyle);
        out.setFieldWidth(6);
        out.setPadChar('0');
        out << -1.2;
        QCOMPARE(result, QLatin1String("-001.2"));
    }

    {
        QString result;
        VTextStream out(&result);
        out.setFieldAlignment(VTextStream::AlignAccountingStyle);
        out.setFieldWidth(6);
        out.setPadChar('0');
        out << "-1.2";
        QCOMPARE(result, QLatin1String("00-1.2"));
    }
}

void TST_VTextStream::setCodec()
{
#ifdef WITH_TEXTCODEC
    QByteArray ba("\xe5 v\xe6r\n\xc3\xa5 v\xc3\xa6r\n");
    QString res = QLatin1String("\xe5 v\xe6r");

    VTextStream stream(ba);
    stream.setCodec("ISO 8859-1");
    QCOMPARE(stream.readLine(), res);
    stream.setCodec("UTF-8");
    QCOMPARE(stream.readLine(), res);
#else
    QSKIP("QTextCodec disabled");
#endif
}

void TST_VTextStream::double_write_with_flags_data()
{
    QTest::addColumn<double>("number");
    QTest::addColumn<QString>("output");
    QTest::addColumn<int>("numberFlags");
    QTest::addColumn<int>("realNumberNotation");

    QTest::newRow("-ForceSign") << -1.23 << QString("-1.23") << static_cast<int>(VTextStream::ForceSign) << 0;
    QTest::newRow("+ForceSign") << 1.23 << QString("+1.23") << static_cast<int>(VTextStream::ForceSign) << 0;
    QTest::newRow("inf") << qInf() << QString("inf") << 0 << 0;
    QTest::newRow("-inf") << -qInf() << QString("-inf") << 0 << 0;
    QTest::newRow("inf uppercase") << qInf() << QString("INF") << static_cast<int>(VTextStream::UppercaseDigits) << 0;
    QTest::newRow("-inf uppercase") << -qInf() << QString("-INF") << static_cast<int>(VTextStream::UppercaseDigits)
                                    << 0;
    QTest::newRow("nan") << qQNaN() << QString("nan") << 0 << 0;
    QTest::newRow("NAN") << qQNaN() << QString("NAN") << static_cast<int>(VTextStream::UppercaseDigits) << 0;
    QTest::newRow("scientific") << 1.234567e+02 << QString("1.234567e+02") << 0
                                << static_cast<int>(VTextStream::ScientificNotation);
    QTest::newRow("scientific2") << 1.234567e+02 << QString("1.234567e+02")
                                 << static_cast<int>(VTextStream::UppercaseBase)
                                 << static_cast<int>(VTextStream::ScientificNotation);
    QTest::newRow("scientific uppercase")
        << 1.234567e+02 << QString("1.234567E+02") << static_cast<int>(VTextStream::UppercaseDigits)
        << static_cast<int>(VTextStream::ScientificNotation);
}

void TST_VTextStream::double_write_with_flags()
{
    QFETCH(double, number);
    QFETCH(QString, output);
    QFETCH(int, numberFlags);
    QFETCH(int, realNumberNotation);

    QString buf;
    VTextStream stream(&buf);
    if (numberFlags)
        stream.setNumberFlags(VTextStream::NumberFlag(numberFlags));
    if (realNumberNotation)
        stream.setRealNumberNotation(VTextStream::RealNumberNotation(realNumberNotation));
    stream << number;
    QCOMPARE(buf, output);
}

void TST_VTextStream::double_write_with_precision_data()
{
    QTest::addColumn<int>("precision");
    QTest::addColumn<double>("value");
    QTest::addColumn<QString>("result");

    QTest::ignoreMessage(QtWarningMsg, "VTextStream::setRealNumberPrecision: Invalid precision (-1)");
    QTest::newRow("-1") << -1 << 3.14159 << QString("3.14159");
    QTest::newRow("0") << 0 << 3.14159 << QString("3");
    QTest::newRow("1") << 1 << 3.14159 << QString("3");
    QTest::newRow("2") << 2 << 3.14159 << QString("3.1");
    QTest::newRow("3") << 3 << 3.14159 << QString("3.14");
    QTest::newRow("5") << 5 << 3.14159 << QString("3.1416");
    QTest::newRow("6") << 6 << 3.14159 << QString("3.14159");
    QTest::newRow("7") << 7 << 3.14159 << QString("3.14159");
    QTest::newRow("10") << 10 << 3.14159 << QString("3.14159");
}

void TST_VTextStream::double_write_with_precision()
{
    QFETCH(int, precision);
    QFETCH(double, value);
    QFETCH(QString, result);

    QString buf;
    VTextStream stream(&buf);
    stream.setRealNumberPrecision(precision);
    stream << value;
    QCOMPARE(buf, result);
}

void TST_VTextStream::int_read_with_locale_data()
{
    QTest::addColumn<QString>("locale");
    QTest::addColumn<QString>("input");
    QTest::addColumn<int>("output");

    QTest::newRow("C -123") << QString("C") << QString("-123") << -123;
    QTest::newRow("C +123") << QString("C") << QString("+123") << 123;
    QTest::newRow("C 12345") << QString("C") << QString("12345") << 12345;
    QTest::newRow("C 12,345") << QString("C") << QString("12,345") << 12;
    QTest::newRow("C 12.345") << QString("C") << QString("12.345") << 12;

    QTest::newRow("de_DE -123") << QString("de_DE") << QString("-123") << -123;
    QTest::newRow("de_DE +123") << QString("de_DE") << QString("+123") << 123;
    QTest::newRow("de_DE 12345") << QString("de_DE") << QString("12345") << 12345;
    QTest::newRow("de_DE 12.345") << QString("de_DE") << QString("12.345") << 12345;
    QTest::newRow("de_DE .12345") << QString("de_DE") << QString(".12345") << 0;
}

void TST_VTextStream::int_read_with_locale()
{
    QFETCH(QString, locale);
    QFETCH(QString, input);
    QFETCH(int, output);

    VTextStream stream(&input);
    stream.setLocale(QLocale(locale));
    int result;
    stream >> result;
    QCOMPARE(result, output);
}

void TST_VTextStream::int_write_with_locale_data()
{
    QTest::addColumn<QString>("locale");
    QTest::addColumn<int>("numberFlags");
    QTest::addColumn<int>("input");
    QTest::addColumn<QString>("output");

    QTest::newRow("C -123") << QString("C") << 0 << -123 << QString("-123");
    QTest::newRow("C +123") << QString("C") << static_cast<int>(VTextStream::ForceSign) << 123 << QString("+123");
    QTest::newRow("C 12345") << QString("C") << 0 << 12345 << QString("12345");

    QTest::newRow("de_DE -123") << QString("de_DE") << 0 << -123 << QString("-123");
    QTest::newRow("de_DE +123") << QString("de_DE") << static_cast<int>(VTextStream::ForceSign) << 123
                                << QString("+123");
    QTest::newRow("de_DE 12345") << QString("de_DE") << 0 << 12345 << QString("12.345");
}

void TST_VTextStream::int_write_with_locale()
{
    QFETCH(QString, locale);
    QFETCH(int, numberFlags);
    QFETCH(int, input);
    QFETCH(QString, output);

    QString result;
    VTextStream stream(&result);
    stream.setLocale(QLocale(locale));
    if (numberFlags)
        stream.setNumberFlags(VTextStream::NumberFlags(numberFlags));
    stream << input;
    QCOMPARE(result, output);
}

void TST_VTextStream::textModeOnEmptyRead()
{
    const QString filename(tempDir.path() + QLatin1String("/textmodetest.txt"));

    QFile file(filename);
    QVERIFY2(file.open(QIODevice::ReadWrite | QIODevice::Text), qPrintable(file.errorString()));
    VTextStream stream(&file);
    QVERIFY(file.isTextModeEnabled());
    QString emptyLine = stream.readLine(); // Text mode flag cleared here
    QVERIFY(file.isTextModeEnabled());
}
