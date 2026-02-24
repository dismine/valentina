/************************************************************************
 **
 **  @file   compatibility.h
 **  @author Roman Telezhynskyi <dismine(at)gmail.com>
 **  @date   16 1, 2020
 **
 **  @brief
 **  @copyright
 **  This source code is part of the Valentina project, a pattern making
 **  program, whose allow create and modeling patterns of clothing.
 **  Copyright (C) 2020 Valentina project
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
#ifndef COMPATIBILITY_H
#define COMPATIBILITY_H

#include <QFontMetrics>
#include <QLibraryInfo>
#include <QLineF>
#include <QListWidgetItem>
#include <QSet>
#include <QStringList>
#include <QVector>
#include <QtGlobal>

#include "defglobal.h"

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
// WARNING:QVariant::load: unknown user type with name QMarginsF.
// QVariant::value<T>() fails to convert unless QVariant::fromValue<T>() has been called previously.
// https://stackoverflow.com/questions/70974383/qvariantvaluet-fails-to-convert-unless-qvariantfromvaluet-has-been-c
#if QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
#define REGISTER_META_TYPE_STREAM_OPERATORS(TYPE)                                                                      \
    QMetaType::fromType<TYPE>().hasRegisteredDataStreamOperators(); // Dummy call
#else
#define REGISTER_META_TYPE_STREAM_OPERATORS(TYPE) QVariant::fromValue<TYPE>(TYPE{}); // Dummy call
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
#else
#define REGISTER_META_TYPE_STREAM_OPERATORS(TYPE) qRegisterMetaTypeStreamOperators<TYPE>(#TYPE);
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)

// Waiting fix for QTBUG-142804
// #if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
// #define QDOM_LOOP(list, var) for (const auto &(var) : list) // NOLINT(cppcoreguidelines-macro-usage)
// #define QDOM_ELEMENT(list, var) var                         // NOLINT(cppcoreguidelines-macro-usage)
// #else
// NOLINTNEXTLINE(cppcoreguidelines-macro-usage)
#define QDOM_LOOP(list, var) for (int var##_idx = 0; var##_idx < (list).size(); ++var##_idx)
#define QDOM_ELEMENT(list, var) (list).at(var##_idx) // NOLINT(cppcoreguidelines-macro-usage)
// #endif

#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
#define CHECKBOX_STATE_CHANGED &QCheckBox::checkStateChanged
#else
#define CHECKBOX_STATE_CHANGED &QCheckBox::stateChanged
#endif

class QPointF;

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
namespace Qt
{
inline namespace Literals
{
inline namespace StringLiterals
{

QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wunknown-warning-option")
QT_WARNING_DISABLE_CLANG("-Wreserved-identifier")

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEVAL inline auto operator""_L1(char ch)Q_DECL_NOEXCEPT->QLatin1Char
{
    return QLatin1Char(ch);
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEVAL inline auto operator""_L1(const char *str, size_t size)Q_DECL_NOEXCEPT->QLatin1String
{
    return QLatin1String(str, static_cast<vsizetype>(size));
}

QT_WARNING_POP

//---------------------------------------------------------------------------------------------------------------------
inline auto operator""_ba(const char *str, size_t size)Q_DECL_NOEXCEPT->QByteArray
{
    return {str, static_cast<vsizetype>(size)};
}

//---------------------------------------------------------------------------------------------------------------------
inline auto operator""_s(const char16_t *str, size_t size)Q_DECL_NOEXCEPT->QString
{
    return QString::fromUtf16(str, static_cast<vsizetype>(size));
}

} // namespace StringLiterals
} // namespace Literals
} // namespace Qt
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#ifndef Q_DECLARE_MIXED_ENUM_OPERATORS_SYMMETRIC
// like std::to_underlying
template<typename Enum>
constexpr std::underlying_type_t<Enum> qToUnderlying(Enum e) noexcept
{
    return static_cast<std::underlying_type_t<Enum>>(e);
}

// restore bit-wise enum-enum operators deprecated in C++20
// provide user-defined operators to override the deprecated operations:
#define Q_DECLARE_MIXED_ENUM_OPERATOR(op, Ret, LHS, RHS) \
    [[maybe_unused]] constexpr inline Ret operator op(LHS lhs, RHS rhs) noexcept \
    { \
        return static_cast<Ret>(qToUnderlying(lhs) op qToUnderlying(rhs)); \
    } \
    /* end */

#define Q_DECLARE_MIXED_ENUM_OPERATORS(Ret, Flags, Enum) \
    Q_DECLARE_MIXED_ENUM_OPERATOR(|, Ret, Flags, Enum) \
    Q_DECLARE_MIXED_ENUM_OPERATOR(&, Ret, Flags, Enum) \
    Q_DECLARE_MIXED_ENUM_OPERATOR(^, Ret, Flags, Enum) \
/* end */
#define Q_DECLARE_MIXED_ENUM_OPERATORS_SYMMETRIC(Ret, Flags, Enum) \
    Q_DECLARE_MIXED_ENUM_OPERATORS(Ret, Flags, Enum) \
    Q_DECLARE_MIXED_ENUM_OPERATORS(Ret, Enum, Flags) \
    /* end */
#endif

Q_DECLARE_MIXED_ENUM_OPERATORS_SYMMETRIC(int, Qt::KeyboardModifier, Qt::Key)
#endif // QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

// Contains helpful methods to hide version dependent code. It can be deprecation of method or change in API
//---------------------------------------------------------------------------------------------------------------------
template <typename T, template <typename> class C> inline auto ConvertToList(const C<T> &container) -> QList<T>
{
    return QList<T>(std::begin(container), std::end(container));
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, template <typename> class C> inline auto ConvertToStringList(const C<T> &container) -> QStringList
{
    return {ConvertToList(container)};
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, template <typename> class C> inline auto ConvertToSet(const C<T> &container) -> QSet<T>
{
    return QSet<T>(std::begin(container), std::end(container));
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, typename C> inline auto ConvertToSet(const C &container) -> QSet<T>
{
    return QSet<T>(std::begin(container), std::end(container));
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, template <typename> class C> inline auto ConvertToVector(const C<T> &container) -> QVector<T>
{
    return QVector<T>(std::begin(container), std::end(container));
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline auto ConvertToVector(const QSet<T> &container) -> QVector<T>
{
    return QVector<T>(std::begin(container), std::end(container));
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline auto Reverse(const QVector<T> &container) -> QVector<T>
{
    if (container.isEmpty())
    {
        return container;
    }
    QVector<T> reversed(container.size());
    vsizetype j = 0;
    for (vsizetype i = container.size() - 1; i >= 0; --i)
    {
        reversed.replace(j, container.at(i));
        ++j;
    }
    return reversed;
}

template <typename T, template <typename> class C>
//---------------------------------------------------------------------------------------------------------------------
inline auto Reverse(const C<T> &container) -> C<T>
{
    return ConvertToList(Reverse(ConvertToVector(container)));
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, std::enable_if_t<std::is_same_v<T, QStringList>, T> * = nullptr>
inline auto Reverse(const T &container) -> T
{
    return Reverse<QString, QList>(container);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline auto DropEventPos(const T *event) -> QPoint
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->position().toPoint();
#else
    return event->pos();
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline auto QLibraryPath(T loc) -> QString
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return QLibraryInfo::path(loc);
#else
    return QLibraryInfo::location(loc);
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, typename N> inline auto First(const T &list, N n) -> T
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return list.first(n);
#else
    Q_ASSERT(n <= list.size());
    T result;
    result.reserve(n);
    std::copy(list.begin(), list.begin() + n, std::back_inserter(result));
    return result;
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, typename N> inline auto Last(const T &list, N n) -> T
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return list.last(n);
#else
    Q_ASSERT(n <= list.size());
    T result;
    result.reserve(n);
    std::copy(list.end() - n, list.end(), std::back_inserter(result));
    return result;
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, typename N> inline auto Sliced(const T &list, N pos) -> T
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return list.sliced(pos);
#else
    Q_ASSERT(pos <= list.size());
    T result;
    result.reserve(list.size() - pos);
    std::copy(list.begin() + pos, list.end(), std::back_inserter(result));
    return result;
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, typename N> inline auto Sliced(const T &list, N pos, N n) -> T
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return list.sliced(pos, n);
#else
    Q_ASSERT(pos <= list.size());
    Q_ASSERT(n >= 0);
    Q_ASSERT(pos + n <= list.size());
    T result;
    result.reserve(n);
    std::copy(list.begin() + pos, list.begin() + pos + n, std::back_inserter(result));
    return result;
#endif
}

//---------------------------------------------------------------------------------------------------------------------
inline auto FontFromString(const QString &descrip) -> QFont
{
    using namespace Qt::Literals::StringLiterals;

    QFont font;

    if (!descrip.isEmpty())
    {
// Qt 6's QFont::toString returns a value with 17 fields, e.g.
// Ubuntu,11,-1,5,400,0,0,0,0,0,0,0,0,0,0,1
// Qt 5's QFont::fromString expects a value with 11 fields, e.g.
// Ubuntu,10,-1,5,50,0,0,0,0,0
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        const auto l = descrip.split(','_L1);
        // Qt5's QFont::fromString() isn't compatible with Qt6's QFont::toString().
        // If we were built with Qt5, don't try to process a font preference that
        // was created by Qt6.
        if (l.count() <= 11)
        {
            font.fromString(descrip);
        }
#else
        font.fromString(descrip);
#endif
    }
    return font;
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline auto TerritoryToString(const T &territory) -> QString
{
    using namespace Qt::Literals::StringLiterals;

// Since Qt 5.12 country names have spaces
#if QT_VERSION < QT_VERSION_CHECK(6, 2, 0)
    return QLocale::countryToString(territory).remove(' '_L1);
#else
    return QLocale::territoryToString(territory).remove(' '_L1);
#endif
}

//---------------------------------------------------------------------------------------------------------------------
// NOLINTNEXTLINE(readability-inconsistent-declaration-parameter-name)
template <> inline auto TerritoryToString<QLocale>(const QLocale &loc) -> QString
{
    using namespace Qt::Literals::StringLiterals;

// Since Qt 5.12 country names have spaces
#if QT_VERSION < QT_VERSION_CHECK(6, 2, 0)
    return QLocale::countryToString(loc.country()).remove(' '_L1);
#else
    return QLocale::territoryToString(loc.territory()).remove(' '_L1);
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline void SetTextAlignment(T *item, Qt::Alignment alignment)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
    item->setTextAlignment(alignment);
#else
    item->setTextAlignment(static_cast<int>(alignment));
#endif
}

//---------------------------------------------------------------------------------------------------------------------
inline void RemoveLast(QString &str)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    str.removeLast();
#else
    if (!str.isEmpty())
    {
        str.remove(str.size() - 1, 1);
    }
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template<typename T, typename Predicate>
inline auto EraseIf(QSet<T> &set, Predicate pred) -> vsizetype
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
    return erase_if(set, pred);
#else
    vsizetype count = 0;
    auto it = set.begin();
    while (it != set.end())
    {
        if (pred(*it))
        {
            it = set.erase(it);
            ++count;
        }
        else
        {
            ++it;
        }
    }
    return count;
#endif
}

#endif // COMPATIBILITY_H
