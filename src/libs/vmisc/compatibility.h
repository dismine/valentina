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

class QPointF;

#if QT_VERSION < QT_VERSION_CHECK(5, 8, 0)
//---------------------------------------------------------------------------------------------------------------------
inline bool operator==(QChar lhs, const QString &rhs) Q_DECL_NOEXCEPT
{
    return rhs.size() == 1 && lhs == rhs[0];
}

//---------------------------------------------------------------------------------------------------------------------
inline bool operator!=(QChar lhs, const QString &rhs) Q_DECL_NOEXCEPT
{
    return !(lhs == rhs);
}

//---------------------------------------------------------------------------------------------------------------------
inline bool operator==(const QString &lhs, QChar rhs) Q_DECL_NOEXCEPT
{
    return rhs == lhs;
}
#endif

#if QT_VERSION < QT_VERSION_CHECK(5, 10, 0)
//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline bool operator==(QChar c1, char16_t c2) Q_DECL_NOEXCEPT
{
    return c1 == QChar(static_cast<ushort>(c2));
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline bool operator!=(QChar c1, char16_t c2) Q_DECL_NOEXCEPT
{
    return !(c1 == QChar(static_cast<ushort>(c2)));
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline bool operator==(char16_t c1, QChar c2) Q_DECL_NOEXCEPT
{
    return c2 == c1;
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline bool operator!=(char16_t c1, QChar c2) Q_DECL_NOEXCEPT
{
    return c2 != c1;
}
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 4, 0)
namespace Qt
{
inline namespace Literals
{
inline namespace StringLiterals
{

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline auto operator"" _L1(char ch) Q_DECL_NOEXCEPT->QLatin1Char
{
    return QLatin1Char(ch);
}

//---------------------------------------------------------------------------------------------------------------------
Q_DECL_CONSTEXPR inline auto operator"" _L1(const char *str, size_t size) Q_DECL_NOEXCEPT->QLatin1String
{
    return QLatin1String(str, static_cast<vsizetype>(size));
}

//---------------------------------------------------------------------------------------------------------------------
inline auto operator"" _ba(const char *str, size_t size) Q_DECL_NOEXCEPT->QByteArray
{
    return {str, static_cast<vsizetype>(size)};
}

//---------------------------------------------------------------------------------------------------------------------
inline auto operator"" _s(const char16_t *str, size_t size) Q_DECL_NOEXCEPT->QString
{
    return QString::fromUtf16(str, static_cast<vsizetype>(size));
}

} // namespace StringLiterals
} // namespace Literals
} // namespace Qt
#endif

// Contains helpful methods to hide version dependent code. It can be deprecation of method or change in API
//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline auto Intersects(const T &l1, const T &l2, QPointF *intersectionPoint) -> typename T::IntersectType
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return l1.intersects(l2, intersectionPoint);
#else
    return l1.intersect(l2, intersectionPoint);
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, template <typename> class C> inline auto ConvertToList(const C<T> &container) -> QList<T>
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return QList<T>(container.begin(), container.end());
#else
    return container.toList();
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, template <typename> class C> inline auto ConvertToStringList(const C<T> &container) -> QStringList
{
    return {ConvertToList(container)};
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, template <typename> class C> inline auto ConvertToSet(const C<T> &container) -> QSet<T>
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return QSet<T>(container.begin(), container.end());
#else
    return container.toSet();
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, typename C> inline auto ConvertToSet(const C &container) -> QSet<T>
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return QSet<T>(container.begin(), container.end());
#else
    return container.toSet();
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, template <typename> class C> inline auto ConvertToVector(const C<T> &container) -> QVector<T>
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return QVector<T>(container.begin(), container.end());
#else
    return container.toVector();
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T> inline auto ConvertToVector(const QSet<T> &container) -> QVector<T>
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return QVector<T>(container.begin(), container.end());
#else
    return container.toList().toVector();
#endif
}

//---------------------------------------------------------------------------------------------------------------------
// NOTE: Delete if not necessary anymore
// template <typename T>
// inline void SwapItemsAt(T &container, int i, int j)
//{
// #if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
//    container.swapItemsAt(i, j);
// #else
//    container.swap(i, j);
// #endif
//}

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
template <typename T, typename std::enable_if<std::is_same<T, QStringList>::value, T>::type * = nullptr>
inline auto Reverse(const T &container) -> T
{
    return Reverse<QString, QList>(container);
}

//---------------------------------------------------------------------------------------------------------------------
inline auto TextWidth(const QFontMetrics &fm, const QString &text, int len = -1) -> int
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    return fm.horizontalAdvance(text, len);
#else
    return fm.width(text, len);
#endif
}

//---------------------------------------------------------------------------------------------------------------------
inline auto TextWidthF(const QFontMetricsF &fm, const QString &text) -> qreal
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 11, 0)
    return fm.horizontalAdvance(text);
#else
    return fm.width(text);
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template <typename Key, typename T> inline auto Insert(QMap<Key, T> &map1, const QMap<Key, T> &map2) -> void
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    map1.insert(map2);
#else
    auto i = map2.constBegin();
    while (i != map2.constEnd())
    {
        map1.insert(i.key(), i.value());
        ++i;
    }
#endif
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLocaleCharacter(const QString &character) -> QChar
{
    Q_ASSERT(character.size() == 1);
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    return character.front();
#else
    return character.at(0);
#endif
}

//---------------------------------------------------------------------------------------------------------------------
inline auto VLocaleCharacter(const QChar &character) -> QChar
{
    return character;
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
inline auto LineCenter(const QLineF &line) -> QPointF
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 8, 0)
    return line.center();
#else
    return {0.5 * line.p1().x() + 0.5 * line.p2().x(), 0.5 * line.p1().y() + 0.5 * line.p2().y()};
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
inline auto Back(const QString &str) -> QChar
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    return str.back();
#else
    return str.at(str.size() - 1);
#endif
}

//---------------------------------------------------------------------------------------------------------------------
inline auto Front(const QString &str) -> QChar
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
    return str.front();
#else
    return str.at(0);
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

#endif // COMPATIBILITY_H
