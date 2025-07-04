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

#ifndef QMUPDEF_H
#define QMUPDEF_H

#include <QMap>
#include <QString>
#include <QtGlobal>
#include <locale>

// Header <ciso646> is removed in C++20.
#if defined(Q_CC_MSVC) && __cplusplus <= 201703L
#include <ciso646> // and, not, or
#endif

#ifdef __INTEL_COMPILER
#include "qmuparserfixes.h"
#endif

/** @file
    @brief This file contains standard definitions used by the parser.
*/

#ifndef QMUP_VERSION
#define QMUP_VERSION "2.7.0" // Leave while we still use qmake
#endif
#define QMUP_VERSION_DATE "20191030; GC"

// Detect whether the compiler supports C++11 noexcept exception specifications.
#if defined(__clang__)
#if __has_feature(cxx_noexcept)
#define QMUP_NOEXCEPT_EXPR(x) noexcept(x) // Clang 3.0 and above have noexcept
#endif
#elif defined(__GNUC__)
#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6)) && defined(__GXX_EXPERIMENTAL_CXX0X__)
#define QMUP_NOEXCEPT_EXPR(x) noexcept(x) // GCC 4.7 and following have noexcept
#endif
#elif defined(_MSC_FULL_VER) && _MSC_FULL_VER >= 190023026
#define QMUP_NOEXCEPT_EXPR(x) noexcept(x) // Visual Studio 2015 and following have noexcept
#else
#define QMUP_NOEXCEPT_EXPR(x)
#endif

#ifndef __has_cpp_attribute
#define __has_cpp_attribute(x) 0
#endif

/** @brief If this macro is defined mathematical exceptions (div by zero) will be thrown as exceptions. */
// #define QMUP_MATH_EXCEPTIONS

/** @brief Activate this option in order to compile with OpenMP support.

  OpenMP is used only in the bulk mode it may increase the performance a bit.
*/
// #define QMUP_USE_OPENMP

/** @brief Definition of the basic parser string type. */
#define QMUP_STRING_TYPE std::wstring

class QLocale;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
using qmusizetype = qsizetype;
#else
using qmusizetype = int;
#endif

namespace qmu
{
//------------------------------------------------------------------------------
/** @brief Bytecode values.

    \attention The order of the operator entries must match the order in ParserBase::c_DefaultOprt!
*/
enum ECmdCode
{
    // The following are codes for built in binary operators
    // apart from built in operators the user has the opportunity to
    // add user defined operators.
    cmLE = 0,   ///< Operator item:  less or equal
    cmGE = 1,   ///< Operator item:  greater or equal
    cmNEQ = 2,  ///< Operator item:  not equal
    cmEQ = 3,   ///< Operator item:  equals
    cmLT = 4,   ///< Operator item:  less than
    cmGT = 5,   ///< Operator item:  greater than
    cmADD = 6,  ///< Operator item:  add
    cmSUB = 7,  ///< Operator item:  subtract
    cmMUL = 8,  ///< Operator item:  multiply
    cmDIV = 9,  ///< Operator item:  division
    cmPOW = 10, ///< Operator item:  y to the power of ...
    cmLAND = 11,
    cmLOR = 12,
    cmASSIGN = 13,  ///< Operator item:  Assignment operator
    cmBO = 14,      ///< Operator item:  opening bracket
    cmBC = 15,      ///< Operator item:  closing bracket
    cmIF = 16,      ///< For use in the ternary if-then-else operator
    cmELSE = 17,    ///< For use in the ternary if-then-else operator
    cmENDIF = 18,   ///< For use in the ternary if-then-else operator
    cmARG_SEP = 19, ///< function argument separator
    cmVAR = 20,     ///< variable item
    cmVAL = 21,     ///< value item

    // For optimization purposes
    cmVARPOW2,
    cmVARPOW3,
    cmVARPOW4,
    cmVARMUL,
    cmPOW2,

    // operators and functions
    cmFUNC,         ///< Code for a generic function item
    cmFUNC_STR,     ///< Code for a function with a string parameter
    cmFUNC_BULK,    ///< Special callbacks for Bulk mode with an additional parameter for the bulk index
    cmSTRING,       ///< Code for a string token
    cmOPRT_BIN,     ///< user defined binary operator
    cmOPRT_POSTFIX, ///< code for postfix operators
    cmOPRT_INFIX,   ///< code for infix operators
    cmEND,          ///< end of formula
    cmUNKNOWN       ///< uninitialized item
};

//------------------------------------------------------------------------------
/** @brief Types internally used by the parser.
 */
enum ETypeCode
{
    tpSTR = 0, ///< String type (Function arguments and constants only, no string variables)
    tpDBL = 1, ///< Floating point variables
    tpVOID = 2 ///< Undefined type.
};

//------------------------------------------------------------------------------
enum EParserVersionInfo
{
    pviBRIEF,
    pviFULL
};

//------------------------------------------------------------------------------
/** @brief Parser operator precedence values. */
enum EOprtAssociativity
{
    oaLEFT = 0,
    oaRIGHT = 1,
    oaNONE = 2
};

//------------------------------------------------------------------------------
/** @brief Parser operator precedence values. */
enum EOprtPrecedence
{
    // binary operators
    prLOR = 1,
    prLAND = 2,
    prLOGIC = 3,   ///< logic operators
    prCMP = 4,     ///< comparsion operators
    prADD_SUB = 5, ///< addition
    prMUL_DIV = 6, ///< multiplication/division
    prPOW = 7,     ///< power operator priority (highest)

    // infix operators
    prINFIX = 6,  ///< Signs have a higher priority than ADD_SUB, but lower than power operator
    prPOSTFIX = 6 ///< Postfix operator priority (currently unused)
};

//------------------------------------------------------------------------------
// basic types

/** @brief The stringtype used by the parser.

  Depends on wether UNICODE is used or not.
*/
using string_type = std::wstring;

/** @brief The character type used by the parser.

  Depends on wether UNICODE is used or not.
*/
using char_type = string_type::value_type;

/** @brief Typedef for easily using stringstream that respect the parser stringtype. */
using stringstream_type = std::basic_stringstream<char_type, std::char_traits<char_type>, std::allocator<char_type>>;

// Data container types

/** @brief Type used for storing variables. */
using varmap_type = std::map<QString, qreal *>;

/** @brief Type used for storing constants. */
using valmap_type = std::map<QString, qreal>;

/** @brief Type for assigning a string name to an index in the internal string table. */
using strmap_type = std::map<QString, qmusizetype>;

// Parser callbacks

/** @brief Callback type used for functions without arguments. */
using generic_fun_type = qreal (*)();

/** @brief Callback type used for functions without arguments. */
using fun_type0 = qreal (*)();

/** @brief Callback type used for functions with a single arguments. */
using fun_type1 = qreal (*)(qreal);

/** @brief Callback type used for functions with two arguments. */
using fun_type2 = qreal (*)(qreal, qreal);

/** @brief Callback type used for functions with three arguments. */
using fun_type3 = qreal (*)(qreal, qreal, qreal);

/** @brief Callback type used for functions with four arguments. */
using fun_type4 = qreal (*)(qreal, qreal, qreal, qreal);

/** @brief Callback type used for functions with five arguments. */
using fun_type5 = qreal (*)(qreal, qreal, qreal, qreal, qreal);

/** @brief Callback type used for functions with five arguments. */
using fun_type6 = qreal (*)(qreal, qreal, qreal, qreal, qreal, qreal);

/** @brief Callback type used for functions with five arguments. */
using fun_type7 = qreal (*)(qreal, qreal, qreal, qreal, qreal, qreal, qreal);

/** @brief Callback type used for functions with five arguments. */
using fun_type8 = qreal (*)(qreal, qreal, qreal, qreal, qreal, qreal, qreal, qreal);

/** @brief Callback type used for functions with five arguments. */
using fun_type9 = qreal (*)(qreal, qreal, qreal, qreal, qreal, qreal, qreal, qreal, qreal);

/** @brief Callback type used for functions with five arguments. */
using fun_type10 = qreal (*)(qreal, qreal, qreal, qreal, qreal, qreal, qreal, qreal, qreal, qreal);

/** @brief Callback type used for functions without arguments. */
using bulkfun_type0 = qreal (*)(int, int);

/** @brief Callback type used for functions with a single arguments. */
using bulkfun_type1 = qreal (*)(int, int, qreal);

/** @brief Callback type used for functions with two arguments. */
using bulkfun_type2 = qreal (*)(int, int, qreal, qreal);

/** @brief Callback type used for functions with three arguments. */
using bulkfun_type3 = qreal (*)(int, int, qreal, qreal, qreal);

/** @brief Callback type used for functions with four arguments. */
using bulkfun_type4 = qreal (*)(int, int, qreal, qreal, qreal, qreal);

/** @brief Callback type used for functions with five arguments. */
using bulkfun_type5 = qreal (*)(int, int, qreal, qreal, qreal, qreal, qreal);

/** @brief Callback type used for functions with five arguments. */
using bulkfun_type6 = qreal (*)(int, int, qreal, qreal, qreal, qreal, qreal, qreal);

/** @brief Callback type used for functions with five arguments. */
using bulkfun_type7 = qreal (*)(int, int, qreal, qreal, qreal, qreal, qreal, qreal, qreal);

/** @brief Callback type used for functions with five arguments. */
using bulkfun_type8 = qreal (*)(int, int, qreal, qreal, qreal, qreal, qreal, qreal, qreal, qreal);

/** @brief Callback type used for functions with five arguments. */
using bulkfun_type9 = qreal (*)(int, int, qreal, qreal, qreal, qreal, qreal, qreal, qreal, qreal, qreal);

/** @brief Callback type used for functions with five arguments. */
using bulkfun_type10 = qreal (*)(int, int, qreal, qreal, qreal, qreal, qreal, qreal, qreal, qreal, qreal, qreal);

/** @brief Callback type used for functions with a variable argument list. */
using multfun_type = qreal (*)(const qreal *, qmusizetype);

/** @brief Callback type used for functions taking a string as an argument. */
using strfun_type1 = qreal (*)(const QString &);

/** @brief Callback type used for functions taking a string and a value as arguments. */
using strfun_type2 = qreal (*)(const QString &, qreal);

/** @brief Callback type used for functions taking a string and two values as arguments. */
using strfun_type3 = qreal (*)(const QString &, qreal, qreal);

/** @brief Callback used for functions that identify values in a string. */
using identfun_type = int (*)(const QString &, qmusizetype *, qreal *, const QLocale &, bool, const QChar &,
                              const QChar &);

/** @brief Callback used for variable creation factory functions. */
using facfun_type = qreal *(*)(const QString &, void *);
} // namespace qmu

#endif
