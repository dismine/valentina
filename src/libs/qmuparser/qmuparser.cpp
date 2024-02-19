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

#include "qmuparser.h"

#include <QCoreApplication>
#include <QLineF>
#include <QtDebug>
#include <QtGlobal>
#include <QtMath>
#include <sstream>
#include <string>

#include "../vmisc/defglobal.h"
#include "qmudef.h"
#include "qmuparsererror.h"

/**
 * @file
 * @brief Implementation of the standard floating point QmuParser.
 */

namespace
{
//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief CSR calcs special modeling case.
 * According to case we cut a piece with @p length, split up on distance @p split and rotate splited piece on
 * angle that will create arc with length @p arcLength.
 * @param length length of cut line
 * @param split distance between two pieces
 * @param arcLength length of arc that create two pieces after rotation
 * @return an angle the second piece should be rotated
 */
auto CSR(qreal length, qreal split, qreal arcLength) -> qreal
{
    length = qAbs(length);
    arcLength = qAbs(arcLength);

    if (qFuzzyIsNull(length) || qFuzzyIsNull(split) || qFuzzyIsNull(arcLength))
    {
        return 0;
    }
    const qreal sign = std::copysign(1.0, split);

    const QLineF line(QPointF(0, 0), QPointF(0, length));

    QLineF tmp = line;
    tmp.setAngle(tmp.angle() + 90.0 * sign);
    tmp.setLength(split);

    QPointF const p1 = tmp.p2();

    tmp = QLineF(QPointF(0, length), QPointF(0, 0));
    tmp.setAngle(tmp.angle() - 90.0 * sign);
    tmp.setLength(split);

    QPointF const p2 = tmp.p2();

    const QLineF line2(p1, p2);

    qreal angle = 180;
    qreal arcL = INT_MAX;
    do
    {
        if (arcL > arcLength)
        {
            angle = angle - angle / 2.0;
        }
        else if (arcL < arcLength)
        {
            angle = angle + angle / 2.0;
        }
        else
        {
            return angle;
        }

        if (angle < 0.00001 || angle >= 360)
        {
            return 0;
        }

        tmp = line2;
        tmp.setAngle(tmp.angle() + angle * sign);

        QPointF crosPoint;
        const auto type = line.intersects(tmp, &crosPoint);
        if (type == QLineF::NoIntersection)
        {
            return 0;
        }

        QLineF const radius(crosPoint, tmp.p2());
        const qreal arcAngle = sign > 0 ? line.angleTo(radius) : radius.angleTo(line);
        arcL = (M_PI * radius.length()) / 180.0 * arcAngle;
    } while (qAbs(arcL - arcLength) > (0.5 /*mm*/ / 25.4) * PrintDPI);

    return angle;
}
} // namespace

/**
 * @brief Namespace for mathematical applications.
 */
namespace qmu
{
//---------------------------------------------------------------------------------------------------------------------
// Trigonometric function
auto QmuParser::DegreeToRadian(qreal deg) -> qreal
{
    return qDegreesToRadians(deg);
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParser::RadianToDegree(qreal rad) -> qreal
{
    return qRadiansToDegrees(rad);
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParser::Sinh(qreal v) -> qreal
{
    return sinh(v);
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParser::ASinh(qreal v) -> qreal
{
    return log(v + qSqrt(v * v + 1));
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParser::Cosh(qreal v) -> qreal
{
    return cosh(v);
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParser::ACosh(qreal v) -> qreal
{
    return log(v + qSqrt(v * v - 1));
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParser::Tanh(qreal v) -> qreal
{
    return tanh(v);
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParser::ATanh(qreal v) -> qreal
{
    return (0.5 * log((1 + v) / (1 - v)));
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParser::SinD(qreal v) -> qreal
{
    return qSin(qDegreesToRadians(v));
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParser::ASinD(qreal v) -> qreal
{
    return qRadiansToDegrees(qAsin(v));
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParser::CosD(qreal v) -> qreal
{
    return qCos(qDegreesToRadians(v));
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParser::ACosD(qreal v) -> qreal
{
    return qRadiansToDegrees(qAcos(v));
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParser::TanD(qreal v) -> qreal
{
    return qTan(qDegreesToRadians(v));
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParser::ATanD(qreal v) -> qreal
{
    return qRadiansToDegrees(qAtan(v));
}

//---------------------------------------------------------------------------------------------------------------------
// Logarithm functions

//---------------------------------------------------------------------------------------------------------------------
// Logarithm base 2
auto QmuParser::Log2(qreal v) -> qreal
{
#ifdef MUP_MATH_EXCEPTIONS
    if (v <= 0)
    {
        throw QmuParserError(ecDOMAIN_ERROR, "Log2");
    }
#endif
    return log(v) / log(2.0);
}

//---------------------------------------------------------------------------------------------------------------------
// Logarithm base 10
auto QmuParser::Log10(qreal v) -> qreal
{
#ifdef MUP_MATH_EXCEPTIONS
    if (v <= 0)
    {
        throw QmuParserError(ecDOMAIN_ERROR, "Log10");
    }
#endif
    return log10(v);
}

//---------------------------------------------------------------------------------------------------------------------
//  misc
auto QmuParser::Abs(qreal v) -> qreal
{
    return qAbs(v);
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParser::Rint(qreal v) -> qreal
{
    return qFloor(v + 0.5);
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParser::R2CM(qreal v) -> qreal
{
    return Rint(v * 10.0) / 10.0;
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParser::CSRCm(qreal length, qreal split, qreal arcLength) -> qreal
{
    length = ((length * 10.0) / 25.4) * PrintDPI;
    split = ((split * 10.0) / 25.4) * PrintDPI;
    arcLength = ((arcLength * 10.0) / 25.4) * PrintDPI;

    return CSR(length, split, arcLength);
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParser::CSRInch(qreal length, qreal split, qreal arcLength) -> qreal
{
    length = length * PrintDPI;
    split = split * PrintDPI;
    arcLength = arcLength * PrintDPI;

    return CSR(length, split, arcLength);
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParser::Sign(qreal v) -> qreal
{
    return ((v < 0) ? -1 : (v > 0) ? 1 : 0);
}

//---------------------------------------------------------------------------------------------------------------------
auto QmuParser::FMod(qreal number, qreal denom) -> qreal
{
    return fmod(number, denom);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Callback for adding multiple values.
 * @param [in] a_afArg Vector with the function arguments
 * @param [in] a_iArgc The size of a_afArg
 */
auto QmuParser::Sum(const qreal *a_afArg, qmusizetype a_iArgc) -> qreal
{
    if (a_iArgc == 0)
    {
        throw QmuParserError(
            QCoreApplication::translate("QmuParser", "too few arguments for function sum.", "parser error message"));
    }
    qreal fRes = 0;
    for (int i = 0; i < a_iArgc; ++i)
    {
        fRes += a_afArg[i];
    }
    return fRes;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Callback for averaging multiple values.
 * @param [in] a_afArg Vector with the function arguments
 * @param [in] a_iArgc The size of a_afArg
 */
auto QmuParser::Avg(const qreal *a_afArg, qmusizetype a_iArgc) -> qreal
{
    if (a_iArgc == 0)
    {
        throw QmuParserError(
            QCoreApplication::translate("QmuParser", "too few arguments for function sum.", "parser error message"));
    }
    qreal fRes = 0;
    for (int i = 0; i < a_iArgc; ++i)
    {
        fRes += a_afArg[i];
    }
    return fRes / static_cast<qreal>(a_iArgc);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Callback for determining the minimum value out of a vector.
 * @param [in] a_afArg Vector with the function arguments
 * @param [in] a_iArgc The size of a_afArg
 */
auto QmuParser::Min(const qreal *a_afArg, qmusizetype a_iArgc) -> qreal
{
    if (a_iArgc == 0)
    {
        throw QmuParserError(
            QCoreApplication::translate("QmuParser", "too few arguments for function min.", "parser error message"));
    }
    qreal fRes = a_afArg[0];
    for (int i = 0; i < a_iArgc; ++i)
    {
        fRes = qMin(fRes, a_afArg[i]);
    }
    return fRes;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Callback for determining the maximum value out of a vector.
 * @param [in] a_afArg Vector with the function arguments
 * @param [in] a_iArgc The size of a_afArg
 */
auto QmuParser::Max(const qreal *a_afArg, qmusizetype a_iArgc) -> qreal
{
    if (a_iArgc == 0)
    {
        throw QmuParserError(
            QCoreApplication::translate("QmuParser", "too few arguments for function min.", "parser error message"));
    }
    qreal fRes = a_afArg[0];
    for (int i = 0; i < a_iArgc; ++i)
    {
        fRes = qMax(fRes, a_afArg[i]);
    }
    return fRes;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Default value recognition callback.
 * @param [in] a_szExpr Pointer to the expression
 * @param [in, out] a_iPos Pointer to an index storing the current position within the expression
 * @param [out] a_fVal Pointer where the value should be stored in case one is found.
 * @return 1 if a value was found 0 otherwise.
 */
auto QmuParser::IsVal(const QString &a_szExpr, qmusizetype *a_iPos, qreal *a_fVal, const QLocale &locale, bool cNumbers,
                      const QChar &decimal, const QChar &thousand) -> int
{
    qreal fVal(0);

    qmusizetype const pos =
        ReadVal(a_szExpr, fVal, locale != QLocale::c() && cNumbers ? QLocale::c() : locale, decimal, thousand);

    if (pos == -1)
    {
        return 0;
    }

    *a_iPos += pos;
    *a_fVal = fVal;
    return 1;
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Constructor.
 *
 * Call QmuParserBase class constructor and trigger Function, Operator and Constant initialization.
 */
QmuParser::QmuParser()
  : QmuParserBase()
{
    AddValIdent(IsVal);

    Init();
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Define the character sets.
 * @sa DefineNameChars, DefineOprtChars, DefineInfixOprtChars
 *
 * This function is used for initializing the default character sets that define
 * the characters to be useable in function and variable names and operators.
 */
void QmuParser::InitCharSets()
{
    DefineNameChars(QStringLiteral("0123456789_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"));
    DefineOprtChars(QStringLiteral("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+-*^/?<>=#!$%&|~'_{}"));
    DefineInfixOprtChars(QStringLiteral("/+-*^?<>=#!$%&|~'_"));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initialize the default functions.
 */
void QmuParser::InitFun()
{
    // trigonometric helper functions
    DefineFun(QStringLiteral("degTorad"), DegreeToRadian);
    DefineFun(QStringLiteral("radTodeg"), RadianToDegree);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#define QSIN_FUN qSin<qreal>
#define QCOS_FUN qCos<qreal>
#define QTAN_FUN qTan<qreal>
#define QASIN_FUN qAsin<qreal>
#define QACOS_FUN qAcos<qreal>
#define QATAN_FUN qAtan<qreal>
#define QATAN2_FUN qAtan2<qreal, qreal>
#define QLN_FUN qLn<qreal>
#define QEXP_FUN qExp<qreal>
#define QSQRT_FUN qSqrt<qreal>
#else
#define QSIN_FUN qSin
#define QCOS_FUN qCos
#define QTAN_FUN qTan
#define QASIN_FUN qAsin
#define QACOS_FUN qAcos
#define QATAN_FUN qAtan
#define QATAN2_FUN qAtan2
#define QLN_FUN qLn
#define QEXP_FUN qExp
#define QSQRT_FUN qSqrt
#endif

    // trigonometric functions
    DefineFun(QStringLiteral("sin"), QSIN_FUN);
    DefineFun(QStringLiteral("cos"), QCOS_FUN);
    DefineFun(QStringLiteral("tan"), QTAN_FUN);
    DefineFun(QStringLiteral("sinD"), SinD);
    DefineFun(QStringLiteral("cosD"), CosD);
    DefineFun(QStringLiteral("tanD"), TanD);
    // arcus functions
    DefineFun(QStringLiteral("asin"), QASIN_FUN);
    DefineFun(QStringLiteral("acos"), QACOS_FUN);
    DefineFun(QStringLiteral("atan"), QATAN_FUN);
    DefineFun(QStringLiteral("atan2"), QATAN2_FUN);
    DefineFun(QStringLiteral("asinD"), ASinD);
    DefineFun(QStringLiteral("acosD"), ACosD);
    DefineFun(QStringLiteral("atanD"), ATanD);
    // hyperbolic functions
    DefineFun(QStringLiteral("sinh"), Sinh);
    DefineFun(QStringLiteral("cosh"), Cosh);
    DefineFun(QStringLiteral("tanh"), Tanh);
    // arcus hyperbolic functions
    DefineFun(QStringLiteral("asinh"), ASinh);
    DefineFun(QStringLiteral("acosh"), ACosh);
    DefineFun(QStringLiteral("atanh"), ATanh);
    // Logarithm functions
    DefineFun(QStringLiteral("log2"), Log2);
    DefineFun(QStringLiteral("log10"), Log10);
    DefineFun(QStringLiteral("log"), Log10);
    DefineFun(QStringLiteral("ln"), QLN_FUN);
    // misc
    DefineFun(QStringLiteral("exp"), QEXP_FUN);
    DefineFun(QStringLiteral("sqrt"), QSQRT_FUN);
    DefineFun(QStringLiteral("sign"), Sign);
    DefineFun(QStringLiteral("rint"), Rint);
    DefineFun(QStringLiteral("r2cm"), R2CM);
    DefineFun(QStringLiteral("csrCm"), CSRCm);
    DefineFun(QStringLiteral("csrInch"), CSRInch);
    DefineFun(QStringLiteral("abs"), Abs);
    DefineFun(QStringLiteral("fmod"), FMod);
    // Functions with variable number of arguments
    DefineFun(QStringLiteral("sum"), Sum);
    DefineFun(QStringLiteral("avg"), Avg);
    DefineFun(QStringLiteral("min"), Min);
    DefineFun(QStringLiteral("max"), Max);
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initialize constants.
 *
 * By default the QmuParser recognizes two constants. Pi ("pi") and the eulerian
 * number ("_e").
 */
void QmuParser::InitConst()
{
    DefineConst(QStringLiteral("_pi"), static_cast<qreal>(M_PI));
    DefineConst(QStringLiteral("_e"), static_cast<qreal>(M_E));
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Initialize operators.
 *
 * By default only the unary minus operator is added.
 */
void QmuParser::InitOprt()
{
    DefineInfixOprt(LocaleNegativeSign(m_locale), UnaryMinus);
}

//---------------------------------------------------------------------------------------------------------------------
void QmuParser::OnDetectVar(const QString &pExpr, qmusizetype &nStart, qmusizetype &nEnd)
{
    Q_UNUSED(pExpr)
    Q_UNUSED(nStart)
    Q_UNUSED(nEnd)
    // this is just sample code to illustrate modifying variable names on the fly.
    // I'm not sure anyone really needs such a feature...
    /*


    string sVar(pExpr->begin()+nStart, pExpr->begin()+nEnd);
    string sRepl = std::string("_") + sVar + "_";

    int nOrigVarEnd = nEnd;
    cout << "variable detected!\n";
    cout << "  Expr: " << *pExpr << "\n";
    cout << "  Start: " << nStart << "\n";
    cout << "  End: " << nEnd << "\n";
    cout << "  Var: \"" << sVar << "\"\n";
    cout << "  Repl: \"" << sRepl << "\"\n";
    nEnd = nStart + sRepl.length();
    cout << "  End: " << nEnd << "\n";
    pExpr->replace(pExpr->begin()+nStart, pExpr->begin()+nOrigVarEnd, sRepl);
    cout << "  New expr: " << *pExpr << "\n";
    */
}

//---------------------------------------------------------------------------------------------------------------------
/**
 * @brief Numerically differentiate with regard to a variable.
 * @param [in] a_Var Pointer to the differentiation variable.
 * @param [in] a_fPos Position at which the differentiation should take place.
 * @param [in] a_fEpsilon Epsilon used for the numerical differentiation.
 *
 * Numerical differentiation uses a 5 point operator yielding a 4th order
 * formula. The default value for epsilon is 0.00074 which is
 * numeric_limits<double>::epsilon() ^ (1/5) as suggested in the muQmuParser
 * forum:
 *
 * http://sourceforge.net/forum/forum.php?thread_id=1994611&forum_id=462843
 */
// cppcheck-suppress unusedFunction
auto QmuParser::Diff(qreal *a_Var, qreal a_fPos, qreal a_fEpsilon) const -> qreal
{
    qreal fRes(0), fBuf(*a_Var), f[4] = {0, 0, 0, 0}, fEpsilon(a_fEpsilon);

    // Backwards compatible calculation of epsilon inc case the user doesnt provide
    // his own epsilon
    if (qFuzzyIsNull(fEpsilon))
    {
        fEpsilon = qFuzzyIsNull(a_fPos) ? static_cast<qreal>(1e-10) : static_cast<qreal>(1e-7) * a_fPos;
    }

    *a_Var = a_fPos + 2 * fEpsilon;
    f[0] = Eval();
    *a_Var = a_fPos + 1 * fEpsilon;
    f[1] = Eval();
    *a_Var = a_fPos - 1 * fEpsilon;
    f[2] = Eval();
    *a_Var = a_fPos - 2 * fEpsilon;
    f[3] = Eval();
    *a_Var = fBuf; // restore variable

    fRes = (-f[0] + 8 * f[1] - 8 * f[2] + f[3]) / (12 * fEpsilon);
    return fRes;
}
} // namespace qmu
