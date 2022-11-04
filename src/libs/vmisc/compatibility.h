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

#include <QtGlobal>
#include <QStringList>
#include <QSet>
#include <QVector>
#include <QFontMetrics>

#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)

class QPointF;

// Contains helpful methods to hide version dependent code. It can be deprecation of method or change in API

//---------------------------------------------------------------------------------------------------------------------
template <typename T, template <typename> class Cont>
inline auto ConstFirst (const Cont<T> &container) -> const T&
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    return container.constFirst();
#else
    return container.first(); // clazy:exclude=detaching-temporary
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, typename C>
inline auto ConstFirst (const C &container) -> const T&
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    return container.constFirst();
#else
    return container.first(); // clazy:exclude=detaching-temporary
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, template <typename> class Cont>
inline auto ConstLast (const Cont<T> &container) -> const T&
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    return container.constLast();
#else
    return container.last(); // clazy:exclude=detaching-temporary
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, typename C>
inline auto ConstLast (const C &container) -> const T&
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    return container.constLast();
#else
    return container.last(); // clazy:exclude=detaching-temporary
#endif
}

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
template <typename T, template <typename> class C>
inline auto ConvertToList(const C<T> &container) -> QList<T>
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return QList<T>(container.begin(), container.end());
#else
    return container.toList();
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, template <typename> class C>
inline auto ConvertToStringList(const C<T> &container) -> QStringList
{
    return {ConvertToList(container)};
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, template <typename> class C>
inline auto ConvertToSet(const C<T> &container) -> QSet<T>
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return QSet<T>(container.begin(), container.end());
#else
    return container.toSet();
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, typename C>
inline auto ConvertToSet(const C &container) -> QSet<T>
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return QSet<T>(container.begin(), container.end());
#else
    return container.toSet();
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T, template <typename> class C>
inline auto ConvertToVector(const C<T> &container) -> QVector<T>
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return QVector<T>(container.begin(), container.end());
#else
    return container.toVector();
#endif
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline auto ConvertToVector(const QSet<T> &container) -> QVector<T>
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    return QVector<T>(container.begin(), container.end());
#else
    return container.toList().toVector();
#endif
}

//---------------------------------------------------------------------------------------------------------------------
// NOTE: Delete if not necessary anymore
//template <typename T>
//inline void SwapItemsAt(T &container, int i, int j)
//{
//#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)
//    container.swapItemsAt(i, j);
//#else
//    container.swap(i, j);
//#endif
//}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline void Move(T &vector, int from, int to)
{
    QT_WARNING_PUSH
    // cppcheck-suppress unknownMacro
    QT_WARNING_DISABLE_GCC("-Wstrict-overflow")

#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
    Q_ASSERT_X(from >= 0 && from < vector.size(), "QVector::move(int,int)", "'from' is out-of-range");
    Q_ASSERT_X(to >= 0 && to < vector.size(), "QVector::move(int,int)", "'to' is out-of-range");
    if (from == to) // don't detach when no-op
    {
        return;
    }
    typename T::iterator b = vector.begin();
    from < to ? std::rotate(b + from, b + from + 1, b + to + 1)
              : std::rotate(b + to, b + from, b + from + 1);
#else
    vector.move(from, to);
#endif // QT_VERSION < QT_VERSION_CHECK(5, 6, 0)

    QT_WARNING_POP
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline auto Reverse(const QVector<T> &container) -> QVector<T>
{
    if (container.isEmpty())
    {
        return container;
    }
    QVector<T> reversed(container.size());
    qint32 j = 0;
    for (qint32 i = container.size() - 1; i >= 0; --i)
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
template <typename T, typename std::enable_if<std::is_same<T, QStringList>::value, T>::type* = nullptr>
inline auto Reverse(const T &container) -> T
{
    return Reverse<QString, QList>(container);
}

//---------------------------------------------------------------------------------------------------------------------
template <typename Cont, typename Input>
inline void AppendTo(Cont &container, const Input &input)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
    container.append(input);
#else
    for (auto &item : input)
    {
        container.append(item);
    }
#endif // QT_VERSION >= QT_VERSION_CHECK(5, 5, 0)
}

//---------------------------------------------------------------------------------------------------------------------
template <typename T>
inline auto SetIntersects(const QSet<T> &set1, const QSet<T> &set2) -> bool
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    return set1.intersects(set2);
#else
    const bool otherIsBigger = set2.size() > set1.size();
    const QSet<T> &smallestSet = otherIsBigger ? set1 : set2;
    const QSet<T> &biggestSet = otherIsBigger ? set2 : set1;
    typename QSet<T>::const_iterator i = smallestSet.cbegin();
    typename QSet<T>::const_iterator e = smallestSet.cend();
    while (i != e)
    {
        if (biggestSet.contains(*i))
        {
            return true;
        }
        ++i;
    }
    return false;
#endif
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

#endif // COMPATIBILITY_H
