/***************************************************************************************************
 **
 **  Copyright (C) 2015 Roman Telezhynskyi <dismine(at)gmail.com>
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

#ifndef QMUDEF_H
#define QMUDEF_H

#include "qmuparser_global.h"
#include "qmuparserdef.h"

#ifndef Q_DISABLE_ASSIGN
#define Q_DISABLE_ASSIGN(Class) Class &operator=(const Class &) = delete;
#endif

#ifndef Q_DISABLE_ASSIGN_MOVE
#define Q_DISABLE_ASSIGN_MOVE(Class)                                                                                   \
    Q_DISABLE_ASSIGN(Class)                                                                                            \
    Class(Class &&) = delete;                                                                                          \
    Class &operator=(Class &&) = delete;
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wattributes")

class QLocale;
class QChar;
class QString;

QMUPARSERSHARED_EXPORT auto SupportedLocale(const QLocale &locale) -> bool;

QMUPARSERSHARED_EXPORT auto LocalePositiveSign(const QLocale &locale) -> QChar;
QMUPARSERSHARED_EXPORT auto LocaleNegativeSign(const QLocale &locale) -> QChar;
QMUPARSERSHARED_EXPORT auto LocaleSign0(const QLocale &locale) -> QChar;
QMUPARSERSHARED_EXPORT auto LocaleSign1(const QLocale &locale) -> QChar;
QMUPARSERSHARED_EXPORT auto LocaleSign2(const QLocale &locale) -> QChar;
QMUPARSERSHARED_EXPORT auto LocaleSign3(const QLocale &locale) -> QChar;
QMUPARSERSHARED_EXPORT auto LocaleSign4(const QLocale &locale) -> QChar;
QMUPARSERSHARED_EXPORT auto LocaleSign5(const QLocale &locale) -> QChar;
QMUPARSERSHARED_EXPORT auto LocaleSign6(const QLocale &locale) -> QChar;
QMUPARSERSHARED_EXPORT auto LocaleSign7(const QLocale &locale) -> QChar;
QMUPARSERSHARED_EXPORT auto LocaleSign8(const QLocale &locale) -> QChar;
QMUPARSERSHARED_EXPORT auto LocaleSign9(const QLocale &locale) -> QChar;
QMUPARSERSHARED_EXPORT auto LocaleExpUpper(const QLocale &locale) -> QChar;
QMUPARSERSHARED_EXPORT auto LocaleExpLower(const QLocale &locale) -> QChar;
QMUPARSERSHARED_EXPORT auto LocaleDecimalPoint(const QLocale &locale) -> QChar;
QMUPARSERSHARED_EXPORT auto LocaleGroupSeparator(const QLocale &locale) -> QChar;

#define INIT_LOCALE_VARIABLES(locale)                                                                                  \
    const QChar positiveSign = LocalePositiveSign((locale));                                                           \
    const QChar negativeSign = LocaleNegativeSign((locale));                                                           \
    const QChar sign0 = LocaleSign0((locale));                                                                         \
    const QChar sign1 = LocaleSign1((locale));                                                                         \
    const QChar sign2 = LocaleSign2((locale));                                                                         \
    const QChar sign3 = LocaleSign3((locale));                                                                         \
    const QChar sign4 = LocaleSign4((locale));                                                                         \
    const QChar sign5 = LocaleSign5((locale));                                                                         \
    const QChar sign6 = LocaleSign6((locale));                                                                         \
    const QChar sign7 = LocaleSign7((locale));                                                                         \
    const QChar sign8 = LocaleSign8((locale));                                                                         \
    const QChar sign9 = LocaleSign9((locale));                                                                         \
    const QChar expUpper = LocaleExpUpper((locale));                                                                   \
    const QChar expLower = LocaleExpLower((locale));                                                                   \
    const QChar decimalPoint = LocaleDecimalPoint((locale));                                                           \
    const QChar groupSeparator = LocaleGroupSeparator((locale));

enum class VariableRegex : quint8
{
    Variable,
    KnownMeasurement
};

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
Q_DECL_CONST_FUNCTION inline auto qHash(VariableRegex type, uint seed = 0) noexcept -> uint
{
    auto underlyingValue = static_cast<typename std::underlying_type<VariableRegex>::type>(type);
    return ::qHash(underlyingValue, seed);
}
#endif

QMUPARSERSHARED_EXPORT auto NameRegExp(VariableRegex type = VariableRegex::Variable) -> QString;

QT_WARNING_POP

Q_REQUIRED_RESULT static inline auto QmuFuzzyComparePossibleNulls(double p1, double p2) -> bool;
static inline auto QmuFuzzyComparePossibleNulls(double p1, double p2) -> bool
{
    if (qFuzzyIsNull(p1))
    {
        return qFuzzyIsNull(p2);
    }

    if (qFuzzyIsNull(p2))
    {
        return false;
    }

    return qFuzzyCompare(p1, p2);
}

QMUPARSERSHARED_EXPORT auto ReadVal(const QString &formula, qreal &val, const QLocale &locale, const QChar &decimal,
                                    const QChar &thousand) -> qmusizetype;

QMUPARSERSHARED_EXPORT auto FindFirstNotOf(const QString &string, const QString &chars, qmusizetype pos = 0)
    -> qmusizetype;

#endif // QMUDEF_H
