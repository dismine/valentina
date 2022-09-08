////////////////////////////////////////////////////////////////////////////////
/// \file chrono.hpp
///
/// \brief This header provides definitions from the C++ header <chrono>
////////////////////////////////////////////////////////////////////////////////

/*
  The MIT License (MIT)

  Copyright (c) 2020 Matthew Rodusek All rights reserved.

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/
#ifndef BPSTD_CHRONO_HPP
#define BPSTD_CHRONO_HPP

#include "detail/config.hpp"

#include <chrono>  // std::chrono::duration, std::chrono::system_clock, etc
#include <cstdint> // std::int32_t

#if __cplusplus < 201402L
#include <QtGlobal>

#if QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#include "../vmisc/diagnostic.h"
#endif // QT_VERSION < QT_VERSION_CHECK(5, 5, 0)
#endif

BPSTD_COMPILER_DIAGNOSTIC_PREAMBLE

namespace bpstd {
  namespace chrono {

    template <typename Rep, typename Period = std::ratio<1>>
    using duration = std::chrono::duration<Rep,Period>;

    template <typename Clock, typename Duration = typename Clock::duration>
    using time_point = std::chrono::time_point<Clock, Duration>;

    using nanoseconds  = std::chrono::nanoseconds;
    using microseconds = std::chrono::microseconds;
    using milliseconds = std::chrono::milliseconds;
    using seconds      = std::chrono::seconds;
    using minutes      = std::chrono::minutes;
    using hours        = std::chrono::hours;
    using days         = std::chrono::duration<std::int32_t, std::ratio<86400>>;
    using weeks        = std::chrono::duration<std::int32_t, std::ratio<604800>>;
    using months       = std::chrono::duration<std::int32_t, std::ratio<2629746>>;
    using years        = std::chrono::duration<std::int32_t, std::ratio<31556952>>;

    using steady_clock = std::chrono::steady_clock;
    using system_clock = std::chrono::system_clock;

    template <typename Duration>
    using sys_time    = time_point<system_clock, Duration>;
    using sys_seconds = sys_time<seconds>;
    using sys_days    = sys_time<days>;


  } // namespace chrono

  //============================================================================
  // non-member functions
  //============================================================================

  //----------------------------------------------------------------------------
  // Literals
  //----------------------------------------------------------------------------

  inline namespace literals {
    inline namespace chrono_literals {

      constexpr auto operator""_h(unsigned long long x) -> std::chrono::hours;
      constexpr auto operator""_h(long double x) -> std::chrono::duration<double, std::ratio<3600,1>>;

#if __cplusplus < 201402L
      QT_WARNING_PUSH
      QT_WARNING_DISABLE_CLANG("-Wuser-defined-literals")
      QT_WARNING_DISABLE_GCC("-Wliteral-suffix")

      // available only since C++14
      constexpr auto operator""h(unsigned long long x) -> std::chrono::hours;
      constexpr auto operator""h(long double x) -> std::chrono::duration<double, std::ratio<3600,1>>;

      QT_WARNING_POP
#endif

      constexpr auto operator""_min(unsigned long long x) -> chrono::minutes;
      constexpr auto operator""_min(long double x) -> chrono::duration<double, std::ratio<60>>;

#if __cplusplus < 201402L
      QT_WARNING_PUSH
      QT_WARNING_DISABLE_CLANG("-Wuser-defined-literals")
      QT_WARNING_DISABLE_GCC("-Wliteral-suffix")

      // available only since C++14
      constexpr auto operator""min(unsigned long long x) -> chrono::minutes;
      constexpr auto operator""min(long double x) -> chrono::duration<double, std::ratio<60>>;

      QT_WARNING_POP
#endif

      constexpr auto operator""_s(unsigned long long x) -> chrono::seconds;
      constexpr auto operator""_s(long double x) -> chrono::duration<double>;

#if __cplusplus < 201402L
      QT_WARNING_PUSH
      QT_WARNING_DISABLE_CLANG("-Wuser-defined-literals")
      QT_WARNING_DISABLE_GCC("-Wliteral-suffix")

      // available only since C++14
      constexpr auto operator""s(unsigned long long x) -> chrono::seconds;
      constexpr auto operator""s(long double x) -> chrono::duration<double>;

      QT_WARNING_POP
#endif

      constexpr auto operator""_ms(unsigned long long x) -> chrono::milliseconds;
      constexpr auto operator""_ms(long double x) -> chrono::duration<double, std::milli>;

#if __cplusplus < 201402L
      QT_WARNING_PUSH
      QT_WARNING_DISABLE_CLANG("-Wuser-defined-literals")
      QT_WARNING_DISABLE_GCC("-Wliteral-suffix")

      // available only since C++14
      constexpr auto operator""ms(unsigned long long x) -> chrono::milliseconds;
      constexpr auto operator""ms(long double x) -> chrono::duration<double, std::milli>;

      QT_WARNING_POP
#endif

      constexpr auto operator""_us(unsigned long long x) -> chrono::microseconds;
      constexpr auto operator""_us(long double x) -> chrono::duration<double, std::micro>;

#if __cplusplus < 201402L
      QT_WARNING_PUSH
      QT_WARNING_DISABLE_CLANG("-Wuser-defined-literals")
      QT_WARNING_DISABLE_GCC("-Wliteral-suffix")

      // available only since C++14
      constexpr auto operator""us(unsigned long long x) -> chrono::microseconds;
      constexpr auto operator""us(long double x) -> chrono::duration<double, std::micro>;

      QT_WARNING_POP
#endif

      constexpr auto operator""_ns(unsigned long long x) -> chrono::nanoseconds;
      constexpr auto operator""_ns(long double x) -> chrono::duration<double, std::nano>;

#if __cplusplus < 201402L
      QT_WARNING_PUSH
      QT_WARNING_DISABLE_CLANG("-Wuser-defined-literals")
      QT_WARNING_DISABLE_GCC("-Wliteral-suffix")

      // available only since C++14
      constexpr auto operator""ns(unsigned long long x) -> chrono::nanoseconds;
      constexpr auto operator""ns(long double x) -> chrono::duration<double, std::nano>;

      QT_WARNING_POP
#endif

    }  // namespace chrono_literals
  }  // namespace literals
} // namespace bpstd

inline BPSTD_INLINE_VISIBILITY constexpr
auto
  bpstd::literals::chrono_literals::operator""_h(unsigned long long x) -> std::chrono::hours
{
  return chrono::hours{x};
}

#if __cplusplus < 201402L
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wuser-defined-literals")
QT_WARNING_DISABLE_GCC("-Wliteral-suffix")

inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""h(unsigned long long x) -> std::chrono::hours
{
    return chrono::hours{x};
}

QT_WARNING_POP
#endif

inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""_h(long double x) -> std::chrono::duration<double, std::ratio<3600,1>>
{
  return chrono::duration<double, std::ratio<3600,1>>{x};
}

#if __cplusplus < 201402L
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wuser-defined-literals")
QT_WARNING_DISABLE_GCC("-Wliteral-suffix")

// available only since C++14
inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""h(long double x) -> std::chrono::duration<double, std::ratio<3600,1>>
{
    return chrono::duration<double, std::ratio<3600,1>>{x};
}

QT_WARNING_POP
#endif

inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""_min(unsigned long long x) -> bpstd::chrono::minutes
{
  return chrono::minutes{x};
}

#if __cplusplus < 201402L
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wuser-defined-literals")
QT_WARNING_DISABLE_GCC("-Wliteral-suffix")

// available only since C++14
inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""min(unsigned long long x) -> bpstd::chrono::minutes
{
    return chrono::minutes{x};
}

QT_WARNING_POP
#endif

inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""_min(long double x) -> bpstd::chrono::duration<double, std::ratio<60>>
{
  return chrono::duration<double, std::ratio<60>>{x};
}

#if __cplusplus < 201402L
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wuser-defined-literals")
QT_WARNING_DISABLE_GCC("-Wliteral-suffix")

// available only since C++14
inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""min(long double x) -> bpstd::chrono::duration<double, std::ratio<60>>
{
    return chrono::duration<double, std::ratio<60>>{x};
}

QT_WARNING_POP
#endif

inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""_s(unsigned long long x) -> bpstd::chrono::seconds
{
  return chrono::seconds{x};
}

#if __cplusplus < 201402L
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wuser-defined-literals")
QT_WARNING_DISABLE_GCC("-Wliteral-suffix")

// available only since C++14
inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""s(unsigned long long x) -> bpstd::chrono::seconds
{
    return chrono::seconds{x};
}

QT_WARNING_POP
#endif

inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""_s(long double x) -> bpstd::chrono::duration<double>
{
  return chrono::duration<double>{x};
}

#if __cplusplus < 201402L
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wuser-defined-literals")
QT_WARNING_DISABLE_GCC("-Wliteral-suffix")

// available only since C++14
inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""s(long double x) -> bpstd::chrono::duration<double>
{
    return chrono::duration<double>{x};
}

QT_WARNING_POP
#endif

inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""_ms(unsigned long long x) -> bpstd::chrono::milliseconds
{
  return chrono::milliseconds{x};
}

#if __cplusplus < 201402L
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wuser-defined-literals")
QT_WARNING_DISABLE_GCC("-Wliteral-suffix")

// available only since C++14
inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""ms(unsigned long long x) -> bpstd::chrono::milliseconds
{
    return chrono::milliseconds{x};
}

QT_WARNING_POP
#endif

inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""_ms(long double x) -> bpstd::chrono::duration<double, std::milli>
{
  return chrono::duration<double, std::milli>{x};
}

#if __cplusplus < 201402L
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wuser-defined-literals")
QT_WARNING_DISABLE_GCC("-Wliteral-suffix")

// available only since C++14
inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""ms(long double x) -> bpstd::chrono::duration<double, std::milli>
{
    return chrono::duration<double, std::milli>{x};
}

QT_WARNING_POP
#endif

inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator ""_us(unsigned long long x) -> bpstd::chrono::microseconds
{
  return chrono::microseconds{x};
}

#if __cplusplus < 201402L
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wuser-defined-literals")
QT_WARNING_DISABLE_GCC("-Wliteral-suffix")

// available only since C++14
inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator ""us(unsigned long long x) -> bpstd::chrono::microseconds
{
    return chrono::microseconds{x};
}

QT_WARNING_POP
#endif

inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""_us(long double x) -> bpstd::chrono::duration<double, std::micro>
{
  return chrono::duration<double, std::micro>{x};
}

#if __cplusplus < 201402L
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wuser-defined-literals")
QT_WARNING_DISABLE_GCC("-Wliteral-suffix")

// available only since C++14
inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""us(long double x) -> bpstd::chrono::duration<double, std::micro>
{
    return chrono::duration<double, std::micro>{x};
}

QT_WARNING_POP
#endif

inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""_ns(unsigned long long x) -> bpstd::chrono::nanoseconds
{
  return chrono::nanoseconds{x};
}

#if __cplusplus < 201402L
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wuser-defined-literals")
QT_WARNING_DISABLE_GCC("-Wliteral-suffix")

// available only since C++14
inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""ns(unsigned long long x) -> bpstd::chrono::nanoseconds
{
    return chrono::nanoseconds{x};
}

QT_WARNING_POP
#endif

inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""_ns(long double x) -> bpstd::chrono::duration<double, std::nano>
{
  return chrono::duration<double, std::nano>{x};
}

#if __cplusplus < 201402L
QT_WARNING_PUSH
QT_WARNING_DISABLE_CLANG("-Wuser-defined-literals")
QT_WARNING_DISABLE_GCC("-Wliteral-suffix")

// available only since C++14
inline BPSTD_INLINE_VISIBILITY constexpr
auto bpstd::literals::chrono_literals::operator""ns(long double x) -> bpstd::chrono::duration<double, std::nano>
{
    return chrono::duration<double, std::nano>{x};
}

QT_WARNING_POP
#endif

BPSTD_COMPILER_DIAGNOSTIC_POSTAMBLE

#endif /* BPSTD_CHRONO_HPP */
