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

#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)

/*
 * Warning/diagnostic handling
 */

#define QT_DO_PRAGMA(text)                      _Pragma(#text)
#if defined(Q_CC_INTEL) && defined(Q_CC_MSVC)
/* icl.exe: Intel compiler on Windows */
#  undef QT_DO_PRAGMA                           /* not needed */
#  define QT_WARNING_PUSH                       __pragma(warning(push))
#  define QT_WARNING_POP                        __pragma(warning(pop))
#  define QT_WARNING_DISABLE_MSVC(number)
#  define QT_WARNING_DISABLE_INTEL(number)      __pragma(warning(disable: number))
#  define QT_WARNING_DISABLE_CLANG(text)
#  define QT_WARNING_DISABLE_GCC(text)
#elif defined(Q_CC_INTEL)
/* icc: Intel compiler on Linux or OS X */
#  define QT_WARNING_PUSH                       QT_DO_PRAGMA(warning(push))
#  define QT_WARNING_POP                        QT_DO_PRAGMA(warning(pop))
#  define QT_WARNING_DISABLE_INTEL(number)      QT_DO_PRAGMA(warning(disable: number))
#  define QT_WARNING_DISABLE_MSVC(number)
#  define QT_WARNING_DISABLE_CLANG(text)
#  define QT_WARNING_DISABLE_GCC(text)
#elif defined(Q_CC_MSVC) && _MSC_VER >= 1500
#  undef QT_DO_PRAGMA                           /* not needed */
#  define QT_WARNING_PUSH                       __pragma(warning(push))
#  define QT_WARNING_POP                        __pragma(warning(pop))
#  define QT_WARNING_DISABLE_MSVC(number)       __pragma(warning(disable: number))
#  define QT_WARNING_DISABLE_INTEL(number)
#  define QT_WARNING_DISABLE_CLANG(text)
#  define QT_WARNING_DISABLE_GCC(text)
#elif defined(Q_CC_CLANG)
#  define QT_WARNING_PUSH                       QT_DO_PRAGMA(clang diagnostic push)
#  define QT_WARNING_POP                        QT_DO_PRAGMA(clang diagnostic pop)
#  define QT_WARNING_DISABLE_CLANG(text)        QT_DO_PRAGMA(clang diagnostic ignored text)
#  define QT_WARNING_DISABLE_GCC(text)          QT_DO_PRAGMA(GCC diagnostic ignored text)// GCC directives work in Clang too
#  define QT_WARNING_DISABLE_INTEL(number)
#  define QT_WARNING_DISABLE_MSVC(number)
#elif defined(Q_CC_GNU) && (__GNUC__ * 100 + __GNUC_MINOR__ >= 406)
#  define QT_WARNING_PUSH                       QT_DO_PRAGMA(GCC diagnostic push)
#  define QT_WARNING_POP                        QT_DO_PRAGMA(GCC diagnostic pop)
#  define QT_WARNING_DISABLE_GCC(text)          QT_DO_PRAGMA(GCC diagnostic ignored text)
#  define QT_WARNING_DISABLE_CLANG(text)
#  define QT_WARNING_DISABLE_INTEL(number)
#  define QT_WARNING_DISABLE_MSVC(number)
#else       // All other compilers, GCC < 4.6 and MSVC < 2008
#  define QT_WARNING_DISABLE_GCC(text)
#  define QT_WARNING_PUSH
#  define QT_WARNING_POP
#  define QT_WARNING_DISABLE_INTEL(number)
#  define QT_WARNING_DISABLE_MSVC(number)
#  define QT_WARNING_DISABLE_CLANG(text)
#  define QT_WARNING_DISABLE_GCC(text)
#endif

#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#define Q_DISABLE_COPY_MOVE(Class) \
    Q_DISABLE_COPY(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;
#endif

#ifndef Q_DISABLE_ASSIGN_MOVE
#define Q_DISABLE_ASSIGN_MOVE(Class) \
    Q_DISABLE_ASSIGN(Class) \
    Class(Class &&) = delete; \
    Class &operator=(Class &&) = delete;
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wattributes")

#include <ciso646>

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

#define INIT_LOCALE_VARIABLES(locale)                        \
const QChar positiveSign   = LocalePositiveSign((locale));   \
const QChar negativeSign   = LocaleNegativeSign((locale));   \
const QChar sign0          = LocaleSign0((locale));          \
const QChar sign1          = LocaleSign1((locale));          \
const QChar sign2          = LocaleSign2((locale));          \
const QChar sign3          = LocaleSign3((locale));          \
const QChar sign4          = LocaleSign4((locale));          \
const QChar sign5          = LocaleSign5((locale));          \
const QChar sign6          = LocaleSign6((locale));          \
const QChar sign7          = LocaleSign7((locale));          \
const QChar sign8          = LocaleSign8((locale));          \
const QChar sign9          = LocaleSign9((locale));          \
const QChar expUpper       = LocaleExpUpper((locale));       \
const QChar expLower       = LocaleExpLower((locale));       \
const QChar decimalPoint   = LocaleDecimalPoint((locale));   \
const QChar groupSeparator = LocaleGroupSeparator((locale));

QMUPARSERSHARED_EXPORT QString NameRegExp();

QT_WARNING_POP

Q_REQUIRED_RESULT static inline bool QmuFuzzyComparePossibleNulls(double p1, double p2);
static inline bool QmuFuzzyComparePossibleNulls(double p1, double p2)
{
    if(qFuzzyIsNull(p1))
    {
        return qFuzzyIsNull(p2);
    }

    if(qFuzzyIsNull(p2))
    {
        return false;
    }

    return qFuzzyCompare(p1, p2);
}

QMUPARSERSHARED_EXPORT qmusizetype ReadVal(const QString &formula, qreal &val, const QLocale &locale,
                                         const QChar &decimal, const QChar &thousand);

QMUPARSERSHARED_EXPORT qmusizetype FindFirstNotOf(const QString &string, const QString &chars, qmusizetype pos = 0);

#endif // QMUDEF_H
