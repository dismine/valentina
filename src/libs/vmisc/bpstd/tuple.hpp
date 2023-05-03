////////////////////////////////////////////////////////////////////////////////
/// \file tuple.hpp
///
/// \brief This header provides definitions from the C++ header <tuple>
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
#ifndef BPSTD_TUPLE_HPP
#define BPSTD_TUPLE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "type_traits.hpp" // invoke_result
#include "utility.hpp"     // index_sequence, forward
#include "functional.hpp"  // invoke

#include <tuple>   // std::tuple_element, and to proxy API
#include <cstddef> // std::size_t

BPSTD_COMPILER_DIAGNOSTIC_PREAMBLE

namespace bpstd {

  //============================================================================
  // class : tuple
  //============================================================================

  template <typename...Types>
  using tuple = std::tuple<Types...>;

  //============================================================================
  // utilities : tuple
  //============================================================================

  template <std::size_t I, typename T>
  using tuple_element = std::tuple_element<I,T>;

  template <std::size_t I, typename T>
  using tuple_element_t = typename tuple_element<I, T>::type;

  //----------------------------------------------------------------------------

  template <typename T>
  using tuple_size = std::tuple_size<T>;

#if BPSTD_HAS_TEMPLATE_VARIABLES
  template <typename T>
  BPSTD_CPP17_INLINE constexpr auto tuple_size_v = tuple_size<T>::value;
#endif

  //============================================================================
  // non-member functions : class : tuple
  //============================================================================

  //----------------------------------------------------------------------------
  // Utilities
  //----------------------------------------------------------------------------

  namespace detail {
    template <typename T>
    struct is_tuple : false_type{};

    template <typename...Types>
    struct is_tuple<std::tuple<Types...>> : true_type{};
  } // namespace detail

  template <std::size_t N, typename Tuple,
            typename = enable_if_t<detail::is_tuple<remove_cvref_t<Tuple>>::value && is_lvalue_reference<Tuple>::value>>
  inline BPSTD_INLINE_VISIBILITY BPSTD_CPP14_CONSTEXPR auto get(Tuple &&t) noexcept
      -> tuple_element_t<N, remove_reference_t<Tuple>> &
  {
    return std::get<N>(t);
  }

  template <
      std::size_t N, typename Tuple,
      typename = enable_if_t<detail::is_tuple<remove_cvref_t<Tuple>>::value && !is_lvalue_reference<Tuple>::value>>
  inline BPSTD_INLINE_VISIBILITY BPSTD_CPP14_CONSTEXPR auto get(Tuple &&t) noexcept
      -> tuple_element_t<N, remove_reference_t<Tuple>> &&
  {
    return bpstd::move(std::get<N>(t));
  }

  template <typename T, typename... Types> BPSTD_CPP14_CONSTEXPR auto get(tuple<Types...> &t) noexcept -> T &;
  template <typename T, typename... Types> BPSTD_CPP14_CONSTEXPR auto get(tuple<Types...> &&t) noexcept -> T &&;
  template <typename T, typename... Types>
  BPSTD_CPP14_CONSTEXPR auto get(const tuple<Types...> &t) noexcept -> const T &;
  template <typename T, typename... Types>
  BPSTD_CPP14_CONSTEXPR auto get(const tuple<Types...> &&t) noexcept -> const T &&;

  //----------------------------------------------------------------------------

  namespace detail {

    // primary template left undefined
    template <typename Fn, typename Seq, typename Tuple>
    struct apply_result_impl;

    template <typename Fn, std::size_t...Idx, typename Tuple>
    struct apply_result_impl<Fn, index_sequence<Idx...>, Tuple>
      : invoke_result<Fn, tuple_element_t<Idx, Tuple>...>{};

    template <typename Fn, typename Tuple>
    struct apply_result : apply_result_impl<
      Fn,
      make_index_sequence<tuple_size<remove_reference_t<Tuple>>::value>,
      Tuple
    >{};

    template <typename Fn, typename Tuple>
    using apply_result_t = typename apply_result<Fn, Tuple>::type;
  } // namespace detail

  /// \brief Invokes the function \p fn using the arguments in \p tuple
  ///
  /// This invokes \p fn as if it were a call to bpstd::invoke using the
  /// arguments in \p tuple
  ///
  /// \param fn the function to invoke
  /// \param tuple the tuple of arguments to pass to fn
  /// \return the result from \p fn
  template <typename Fn, typename Tuple>
  constexpr auto apply(Fn &&fn, Tuple &&tuple) -> detail::apply_result_t<Fn, Tuple>;

  /// \brief Constructs a type \p T using the arguments in \p tuple
  ///
  /// \tparam T the type to construct
  /// \param tuple the tuple of arguments to pass to T's constructor
  template <typename T, typename Tuple> constexpr auto make_from_tuple(Tuple &&tuple) -> T;

} // namespace bpstd

//==============================================================================
// definitions : non-member functions : class : tuple
//==============================================================================

//------------------------------------------------------------------------------
// Utilities
//------------------------------------------------------------------------------

namespace bpstd { namespace detail {

  template <typename T, std::size_t Index, typename...Types>
  struct index_of_impl;

  template <typename T, std::size_t Index, typename Type0, typename...Types>
  struct index_of_impl<T, Index, Type0, Types...>
    : index_of_impl<T, Index + 1, Types...>{};

  template <typename T, std::size_t Index, typename...Types>
  struct index_of_impl<T, Index, T, Types...>
    : integral_constant<std::size_t, Index>{};

  template <typename T, typename...Types>
  struct index_of : index_of_impl<T,0,Types...>{};

}} // namespace bpstd::detail

template <typename T, typename... Types>
inline BPSTD_INLINE_VISIBILITY BPSTD_CPP14_CONSTEXPR auto bpstd::get(tuple<Types...> &t) noexcept -> T &
{
  return std::get<detail::index_of<T,Types...>::value>(t);
}

template <typename T, typename... Types>
inline BPSTD_INLINE_VISIBILITY BPSTD_CPP14_CONSTEXPR auto bpstd::get(tuple<Types...> &&t) noexcept -> T &&
{
  return move(std::get<detail::index_of<T,Types...>::value>(t));
}

template <typename T, typename... Types>
inline BPSTD_INLINE_VISIBILITY BPSTD_CPP14_CONSTEXPR auto bpstd::get(const tuple<Types...> &t) noexcept -> const T &
{
  return std::get<detail::index_of<T,Types...>::value>(t);
}

template <typename T, typename... Types>
inline BPSTD_INLINE_VISIBILITY BPSTD_CPP14_CONSTEXPR auto bpstd::get(const tuple<Types...> &&t) noexcept -> const T &&
{
  return move(std::get<detail::index_of<T,Types...>::value>(t));
}

//==============================================================================
// definition : apply
//==============================================================================

#if defined(_MSC_VER)
# pragma warning(push)
# pragma warning(disable:4100) // MSVC warns that 'tuple' is not used below
#endif

namespace bpstd {
  namespace detail {
  template <typename Fn, typename Tuple, std::size_t... I>
  inline BPSTD_INLINE_VISIBILITY constexpr auto apply_impl(Fn &&fn, Tuple &&tuple, index_sequence<I...>)
      -> apply_result_t<Fn, Tuple>
  {
  return ::bpstd::invoke(bpstd::forward<Fn>(fn), std::get<I>(bpstd::forward<Tuple>(tuple))...);
    }
  } // namespace detail
} // namespace bpstd

#if defined(_MSC_VER)
# pragma warning(pop)
#endif

template <typename Fn, typename Tuple>
inline BPSTD_INLINE_VISIBILITY constexpr auto bpstd::apply(Fn &&fn, Tuple &&tuple)
    -> bpstd::detail::apply_result_t<Fn, Tuple>
{
  return detail::apply_impl(
    bpstd::forward<Fn>(fn),
    bpstd::forward<Tuple>(tuple),
    make_index_sequence<tuple_size<remove_reference_t<Tuple>>::value>{}
  );
}

//==============================================================================
// definition : make_from_tuple
//==============================================================================

#if defined(_MSC_VER)
# pragma warning(push)
# pragma warning(disable:4100) // MSVC warns that 'tuple' is not used below
#endif

namespace bpstd {
  namespace detail {
  template <typename T, typename Tuple, std::size_t... I>
  inline BPSTD_INLINE_VISIBILITY constexpr auto make_from_tuple_impl(Tuple &&tuple, index_sequence<I...>) -> T
  {
  return T(std::get<I>(bpstd::forward<Tuple>(tuple))...);
    }
  } // namespace detail
} // namespace bpstd

#if defined(_MSC_VER)
# pragma warning(pop)
#endif

template <typename T, typename Tuple>
inline BPSTD_INLINE_VISIBILITY constexpr auto bpstd::make_from_tuple(Tuple &&tuple) -> T
{
  return detail::make_from_tuple_impl<T>(
    bpstd::forward<Tuple>(tuple),
    make_index_sequence<tuple_size<remove_reference_t<Tuple>>::value>{}
  );
}

BPSTD_COMPILER_DIAGNOSTIC_POSTAMBLE

#endif /* BPSTD_TUPLE_HPP */
