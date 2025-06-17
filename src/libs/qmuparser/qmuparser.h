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

#ifndef QMUPARSER_H
#define QMUPARSER_H

#include <QString>
#include <QtGlobal>
#include <locale>

#include "qmuparser_global.h"
#include "qmuparserbase.h"

/**
 * @file
 * @brief Definition of the standard floating point parser.
 */

namespace qmu
{
QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wsuggest-final-types")

/** @brief Mathematical expressions parser.
 *
 * Standard implementation of the mathematical expressions parser.
 * Can be used as a reference implementation for subclassing the parser.
 *
 * <small>
 * (C) 2011 Ingo Berg<br>
 * muparser(at)gmx.de
 * </small>
 */
/* final */ class QMUPARSERSHARED_EXPORT QmuParser : public QmuParserBase
{
public:
    QmuParser();
    void InitCharSets() override;
    void InitFun() override;
    void InitConst() override;
    void InitOprt() override;
    void OnDetectVar(const QString &pExpr, qmusizetype &nStart, qmusizetype &nEnd) override;
    auto Diff(qreal *a_Var, qreal a_fPos, qreal a_fEpsilon = 0) const -> qreal;

protected:
    static auto IsVal(const QString &a_szExpr, qmusizetype *a_iPos, qreal *a_fVal, const QLocale &locale, bool cNumbers,
                      const QChar &decimal, const QChar &thousand) -> int;
    // hyperbolic functions
    static auto Sinh(qreal) -> qreal;
    static auto Cosh(qreal) -> qreal;
    static auto Tanh(qreal) -> qreal;
    // arcus hyperbolic functions
    static auto ASinh(qreal) -> qreal;
    static auto ACosh(qreal) -> qreal;
    static auto ATanh(qreal) -> qreal;
    // functions working with degrees
    static auto DegreeToRadian(qreal) -> qreal;
    static auto RadianToDegree(qreal) -> qreal;
    static auto SinD(qreal) -> qreal;
    static auto CosD(qreal) -> qreal;
    static auto TanD(qreal) -> qreal;
    static auto ASinD(qreal) -> qreal;
    static auto ACosD(qreal) -> qreal;
    static auto ATanD(qreal) -> qreal;

    // Logarithm functions
    static auto Log2(qreal) -> qreal;  // Logarithm Base 2
    static auto Log10(qreal) -> qreal; // Logarithm Base 10
    // misc
    static auto Abs(qreal) -> qreal;
    static auto Rint(qreal) -> qreal;
    static auto R2CM(qreal) -> qreal;
    static auto CSRCm(qreal length, qreal split, qreal arcLength) -> qreal;
    static auto CSRInch(qreal length, qreal split, qreal arcLength) -> qreal;
    static auto Sign(qreal) -> qreal;
    static auto FMod(qreal, qreal) -> qreal;
    // Prefix operators
    // !!! Unary Minus is a MUST if you want to use negative signs !!!
    static auto UnaryMinus(qreal v) -> qreal;
    // Functions with variable number of arguments
    static auto Sum(const qreal *, qmusizetype) -> qreal; // sum
    static auto Avg(const qreal *, qmusizetype) -> qreal; // mean value
    static auto Min(const qreal *, qmusizetype) -> qreal; // minimum
    static auto Max(const qreal *, qmusizetype) -> qreal; // maximum
};

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Callback for the unary minus operator.
 * @param v The value to negate
 * @return -v
 */
inline auto QmuParser::UnaryMinus(qreal v) -> qreal
{
    return -v;
}

} // namespace qmu

#endif // QMUPARSER_H
