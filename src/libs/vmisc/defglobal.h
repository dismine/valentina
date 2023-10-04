/************************************************************************
 **
 **  @file   defglobal.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   13 11, 2017
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2017 Valentina project
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
#ifndef DEFGLOBAL_H
#define DEFGLOBAL_H

#include <QtGlobal>

constexpr qreal PrintDPI = 96.0;

#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
// this adds const to non-const objects (like std::as_const)
template <typename T> Q_DECL_CONSTEXPR typename std::add_const<T>::type &qAsConst(T &t) noexcept
{
    return t;
}
// prevent rvalue arguments:
template <typename T> void qAsConst(const T &&) Q_DECL_EQ_DELETE;
#endif

#ifndef Q_DISABLE_ASSIGN
#define Q_DISABLE_ASSIGN(Class) Class &operator=(const Class &) = delete;
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
#define Q_DISABLE_COPY_MOVE(Class)                                                                                     \
    Q_DISABLE_COPY(Class)                                                                                              \
    Class(Class &&) = delete;                                                                                          \
    Class &operator=(Class &&) = delete;
#endif

#ifndef Q_DISABLE_ASSIGN_MOVE
#define Q_DISABLE_ASSIGN_MOVE(Class)                                                                                   \
    Q_DISABLE_ASSIGN(Class)                                                                                            \
    Class(Class &&) = delete;                                                                                          \
    Class &operator=(Class &&) = delete;
#endif

#define SUFFIX_APPEND(x, y) x##y // NOLINT(cppcoreguidelines-macro-usage)

#define HOURS_INT(x) ((x)*3600000) // NOLINT(cppcoreguidelines-macro-usage)
#define MINUTES_INT(x) ((x)*60000) // NOLINT(cppcoreguidelines-macro-usage)
#define SECONDS_INT(x) ((x)*1000)  // NOLINT(cppcoreguidelines-macro-usage)
#define MSECONDS_INT(x) (x)        // NOLINT(cppcoreguidelines-macro-usage)
#define MICSECONDS_INT(x) (x)      // NOLINT(cppcoreguidelines-macro-usage)
#define NANOSECONDS_INT(x) (x)     // NOLINT(cppcoreguidelines-macro-usage)

#if (defined(Q_CC_GNU) && Q_CC_GNU < 409) && !defined(Q_CC_CLANG)
#define HOURS(x) HOURS_INT(x)             // NOLINT(cppcoreguidelines-macro-usage)
#define MINUTES(x) MINUTES_INT(x)         // NOLINT(cppcoreguidelines-macro-usage)
#define SECONDS(x) SECONDS_INT(x)         // NOLINT(cppcoreguidelines-macro-usage)
#define MSECONDS(x) MSECONDS_INT(x)       // NOLINT(cppcoreguidelines-macro-usage)
#define MICSECONDS(x) MICSECONDS_INT(x)   // NOLINT(cppcoreguidelines-macro-usage)
#define NANOSECONDS(x) NANOSECONDS_INT(x) // NOLINT(cppcoreguidelines-macro-usage)
#else
#if __cplusplus >= 201402L
#define HOURS(x) SUFFIX_APPEND(x, h)        // NOLINT(cppcoreguidelines-macro-usage)
#define MINUTES(x) SUFFIX_APPEND(x, min)    // NOLINT(cppcoreguidelines-macro-usage)
#define SECONDS(x) SUFFIX_APPEND(x, s)      // NOLINT(cppcoreguidelines-macro-usage)
#define MSECONDS(x) SUFFIX_APPEND(x, ms)    // NOLINT(cppcoreguidelines-macro-usage)
#define MICSECONDS(x) SUFFIX_APPEND(x, us)  // NOLINT(cppcoreguidelines-macro-usage)
#define NANOSECONDS(x) SUFFIX_APPEND(x, ns) // NOLINT(cppcoreguidelines-macro-usage)
#else
#define HOURS(x) SUFFIX_APPEND(x, _h)        // NOLINT(cppcoreguidelines-macro-usage)
#define MINUTES(x) SUFFIX_APPEND(x, _min)    // NOLINT(cppcoreguidelines-macro-usage)
#define SECONDS(x) SUFFIX_APPEND(x, _s)      // NOLINT(cppcoreguidelines-macro-usage)
#define MSECONDS(x) SUFFIX_APPEND(x, _ms)    // NOLINT(cppcoreguidelines-macro-usage)
#define MICSECONDS(x) SUFFIX_APPEND(x, _us)  // NOLINT(cppcoreguidelines-macro-usage)
#define NANOSECONDS(x) SUFFIX_APPEND(x, _ns) // NOLINT(cppcoreguidelines-macro-usage)
#endif                                       // __cplusplus >= 201402L
#endif                                       // (defined(Q_CC_GNU) && Q_CC_GNU < 409) && !defined(Q_CC_CLANG)

#if (defined(Q_CC_GNU) && Q_CC_GNU < 409) && !defined(Q_CC_CLANG)
#define V_HOURS(x) HOURS(x)             // NOLINT(cppcoreguidelines-macro-usage)
#define V_MINUTES(x) MINUTES(x)         // NOLINT(cppcoreguidelines-macro-usage)
#define V_SECONDS(x) SECONDS(x)         // NOLINT(cppcoreguidelines-macro-usage)
#define V_MSECONDS(x) MSECONDS(x)       // NOLINT(cppcoreguidelines-macro-usage)
#define V_MICSECONDS(x) MICSECONDS(x)   // NOLINT(cppcoreguidelines-macro-usage)
#define V_NANOSECONDS(x) NANOSECONDS(x) // NOLINT(cppcoreguidelines-macro-usage)
#else
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
#define V_HOURS(x) HOURS(x)             // NOLINT(cppcoreguidelines-macro-usage)
#define V_MINUTES(x) MINUTES(x)         // NOLINT(cppcoreguidelines-macro-usage)
#define V_SECONDS(x) SECONDS(x)         // NOLINT(cppcoreguidelines-macro-usage)
#define V_MSECONDS(x) MSECONDS(x)       // NOLINT(cppcoreguidelines-macro-usage)
#define V_MICSECONDS(x) MICSECONDS(x)   // NOLINT(cppcoreguidelines-macro-usage)
#define V_NANOSECONDS(x) NANOSECONDS(x) // NOLINT(cppcoreguidelines-macro-usage)
#else
#define V_HOURS(x) HOURS_INT(x)             // NOLINT(cppcoreguidelines-macro-usage)
#define V_MINUTES(x) MINUTES_INT(x)         // NOLINT(cppcoreguidelines-macro-usage)
#define V_SECONDS(x) SECONDS_INT(x)         // NOLINT(cppcoreguidelines-macro-usage)
#define V_MSECONDS(x) MSECONDS_INT(x)       // NOLINT(cppcoreguidelines-macro-usage)
#define V_MICSECONDS(x) MICSECONDS_INT(x)   // NOLINT(cppcoreguidelines-macro-usage)
#define V_NANOSECONDS(x) NANOSECONDS_INT(x) // NOLINT(cppcoreguidelines-macro-usage)
#endif                                      // QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
#endif                                      // (defined(Q_CC_GNU) && Q_CC_GNU < 409) && !defined(Q_CC_CLANG)

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
using vsizetype = qsizetype;
#else
using vsizetype = int;
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
class QTextCodec;
using VTextCodec = QTextCodec;
#endif

#endif // DEFGLOBAL_H
