// Copyright (C) 2024 Christian Ehrlicher <ch.ehrlicher@gmx.de>
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only

#ifndef QPAINTERSTATEGUARD_H
#define QPAINTERSTATEGUARD_H

#include <QtGlobal>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QtCore/qtclasshelpermacros.h>
#endif // QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)

#include <QtGui/qpainter.h>

#if QT_VERSION < QT_VERSION_CHECK(6, 6, 0)
#if __has_cpp_attribute(nodiscard) >= 201907L /* used for both P1771 and P1301... */
// [[nodiscard]] constructor (P1771)
#ifndef Q_NODISCARD_CTOR
#define Q_NODISCARD_CTOR [[nodiscard]]
#endif
#else
#ifndef Q_NODISCARD_CTOR
#define Q_NODISCARD_CTOR
#endif
#endif
#endif // QT_VERSION < QT_VERSION_CHECK(6, 6, 0)

QT_BEGIN_NAMESPACE

#if QT_VERSION < QT_VERSION_CHECK(6, 2, 4)
// pure compile-time micro-optimization for our own headers, so not documented:
template<typename T>
constexpr inline void qt_ptr_swap(T *&lhs, T *&rhs) noexcept
{
    T *tmp = lhs;
    lhs = rhs;
    rhs = tmp;
}
#endif // QT_VERSION < QT_VERSION_CHECK(6, 5, 0)

class QPainterStateGuard
{
    Q_DISABLE_COPY(QPainterStateGuard)
public:
    enum class InitialState : quint8
    {
        Save,
        NoSave,
    };

    QPainterStateGuard(QPainterStateGuard &&other) noexcept
        : m_painter(std::exchange(other.m_painter, nullptr))
        , m_level(std::exchange(other.m_level, 0))
    {}

    auto operator=(QPainterStateGuard &&other) noexcept -> QPainterStateGuard &
    {
        QPainterStateGuard moved(std::move(other));
        swap(moved);
        return *this;
    }

    void swap(QPainterStateGuard &other) noexcept
    {
        qt_ptr_swap(m_painter, other.m_painter);
        std::swap(m_level, other.m_level);
    }

    Q_NODISCARD_CTOR
    explicit QPainterStateGuard(QPainter *painter, InitialState state = InitialState::Save)
        : m_painter(painter)
    {
        verifyPainter();
        if (state == InitialState::Save)
        {
            save();
        }
    }

    ~QPainterStateGuard()
    {
        while (m_level > 0)
        {
            restore();
        }
    }

    void save()
    {
        verifyPainter();
        m_painter->save();
        ++m_level;
    }

    void restore()
    {
        verifyPainter();
        Q_ASSERT(m_level > 0);
        --m_level;
        m_painter->restore();
    }

private:
    void verifyPainter()
    {
        Q_ASSERT(m_painter);
    }

    QPainter *m_painter;
    int m_level = 0;
};

QT_END_NAMESPACE

#endif  // QPAINTERSTATEGUARD_H
