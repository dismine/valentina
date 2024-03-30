/***************************************************************************************************
 **
 **  Copyright (C) 2013 Ingo Berg
 **
 **  Permission is hereby granted, free of charge, to any person obtaining a copy of this
 **  software and associated documentation files (the "Software"), to deal in the Software
 **  without restriction, including without limitation the rights to use, copy, modify,
 **  merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 **  permit persons to whom the Software is furnished to do so, subject to the following conditions:
 **
 **  The above copyright notice and this permission notice shall be included in all copies or
 **  substantial portions of the Software.
 **
 **  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
 **  NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 **  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 **  DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 **  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **
 ******************************************************************************************************/

#ifndef QMUPARSERTEST_H
#define QMUPARSERTEST_H

#include <QMetaObject>
#include <QObject>
#include <QRandomGenerator>
#include <QString>
#include <QVector>
#include <QtGlobal>
#include <locale>
#include <stdlib.h>

#include "qmuparser.h"
#include "qmuparser_global.h"
#include "qmuparserdef.h"
#include "qmuparsererror.h"

/**
 * @file
 * @brief This file contains the parser test class.
 */

/**
 * @brief Namespace for test cases.
 */
namespace qmu::Test
{
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Test cases for unit testing.
 *
 * (C) 2004-2011 Ingo Berg
 */
class QMUPARSERSHARED_EXPORT QmuParserTester : public QObject // final
{
    Q_OBJECT // NOLINT

public:
    using testfun_type = int (QmuParserTester::*)();

    explicit QmuParserTester(QObject *parent = nullptr);

public slots:
    void Run();

private:
    Q_DISABLE_COPY_MOVE(QmuParserTester) // NOLINT
    QVector<testfun_type> m_vTestFun;
    static int c_iCount;

    void AddTest(testfun_type a_pFun);

    // Test Double Parser
    static auto EqnTest(const QString &a_str, double a_fRes, bool a_fPass) -> int;
    static auto EqnTestWithVarChange(const QString &a_str, double a_fRes1, double a_fVar1, double a_fRes2,
                                     double a_fVar2) -> int;
    static auto ThrowTest(const QString &a_str, int a_iErrc, bool a_bFail = true) -> int;

    // Test Bulkmode
    static auto EqnTestBulk(const QString &a_str, double a_fRes[4], bool a_fPass) -> int;

    // Multiarg callbacks
    static auto f1of1(qreal v) -> qreal { return v; }

    static auto f1of2(qreal v, qreal) -> qreal { return v; }

    static auto f2of2(qreal, qreal v) -> qreal { return v; }

    static auto f1of3(qreal v, qreal, qreal) -> qreal { return v; }

    static auto f2of3(qreal, qreal v, qreal) -> qreal { return v; }

    static auto f3of3(qreal, qreal, qreal v) -> qreal { return v; }

    static auto f1of4(qreal v, qreal, qreal, qreal) -> qreal { return v; }

    static auto f2of4(qreal, qreal v, qreal, qreal) -> qreal { return v; }

    static auto f3of4(qreal, qreal, qreal v, qreal) -> qreal { return v; }

    static auto f4of4(qreal, qreal, qreal, qreal v) -> qreal { return v; }

    static auto f1of5(qreal v, qreal, qreal, qreal, qreal) -> qreal { return v; }

    static auto f2of5(qreal, qreal v, qreal, qreal, qreal) -> qreal { return v; }

    static auto f3of5(qreal, qreal, qreal v, qreal, qreal) -> qreal { return v; }

    static auto f4of5(qreal, qreal, qreal, qreal v, qreal) -> qreal { return v; }

    static auto f5of5(qreal, qreal, qreal, qreal, qreal v) -> qreal { return v; }

    static auto Min(qreal a_fVal1, qreal a_fVal2) -> qreal { return (a_fVal1 < a_fVal2) ? a_fVal1 : a_fVal2; }

    static auto Max(qreal a_fVal1, qreal a_fVal2) -> qreal { return (a_fVal1 > a_fVal2) ? a_fVal1 : a_fVal2; }

    static auto plus2(qreal v1) -> qreal { return v1 + 2; }

    static auto times3(qreal v1) -> qreal { return v1 * 3; }

    static auto sqr(qreal v1) -> qreal { return v1 * v1; }

    static auto sign(qreal v) -> qreal { return -v; }

    static auto add(qreal v1, qreal v2) -> qreal { return v1 + v2; }

    static auto land(qreal v1, qreal v2) -> qreal { return static_cast<int>(v1) & static_cast<int>(v2); }

    static auto FirstArg(const qreal *a_afArg, qmusizetype a_iArgc) -> qreal
    {
        if (a_iArgc == 0)
        {
            throw QmuParserError("too few arguments for function FirstArg.");
        }

        return a_afArg[0];
    }

    static auto LastArg(const qreal *a_afArg, qmusizetype a_iArgc) -> qreal
    {
        if (a_iArgc == 0)
        {
            throw QmuParserError("too few arguments for function LastArg.");
        }

        return a_afArg[a_iArgc - 1];
    }

    static auto Sum(const qreal *a_afArg, qmusizetype a_iArgc) -> qreal
    {
        if (a_iArgc == 0)
        {
            throw QmuParserError("too few arguments for function sum.");
        }

        qreal fRes = 0;
        for (int i = 0; i < a_iArgc; ++i)
        {
            fRes += a_afArg[i];
        }
        return fRes;
    }

    static auto Rnd(qreal v) -> qreal
    {
        return static_cast<qreal>(
            (1 + (v * QRandomGenerator().bounded(static_cast<qreal>(RAND_MAX)) / (RAND_MAX + 1.0))));
    }

    static auto RndWithString(const char_type *) -> qreal
    {
        return static_cast<qreal>(
            (1 + (1000.0f * QRandomGenerator().bounded(static_cast<qreal>(RAND_MAX)) / (RAND_MAX + 1.0))));
    }

    static auto Ping() -> qreal { return 10; }

    static auto ValueOf(const QString &) -> qreal { return 123; }

    static auto StrFun1(const QString &v1) -> qreal
    {
        int const val = v1.toInt();
        return static_cast<qreal>(val);
    }

    static auto StrFun2(const QString &v1, qreal v2) -> qreal
    {
        int const val = v1.toInt();
        return static_cast<qreal>(val + v2);
    }

    static auto StrFun3(const QString &v1, qreal v2, qreal v3) -> qreal
    {
        int const val = v1.toInt();
        return val + v2 + v3;
    }

    static auto StrToFloat(const QString &a_szMsg) -> qreal
    {
        qreal const val = a_szMsg.toDouble();
        return val;
    }

    // postfix operator callback
    static auto Mega(qreal a_fVal) -> qreal { return a_fVal * static_cast<qreal>(1e6); }

    static auto Micro(qreal a_fVal) -> qreal { return a_fVal * static_cast<qreal>(1e-6); }

    static auto Milli(qreal a_fVal) -> qreal { return a_fVal / static_cast<qreal>(1e3); }

    // Custom value recognition
    static auto IsHexVal(const QString &a_szExpr, qmusizetype *a_iPos, qreal *a_fVal, const QLocale &locale,
                         bool cNumbers, const QChar &decimal, const QChar &thousand) -> int;

    auto TestNames() -> int;
    auto TestSyntax() -> int;
    auto TestMultiArg() -> int;
    auto TestPostFix() -> int;
    auto TestExpression() -> int;
    auto TestInfixOprt() -> int;
    auto TestBinOprt() -> int;
    auto TestVarConst() -> int;
    auto TestInterface() -> int;
    auto TestException() -> int;
    auto TestStrArg() -> int;
    auto TestIfThenElse() -> int;
    auto TestBulkMode() -> int;

    static void Abort();
};
} // namespace qmu::Test

#endif
