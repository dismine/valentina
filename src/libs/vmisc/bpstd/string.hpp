////////////////////////////////////////////////////////////////////////////////
/// \file string.hpp
///
/// \brief This header provides definitions from the C++ header <string>
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
#ifndef BPSTD_STRING_HPP
#define BPSTD_STRING_HPP

#include "detail/config.hpp"

#include <string>

BPSTD_COMPILER_DIAGNOSTIC_PREAMBLE

namespace bpstd {

  inline namespace literals {
    inline namespace string_literals {

    auto operator""_s(const char *s, std::size_t len) -> std::string;
    auto operator""_s(const char16_t *s, std::size_t len) -> std::u16string;
    auto operator""_s(const char32_t *s, std::size_t len) -> std::u32string;
    auto operator""_s(const wchar_t *s, std::size_t len) -> std::wstring;

    } // inline namespace string_literals
  } // inline namespace literals

} // namespace bpstd

inline BPSTD_INLINE_VISIBILITY auto bpstd::literals::string_literals::operator""_s(const char *s, std::size_t len)
    -> std::string
{
  return std::string{s, len};
}

inline BPSTD_INLINE_VISIBILITY auto bpstd::literals::string_literals::operator""_s(const char16_t *s, std::size_t len)
    -> std::u16string
{
  return std::u16string{s, len};
}

inline BPSTD_INLINE_VISIBILITY auto bpstd::literals::string_literals::operator""_s(const char32_t *s, std::size_t len)
    -> std::u32string
{
  return std::u32string{s, len};
}

inline BPSTD_INLINE_VISIBILITY auto bpstd::literals::string_literals::operator""_s(const wchar_t *s, std::size_t len)
    -> std::wstring
{
  return std::wstring{s, len};
}

BPSTD_COMPILER_DIAGNOSTIC_POSTAMBLE

#endif /* BPSTD_STRING_HPP */
