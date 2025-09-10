/************************************************************************
 **
 **  @file   tst_vtextstream.h
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
#ifndef TST_VTEXTSTREAM_H
#define TST_VTEXTSTREAM_H

#include <QObject>
#include <QTemporaryDir>

class TST_VTextStream : public QObject
{
    Q_OBJECT

public:
    explicit TST_VTextStream(QObject *parent = nullptr);

public slots:
    void initTestCase();
    void cleanup();
    void cleanupTestCase();

private slots:
    void getSetCheck();
    void construction();

    // lines
    void readLineFromDevice_data();
    void readLineFromDevice();
    void readLineFromString_data();
    void readLineFromString();
    void readLineFromTextDevice_data();
    void readLineFromTextDevice();
    void readLineUntilNull();
    void readLineMaxlen_data();
    void readLineMaxlen();
    void readLinesFromBufferCRCR();
    void readLineInto();

    // all
    void readAllFromDevice_data();
    void readAllFromDevice();
    void readAllFromString_data();
    void readAllFromString();
    void readLineFromStringThenChangeString();

    // device tests
    void setDevice();

    // char operators
    void QChar_operators_FromDevice_data();
    void QChar_operators_FromDevice();
    void char_operators_FromDevice_data();
    void char_operators_FromDevice();

    // natural number read operator
    void signedShort_read_operator_FromDevice_data();
    void signedShort_read_operator_FromDevice();
    void unsignedShort_read_operator_FromDevice_data();
    void unsignedShort_read_operator_FromDevice();
    void signedInt_read_operator_FromDevice_data();
    void signedInt_read_operator_FromDevice();
    void unsignedInt_read_operator_FromDevice_data();
    void unsignedInt_read_operator_FromDevice();
    void qlonglong_read_operator_FromDevice_data();
    void qlonglong_read_operator_FromDevice();
    void qulonglong_read_operator_FromDevice_data();
    void qulonglong_read_operator_FromDevice();

    // natural number write operator
    void signedShort_write_operator_ToDevice_data();
    void signedShort_write_operator_ToDevice();
    void unsignedShort_write_operator_ToDevice_data();
    void unsignedShort_write_operator_ToDevice();
    void signedInt_write_operator_ToDevice_data();
    void signedInt_write_operator_ToDevice();
    void unsignedInt_write_operator_ToDevice_data();
    void unsignedInt_write_operator_ToDevice();
    void qlonglong_write_operator_ToDevice_data();
    void qlonglong_write_operator_ToDevice();
    void qulonglong_write_operator_ToDevice_data();
    void qulonglong_write_operator_ToDevice();

    void int_read_with_locale_data();
    void int_read_with_locale();

    void int_write_with_locale_data();
    void int_write_with_locale();

    // real number read operator
    void float_read_operator_FromDevice_data();
    void float_read_operator_FromDevice();
    void double_read_operator_FromDevice_data();
    void double_read_operator_FromDevice();

    // real number write operator
    void float_write_operator_ToDevice_data();
    void float_write_operator_ToDevice();
    void double_write_operator_ToDevice_data();
    void double_write_operator_ToDevice();

    void double_write_with_flags_data();
    void double_write_with_flags();

    void double_write_with_precision_data();
    void double_write_with_precision();

    // text read operators
    void charPtr_read_operator_FromDevice_data();
    void charPtr_read_operator_FromDevice();
    void stringRef_read_operator_FromDevice_data();
    void stringRef_read_operator_FromDevice();
    void byteArray_read_operator_FromDevice_data();
    void byteArray_read_operator_FromDevice();

    // text write operators
    void string_write_operator_ToDevice_data();
    void string_write_operator_ToDevice();
    void latin1String_write_operator_ToDevice();
    void stringview_write_operator_ToDevice();

    // other
    void skipWhiteSpace_data();
    void skipWhiteSpace();
    void lineCount_data();
    void lineCount();
    void performance();
    void hexTest_data();
    void hexTest();
    void binTest_data();
    void binTest();
    void octTest_data();
    void octTest();
    void zeroTermination();
    void ws_manipulator();
    void readNewlines_data();
    void readNewlines();
    void seek();
    void pos();
    void pos2();
    void pos3LargeFile();
    void readStdin();
    void readAllFromStdin();
    void readLineFromStdin();
    void read();
    void qbool();
    void forcePoint();
    void forceSign();
    void read0d0d0a();
    void numeralCase_data();
    void numeralCase();
    void nanInf();
    void utf8IncompleteAtBufferBoundary_data();
    void utf8IncompleteAtBufferBoundary();
    void writeSeekWriteNoBOM();

    // status
    void status_real_read_data();
    void status_real_read();
    void status_integer_read();
    void status_word_read();
    void status_write_error();

    // use case tests
    void useCase1();
    void useCase2();

    // manipulators
    void manipulators_data();
    void manipulators();

    // UTF-16 BOM (Byte Order Mark)
    void generateBOM();
    void readBomSeekBackReadBomAgain();

    // Regression tests for old bugs
    void alignAccountingStyle();
    void setCodec();

    void textModeOnEmptyRead();

private:
    void generateLineData(bool for_QString);
    void generateAllData(bool for_QString);
    void generateOperatorCharData(bool for_QString);
    void generateNaturalNumbersData(bool for_QString);
    void generateRealNumbersData(bool for_QString);
    void generateStringData(bool for_QString);
    void generateRealNumbersDataWrite();

    QTemporaryDir tempDir;
    QString testFileName;
#ifdef BUILTIN_TESTDATA
    QSharedPointer<QTemporaryDir> m_dataDir;
#endif
    const QString m_rfc3261FilePath;
    const QString m_shiftJisFilePath;
};

#endif // TST_VTEXTSTREAM_H
