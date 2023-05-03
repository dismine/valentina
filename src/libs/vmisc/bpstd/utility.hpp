////////////////////////////////////////////////////////////////////////////////
/// \file utility.hpp
///
/// \brief This header provides definitions from the C++ header <utility>
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
#ifndef BPSTD_UTILITY_HPP
#define BPSTD_UTILITY_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "detail/config.hpp"
#include "detail/move.hpp" // IWYU pragma: export
#include "type_traits.hpp" // add_const_t

#include <utility> // to proxy the API
#include <cstddef> // std::size_t

BPSTD_COMPILER_DIAGNOSTIC_PREAMBLE

namespace bpstd {

  //============================================================================
  // struct : in_place_t
  //============================================================================

  /// \brief This function is a special disambiguation tag for variadic
  ///        functions, used in any and optional
  ///
  /// \note Calling this function results in undefined behaviour.
  struct in_place_t
  {
    explicit in_place_t() = default;
  };
  BPSTD_CPP17_INLINE constexpr in_place_t in_place{};

  //============================================================================
  // in_place_type_t
  //============================================================================

  /// \brief This function is a special disambiguation tag for variadic
  ///        functions, used in any and optional
  ///
  /// \note Calling this function results in undefined behaviour.
  template<typename T>
  struct in_place_type_t
  {
    explicit in_place_type_t() = default;
  };

#if BPSTD_HAS_TEMPLATE_VARIABLES
  template<typename T>
  BPSTD_CPP17_INLINE constexpr in_place_type_t<T> in_place_type{};
#endif

  //============================================================================
  // in_place_index_t
  //============================================================================

  /// \brief This function is a special disambiguation tag for variadic
  ///        functions, used in any and optional
  ///
  /// \note Calling this function results in undefined behaviour.
  template<std::size_t I> struct in_place_index_t
  {
    explicit in_place_index_t() = default;
  };

#if BPSTD_HAS_TEMPLATE_VARIABLES
  template<std::size_t I>
  BPSTD_CPP17_INLINE constexpr in_place_index_t<I> in_place_index{};
#endif

  //============================================================================
  // non-member functions
  //============================================================================

  //----------------------------------------------------------------------------
  // Utilities
  //----------------------------------------------------------------------------

  /// \brief Moves a type \p x if it move-construction is non-throwing
  ///
  /// \param x the parameter to move
  /// \return an rvalue reference if nothrow moveable, const reference otherwise
  template <typename T>
  constexpr auto move_if_noexcept(T &x) noexcept -> typename bpstd::conditional<
      !bpstd::is_nothrow_move_constructible<T>::value && bpstd::is_copy_constructible<T>::value, const T &, T &&>::type;

  /// \brief Forms an lvalue reference to const type of t
  ///
  /// \param t the type to form an lvalue reference to
  /// \return the reference to const T
  template <typename T> constexpr auto as_const(T &t) noexcept -> add_const_t<T> &;
  template <typename T>
  void as_const(const T&&) = delete;

  /// \brief Replaces the value of obj with new_value and returns the old value
  ///        of obj.
  ///
  /// \pre \p T must meet the requirements of MoveConstructible.
  ///
  /// \pre It must be possible to move-assign objects of type \p U to objects of
  ///      type \p T
  ///
  /// \param obj object whose value to replace
  /// \param new_value the value to assign to obj
  template <typename T, typename U = T> BPSTD_CPP14_CONSTEXPR auto exchange(T &obj, U &&new_value) -> T;

  //============================================================================
  // class : pair
  //============================================================================

  template <typename T, typename U>
  using pair = std::pair<T,U>;

  //============================================================================
  // non-member functions : class : pair
  //============================================================================

  //----------------------------------------------------------------------------
  // Utilities
  //----------------------------------------------------------------------------

  // C++11 does not implement const pair&&
  template <std::size_t N, typename T, typename U>
  constexpr auto get(pair<T, U> &p) noexcept -> conditional_t<N == 0, T, U> &;
  template <std::size_t N, typename T, typename U>
  constexpr auto get(pair<T, U> &&p) noexcept -> conditional_t<N == 0, T, U> &&;
  template <std::size_t N, typename T, typename U>
  constexpr auto get(const pair<T, U> &p) noexcept -> const conditional_t<N == 0, T, U> &;
  template <std::size_t N, typename T, typename U>
  constexpr auto get(const pair<T, U> &&p) noexcept -> const conditional_t<N == 0, T, U> &&;

  template <typename T, typename U> constexpr auto get(pair<T, U> &p) noexcept -> T &;
  template <typename T, typename U> constexpr auto get(pair<T, U> &&p) noexcept -> T &&;
  template <typename T, typename U> constexpr auto get(const pair<T, U> &p) noexcept -> const T &;
  template <typename T, typename U> constexpr auto get(const pair<T, U> &&p) noexcept -> const T &&;

  template <typename T, typename U> constexpr auto get(pair<U, T> &p) noexcept -> T &;
  template <typename T, typename U> constexpr auto get(const pair<U, T> &p) noexcept -> const T &;
  template <typename T, typename U> constexpr auto get(pair<U, T> &&p) noexcept -> T &&;
  template <typename T, typename U> constexpr auto get(const pair<U, T> &&p) noexcept -> const T &&;

  //============================================================================
  // struct : integer_sequence
  //============================================================================

  template <typename T, T... Ints>
  struct integer_sequence
  {
    using value_type = T;

    static constexpr auto size() noexcept -> std::size_t { return sizeof...(Ints); }
  };

  template <std::size_t... Ints>
  using index_sequence = integer_sequence<std::size_t, Ints...>;

  namespace detail {
    template <typename T, bool End, T N, T...Tails>
    struct make_integer_sequence_impl
      : make_integer_sequence_impl<T,((N-1) == T(0)), N-1, N-1, Tails...>{};

    template <typename T, T N, T...Tails>
    struct make_integer_sequence_impl<T, true, N, Tails...>
      : type_identity<integer_sequence<T, Tails...>>{};

  } // namespace detail

  template <typename T, T N>
  using make_integer_sequence
    = typename detail::make_integer_sequence_impl<T, (N==T(0)), N>::type;

  template<std::size_t N>
  using make_index_sequence = make_integer_sequence<std::size_t, N>;

  template<typename... T>
  using index_sequence_for = make_index_sequence<sizeof...(T)>;

} // namespace bpstd

//==============================================================================
// non-member functions
//==============================================================================

//------------------------------------------------------------------------------
// Utilities
//------------------------------------------------------------------------------

template <typename T>
inline BPSTD_INLINE_VISIBILITY constexpr auto bpstd::move_if_noexcept(T &x) noexcept -> typename bpstd::conditional<
    !bpstd::is_nothrow_move_constructible<T>::value && bpstd::is_copy_constructible<T>::value, const T &, T &&>::type
{
  using result_type = conditional_t<
    !is_nothrow_move_constructible<T>::value && is_copy_constructible<T>::value,
    const T&,
    T&&
  >;

  return static_cast<result_type>(x);
}

template <typename T>
inline BPSTD_INLINE_VISIBILITY constexpr auto bpstd::as_const(T &t) noexcept -> bpstd::add_const_t<T> &
{
  return t;
}

template <typename T, typename U>
inline BPSTD_INLINE_VISIBILITY BPSTD_CPP14_CONSTEXPR auto bpstd::exchange(T &obj, U &&new_value) -> T
{
  auto old_value = bpstd::move(obj);
  obj = bpstd::forward<U>(new_value);
  return old_value;
}

//==============================================================================
// definitions : non-member functions : class : pair
//==============================================================================

//------------------------------------------------------------------------------
// Utilities
//------------------------------------------------------------------------------

template <std::size_t N, typename T, typename U>
inline BPSTD_INLINE_VISIBILITY constexpr auto bpstd::get(pair<T, U> &p) noexcept -> bpstd::conditional_t<N == 0, T, U> &
{
  static_assert(N<=1, "N must be either 0 or 1 for get<N>(pair<T,U>)");

  return std::get<N>(p);
}

template <std::size_t N, typename T, typename U>
inline BPSTD_INLINE_VISIBILITY constexpr auto bpstd::get(pair<T, U> &&p) noexcept
    -> bpstd::conditional_t<N == 0, T, U> &&
{
  static_assert(N<=1, "N must be either 0 or 1 for get<N>(pair<T,U>)");

  return move(std::get<N>(p));
}

template <std::size_t N, typename T, typename U>
inline BPSTD_INLINE_VISIBILITY constexpr auto bpstd::get(const pair<T, U> &p) noexcept
    -> const bpstd::conditional_t<N == 0, T, U> &
{
  static_assert(N<=1, "N must be either 0 or 1 for get<N>(pair<T,U>)");

  return std::get<N>(p);
}

template <std::size_t N, typename T, typename U>
inline BPSTD_INLINE_VISIBILITY constexpr auto bpstd::get(const pair<T, U> &&p) noexcept
    -> const bpstd::conditional_t<N == 0, T, U> &&
{
  static_assert(N<=1, "N must be either 0 or 1 for get<N>(pair<T,U>)");

  return move(std::get<N>(p));
}

template <typename T, typename U>
inline BPSTD_INLINE_VISIBILITY constexpr auto bpstd::get(pair<T, U> &p) noexcept -> T &
{
  return p.first;
}

template <typename T, typename U>
inline BPSTD_INLINE_VISIBILITY constexpr auto bpstd::get(const pair<T, U> &p) noexcept -> const T &
{
  return p.first;
}

template <typename T, typename U>
inline BPSTD_INLINE_VISIBILITY constexpr auto bpstd::get(pair<T, U> &&p) noexcept -> T &&
{
  return move(p.first);
}

template <typename T, typename U>
inline BPSTD_INLINE_VISIBILITY constexpr auto bpstd::get(const pair<T, U> &&p) noexcept -> const T &&
{
  return move(p.first);
}

template <typename T, typename U>
inline BPSTD_INLINE_VISIBILITY constexpr auto bpstd::get(pair<U, T> &p) noexcept -> T &
{
  return p.second;
}

template <typename T, typename U>
inline BPSTD_INLINE_VISIBILITY constexpr auto bpstd::get(const pair<U, T> &p) noexcept -> const T &
{
  return p.second;
}

template <typename T, typename U>
inline BPSTD_INLINE_VISIBILITY constexpr auto bpstd::get(pair<U, T> &&p) noexcept -> T &&
{
  return move(p.second);
}

template <typename T, typename U>
inline BPSTD_INLINE_VISIBILITY constexpr auto bpstd::get(const pair<U, T> &&p) noexcept -> const T &&
{
  return move(p.second);
}

BPSTD_COMPILER_DIAGNOSTIC_POSTAMBLE

#endif /* BPSTD_UTILITY_HPP */
